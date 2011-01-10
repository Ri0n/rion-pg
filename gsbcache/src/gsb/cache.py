'''
Created on 28.12.2010

@author: rion
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
    