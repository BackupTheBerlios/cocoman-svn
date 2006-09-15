#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


class InvalidUserIdError(Exception):
    pass

class InvalidNameError(Exception):
    pass


def create_user(name):
    pass

class User:
    """This class is an abstraction of an entry in the users file. The file will 
    be read on any 'get' call and the most recent value returned. If the users 
    file doesn't contain an entry with the instance's user id on instantiation 
    or any time a get method is called, an InvalidUserIdError is thrown.
    """
    def __init__(self, user_id):
        pass

    def get_id(self):
        pass

    def get_name(self):
        pass



