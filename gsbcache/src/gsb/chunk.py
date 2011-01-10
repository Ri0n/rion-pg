'''
Created on 29.12.2010

@author: rion
'''

import struct

from gsb.error import AlreadyInList

class ChunkNumbersList(list):
    def append(self, num, strict = True):
        
        def glueLeft(index):
            if index:
                if isinstance(self[index], tuple):
                    if isinstance(self[index-1], tuple):
                        if self[index-1][1] == self[index][0] - 1:
                            self[index-1:index+1] = [(self[index-1][0], self[index][1])]
                    else:
                        if self[index-1] == self[index][0] - 1:
                            self[index-1:index+1] = [(self[index-1], self[index][1])]
                else:
                    if isinstance(self[index-1], tuple):
                        if self[index-1][1] == self[index] - 1:
                            self[index-1:index+1] = [(self[index-1][0], self[index])]
                    else:
                        if self[index-1] == self[index] - 1:
                            self[index-1:index+1] = [(self[index-1], self[index])]
        
        def glueRight(index):
            if index < len(self) - 1:
                glueLeft(index + 1)
        
        if isinstance(num, tuple):
            left = num[0]
            right = num[1]
            indexLeft, exactLeft = self._bisectLeft(left)
            indexRight, exactRight = self._bisectLeft(right)
            if strict and (exactLeft or exactRight or indexLeft != indexRight):
                raise AlreadyInList("%s intersects existing elements" % num)
            #if indexRight - indexLeft > 1:
            #    self[indexLeft+1:indexRight] = []
            if exactLeft:
                if exactRight:
                    if indexLeft == indexRight:
                        return
                    repl = ((self[indexLeft][0] if isinstance(self[indexLeft], tuple)
                                        else self[indexLeft]),
                            (self[indexRight][1] if isinstance(self[indexRight], tuple)
                                        else self[indexRight]))
                    self[indexLeft:indexRight+1] = [repl]
                else:
                    repl = ((self[indexLeft][0] if isinstance(self[indexLeft], tuple)
                                        else self[indexLeft]),
                            right)
                    self[indexLeft:indexRight+1] = [repl]
                    glueRight(indexLeft)
            else:
                if exactRight:
                    self[indexLeft+1:indexRight+1] = [(left,
                            (self[indexRight][1] if isinstance(self[indexRight], tuple)
                                                else self[indexRight]))]
                    glueLeft(indexLeft+1)
                else:
                    self[indexLeft+1:indexRight+1] = [(left, right)]
                    glueRight(indexLeft+1)
                    glueLeft(indexLeft+1)
                    
        else: # not a tuple
            index, exact = self._bisectLeft(num)
            if exact:
                if strict:
                    raise AlreadyInList("%s already exists in the list" % num)
                else:
                    return
                
            self[index+1:index+1] = [num]
            glueRight(index+1)
            glueLeft(index+1)

    
    def remove(self, num, strict = True):
        if isinstance(num, tuple):
            indexLeft, exactLeft = self._bisectLeft(num[0])
            indexRight, exactRight = self._bisectLeft(num[1])
            if strict and (not exactLeft or not exactRight or 
                           indexLeft != indexRight or indexLeft == -1):
                raise ValueError("can't remove not existing elements")
            left = []
            right = []
            if exactLeft and num[0] != self[indexLeft][0]:
                left = [(self[indexLeft][0], num[0] - 1) if self[indexLeft][0] < num[0] - 1 else num[0] - 1]
            if exactRight and num[1] != self[indexRight][1]:
                right = [(num[1] + 1, self[indexRight][1]) if num[1] + 1  < self[indexRight][1] else num[1] + 1]
            repl = left + right
            self[indexLeft if exactLeft else indexLeft+1 : indexRight+1] = repl
            
        else:
            index, exact = self._bisectLeft(num)
            if not exact:
                if strict:
                    raise ValueError("%d is in the list" % num)
                else:
                    return
            repl = []
            if isinstance(self[index], tuple):
                left = []
                right = []
                if num != self[index][0]:
                    left = [(self[index][0], num - 1) if self[index][0] < num - 1 else num - 1]
                if num != self[index][1]:
                    right = [(num + 1, self[index][1]) if num + 1  < self[index][1] else num + 1]
                repl = left + right
                
            self[index:index+1] = repl
    
    def indexOf(self, num):
        index, exact = self._bisectLeft(num)
        return index if exact else -1
        
    def _bisectLeft(self, num):
        if not self:
            return 0, False
        start = 0
        end = len(self)
        while start != end:
            mid = int((start + end)/2)
            if isinstance(self[mid], tuple):
                if num < self[mid][0]:
                    if mid == end:
                        return mid, False
                    end = mid
                    continue
                if num > self[mid][1]:
                    if mid == start:
                        return mid, False
                    start = mid
                    continue
                return mid, True
            else:
                if num < self[mid]:
                    if mid == end:
                        return mid, False
                    end = mid
                    continue
                if num > self[mid]:
                    if mid == start:
                        return mid, False
                    start = mid
                    continue
                return mid, True
        
        if start == end == 0:
            return -1, False
        return start, False
        
    @classmethod
    def fromString(cls, s):
        def sc2c(c):
            l = [int(e) for e in c.split('-')]
            return (l[0], l[1]) if len(l) > 1 and l[0] < l[1] else l[0]
        s = s.strip()
        l = [sc2c(c) for c in s.split(",")] if s else []
        l.sort(key=lambda r: r[0] if isinstance(r, tuple) else r)
        
        ret = cls()
        ret[:] = l[:1]
        if len(l) < 2:
            return ret
        del l[:1]
        while len(l):
            cur = l.pop(0)
            if isinstance(ret[-1], tuple):
                if isinstance(cur, tuple):
                    if ret[-1][1] >= cur[0]:
                        ret[-1] = (ret[-1][0], cur[1])
                        continue
                else:
                    if cur <= ret[-1][1]:
                        continue
            else:
                if isinstance(cur, tuple):
                    if ret[-1] == cur[0]: # ret[-1] can't greater because of sorting, so == is enough
                        ret[-1] = cur
                        continue
                else:
                    if ret[-1] == cur:
                        continue
            ret += [cur] # a bit strange way to not used overridden append =)
        return ret
        
    
    def toString(self):
        return ",".join([str(c[0]) + "-" + str(c[1]) \
                if isinstance(c, tuple) else str(c) for c in self])




class Chunk(object):
    TypeAdd = 0
    TypeSub = 1

    def __init__(self, index, type):
        self._index = index
        self._type = type
    
    def index(self):
        '''
        Returns chunk's number
        '''
        return self._index
    
    def type(self):
        '''
        Returns chunk's type (Chunk.TypeAdd or Chunk.TypeSub)
        '''
        return self._type
        

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
                
        
