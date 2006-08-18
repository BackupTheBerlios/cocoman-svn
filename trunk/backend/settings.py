#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2
import ConfigParser

KEYS = ['root', 
        'poll_interval', 
        'execution_timeout', 
        'java_binary', 
        'number_of_problems',
        'allowed_languages',
        'allowed_ips',
    ]
# TODO Figure out a better way to share these amongst instances
class Settings:
    """Use the singleton pattern - all users of the class must be referring to 
    the same instance.
    """
    __setfuncs__ = {}
    __getfuncs__ = {}
    shared_attrs = {}
    def __init__(self):
        self.__dict__ = self.shared_attrs

        self.root = None
        self.poll_interval = None
        self.execution_timeout = None
        self.java_binary = None
        self.number_of_problems = None
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
        parser = ConfigParser.ConfigParser()
        parser.read(file_name)
        for key in KEYS:
            try:
                setattr(self, key, parser.get("global", key))
            except ConfigParser.NoOptionError, e:
                print "%s option not found in %s" % (key, file_name)
        self._file_name = file_name

    
    def save(self, file_name=None):
        parser = ConfigParser.ConfigParser()
        parser.add_section('global')
        for key in KEYS:
            parser.set('global', key, getattr(self, key))
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
        pass
    
    def set_poll_interval(self, seconds):
        pass
    
    def get_execution_timeout(self):
        pass
    
    def set_execution_timeout(self, seconds):
        pass
    
    def get_java_binary(self):
        pass
    
    def set_java_binary(self, path):
        pass
    
    def get_allowed_ips(self):
        """ips can contain one '*' as a wildcard (eg 192.168.*)."""
        pass
    
    def set_allowed_ips(self, ips):
        """ips can contain one '*' as a wildcard (eg 192.168.*)."""
        pass
    
    def get_number_of_problems(self):
        pass
    
    def set_number_of_problems(self, number_of_problems):
        pass

    def get_allowed_languages(self):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        pass
    
    def add_allowed_lanuguage(self, language):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        pass
    
    def remove_allowed_lanuguage(self, language):
        """The language must match the first part of the class name of the 
        support class for that language (eg "C" for "CSupport", "Cpp" for 
        "CppSupport").
        """
        pass
    

if __name__ == "__main__":
    s = Settings()
    s.load('contest.ini')
    s.save('contest.ini.sav')
    s.root = 'test'
    print s.root
