
import os
import platform
import re
import sys
import logging

class TestSuiteNotFoundException(Exception): pass

class GDBContinueError(Exception): pass

class StopReason(object):
    """ The GDB client stops for one of these reasons."""
    NOT_ATTACHED=0
    NOT_RUNNING=1
    BREAKPOINT_HIT=2
    PROGRAM_EXITED=3
    TIMED_OUT=4
    GDB_CRASHED=5

def os_is_windows():
    return platform.system() == 'Windows'

def find_on_path(file, path, delim=":"):
    """ Returns the first valid absolute path to file on path """

    for pathcmp in path.split(":"):
        candidate = os.path.join(pathcmp, file)
        if os.path.exists(candidate):
            return os.path.abspath(candidate)
    return None
def loglevel(lev):
    """ Setting log level """
    logging.basicConfig(stream=sys.stderr, level=lev)

def logi(message):
    """ Log informational message """
    logging.info(message)

def loge(message):
    """ Log error message """
    logging.error(message)

def logw(message):
    """ Log a warning message """
    logging.warning(message)

def logd(message):
    """ Log debug message """
    logging.debug(message)

class TestClient(object):
    def __init__(self, cl_dir_path):
        self.cl_dir_path = cl_dir_path

    def cl_abs_filename(self, cl_name):
        """ Make the canonical absolute path from the name of a cl_file.
        """
        path = os.path.abspath(os.path.join(self.cl_dir_path, cl_name))
        path = re.sub(r'\\', '/', path)
        if os_is_windows():
            return path.lower()
        else:
            return path
