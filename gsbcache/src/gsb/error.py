'''
Created on 30.12.2010

@author: rion
'''

class GSBError(Exception):
    pass

class MalformedResponseError(GSBError):
    pass

class OutOfTriesError(GSBError):
    pass

class UnsupportedListFormat(GSBError):
    pass