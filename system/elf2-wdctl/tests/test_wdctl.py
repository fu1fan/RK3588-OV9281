"""Regression tests for elf2-wdctl driven against a wrapped fake watchdog."""
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time
import unittest

HERE = Path(__file__).resolve().parent
SOURCE = HERE.parent


def wait_for(predicate, timeout=10.0, step=0.05):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        if predicate():
            return True
        time.sleep(step)
    return predicate()


def kicks(log):
    try:
        return log.read_text().count('kick\n')
    except FileNotFoundError:
        return 0


class WatchdogControlTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.tmp.name)
        cls.binary = cls.root / 'elf2-wdctl'
        subprocess.run([os.environ.get('CC', 'cc'), '-O2', '-Wall', '-Wextra',
                        '-Werror', '-o', str(cls.binary), str(SOURCE / 'elf2-wdctl.c'),
                        str(HERE / 'wdt-fake-ioctl.c'), '-Wl,--wrap=ioctl', '-Wl,--wrap=pthread_create',
                        '-lpthread'], check=True)
        cls.device = cls.root / 'watchdog'
        cls.device.write_bytes(b'')

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def start(self, name, interval='1', extra_env=None):
        log = self.root / (name + '.log')
        log.unlink(missing_ok=True)
        errors = self.root / (name + '.err')
        env = dict(os.environ, ELF2_WDCTL_TEST_LOG=str(log))
        env.update(extra_env or {})
        with errors.open('wb') as sink:
            process = subprocess.Popen(
                [str(self.binary), '-f', '-d', str(self.device), '-t', '30',
                 '-i', interval], env=env, stdout=subprocess.DEVNULL, stderr=sink)

        def cleanup():
            if process.poll() is None:
                process.kill()
            process.wait()
        self.addCleanup(cleanup)
        return process, log, errors

    def test_kicks_before_the_hardware_timeout(self):
        process, log, errors = self.start('single')
        self.assertTrue(wait_for(lambda: kicks(log) >= 3),
                        'no kick: ' + errors.read_text())
        self.assertIn('settimeout 30', log.read_text())
        process.send_signal(signal.SIGTERM)
        self.assertEqual(process.wait(timeout=5), 0)
        self.assertIn('setoptions 1', log.read_text())

    def test_actual_timeout_shortens_interval(self):
        process, log, errors = self.start('short', interval='10',
            extra_env={'ELF2_WDCTL_TEST_TIMEOUT': '2'})
        self.assertTrue(wait_for(lambda: kicks(log) >= 3, timeout=3))
        self.assertIn('actual timeout 2s, kick interval 1s', errors.read_text())
        self.assertIsNone(process.poll())

    def test_set_failure_uses_current_timeout(self):
        process, log, errors = self.start('set-fail', interval='10',
            extra_env={'ELF2_WDCTL_TEST_TIMEOUT': '2', 'ELF2_WDCTL_TEST_FAIL': 'set'})
        self.assertTrue(wait_for(lambda: kicks(log) >= 3, timeout=3))
        self.assertIn('actual timeout 2s, kick interval 1s', errors.read_text())
        self.assertIsNone(process.poll())

    def test_unknown_timeout_or_initial_kick_failure_is_fatal(self):
        for failure in ['get', 'kick']:
            process, log, errors = self.start('failure-'+failure,
                extra_env={'ELF2_WDCTL_TEST_FAIL': failure})
            self.assertEqual(process.wait(timeout=3), 1, errors.read_text())
            self.assertEqual(kicks(log), 0)

    def test_runtime_keepalive_failure_is_reported(self):
        process, log, errors = self.start('later', extra_env={'ELF2_WDCTL_TEST_FAIL': 'later'})
        self.assertEqual(process.wait(timeout=4), 1, errors.read_text())
        self.assertNotIn('setoptions', log.read_text())

    def test_malformed_options_are_rejected(self):
        for args in [['-t', '-1'], ['-t', '4294967296'], ['-i', '1oops'], ['foo'], ['-foreground']]:
            result = subprocess.run([str(self.binary), *args], capture_output=True)
            self.assertEqual(result.returncode, 2, args)

    def test_daemon_parent_reports_thread_start_failure(self):
        env = dict(os.environ, ELF2_WDCTL_TEST_FAIL='thread')
        result = subprocess.run([str(self.binary), '-d', str(self.device)],
                                env=env, capture_output=True, timeout=3)
        self.assertEqual(result.returncode, 1)
        self.assertIn(b'cannot start kick thread', result.stderr)

    def test_missing_device_fails_without_starting_a_daemon(self):
        env = dict(os.environ, ELF2_WDCTL_TEST_LOG=str(self.root / 'missing.log'))
        result = subprocess.run([str(self.binary), '-f', '-d', str(self.root / 'absent'),
                                 '-t', '30', '-i', '1'], env=env, capture_output=True,
                                text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('cannot open', result.stderr)
        self.assertFalse((self.root / 'missing.log').exists())

    def test_interval_must_be_shorter_than_the_timeout(self):
        for args in (['-t', '10', '-i', '10'], ['-t', '10', '-i', '0']):
            result = subprocess.run([str(self.binary), *args], capture_output=True,
                                    text=True)
            self.assertEqual(result.returncode, 2, args)
            self.assertIn('Usage', result.stderr)

    def test_sigterm_is_honoured_promptly(self):
        process, log, _ = self.start('stop')
        self.assertTrue(wait_for(lambda: kicks(log) >= 2))
        started = time.monotonic()
        process.send_signal(signal.SIGTERM)
        self.assertEqual(process.wait(timeout=5), 0)
        self.assertLess(time.monotonic() - started, 2.0)

    def test_sigusr1_stops_kicking_on_purpose(self):
        process, log, errors = self.start('hang')
        self.assertTrue(wait_for(lambda: kicks(log) >= 2))
        process.send_signal(signal.SIGUSR1)
        self.assertTrue(wait_for(lambda: 'stopping kicks on purpose' in
                                 errors.read_text()))
        settled = kicks(log)
        time.sleep(2.5)
        self.assertEqual(kicks(log), settled, 'kicks continued after SIGUSR1')
        self.assertIsNone(process.poll(), 'daemon exited instead of holding still')


if __name__ == '__main__':
    unittest.main()
