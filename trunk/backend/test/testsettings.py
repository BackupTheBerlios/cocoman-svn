import sys
import os
# Enable importing from parent directory
relativePathToScript = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relativePathToScript, "..")
sys.path.append(relativePathToParent)
import unittest
from settings import Settings

class TestSettings(unittest.TestCase):
    
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
        os.remove('cocoman.conf')

    def testReadNonExistantSettingsFile(self):
        """Tests that reading a non existant settings file will throw an exception"""
        settings = Settings()
        self.assertRaises(IOError, settings.load, "nonexistantfilethatwouldneverappear")
    
    def testReadSetting(self):
        s = Settings()
        for setting in [ 'root', 'poll_interval', 'execution_timeout', 'java_binary', 'number_of_problems' ]:
            value = getattr(s, setting)
        
    
    def testWriteThenReadSetting(self):
        s = Settings()
        s.root = "/hello/world"
        self.assertEqual(s.root, "/hello/world")

    def testReadingNonExistantSetting(self):
        self.assertRaises(AttributeError, self._readNonExistantSetting)
    
    def _readNonExistantSetting(self):
        settings = Settings()
        value = settings.pie


if __name__ == "__main__":
    unittest.main()
