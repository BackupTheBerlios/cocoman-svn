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
    __initialized = False
    shared_attrs = {}
    def __init__(self):
        self._root = None
        self._poll_interval = None
        self._execution_timeout = None
        self._java_binary = None
        self._number_of_problems = None
        self._allowed_languages = []
        self._allowed_ips = []
        self._initialized = True

        self.__dict__ = self.shared_attrs
    
    def __setattr__(self, attr, value):
        if attr[0] == '_':
            # variable is private, so we try finding it in our dict.
            self.__dict__[attr] = value
            return
        
        # try finding the corresponding set_ function for the variable.
        try:
            func = getattr(self, "set_%s" % (attr))
        except AttributeError:
            raise AttributeError("'Settings' object has no attribute '%s'" % (attr))
        
        return func(value)
    
    def __getattr__(self, attr):
        if attr in self.__dict__:
            return self.__dict__[attr]
        
        # If we are trying to find a function that doesn't exist, we raise an exception
        if attr.find('get_') == 0 or attr.find('set_') == 0:
            raise AttributeError
        
        # We try finding a corresponding function for the variable
        try:
            func = getattr(self, "get_%s" % (attr))
        except:
            raise AttributeError("'Settings' object has no attribute '%s'" % (attr))
        
        return func()
    
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
        global _root
        return _root 
    
    def set_root(self, path):
        global _root
        _root = path 
    
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
