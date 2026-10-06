import struct
import sys
import unittest
import zlib
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from tactics_patch import number,create,apply
class PatchTests(unittest.TestCase):
    def test_integers(self):
        self.assertEqual(number(0),b'\x80');self.assertEqual(number(127),b'\xff');self.assertEqual(number(128),b'\x00\x80')
    def test_roundtrip(self):
        for source,target in [(b'hello',b'hero'),(b'',b'abc'),(b'abcdef',b'ab'),(b'abc',b'abc'*100),(b'abc',b'')]:
            self.assertEqual(apply(source,create(source,target)),target)
    def test_corruption(self):
        patch=bytearray(create(b'abc',b'abcd'));patch[5]^=1
        with self.assertRaises(ValueError):apply(b'abc',patch)
        with self.assertRaises(ValueError):apply(b'bad',create(b'abc',b'abcd'))
    def test_copy_actions(self):
        source=b'abcdef';target=b'cdecdecde'
        body=b'BPS1'+number(6)+number(9)+number(0)+number((2<<2)|2)+number(4)+number((5<<2)|3)+number(0)
        patch=body+struct.pack('<II',zlib.crc32(source),zlib.crc32(target));patch+=struct.pack('<I',zlib.crc32(patch))
        self.assertEqual(apply(source,patch),target)
if __name__=='__main__':unittest.main()
