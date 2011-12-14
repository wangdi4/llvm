import os, sys, platform
import framework.cmdtool
from framework.core import VolcanoTestRunner, VolcanoTestSuite, TIMEOUT_HALFHOUR
from optparse import OptionParser
from Volcano_Common import VolcanoRunConfig, SUPPORTED_TARGETS, SUPPORTED_BUILDS, DEFAULT_OCL_SOLUTION
from Volcano_Build import VolcanoBuilder, VolcanoBuilderConfig
from Volcano_Tasks import LitTest 

class VolcanoLIT(VolcanoTestSuite):
    """ Runs the Volcano LIT tests. Usually runs right after the VolcanoBuild task """
    def __init__(self, name, config, solution_name = DEFAULT_OCL_SOLUTION):
        VolcanoTestSuite.__init__(self, name)
        self.config  = config

        # LLVM Regression suite
        self.addTask(LitTest('Check', 'check', config, solution_name))

        # Vectorizer suite
        self.addTask(LitTest('Check_Vectorizer', 'check_vectorizer', config, solution_name))

        # Regression
        self.addTask(LitTest('Check_Regression', 'check_regression', config, solution_name))

        # OCL Reference compiler checks
        self.addTask(LitTest('Check_OCL_Reference', 'check_ocl_ref', config, solution_name), skiplist=[['.*','.*64']])
        
        # Barriers tests
        self.addTask(LitTest('Barriers', 'check_barrier', config, solution_name), skiplist=[['.*','.*64']])

    def startUp(self):
        os.environ['VOLCANO_ARCH'] = self.config.cpu
        os.environ['VOLCANO_CPU_FEATURES'] = self.config.cpu_features
        os.environ['VOLCANO_TRANSPOSE_SIZE'] = self.config.transpose_size
        
def main():
    parser = OptionParser()
    parser.add_option("-r", "--root",      dest="root_dir",    help="project root directory. Default: Autodetect", default=None)
    parser.add_option("-t", "--target",    dest="target_type",  help="Target type: " + str(SUPPORTED_TARGETS) + ". Default: Win32", default="Win32")
    parser.add_option("-b", "--build_type",dest="build_type",   help="Build type: " + str(SUPPORTED_BUILDS) + ". Default: Release", default="Release")
    parser.add_option("-s", "--skipbuild", dest="skip_build",   action="store_true",  help="skip the build", default=False)
    parser.add_option("--norebuild",       dest="rebuild",      action="store_false", help="Perform only regular build.", default=True )
    parser.add_option("--volcano",         dest="volcano_only", action="store_true",  help="Build the Volcano solution.", default=False)
    parser.add_option("--ocl",             dest="volcano_only", action="store_false", help="Build the OCL solution. Default")
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    skiplist = []
    if True == options.skip_build:
        skiplist=[['.*']]
    
    framework.cmdtool.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              'auto',
                              '',
                              '0')
                              
    build_config = VolcanoBuilderConfig( options.volcano_only)
    
    config.sub_configs[VolcanoBuilderConfig.CFG_NAME]= build_config
                              
    suite = VolcanoTestSuite('')
    suite.addTask(VolcanoBuilder('Build', config, rebuild = options.rebuild, skip_build = False), stop_on_failure = True, always_pass = False, skiplist=skiplist )
    suite.addTask(VolcanoLIT("LitTests", config, build_config.solution_name), stop_on_failure = True, always_pass = False)

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


