#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


# TODO Figure out a better way to share these amongst instances
_root = None
_poll_interval = None
_execution_timeout = None
_java_binary = None
_number_of_problems = None
_allowed_languages = []
_allowed_ips = []
_initialized = True

class Settings:
    """Use the singleton pattern - all users of the class must be referring to 
    the same instance.
    """
    __initialized = False
    
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
        pass
    
    def save(self, file_name=None):
        pass
    
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

    def get_allowed_lanuguages(self):
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
    s.root = 'test'
    print s.root
#    s.test = 'blah'
    s.g
