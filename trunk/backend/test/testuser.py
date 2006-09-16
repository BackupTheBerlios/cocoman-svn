import sys
import os
import unittest
# Enable importing from parent directory
relative_path_to_script = os.path.split(sys.argv[0])[0]
relativePathToParent = os.path.join(relative_path_to_script, "..")
sys.path.insert(0, relativePathToParent)
from user import create_user, User
from settings import settings
settings.root = relative_path_to_script

class TestUser(unittest.TestCase):
    def setUp(self):
        self.conf_dir = os.path.join(settings.root, 'manager', 'conf')
        self.users_file = os.path.join(self.conf_dir, 'users.txt')
        os.makedirs(self.conf_dir)
        open(self.users_file, 'w').close() # touch file
    
    def tearDown(self):
        os.remove(self.users_file)
        os.removedirs(self.conf_dir)
    
    def testCreateUser(self):
        bob = create_user('bob')
        assert isinstance(bob, User)

if __name__ == "__main__":
    unittest.main()
