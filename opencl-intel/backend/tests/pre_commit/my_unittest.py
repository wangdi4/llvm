import unittest, threading

def survival_of_fitness(p_handle, name):
    try:
        p_handle.kill()
        p_handle.returncode = -1
        p_handle.kill()
    except:
        pass

class CommandLineTool:
    def __init__(self, default_timeout):
        self.default_timeout = default_timeout

    def runCommand(self, command, timeout=-1):
        """Returns errorcode, output"""
        if -1==timeout:
            timeout=self.default_timeout

        import subprocess
        
        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        t = threading.Timer(timeout, survival_of_fitness, [proc, 'foo'])
        t.start()
        out, err = proc.communicate()
        t.cancel()
        out = command + out
        return proc.returncode, out

class MyTestCase(unittest.TestCase):
    def __init__(self, methodName, UUT_bin, default_timeout):        
        import platform, os.path
        if not os.path.exists(UUT_bin):
            raise IOError("UUT binary not found:" + UUT_bin)
        unittest.TestCase.__init__(self, methodName)
        self.CLT = CommandLineTool(default_timeout)
        self.UUT_bin = UUT_bin
        self.default_timeout = default_timeout
