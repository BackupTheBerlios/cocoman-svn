import sys
import os
# Enable importing from parent directory
relativePathToScript = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relativePathToScript, "..")
sys.path.append(relativePathToParent)
import unittest
from settings import settings
from submissionprocessor import SubmissionProcessor


class TestSubmissionProcessor(unittest.TestCase):
    def setUp(self):
        self.submission_processor = SubmissionProcessor()
    
    def testLoadLanguageSupport(self):
        settings.add_allowed_language('Cpp')
        self.submission_processor.load_language_support()


def suite():
    # TODO
    pass

if __name__ == "__main__":
    unittest.main()
