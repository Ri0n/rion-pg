'''
Created on 01.01.2011

@author: rion
'''

import ConfigParser

class NoValue:
    pass

class Config(object):
    '''
    Configuration manager
    '''


    def __init__(self):
        assert len(self._confFile)
        self._parser = ConfigParser.ConfigParser({
            "new-key-required" : False,
            "use-mac" : False,
            "last-errors-amount" : 0,
            "delayed-until" : 0
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
    
    def get(self, option, defaultValue = NoValue(), method = None):
        try:
            return getattr(self._parser, method)("DEFAULT", option) \
                if method else self._parser.get("DEFAULT", option)
        except:
            if isinstance(defaultValue, NoValue):
                raise
            return defaultValue
        
    def getint(self, option, defaultValue = NoValue()):
        return self.get(option, defaultValue, "getint")
    
    def getboolean(self, option, defaultValue = NoValue()):
        return self.get(option, defaultValue, "getboolean")
    
    def set(self, option, value):
        return self._parser.set("DEFAULT", option, value)
