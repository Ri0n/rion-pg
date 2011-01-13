'''
Created on 11.01.2011

@author: rion
'''
import os
from gsb.config import Config

#from gsb.extra import RangedMap

#class FileMap(RangedMap):
#    pass

class Iterator(object):
    def __init__(self, file):
        self.file = file
    
    def __iter__(self):
        return self
    
    def next(self):
        d = self.file.read()
        if not d:
            raise StopIteration("EOF reached")
        return d

class StructuredFile(object):
    def __init__(self, filename, itemSize, reader, serializer):
        self._fileName = filename
        self._itemSize = itemSize
        self._reader = reader
        self._serializer = serializer
        
        #self._buffer = {}
        
        if os.path.exists(filename):
            self._fp = open(filename, 'r+b')
            self._fileSize = os.path.getsize(filename)
            assert self._fileSize % itemSize == 0, "Input file is corrupted"
        else:
            self._fp = open(filename, 'w+')
            self._fileSize = 0
        self.seek(0)
        
    def __del__(self):
        #self.flush()
        self._fp.close()
        
    def __len__(self):
        return self._fileSize / self._itemSize
        
    def seek(self, n):
        self._seekPos = n * self._itemSize;
        self._fp.seek(n * self._itemSize)
        
    def read(self, n = 1):
        data = self._fp.read(self._itemSize * n)
        ret = []
        for i in xrange(n):
            d = data[i * self._itemSize : (i + 1) * self._itemSize]
            if not d:
                break
            ret.append(self._reader(d))
        return ret
    
    def write(self, obj):
        obj = obj if type(obj) == list else [obj]
        for o in obj:
            self._fp.write(self._serializer(o)[:self._itemSize])
        pos = self._fp.tell()
        if pos > self._fileSize:
            self._fileSize = pos
    
    def __getitem__(self, index):
        self.seek(index)
        return self.read()

    def __setitem__(self, index, value):
        self.seek(index)
        return self.write(value)
    
    def __iter__(self):
        return Iterator(self)
    
    def insert(self, index, data):
        pass # TODO implement =)

class StructuredSortedFile(StructuredFile):
    '''
    This class assumes the data it handles is sorted.
    In other words when there are no instances of this class
    in the memory then all files which are used with this class
    stored on hdd sorted. If stored data is not sorted then probably
    you are going to sort it atm, this happens for example in case
    when you append new data, so you MUST call sort method before you
    leave file on your hdd (TODO: automate).
    
    '''
    def __init__(self, filename, itemSize, reader, serializer, key):
        super(StructuredSortedFile, self).__init__(filename, itemSize, reader, serializer)
        self._key = key
        
    def seek(self, n):
        super(StructuredSortedFile, self).seek(n)
        assert self._seekPos <= self._fileSize
        
#    def write(self, obj):
#        index, exact = self.seekKey(self._key(obj))
#        if exact:
#            return super(StructuredSortedFile, self).write(obj)
#        return super(StructuredSortedFile, self).insert(index, obj)
    
    def seekKey(self, key):
        pass # TODO implement =)
    
    def _merge(self, left, right):
        result = []
        i ,j = 0, 0
        while i < len(left) and j < len(right):
            if left[i] <= right[j]:
                result.append(left[i])
                i += 1
            else:
                result.append(right[j])
                j += 1
    
        result += left[i:]
        result += right[j:]
        return result
    
    def _mergesort(self):
        if len(list) < 2:
            return list
        else:
            middle = len(list) / 2
            left = self.sort(list[:middle])
            right = self.sort(list[middle:])
            return self._merge(left, right)

    def sort(self):
        if len(self) < 2:
            return
        availMem = Config.instance().getint("memory-limit") * 1024 * 1024 # 100 - Mb
        availMem /= self._itemSize # max amount of items in memory 
        if availMem < 3:
            raise MemoryError("insufficient memory for sorting")
        chunksAmount = len(self) / availMem + 1
        chunkSize = len(self) / chunksAmount
        chunks = []
        # sorting data in chunks
        for i in xrange(chunksAmount):
            self.seek(i * chunkSize)
            data = self.read(chunkSize)
            if not data:
                break
            data.sort(key = self._key)
            self.seek(i * chunkSize)
            self.write(data)
            chunks.append(dict(start=i * chunkSize, size=len(data)))
            
        # merging sorted chunks
        
        # size of memory for loading part of one chunk
        memPiece = availMem / (len(chunks) + 1) # 1 for buffer
        # TODO implement further
