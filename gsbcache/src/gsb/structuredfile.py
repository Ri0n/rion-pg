'''
Created on 11.01.2011

@author: rion
'''
import os
import struct

from gsb.error import AlreadyOpened, AlreadyClosed
from gsb.extra import NumbersList

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


class ManagableFile(object):
    def __init__(self, filename):
        self._fileName = filename
        self._fp = None
        self._memoryLimit = None # no limits
        
    def open(self):
        if self._fp:
            raise AlreadyOpened("file %s is already opened" % self._fileName)
        if os.path.exists(self._fileName):
            self._fp = open(self._fileName, 'r+b')
            self._fileSize = os.path.getsize(self._fileName)
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
            
    def setMemoryLimit(self, limit):
        '''
        Sets maximum available memory for various memory consumption operations
        like removing/inserting record
        '''
        self._memoryLimit = limit
    
    def read(self, size = -1):
        return self._fp.read(size)
    
    def write(self, data):
        self._fp.write(data)
        pos = self._fp.tell()
        if pos > self._fileSize:
            self._fileSize = pos
            
    def _movePart(self, fromPos, toPos, size = None):
        assert fromPos != toPos
        maxPageSize = self._memoryLimit or 50 * 1024 * 1024
        size = size or self._fileSize - fromPos
        if fromPos < toPos: # move right
            while size:
                btr = maxPageSize if maxPageSize < size else size # bytes to read
                self.seek(fromPos + size - btr)
                data = ManagableFile.read(self, btr * self._itemSize)
                self.seek(toPos + size - btr)
                ManagableFile.write(self, data)
                size -= btr
        else: # move left
            while size:
                btr = maxPageSize if maxPageSize < size else size # bytes to read
                self.seek(fromPos)
                data = ManagableFile.read(self, btr * self._itemSize)
                self.seek(toPos)
                ManagableFile.write(self, data)
                fromPos += btr
                toPos += btr
                size -= btr




class StructuredFile(ManagableFile):
    def __init__(self, filename, itemSize, reader, serializer):
        super(StructuredFile, self).__init__(filename)
        self._itemSize = itemSize
        self._reader = reader
        self._serializer = serializer
        self.open()
        
    def open(self):
        super(StructuredFile, self).open()
        assert self._fileSize % self._itemSize == 0, "Input file is corrupted"
        
    def __len__(self):
        return self._fileSize / self._itemSize
        
    def truncate(self, n = 0):
        # TODO make checks for big n
        self._fileSize = n * self._itemSize
        self._fp.truncate(self._fileSize)
        
    def seek(self, n):
        self._seekPos = n * self._itemSize
        self._fp.seek(n * self._itemSize)
        
    def read(self, n = 1):
        data = super(StructuredFile, self).read(self._itemSize * n)
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
        super(StructuredFile, self).write(data)
    
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
    
    def insert(self, index, data, buffered = True):
        pass # TODO implement =)
    
    def remove(self, index):
        '''
        Removes indexed record or records(in case of NumbersList) and truncates file accordingly
        '''
        if isinstance(index, NumbersList):
            if not index:
                return
            savePoint = index[0][0] if isinstance(index[0], tuple) else index[0]
            for ni in xrange(len(index)):
                rmPos, rmSize = (index[ni][0], index[ni][1] - index[ni][0] + 1) \
                                if isinstance(index[ni], tuple) else (index[ni], 1)
                if rmPos >= len(self):
                    break
                nextDataPos = rmPos + rmSize
                if nextDataPos >= len(self):  # deleting more than tail of file
                    break
                    
                nextRmPos = len(self)
                if ni < len(index) - 1:
                    nextRmPos = index[ni + 1][0] if isinstance(index[ni + 1], tuple) else index[ni + 1]
                    nextRmPos = nextRmPos if nextRmPos < len(self) else len(self)
                sizeToMove = nextRmPos - nextDataPos
                
                self._movePart(nextDataPos, savePoint, sizeToMove)
                savePoint += sizeToMove
            
            self.truncate(savePoint)
        else:
            self._movePart(index+1, index)
            self.truncate(len(self) - 1)
            
    def _movePart(self, fromPos, toPos, size = None):
        assert fromPos != toPos
        ManagableFile._movePart(self, fromPos * self._itemSize, toPos * self._itemSize,
                                size and size  * self._itemSize)
            

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
        '''
        Returns tuple (index of record, exact match)
        if key is found then index will point exact to this record and exact flag = true,
        otherwise index will point to element on left side if any or -1 if no one and
        exact flag will be false.
        '''
        if not len(self):
            return -1, False
        
        # TODO optimize by preloading more data when jump step starts to fit in memoryLimit
        
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
        
    
    def sort(self):
        '''
        sorts file by key 
        '''
        if len(self) < 2:
            return
        availMem = self._memoryLimit / self._itemSize #or Config.instance().getint("memory-limit") * 1024 * 1024 # 100 - Mb
        if not availMem or availMem > len(self):
            data = self[0:len(self)]
            if not data:
                return
            data.sort(key = self._key)
            self.seek(0)
            self.write(data)
            return
            
        #availMem /= self._itemSize # max amount of items in memory 
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


class SequentialBlock(object):
    def __init__(self, seqFile, offset, blockSize, data):
        self._file = seqFile
        self._offset = offset
        self._blockSize = blockSize
        #self._dataSize = len(data)
        self.data = data
    
    @property
    def blockSize(self):
        return self._blockSize
    
    @property
    def offset(self):
        return self._offset
    
    def save(self):
        self._file.write(self)

class SequentialFile(ManagableFile):
    '''
    This class works with file divided into blocks of arbitrary length.
    Though all blocks must be less than memoryLimit in size.
    
    first 4 bytes of each block are size of block allocated first time,
        excluding next 4 bytes, they may be changed only if this is a last block
    next 4 bytes are real data size which change each time when we write
        less data into the same block
    next bytes are block data
    
    So full full size of block including header = block size + 8
    where 8 is size of header
        
    If we are trying to write more data than block size, new block will be
    created
    
    Example (byte array): 4 0 0 0 1 0 0 0 48 1 2 3 1 0 0 0 1 0 0 0 49
        Here we see 2 blocks:
            4 bytes - uint 4   - size of block (amount of followed bytes)
            4 bytes - uint 1   - real size of next data
            1 bytes - uchar 48 - data. byte 48 (ascii "0")
            3 bytes - uchar[]  - junk probably from previous allocations
            4 bytes - uint 1   - next block size
            4 bytes - unit 1   - data size
            1 bytes - uchar 49 - data
    '''
    
    def read(self):
        '''
        reads file block from current position
        '''
        offset = self._fp.tell()
        sizeSata = ManagableFile.read(self, 8)
        if sizeSata:
            blockSize, dataSize = struct.unpack("II", sizeSata)
            if self._memoryLimit and dataSize > self._memoryLimit:
                raise MemoryError("block size too large")
            data = ManagableFile.read(self, sizeSata)
            if len(data) != dataSize:
                raise IOError("block truncated")
            return SequentialBlock(self, offset, blockSize, data)

    def write(self, data):
        if isinstance(data, SequentialBlock):
            if len(data.data) > data.blockSize and data.offset + data.blockSize + 8 < self._fileSize: # no space for overwriting
                self.markInvalid(data.offset)
                return self.write(data.data) # create new block
            else:
                self._fp.seek(data.offset + 4)
                self._fp.write(struct.pack("I", len(data.data)))
                self._fp.write(data.data)
                return data.offset
        else:
            self._fp.seek(self._fileSize)
            dataOffset = self._fileSize
            self._fp.write(struct.pack("II", len(data), len(data)))
            self._fp.write(data)
            return dataOffset

    def markInvalid(self, offset):
        '''
        sets data size to 0, that means invalid block 
        '''
        self._fp.seek(offset + 4)
        self._fp.write('\x00' * 4) # mark block as empty / invalid
        #TODO truncate file if last block
        
    def vacuum(self):
        # TODO implement
        pass
