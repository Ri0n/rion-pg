'''
Created on 01.01.2011

@author: rion
'''

import os
import datetime
import ConfigParser

class Config(object):
    '''
    Configuration manager
    '''


    def __init__(self):
        assert len(self._confFile)
        self._parser = ConfigParser.ConfigParser({
            "last-update" : datetime.datetime(1970, 1, 1),
            "new-key-required" : False,
            "delayed-until" : 0,
            "next-delay" : 2
        })
        self._parser.read(self._confFile)
        
    def save(self):
        with open(self._confFile, 'wb') as cf:
            self._parser.write(cf)
        
    @classmethod
    def init(cls, confFile):
        cls._confFile = confFile
        
    @classmethod
    def instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
            def singletonRaiser():
                raise Exception("You have to use %s.instance() to access singleton" % cls.__name__)
            cls.__init__ = singletonRaiser
        return cls._instance
    
    def get(self, option, section = "DEFAULT"):
        return self._parser.get(section, option)
    
    def set(self, option, value, section = "DEFAULT"):
        return self._parser.set(section, option, value)
