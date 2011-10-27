"""
Simple test framework
"""
import sys, os.path, platform, re, time, traceback
from datetime import timedelta
from Volcano_CmdUtils import CommandLineTool
import glob
import shutil

#Volcano specific global default paths
SAMBA_SERVER = '//ismb014.iil.intel.com'
DX_PERFORMANCE_SHADERS_ROOT = '/nfs/iil/disks/cvcc/testbase/Shaders/DX/root/Performance/NoDcls/PerformanceCriticalShaders'
DX_10_SHADERS_ROOT          = '/nfs/iil/disks/cvcc/testbase/Shaders/DX/root'
PERFORMANCE_LOG_ROOT        = '/nfs/iil/disks/cvcc/vdovleka/logs/Volcano/Performance'
PERFORMANCE_TESTS_ROOT      = '/Volcano/Performance/Tests'
OCL_CONFORMANCE_TESTS_ROOT  = '/nfs/iil/disks/cvcc/vdovleka/tests/Volcano/Conformance'
DEFAULT_WORLOADS_ROOT       = '/nfs/iil/disks/cvcc/OclConformance_14756/trunk/ReleaseCriteria/'
DEFAULT_VS_VERSION          = 9
DEFAULT_VOLCANO_SOLUTION    = 'Backend.sln'
DEFAULT_OCL_SOLUTION        = 'OCL.sln'

if platform.system() == 'Windows':
    DX_PERFORMANCE_SHADERS_ROOT = SAMBA_SERVER + DX_PERFORMANCE_SHADERS_ROOT
    DX_10_SHADERS_ROOT          = SAMBA_SERVER + DX_10_SHADERS_ROOT
    PERFORMANCE_LOG_ROOT        = SAMBA_SERVER + PERFORMANCE_LOG_ROOT
    OCL_CONFORMANCE_TESTS_ROOT  = SAMBA_SERVER + OCL_CONFORMANCE_TESTS_ROOT
    DEFAULT_WORLOADS_ROOT       = SAMBA_SERVER + DEFAULT_WORLOADS_ROOT

TIMEOUT_DAY = 60*60*24
TIMEOUT_HOUR= 60*60
TIMEOUT_HALFHOUR = 60*30
TIMEOUT_HOURANDHALF = 60*90

SUPPORTED_CPUS = ['auto', 'corei7', 'sandybridge']
SUPPORTED_TARGETS = ['Win32', 'Win64', 'Linux64']
SUPPORTED_BUILDS = ['Release', 'Debug']
SUPPORTED_VECTOR_SIZES = ['0', '1', '4', '8', '16']

TARGETS_MAP = { 
                'Win32'  : ['Windows', 32],
                'Win64'  : ['Windows', 64],
                'Linux64': ['Linux', 64] 
              }

class TimeOutException(Exception):
    "Custom timeout exception. Usually raised on task or suite timeouts"
    pass

class EnvironmentValue:
    """
       Simple class to store named environment value. 
       The main usage of this class is to simplify setting the environment variable,
       and then being able to restore its state back 
    """
    def __init__(self, envname):
        self.envname = envname
        if self.envname in os.environ: 
            self.oldvalue = os.environ[self.envname]
            self.value_exist = True
        else:
            self.value_exist = False
    
    def isExisted(self):
        return self.value_exist
    
    def getOldValue(self):
        if self.isExisted():
            return os.environ[self.envname]
        return ''
    
    def setValue(self, value):
        os.environ[self.envname] = value
    
    def appendToFront(self, value, delim = ''):
        if delim == '':
            newVal = value + self.getOldValue()
        elif self.getOldValue() != '':
            newVal = value + delim + self.getOldValue()
        else:
            newVal = value
        self.setValue(newVal)

    def appendToBack(self, value, delim = ''):
        if delim == '':
            newVal = self.getOldValue() + value
        elif self.getOldValue() != '':
            newVal = self.getOldValue() + delim + value  
        else:
            newVal = value
        self.setValue(newVal)
    
    def restoreValue(self):
        if(self.value_exist):
            os.environ[self.envname] = self.oldvalue
        else:
            os.environ.pop(self.envname)

class VolcanoTestTask:
    """Base task class. All Volcano tests must be inherited from VolcanoTestTask"""
    def __init__(self, name):
        self.name = name
        self.parent  = None
        self.timeout = -1
        self.generate_report = True
        self.elapsed = 0
        
    def fullName(self):
        if( self.parent != None ):
            names = [self.parent.fullName(), self.name]
            return '.'.join([str(v) for v in names if v != ''])        
        return self.name

    def startUp(self):
        pass

    def tearDown(self):
        pass

    def runTest(self, observer, config):
        return (True, "")
        
    def run(self, observer, config):
        passed = True
        started = time.time();
        
        try:
            self.startUp()
            (passed, stdoutdata) = self.runTest(observer, config)
        except Exception as e:
            errmsg = '!!! Exception raised during task execution:' + str(e) + "\n" + traceback.format_exc()
            print errmsg
            return (False, errmsg)
        finally:
            self.tearDown()
            ended  = time.time()
            self.elapsed = ended - started
            
        return passed, stdoutdata
    
    def onBeforeExecution(self, observer):
        observer.OnBeforeTaskExecution(self)
        
    def onAfterExecution(self, observer, result, output):
        observer.OnAfterTaskExecution(self, result, output) 
            
class VolcanoCmdTask(VolcanoTestTask):
    """Base task class for executing the single command"""
    def __init__(self, name):
        VolcanoTestTask.__init__(self, name)
        self.name = name
        self.workdir = ''
        self.command = ''
        self.success_code = 0
        self.return_code = self.success_code 

    def runTest(self, observer, config):
        passed = True
        if not os.path.exists(self.workdir):
            errmsg = "Command Execution Error: Working directory doesn't exist: '" + self.workdir + "' while executing command: " + self.command
            return (False, errmsg)
            
        os.chdir(self.workdir)
        
        cmd = CommandLineTool()
        (retcode, stdoutdata) = cmd.runCommand(self.command, self.timeout)
        self.return_code = retcode
        passed = self.return_code == self.success_code and passed
            
        return passed, stdoutdata

class VolcanoTestTaskInfo:
    """Internal class. Used to provide task runtime options for the given task"""
    def __init__(self, task, stop_on_failure = False, always_pass = False, skiplist=[]):
        self.task = task
        self.stop_on_failure = stop_on_failure
        self.always_pass = always_pass
        self.skip_configs = skiplist
        
    def isIgnored(self, config):
        confstr = ':'.join(config)+':'
        for item in self.skip_configs:
            pattern_str = ':'.join(item) + ':.*'
            pattern = re.compile(pattern_str)
            match   = pattern.match(confstr)
            if( match != None):
                if(match):
                    print 'Ignored conf(' + confstr + '), pattern (' + pattern_str + ')'
                    return True
        return False;

class VolcanoTestSuite(VolcanoTestTask):
    """Base class for defining the test suite. Test suite is a collection of tasks and optional skip list"""
    def __init__(self, name):
        VolcanoTestTask.__init__(self, name)
        self.name = name
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
        suiteOutput = ''
        errmsg      = ''
        started = time.time()
        timeleft = self.timeout
        
        
        try:
            self.startUp()
            for t in self.tasks:
                if 0 == timeleft:
                    raise TimeOutException()
                
                t.task.onBeforeExecution(observer)
    
                if not t.isIgnored([config.cpu, config.target_type, config.build_type, config.target_type, config.transpose_size]):
                    t.task.timeout = self.minTimeout(timeleft, t.task.timeout)                                         
                    taskPassed, stdoutdata  = t.task.run(observer, config)
                    if timeleft != -1:
                        timeleft = max(0, timeleft - int(t.task.elapsed))
                    taskPassed  = taskPassed or t.always_pass
                    suitePassed = taskPassed and suitePassed
                    suiteOutput += stdoutdata
                    
                    if( taskPassed ):
                        t.task.onAfterExecution(observer, TestTaskResult.Passed, stdoutdata)
                    else:
                        t.task.onAfterExecution(observer, TestTaskResult.Failed, stdoutdata)
                else:
                    t.task.onAfterExecution(observer, TestTaskResult.Skipped, '')
                    
                if not suitePassed and t.stop_on_failure:
                    break
        except TimeOutException:
            errmsg = "!!! Suite '" + self.name + "' has timed out (" + str(self.timeout) + " sec) and was terminated"
            suitePassed = False
        except Exception as e:
            errmsg = '!!! Exception raised during suite execution:' + str(e) + "\n" + traceback.format_exc()
            suitePassed = False
        finally:
            self.tearDown()    
            ended  = time.time()
            self.elapsed = ended - started
            if errmsg != '':
                print errmsg
            
        return suitePassed,suiteOutput + errmsg
        
    def onBeforeExecution(self, observer):
        observer.OnBeforeSuiteExecution(self)
        
    def onAfterExecution(self, observer, result, output):
        observer.OnAfterSuiteExecution(self, result, output) 

class TestTaskResult:
    """Simple enumerator for task execution result"""
    Passed = 0
    Failed = 1
    Skipped= 2
    
class VolcanoTestRunner:
    """Base class providing basic test task running"""
    def __init__(self):
        self.resultMap = {
          TestTaskResult.Passed:  'Passed',
          TestTaskResult.Failed:  'Failed',
          TestTaskResult.Skipped: 'Skipped'
        }
        self.resultUnmap = {
          True: TestTaskResult.Passed,
          False:TestTaskResult.Failed
        }
    
    def runTask(self, task, config): 
        task.onBeforeExecution(self)       
        result,output = task.run(self, config)
        task.onAfterExecution(self, self.resultUnmap[result], output)
        return result
    
    def publishResults(self):
        pass

    def OnBeforeTaskExecution(self, task):
        print 'Task:',    task.name
        if isinstance(task, VolcanoCmdTask):
            print 'Command:', task.command
            print 'WorkDir:',  task.workdir
    
    def OnAfterTaskExecution(self, task, result, stdoutdata):
        t = timedelta(seconds=task.elapsed)
        print 'Task ' + self.resultMap[result] + ': ' + task.name
        print 'Elapsed: ' + str(t) 
        
        if result == TestTaskResult.Failed:
            if isinstance(task, VolcanoCmdTask):
                print 'Return Code: ' + str(task.return_code) 
        print'=-----------------------------------------------------------------------------'
        sys.stdout.flush()
            
    def OnBeforeSuiteExecution(self, suite):
        print'=============================================================================='
        print'Suite:', suite.name
        print'=============================================================================='

    def OnAfterSuiteExecution(self, suite, result, stdoutdata):
        t = timedelta(seconds=suite.elapsed)
        print'=============================================================================='
        print'Suite ' + self.resultMap[result] + ': ' + suite.name
        print'Elapsed: '  + str(t) 
        print'=============================================================================='

class VolcanoRunConfig:
    def __init__(self, root_dir, target_type, build_type, cpu, cpu_features, vec):
        self.cpu            = cpu
        self.cpu_features   = cpu_features
        self.target_type    = target_type
        self.build_type     = build_type
        self.transpose_size = vec
        self.root_dir       = root_dir
        self.sub_configs    = {}
        
        #Configuration validation
        if not self.cpu in SUPPORTED_CPUS:
            raise Exception("Configuration Error: Unsupported CPU specified:" + self.cpu + ". Supported CPUs are:" + str(SUPPORTED_CPUS))

        if not self.target_type in SUPPORTED_TARGETS:
            raise Exception("Configuration Error: Unsupported target specified:" + self.target_type + ". Supported targets are:" + str(SUPPORTED_TARGETS))
        
        if not self.build_type in SUPPORTED_BUILDS:
            raise Exception("Configuration Error: Unsupported build type specified:" + self.build_type + ". Supported build types are:" + str(SUPPORTED_BUILDS))
        
        if not self.transpose_size in SUPPORTED_VECTOR_SIZES:
            raise Exception("Configuration Error: Unsupported transpose size specified:" + self.transpose_size + ". Supported transpose sizes are:" + str(SUPPORTED_VECTOR_SIZES))

        #
        # Calculating derived configuration
        #
        self.target_os     = TARGETS_MAP[self.target_type][0]
        self.target_os_bit = TARGETS_MAP[self.target_type][1]
        
        # Paths
        if self.root_dir == '' or self.root_dir == None:
            self.root_dir = os.path.join(os.path.curdir, os.path.pardir, os.path.pardir, os.path.pardir, os.path.pardir)
        
        self.root_dir     = os.path.abspath(self.root_dir)
        self.src_dir      = os.path.join(self.root_dir, 'src')
        self.besrc_dir    = os.path.join(self.src_dir,  'backend' )
        self.scripts_dir  = os.path.join(self.besrc_dir, 'build')
        self.install_dir  = os.path.join(self.root_dir, 'install',  self.target_type, self.build_type)
        self.bin_dir      = os.path.join(self.install_dir, 'bin')
    
        if self.target_os == 'Windows':
            self.solution_dir = os.path.join(self.root_dir, 'build', self.target_type)
        else:
            self.solution_dir = os.path.join(self.root_dir, 'build', self.target_type, self.build_type)
    
