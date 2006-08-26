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
    __setfuncs__ = {}
    __getfuncs__ = {}
    def __init__(self):
        self.root = ""
        self.poll_interval = 5
        self.execution_timeout = 30
        self.java_binary = ""
        self.number_of_problems = 1
        self.allowed_languages = []
        self.allowed_ips = []
        self.__register_funcs__()
    
    def __register_funcs__(self):
        # Register the __setattr__ functions
        self.__setfuncs__['root'] = self.set_root
        self.__setfuncs__['poll_interval'] = self.set_poll_interval
        self.__setfuncs__['execution_timeout'] = self.set_execution_timeout
        self.__setfuncs__['java_binary'] = self.set_java_binary
        self.__setfuncs__['number_of_problems'] = self.set_number_of_problems
        self.__setfuncs__['allowed_ips'] = self.set_allowed_ips
        
        # Register the __getattr__ functions
        self.__getfuncs__['root'] = self.get_root
        self.__getfuncs__['poll_interval'] = self.get_poll_interval
        self.__getfuncs__['execution_timeout'] = self.get_execution_timeout
        self.__getfuncs__['java_binary'] = self.get_java_binary
        self.__getfuncs__['number_of_problems'] = self.get_number_of_problems
        self.__getfuncs__['allowed_languages'] = self.get_allowed_languages
        self.__getfuncs__['allowed_ips'] = self.get_allowed_ips
    
    def __setattr__(self, attr, value):
        try:
            func = self.__setfuncs__[attr]
            func(value)
        except KeyError:
            self.__dict__[attr] = value
    
    def __getattr__(self, attr):
        try:
            func = self.__getfuncs__[attr]
            return func()
        except KeyError:
            return self.__dict__[attr]
    
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
        return self.root 
    
    def set_root(self, path):
        self.__dict__['root'] = path 
    
    def get_poll_interval(self):
        return self.poll_interval
    
    def set_poll_interval(self, seconds):
        pass
    
    def get_execution_timeout(self):
        return self.execution_timeout
    
    def set_execution_timeout(self, seconds):
        pass
    
    def get_java_binary(self):
        return self.java_binary
    
    def set_java_binary(self, path):
        pass
    
    def get_allowed_ips(self):
        """ips can contain one '*' as a wildcard (eg 192.168.*)."""
        return self.allowed_ips
    
    def set_allowed_ips(self, ips):
        """ips can contain one '*' as a wildcard (eg 192.168.*)."""
        pass
    
    def get_number_of_problems(self):
        return self.number_of_problems
    
    def set_number_of_problems(self, number_of_problems):
        pass
    
    def get_allowed_languages(self):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        Returns a sequence of strings.
        """
        return self.allowed_languages
    
    def add_allowed_language(self, language):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        if language not in self.allowed_languages:
            self.allowed_languages.append(language)
    
    def remove_allowed_language(self, language):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        pass


settings = Settings()
##settings.save('cocoman.ini')
