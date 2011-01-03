'''
Created on 01.01.2011

@author: rion

All request classes just try to make it a bit more easy
to get data from the gsb service. They don't validate response
data for full match to the API, so sometimes its possible
that the response contains some trailing data which will stay
unnoticed or for example some pieces of invalid data can be
understood as valid. Full validation requires time from the
programmer as well as from the program, so lets consider it's 
ok w/o additional manipulations =)  
'''

import urlparse
import urllib2
from gsb.config import Config
from gsb.error import MalformedResponseError
import datetime

class Request(object):
    '''
    Base class for all requests
    '''
    
    ClientName = "Firefox"
    ClientVersion = "3.6.13"
    APIVersion = "2.2"
    
    #ClientName = "navclient-auto-ffox"
    #ClientVersion = "3.6.9"
    #APIVersion = "2.2"
    
    def __init__(self, url):
        # lets it will be here for a while
        standardParams = "client="+self.ClientName+"&appver=" + self.ClientVersion + \
                        "&pver="+self.APIVersion
        self._useMac = Config.instance().getboolean("use-mac")
        if self._useMac:
            standardParams += ("&wrkey=" + Config.instance().get("mac-key"))
        
        parts = urlparse.urlsplit(url, "http")
        query = parts.query + ("&" + standardParams if parts.query else standardParams)
        parts = list(parts)
        parts[3] = query
        self._conn = urllib2.Request(urlparse.urlunsplit(parts))
            
    def send(self, body = ""):
        print("send: %s" % body)
        self._conn.add_data(body)
        f = urllib2.urlopen(self._conn)
        data = f.read()
        print("read: %s" % data)
        return data




class NormalRequest(Request):
    '''
    Normal client requests
    '''
    
    Rekey = "e:pleaserekey"
    
    def __init__(self, uri = ""):
        self._needNewKey = False
        Request.__init__(self, "http://safebrowsing.clients.google.com/safebrowsing/" + uri)
        
    def needNewKey(self):
        return self._needNewKey
    
    def validateMac(self, mac):
        return # TODO validate it. compare with generated hmac from client key





class ListRequest(NormalRequest):
    '''
    Returns available lists
    '''
    def __init__(self):
        NormalRequest.__init__(self, "list")
        
    def send(self):
        data = NormalRequest.send(self)
        lines = data.strip().split("\n")[1:]
        if self._useMac:
            if data.startswith(self.Rekey):
                self._needNewKey = True
                return
            self.validateMac(lines[0])
            return lines[1:]
        return lines





class DownloadRequest(NormalRequest):
    '''
    Returns list updates
    '''
    
    def __init__(self):
        self._lists = []
        self._size = 0
        self._needReset = False
        self._nextDelay = datetime.timedelta()
        NormalRequest.__init__(self, "downloads")
        
    def setSize(self, size):
        self._size = size
        
    def addList(self, name, aList = [], sList = []):
        self._lists.append((name, aList, sList))
        
    def send(self):
        def chunksToStr(l):
            return ",".join([str(c[0]) + "-" + str(c[1]) \
                      if isinstance(c, tuple) else str(c) for c in l])
            
        def chunksFromStr(s):
            def sc2c(c):
                l = c.split('-')
                return int(l[0]), int(l[1]) if len(l) > 1 else int(l[0])
            return [sc2c(c) for c in s.split(",")]
            
        body = []
        if self._size:
            body.append("s;" + self._size)
        
        for l in self._lists:
            line = []
            aLine = chunksToStr(l[1])
            if aLine: line += ["a", aLine]
            sLine = chunksToStr(l[2])
            if sLine: line += ["s", sLine]
            body.append(l[0] + ";" + (":".join(line)))
        
        data = NormalRequest.send(self, ("\n".join(body)) + "\n")
        
        # Parse response
        lines = data.strip().split("\n")
        if self._useMac:
            l = lines.pop(0)
            if l == self.Rekey:
                self._needNewKey = True
            else:
                self.validateMac(l)
        next = lines.pop(0)
        if not next.startswith("n:"):
            raise MalformedResponseError('NEXT expected')
        self._nextDelay = datetime.timedelta(seconds = int(next[2:]))
        
        if lines[0].startswith("r:pleasereset"):
            self._needReset = True
            return # invalid trailing data is ignored if exists
        
        self._responseLists = {}
        while (True):
            if not len(lines):
                break
            key, listName = lines.pop(0).split(":")
            if key ==  "i":
                self._responseLists[listName] = {"urls": [], "adddel": [], "subdel": []}
                while len(lines) and not lines[0].startswith("i:"):
                    l = lines.pop(0)
                    if not l: break # next list
                    key, val = l.split(":")
                    if key == "u":
                        self._responseLists[listName]["urls"].append(val)
                    elif key == "ad":
                        self._responseLists[listName]["adddel"] = chunksFromStr(val)
                    elif key == "sd":
                        self._responseLists[listName]["subdel"] = chunksFromStr(val)
        
        return self._responseLists

    def needReset(self):
        return self._needReset
    
    def nextDelay(self):
        return self._nextDelay


class SecureRequest(Request):
    def __init__(self, uri = ""):
        Request.__init__(self, "https://sb-ssl.google.com/safebrowsing/" + uri)




class NewKeyRequest(SecureRequest):
    def __init__(self):
        SecureRequest.__init__(self, "newkey")
        
    def send(self):
        data = SecureRequest.send(self)
        lines = data.strip().split("\n")
        keys = {}
        for l in lines:
            name, size, value = l.split(":")
            if int(size) != len(value):
                raise MalformedResponseError("actual(%d) and given(%d) sizes of key %s do not match" %
                                        (len(value), int(size), name))
            keys[name] = value
        return keys