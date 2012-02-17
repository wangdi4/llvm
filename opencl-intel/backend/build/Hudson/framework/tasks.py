import os,shutil
from core import VolcanoCmdTask, VolcanoTestSuite, VolcanoTestTask, TestTaskResult
from cmdtool import CommandLineTool, TIMEOUT_RETCODE 

class SimpleTest(VolcanoCmdTask):
    """ This a simple test that just run the given command from a given working directory """
    def __init__(self, name, workdir, command):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = workdir
        self.command = command

class DirCleanup(VolcanoTestTask):
    """ Cleans the given directory """
    def __init__(self, name, config, dir_name):
        VolcanoTestTask.__init__(self, name)
        self.dir_name = dir_name
        
    def runTest(self, observer, config):
        shutil.rmtree(self.dir_name, ignore_errors=True)
        return TestTaskResult.Passed

class GoogleTestSuite(VolcanoTestSuite):
    """
        Defines the test suite for google test based tests.
        During the startup the suite will determine the list of the google tests and 
        add each one of them as a separate task
    """
    def __init__(self, name, command, workdir, config):
        VolcanoTestSuite.__init__(self, name)
        self.command = command
        self.workdir = workdir
        self.config  = config
        self.default_success_code = 0

    def parseTests(self, testlist):
        lines = testlist.splitlines()
        suite = ''
        for line in lines:
            name = line.strip()
            if(name == ''):
                #empty line goes after the test list. we ignore all that comes after it
                break
            if(name.endswith('.')):
                #lines which ends with '.'(dot) represents the test suites names
                suite = name
            else:
                skiplist = []
                testname = name
                #detect the disabled tests and mark them skipped
                if( name.startswith('DISABLED_')):
                    skiplist = [['.*']]
                    testname = name.replace('DISABLED_','',1)
                filter = suite + name
                test = SimpleTest(self.getTestName(testname), self.workdir,  self.command + ' --gtest_filter=' + filter)
                test.success_code = self.default_success_code
                self.addTask(test,skiplist=skiplist)
    
    def getTestName(self, name):
        tname = name
        count = 1
        while tname in self.getTasksNames():
            tname = '_'.join([name, str(count)])
            count += 1
        return tname
        
        
    def startUp(self):
        if not os.path.exists(self.workdir):
            self.result = TestTaskResult.Failed
            self.logAndPrint("Command Execution Error: Working directory doesn't exist: '" + self.workdir + "' while executing command: " + self.command)
            
        os.chdir(self.workdir)
        
        cmd = CommandLineTool()
        cmd.print_output = False
        (retcode, stdoutdata) = cmd.runCommand(self.command + ' --gtest_list_tests', self.timeout)
        
        if( self.default_success_code == retcode):
            self.result = TestTaskResult.Passed
            self.parseTests(stdoutdata)
        elif( TIMEOUT_RETCODE == retcode):
            self.result = TestTaskResult.TimedOut
        else:
            self.result = TestTaskResult.Failed
