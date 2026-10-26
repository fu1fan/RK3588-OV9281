"""Behaviour tests for /etc/init.d/S01watchdog with stubbed userland."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'init.d' / 'S01watchdog'

# The real feeder backgrounds itself after opening the device; the stub has to
# do the same or the init script would wait for it.
DAEMON_STUB = """#!/bin/sh
echo "$*" >> LOG
setsid /bin/sleep 300 >/dev/null 2>&1 &
exit 0
"""


class WatchdogInitTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        (self.root / 'bin').mkdir()
        (self.root / 'var/run').mkdir(parents=True)
        (self.root / 'dev').mkdir()
        self.log = self.root / 'calls.log'
        daemon = self.root / 'bin/elf2-wdctl'
        daemon.write_text(DAEMON_STUB.replace('LOG', str(self.log)))
        daemon.chmod(0o755)
        self.script = self.root / 'S01watchdog'
        text = SCRIPT.read_text()
        for key, value in (('DAEMON', daemon), ('PIDFILE', self.root / 'var/run/elf2-wdctl.pid')):
            text = re.sub(r'^%s=.*$' % key, '%s=%s' % (key, value), text, flags=re.M)
        self.script.write_text(text)
        self.script.chmod(0o755)

    def tearDown(self):
        self.tmp.cleanup()

    def run_service(self, action='start', extra_env=None):
        env = dict(os.environ, PATH=str(self.root / 'bin') + ':/usr/bin:/bin')
        env.update(extra_env or {})
        return subprocess.run([str(self.script), action], env=env, capture_output=True,
                              text=True, timeout=30)

    def calls(self):
        return self.log.read_text() if self.log.exists() else ''

    def test_missing_device_is_skipped_not_fatal(self):
        result = self.run_service(extra_env={'ELF2_WATCHDOG_DEVICE': str(self.root / 'dev/absent')})
        self.assertEqual(result.returncode, 0)
        self.assertIn('SKIPPED', result.stdout)
        self.assertEqual(self.calls(), '')

    def test_present_device_starts_the_feeder_with_the_configured_interval(self):
        device = self.root / 'dev/watchdog'
        device.write_bytes(b'')
        result = self.run_service(extra_env={'ELF2_WATCHDOG_DEVICE': str(device),
                                             'ELF2_WATCHDOG_TIMEOUT': '30',
                                             'ELF2_WATCHDOG_INTERVAL': '10'})
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn('OK', result.stdout)
        self.assertIn('-d ' + str(device), self.calls())
        self.assertIn('-t 30', self.calls())
        self.assertIn('-i 10', self.calls())

    def test_missing_daemon_is_reported(self):
        device = self.root / 'dev/watchdog'
        device.write_bytes(b'')
        (self.root / 'bin/elf2-wdctl').unlink()
        result = self.run_service(extra_env={'ELF2_WATCHDOG_DEVICE': str(device)})
        self.assertEqual(result.returncode, 1)
        self.assertIn('is missing', result.stdout)


if __name__ == '__main__':
    unittest.main()
