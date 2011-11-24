import os.path, sys, platform
import Volcano_CmdUtils
from Hudson_Common import HudsonBuildEnvironment, HudsonTestRunner, HudsonRunConfig
from Volcano_Common import TestTaskResult, VolcanoCmdTask, PERFORMANCE_LOG_ROOT, VolcanoTestSuite, VolcanoTestRunner
from Volcano_Performance import perf_suites, PerformanceTask, VolcanoPerformanceSuite, PerformanceRunConfig, PerformanceTestRunner, VolcanoWOLFPerformanceSuite, VolcanoWOLFBenchPerformanceSuite, VolcanoVCSDPerformanceSuite, VolcanoCyberLinkPerformanceSuite, VolcanoSandraPerformanceSuite, VolcanoLuxMarkPerformanceSuite, VolcanoBIMeterPerformanceSuite, VolcanoAVX256_P1_PerformanceSuite, VolcanoPhoronixPerformanceSuite, VolcanoGEHCPerformanceSuite, VolcanoSHOCPerformanceSuite
from Volcano_Build import VolcanoBinaryCopy


 
class HudsonPerformanceRunConfig(PerformanceRunConfig):
    def __init__(self, tests_path, log_path):
        PerformanceRunConfig.__init__(self, tests_path)
        
        if '' == log_path:
            log_path = PERFORMANCE_LOG_ROOT

        env = HudsonBuildEnvironment()
        self.logs_root_dir = log_path
        self.svn_revision  = env.getParam('SVN_Requested_Revision')
        self.test_suite    = env.getParam('Test_Suite')
        
class HudsonPerformanceTestRunner(HudsonTestRunner):
    """
    We need this class to be able to switch the csv file names for each performance suite
    """
    def __init__(self, config):
        HudsonTestRunner.__init__(self, config, '')
        perf_config  = config.sub_configs[PerformanceRunConfig.CFG_NAME]
        self.csv_filename_base = os.path.join( perf_config.logs_root_dir,  "volcano_wolf_perf_" + '_'.join([perf_config.svn_revision, config.target_type, config.cpu, config.transpose_size]))
        self.csv_filename = self.csv_filename_base

    def OnAfterTaskExecution(self, task, result, stdoutdata):
        VolcanoTestRunner.OnAfterTaskExecution(self, task, result, stdoutdata)
        if( isinstance(task,PerformanceTask)):
            if  TestTaskResult.Passed == result:
                with open(self.csv_filename, 'a') as csv_file:
                    print >> csv_file, stdoutdata.rstrip()
                            
    def OnBeforeSuiteExecution(self, suite):
        HudsonTestRunner.OnBeforeSuiteExecution(self, suite)
        if( isinstance(suite, VolcanoPerformanceSuite)):
            self.csv_filename = self.csv_filename_base + "_" + suite.suitename + ".csv"


class HudsonPerformanceSuite(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)

        self.addTask(VolcanoBinaryCopy("CopyVolcanoBinary", config),stop_on_failure = True)
    
        perf_config  = config.sub_configs[PerformanceRunConfig.CFG_NAME]
        
        suite  = perf_suites[perf_config.test_suite][0](perf_config.test_suite, config)
        filter = perf_suites[perf_config.test_suite][1]
    
        self.addTask( suite, skiplist=filter)

def main():
    # get the configuration parameters from the environment
    root_dir    = os.path.join(os.getcwd(),'trunk')

    # setup the configuration and test suite 
    config = HudsonRunConfig(root_dir)
    perf_config = HudsonPerformanceRunConfig('','')
    config.sub_configs[PerformanceRunConfig.CFG_NAME]=perf_config

    suite  = HudsonPerformanceSuite("PerformanceSuite", config) 
    runner = HudsonPerformanceTestRunner(config)
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
