'''
Created on 28.12.2010

@author: rion
'''

import datetime
import httplib

from gsb.error import HttpError, MalformedResponse
from gsb.config import Config


class Client(object):
    
    host = "safebrowsing.clients.google.com"
    keyHost = "sb-ssl.google.com"
    uri = "/safebrowsing/"
    
    ffVer = "3.6.13"
    pver = "2.2"

    # gotten from logs    
    defaultWrkey = "AKEgNit_MOYot7yU_tKwygYRBvj_k-aTBU1fx0pQZvRDUd1RM4B5TAqT5cwzuQwnB9ZxeRhwPm7kY1pZS7NFU5m46DeeOyo_4Q=="
    
    def __init__(self):
        pass

    def _makeUri(self, method):
        mac = Config.instance().get("mac-key")
        return self.uri + str(method) + ("?client=Firefox&appver="+self.ffVer+
                                         "&pver="+self.pver+"&wrkey=" + mac)
        
    def _request(self, method, body = "", conn = None):
        conn = conn or httplib.HTTPConnection(self.host)
        conn.request("POST", self._makeUri(method), headers={"Content-Length" : len(body)})
        r = conn.getresponse()
        if r.status != 200:
            raise HttpError(r.status, r.reason)
        return r.read()


    def isReady(self):
        return self.delayedUntil < datetime.datetime.today()
            
    def getLists(self):
        while (True):
            data = self._request("list")
            if data.startswith("e:pleaserekey"):
                self.updateKey()
            else:
                return data.strip().split("\n")[1:]

    
    def updateList(self):
        if not self.mac:
            self.updateKey()
        conn = httplib.HTTPConnection(self.host)
        conn.request("POST", self._makeUri("downloads"), headers={"Content-Length" : 0})
        
            
        #Config.instance().set("last-update", str(int(time.time())))
            
    def updateKey(self):
        data = self._request("newkey", conn=httplib.HTTPSConnection(self.keyHost, 443,
                            key_file=self.sslKey, cert_file=self.sslCert))
        lines = data.strip().split("\n")
        keys = {}
        for l in lines:
            name, size, value = l.split(":")
            if int(size) != len(value):
                raise MalformedResponse("actual(%d) and given(%d) sizes of key %s do not match" %
                                        (len(value), int(size), name))
            keys[name] = value
            
        self.mac = keys["wrappedkey"]
        Config.instance().set("mac-key", self.mac)
        # here is also `clientkey` in the dict
