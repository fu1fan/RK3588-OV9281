import importlib.util
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import build
import publish
import rootfs_services
from image import Ext4, install


GIT_ENV = {'GIT_AUTHOR_NAME': 'CI test', 'GIT_AUTHOR_EMAIL': 'ci@localhost',
           'GIT_COMMITTER_NAME': 'CI test', 'GIT_COMMITTER_EMAIL': 'ci@localhost',
           'GIT_CONFIG_GLOBAL': '/dev/null', 'GIT_CONFIG_SYSTEM': '/dev/null'}


def git(root, *args):
    return subprocess.check_output(['git', *args], cwd=root, text=True,
                                   env={**os.environ, **GIT_ENV}).strip()


class IncrementalSourceTests(unittest.TestCase):
    def test_cache_key_keeps_driver_edits_incremental_but_invalidates_board_config(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            git(root,'init','-q');git(root,'config','user.name','CI test');git(root,'config','user.email','ci@localhost')
            files={'.ci/Dockerfile':'environment', '.ci/build.py':'builder', 'dependencies.json':'{}',
                   'kernel/Makefile':'VERSION=5', 'kernel/drivers/example.c':'first',
                   'device/rockchip/.chips/elf2/elf2_sdkbuild_defconfig':'RK_BUILDROOT=y'}
            for name,text in files.items():
                p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(text)
            git(root,'add','.');git(root,'commit','-qm','baseline')
            first=build.compatibility_key(root,root/'dependencies.json')
            (root/'kernel/drivers/example.c').write_text('second')
            git(root,'add','.');git(root,'commit','-qm','driver edit')
            self.assertEqual(build.compatibility_key(root,root/'dependencies.json'),first)
            (root/'device/rockchip/.chips/elf2/elf2_sdkbuild_defconfig').write_text('RK_BUILDROOT=n')
            git(root,'add','.');git(root,'commit','-qm','board config edit')
            self.assertNotEqual(build.compatibility_key(root,root/'dependencies.json'),first)

    def test_tag_switch_preserves_unchanged_source_and_objects_and_removes_deleted_source(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory); source=root/'source'; source.mkdir(); work=root/'work'
            git(source,'init','-q');git(source,'config','user.name','CI test');git(source,'config','user.email','ci@localhost')
            (source/'unchanged.c').write_text('unchanged\n');(source/'changed.c').write_text('first\n');(source/'deleted.c').write_text('deleted\n')
            git(source,'add','.');git(source,'commit','-qm','first');first=git(source,'rev-parse','HEAD')
            build.sync_source(source,work,first)
            stamp=(work/'unchanged.c').stat().st_mtime_ns
            (work/'cached.o').write_bytes(b'object-cache')
            (source/'changed.c').write_text('second\n');(source/'deleted.c').unlink()
            git(source,'add','-A');git(source,'commit','-qm','second');second=git(source,'rev-parse','HEAD')
            build.sync_source(source,work,second)
            self.assertEqual((work/'unchanged.c').stat().st_mtime_ns,stamp)
            self.assertEqual((work/'cached.o').read_bytes(),b'object-cache')
            self.assertFalse((work/'deleted.c').exists())
            self.assertEqual((work/'changed.c').read_text(),'second\n')
            build.sync_source(source,work,first)
            self.assertEqual((work/'changed.c').read_text(),'first\n')
            self.assertTrue((work/'deleted.c').exists())


class RootfsTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('debugfs') and shutil.which('mke2fs'), 'e2fsprogs required')
    def test_replace_modules_and_preserve_script_permissions_without_mounting(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory); tree=root/'tree'; old=tree/'lib/modules/old/kernel';old.mkdir(parents=True)
            (old/'old.ko').write_text('old-module')
            outside=root/'outside';outside.write_text('must survive')
            (old.parent/'source').symlink_to(outside)
            (tree/'etc/init.d').mkdir(parents=True)
            image=root/'rootfs.ext4'
            with image.open('wb') as f:f.truncate(32*1024*1024)
            subprocess.run(['mke2fs','-q','-F','-t','ext4','-d',str(tree),str(image)],check=True)
            modules=root/'modules/new/kernel';modules.mkdir(parents=True)
            (modules/'new.ko').write_text('new-module')
            script=root/'S01test';script.write_text('#!/bin/sh\nexit 0\n');script.chmod(0o755)
            fs=install(image,root/'modules',[(script,'/etc/init.d/S01test')],root/'edits')
            self.assertFalse(fs.exists('/lib/modules/old'))
            self.assertEqual(fs.run('cat /lib/modules/new/kernel/new.ko').strip(),'new-module')
            self.assertEqual(outside.read_text(),'must survive')
            metadata=fs.run('stat /etc/init.d/S01test')
            self.assertRegex(metadata,r'Mode:\s+0755')
            self.assertRegex(metadata,r'User:\s+0\s+Group:\s+0')


class RootfsServiceTests(unittest.TestCase):
    """The three system fixes have to reach the final rootfs, not just the tree."""

    def test_watchdog_feeder_is_scheduled_before_wifi_and_network_bringup(self):
        self.assertLess(rootfs_services.WATCHDOG_ORDER, rootfs_services.WIFI_ORDER)
        names = sorted(['S01cgroup', 'S80dnsmasq', 'S98Bluez_init',
                        rootfs_services.WATCHDOG_SCRIPT.rsplit('/', 1)[1],
                        rootfs_services.WIFI_SCRIPT.rsplit('/', 1)[1]])
        self.assertEqual(names[0], 'S01cgroup')
        self.assertLess(names.index('S01watchdog'), names.index('S36wifistation'))
        self.assertLess(names.index('S36wifistation'), names.index('S80dnsmasq'))

    def test_shipped_sta_template_has_no_credentials(self):
        rootfs_services.check_no_credentials(rootfs_services.SYSTEM/'wifistation.conf.example')

    def test_a_filled_in_template_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            template = Path(directory)/'wifistation.conf.example'
            template.write_text('SSID=home\nPSK=secret123\n')
            with self.assertRaises(RuntimeError):
                rootfs_services.check_no_credentials(template)

    def test_wifi_script_does_not_carry_camera_workarounds(self):
        rootfs_services.check_decoupled(rootfs_services.SYSTEM/'init.d'/'S36wifistation')
        with tempfile.TemporaryDirectory() as directory:
            bad = Path(directory)/'S36wifistation'
            bad.write_text('#!/bin/sh\necho 0 > /sys/devices/x/is_use_dummybuf\n')
            with self.assertRaises(RuntimeError):
                rootfs_services.check_decoupled(bad)

    def test_init_scripts_are_valid_posix_shell(self):
        rootfs_services.check_script(rootfs_services.SYSTEM/'init.d'/'S01watchdog')
        rootfs_services.check_script(rootfs_services.SYSTEM/'init.d'/'S36wifistation')

    def test_non_aarch64_binaries_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            fake = Path(directory)/'elf'
            # e_ident[16] + e_type[2] + e_machine[2]
            header = b'\x7fELF\x02\x01\x01' + b'\x00' * 9 + b'\x02\x00'
            fake.write_bytes(header + b'\xb7\x00')
            rootfs_services.check_elf_aarch64(fake)
            fake.write_bytes(header + b'\x3e\x00')
            with self.assertRaises(RuntimeError):
                rootfs_services.check_elf_aarch64(fake)

    @unittest.skipUnless(shutil.which('debugfs') and shutil.which('mke2fs'),
                         'e2fsprogs required')
    def test_services_land_in_the_image_with_expected_modes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tree = root/'tree'
            (tree/'etc/init.d').mkdir(parents=True)
            (tree/'lib').mkdir()
            (tree/'dev').mkdir()
            (tree/'var/run').mkdir(parents=True)
            image = root/'rootfs.ext4'
            with image.open('wb') as handle:
                handle.truncate(16*1024*1024)
            subprocess.run(['mke2fs','-q','-F','-t','ext4','-d',str(tree),str(image)],check=True)
            binary = root/'elf2-wdctl'
            binary.write_bytes(b'not really an elf, image content only')
            modules = root/'modules'
            modules.mkdir()
            overlay = [(binary, rootfs_services.WATCHDOG_BINARY, 0o755),
                       (rootfs_services.SYSTEM/'init.d'/'S01watchdog', rootfs_services.WATCHDOG_SCRIPT, 0o755),
                       (rootfs_services.SYSTEM/'init.d'/'S36wifistation', rootfs_services.WIFI_SCRIPT, 0o755),
                       (rootfs_services.SYSTEM/'wifistation.conf.example', rootfs_services.WIFI_TEMPLATE, 0o644)]
            fs = install(image, modules, overlay, root/'edits')
            rootfs_services.verify(fs)
            self.assertRegex(fs.run('stat '+rootfs_services.WIFI_TEMPLATE), r'Mode:\s+(?:0?100644|0644)')
            self.assertIn('elf2-wdctl', fs.run('cat '+rootfs_services.WATCHDOG_SCRIPT))

    def test_kernel_requirements_cover_watchdog_and_sta_driver(self):
        script = (Path(__file__).resolve().parents[1]/'build.py').read_text()
        for symbol in ('CONFIG_WATCHDOG=y', 'CONFIG_DW_WATCHDOG=y', 'CONFIG_IWLWIFI=m',
                       'CONFIG_VIDEO_OV9281=y'):
            self.assertIn(repr(symbol), script)



if __name__=='__main__':unittest.main()


class PublicationTests(unittest.TestCase):
    def test_cnb_media_type_and_token_are_restricted_to_api_host(self):
        with mock.patch.dict(os.environ, {'CNB_TOKEN':'test-only', 'CNB_REPO_SLUG':'owner/repo'}):
            api=publish.API()
        response=mock.MagicMock()
        response.__enter__.return_value.read.return_value=b'[]'
        with mock.patch('urllib.request.urlopen',return_value=response) as send:
            self.assertEqual(api.request('GET','/git/commit-assets/example'),[])
            request=send.call_args.args[0]
            self.assertEqual(request.get_header('Accept'),'application/vnd.cnb.api+json')
            self.assertEqual(request.get_header('Authorization'),'Bearer test-only')
            with self.assertRaises(RuntimeError):api.request('POST','https://another.example/verify')
            self.assertEqual(send.call_count,1)
