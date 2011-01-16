'''
Created on 14.01.2011

@author: rion
'''
import unittest
import struct
import os
from random import shuffle

import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from gsb.error import AlreadyOpened, AlreadyClosed
from gsb.extra import NumbersList


class StructuredFileTest(unittest.TestCase):


    def setUp(self):
        from gsb.structuredfile import StructuredFile
        os.remove("test.data")
        self.sfile = StructuredFile("test.data", 4, lambda s: struct.unpack("I", s)[0], lambda i: struct.pack("I", i))

    def test_open_close(self):
        self.assertRaises(AlreadyOpened, self.sfile.open)
        self.sfile.close()
        self.assertEqual(self.sfile._fp, None)
        self.assertRaises(AlreadyClosed, self.sfile.close)
        self.sfile.open()


    def test_read_write(self):
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
        
    def test_remove_records(self):
        self.sfile.write(range(0, 100))
        self.assertEqual(len(self.sfile), 100)
        self.assertEqual(self.sfile[0:100], range(100))
        self.sfile.remove(50)
        testRange = range(0, 100)
        testRange.pop(50)
        self.assertEqual(self.sfile[0:len(self.sfile)], testRange)
        
        self.sfile.truncate()
        self.sfile.seek(0)
        self.sfile.write(range(0, 20))
        removeIndexes = NumbersList()
        removeIndexes.append((0,5))
        removeIndexes.append(8)
        removeIndexes.append(12)
        removeIndexes.append((15,30))
        self.sfile.remove(removeIndexes)
        self.assertEqual(self.sfile[0:len(self.sfile)], [6,7,9,10,11,13,14])
        
        
class StructuredSortedFileTest(unittest.TestCase):
    def setUp(self):
        from gsb.structuredfile import StructuredSortedFile
        os.remove("test.data")
        self.sfile = StructuredSortedFile("test.data", 4, lambda s: struct.unpack("I", s)[0],
                                          lambda i: struct.pack("I", i),
                                          lambda item: item)
        
    def test_sort(self):
        start = 105
        finish = 5220
        numbers = range(start, finish)
        shuffle(numbers)
        self.sfile.write(numbers)
        self.sfile.setMemoryLimit(2000)
        self.sfile.sort()
        self.maxDiff = None
        self.assertEqual(self.sfile[0:finish - start], range(start, finish))
        
    def test_lookup(self):
        start = 105
        finish = 5220
        keyIn = 5111
        keyLeft = 99
        keyRight = 9234
        numbers = range(start, finish)
        self.sfile.write(numbers)
        index, exact = self.sfile.seekKey(keyIn)
        self.assertTrue(exact)
        self.assertEqual(index, keyIn - start)
        index, exact = self.sfile.seekKey(keyLeft)
        self.assertTrue(not exact)
        self.assertEqual(index, -1)
        index, exact = self.sfile.seekKey(keyRight)
        self.assertTrue(not exact)
        self.assertEqual(index, finish - start - 1)



class SequentialFileTest(unittest.TestCase):
    def setUp(self):
        from gsb.structuredfile import SequentialFile
        os.remove("test.data")
        self.sfile = SequentialFile("test.data")
    
    def test_read_write(self):
        str1 = "hello world"
        str2 = "i like cookies"
        str3 = "cutted"
        str4 = "very very long string"
        self.sfile.write(str1)
        self.assertEqual(len(self.sfile), len(str1)+8)
        firstBlock = self.sfile.readFrom(0)
        self.assertEqual(firstBlock.data, str1)
        self.sfile.write(str2)
        self.assertEqual(len(self.sfile), len(str1)+len(str2)+16)
        self.assertEqual(self.sfile.readFrom(len(str1) + 8).data, str2)
        self.assertEqual(len(self.sfile), len(str1)+len(str2)+16)
        self.sfile.seek(0)
        firstBlock.data = str3
        firstBlock.save()
        self.assertEqual(self.sfile.readFrom(0).data, str3)
        self.assertEqual(len(self.sfile), len(str1)+len(str2)+16)
        firstBlock.data = str4
        offset = firstBlock.save()
        self.assertEqual(offset, len(str1)+len(str2)+16)
        self.assertEqual(self.sfile.readFrom(len(str1)+len(str2)+16).data, str4)
        
    def test_vacuum(self):
        self.sfile.write("abc") # 0
        self.sfile.write("def") # 11
        self.sfile.write("ghi") # 22
        self.sfile.write("jkl") # 33
        self.sfile.write("mno") # 44
        self.sfile.write("pqr") # 55
        self.sfile.markInvalid(11)
        self.sfile.markInvalid(22)
        self.assertEqual(len(self.sfile), 66)
        self.sfile.markInvalid(55)
        self.assertEqual(len(self.sfile), 55)
        self.sfile.vacuum()
        self.assertEqual(len(self.sfile), 33)
        self.assertEqual(self.sfile.readFrom(0).data, "abc")
        self.assertEqual(self.sfile.readFrom(11).data, "jkl")
        self.assertEqual(self.sfile.readFrom(22).data, "mno")
        

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    
    unittest.main()