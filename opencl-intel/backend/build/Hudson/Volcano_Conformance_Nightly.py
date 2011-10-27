import os, sys, platform
import Volcano_CmdUtils
from optparse import OptionParser
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestSuite, EnvironmentValue, TIMEOUT_HALFHOUR, TIMEOUT_HOUR, TIMEOUT_HOURANDHALF, TIMEOUT_DAY,SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Tasks import SimpleTest
from Volcano_WOLF import VolcanoWolfNightly, VolcanoWolfPerformance
from Volcano_Conformance_Framework import VolcanoConformanceFramework

class VolcanoConformanceNightly(VolcanoTestSuite):
    def __init__(self, config):
        VolcanoTestSuite.__init__(self,"Conformance")
        self.config = config
        self.conformance_dir     = os.path.join(config.bin_dir, 'validation', 'conformance')
        self.cpu_device_test_dir = os.path.join(config.bin_dir, 'validation', 'cpu_device_test_type')

        # Framework
        frameworkTest = VolcanoConformanceFramework(config)
        frameworkTest.timeout = TIMEOUT_HOURANDHALF
        self.addTask(frameworkTest)

        # CPUDevice
        cpuDeviceTest = SimpleTest('CPUDevice', config.bin_dir, os.path.join('validation', 'cpu_device_test_type','cpu_device_test_type'))
        cpuDeviceTest.success_code = 1 
        cpuDeviceTest.timeout = TIMEOUT_HALFHOUR
        self.addTask(cpuDeviceTest)

        # Basic
        basicTest = SimpleTest('Basic', config.bin_dir, os.path.join('validation', 'conformance', 'basic','test_basic'))
        basicTest.timeout = TIMEOUT_HOUR
        self.addTask(basicTest)
        
        # Half
        halfTest = SimpleTest('Half', config.bin_dir, os.path.join('validation', 'conformance', 'half','test_half') +' -w')
        halfTest.timeout = TIMEOUT_HALFHOUR
        self.addTask(halfTest)

        #Conversions
        convertionsTest = SimpleTest('Conversions', config.bin_dir, os.path.join('validation', 'conformance', 'conversions','test_conversions') +' -w')
        convertionsTest.timeout = TIMEOUT_HOURANDHALF
        self.addTask(convertionsTest)
   
        # WOLF benchmark
        # wolfNightly = VolcanoWolfNightly("WOLF_Benchmark", config, capture_data =  False)
        # self.addTask(wolfNightly, stop_on_failure = False, always_pass = False, skiplist=[['.*','Linux64']])
        
        # wolfPerf = VolcanoWolfPerformance("WOLF_Performance", config, capture_data =  False)
        # self.addTask(wolfPerf, stop_on_failure = False, always_pass = False, skiplist=[['.*','Linux64']])

        #MathBruteForce
        mathTest = SimpleTest('MathBruteForce', self.conformance_dir, os.path.join('math_brute_force','bruteforce') +' -w')
        mathTest.timeout = TIMEOUT_HOURANDHALF
        self.addTask(mathTest)

        # Conformance
        runTest = SimpleTest('RunConformance', self.conformance_dir, 'python run_conformance.py opencl_conformance_tests_nightly.csv CL_DEVICE_TYPE_CPU')
        runTest.timeout = 12 * TIMEOUT_HOUR
        self.addTask(runTest)
    
    def startUp(self):
        if platform.system() != 'Windows':
            self.path_env = EnvironmentValue('LD_LIBRARY_PATH')
        else:
            self.path_env = EnvironmentValue("PATH")
        
        self.path_env.appendToFront(self.config.bin_dir, os.pathsep)
    
    def tearDown(self):
        self.path_env.restoreValue()
 
        
def main():
    parser = OptionParser()
    parser.add_option("-r", "--root", dest="root_dir", help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32/64,Linux64", default="Win32")
    parser.add_option("-b", "--build", dest="build_type", help="build type: Debug, Release", default="Release")
    parser.add_option("-c", "--cpu",  dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size:0(auto),1,4,8,16", default="0")    
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    Volcano_CmdUtils.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoConformanceNightly(config)
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
            sys,exit(1)
        main_result = main()
        sys.exit(main_result)


