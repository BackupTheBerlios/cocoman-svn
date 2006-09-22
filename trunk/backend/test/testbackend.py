import sys
import os
import unittest
# Enable importing from parent directory
relative_path_to_script = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relative_path_to_script, "..")
sys.path.insert(0, relativePathToParent)
from settings import settings
settings.root = os.path.join(relative_path_to_script, 'test_root')
from backend import Backend
import time
from threading import Thread
import shutil
from subprocess import Popen, PIPE, STDOUT

class TestRegistration(unittest.TestCase):
    def setUp(self):
        self.conf_dir = os.path.join(settings.root, 'manager', 'conf')
        self.users_file = os.path.join(self.conf_dir, 'users.txt')
        self.temp_web_dir = os.path.join(settings.root, 'temp_web')
        os.makedirs(self.conf_dir)
        os.makedirs(self.temp_web_dir)
        open(self.users_file, 'w').close() # touch file
        backend_filename = os.path.join(relativePathToParent, 'backend.py')
        command = ['python', backend_filename, 'registration', '-r', settings.root, '-d']
        self.backend = Popen(command, stdin=PIPE, stdout=PIPE, 
                                    stderr=STDOUT, close_fds=True)    
    
    def tearDown(self):
        os.kill(self.backend.pid, 9)
        shutil.rmtree(settings.root)
    
    def testNormalRegistration(self):
        random_chars = 'ahc8kncuao0'
        self._writeRequest(random_chars, 'Pete', '192.168.1.1')
        time.sleep(1.1)
        reply = self._readStatus(random_chars)
        self.assertEqual(reply[0], '0')
        self.assertEqual(reply[1], 'Registration successful')
        assert reply[2].isdigit()
        self._writeDone(random_chars)
        time.sleep(1.1)
        list_of_files = os.listdir(self.temp_web_dir)
        self.assertEqual(len(list_of_files), 0)
    
    def _writeRequest(self, random_chars, name, ip_address):
        request_file_name = 'registration_request-%s' % random_chars
        request_file_name = os.path.join(self.temp_web_dir, request_file_name)
        request_file = open(request_file_name, 'w')
        request_file.write('%s,%s\n' % (name, ip_address))
        request_file.close()
    
    def _readStatus(self, random_chars):
        status_file_name = os.path.join(self.temp_web_dir, 
                                        'registration_status-%s' % random_chars)
        status_file = open(status_file_name)
        status_line = status_file.readline()
        status_file.close()
        reply = status_line.strip('\n').split(',')
        self.assertEqual(len(reply), 3)
        return reply
    
    def _writeDone(self, random_chars):
        done_file_name = os.path.join(self.temp_web_dir,
                                        'registration_done-%s' % random_chars)
        done_file = open(done_file_name, 'w')
        done_file.close()

if __name__ == "__main__":
    unittest.main()
