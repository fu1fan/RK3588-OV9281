import importlib.util
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import build
from image import Ext4, install


def git(root, *args):
    return subprocess.check_output(['git', *args], cwd=root, stderr=subprocess.DEVNULL, text=True).strip()


class IncrementalSourceTests(unittest.TestCase):
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


if __name__=='__main__':unittest.main()
