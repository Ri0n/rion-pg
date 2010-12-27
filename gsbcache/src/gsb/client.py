'''
Created on 28.12.2010

@author: rion
'''
class GSBClient(object):
    
    baseUrl = "http://safebrowsing.clients.google.com/safebrowsing/"
    ffVer = "3.6.13"
    pver = "2.2"
    
    def __init__(self):
        pass
    
    def update(self):
        wrkey = "AKEgNit_MOYot7yU_tKwygYRBvj_k-aTBU1fx0pQZvRDUd1RM4B5TAqT5cwzuQwnB9ZxeRhwPm7kY1pZS7NFU5m46DeeOyo_4Q=="
        
        dlUrl = "downloads?client=Firefox&appver=%(ffVer)s&pver=$(pver)s&wrkey=$(wrkey)s" \
            % (self.ffVer, self.pver, wrkey)
         