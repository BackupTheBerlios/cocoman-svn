#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


class LanguageSupport:

    def get_supported_extensions(self):
        """Case of letters doesn't matter- extensions will be matched case 
        insensitively (all lower case recommended for consistancy).
        Should not include a '.' before the letters of the extension.
        """
        pass

    def build(self, source_file_name):
        """Throws:
        CompileFailedError
        """
        pass

    def run(self, executable_file_name):
        """Throws:
        ExecutionTimedOutError?
        FailedTestError?
        I don't know yet how piping in the input and grabbing the output will work.
        """
        pass

    def _run_commands_capture_output(self, commands):
        pass
