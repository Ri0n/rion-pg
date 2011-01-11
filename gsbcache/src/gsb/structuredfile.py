'''
Created on 11.01.2011

@author: rion
'''

class StructuredFile(object):
    def __init__(self, filename, itemSize, reader, serializer):
        self._fileName = filename
        self._itemSize = itemSize
        self._reader = reader
        self._serializer = serializer
        self._fp = open(filename, 'r+b')
        self._buffer = {}
        
    def __del__(self):
        self.flush()
        self._fp.close()
        
    def seek(self, n):
        assert self._fp
        self._fp.seek(n * self._itemSize)
        
    def read(self):
        d = self._fp.read(self._itemSize)
        return self._reader(d)
    
    def write(self, obj):
        return self._fp.write(self._serializer(obj))
    
    def __getitem__(self, index):
        self.seek(index)
        return self.read()

    def __setitem__(self, index, value):
        self.seek(index)
        return self.write(value)
    
    def insert(self, index, data):
        pass # TODO implement =)

class StructuredSortedFile(StructuredFile):
    def __init__(self, filename, itemSize, reader, serializer, key):
        super(StructuredSortedFile, self).__init__(filename, itemSize, reader, serializer)
        self._key = key
        
    def write(self, obj):
        index, exact = self.seekKey(self._key(obj))
        if exact:
            return super(StructuredSortedFile, self).write(obj)
        return super(StructuredSortedFile, self).insert(index, obj)
    
    def seekKey(self, key):
        pass # TODO implement =)
