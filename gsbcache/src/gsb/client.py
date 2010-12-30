'''
Created on 28.12.2010

@author: rion
'''

import os
import time
import datetime
import ConfigParser
import httplib

from gsb.error import HttpError, MalformedResponse


class Client(object):
    
    host = "safebrowsing.clients.google.com"
    keyHost = "sb-ssl.google.com"
    uri = "/safebrowsing/"
    #baseUrl = "http://" + host + "/safebrowsing/"
    ffVer = "3.6.13"
    pver = "2.2"
    
    configFile = "config.ini"
    defaultWrkey = "AKEgNit_MOYot7yU_tKwygYRBvj_k-aTBU1fx0pQZvRDUd1RM4B5TAqT5cwzuQwnB9ZxeRhwPm7kY1pZS7NFU5m46DeeOyo_4Q=="
    
    def __init__(self, storage, sslKey, sslCert):
        self.storage = storage
        self.sslKey = sslKey
        self.sslCert = sslCert
        print storage
        if not os.path.exists(storage):
            os.makedirs(storage, 0755)
            
        self.config = ConfigParser.ConfigParser()
        self.config.read(os.path.join(storage, self.configFile))
        
        try:
            self.lastUpdate = datetime.datetime.fromtimestamp(
                                    self.config.getint("DEFAULT", "last-update"))
        except:
            self.lastUpdate = datetime.datetime(1970, 1, 1)
            
        try:
            self.mac = self.config.get("DEFAULT", "mac-key")
        except:
            self.mac = self.defaultWrkey
                 
    def __del__(self):
        with open(os.path.join(self.storage, self.configFile), 'wb') as cf:
            self.config.write(cf)
            
    def _makeUri(self, method):
        return self.uri + str(method) + ("?client=Firefox&appver="+self.ffVer+
                                         "&pver="+self.pver+"&wrkey=" +self.mac)
        
    def _request(self, method, body = "", conn = None):
        conn = conn or httplib.HTTPConnection(self.host)
        conn.request("POST", self._makeUri(method), headers={"Content-Length" : len(body)})
        r = conn.getresponse()
        if r.status != 200:
            raise HttpError(r.status, r.reason)
        return r.read()

            
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
        
            
        self.config.set("DEFAULT", "last-update", str(int(time.time())))
            
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
        self.config.set("DEFAULT", "mac-key", self.mac)
        # here is also `clientkey` in the dict
