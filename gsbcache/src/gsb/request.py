'''
Created on 01.01.2011

@author: rion
'''

from httplib import HTTPSConnection

class Request(object):
    '''
    classdocs
    '''


    def __init__(selfparams):
        '''
        Constructor
        '''
        
class NormalRequest(Request):
    pass

class SecureRequest(Request):
    def __init__(self):
        Request.__init__(self, HTTPSConnection())