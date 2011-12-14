import shutil
from core import VolcanoCmdTask, VolcanoTestTask, TestTaskResult

class SimpleTest(VolcanoCmdTask):
    """ This a simple test that just run the given command with given working directory """
    def __init__(self, name, workdir, command):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = workdir
        self.command = command

class DirCleanup(VolcanoTestTask):
    """ Cleans both the install and build directories """
    def __init__(self, name, config, dir_name):
        VolcanoTestTask.__init__(self, name)
        self.dir_name = dir_name
        
    def runTest(self, observer, config):
        shutil.rmtree(self.dir_name, ignore_errors=True)
        return TestTaskResult.Passed
