'''
Created on 28.12.2010

@author: rion
'''

import time
import datetime
import random
import base64
from urllib2 import HTTPError

from gsb.config import Config
from gsb.cache import Cache
from gsb.request import (
    NewKeyRequest,
    ListRequest,
    DownloadRequest,
    RedirectRequest,
    Request
)
from gsb.error import (
    OutOfTriesError,
    MalformedResponseError,
    UnsupportedListFormat
)
from gsb.chunk import ShavarChunk, Chunk
from gsb.structuredfile import ManagableFile

class Client(object):
    
    MaxTries = 3
    
    StatusError = 0
    StatusDone = 1
    
    def __init__(self):
        self._lists = {}
        c = Config.instance()
        if c.getboolean("use-mac"):
            if c.getboolean("new-key-required") or not c.get("wrappedkey", ""):
                self.updateKey()
            Request.WrappedKey = c.get("wrappedkey")
            Request.ClientKey = base64.urlsafe_b64decode(c.get("clientkey"))
        ManagableFile.setMemoryLimit(c.getint("memory-limit") * 1024 * 1024)

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
            self._lists[listName] = Cache.factory(listName)
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
            dlReq = DownloadRequest()
            for n in names:
                cache = self.getCache(n)
                dlReq.addList(n, cache.getAddList(), cache.getSubList())
            lists = dlReq.send()
            for listName, props in lists.iteritems():
                nameParts = listName.split("-")
                if nameParts[2] != "shavar":
                    raise UnsupportedListFormat(nameParts[2] + "is not supported")
                    
                props["addChunks"] = []
                props["subChunks"] = []
                for url in props["urls"]:
                    rReq = RedirectRequest(url)
                    # on embed devices its better to save request result
                    # into some temporary file
                    chunks = rReq.send()
                    for c in chunks:
                        if c["type"] == 'a':
                            props["addChunks"].append(ShavarChunk(c["cn"], Chunk.TypeAdd,
                                                                  c["hl"], c["data"]))
                        else:
                            props["subChunks"].append(ShavarChunk(c["cn"],Chunk.TypeSub,
                                                                  c["hl"], c["data"]))
            
            # data is fully parsed now, lets put it into cache
            for listName, props in lists.iteritems():
                cache = self.getCache(listName)
                cache.updateAddChunks(props["addChunks"])
                cache.updateSubChunks(props["subChunks"])
                cache.deleteAddChunks(props["adddel"])
                cache.deleteSubChunks(props["subdel"])
            
            return self.StatusDone, None
            
        return self._requestRepeater(do)
            
    def updateKey(self):
        def do():
            req = NewKeyRequest()
            keys = req.send()
            return self.StatusDone, keys
            
        self._setNeedKey(True)
        keys = self._requestRepeater(do)
        Config.instance().set("wrappedkey", keys["wrappedkey"])
        Config.instance().set("clientkey", keys["clientkey"])
        Request.WrappedKey = keys["wrappedkey"]
        Request.ClientKey = base64.urlsafe_b64decode(keys["clientkey"])
        self._setNeedKey(False)
