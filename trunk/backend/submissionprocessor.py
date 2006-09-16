#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


from settings import settings
from logging import debug, info, warning


class UnhandledExtensionError(Exception):
    def __init__(self, extension):
        self.extension = extension

class FailedTestError(Exception):
    pass

class CrashedError(Exception):
    pass

class ExecutionTimedOutError(Exception):
    pass


class SubmissionProcessor:
    
    # A dictionary of file extension strings to instances of the class that 
    # supports that language.
    supported_extensions = {}
    
    def load_language_support(self):
        """Throws:
        DuplicateExtentionSupportError
        UnhandledExtensionError
        """
        for language in settings.get_allowed_languages():
            support_class = language + 'Support'
            module_name = support_class.lower()
            try:
                module = __import__(module_name)
                language_support = module.__dict__[support_class]()
            except ImportError:
                raise UnhandledExtensionError
            for new_extension in language_support.get_supported_extensions():
                if new_extension in self.supported_extensions.keys():
                    raise DuplicateExtentionSupportError
                else:
                    self.supported_extensions[new_extension] = language_support
        info("Loaded support for the following extensions: " 
                + str(self.supported_extensions.keys()).strip('[]'))
    
    def process_submission(self, source_file_name, problem_number):
        """Compiles a submission and runs the tests cases on it.
        
        Uses the appropriate language support to compile a submission and run
        the test cases on it. Doesn't return anything on success. Can raise
        FailedTestError, CrashedError, TimedOutError, BuildFailedError, 
        UnsupportedLanguageError.
        
        """
        assert problem_number <= settings.number_of_problems
        # Find the right language support
        # Compile the source and get the executable name
        # Have the language support run the executable and pass back a handle
        # Split off seperate threads for input and output of the process
        # Start a timer
        # Have the other threads do their thing
        # If the timeout is exceeded raise exception
        # If the program crashes raise exception
        # When the program finishes, if the output is wrong raise exception
        try:
            extension = source_file_name.rsplit('.')[-1]
            support = self.supported_extensions[extension]
        except KeyError:
            raise UnsupportedLanguageError(extension)
        executable = support.compile(source_file_name)
        print executable
        for i in self._find_number_of_testcases()
            self._run_submission_with_testcase(executable, support, i)
    
    def _self._run_submission_with_testcase(self, executable, language_support, 
            problem_number):
        problems_dir = os.path.join(settings.root, 'problems')
        problem_dir = os.path.join(problems_dir, 'Problem' + problem_number)

    def _run_commands_capture_output(self, commands):
        """Runs the specified commands, kill them if the timeout is exceeded, 
        and returns anything the program(s) output on stdout or stderr.
        commands must be a list of commands to run where each command to run
        is a list with the command to run in the 0th element followed by the 
        args in the following positions. Eg:
        [
            ["ls"],
            ["mv", "foo", "bar"]
        ]
        Can raise ExecutionTimedOutError.
        Implementation: This function spawns a second thread to do the execution
        of the commands and read their output into a class variable. Since that 
        thread doesn't block on anything and reads both stdout and stderr in one
        stream, there shouldn't be any possible deadlocks.
        The first thread does the timing and kills the child process if the 
        timeout specified in the settings is exceeded. Then it returns the data 
        captured by the second thread.
        """
        assert type(commands) == list
        assert len(commands) > 0
        assert type(commands[0]) == list
        assert len(commands[0]) > 0
        self.commands_output = ""
        # commands needs to be in [] because the call dereferences one set of brackets
        run_commands_thread = Thread(target=self.__actual_run_commands_capture_output, args=[commands])
        # TODO start timer
        run_commands_thread.run()
        # TODO kill process when timer expires
        return self.commands_output
    
    def __actual_run_commands_capture_output(self, commands):
        """Runs the specified commands and puts the output in 
        self.commands_output.
        commands must be a list of commands to run where each command to run
        is a list with the command to run in the 0th element followed by the 
        args in the following positions. Eg:
        [
            ["ls"],
            ["mv", "foo", "bar"]
        ]
        """
        for command in commands:
            self.process = Popen(command, stdin=PIPE, stdout=PIPE, 
                                    stderr=STDOUT, close_fds=True)
            print "type of pipe is %s" % type(self.process.stdin)
    
    def _find_number_of_testcases(self, problem_number):
        """Looks in root/problems/ for a directory Problem<problem_number>. Then
        it looks for pairs of files of the form <int>.input and <int>.output
        starting from 1. As soon as it finds the first number which doesn't have
        both, it stops and returns the number of pairs found.
        
        """
        problems_dir = os.path.join(settings.root, 'problems')
        problem_dir = os.path.join(problems_dir, 'Problem' + problem_number)
        files = os.listdir(problem_dir)
        number_of_testcases = 1
        while True:
            if str(number_of_testcases) + '.input' in files \
                    and str(number_of_testcases) + '.output' in files:
                number_of_testcases = number_of_testcases + 1
            else:
                number_of_testcases = number_of_testcases - 1
                return number_of_testcases
