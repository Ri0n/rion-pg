'''
Created on 14.01.2011

@author: rion
'''
import unittest
import struct
from random import shuffle

from gsb.error import AlreadyOpened, AlreadyClosed


class StructuredFileTest(unittest.TestCase):


    def setUp(self):
        from gsb.structuredfile import StructuredFile
        self.sfile = StructuredFile("gg.txt", 4, lambda s: struct.unpack("I", s)[0], lambda i: struct.pack("I", i))

    def test_open_close(self):
        self.assertRaises(AlreadyOpened, self.sfile.open)
        self.sfile.close()
        self.assertEqual(self.sfile._fp, None)
        self.assertRaises(AlreadyClosed, self.sfile.close)
        self.sfile.open()


    def test_read_write(self):
        self.sfile.truncate()
        self.assertEqual(self.sfile._fileSize, 0)
        self.assertEqual(len(self.sfile), 0)
        self.sfile.write(12345) # + 1 record (4 bytes)
        self.assertEqual(len(self.sfile), 1)
        self.sfile.write(56789)
        self.assertEqual(len(self.sfile), 2)
        self.sfile.seek(1) # move to secod record
        self.assertEqual(self.sfile.read(1), [56789])
        self.assertEqual(self.sfile[0], 12345)
        self.assertEqual(self.sfile[1], 56789)
        self.sfile.seek(0)
        self.assertEqual(self.sfile.read(2), [12345, 56789])
        self.sfile[5] = 13579
        self.sfile.seek(1)
        self.assertEqual(self.sfile[5], 13579)
        self.assertEqual(len(self.sfile), 6)
        self.assertEqual(self.sfile[0:6:5], [12345, 13579])
        
        
class StructuredSortedFileTest(unittest.TestCase):
    def setUp(self):
        from gsb.structuredfile import StructuredSortedFile
        self.sfile = StructuredSortedFile("gg.txt", 4, lambda s: struct.unpack("I", s)[0],
                                          lambda i: struct.pack("I", i),
                                          lambda item: item)
        self.sfile.truncate()
        
    def test_sort(self):
        numbers = range(100, 200)
        shuffle(numbers)
        self.sfile.write(numbers)
        self.sfile.sort(200)
        self.maxDiff = None
        self.assertEqual(self.sfile[0:100], range(100, 200))
        


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    
    unittest.main()