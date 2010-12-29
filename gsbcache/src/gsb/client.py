'''
Created on 28.12.2010

@author: rion
'''

import os
import time
import datetime
import ConfigParser
import httplib

from gsb.error import HttpError


class Client(object):
    
    host = "safebrowsing.clients.google.com"
    uri = "/safebrowsing/"
    #baseUrl = "http://" + host + "/safebrowsing/"
    ffVer = "3.6.13"
    pver = "2.2"
    
    configFile = "config.ini"
    defaultWrkey = "AKEgNit_MOYot7yU_tKwygYRBvj_k-aTBU1fx0pQZvRDUd1RM4B5TAqT5cwzuQwnB9ZxeRhwPm7kY1pZS7NFU5m46DeeOyo_4Q=="
    
    def __init__(self, storage):
        self.storage = storage
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

            
    def getLists(self):
        conn = httplib.HTTPConnection(self.host)
        while (True):
            conn.request("POST", self._makeUri("list"), headers={"Content-Length" : 0})
            r = conn.getresponse()
            if r.status == 200:
                data = r.read()
                if data.startswith("e:pleaserekey"):
                    self.updateKey()
                else:
                    return data.strip().split("\n")[1:]
            else:
                raise HttpError(r.status, r.reason)


        

    
    def updateList(self):
        if not self.mac:
            self.updateKey()
        conn = httplib.HTTPConnection(self.host)
        conn.request("POST", self._makeUri("downloads"), headers={"Content-Length" : 0})
        
            
        self.config.set("DEFAULT", "last-update", str(int(time.time())))
            
    def updateKey(self):
        pass
         