import os, shutil, platform
from xml.dom.minidom import Document
from Volcano_Common import VolcanoTestRunner, TestTaskResult, VolcanoRunConfig

defaultLogPath = os.path.realpath(os.path.join(os.path.curdir, 'buildLogs'))
if platform.system() == 'Windows':
    #need to ensure that event very long filenames will be supported on Windows platform
    defaultLogPath = u'\\\\?\\' + unicode(defaultLogPath)

def touch(fname, times = None):
    """Mimics the touch command"""
    with file(fname, 'a'):
        os.utime(fname, times)

class HudsonEnvironment:
    """
    Role: Hudson Environment Wrapper
    Responsibility: 
        Provides the easy access to standard environment setting in Hudson environment
        The following environment variable are supported
        
    BUILD_NUMBER
    The current build number, such as "153"
    BUILD_ID
    The current build id, such as "2005-08-22_23-59-59" (YYYY-MM-DD_hh-mm-ss)
    JOB_NAME
    Name of the project of this build, such as "foo"
    BUILD_TAG
    String of "hudson-${JOB_NAME}-${BUILD_NUMBER}". Convenient to put into a resource file, a jar file, etc for easier identification.
    EXECUTOR_NUMBER
    The unique number that identifies the current executor (among executors of the same machine) that's carrying out this build. This is the number you see in the "build executor status", except that the number starts from 0, not 1.
    NODE_NAME
    Name of the slave if the build is on a slave, or "" if run on master
    NODE_LABELS
    Whitespace-separated list of labels that the node is assigned.
    JAVA_HOME
    If your job is configured to use a specific JDK, this variable is set to the JAVA_HOME of the specified JDK. When this variable is set, PATH is also updated to have $JAVA_HOME/bin.
    WORKSPACE
    The absolute path of the workspace.
    HUDSON_URL
    Full URL of Hudson, like http://server:port/hudson/
    BUILD_URL
    Full URL of this build, like http://server:port/hudson/job/foo/15/
    JOB_URL
    Full URL of this job, like http://server:port/hudson/job/foo/
    SVN_REVISION
    For Subversion-based projects, this variable contains the revision number of the module.
    CVS_BRANCH
    For CVS-based projects, this variable contains the branch of the module. If CVS is configured to check out the trunk, this environment variable will not be set.
    """
    def getParam(self, name):
        if( not name in os.environ):
            return ''
        
        return os.environ[name]
    
    def getBuildNumber(self):
        return self.getParam('BUILD_NUMBER')
        
    def getBuildId(self):
        return self.getParam('BUILD_ID')
        
    def getJobName(self):
        return self.getParam('JOB_NAME')
        
    def getBuildTag(self):
        return self.getParam('BUILD_TAG')
        
    def getExecutorNumber(self):
        return self.getParam('EXECUTOR_NUMBER')
        
    def getNodeName(self):
        return self.getParam('NODE_NAME')
        
    def getNodeLabels(self):
        return self.getParam('NODE_LABELS')
        
    def getWorkspacePath(self):
        return self.getParam('WORKSPACE')
        
    def getBuildUrl(self):
        return self.getParam('BUILD_URL')
        
    def getJobUrl(self):
        return self.getParam('JOB_URL')
        
    def getSvnRevision(self):
        return self.getParam('SVN_REVISION')

    def getSvnUrl(self):
        return self.getParam('SVN_URL')
    
    def getSvnUrlN(self, n):
        return self.getParam('SVN_URL_' + str(n))
    
class HudsonBuildEnvironment(HudsonEnvironment):
    """
    Role: Volcano Hudson Job specific environment wrapper
    Responsibility:
        Provides information about Volcano specific settings, like Volcano Labels mapping to target platform and cpu information
    """
    def getTargetPlatform(self):
        targetType = self.getParam('Target_Type')
        if( targetType == '' ):
            # WORKAROUND: Maps a Hudson job label to a target string.
            #             Enables the legacy jobs to continue to work with old configurations
            label2TargetMap = {'Volcano_Linux64':'Linux64', 'Volcano_SLES11': 'Linux64', 'Volcano_Win32': 'Win32', 'Volcano_Win64': 'Win64', 'Volcano_Win32_snb': 'Win32', 'Volcano_Win64_snb': 'Win64', 'NN_Win32': 'Win32', 'NN_Win64': 'Win64' }
            targetType = label2TargetMap[ self.getParam('label')]
        return targetType     

    def getCPUType(self):
        return self.getParam('Cpu_Arch')

    def getTransposeSize(self):
        return self.getParam('Transpose_Size')

    def getBuildType(self):
        return self.getParam('Build_Type')

class JUnitFormatter:
    '''
    Role: test results resport formatter
    Responsibility: Generates the test report in JUnit format:
    
    <testsuite failures="0" time="0.289" errors="0" tests="3" skipped="0" name="UnitTests.MainClassTest">
        <testcase time="0.146" name="TestPropertyValue"     classname="UnitTests.MainClassTest"/>
        <testcase time="0.001" name="TestMethodUpdateValue" classname="UnitTests.MainClassTest"/>
        <testcase time="0.092" name="TestFailure"           classname="UnitTests.MainClassTest"/>
    </testsuite>
    '''
    def __init__(self, name):
        self.doc    = Document() # test document
        # Create the <testsuits> base element
        self.node_testsuite = self.doc.createElement("testsuite")
        self.node_testsuite.setAttribute('time', '0.1')
        self.node_testsuite.setAttribute('name', name)
        # Create the system-out element
        node_systemout = self.doc.createElement("system-out")
        node_systemerr = self.doc.createElement("system-err")
        
        node_comment_out   = self.doc.createCDATASection("")
        node_comment_err   = self.doc.createCDATASection("")
        node_systemout.appendChild(node_comment_out)
        node_systemerr.appendChild(node_comment_err)
        
        self.node_testsuite.appendChild(node_systemout)
        self.node_testsuite.appendChild(node_systemerr)
        self.doc.appendChild(self.node_testsuite)

        
    def addPassedTest(self, name, classname, testurl, log, elapsed):
        # <testcase time="0.001" name="TestMethodUpdateValue"
        # classname="UnitTests.MainClassTest"/>
        node_comment= self.doc.createCDATASection(log)
        node_stdout = self.doc.createElement("system-out")
        node_stdout.appendChild(node_comment)
        
        node_url= self.doc.createCDATASection(testurl)
        node_stderr = self.doc.createElement("system-err")
        node_stderr.appendChild(node_url)

        
        node_testcase = self.doc.createElement("testcase")
        node_testcase.setAttribute('time', str(elapsed))
        node_testcase.setAttribute('name',  name)
        node_testcase.setAttribute('classname', classname)
        node_testcase.appendChild(node_stdout)
        node_testcase.appendChild(node_stderr)
        
        self.node_testsuite.appendChild(node_testcase)

    def addSkippedTest(self, name, classname):
        # <testcase time="0.001" name="TestMethodUpdateValue"
        # classname="UnitTests.MainClassTest"/>
        node_skipped = self.doc.createElement("skipped")
        
        node_testcase = self.doc.createElement("testcase")
        node_testcase.setAttribute('time', '0.0')
        node_testcase.setAttribute('name',  name)
        node_testcase.setAttribute('classname', classname)
        node_testcase.appendChild(node_skipped)
        
        self.node_testsuite.appendChild(node_testcase)

    def addFailedTest(self, name, classname, testurl, log, elapsed):
        # <testcase time="0.001" name="TestMethodUpdateValue" classname="UnitTests.MainClassTest">
        # <failure>Error</failure>
        # </testcase>
        node_comment= self.doc.createCDATASection(log)
        node_stdout = self.doc.createElement("system-out")
        node_stdout.appendChild(node_comment)
        
        node_url= self.doc.createCDATASection(testurl)
        node_stderr = self.doc.createElement("system-err")
        node_stderr.appendChild(node_url)

        node_failure  = self.doc.createElement("failure")
        node_failure.setAttribute('message', log)
        node_failure.nodeValue = log
        
        node_testcase = self.doc.createElement("testcase")
        node_testcase.setAttribute('time', str(elapsed))
        node_testcase.setAttribute('name',  name)
        node_testcase.setAttribute('classname', classname)
        node_testcase.appendChild(node_failure)
        node_testcase.appendChild(node_stdout)
        node_testcase.appendChild(node_stderr)
        
        self.node_testsuite.appendChild(node_testcase)

    def updateCounts(self, fail_count, skip_count, pass_count):
        self.node_testsuite.setAttribute('failures', str(fail_count))
        self.node_testsuite.setAttribute('errors', str(skip_count))
        self.node_testsuite.setAttribute('tests', str(fail_count + skip_count + pass_count))
        
    def save(self, name):
        report_file = open(name, 'w')
        print >> report_file, self.doc.toprettyxml(indent="  ")
        report_file.close()

class ResultsFileManager:
    """
    Role: Hudson job result files manager
    Responsibility: 
        Generates the unique results files for given job
        Provides the means to report about job success,failure,ignorance
        http://cvcc-w7-mrm-03/view/vdovleka/job/vdovleka_Volcano_Pre_Commit/Build_Type=Release,label=Volcano_SLES11/lastSuccessfulBuild/artifact/buildLogs/test_log_SNB_Linux64_Release_build.log
        http://cvcc-w7-mrm-03/view/vdovleka/job/vdovleka_Volcano_Pre_Commit/Build_Type=Release,label=Volcano_SLES11/36/artifact/buildLogs/test_log_SNB_Linux64_Release_build.log
    """
    def __init__(self, config, name, logPath = defaultLogPath):
        suffix = self.GetReportSuffix(config)
        self.baseLogName = 'test_log_' + suffix
        self.reportName  = 'test_report_' + name + '_' + suffix + '.xml'
        self.logPath   = logPath
        if( not os.path.exists(logPath)):
            os.makedirs(logPath)
    
    def GetReportSuffix(self, config):
        suffix= [config.cpu,
                 config.transpose_size,
                 config.target_type,
                 config.build_type]

        return '_'.join([str(v) for v in suffix if v != ''])        
        
            
    def GetLogPath(self):
        return self.logPath

    def GetReportName(self):
        return os.path.join(self.logPath, self.reportName)
    
    def GetBaseLogName(self):
        return os.path.join(self.logPath, self.baseLogName)

    def GetJobLogName(self):
        return self.GetBaseLogName() + '.log'

    def GetTestLogName(self, test_name):
        return self.GetBaseLogName() + '_' + test_name + '.log'
    
    def GetTestArtifactLogName(self, test_name):
        env = HudsonEnvironment()
        return env.getBuildUrl() + 'artifact/buildLogs/' + self.baseLogName + '_' + test_name + '.log'
    
    def GetTestResultsLogName(self, test_name):
        env = HudsonEnvironment()
        test_name = test_name.replace('.', '/')
        test_name = test_name.replace('.', '/')
        test_name = test_name.replace('//', '/')
        return 'testReport/' + test_name
    
    def UpdateLog(self, test_name, log):
        log_fname = self.GetTestLogName(test_name)
        log_file = open(log_fname, 'w')
        log_file.write(log)
        log_file.close()

class HudsonTestRunner(VolcanoTestRunner):
    """
    Role: Hudson specific Test Runner
    Responsibilities:
        Provide the same functionality as VolcanoTestRunner
        In addition manage the task logs and task results reporting
    """
    def __init__(self, config, name):
        VolcanoTestRunner.__init__(self)
        self.fm = ResultsFileManager(config, name, defaultLogPath)
        self.formatter  = JUnitFormatter("JobName")
        self.config     = config
        self.fail_count = 0
        self.skip_count = 0
        self.pass_count = 0
        self.name = name
    
    def GetTestName(self):
        transpose_size = self.config.transpose_size
        if transpose_size != '':
            transpose_size = 'Vec' + transpose_size
        name = [self.config.target_type, self.config.build_type, self.config.cpu, transpose_size ]
        return '_'.join([ str(n) for n in name if n != ''])
    
    def GetTestClassName(self, tname):
        return tname #self.name + '.' + tname
    
    def ProcessExecutionResult(self, task, result, stdoutdata):
        tname = task.fullName()
        self.fm.UpdateLog(tname, stdoutdata)
        if  TestTaskResult.Passed == result:
            self.pass_count += 1
            if task.generate_report:
                self.formatter.addPassedTest(self.GetTestName(), 
                                             self.GetTestClassName(tname),
                                             self.fm.GetTestArtifactLogName(tname), 
                                             self.fm.GetTestResultsLogName(tname), 
                                             task.elapsed)
        elif TestTaskResult.Skipped == result:
            self.skip_count += 1
            if task.generate_report:
                self.formatter.addSkippedTest(self.GetTestName(), self.GetTestClassName(tname))
        else:
            self.fail_count += 1
            if task.generate_report:
                self.formatter.addFailedTest(self.GetTestName(), 
                                             self.GetTestClassName(tname), 
                                             self.fm.GetTestArtifactLogName(tname), 
                                             self.fm.GetTestResultsLogName(tname), 
                                             task.elapsed)
    
    def OnAfterTaskExecution(self, task, result, stdoutdata):
        VolcanoTestRunner.OnAfterTaskExecution(self, task, result, stdoutdata)
        self.ProcessExecutionResult(task, result, stdoutdata)

    def OnAfterSuiteExecution(self, suite, result, stdoutdata):
        VolcanoTestRunner.OnAfterSuiteExecution(self, suite, result, stdoutdata)
        self.ProcessExecutionResult(suite, result, stdoutdata)

    def publishResults(self):
        self.formatter.updateCounts(self.fail_count, self.skip_count, self.pass_count)
        self.formatter.save(self.fm.GetReportName())

class HudsonRunConfig(VolcanoRunConfig):
    def __init__(self, root_dir ):
        env = HudsonBuildEnvironment()
        cpu = env.getCPUType()
        cpu_features = ''
        if( cpu == 'avx128'):
            #currently the AVX256 is not supported for sandybridge 
            cpu_features = '-avx256'
        VolcanoRunConfig.__init__(self, 
                                  root_dir,
                                  env.getTargetPlatform(), 
                                  env.getBuildType(),
                                  cpu,
                                  cpu_features,
                                  env.getTransposeSize())

