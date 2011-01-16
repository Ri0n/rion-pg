'''
Created on 28.12.2010

@author: rion

created files:
listname.ini - stores chunk numbers and maybe some other data 
listname.chunks - "add" chunks headers
listname.chunksdata - "add" chunks prefixes
listname.hosts - host keys with hashes reference
listname.hashes - hashes partial or full and their state


lookup process
1) compute host key
2) lookup hostkey in listname.hosts table. return false if not found
3) if hostkey doesn't have prefixes(hashesOffset = -1) then its a match, return true
4) read hashes from listname.hashes by prefix hashesOffset
5) canonicalize full url and lookup prefix. return false if not found
6) if hash is not full hash, request it from google and update listname.hashes table
7) check full hashes for match, return true if found
8) return false

listname.hosts (random access):
hostkey - 32 bit
hashesOffset = 32bit (-1 for no prefixes)

listname.hashes(sequential access):
hashesAmount = 16bit
hashes = prAmount * 9 bytes (hash - 8 bytes, current size - 1 byte)

listname.chunks ("add" chunks, random access):
chunkNumber - 32bit
hashLen - 8bit
flags - 8bit(0 - has data, 1 - , 2 - , 3 - , 4 - , 5 - , 6 -, 7 - )
dataOffset - 32bit

listname.chunksdata(sequential access):
hostKey - 32bit
prefixesAmount - 16bit
prefixes - prefixesAmount * hashLen bytes

'''

import os
import struct

from gsb.config import ConfigBase, Config
from gsb.extra import NumbersList
from gsb.error import UnsupportedListFormat, AlreadyInList
from gsb.structuredfile import StructuredSortedFile, SequentialFile

class CacheConfig(ConfigBase):
    def addList(self):
        return NumbersList.fromString(self.get("add-list", ""))
    
    def setAddList(self, l):
        self.set("add-list", str(l))
        
    def subList(self):
        return NumbersList.fromString(self.get("sub-list", ""))
    
    def setSubList(self, l):
        self.set("sub-list", str(l))


class Cache(object):
    
    @staticmethod
    def factory(listName):
        nameParts = listName.split("-")
        if nameParts[2] == "shavar":
            return ShavarCache(listName)
        raise UnsupportedListFormat(nameParts[2] + "is not supported")
    
    
    def __init__(self, listName):
        self._listName = listName
        self._listPathBase = os.path.join(Config.instance().get("storage"), listName)
        self._config = CacheConfig(self._listPathBase + ".ini")
        self._addNumbers = self._config.addList()
        self._subNumbers = self._config.subList()
        
    def __del__(self):
        self._config.setAddList(self._addNumbers)
        self._config.setSubList(self._subNumbers)
        
    def deleteAddChunks(self, chunks):
        for c in chunks:
            self._addNumbers.remove(c)
    
    def deleteSubChunks(self, chunks):
        for c in chunks:
            self._subNumbers.remove(c, False)
        
    def clear(self):
        self._addNumbers[:] = []
        self._subNumbers[:] = []
    
    def getAddList(self):
        return self._addNumbers
    
    def getSubList(self):
        return self._subNumbers



class ShavarHostRecord(object):
    '''
    hostkey - 32 bit
    hashesOffset = 32bit (-1 for no prefixes)
    '''
    
    format = "Ii"
    
    def __init__(self, s = ""):
        if s:
            self.hostKey, self.hashesOffset = struct.unpack(self.format, s)
        else:
            self.hostKey, self.hashesOffset = (None, None)
            
    def toStream(self):
        return struct.pack(self.format, self.hostKey, self.hashesOffset)

class ShavarChunkRecord(object):
    '''
    chunkNumber - 32bit
    dataOffset - 32bit
    hashLen - 8bit
    flags - 8bit(0 - has data, 1 - , 2 - , 3 - , 4 - , 5 - , 6 -, 7 - )
    '''
    
    format = "=IIBB"
    
    HasDataFlag = 1
    
    def __init__(self, s = ""):
        if s:
            self.chunkNumber, self.dataOffset, self.hashLen, self.flags = \
                struct.unpack(self.format, s)
        else:
            self.chunkNumber = self.hashLen = self.dataOffset = None
            self.flags = 0
            
    def toStream(self):
        return struct.pack(self.format, self.chunkNumber, self.dataOffset,
                           self.hashLen, self.flags)


class ShavarCache(Cache):
    def __init__(self, listName):
        super(ShavarCache, self).__init__(listName)
        self._hostsTable = StructuredSortedFile(self._listPathBase + ".hosts", 8,
                                                lambda s: ShavarHostRecord(s),
                                                lambda i: i.toStream(),
                                                lambda r: r.hostKey)
        self._chunksTable = StructuredSortedFile(self._listPathBase + ".chunks", 10,
                                                lambda s: ShavarChunkRecord(s),
                                                lambda i: i.toStream(),
                                                lambda r: r.chunkNumber)
        self._chunksData = SequentialFile(self._listPathBase + ".chunksdata")
        
    def updateAddChunks(self, chunks):
        self._chunksTable.seek(len(self._chunksTable))
        for c in chunks:
            data = c.toStream()
            offset = self._chunksData.write(data) if data else 0
            record = ShavarChunkRecord()
            record.chunkNumber = c.chunkNumber
            record.hashLen = c.hashLen
            record.flags = ShavarChunkRecord.HasDataFlag if data else 0
            record.dataOffset = offset
            self._chunksTable.write(record)
            # TODO update black list
            self._addNumbers.append(c.chunkNumber)
            
        self._chunksTable.sort()
                
    def updateSubChunks(self, chunks):
        from gsb.chunk import ShavarChunk
        addChunks = {}
        for c in chunks:
            try:
                self._subNumbers.append(c.chunkNumber)
            except AlreadyInList:
                continue # that's strange. server sends duplicates?
            for hostKey, prefixes in c.data.iteritems():
                if type(prefixes) == int:
                    prefixes = [prefixes]
                for p in prefixes:
                    addChunkNumber, prefix = p if isinstance(p, tuple) else (p, None)
                    if self._addNumbers.indexOf(addChunkNumber) == -1:
                        continue
                    addChunkIndex, exact = self._chunksTable.seekKey(addChunkNumber)
                    #print ("" if exact else "not ") + "found at: ", addChunkIndex 
                    if not exact:
                        print "INDEX CORRUPTION DETECTED, RESET REQUIRED.\n%d not found in chunks table" % addChunkNumber
                        continue # add chunk not found. nothing to update
                    
                    chunkRecord = self._chunksTable[addChunkIndex]
                    if not chunkRecord.flags & ShavarChunkRecord.HasDataFlag:
                        continue # add chunk is empty. nothing to update
                    
                    addChunks[addChunkNumber] = addChunks.get(addChunkNumber,
                                                {"index": addChunkIndex, "record": chunkRecord, "hostKeys": {}})
                    if hostKey not in addChunks[addChunkNumber]["hostKeys"]:
                        addChunks[addChunkNumber]["hostKeys"][hostKey] = []
                    addChunks[addChunkNumber]["hostKeys"][hostKey].append(prefix)
            
        removeCandidates = NumbersList()
        for addChunkNumber, chunkInfo in addChunks.iteritems():
            self._chunksData.seek(chunkInfo["record"].dataOffset)
            chunkData = self._chunksData.read()
            addChunk = ShavarChunk(addChunkNumber, ShavarChunk.TypeAdd,
                                   chunkInfo["record"].hashLen, chunkData.data)
            for hostKey, prefixes in chunkInfo["hostKeys"].iteritems():
                for prefix in prefixes:
                    addChunk.removePrefix(hostKey, prefix)

            if not addChunk.data:
                chunkInfo["record"].flags & ~ShavarChunkRecord.HasDataFlag # not necessary actually because of remove below
                self._chunksTable[chunkInfo["index"]]
                chunkData.markInvalid()
                removeCandidates.append(addChunkNumber)
            else:
                chunkData.data = addChunk.toStream()
                chunkData.save()
                
        if removeCandidates:
            self._chunksTable.remove(removeCandidates) # probably not a good idea until adddel..
                
        # TODO update black list
        
    def deleteAddChunks(self, chunks):
        chunks = self._addNumbers.intersect(chunks)
        removeIndexes = NumbersList()
        for chunkNumber in chunks.iterAll():
            recordIndex, exact = self._chunksTable.seekKey(chunkNumber)
            record = self._chunksTable[recordIndex]
            if not exact:
                continue
            self._chunksData.markInvalid(record.offset)
            # TODO call vacuum and invalidate data offsets
            removeIndexes.append(recordIndex)
            # TODO update black list

        self._chunksTable.remove(removeIndexes)
        Cache.deleteAddChunks(self, chunks)
        
    def clear(self):
        self._chunksTable.truncate()
        self._chunksData.truncate()
        Cache.clear(self)
