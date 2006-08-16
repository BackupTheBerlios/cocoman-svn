#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


from settings import Settings
import os

debug_on = False
settings = Settings()

def log1(message):
    """Writes a message to the user log file (cocoman.log). If debug_on is True
    the message will be prefixed with 'log1:   ' so the content lines up with the
    debug messages.
    """
    create_log_dir_if_missing()
    log_file_name = os.path.join(settings.root, 'log')
    log_file_name = os.path.join(log_file_name, 'cocoman.log')
    log_file = open(log_file_name, "a")
    if debug_on:
        message = 'log1:   ' + message + '\n'
    else:
        message = message + '\n'
    log_file.write(message)
    log_file.close()

def debug1(message):
    """If debug_on is True, writes lines to the user log file (cocoman.log) 
    prefixed with 'debug1: '. If debug_on is False, does nothing.
    """
    if debug_on:
        create_log_dir_if_missing()
        log_file_name = os.path.join(settings.root, 'log')
        log_file_name = os.path.join(log_file_name, 'cocoman.log')
        log_file = open(log_file_name, "a")
        log_file.write('debug1: ' + message + '\n')
        log_file.close()

def log_build(source_file_name, message):
    create_log_dir_if_missing()
    compile_log_dir = os.path.join(settings.root, 'log')
    compile_log_dir = os.path.join(compile_log_dir, 'compiles')
    if not os.path.exists(compile_log_dir):
        os.mkdir(compile_log_dir)
    assert(type(source_file_name) == str)
    assert(len(source_file_name) > 0)
    last_period_index = source_file_name.rfind('.') # -1 if not found
    base_name = source_file_name[0:last_period_index]
    log_file_name = os.path.join(compile_log_dir, base_name + '.log')
    assert(not os.path.exists(log_file_name))
    log_file = open(log_file_name, "w")
    log_file.write(message + '\n')
    log_file.close()

def log_to_frontend(message):
    create_log_dir_if_missing()
    log_file_name = os.path.join(settings.root, 'log')
    log_file_name = os.path.join(log_file_name, 'log')
    log_file = open(log_file_name, "a")
    log_file.write(message + '\n')
    log_file.close()

def create_log_dir_if_missing():
    log_dir = os.path.join(settings.root, 'log')
    if not os.path.exists(log_dir):
        os.mkdir(log_dir)
