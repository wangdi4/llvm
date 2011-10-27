import os
from Volcano_Common import VolcanoTestSuite
from Volcano_Tasks import SimpleTest

FrameworkTestsNames = [
'Test_clGetDeviceIDsTest',
'Test_clGetPlatformInfoTest',
'Test_clGetDeviceInfoTest',
'Test_clOutOfOrderTest',
'Test_clCreateContextTest',
'Test_clBuildProgramWithBinaryTest',
'Test_clBuildProgramWithSourceTest',
'Test_clCreateKernelTest',
'Test_clCreateBufferTest',
'Test_clExecutionTest',
'Test_clOODotProductTest',
'Test_clEnqueueCopyBufferTest',
'Test_clMapBufferTest',
'Test_clCopyImageTest',
'Test_clBuildProgramMaxArgsTest',
'Test_clKernelAttributesTest',
'Test_clKernelBarrierTest',
'Test_clImageExecuteTest',
'Test_clIntegerExecuteTest',
'Test_clMathExecuteTest',
'Test_memset_test',
'Test_MT_context_retain',
'Test_MT_release',
'Test_MT_execution',
'Test_ConcurrentBuildProgramTest',
'Test_clNativeFunctionTest',
'Test_clRelaxedFunctionTest',
'Test_EnqueueNativeKernelTest',
'Test_TBB',
'Test_clLocalStructTest',
'Test_opencl_printf_test',
'Test_VecTypeHintTest',
'Test_EventCallbackTest',
'Test_clFinishTest',
'Test_MT_order',
'Test_MisalignedHostPtr',
'Test_overloadingTest',
'Test_clIntelOfflineCompilerTest',
'Test_clIntelOfflineCompilerThreadsTest',
'Test_clIntelOfflineCompilerBuildOptionsTest',
]


class VolcanoConformanceFramework(VolcanoTestSuite):
    def __init__(self, config):
        VolcanoTestSuite.__init__(self,"Framework")
        self.generate_children_report = False

        for testname in FrameworkTestsNames:
            test = SimpleTest(testname, config.bin_dir,  os.path.join( 'validation', 'framework_test_type', 'framework_test_type') + ' --gtest_filter=FrameworkTestType.' + testname)
            test.success_code = 1
            self.addTask(test)

        self.updateTask("Test_opencl_printf_test", skiplist = [[".*","Linux64"]])
        self.updateTask("Test_VecTypeHintTest", skiplist = [[".*",".*",".*",".*","1"]])
        self.updateTask("Test_clFinishTest", skiplist = [[".*"]])
