import os.path, sys, platform
from Volcano_Common import VolcanoTestTask, PERFORMANCE_LOG_ROOT, VolcanoTestSuite
from Volcano_Performance import VolcanoPerformanceSuite, PerformanceRunConfig, PerformanceTestRunner, PerformanceTask
from Volcano_Build import VolcanoBinaryCopy
from Volcano_Tasks import SimpleTest, UnarchiverTask, BINARIES_ARCH_NAME
from Hudson_Performance import HudsonPerformanceRunConfig, PerformanceReportTask

class HudsonPerformanceSingleSuite(VolcanoTestSuite):
    def __init__(self, name, kernels, config):
        VolcanoTestSuite.__init__(self, name)

        self.addTask(SimpleTest('Cleanup', config.build_dir, 'python cleanup.py -r ' + config.root_dir + ' -t ' + config.target_type + ' -b ' + config.build_type), stop_on_failure=True)
        self.addTask(UnarchiverTask('UnpackVolcanoBinaries', os.path.join(config.root_dir,BINARIES_ARCH_NAME), config.bin_dir), stop_on_failure = True)
        
        for kernel in kernels.split(' '):
            self.addTask(PerformanceTask(kernel, 16, config))

def main():
    # get the configuration parameters from the environment
    root_dir    = os.path.join(os.getcwd(),'trunk')
    target_type = os.environ['Target_Type']
    cpu         = os.environ['cpu_arch']
    vector_size = os.environ['vector_size']
    svn_revision= os.environ['SVN_Requested_Revision']
    host_name   = os.environ['NODE_NAME']
    kernel_names= os.environ['Kernels']
    branch_name = 'trunk'    
    
    # setup the configuration and test suite 
    config = HudsonPerformanceRunConfig(root_dir, 
                                        target_type, 
                                        'Release',
                                        cpu,
                                        vector_size, 
                                        '', 
                                        '',
                                        svn_revision,
                                        host_name,
                                        branch_name)


    suite  = HudsonPerformanceSingleSuite("PerformanceSuite", kernel_names, config)
    suite.addTask(PerformanceReportTask(config), True, False)
    
    runner = PerformanceTestRunner(config.log_path)
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


