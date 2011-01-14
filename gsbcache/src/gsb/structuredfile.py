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
            self._fp = open(filename, 'r+b') # TODO check for errors
            self._fileSize = os.path.getsize(filename)
            assert self._fileSize % itemSize == 0, "Input file is corrupted"
        else:
            self._fp = open(filename, 'w+') # TODO check for errors
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
        data = ""
        for o in obj:
            data += self._serializer(o)[:self._itemSize]
        self._fp.write(data)
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
            chunks.append(dict(start=i * chunkSize, size=len(data), mergeOffset=0))
            
        # merging sorted chunks
        
        # chunks - all chunks
        # maxWays - maximum number of chunks per one merge
        # chunksGroup - maxWays or less of chunks participating in merge at some instant
        # memPiece - RAM required to store one piece of chunk from chunksGroup (maxWays or less pieces loaded at once)
        
        maxWays = 25
        isSorted = False
        while not isSorted:
            chunksAmount = maxWays if len(chunks) > maxWays else len(chunks)
            passOffset = 0
            chunksGroup = chunks[passOffset:chunksAmount]
            largeChunk = dict(start=chunksGroup[0]["start"], size=len(data), mergeOffset=0)
            # size of memory for loading part of one chunk
            memPiece = availMem / (chunksAmount + 1) # 1 for buffer
            if not memPiece:
                raise MemoryError("insufficient memory for sorting") # actually we can use less chunks..
            
            def loadNextPiece(i):
                '''
                Loads next piece of chunk into memory and saves current chunk offset.
                Return false if not more pieces available
                '''
                if c["mergeOffset"] >= c["size"]:
                    return False
                c = chunksGroup[i]
                size = c["size"] if memPiece > c["size"] else memPiece
                self.seek(c["start"] + c["mergeOffset"])
                c["data"] = self.read(memPiece) # small part of one chunk
                c["mergeOffset"] += size
                return True
                
            for i in xrange(chunksAmount):
                loadNextPiece(i) # no checks here since we are sure the data is available
                largeChunk["size"] += chunksGroup[i]["size"]
                
            sortedFile = self._fileName + ".sorted"
            if os.path.exists(sortedFile):
                os.remove(sortedFile)
            sf = StructuredFile(sortedFile)
            buffer = []
            
            while len(chunksGroup):
                # get chunk from group with smallest key of
                smallestIndex = 0
                for i in xrange(1, len(chunksGroup)):
                    if self._key(chunksGroup[i]["data"][0])<self._key(chunksGroup[smallestIndex]["data"][0]):
                        smallestIndex = i
                c = chunksGroup[smallestIndex]
                buffer.append(c["data"].pop(0))
                
                # flush buffer
                if len(buffer) >= memPiece:
                    sf.write(buffer)
                    buffer = []
                
                # load next piece or remove empty chunk if current piece is fully merged
                if not len(c["data"]):
                    if not loadNextPiece(smallestIndex):
                        chunksGroup.pop(smallestIndex)
                        
            if len(buffer):
                sf.write(buffer)
                buffer = []
                        
            # at this moment chunksGroup fully merged into one large chunk
            # we should exchange this chunksGroup with one chunk and merge next group if available
            chunks[passOffset:passOffset+chunksAmount] = largeChunk
            passOffset += 1
            
            # TODO make checks for next passes
