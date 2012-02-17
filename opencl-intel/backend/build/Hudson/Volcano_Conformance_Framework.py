import os
from framework.tasks import GoogleTestSuite

class VolcanoConformanceFramework(GoogleTestSuite):
    def __init__(self, config):
        GoogleTestSuite.__init__(self, 
                                 "Framework", 
                                 os.path.join('validation', 'framework_test_type','framework_test_type'),
                                 config.bin_dir,
                                 config )
        self.default_success_code  = 1
        #self.generate_children_report = False

    def startUp(self):
        GoogleTestSuite.startUp(self)
        self.updateTask("Test_opencl_printf_test", skiplist = [['.*','SLES64'],['.*','RH64']])
        self.updateTask("Test_VecTypeHintTest", skiplist = [[".*",".*",".*",".*","1"]])
        self.updateTask("Test_clFinishTest", skiplist = [[".*"]])
        self.updateTask("Test_MT_execution", skiplist = [[".*"]])
