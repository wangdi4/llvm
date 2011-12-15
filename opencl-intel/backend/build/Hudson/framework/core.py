"""
Simple test framework
"""
import sys, os.path, platform, re, time, traceback
import cmdtool
from datetime import timedelta
from utils import TimeOutException, EnvironmentValue
import glob
import shutil

# Common definitions
TIMEOUT_DAY = 60*60*24
TIMEOUT_HOUR= 60*60
TIMEOUT_HALFHOUR = 60*30
TIMEOUT_HOURANDHALF = 60*90

class TaskVisitor:
    """Base visitor class for task/suite hierarchy """
    def __init__(self):
        pass
    
    def OnVisitTask(self, task):
        pass
        
    def OnVisitSuite(self, suite):
        pass

class TestTaskResult:
    """Simple enumerator for task execution result"""
    NotRun   = -1 # the task was not run
    Passed   = 0  # the task has succeeded 
    Failed   = 1  # the task has failed
    Skipped  = 2  # the task was skipped intentionally
    TimedOut = 3  # the task has timed out and was terminated

    NameMap = {
        NotRun:   'Not Run',
        Passed:   'Passed',
        Failed:   'Failed',
        Skipped:  'Skipped',
        TimedOut: 'TimedOut'
    }
    
    BooleanMap = {
      True:  Passed,
      False: Failed
    }
    
    @staticmethod 
    def fromBoolean(result):
        return TestTaskResult.BooleanMap[result]
        
    @staticmethod 
    def resultName(result):
        return TestTaskResult.NameMap[result]
    
class VolcanoTestTask:
    """Base task class. All Volcano tests must be inherited from VolcanoTestTask"""
    def __init__(self, name):
        self.name    = name
        self.parent  = None
        self.timeout = -1
        self.generate_report = True
        self.elapsed = 0
        self.result  = TestTaskResult.NotRun
        self.stdout  = ''
        self.stdout_raw = ''
        
    def fullName(self):
        if( self.parent != None ):
            names = [self.parent.fullName(), self.name]
            return '.'.join([str(v) for v in names if v != ''])        
        return self.name

    def isPassed(self):
        return TestTaskResult.Passed == self.result
        
    def startUp(self):
        pass

    def tearDown(self):
        pass

    def runTest(self, observer, config):
        return TestTaskResult.Passed
        
    def run(self, observer, config):
        passed = True
        started = time.time();
        
        try:
            self.startUp()
            if( cmdtool.demo_mode ):
                self.logAndPrint("Demo mode output")
                self.result = TestTaskResult.Passed
            else:
                self.result = self.runTest(observer, config)
        except Exception as e:
            self.result = TestTaskResult.fromBoolean(False)
            self.logAndPrint( '!!! Exception raised during task execution:' + str(e) + "\n" + traceback.format_exc() )
        finally:
            self.tearDown()
            ended  = time.time()
            self.elapsed = ended - started
    
    def onBeforeExecution(self, observer):
        observer.OnBeforeTaskExecution(self)
        
    def onAfterExecution(self, observer):
        observer.OnAfterTaskExecution(self) 
    
    def visit( self, visitor):
        visitor.OnVisitTask(self)
        
    def log(self, str):
        self.stdout += str + '\n'
        self.stdout_raw += str + '\n'
            
    def logAndPrint(self, str):
        self.stdout += str + '\n'
        print str
        

    
class VolcanoCmdTask(VolcanoTestTask):
    """Base task class for executing the single command"""
    def __init__(self, name):
        VolcanoTestTask.__init__(self, name)
        self.workdir = ''
        self.command = ''
        self.success_code = 0
        self.return_code = self.success_code 

    def runTest(self, observer, config):
        if not os.path.exists(self.workdir):
            self.result = TestTaskResult.Failed
            self.return_code = -1
            self.logAndPrint("Command Execution Error: Working directory doesn't exist: '" + self.workdir + "' while executing command: " + self.command)
            return TestTaskResult.Failed 
            
        os.chdir(self.workdir)
        
        cmd = cmdtool.CommandLineTool()
        (retcode, stdoutdata) = cmd.runCommand(self.command, self.timeout)
        self.log(stdoutdata)
        self.return_code = retcode
        
        if( self.return_code == self.success_code ):
            return TestTaskResult.Passed
        if( self.return_code == cmdtool.TIMEOUT_RETCODE):
            return TestTaskResult.TimedOut
        return TestTaskResult.Failed

class VolcanoTestTaskInfo:
    """Internal class. Used to provide task runtime options for the given task"""
    def __init__(self, task, stop_on_failure = False, always_pass = False, skiplist=[]):
        self.task = task
        self.stop_on_failure = stop_on_failure
        self.always_pass = always_pass
        self.skip_configs = skiplist
        
    def isIgnored(self, configMask):
        confstr = ':'.join(configMask)+':'
        for item in self.skip_configs:
            pattern_str = ':'.join(item) + ':.*'
            pattern = re.compile(pattern_str)
            match   = pattern.match(confstr)
            if( match != None):
                if(match):
                    self.task.logAndPrint('Ignored conf(' + confstr + '), pattern (' + pattern_str + ')')
                    return True
        return False;

class VolcanoTestSuite(VolcanoTestTask):
    """Base class for defining the test suite. Test suite is a collection of tasks and optional skip list"""
    def __init__(self, name):
        VolcanoTestTask.__init__(self, name)
        self.tasks = []
        self.generate_children_report = True
        
    def addTask(self, task, stop_on_failure = False, always_pass = False, skiplist=[]):
        self.tasks.append(VolcanoTestTaskInfo(task, stop_on_failure, always_pass, skiplist))
        task.parent = self
        if( self.generate_children_report == False):
            task.generate_report = False

    def insertTask(self, index, task, stop_on_failure = False, always_pass = False, skiplist=[]):
        self.tasks.insert(index, VolcanoTestTaskInfo(task, stop_on_failure, always_pass, skiplist))
        task.parent = self
        if( self.generate_children_report == False):
            task.generate_report = False
      
    def updateTask(self, taskname, stop_on_failure = False, always_pass = False, skiplist=[]):
        for t in self.tasks:
            if( t.task.name == taskname):
                t.stop_on_failure = stop_on_failure
                t.always_pass = always_pass
                t.skip_configs = skiplist
    
    def getTasksNames(self):
        return [ t.task.name for t in self.tasks]

    def minTimeout(self, t1, t2):
        if t1 == -1:
            return t2
        if t2 == -1:
            return t1
        return min(t1, t2)
            
    def run(self, observer, config):
        suitePassed = True
        errmsg      = ''
        started = time.time()
        timeleft = self.timeout
        
        try:
            self.startUp()
            for t in self.tasks:
                if 0 == timeleft:
                    raise TimeOutException()
                
                t.task.onBeforeExecution(observer)

                if not t.isIgnored( config.getConfigMask()):
                    t.task.timeout = self.minTimeout(timeleft, t.task.timeout)                                         
                    t.task.run(observer, config)
                    if timeleft != -1:
                        timeleft = max(0, timeleft - int(t.task.elapsed))
                    taskPassed  = t.task.isPassed() or t.always_pass
                    suitePassed = taskPassed and suitePassed
                else:
                    t.task.result = TestTaskResult.Skipped

                t.task.onAfterExecution(observer)
                self.log( t.task.stdout )
                        
                if not suitePassed and t.stop_on_failure:
                    break
                    
        except TimeOutException:
            self.logAndPrint("!!! Suite '" + self.name + "' has timed out (" + str(self.timeout) + " sec) and was terminated")
            suitePassed = False
        except Exception as e:
            self.logAndPrint('!!! Exception raised during suite execution:' + str(e) + "\n" + traceback.format_exc())
            suitePassed = False
        finally:
            self.tearDown()    
            ended  = time.time()
            self.elapsed = ended - started
            self.result = TestTaskResult.fromBoolean(suitePassed)
        
    def onBeforeExecution(self, observer):
        observer.OnBeforeSuiteExecution(self)
        
    def onAfterExecution(self, observer):
        observer.OnAfterSuiteExecution(self) 
        
    def visit(self, visitor):
        visitor.OnVisitSuite(self)
        for t in self.tasks:
            t.task.visit(visitor)

    
class VolcanoTestRunner:
    """Base class providing basic test task running"""
    def __init__(self):
        pass
    
    def runTask(self, task, config): 
        task.onBeforeExecution(self)       
        task.run(self, config)
        task.onAfterExecution(self)
        return task.isPassed()
    
    def publishResults(self):
        pass

    def OnBeforeTaskExecution(self, task):
        task.logAndPrint('=-----------------------------------------------------------------------------')
        task.logAndPrint( 'Task:' +  task.name)
        if isinstance(task, VolcanoCmdTask):
            task.logAndPrint('Command:' + task.command)
            task.logAndPrint('WorkDir:' + task.workdir)
    
    def OnAfterTaskExecution(self, task):
        t = timedelta(seconds=task.elapsed)
        task.logAndPrint('Task ' + TestTaskResult.resultName(task.result) + ': ' + task.name)
        task.logAndPrint('Elapsed: ' + str(t) )
        
        if TestTaskResult.Failed == task.result:
            if isinstance(task, VolcanoCmdTask):
                task.logAndPrint('Return Code: ' + str(task.return_code) )
        sys.stdout.flush()
            
    def OnBeforeSuiteExecution(self, suite):
        suite.logAndPrint('==============================================================================')
        suite.logAndPrint('Suite:' + suite.name)

    def OnAfterSuiteExecution(self, suite):
        t = timedelta(seconds=suite.elapsed)
        suite.logAndPrint('==============================================================================')
        suite.logAndPrint('Suite ' + TestTaskResult.resultName(suite.result) + ': ' + suite.name)
        suite.logAndPrint('Elapsed: '  + str(t) )



class RunConfig:
    def __init__(self):
        pass
    
    def getConfigMask(self):
        return []
    
