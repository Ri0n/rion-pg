import unittest
import struct
import os

import sys
import pprint
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from gsb.extra import NumbersList
from gsb.request import Request
from gsb.config import Config
from gsb import Client

Request.cacheData = True


class StructuredFileTest(unittest.TestCase):


    def setUp(self):
        storage = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "data"))
        if not os.path.exists(storage):
            os.makedirs(storage, 0755)
        Config.init(os.path.join(storage, "config.ini"))
        Config.instance().set("storage", storage)

    def runTest(self):
        client = Client()
    
        try:
            #print client.getLists()
            pprint.pprint(client.downloadList("goog-malware-shavar"))
            #client.updateKey()
        finally:    
            Config.instance().save()

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    
    unittest.main()