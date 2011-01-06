'''
Created on 29.12.2010

@author: rion
'''

import struct

class Chunk(object):
    TypeAdd = 0
    TypeSub = 1

    def __init__(self, index, type):
        self.index = index
        self.type = type
        

class ShavarChunk(Chunk):
    def __init__(self, index, type, hashLen, data):
        Chunk.__init__(self, index, type)
        self.hashLen = hashLen
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
                
        
