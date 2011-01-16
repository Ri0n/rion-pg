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

import io
import os
import urlparse
import urllib2
from gsb.config import Config
from gsb.error import MalformedResponseError, MacCheckFailed
import datetime
import base64
import hashlib
import hmac

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
    
    WrappedKey = None
    ClientKey = None
    
    cacheData = False # just for debug atm
    
    def __init__(self, url):
        standardParams = self._standardParams()
        if standardParams:
            parts = urlparse.urlsplit(url, "http")
            query = parts.query + ("&" + standardParams if parts.query else standardParams)
            parts = list(parts)
            parts[3] = query
            url = urlparse.urlunsplit(parts)
        self._httpreq = urllib2.Request(url)
            
    def send(self, body=""):
        print("send: %s" % body)
        f = None
        if self.cacheData:
            fname = os.path.join(Config.instance().get("storage"),
                                 hashlib.sha1(self.url() + (body and body or "")).digest().encode("hex"))
            if os.path.exists(fname):
                f = io.FileIO(fname)
                data = f.read()
        if not f:
            self._httpreq.add_data(body)
            f = urllib2.urlopen(self._httpreq)
            data = f.read()
            if self.cacheData:
                f = io.FileIO(fname, "w")
                f.write(data)
            
        print("read: %s" % data)
        return data

    def url(self):
        return self._httpreq.get_full_url()
    
    def _standardParams(self):
        # lets it will be here for a while
        standardParams = "client=" + self.ClientName + "&appver=" + self.ClientVersion + \
                        "&pver=" + self.APIVersion
        self._useMac = Config.instance().getboolean("use-mac")
        if self._useMac:
            standardParams += ("&wrkey=" + Request.WrappedKey)
        
        return standardParams



class NormalRequest(Request):
    '''
    Normal client requests
    '''
    
    Rekey = "e:pleaserekey"
    
    def __init__(self, uri=""):
        self._needNewKey = False
        Request.__init__(self, "http://safebrowsing.clients.google.com/safebrowsing/" + uri)
        
    def needNewKey(self):
        return self._needNewKey
    
    def validateMac(self, mac, data):
#        if hmac.new(self.ClientKey, data, hashlib.sha1).digest() != \
#            base64.urlsafe_b64decode(mac):
#            raise MacCheckFailed("mac %s is invalid" % mac)
        return # check if lines above are working





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
            self.validateMac(lines[0], data) # FIXME possible invalid data param
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
        
    def addList(self, name, aList=[], sList=[]):
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
                self.validateMac(l, data) # FIXME data
        next = lines.pop(0)
        if not next.startswith("n:"):
            raise MalformedResponseError('NEXT expected')
        self._nextDelay = datetime.timedelta(seconds=int(next[2:]))
        
        if lines[0].startswith("r:pleasereset"):
            self._needReset = True
            return # invalid trailing data is ignored if exists
        
        self._responseLists = {}
        while (True):
            if not len(lines):
                break
            key, listName = lines.pop(0).split(":")
            if key == "i":
                self._responseLists[listName] = {"urls": [], "adddel": [], "subdel": []}
                while len(lines) and not lines[0].startswith("i:"):
                    l = lines.pop(0)
                    if not l: break # next list
                    key, val = l.split(":")
                    if key == "u":
                        if self._useMac:
                            up = val.split(",")
                            if len(up) > 1:
                                self.validateMac(up.pop(), data) # TODO most likely this is for redirect request, not for this one
                                val = ",".join(up)
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



class RedirectRequest(Request):
    
    def __init__(self, url):
        Request.__init__(self, "http://" + url)
        
    def _standardParams(self):
        return ""
        
    def send(self):
        fp = io.BytesIO()
        fp.write(Request.send(self, None))
        fp.seek(0)
        chunks = []
        while (True):
            l = fp.readline()
            if not l:
                break
            cType, cNum, hashLen, cLen = l.strip().split(":")
            cLen = int(cLen)
            chunk = fp.read(cLen) if cLen else ""
            print "chunk data: ", chunk.encode("hex")
            chunks.append({"type": cType, "cn": int(cNum),
                           "hl": int(hashLen), "data": chunk})
        print "parsed: " + self.url()
        return chunks
            


class SecureRequest(Request):
    def __init__(self, uri=""):
        Request.__init__(self, "https://sb-ssl.google.com/safebrowsing/" + uri)




class NewKeyRequest(SecureRequest):
    def __init__(self):
        SecureRequest.__init__(self, "newkey")
    
    def _standardParams(self):
        # lets it will be here for a while
        return "client=" + self.ClientName + "&appver=" + self.ClientVersion + \
                "&pver=" + self.APIVersion
    
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
