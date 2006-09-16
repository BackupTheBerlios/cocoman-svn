#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


import os
from settings import settings
import random
random.seed()
import logging


class UserError(Exception):
    def __init__(self, message):
        self.message = message
    
    def __str__(self):
        return self.message
    
class UserCreationError(UserError):
    def __init__(self, message=None):
        UserError.__init__(self, message or 'Could not create user')

class InvalidNameError(UserCreationError):
    # TODO Add reason
    pass

class InvalidUserIdError(UserError):
    def __init__(self, id=None, message=None):
        if message is None:
            message = 'Invalid user id'
            if id is not None:
                message = message + ': %s' % id
        UserError.__init__(self, message)


def create_user(name):
    """Creates a new User. Writes an entry in users.txt for the new user. 
    Exceptions:
    If the specified name contains any characters other than letters, numbers, 
    or spaces, raises an InvalidNameError.
    If another user exists with this name, raises a TODO.
    
    """
    # TODO Validate name
    
    try:
        users_file = None
        users_file = open(User.users_file)
    except IOError, e:
        logging.critical("There was an error opening the users file (%s). The "
                      "error is: %s." % (User.users_file, e))
        raise UserCreationError()
    try:
        lines = users_file.readlines()
    except IOError, e:
        logging.critical("There was an error reading the users file (%s). The "
                      "error is: %s." % (User.users_file, e))
        raise UserCreationError()
    users_file.close()
    
    ids = []
    names = []
    for line in lines:
        entry = line.split(':')
        if len(entry) != 2:
            logging.error("There is an invalid entry in the users file: %s. "
                      "Skipping this entry." % line)
        ids.append(entry[0])
        names.append(entry[1])
    if name in names:
        raise NameAlreadyExistsError()
    id = random.randint(1000, 9999)
    while str(id) in ids:
        id = random.randint(1000, 9999)
    try:
        file = open(User.users_file, 'a')
        file.write("%s:%s\n" % (id, name))
    except IOError, e:
        logging.critical("There was an error opening the users file (%s). The "
                      "error is: %s." % (User.users_file, e))
    file.close()
    return User(id)


class User(object):
    """This class is an abstraction of an entry in the users file. The file will 
    be read on any 'get' call and the most recent value returned. If the users 
    file doesn't contain an entry with the instance's user id on instantiation 
    or any time a get method is called, an InvalidUserIdError is thrown.
    """
    
    users_file = os.path.join(settings.root, 'manager', 'conf', 'users.txt')
    
    def __init__(self, user_id):
        self._id = user_id
        self.__read_my_entry()
    
    def __read_my_entry(self):
        try:
            file = open(self.users_file)
        except IOError, e:
            logging.critical("There was an error opening the users file (%s). The "
                          "error is: %s." % (User.users_file, e))
            raise UserError('Error opening users file')
        try:
            lines = file.readlines()
        except IOError, e:
            logging.critical("There was an error opening the users file (%s). The "
                          "error is: %s." % (User.users_file, e))
            raise UserError('Error reading users file')
        file.close()
        for line in lines:
            entry = line.split(':')
            if len(entry) != 2:
                logging.error("There is an invalid entry in the users file: %s. "
                          "Skipping this entry." % line)
                continue
            if int(entry[0]) == self._id:
                self._name = entry[1].strip('\n')
                return
        raise InvalidUserIdError(user_id)
    
    def get_id(self):
        return self._id
    
    def get_name(self):
        self.__read_my_entry()
        return self._name
    
    def set_name(self, name):
        raise NotImplementedError()
    
    id = property(get_id)
    name = property(get_name, set_name)
