'''
Created on 28.12.2010

@author: rion
'''

import os
import argparse
import pprint

from gsb import Client, Config

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser(description='Keep google safe browsing cache.')
    parser.add_argument('-c', '--config', dest='config', metavar="configFile",
           help='set configuration file location (must be writable)')
    
    args = parser.parse_args()
    if args.config:
        Config.init(args.config)
    else: # our self-created testing config
        storage = os.path.join(os.path.dirname(__file__), "data")
        if not os.path.exists(storage):
            os.makedirs(storage, 0755)
        Config.init(os.path.join(storage, "config.ini"))
        Config.instance().set("storage", storage)
        Config.instance().set("ssl-key-file",
                              os.path.join(os.path.dirname(__file__), "ssl.key"))
        Config.instance().set("ssl-crt-file",
                              os.path.join(os.path.dirname(__file__), "ssl.crt"))
        try:
            Config.instance().get("mac-key")
        except:
            Config.instance().set("mac-key",
                                  "AKEgNit_MOYot7yU_tKwygYRBvj_k-aTBU1fx0pQZvRDUd1RM4B5TAqT5cwzuQwnB9ZxeRhwPm7kY1pZS7NFU5m46DeeOyo_4Q==")
        
    client = Client()
    
    try:
        #print client.getLists()
        pprint.pprint(client.downloadList("goog-malware-shavar"))
        #client.updateKey()
    finally:    
        Config.instance().save()
    