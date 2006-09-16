#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


import threading
from threading import Thread
import subprocess
from subprocess import Popen, PIPE, STDOUT


class CompileFailedError(Exception):
    def __init__(self, compile_log):
        self.compile_log = compile_log


class LanguageSupport:
    """This is a base class which all classes that offer support for
    languages must derive from. This class can't be instantiated.
    """
    def __init__(self):
        if type(self) == "LanguageSupport":
            raise NotImplementedError
            # TODO Fix this
    
    def get_supported_extensions(self):
        """Case of letters doesn't matter- extensions will be matched case 
        insensitively (all lower case recommended for consistancy).
        Should not include a '.' before the letters of the extension.
        """
        raise NotImplementedError

    def compile(self, source_file_name):
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
