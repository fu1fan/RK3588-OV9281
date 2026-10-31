"""Regression checks for the board/kernel-specific NCM startup contract."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class USBVideoTests(unittest.TestCase):
    def test_prepare_is_idempotent_when_function_is_bound(self):
        source = (ROOT / 'external/rkscript/usbdevice').read_text()
        functions = source.split('# ---- USB video network', 1)[1].split('\n', 1)[1].split('# ---- ntb', 1)[0]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'f-ncm.0').symlink_to('bound-function')
            # Existing function attributes must not be rewritten by udev update:
            # the kernel rejects those writes with EBUSY after link/bind.
            subprocess.run(['sh', '-ec', functions + '\nncm_prepare'], cwd=root,
                           env={'USB_CONFIGS_DIR': directory, 'PATH': '/usr/bin:/bin'}, check=True)
            self.assertFalse((root / 'ifname').exists())

    def test_unbound_prepare_uses_kernel_ifname_pattern(self):
        source = (ROOT / 'external/rkscript/usbdevice').read_text()
        functions = source.split('# ---- USB video network', 1)[1].split('\n', 1)[1].split('# ---- ntb', 1)[0]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'serialnumber').write_text('3bcb9deb719aff6f\n')
            subprocess.run(['sh', '-ec', functions + '\nncm_prepare'], cwd=root, env={
                'USB_CONFIGS_DIR': directory, 'USB_GADGET_STRINGS_DIR': directory,
                'USB_GADGET_DIR': directory, 'PATH': '/usr/bin:/bin'}, check=True)
            self.assertEqual((root / 'ifname').read_text().strip(), 'vrusb%d')
            self.assertEqual((root / 'dev_addr').read_text().strip(), '02:eb:71:9a:ff:6f')
            self.assertEqual((root / 'host_addr').read_text().strip(), '06:eb:71:9a:ff:6f')

    def test_usb_dhcp_does_not_advertise_a_gateway_or_dns(self):
        lines = (ROOT / 'system/usb-video-dnsmasq.conf').read_text().splitlines()
        self.assertIn('interface=vrusb0', lines)
        self.assertIn('port=0', lines)
        self.assertIn('dhcp-option=3', lines)
        self.assertIn('dhcp-option=6', lines)
        self.assertFalse(any(line.startswith(('dhcp-option=3,', 'dhcp-option=6,')) for line in lines))


if __name__ == '__main__':
    unittest.main()
