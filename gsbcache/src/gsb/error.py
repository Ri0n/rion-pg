'''
Created on 30.12.2010

@author: rion
'''
class HttpError(Exception):
    def __init__(self, status, reason):
        self.status = status
        self.reason = reason
        
    def __str__(self):
        return "HTTP Error (%s: %s)" % (str(self.status), str(self.reason))
        
class MalformedResponse(Exception):
    pass