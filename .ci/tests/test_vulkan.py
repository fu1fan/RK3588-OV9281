import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import vulkan
from image import install


class VulkanTests(unittest.TestCase):
    def test_bad_download_never_replaces_cached_dependency(self):
        with tempfile.TemporaryDirectory() as directory:
            cache = Path(directory)
            (cache / 'test').write_bytes(b'old corrupt cache')
            def fake_download(*args):
                Path(args[-1]).write_bytes(b'corrupt download')
            sources = {'test': ('https://example.invalid/test', hashlib.sha256(b'expected').hexdigest())}
            with mock.patch.dict(vulkan.SOURCES, sources), mock.patch.object(vulkan, 'run', side_effect=fake_download):
                with self.assertRaisesRegex(RuntimeError, 'checksum mismatch'):
                    vulkan.download('test', cache)
            self.assertEqual((cache / 'test').read_bytes(), b'old corrupt cache')

    def test_valid_cached_dependency_needs_no_network(self):
        with tempfile.TemporaryDirectory() as directory:
            cache = Path(directory)
            (cache / 'test').write_bytes(b'valid')
            sources = {'test': ('https://example.invalid/test', hashlib.sha256(b'valid').hexdigest())}
            with mock.patch.dict(vulkan.SOURCES, sources), mock.patch.object(vulkan, 'run') as run:
                self.assertEqual(vulkan.download('test', cache), cache / 'test')
                run.assert_not_called()

    @unittest.skipUnless(shutil.which('debugfs') and shutil.which('mke2fs'), 'e2fsprogs required')
    def test_final_ext4_readback_detects_corruption_and_missing_icd(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tree = root / 'tree'
            (tree / 'lib/modules').mkdir(parents=True)
            (root / 'modules').mkdir()
            image = root / 'rootfs.ext4'
            with image.open('wb') as stream:
                stream.truncate(16 * 1024 * 1024)
            subprocess.run(['mke2fs', '-q', '-F', '-t', 'ext4', '-d', str(tree), str(image)], check=True)
            src = root / 'mali.json'
            src.write_text('{"ICD":{"library_path":"/usr/lib/libmali-vulkan.so"}}')
            target = '/usr/share/vulkan/icd.d/mali.json'
            overlay = [(src, target, 0o644)]
            fs = install(image, root / 'modules', overlay, root / 'edits')
            vulkan.verify(fs, overlay)
            src.write_text('different content')
            with self.assertRaisesRegex(RuntimeError, 'content mismatch'):
                vulkan.verify(fs, overlay)
            fs.apply(['rm ' + target], root / 'remove')
            with self.assertRaisesRegex(RuntimeError, 'content mismatch'):
                vulkan.verify(fs, overlay)

    def test_bundle_has_root_ownership_modes_and_relative_paths(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            binary = root / 'test'
            binary.write_bytes(b'test')
            bundle = root / 'runtime.tar.gz'
            vulkan.bundle([(binary, '/usr/bin/test', 0o755)], bundle)
            with tarfile.open(bundle) as archive:
                member = archive.getmember('usr/bin/test')
                self.assertEqual((member.uid, member.gid, member.mode), (0, 0, 0o755))
                self.assertEqual(archive.extractfile(member).read(), b'test')
