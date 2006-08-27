import sys
import os
# Enable importing from parent directory
relativePathToScript = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relativePathToScript, "..")
sys.path.append(relativePathToParent)
import unittest
from settings import settings
from submissionprocessor import SubmissionProcessor, UnhandledExtensionError


class TestSubmissionProcessor(unittest.TestCase):
    def setUp(self):
        self.submission_processor = SubmissionProcessor()
    
    def tearDown(self):
        for language in settings.get_allowed_languages():
            settings.remove_allowed_language(language)
    
    def testLoadLanguageSupport(self):
        settings.add_allowed_language('Cpp')
        self.submission_processor.load_language_support()
    
    def testInvalidLanguage(self):
        settings.add_allowed_language('Bob')
        self.assertRaises(UnhandledExtensionError, self.submission_processor.load_language_support)


if __name__ == "__main__":
    unittest.main()
