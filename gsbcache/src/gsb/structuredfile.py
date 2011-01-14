'''
Created on 11.01.2011

@author: rion
'''
import os
from gsb.config import Config
from gsb.error import AlreadyOpened, AlreadyClosed

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
        self._fp = None
        self.open()
        
    def open(self):
        if self._fp:
            raise AlreadyOpened("file %s is already opened" % self._fileName)
        if os.path.exists(self._fileName):
            self._fp = open(self._fileName, 'r+b')
            self._fileSize = os.path.getsize(self._fileName)
            assert self._fileSize % self._itemSize == 0, "Input file is corrupted"
        else:
            self._fp = open(self._fileName, 'w+')
            self._fileSize = 0
        self.seek(0)
        
    def close(self):
        if not self._fp:
            raise AlreadyClosed("file %s is not opened" % self._fileName)
        self._fp.close()
        self._fp = None
        
    def __del__(self):
        #self.flush()
        if self._fp:
            self.close()
        
    def __len__(self):
        return self._fileSize / self._itemSize
        
    def truncate(self, n = 0):
        self._fileSize = n * self._itemSize
        self._fp.truncate(self._fileSize)
        
    def seek(self, n):
        self._seekPos = n * self._itemSize
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
    
    def __getitem__(self, item):
        if isinstance(item, slice):
            indices = item.indices(len(self))
            self.seek(indices[0])
            n = indices[1] - indices[0]
            return self.read(n)[0:n:indices[2]]

        self.seek(item)
        return self.read()[0]

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
    
    def seekKey(self, key, memoryLimit = None):
        '''
        Returns tuple (index of record, exact match)
        if key is found then index will point exact to this record and exact flag = true,
        otherwise index will point to element on left side if any or -1 if no one and
        exact flag will be false.
        '''
        if not len(self):
            return 0, False
        
        # TODO optimize by preloading more data when jump step starts to fit in memoryLimit
        #availMem = memoryLimit or Config.instance().getint("memory-limit") * 1024 * 1024 # 100 - Mb
        #availMem /= self._itemSize
        
        start = 0
        end = len(self)
        while start != end:
            mid = int((start + end)/2)
            if key < self._key(self[mid]):
                if mid == end:
                    return mid, False
                end = mid
                continue
            if key > self._key(self[mid]):
                if mid == start:
                    return mid, False
                start = mid
                continue
            return mid, True
        
        if start == end == 0:
            return -1, False
        return start, False
        
    
    def sort(self, memoryLimit = None):
        '''
        sorts file by key 
        '''
        if len(self) < 2:
            return
        availMem = memoryLimit or Config.instance().getint("memory-limit") * 1024 * 1024 # 100 - Mb
        availMem /= self._itemSize # max amount of items in memory 
        if availMem < 3:
            raise MemoryError("insufficient memory for sorting")
        allChunksAmount = len(self) / availMem + (len(self) % availMem and 1)
        #chunkSize = len(self) / allChunksAmount
        allChunks = []
        # sorting data in chunks
        for i in xrange(allChunksAmount):
            self.seek(i * availMem)
            data = self.read(availMem)
            if not data:
                break
            data.sort(key = self._key)
            self.seek(i * availMem)
            self.write(data)
            allChunks.append(dict(start=i * availMem, size=len(data), mergeOffset=0))
            
        # merging sorted chunks
        
        # allChunks - all chunks
        # maxWays - maximum number of chunks per one merge
        # chunksGroup - maxWays or less of chunks participating in merge at some instant
        # chunkBufferSize - RAM required to store one piece of chunk from chunksGroup (maxWays or less pieces loaded at once)
        # chunksGroupSize
        
        def loadNextPiece(i):
            '''
            Loads next piece of chunk into memory and saves current chunk offset.
            Return false if not more pieces available
            '''
            c = chunksGroup[i]
            if c["mergeOffset"] >= c["size"]:
                return False
            size = c["size"] - c["mergeOffset"] if c["mergeOffset"] + chunkBufferSize > c["size"] \
                                                else chunkBufferSize
            self.seek(c["start"] + c["mergeOffset"])
            c["data"] = self.read(size) # small part of one chunk
            c["mergeOffset"] += size
            return True
        
        def bufferedOut(r = None):
            '''
            Puts record into buffer and flashes it bound is reached.
            if `r` == None only buffer will be flashed unconditionally 
            '''
            if r:
                outBuffer.append(r)
            # flush buffer
            if len(outBuffer) >= chunkBufferSize or (r == None and len(outBuffer)):
                self._sortedOutFile.write(outBuffer)
                outBuffer[:] = [] # clear buffer
        
        maxWays = 25
        outBuffer = []
        while len(allChunks) > 1: # while not fully sorted
            chunksGroupSize = maxWays if len(allChunks) > maxWays else len(allChunks)
            allChunksOffset = 0
            chunksGroup = allChunks[allChunksOffset:chunksGroupSize]
            largeChunk = dict(start=chunksGroup[0]["start"], size=len(data), mergeOffset=0)
            # size of memory for loading part of one chunk
            chunkBufferSize = availMem / (chunksGroupSize + 1) # 1 for buffer
            if not chunkBufferSize:
                raise MemoryError("insufficient memory for sorting") # actually we can use less chunks..
            
            # preload first chunk pieces.
            for i in xrange(chunksGroupSize):
                loadNextPiece(i) # no checks here since we are sure the data is available
                largeChunk["size"] += chunksGroup[i]["size"] # compute size of future merged chunk
                
            sortedFileName = self._fileName + ".sorted"
            if os.path.exists(sortedFileName):
                os.remove(sortedFileName)
            self._sortedOutFile = StructuredFile(sortedFileName, self._itemSize, self._reader, self._serializer)
            
            while len(chunksGroup):
                # get chunk from group with smallest key of
                smallestIndex = 0
                for i in xrange(1, len(chunksGroup)):
                    if self._key(chunksGroup[i]["data"][0])<self._key(chunksGroup[smallestIndex]["data"][0]):
                        smallestIndex = i
                c = chunksGroup[smallestIndex]
                bufferedOut(c["data"].pop(0))
                
                # load next piece or remove empty chunk if current piece is fully merged
                if not len(c["data"]) and not loadNextPiece(smallestIndex):
                    chunksGroup.pop(smallestIndex)
                        
            bufferedOut(None) # flush
                        
            # at this moment chunksGroup fully merged into one large chunk
            # we should exchange this chunksGroup with one chunk and merge next group if available
            allChunks[allChunksOffset:allChunksOffset+chunksGroupSize] = [largeChunk]
            allChunksOffset += 1
            if allChunksOffset >= len(allChunks):
                allChunksOffset = 0
                del self._sortedOutFile # flush and close
                self.close()
                os.remove(self._fileName)
                os.rename(sortedFileName, self._fileName)
                self.open()
                
