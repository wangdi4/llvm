import os.path, sys, platform
import Volcano_CmdUtils
from Hudson_Common import HudsonBuildEnvironment
from Volcano_Common import VolcanoTestTask, PERFORMANCE_LOG_ROOT, VolcanoTestSuite
from Volcano_Performance import VolcanoPerformanceSuite, PerformanceRunConfig, PerformanceTestRunner, VolcanoWOLFPerformanceSuite, VolcanoWOLFBenchPerformanceSuite, VolcanoVCSDPerformanceSuite, VolcanoCyberLinkPerformanceSuite, VolcanoSandraPerformanceSuite, VolcanoLuxMarkPerformanceSuite, VolcanoBIMeterPerformanceSuite, VolcanoAVX256_P1_PerformanceSuite, VolcanoPhoronixPerformanceSuite, VolcanoGEHCPerformanceSuite, VolcanoSHOCPerformanceSuite
from Volcano_Build import VolcanoBinaryCopy
from Volcano_Tasks import SimpleTest, UnarchiverTask, BINARIES_ARCH_NAME


 
class HudsonPerformanceRunConfig(PerformanceRunConfig):
    def __init__(self, root_dir, tests_path, log_path):
        env = HudsonBuildEnvironment()
        cpu = env.getCPUType()
        cpu_features = ""
        if( cpu == 'avx128'):
            cpu = 'sandybridge'
            cpu_features = '-avx256'
        PerformanceRunConfig.__init__(self,
                                  root_dir,
                                  env.getParam('Target_Type'), 
                                  'Release',
                                  cpu,
                                  cpu_features,
                                  env.getTransposeSize(),
                                  tests_path)
        
        if '' == log_path:
            log_path = PERFORMANCE_LOG_ROOT

        self.logs_root_dir = log_path
        self.svn_revision  = env.getParam('SVN_Requested_Revision')
        self.host_name     = env.getParam('NODE_NAME')
        self.branch_name   = env.getParam('SVN_Requested_URL')
        self.test_suite    = env.getParam('Test_Suite')
        self.log_path      = os.path.join( self.logs_root_dir,  "volcano_wolf_perf_" + '_'.join([self.svn_revision, self.target_type, self.cpu, self.vector_size]))
        self.db_server     = 'cvcc-w7-nhlm-01'
        self.db_path       = 'Volcano'
        
class HudsonPerformanceTestRunner(PerformanceTestRunner):
    """
    We need this class to be able to switch the csv file names for each performance suite
    """
    def __init__(self, csv_filename_base):
        PerformanceTestRunner.__init__(self, csv_filename_base)
        self.csv_filename_base = csv_filename_base
        
    def OnBeforeSuiteExecution(self, suite):
        PerformanceTestRunner.OnBeforeSuiteExecution(self, suite)
        if( isinstance(suite, VolcanoPerformanceSuite)):
            self.csv_filename = self.csv_filename_base + "_" + suite.suitename + ".csv"

class PerformanceReportTask(VolcanoTestTask):
    def __init__(self, suitename, config):
        VolcanoTestTask.__init__(self, 'save_to_db')
        self.workdir  = os.path.join( config.build_dir, 'tools', config.target_type, 'reportgen' )
        csv_filename  = config.log_path  + "_" + suitename + ".csv"
        self.command  = 'reportgen.exe ' + ' '.join( [suitename, config.host_name, config.branch_name, config.target_type, config.cpu, config.svn_revision, config.vector_size, csv_filename, config.db_server, config.db_path]) 

class HudsonPerformanceSuite(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)

        self.addTask(SimpleTest('Cleanup', config.build_dir, 'python cleanup.py -r ' + config.root_dir + ' -t ' + config.target_type + ' -b ' + config.build_type), stop_on_failure=True)
        self.addTask(UnarchiverTask('Prepare_Binaries', os.path.join(config.root_dir,BINARIES_ARCH_NAME), config.bin_dir), stop_on_failure = True)

        suites = { "WOLF":      [VolcanoWOLFPerformanceSuite,      []],
                   "WOLFbench": [VolcanoWOLFBenchPerformanceSuite, []],
                   "CyberLink": [VolcanoCyberLinkPerformanceSuite, [['.*','*64']]],
                   "VCSD":      [VolcanoVCSDPerformanceSuite,      [['.*','*64']]],
                   "Sandra":    [VolcanoSandraPerformanceSuite,    [['.*','Win32']]],
                   "LuxMark":   [VolcanoLuxMarkPerformanceSuite,   []],
                   "BIMeter":   [VolcanoBIMeterPerformanceSuite,   [['.*','*64']]],
                   "AVX256_P1": [VolcanoAVX256_P1_PerformanceSuite,[]],
                   "Phoronix":  [VolcanoPhoronixPerformanceSuite,  [['.*','Win32']]],
                   "GEHC":      [VolcanoGEHCPerformanceSuite,      [['.*','Win32']]],
                   "SHOC":      [VolcanoSHOCPerformanceSuite,      [['.*','Win32']]]
                 }
    
        suite  = suites[config.test_suite][0](config.test_suite, config)
        filter = suites[config.test_suite][1]
    
        self.addTask( suite, skiplist=filter)

def main():
    # get the configuration parameters from the environment
    root_dir    = os.path.join(os.getcwd(),'trunk')

    # setup the configuration and test suite 
    config = HudsonPerformanceRunConfig(root_dir, 
                                        '', # use the default tests path settings 
                                        '') # use the default logs path settings

    suite  = HudsonPerformanceSuite("PerformanceSuite", config) 
    runner = HudsonPerformanceTestRunner(config.log_path)
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
