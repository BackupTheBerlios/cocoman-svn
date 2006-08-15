import sys
import os
# Enable importing from parent directory
relativePathToScript = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relativePathToScript, "..")
sys.path.append(relativePathToParent)
import unittest
from settings import Settings
import log
from log import debug1, log1, log_build, log_to_frontend
import random

class TestLog(unittest.TestCase):
    
    def setUp(self):
        self.settings = Settings()
        self.settings.root = relativePathToScript
    
    def testDebugOn(self):
        log.debug_on = True
        log_file_name = os.path.join(self.settings.root, 'log')
        log_file_name = os.path.join(log_file_name, 'cocoman.log')
        try:
            os.remove(log_file_name)
        except:
            pass
        debug1("testing debug 1")
        log_file = file(log_file_name)
        lines = log_file.readlines()
        self.assertEqual(lines[-1].strip(), 'debug1: testing debug 1')
        os.remove(log_file_name)
    
    def testDebugOff(self):
        log.debug_on = False
        log_file_name = os.path.join(self.settings.root, 'log')
        log_file_name = os.path.join(log_file_name, 'cocoman.log')
        try:
            os.remove(log_file_name)
        except:
            pass
        debug1("testing debug 1")
        self.assertRaises(IOError, open, log_file_name)


def suite():
    # TODO
    pass

if __name__ == "__main__":
    unittest.main()
