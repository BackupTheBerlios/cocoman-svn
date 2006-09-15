#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


import os
from settings import settings
import logging


class Backend:

    def __init__(self):
        # A list of open registrations. A registration is open from when the 
        # request is read until its done file is processed. Each item is a tuple
       # containing (random chars, time of request).
        self.open_registrations = []
    
    def setup(self):
        pass

    def process_registrations(self):
        """Doesn't return."""
        registration_dir = settings.root + 'temp_web'
        
        while True:
            try:
                dir_contents = os.listdir(registration_dir)
            except OSError, e:
                logging.error("There was an error reading the 'temp_web' " \
                              "directory (%s). The error was: %s" % \
                              (registration_dir, e))
            for entry in dir_contents:
                # New requests
                if entry.startswith(request_prefix):
                    self._process_registration_request(entry)
                
                # Cleanup - TODO
                for open_request in open_requests:
                    pass
                    # if find done file with those chars
                    # delete all 3 prefixes with those chars
                    # remove from open list
                    # else
                    # if too old, log it
    
    def process_submissions(self):
        """Doesn't return."""
        pass

    def _process_registration_request(self, request_filename):        
        def RegistrationError(Exception):
            def __init__(self, status_code, message, random_chars, user_id=0):
                self.status_code = status_code
                self.message = message
                self.random_chars = random_chars
                self.user_id = user_id
        
        request_prefix = 'registration_request-'
        try:
            logging.debug("Processing registration request file '%s'." %
                          request_filename)
            random_chars = request_filename[len(request_prefix):]
            
            try:
                request_file = open(request_filename)
                request_line = request_file.readline()
                request_file.close()
            except IOError, e:
                logging.error("There was an error reading registration"
                              " request file '%s'. The error was: %s. "
                              % (request_filename, e))
                raise RegistrationError(10, "Configuration error", random_chars)
            request = request_line.strip().split(',', 2)
            if len(request) < 2:
                logging.error("Registration request file '%s' doesn't "
                              "have enough sections. Skipping this "
                              "file." % request_filename)
                raise RegistrationError(11, "Registration request error", random_chars)
            if len(request) > 2:
                logging.warn("Registration request file '%s' has too "
                              "many sections. Ignoring the extra ones."
                              % request_filename)
            
            # TODO Validate IP address
            try:
                new_user = user.create_user(request[0])
                new_user.ip = request[1]
            except user.InvalidName, e:
                logging.error("Registration request file '%s' contained"
                              " an invalid name (%s). Skipping this "
                              "file." % (request_filename, e))
                raise RegistrationError(12, "Invalid name", random_chars)
            except IOError, e:
                logging.error("There was an error creating user '%s': "
                              "%s." % (request[0], e))
                raise RegistrationError(13, "Could not create user", random_chars)
                
            logging.info("Created user %s." % new_user)
            
            self.open_registrations.append((random_chars, '0')) # TODO time
            
            try:
                os.remove(request_filename)
            except OSError, e:
                logging.error("There was an error removing the "
                              "registration request file '%s'. The "
                              "error was: %s." % (request_filename, e))
            
            self._write_registration_status_file(0, "Registration successful",
                                                 random_chars, new_user.id)
        except RegistrationError, e:
            self._write_registration_status_file(e.status_code, e.message,
                                                 e.random_chars, e.user_id)
    
    def _write_registration_status_file(self, status_code, message, 
                                        random_chars, user_id):
        try:
            # TODO Add check for file already existing
            status_file_name = 'registration_status-%s' % \
                               random_chars
            status_file = open(status_file_name, 'w')
            status_file.write("%s,%s,%s,%s" % (status_code, message, 
                                               random_chars, user_id))
            status_file.close()
        except IOError, e:
            logging.error("There was an error writing status "
                          "file '%s' for user %s. The error was: %s."
                          % (status_file_name, user_id, e))
