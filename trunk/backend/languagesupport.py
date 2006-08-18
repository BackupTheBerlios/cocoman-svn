#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


class LanguageSupport:
    """This is a base class which all classes that offer support for
    languages must derive from. This class can't be instantiated.
    """
    def __init__(self):
        if type(self) == LanguageSupport:
            raise NotImplementedError
    
    def get_supported_extensions(self):
        """Case of letters doesn't matter- extensions will be matched case 
        insensitively (all lower case recommended for consistancy).
        Should not include a '.' before the letters of the extension.
        """
        raise NotImplementedError

    def build(self, source_file_name):
        """Throws:
        CompileFailedError
        """
        raise NotImplementedError

    def run(self, executable_file_name):
        """Throws:
        ExecutionTimedOutError?
        FailedTestError?
        I don't know yet how piping in the input and grabbing the output will work.
        """
        raise NotImplementedError

    def _run_commands_capture_output(self, commands):
        raise NotImplementedError
