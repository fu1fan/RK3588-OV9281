"""Behaviour tests for /etc/init.d/S36wifistation with stubbed userland.

The stub daemons are small shell scripts that fork a detached sleeper and
write its pid where the real tools would, so the service's own pid/argv
checks run exactly as they would on the board.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'init.d' / 'S36wifistation'
SLEEPER = Path(__file__).resolve().parent / 'sleeper'

WPA_STUB = """#!/bin/sh
echo "wpa_supplicant $*" >> LOG
iface=""; pidfile=""; prev=""
for a in "$@"; do
  [ "$prev" = "-i" ] && iface="$a"
  [ "$prev" = "-P" ] && pidfile="$a"
  prev="$a"
done
setsid SLEEPER "wpa_supplicant -i$iface" >/dev/null 2>&1 &
sleep 0.2
[ -n "$pidfile" ] && pgrep -n -f "wpa_supplicant -i$iface" > "$pidfile"
exit 0
"""

WPA_CLI_STUB = """#!/bin/sh
echo "wpa_cli $*" >> LOG
echo "wpa_state=${FAKE_WPA_STATE:-COMPLETED}"
"""

UDHCPC_STUB = """#!/bin/sh
echo "udhcpc $*" >> LOG
iface=""; pidfile=""; prev=""
for a in "$@"; do
  [ "$prev" = "-i" ] && iface="$a"
  [ "$prev" = "-p" ] && pidfile="$a"
  prev="$a"
done
setsid SLEEPER "udhcpc -i $iface" >/dev/null 2>&1 &
sleep 0.2
[ -n "$pidfile" ] && pgrep -n -f "udhcpc -i $iface" > "$pidfile"
exit 0
"""

IFCONFIG_STUB = """#!/bin/sh
echo "ifconfig $*" >> LOG
"""


class WifiStationTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        (self.root / 'bin').mkdir()
        (self.root / 'var/run').mkdir(parents=True)
        (self.root / 'etc').mkdir()
        (self.root / 'sys/class/net/lo').mkdir(parents=True)
        self.log = self.root / 'calls.log'
        for name, template in (('wpa_supplicant', WPA_STUB), ('wpa_cli', WPA_CLI_STUB),
                               ('udhcpc', UDHCPC_STUB), ('ifconfig', IFCONFIG_STUB)):
            path = self.root / 'bin' / name
            path.write_text(template.replace('LOG', str(self.log))
                            .replace('SLEEPER', str(SLEEPER)))
            path.chmod(0o755)
        self.script = self.root / 'S36wifistation'
        text = SCRIPT.read_text()
        for key, value in (('CONF', self.root / 'etc/wifistation.conf'),
                           ('PIDDIR', self.root / 'var/run'),
                           ('WPACONF', self.root / 'etc/wpa.conf')):
            text = re.sub(r'^%s=.*$' % key, '%s=%s' % (key, value), text, flags=re.M)
        text = text.replace('/sys/class/net', str(self.root / 'sys/class/net'))
        text = re.sub(r'^ASSOC_TIMEOUT=.*$', 'ASSOC_TIMEOUT=3', text, flags=re.M)
        text = re.sub(r'^DISCOVER_TIMEOUT=.*$', 'DISCOVER_TIMEOUT=2', text, flags=re.M)
        self.script.write_text(text)
        self.script.chmod(0o755)
        self.addCleanup(self._kill_strays)
        self.addCleanup(self.tmp.cleanup)

    def _kill_strays(self):
        # Only this test's own sleeper: match the exact executable path, so the
        # pattern cannot hit the test runner's own command line.
        subprocess.run(['pkill', '-f', str(SLEEPER)], check=False,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def run_service(self, action='start', extra_env=None):
        env = dict(os.environ, PATH=str(self.root / 'bin') + ':/usr/bin:/bin')
        env.update(extra_env or {})
        return subprocess.run([str(self.script), action], env=env, capture_output=True,
                              text=True, timeout=90)

    def calls(self):
        return self.log.read_text() if self.log.exists() else ''

    def write_conf(self, ssid='testnet', psk='secretpass'):
        body = 'SSID=%s\n' % ssid
        if psk is not None:
            body += 'PSK=%s\n' % psk
        (self.root / 'etc/wifistation.conf').write_text(body)

    def add_interface(self):
        (self.root / 'sys/class/net/wlan0/wireless').mkdir(parents=True)

    def test_without_configuration_nothing_is_started(self):
        result = self.run_service()
        self.assertEqual(result.returncode, 0)
        self.assertIn('STA mode disabled', result.stdout)
        self.assertEqual(self.calls(), '')

    def test_with_configuration_but_no_interface_it_fails_fast(self):
        self.write_conf()
        result = self.run_service()
        self.assertEqual(result.returncode, 1)
        self.assertIn('no wireless interface', result.stdout)

    def test_configured_interface_gets_wpa_supplicant_and_dhcp(self):
        self.write_conf()
        self.add_interface()
        result = self.run_service()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        calls = self.calls()
        self.assertIn('ifconfig wlan0 up', calls)
        self.assertIn('wpa_supplicant -B -D nl80211 -i wlan0', calls)
        self.assertIn('udhcpc', calls)
        config = (self.root / 'etc/wpa.conf').read_text()
        self.assertIn('ssid="testnet"', config)
        self.assertIn('psk="secretpass"', config)
        self.assertNotIn('secretpass', calls)
        self.assertNotIn('secretpass', result.stdout)

    def test_second_start_reuses_the_running_processes(self):
        self.write_conf()
        self.add_interface()
        self.assertEqual(self.run_service().returncode, 0)
        first = self.calls()
        result = self.run_service()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn('already running', result.stdout)
        # No second daemon and no second DHCP client may be spawned; re-reading
        # the association state is expected.
        self.assertEqual(self.calls().count('wpa_supplicant -B'), 1)
        self.assertEqual(self.calls().count('udhcpc -b'), 1)
        self.assertTrue(self.calls().startswith(first))

    def test_missing_dhcp_binary_is_reported(self):
        self.write_conf()
        self.add_interface()
        (self.root / 'bin/udhcpc').unlink()
        result = self.run_service()
        self.assertEqual(result.returncode, 1)
        self.assertIn('udhcpc is not installed', result.stdout)

    def test_open_network_needs_no_passphrase(self):
        self.write_conf(psk=None)
        text = self.script.read_text().replace('KEY_MGMT=WPA-PSK', 'KEY_MGMT=NONE')
        self.script.write_text(text)
        self.add_interface()
        result = self.run_service()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn('key_mgmt=NONE', (self.root / 'etc/wpa.conf').read_text())

    def test_psk_is_required_for_wpa_psk(self):
        self.write_conf(psk=None)
        self.add_interface()
        result = self.run_service()
        self.assertEqual(result.returncode, 1)
        self.assertIn('needs a PSK', result.stdout)


if __name__ == '__main__':
    unittest.main()
