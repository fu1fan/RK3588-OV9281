"""Run the unmodified script at production config/runtime paths with real /proc argv.

Requires root, unused station config/runtime paths and a host C compiler.
Network commands are stubs, so the host loopback interface is never modified.
Only the network daemons are replaced; their original argv is preserved.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'init.d/S36wifistation'
STUB = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv) {
 const char *name = strrchr(argv[0], '/');
 name = name ? name + 1 : argv[0];
 char path[4096];
 const char *root=getenv("WIFI_TEST_ROOT");
 snprintf(path,sizeof(path),"%s/calls",root);
 FILE *log = fopen(path, "a");
 for (int i=0; i<argc; i++) fprintf(log, "%s%s", i ? " " : "", argv[i]);
 fputc('\n', log); fclose(log);
 if (!strcmp(name,"ifconfig")) return 0;
 const char *pidfile = NULL;
 for (int i=1; i+1<argc; i++)
  if (!strcmp(argv[i],"-P") || !strcmp(argv[i],"-p")) pidfile=argv[i+1];
 if (!strcmp(name,"wpa_supplicant")) {
  pid_t p=fork(); if(p<0) return 1;
  if(p>0) { FILE *f=fopen(pidfile,"w"); fprintf(f,"%d\n",p); fclose(f); return 0; }
  setsid();
 }
 if (!strcmp(name,"udhcpc")) {
  FILE *f=fopen(pidfile,"w"); fprintf(f,"%d\n",getpid()); fclose(f);
 }
 close(0); close(1); close(2);
 for (;;) {
  snprintf(path,sizeof(path),"%s/associated",root);
  if (!strcmp(name,"udhcpc") && !access(path,F_OK)) {
   snprintf(path,sizeof(path),"%s/lease",root);
   FILE *f=fopen(path,"w"); fputs("leased",f); fclose(f);
  }
  usleep(100000);
 }
}
'''

class WifiStationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory()
        cls.source = Path(cls.build.name) / 'stub.c'
        cls.source.write_text(STUB)
        cls.binary = cls.source.with_suffix('')
        subprocess.run(['cc', '-static', '-Wall', '-Wextra', '-Werror',
                        str(cls.source), '-o', str(cls.binary)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.build.cleanup()

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.conf = Path('/etc/wifistation.conf')
        self.runtime = Path('/var/run/wifistation')
        if self.conf.exists() or self.runtime.exists():
            self.tmp.cleanup()
            self.fail('Run in a disposable environment without station config/runtime files')
        (self.root / 'bin').mkdir()
        for name in ['wpa_supplicant','udhcpc','ifconfig']:
            shutil.copy(self.binary, self.root / 'bin' / name)
        for name in ["cat", "tr", "awk", "rm", "sleep", "mkdir", "rmdir", "od", "sed"]:
            (self.root / "bin" / name).symlink_to(shutil.which(name))
        self.addCleanup(self.cleanup)

    def cleanup(self):
        for name in ['dhcp.pid','wpa.pid']:
            p = self.runtime / name
            if p.exists():
                pid = int(p.read_text())
                try:
                    if os.readlink('/proc/%d/exe' % pid).startswith(str(self.root)):
                        os.kill(pid, 15)
                except (FileNotFoundError, ProcessLookupError):
                    pass
        self.conf.unlink(missing_ok=True)
        shutil.rmtree(self.runtime, ignore_errors=True)
        self.tmp.cleanup()

    def config(self, text='IFACE=lo\nSSID=testnet\nPSK=secretpass\n'):
        self.conf.write_text(text)

    def run_service(self, action='start'):
        return subprocess.run(['/bin/sh', str(SCRIPT), action],
                              env=dict(os.environ, PATH=str(self.root / 'bin'),
                                       WIFI_TEST_ROOT=str(self.root)),
                              capture_output=True, text=True, timeout=20)

    def calls(self):
        p = self.root / 'calls'
        return p.read_text() if p.exists() else ''

    def test_disabled(self):
        self.assertEqual(self.run_service().returncode, 0)
        self.assertEqual(self.calls(), '')

    def test_start_uses_production_paths_and_actual_argv(self):
        self.config()
        r = self.run_service()
        self.assertEqual(r.returncode, 0, r.stdout+r.stderr)
        self.assertIn('-i lo -c /var/run/wifistation/wpa.conf', self.calls())
        self.assertIn('udhcpc -f -R', self.calls())
        conf = self.runtime / 'wpa.conf'
        self.assertIn('ssid=746573746e6574', conf.read_text())
        self.assertEqual(conf.stat().st_mode & 0o777, 0o600)

    def test_late_association_gets_dhcp_and_second_start_is_idempotent(self):
        self.config()
        self.assertEqual(self.run_service().returncode, 0)
        self.assertFalse((self.root / 'lease').exists())
        self.assertEqual(self.run_service().returncode, 0)
        self.assertEqual(self.calls().count('wpa_supplicant'), 1)
        self.assertEqual(self.calls().count('udhcpc'), 1)
        (self.root / 'associated').touch()
        end = time.monotonic()+3
        while not (self.root / 'lease').exists() and time.monotonic()<end:
            time.sleep(.05)
        self.assertTrue((self.root / 'lease').exists())

    def test_stop_uses_saved_interface_even_if_configuration_changed(self):
        self.config()
        self.assertEqual(self.run_service().returncode, 0)
        self.config('IFACE=wlan9\nSSID=other\nPSK=otherpass\n')
        r = self.run_service('stop')
        self.assertEqual(r.returncode, 0, r.stdout+r.stderr)
        self.assertIn('ifconfig lo down', self.calls())
        self.assertNotIn('wlan9 down', self.calls())
        self.assertFalse((self.runtime / 'interface').exists())

    def test_restart_and_literal_credentials(self):
        self.config('IFACE=lo\nSSID=a"b\\c\nPSK=secret"\\pass\n')
        self.assertEqual(self.run_service().returncode, 0)
        r = self.run_service('restart')
        self.assertEqual(r.returncode, 0, r.stdout+r.stderr)
        self.assertEqual(self.calls().count('wpa_supplicant'), 2)
        self.assertIn('psk="secret\\"\\\\pass"',
                      (self.runtime / 'wpa.conf').read_text())

    def test_invalid_config_and_missing_binary(self):
        self.config('IFACE=lo\nSSID=test\n')
        self.assertNotEqual(self.run_service().returncode, 0)
        self.config()
        (self.root / 'bin/udhcpc').unlink()
        self.assertNotEqual(self.run_service().returncode, 0)

    def test_stale_pid_does_not_signal_an_unrelated_process(self):
        self.runtime.mkdir()
        (self.runtime / 'interface').write_text('lo')
        process = subprocess.Popen(['/bin/sleep', '30'])
        try:
            (self.runtime / 'wpa.pid').write_text(str(process.pid))
            (self.runtime / 'dhcp.pid').write_text(str(process.pid))
            result = self.run_service('stop')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIsNone(process.poll())
        finally:
            process.terminate()
            process.wait()

    def test_open_network(self):
        self.config('IFACE=lo\nSSID=test\nKEY_MGMT=NONE\n')
        self.assertEqual(self.run_service().returncode, 0)
        self.assertNotIn('psk=', (self.runtime / 'wpa.conf').read_text())

if __name__ == '__main__':
    unittest.main()
