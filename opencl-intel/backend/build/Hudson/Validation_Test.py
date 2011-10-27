
'''
Runs conformance testing for specified conformance suite and provided captured data  
'''
from Validation_Record import OclRefSuites
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestSuite, \
    VolcanoTestTask, TestTaskResult, TIMEOUT_HOUR, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES, EnvironmentValue
from Volcano_Tasks import LitTest, UnarchiverTask, SimpleTest
from optparse import OptionParser
from string import Template, atoi
from xml.dom import minidom
import StringIO
import glob
import os
import platform
import re
import sys

# Standalone tests default settings
DEFAULT_SINGLE_WG = 1
DEFAULT_NEAT = 1
DEFAULT_REDUCE_WORK = 0
DEFAULT_SATEST_MODE = 'VAL'
DEFAULT_DETAILED_STAT = 0
DEFAULT_FORCE_REF = 1

class SATestOptions:
    def __init__(self,
                 single_wg=DEFAULT_SINGLE_WG,
                 neat=DEFAULT_NEAT,
                 mode=DEFAULT_SATEST_MODE,
                 detailed_stat=DEFAULT_DETAILED_STAT,
                 force_ref=DEFAULT_FORCE_REF):
        self.single_wg = single_wg
        self.neat = neat
        self.mode = mode
        self.detailed_stat = detailed_stat
        self.force_ref = force_ref
        
    def GetRunParams(self):
        modeStr = ' -' + self.mode
        neatStr = ' -neat=' + str(int(self.neat))
        singlewgStr = ' -single_wg=' + str(int(self.single_wg))
        detailedStatStr = ' -detailed_stat=' + str(int(self.detailed_stat))
        forceRefStr = ' -force_ref=' + str(int(self.force_ref))
        return modeStr + neatStr + singlewgStr + detailedStatStr + forceRefStr

class OclRefConformanceTest(VolcanoTestSuite):
    def __init__(self,
                 name,
                 stCount,
                 config,
                 saTestConfig,
                 test_path,
                 indexes_to_skip = []):
        VolcanoTestSuite.__init__(self, name)
        
        self.name = name
        self.config = config
        self.saTestConfig = saTestConfig
        self.generate_children_report = False
#       Read configuration files from file system
        config_list = self.getConfigs(name, test_path)
        
        for test_idx in range(1, stCount+1):
            subTestName = self.getStName(name, test_idx)
            cfg_file = subTestName + ".cfg"
            
            otherOptions = saTestConfig.GetRunParams()
            cfg_path = os.path.join(test_path, cfg_file)
            options = " -config=" + cfg_path + " -basedir=. " + "-cpuarch=" + config.cpu + " -cpufeatures=" + config.cpu_features + otherOptions
            
            executable = os.path.join(config.bin_dir, "SATest") 
            command = executable + options
            workdir = test_path

            if not test_idx in indexes_to_skip:
                self.addTask(SimpleTest(subTestName, workdir, command))
            else:
                self.addTask(SimpleTest(subTestName, workdir, command), skiplist=[['.*']])
                
    def startUp(self):
        if platform.system() != 'Windows':
            self.path_env = EnvironmentValue('LD_LIBRARY_PATH')
        else:
            self.path_env = EnvironmentValue("PATH")
        self.path_env.appendToFront(self.config.bin_dir, os.pathsep)

    def tearDown(self):
        self.path_env.restoreValue()
    
    def getStName(self, test_name, test_idx):
        """ Returns sub test name for given index and test group name 
            format is the following: for idx = 1: test_name
                                     for idx!= 1: test_name.idx
            ..."""
        if test_idx == 1:
            return test_name
        else:
            return test_name + "." + str(test_idx)    
            
    def getConfigs(self, name, test_path):
        """returns the list of configuration files for this test in the given folder"""
        return glob.glob(os.path.join(test_path, name + '*.cfg'))

# Suite for running opencl reference conformance tests
# config             Volcano run configuration
# tests_share        Path to zipped conformance tests
# saTestConfig       Options for running sandalone tests
class OclRefConformanceSuite(VolcanoTestSuite):
    def __init__(self,
                 name,
                 config,
                 suite_desc,
                 tests_path,
                 tests_to_skip,
                 saTestConfig=SATestOptions()):
        VolcanoTestSuite.__init__(self, name)
        self.config = config
        self.tests_path = os.path.abspath(tests_path)
        self.saTestConfig = saTestConfig
        self.time_out = TIMEOUT_HOUR
        self.suite_desc = suite_desc
        self.tests_to_skip = tests_to_skip
    
    def startUp(self):
        for test_desc in self.suite_desc.test_list:
            testName = test_desc
#            When running on Hudson here we don't know the number of tests.
#            In order to calculate it one can run
#            Validation_Test.py with line below uncommented
#            It saves number of tests to description. After that
#            list of descriptions can be copy-pasted to the test file
            subTestsCount = self.countSubTests(testName, self.tests_path)
            indexes_to_skip = []
            if testName in self.suite_desc.skip_list:
                indexes_to_skip = self.suite_desc.skip_list[testName]
            suite = OclRefConformanceTest(testName, subTestsCount, self.config, self.saTestConfig, self.tests_path, indexes_to_skip)
            self.addTask(suite)
        for test_name in self.tests_to_skip:
            self.updateTask(test_name[0], skiplist=test_name[1])
#        print self.suite_desc.test_list
        
    def countSubTests(self, name, test_path):
        """returns the list of configuration files for this test name in the given folder"""
        configs = glob.glob(os.path.join(test_path, name + '.cfg'))
        if len(configs) > 1:
            raise Exception("Error parsing configuration files for test" + name) 
        configs.extend((glob.glob(os.path.join(test_path, name + '.*.cfg'))))
        return len(configs)

class FrameworkOclRefSuite(OclRefConformanceSuite):
    def __init__(self, config, tests_path, options):
        OclRefConformanceSuite.__init__(self, "Framework_Test_Type_Suite", config, OclRefSuites["framework_test_type"], tests_path, options)
        self.updateTask('Test_clGetDeviceIDsTest', skiplist=[[".*"]])
        self.updateTask('Test_clGetPlatformInfoTest', skiplist=[[".*"]])
        self.updateTask('Test_clGetDeviceInfoTest', skiplist=[[".*"]])
        self.updateTask('Test_clCreateContextTest', skiplist=[[".*"]])
        self.updateTask('Test_clCreateBufferTest', skiplist=[[".*"]])
        self.updateTask('Test_clEnqueueCopyBufferTest', skiplist=[[".*"]])
        self.updateTask('Test_clCopyImageTest', skiplist=[[".*"]])
        self.updateTask('Test_MT_context_retain', skiplist=[[".*"]])
        self.updateTask('Test_MT_release', skiplist=[[".*"]])
        self.updateTask('Test_clNativeFunctionTest', skiplist=[[".*"]])
        self.updateTask('Test_EnqueueNativeKernelTest', skiplist=[[".*"]])
        self.updateTask('Test_EventCallbackTest', skiplist=[[".*"]])
        self.updateTask('Test_clFinishTest', skiplist=[[".*"]])
        self.updateTask('Test_clIntelOfflineCompilerTest', skiplist=[[".*"]])
        self.updateTask('Test_clIntelOfflineCompilerThreadsTest', skiplist=[[".*"]])
        self.updateTask('Test_clIntelOfflineCompilerBuildOptionsTest', skiplist=[[".*"]])
        # Disable some subtests because of bug #CSSD100006755
        self.updateTask('Test_clImageExecuteTest', skiplist=[['.*', '.*64']])
        self.updateTask('Test_opencl_printf_test', skiplist=[['.*', '.*64']])
        self.updateTask('Test_overloadingTest', skiplist=[['.*', '.*64']])
            
def main():
    parser = OptionParser()
    
    SATEST_MODE = ['VAL', 'REF', 'PERF']
     
    parser.add_option("-r", "--root", dest="root_dir", help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32, Win64, Linux64", default='Win32')
    parser.add_option("-c", "--cpu",  dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features:+avx,-avx256", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size: 0(auto),1,4,8,16", default="0")
    parser.add_option("-o", "--log", dest="log_file", help="Log file name", default='log.txt')
    parser.add_option('-b', '--build_type', dest='build_type', help='Build type. Possible values. Allowed values: ' + str(SUPPORTED_BUILDS), default='Release')
    # SATest options
    parser.add_option("-k", "--kernels", dest="tests_path", help="root path of tests", default='')
    parser.add_option("-s", "--suite", dest="suite", help="Suite to capture: basic_common, basic_images", default=None)
    parser.add_option("-w", "--single_wg", dest='single_wg', help='Running interpreter and back-end in single workgroup', default=DEFAULT_SINGLE_WG)
    parser.add_option("-n", "--neat", dest='neat', help='Running reference with neat support', default=DEFAULT_NEAT)
    parser.add_option("-m", "--satest_mode", dest='satest_mode', help='SATest rinning mode. Allowed values:' + str(SATEST_MODE), default=DEFAULT_SATEST_MODE)
    parser.add_option("-d", "--detailed_stat", dest='detailed_stat', help='Use SATest detailed statistics', default=DEFAULT_DETAILED_STAT)
    parser.add_option("-e", "--force_ref", dest='force_ref', help='Generate reference data instead of loading from files' , default=DEFAULT_FORCE_REF)
    (options, args) = parser.parse_args()
    
    saTestConfig = SATestOptions(options.single_wg,
                                   options.neat,
                                   options.satest_mode,
                                   options.detailed_stat,
                                   options.force_ref)
    
    config = VolcanoRunConfig(options.root_dir,
                                  options.target_type,
                                  options.build_type,
                                  options.cpu,
                                  options.cpu_features,
                                  options.transpose_size)
    
    suite = OclRefConformanceSuite("Validation_Conformance_Suite",
                                   config,
                                   OclRefSuites[options.suite],
                                   options.tests_path,
                                   saTestConfig)
    runner = VolcanoTestRunner()
    passed = runner.runTask(suite, config)
    
    if not passed:
        return 1
    
    return 0

if __name__ == "__main__":
    if platform.platform().startswith("CYGWIN"):
        print "Cygwin Python is not supported. Please use ActiveState Python."
        sys.exit(1);
    if sys.version_info < (2, 6):
        print "Python version 2.6 or later required"
        sys, exit(1)
    main_result = main()
    sys.exit(main_result)

    
