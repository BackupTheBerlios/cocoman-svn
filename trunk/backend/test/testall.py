"""This script searched through all the modules in this directory for classes
that inherits from TestCase, compiles a test suite of all of them, and runs the 
suite.

"""

import sys
import os
# Enable importing from parent directory
relative_path_to_script = os.path.split(sys.argv[0])[0]
sys.path.append(relative_path_to_script)
import unittest

if __name__ == "__main__":
    suite = unittest.TestSuite()
    dir_contents = os.listdir(os.path.join(relative_path_to_script, '.'))
    for entry in dir_contents:
        entry_with_path = os.path.join(relative_path_to_script, entry)
        if os.path.isfile(entry_with_path) and entry.rsplit('.')[-1].lower() == 'py':
            try:
                module = __import__(entry[:-3])
            except ImportError:
                pass
            for attribute in dir(module):
                try:
                    if issubclass(module.__dict__[attribute], unittest.TestCase):
                        print "Adding %s to test suite." % attribute
                        suite.addTest(unittest.makeSuite(module.__dict__[attribute]))
                except TypeError:
                    pass
    runner = unittest.TextTestRunner()
    runner.run(suite)
