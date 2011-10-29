import os, sys, platform
import Volcano_CmdUtils
from optparse import OptionParser
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestSuite, EnvironmentValue, TIMEOUT_HALFHOUR, TIMEOUT_HOUR, TIMEOUT_HOURANDHALF,SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Tasks import LitTest, SimpleTest
from Volcano_WOLF import VolcanoWolf, WolfPostCommit
from Volcano_Conformance_Framework import VolcanoConformanceFramework
from Volcano_Conformance_Basic import VolcanoConformanceBasic


class VolcanoConformancePostCommit(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.config = config

        # Framework
        frameworkTest = VolcanoConformanceFramework(config)
        frameworkTest.timeout = TIMEOUT_HOURANDHALF
        self.addTask(frameworkTest)

        # CPUDevice
        cpuDeviceTest = SimpleTest('CPUDevice', config.bin_dir, os.path.join('validation', 'cpu_device_test_type', 'cpu_device_test_type'))
        cpuDeviceTest.success_code = 1 
        cpuDeviceTest.timeout = TIMEOUT_HALFHOUR
        self.addTask(cpuDeviceTest)

        # Basic
        basicTest = VolcanoConformanceBasic(config)
        basicTest.timeout = TIMEOUT_HOUR
        self.addTask(basicTest)
        
        # Half
        halfTest = SimpleTest('Half', config.bin_dir, os.path.join('validation', 'conformance', 'half','test_half') +' -w')
        halfTest.timeout = TIMEOUT_HALFHOUR
        self.addTask(halfTest)
    
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
    parser.add_option("-v", "--vec", dest="tranpose_size", help="Tranpose Size:0(auto),1,4,8,16", default="0")    
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    Volcano_CmdUtils.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoConformancePostCommit("Conformance", config)
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


