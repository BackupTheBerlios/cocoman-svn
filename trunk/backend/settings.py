#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


class Settings:
    """Use the singleton pattern - all users of the class must be referring to 
    the same instance.
    """

    def __init__(self):
        """Class constructor"""
        # these are just temporary variable names, taken from the function names
        self.__root = None
        self.__poll_interval = None
        self.__execution_timeout = None
        self.__java_binary = None
        self.__number_of_problems = None
        self.__allowed_languages = []
        self.__allowed_ips = []


    def load(self, file_name):
        pass

    def save(self, file_name=None):
        pass

    def get_root(self):
        pass

    def set_root(self, path):
        pass

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
