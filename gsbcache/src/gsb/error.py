'''
Created on 30.12.2010

@author: rion
'''
class HttpError(Exception):
    def __init__(self, status, reason):
        self.status = status
        self.reason = reason
        
    def __str__(self):
        return "HTTP Error ($s: %s)" % (self.status, self.reason)
        