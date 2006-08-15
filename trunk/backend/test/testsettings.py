import sys
import os
# Enable importing from parent directory
relativePathToScript = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relativePathToScript, "..")
sys.path.append(relativePathToParent)
import unittest
from settings import Settings

class TestSettings(unittest.TestCase):
##    def setUp(self):
##        pass
##    
##    def tearDown(self):
##        pass
    
    def testCreateSettingsFile(self):
        """I'm not sure if this is really a good test because it tests an 
        implementation detail and not the interface.
        """
        if os.path.isfile('cocoman.conf'):
            self.fail("A file already exists with the same name the test was going"
                      + " to use. Please remove it.")
        settings = Settings('cocoman.conf')
        assert(os.path.isfile('cocoman.conf') is True)
        os.rm('cocoman.conf')


def suite():
    # TODO
    pass

if __name__ == "__main__":
    unittest.main()