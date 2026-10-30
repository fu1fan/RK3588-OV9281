import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from export_sdk import relativize

class Links(unittest.TestCase):
    def test_absolute_target_remains_inside_sysroot_after_relocation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'root'
            (root / 'usr/lib').mkdir(parents=True)
            (root / 'lib').mkdir()
            (root / 'lib/libtest.so.1').write_text('library')
            (root / 'usr/lib/libtest.so').symlink_to('/lib/libtest.so.1')
            relativize(root)
            moved = Path(tmp) / 'moved'
            root.rename(moved)
            self.assertEqual((moved / 'usr/lib/libtest.so').read_text(), 'library')
