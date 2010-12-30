'''
Created on 28.12.2010

@author: rion
'''

import os
from gsb import Client

if __name__ == '__main__':
    client = Client(
        os.path.join(os.path.dirname(__file__), "data"),
        os.path.join(os.path.dirname(__file__), "ssl.key"),
        os.path.join(os.path.dirname(__file__), "ssl.crt")
    )
    #print client.getLists()
    #client.update("goog-malware-shavar")
    try:
        client.updateKey()
    except Exception, e:
        print str(e)