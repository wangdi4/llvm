import os, sys, platform
import framework.cmdtool
import framework.resultPrinter
from optparse import OptionParser
from framework.core import VolcanoTestRunner, VolcanoTestSuite, TIMEOUT_HALFHOUR, TIMEOUT_HOUR, TIMEOUT_HOURANDHALF
from framework.utils import EnvironmentValue
from framework.tasks import SimpleTest
from Volcano_Common import VolcanoRunConfig, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Tasks import LitTest
from Volcano_WOLF import VolcanoWolf, VolcanoWolfPostCommit
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
        basicTest.timeout = TIMEOUT_HOURANDHALF
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
    parser.add_option("-t", "--target",    action="store",      choices=SUPPORTED_TARGETS, dest="target_type",  default="Win32",   help="Target type: " + str(SUPPORTED_TARGETS) + ". [Default: %default]")
    parser.add_option("-b", "--build_type",action="store",      choices=SUPPORTED_BUILDS,  dest="build_type",   default="Release", help="Build type: " + str(SUPPORTED_BUILDS) + ". [Default: %default]")
    parser.add_option("-c", "--cpu",       action="store",      choices=SUPPORTED_CPUS,    dest="cpu",          default="auto",    help="CPU Type: " + str(SUPPORTED_CPUS) + ". [Default: %default]")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size:0(auto),1,4,8,16", default="0")    
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    framework.cmdtool.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoConformancePostCommit("Conformance", config)
    runner = VolcanoTestRunner()
    passed = runner.runTask(suite, config)
    printer= framework.resultPrinter.ResultPrinter()
    printer.PrintResults(suite)

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


