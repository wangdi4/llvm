import os, sys, platform
import Volcano_CmdUtils
from optparse import OptionParser
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestTask, VolcanoTestSuite, TIMEOUT_HALFHOUR
from Volcano_Build import VolcanoBuilder
from Volcano_Tasks import LitTest 

class VolcanoLIT(VolcanoTestSuite):
    """ Runs the Volcano LIT tests. Usually runs right after the VolcanoBuild task """
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.config  = config

        # LLVM Regression suite
        self.addTask(LitTest('Check', 'check', config))

        # Vectorizer suite
        self.addTask(LitTest('Check_Vectorizer', 'check_vectorizer', config))

        # Regression
        self.addTask(LitTest('Check_Regression', 'check_regression', config))

        # OCL Reference compiler checks
        self.addTask(LitTest('Check_OCL_Reference', 'check_ocl_ref', config), skiplist=[['.*','.*64']])

    def startUp(self):
        os.environ['VOLCANO_ARCH'] = self.config.cpu
        os.environ['VOLCANO_CPU_FEATURES'] = self.config.cpu_features
        os.environ['VOLCANO_TRANSPOSE_SIZE'] = self.config.transpose_size

            
        
        
        
def main():
    parser = OptionParser()
    parser.add_option("-r", "--root", dest="root_dir", help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32/64,Linux64", default="Win32")
    parser.add_option("-b", "--build", dest="build_type", help="build type: Debug,Release", default="Release")
    parser.add_option("-c", "--cpu",  dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features:+avx,-avx256", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size: 0(auto),1,4,8,16", default="0")
    parser.add_option("-s", "--skipbuild", action="store_true", dest="skip_build", help="skip the build", default=False)
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    skiplist = []
    if True == options.skip_build:
        skiplist=[['.*']]
    
    Volcano_CmdUtils.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)

    suite = VolcanoTestSuite('')
    suite.addTask(VolcanoBuilder('Build', config), stop_on_failure = True, always_pass = False, skiplist=skiplist )
    suite.addTask(VolcanoLIT("LitTests", config), stop_on_failure = True, always_pass = False)

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


