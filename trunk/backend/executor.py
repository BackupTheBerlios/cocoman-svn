#                       executor.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2
# Concept for keeping a program from running over timeout length.

import os
import signal
import subprocess
import threading
import time

class TimeOutException(Exception):
    pass

def kill_after_timeout(proc, timeout):
    while timeout > 0:
        time.sleep(1)
        if not proc.returncode is None:
            return
        timeout -= 1
    #TODO: use the windows kill API if we are not on *nix or mac. 
    os.kill(proc.pid, signal.SIGKILL)
    proc.timeout = True
    
def execute(binary, data=None, timeout=3):
    subproc = subprocess.Popen(binary, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    subproc.timeout = False

    # write data into the process
    # TODO: should we start the timeout timer before or after data is written to pipe?

    # can raise an IOError if the process finishes before we hand it any data,
    # keep that in mind.
    if data:
        data += "\000" # write the EOF character, just in case.
        subproc.stdin.write(data)
    
    # start the kill timer.
    killthread = threading.Thread(target=kill_after_timeout, args=[subproc, timeout])
    killthread.start()

    if not subproc.returncode:
        subproc.wait()
    
    if subproc.timeout:
        raise TimeOutException("Program '%s' timed out after %s seconds" % (binary, timeout))
    
    output = subproc.stdout.read()
    return output

if __name__ == "__main__":
    try:
        execute("true", timeout=5)
    except TimeOutException, e:
        print e
    except IOError, e:
        print "Process finished execution before we could hand it data"

    try:
        execute("yes", timeout=3)
    except TimeOutException, e:
        print e
    except IOError, e:
        print "Process finished execution before we could hand it data"
