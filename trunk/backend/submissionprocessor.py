#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


from settings import settings


class SubmissionProcessor:
    
    # A dictionary of file extension strings to instances of the class that 
    # supports that language.
    supported_extensions = {}
    
    def load_language_support(self):
        """Throws:
        DuplicateExtentionSupportError
        UnhandledExtentionError
        """
        for language in settings.get_allowed_languages():
            support_class = language + 'Support'
            module_name = support_class.lower()
            module = __import__(module_name)
            language_support = module.__dict__[support_class]()
            for new_extension in language_support.get_supported_extensions():
                if new_extension in self.supported_extensions.keys():
                    raise DuplicateExtentionSupportError
                else:
                    self.supported_extensions[new_extension] = language_support
        print("Loaded support for the following extensions: " 
                + str(self.supported_extensions.keys()).strip('[]')) #TODO log this

    def process_submission(self, file_name, problem_number):
        pass
