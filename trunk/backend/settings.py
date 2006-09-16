#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2

"""
All users of Settings should use the instance created in this file called settings.
"""

import ConfigParser
import os

KEYS = ['root', 
        'poll_interval', 
        'execution_timeout', 
        'java_binary', 
        'number_of_problems',
        'allowed_languages',
        'allowed_ips',
    ]

class Settings:
    
    def __init__(self):
        self._root = ""
        self._poll_interval = 5
        self._execution_timeout = 30
        self._java_binary = ""
        self._number_of_problems = 1

        self._allowed_languages = []
        self._allowed_ips = []
    
    
    def load(self, file_name):
        """If file_name doesn't exist, raises an IOError."""
        if not os.path.exists(file_name):
            raise IOError("Tried to load settings from a file that doesn't exist.")
        parser = ConfigParser.ConfigParser()
        parser.read(file_name)
        self._file_name = file_name
        for key in KEYS:
            try:
                setattr(self, key, parser.get("global", key))
            except ConfigParser.NoSectionError, e:
                print "The config file doesn't have the necessary section. Using defaults." # TODO Log
                return
            except ConfigParser.NoOptionError, e:
                print "The config file doesn't have a setting for %s. Using default." % key # TODO Log
    
    def save(self, file_name=None):
        """If the file_name argument is provided, the settings will be saved 
        to that file. If not, the settings will be saved to the file they were 
        loaded from. If file_name is not provided and the settings were never 
        loaded, an IOError will be raised. An IOError can also be raised if 
        something goes wrong while trying to save (eg the user doesn't have
        permission to write to the file).
        
        """
        parser = ConfigParser.ConfigParser()
        parser.add_section('global')
        for key in KEYS:
            exec "value = self.get_%s()" % key
            parser.set('global', key, value)
        if not file_name:
            try:
                file_name = self._file_name
            except AttributeError:
                raise IOError("save(): No file specified to save configuration to")
        parser.write(open(file_name, 'w'))
    
    def get_root(self):
        return self._root 
    
    def set_root(self, path):
        self.__dict__['root'] = path 
    
    def get_poll_interval(self):
        return self._poll_interval
    
    def set_poll_interval(self, seconds):
        pass
    
    def get_execution_timeout(self):
        return self._execution_timeout
    
    def set_execution_timeout(self, seconds):
        pass
    
    def get_java_binary(self):
        return self._java_binary
    
    def set_java_binary(self, path):
        pass
    
    def get_allowed_ips(self):
        """ips can contain one '*' as a wildcard (eg 192.168.*)."""
        return self._allowed_ips
    
    def set_allowed_ips(self, ips):
        """ips can contain one '*' as a wildcard (eg 192.168.*)."""
        pass
    
    def get_number_of_problems(self):
        return self._number_of_problems
    
    def set_number_of_problems(self, number_of_problems):
        pass
    
    def get_allowed_languages(self):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        Returns a sequence of strings.
        """
        return self._allowed_languages
    
    def add_allowed_language(self, language):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        if language not in self._allowed_languages:
            self._allowed_languages.append(language)
    
    def remove_allowed_language(self, language):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        self._allowed_languages.remove(language)

    root = property(get_root, set_root)
    poll_interval = property(get_poll_interval, set_poll_interval)
    execution_timeout = property(get_execution_timeout, set_execution_timeout)
    java_binary = property(get_java_binary, set_java_binary)
    number_of_problems = property(get_number_of_problems, set_number_of_problems)


settings = Settings()
##settings.save('cocoman.ini')
