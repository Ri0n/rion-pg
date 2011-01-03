'''
Created on 28.12.2010

@author: rion
'''

import time
import datetime
import random
from urllib2 import HTTPError

from gsb.config import Config
from gsb.cache import Cache
from gsb.request import NewKeyRequest, ListRequest, DownloadRequest
from gsb.error import OutOfTriesError, MalformedResponseError

class Client(object):
    
    MaxTries = 3
    
    StatusError = 0
    StatusDone = 1
    
    def __init__(self):
        self._lists = {}

    def isReady(self):
        return datetime.datetime.fromtimestamp(
                    Config.instance().getint("delayed-until")
                ) < datetime.datetime.today()
    
    def _setNeedKey(self, state = True):
        Config.instance().set("new-key-required", state)
        
    def _setDelay(self, delay):
        '''
        Sets delay to not make http requests too often in case of errors
        or in case of google service requests so. 
        '''
        Config.instance().set("delayed-until", int(time.mktime((
            datetime.datetime.today() + datetime.timedelta(minutes = delay)
        ).timetuple())))
        
    def _requestRepeater(self, func):
        result = ""
        for i in xrange(self.MaxTries): # must be at least 2 because of rekey requests
            try:
                status, result = func()
                if status != self.StatusError:
                    Config.instance().set("last-errors-amount", 0)
                    return result
            except HTTPError, e:
                if 300 <= e.code < 600:
                    ea = Config.instance().getint("last-errors-amount")
                    if ea < 5:
                        delay = pow(2, ea - 1) * 30 * (1 + random.random()) if ea else 1
                    else:
                        delay = 480
                    Config.instance().set("last-errors-amount", ea + 1)
                    self._setDelay(delay)
                    raise
                if i < self.MaxTries - 1:
                    time.sleep(5) # 5 sec before next try
                result = str(e)
            except MalformedResponseError, e:
                result = str(e) # lets try again

        raise OutOfTriesError("request didn't finish successfully after 3 tries (%s)" % result)
    
    def getCache(self, listName):
        if listName not in self._lists:
            self._lists[listName] = Cache(listName)
        return self._lists[listName]
       
    def getLists(self):
        def do():
            req = ListRequest()
            lists = req.send()
            if not req.needNewKey():
                return self.StatusDone, lists
            self.updateKey()
            return self.StatusError, "rekey request" # actually not a error, but..
            
        return self._requestRepeater(do)

    
    def downloadList(self, *names):
        def do():
            req = DownloadRequest()
            for n in names:
                cache = self.getCache(n)
                req.addList(n, cache.getAddList(), cache.getSubList())
            result = req.send()
            return self.StatusDone, result
            
        return self._requestRepeater(do)
            
    def updateKey(self):
        def do():
            req = NewKeyRequest()
            keys = req.send()
            return self.StatusDone, keys
            
        self._setNeedKey(True)
        keys = self._requestRepeater(do)
        self.mac = keys["wrappedkey"]
        Config.instance().set("mac-key", self.mac)
        self._setNeedKey(False)
        # here is also `clientkey` in the dict
