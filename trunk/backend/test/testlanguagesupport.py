import sys
import os
# Enable importing from parent directory
relativePathToScript = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relativePathToScript, "..")
sys.path.append(relativePathToParent)
import unittest
from settings import Settings
import languagesupport
from languagesupport import LanguageSupport


class TestLanguageSupport(unittest.TestCase):
    def setUp(self):
        self.language_support = LanguageSupport()
    
##    def testRunOneCommand(self):
##        output = self.language_support._run_commands_capture_output([["echo", "doody on my head"]])
##        self.assertEqual(output, "doody on my head")


if __name__ == "__main__":
    unittest.main()
