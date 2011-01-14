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

class MacCheckFailed(GSBError):
    pass

class AlreadyInList(GSBError):
    pass

class AlreadyOpened(GSBError):
    pass

class AlreadyClosed(GSBError):
    pass
