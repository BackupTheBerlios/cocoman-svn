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
        """Tests that instantiating and saving a Settings creates the settings 
        file if it doesn't exist.
        """
        if os.path.isfile('cocoman.conf'):
            self.fail("A file already exists with the same name the test was going"
                      + " to use. Please remove it.")
        settings = Settings()
        settings.save('cocoman.conf')
        assert(os.path.isfile('cocoman.conf') is True)
        os.rm('cocoman.conf')

    def testReadNonExistantSettingsFile(self):
        """Tests that reading a non existant settings file will throw an exception"""
        settings = Settings()
        self.assertRaises(IOError, settings.load, "nonexistantfilethatwouldneverappear")
    
    def testSingletonness(self):
        s1 = Settings()
        s1.set_root('aaaaa')
        s2 = Settings()
        self.assertEquals(s2.get_root(), 'aaaaa')


def suite():
    # TODO
    pass

if __name__ == "__main__":
    unittest.main()
