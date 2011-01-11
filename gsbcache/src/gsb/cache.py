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

listname.chunks (random access):
chunkNumber - 32bit
hashLen - 8bit
prefixesAmount - 16bit
dataOffset - 32bit

listname.chunksdata(sequential access):
prefixes - prefixesAmount * hashLen bytes

'''

import os

from gsb.config import ConfigBase, Config

class CacheConfig(ConfigBase):
    def addList(self):
        from gsb.chunk import ChunkNumbersList
        return ChunkNumbersList.fromString(self.get("add-list", ""))
    
    def setAddList(self, l):
        self.set("add-list", l.toString())
        
    def subList(self):
        from gsb.chunk import ChunkNumbersList
        return ChunkNumbersList.fromString(self.get("sub-list", ""))
    
    def setSubList(self, l):
        self.set("sub-list", l.toString())


class Cache(object):
    def __init__(self, listName):
        self._listName = listName
        self._config = CacheConfig(os.path.join(Config.instance().get("storage"), listName + ".ini"))
        self._addNumbers = self._config.addList()
        self._subNumbers = self._config.subList()
        
    def updateChunks(self, chunks):
        from gsb.chunk import Chunk
        for c in chunks:
            if c.type() == Chunk.TypeAdd:
                # TODO update add chunks cache (add new chunks)
                # TODO update black list
                self._addNumbers.append(c.index())
            else: # sub
                # TODO update add chunks cache (change existing chunks)
                # TODO update black list
                self._subNumbers.append(c.index())
        
    def deleteAddChunks(self, chunks):
        for c in chunks:
            # TODO update add chunks cache (delete chunks)
            # TODO update black list
            self._addNumbers.remove(c)
            
    
    def deleteSubChunks(self, chunks):
        for c in chunks:
            self._subNumbers.remove(c)
        
    def clear(self):
        pass
    
    def getAddList(self):
        return self._addNumbers
    
    def getSubList(self):
        return self._subNumbers
    