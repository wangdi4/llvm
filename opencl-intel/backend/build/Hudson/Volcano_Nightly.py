from optparse import OptionParser
import os, sys, platform
import framework.cmdtool
from framework.core import VolcanoTestRunner, VolcanoTestSuite, TIMEOUT_HALFHOUR 
from framework.utils import EnvironmentValue
from framework.tasks import SimpleTest
from Volcano_Common import VolcanoRunConfig, DX_10_SHADERS_ROOT,SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Build import VolcanoBuilder, CopyWolfWorkloads
from Volcano_Tasks import LitTest, VectorizerTest
from Volcano_Conformance_Nightly import VolcanoConformanceNightly

class VolcanoNightlyBAT(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.config = config

        # Copy the WOLF workloads  before the tests execution
        # self.addTask( CopyWolfWorkloads( 'CopyWolfWL', config), stop_on_failure=True)

        # Vectorizer tests
        vectorizerTest = VectorizerTest('Vectorizer_Tests', config)
        vectorizerTest.timeout = TIMEOUT_HALFHOUR
        self.addTask(vectorizerTest , skiplist=[['.*', '.*']])

        # Google tests
        self.addTask(SimpleTest('ValidationTests', config.bin_dir, 'ValidationTests'))
        self.addTask(SimpleTest('LLVMUnitTests', config.bin_dir, 'LLVMUnitTests' ))
                
        # OCL Conformance
        self.addTask(VolcanoConformanceNightly(config))
    
    def startUp(self):
        os.environ['VOLCANO_ARCH'] = self.config.cpu
        os.environ['VOLCANO_CPU_FEATURES'] = self.config.cpu_features
        os.environ['VOLCANO_TRANSPOSE_SIZE'] = self.config.transpose_size
        os.environ['CL_CONFIG_VECTORIZER_HEURISTICS'] = 'false'
        self.vectenv = EnvironmentValue("CL_CONFIG_USE_VECTORIZER")
        if self.config.transpose_size == '1':
            self.vectenv.setValue('false')
        else:
            self.vectenv.setValue('true')
        
    def tearDown(self):
        self.vectenv.restoreValue()

class VolcanoNightly(VolcanoTestSuite):
    def __init__(self, name, config, skip_build):
        VolcanoTestSuite.__init__(self, name)

        # Build
        if( skip_build ): 
            self.addTask(VolcanoBuilder('Build', config), stop_on_failure = True, always_pass = False, skiplist=[['.*']] )
        else:
            self.addTask(VolcanoBuilder('Build', config), stop_on_failure = True, always_pass = False )

        self.addTask(VolcanoNightlyBAT("Tests", config), stop_on_failure = True, always_pass = False)

def main():
    parser = OptionParser()
    parser.add_option("-r", "--root", dest="root_dir", help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32/64,Linux64", default="Win32")
    parser.add_option("-b", "--build", dest="build_type", help="build type: Debug, Release", default="Release")
    parser.add_option("-c", "--cpu",  dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size:0(auto),1,4,8,16", default="0")    
    parser.add_option("-s", "--skipbuild", action="store_true", dest="skip_build", help="skip the build", default=False)
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()
    framework.cmdtool.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoNightly('Nightly', config, options.skip_build)
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


