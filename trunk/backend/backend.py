#!/usr/bin/python
#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


from optparse import OptionParser
from settings import settings
import logging
import os
import shutil
import sys
import time
import user


class Backend:

    def __init__(self):
        # process_registrations initializers
        self.open_registrations = []
        self.registration_dir = os.path.join(settings.root, 'temp_web')
        self.request_prefix = 'registration_request-'

        # process_submissions initializers
        self.submission_dir = os.path.join(settings.root, 'temp_web')
        self.submission_prefix = 'submission-'
        self.__processed__submissions = {} # A dictionary from userid->problemNumber->list of submissions
        self.__queued_submissions = []
        pass
    
    def process_registrations(self):
        # A list of open registrations. A registration is open from when the 
        # request is read until its done file is processed. Each item is a tuple
        # containing (random chars, time of request).
        
        try:
            dir_contents = os.listdir(self.registration_dir)
        except OSError, e:
            logging.error("There was an error reading the 'temp_web' " \
                          "directory (%s). The error was: %s" % \
                          (self.registration_dir, e))
            dir_contents = []

        for entry in dir_contents:
            # New requests
            if entry.startswith(self.request_prefix):
                registration_file = os.path.join(self.registration_dir, entry)
                self._process_registration_request(registration_file)
            
            # Cleanup
            done_registrations = []
            for open_request in self.open_registrations:
                done_file_name = 'registration_done-%s' % open_request[0]
                done_file_name = os.path.join(self.registration_dir, done_file_name)
                if os.path.exists(done_file_name):
                    request_file_name = 'registration_request-%s' % open_request[0]
                    request_file_name = os.path.join(self.registration_dir, request_file_name)
                    status_file_name = 'registration_status-%s' % open_request[0]
                    status_file_name = os.path.join(self.registration_dir, status_file_name)
                    try:
                        os.remove(request_file_name)
                    except OSError, e:
                        pass # TODO Verify errno is 2
                    try:
                        os.remove(status_file_name)
                    except OSError, e:
                        logging.info("There was an error removing a done "
                                     "status file: %s" % e)
                    try:
                        os.remove(done_file_name)
                    except OSError, e:
                        logging.error("There was an error removing a "
                                      "completed request done file: %s" % e)
                    done_registrations.append(open_request)
                    number_open_requests = len(self.open_registrations) - \
                                           len(done_registrations)
                    logging.debug("Cleaned up after request %s. Open "
                                  "requests: %s" % (open_request[0],
                                                    number_open_requests))
                # TODO
                # else
                # if too old, log it
            # There's probably a better way to remove entries from 
            # open_registration, but this works for now
            for done_entry in done_registrations:
                self.open_registrations.remove(done_entry)
                

    def _process_registration_request(self, request_filename):        
        class RegistrationError(Exception):
            def __init__(self, status_code, message, random_chars, user_id=0):
                Exception.__init__(self)
                self.status_code = status_code
                self.message = message
                self.random_chars = random_chars
                self.user_id = user_id
        
        try:
            logging.debug("Processing registration request file '%s'." %
                          request_filename)
            only_file = os.path.split(request_filename)[1]
            random_chars = only_file[len(self.request_prefix):]
            logging.debug('random chars: %s' % random_chars)
            
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
                raise RegistrationError(11, "Registration request error", 
                                        random_chars)
            if len(request) > 2:
                logging.warn("Registration request file '%s' has too "
                              "many sections. Ignoring the extra ones."
                              % request_filename)
            
            # TODO Validate IP address
            try:
                new_user = user.create_user(request[0])
                new_user.ip = request[1]
            except user.InvalidNameError, e:
                logging.error("Registration request file '%s' contained"
                              " an invalid name (%s). Skipping this "
                              "file." % (request_filename, e))
                raise RegistrationError(12, "Invalid name", random_chars)
            except IOError, e:
                logging.error("There was an error creating user '%s': "
                              "%s." % (request[0], e))
                raise RegistrationError(13, "Could not create user", 
                                        random_chars)
                
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
            status_file_name = os.path.join(self.registration_dir, 
                                            status_file_name)
            status_file = open(status_file_name, 'w')
            status_file.write("%s,%s,%s\n" % (status_code, message, user_id))
            status_file.close()
        except IOError, e:
            logging.error("There was an error writing status "
                          "file '%s' for user %s. The error was: %s."
                          % (status_file_name, user_id, e))

    def process_submissions(self):
        """
        Empties temp_web of files that were submitted in the format
        submission-<user id>-<time>-Problem<Num>-randomchars.ext and places
        them in the queue, as well in their respective directories in
        submissions/
        """

        try:
            dir_contents = os.listdir(self.submission_dir)
        except OSError, e:
            logging.error("There was an error reading the 'temp_web' " \
                          "directory (%s). The error was: %s" % \
                          (self.submission_dir, e))
            dir_contents = []
        for entry in dir_contents:
            if entry.startswith(self.submission_prefix):
                submission_file = os.path.join(self.submission_dir, entry)
                self._process_submission_file(submission_file)
    
    def _process_submission_file(self, submission_file):
        """
        Processes a submission file and make sure it is a valid submission.
        Format of the filename is submission-<userid>-<time>-Problem<num>-randomchars.ext
        """
        print submission_file
        basename, ext = os.path.splitext(os.path.basename(submission_file))
        split_name = basename.split('-')
        userid = split_name[1]
        submission_time = split_name[2]
        problem_no = int(split_name[3][7:])
        random_chars = split_name[4]

        logging.debug('random chars: %s' % random_chars)

        valid_file = True
        submission_name = "%s-%s%s" % (userid, submission_time, ext)

#        if ext not in settings.valid_extensions: # Need to figure out whether valid extensions are around.
#            logging.error("%s has an invalid extension of %s" % (submission_file, ext)) 
#            valid_file = False
#        elif userid in valid_userids: # Need to figure out whether userids are around.
#            logging.error("%s was submitted by an invalid userid, %s" % (submission_file, userid))
#            valid_file = False
#        elif problem_no > 0 and problem_no <= settings.number_of_problems:
#            logging.error("%s was submitted as a solution to an invalid problem" % (submission_file))
#            valid_file = False
        
        if valid_file:
            submission_placement = os.path.join("%s/submissions/Problem%s/%s" \
                    % (settings.root, problem_no, submission_name))
            shutil.move(submission_file, submission_placement)
            os.chmod(submission_placement, 400)
            self.__queued_submissions.append(submission_placement)
        else:
            submission_placement = os.path.join("%s/submissions/invalid/" % settings.root)
            shutil.move(submission_file, submission_placement)



        
        pass

def processModes(modes):
    while True:
        for mode in modes:
            mode()
        time.sleep(1)

if __name__ == '__main__':
    usage = "Usage: %%prog [options] <mode>\nType '%s --help' for help." % sys.argv[0]
    parser = OptionParser(usage)
    parser.add_option("-r", "--root", dest="root", default=".",
                      help="Specify the root of the cocoman installation "
                           "(defaults to the current directory)")
    parser.add_option("-d", "--debug", action="store_true", dest="debug", 
                      default=False, help="Enable debug mode")
    (options, args) = parser.parse_args()
    settings.root = options.root
    if options.debug:
        logging.basicConfig(level=logging.DEBUG,
                            format='%(levelname)-8s %(message)s')
    if len(args) == 0:
        parser.error('You must specify a mode.')
    
    app = Backend() # had to move it over here, because intialization takes place after root is set.

    modes = {'registration': app.process_registrations,
             'submission': app.process_submissions,
        }
    legal_modes = []
    for arg in args:
        try:
            legal_modes.append(modes[arg])
        except KeyError, e:
            parser.error('You specified an illegal mode.')

    processModes(legal_modes)
    assert False, "Execution should never reach here."
