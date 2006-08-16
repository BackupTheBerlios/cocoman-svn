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
import shutil


class TestLog(unittest.TestCase):
    
    def setUp(self):
        self.settings = Settings()
        self.settings.root = relativePathToScript
        self.log_dir = os.path.join(self.settings.root, 'log')
        self.user_log_file = os.path.join(self.log_dir, 'cocoman.log')
        shutil.rmtree(self.log_dir, True)
        os.mkdir(self.log_dir)
    
    def tearDown(self):
        shutil.rmtree(self.log_dir, True)

    def testDebugOn(self):
        log.debug_on = True
        debug1("testing debug 1")
        log_file = file(self.user_log_file)
        lines = log_file.readlines()
        log_file.close()
        self.assertEqual(lines[-1].strip(), 'debug1: testing debug 1')
        debug1("hello world")
        log_file = file(self.user_log_file)
        lines = log_file.readlines()
        log_file.close()
        self.assertEqual(lines[-1].strip(), 'debug1: hello world')
   
    def testDebugOff(self):
        log.debug_on = False
        debug1("testing debug 1")
        self.assertRaises(IOError, open, self.user_log_file)
    
    def testLog1DebugOn(self):
        log.debug_on = True
        log1("testing log1")
        log_file = open(self.user_log_file)
        lines = log_file.readlines()
        log_file.close()
        self.assertEqual(lines[-1].strip(), 'log1:   testing log1')
    
    def testLog1DebugOff(self):
        log.debug_on = False
        log1("testing log1")
        log_file = open(self.user_log_file)
        lines = log_file.readlines()
        log_file.close()
        self.assertEqual(lines[-1].strip(), 'testing log1')


def suite():
    # TODO
    pass

if __name__ == "__main__":
    unittest.main()
