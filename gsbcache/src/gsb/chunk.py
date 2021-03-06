'''
Created on 29.12.2010

@author: rion
'''

import struct

class Chunk(object):
    TypeAdd = 0
    TypeSub = 1

    def __init__(self, chunkNumber, type, hashLen):
        self._chunkNumber = chunkNumber
        self._type = type
        self._hashLen = hashLen
    
    @property
    def chunkNumber(self):
        '''
        Returns chunk's number
        '''
        return self._chunkNumber
    
    @property
    def type(self):
        '''
        Returns chunk's type (Chunk.TypeAdd or Chunk.TypeSub)
        '''
        return self._type
    
    @property
    def hashLen(self):
        return self._hashLen
        

class ShavarChunk(Chunk):
    '''
    `data` member stores hash {hostKey: prefixes}
    for add chunk prefixes its just plain list of hash prefixes,
    in case of sub chunk prefixes cab be:
    1) chunk number
    2) list<tuple(add chunk number, prefix)>
    '''
    def __init__(self, chunkNumber, type, hashLen, data):
        Chunk.__init__(self, chunkNumber, type, hashLen)
        offset = 0
        hashes = {}
        if type == self.TypeAdd:
            while offset < len(data):
                hostKey, count = struct.unpack_from("IB", data, offset)
                if hostKey not in hashes:
                    hashes[hostKey] = []
                prefixes = hashes[hostKey]
                offset += 5
                for i in xrange(offset, offset + count * hashLen, hashLen):
                    prefixes.append(data[i:i+hashLen])
                offset += count * hashLen
        else: # Sub
            while offset < len(data):
                hostKey, count = struct.unpack_from("IB", data, offset)
                if not count:
                    hashes[hostKey] = struct.unpack_from("!I", data, offset + 5)[0]
                    offset += 9
                    continue
                if hostKey not in hashes:
                    hashes[hostKey] = []
                prefixes = hashes[hostKey]
                offset += 5
                for i in xrange(offset, offset + count * (hashLen + 4), hashLen + 4):
                    prefixes.append((
                        struct.unpack_from("!I", data, i)[0],
                        data[i+4:i+4+hashLen]
                    ))
                offset += count * (hashLen + 4)
                
        self.data = hashes
        
    def removePrefix(self, hostKey, prefix):
        assert self._type == self.TypeAdd, "unimplemented for sub chunks"
        try:
            if prefix == None:
                del self.data[hostKey]
                return
            l = self.data[hostKey]
            l.remove(prefix)
            if not l:
                del self.data[hostKey]
        except:
            pass
        
    def toStream(self):
        '''
        hostKey - 32bit
        prefixesAmount - 8bit
        prefixes - prefixesAmount * hashLen bytes
        '''
        ret = ""
        
        for hostKey, prefixes in self.data.iteritems():
            spStart = 0
            while (True):
                ret += struct.pack("I", hostKey)
                sub = prefixes[spStart : spStart + 255]
                ret += struct.pack("B", len(sub))
                ret += ("".join(sub))
                if not sub:
                    break
                spStart += 255
                
        return ret
        
