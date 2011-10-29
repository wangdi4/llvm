import os, sys, platform
import Volcano_CmdUtils
from optparse import OptionParser
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestSuite, EnvironmentValue, TIMEOUT_HALFHOUR, TIMEOUT_HOUR, OCL_CONFORMANCE_TESTS_ROOT, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Tasks import LitTest, SimpleTest, UnarchiverTask
from Volcano_WOLF import VolcanoWolf, WolfPostCommit
from Volcano_Conformance_Framework import VolcanoConformanceFramework
from Volcano_Conformance_Basic import VolcanoConformanceBasic
from Validation_Test import SATestOptions, OclRefConformanceSuite, FrameworkOclRefSuite
from Validation_Record import OclRefSuites

class VolcanoReferenceConformanceTest(VolcanoTestSuite):
    def __init__(self, name, config, tests_to_skip):
        VolcanoTestSuite.__init__(self, name)

        dst_path = os.path.join(config.bin_dir, "OclConfRefTests")
        options = SATestOptions()
        options.detailed_stat = 1
        options.force_ref = 0

        # Basic common (w/o images)
        self.LoadTask(name, os.path.join(OCL_CONFORMANCE_TESTS_ROOT, str(config.target_type)), dst_path, config)
        suite = OclRefConformanceSuite("Conformance_Suite_"+name, 
                                             config, 
                                             OclRefSuites[name], 
                                             os.path.join(dst_path, name),
                                             tests_to_skip,
                                             options)
        self.addTask(suite)

    def LoadTask(self, name, arch_path, dst_path, config):
        archive_name = name + ".7z"
        self.addTask(UnarchiverTask("Extract_" + name, os.path.join(arch_path, archive_name), os.path.join(dst_path, name), useCurDir=True), stop_on_failure = True)



class VolcanoReferenceConformancePostCommit(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.config = config

        # Basic common (w/o images)
        skipTestList = [
        # Disable kernel_memory_alignment_* tests because they produce "undefined" integer values as output.
        ['kernel_memory_alignment_local',[['.*']]], ['kernel_memory_alignment_global', [['.*']]], ['kernel_memory_alignment_constant', [['.*']]], ['kernel_memory_alignment_private',[['.*']]],
        # Disable image sub-tests because captured data too large.
        ['readimage', [['.*']]],
        ['readimage_int16', [['.*']]],
        ['readimage_fp32', [['.*']]],
        ['writeimage', [['.*']]],
        ['writeimage_int16', [['.*']]],
        ['writeimage_fp32', [['.*']]],
        ['image_r8', [['.*']]],
        ['imagereadwrite', [['.*']]],
        ['imagereadwrite3d', [['.*']]],
        ['readimage3d', [['.*']]],
        ['readimage3d_int16', [['.*']]],
        ['readimage3d_fp32', [['.*']]],
        ['imagearraycopy', [['.*']]],
        ['imagearraycopy3d', [['.*']]],
        ['imagecopy', [['.*']]],
        ['imagecopy3d', [['.*']]],
        ['imagerandomcopy', [['.*']]],
        ['arrayimagecopy', [['.*']]],
        ['arrayimagecopy3d', [['.*']]],
        ['imagenpot', [['.*']]],
        ['image_param', [['.*']]],
        ['image_multipass_integer_coord', [['.*']]],
        ['image_multipass_float_coord', [['.*']]],
        ['enqueue_map_image', [['.*']]],
        ['imagedim_pow2', [['.*']]],
        ['imagedim_non_pow2', [['.*']]],
        ['mri_one', [['.*']]],
        ['mri_multiple', [['.*']]],
        # Disable some subtests because of bug #CSSD100006755
        ['astype', [['.*', '.*64']]],
        ['vector_creation', [['.*', '.*64']]],
        ['vload_global', [['.*', '.*64']]],
        ['vload_local', [['.*', '.*64']]],
        ['vload_constant', [['.*', '.*64']]],
        ['vload_private', [['.*', '.*64']]],
        ['vstore_global', [['.*', '.*64']]],
        ['vstore_local', [['.*', '.*64']]],
        ['vstore_private', [['.*', '.*64']]],
        ['parameter_types', [['.*', '.*64']]]]
        basicSuite = VolcanoReferenceConformanceTest("basic_common", config, skipTestList)
        self.addTask(basicSuite)

        # Framework tests
        skipTestList = [
        ['Test_clGetDeviceIDsTest', [['.*']]],
        ['Test_clGetPlatformInfoTest', [['.*']]],
        ['Test_clGetDeviceInfoTest', [['.*']]],
        ['Test_clCreateContextTest', [['.*']]],
        ['Test_clCreateBufferTest', [['.*']]],
        ['Test_clEnqueueCopyBufferTest', [['.*']]],
        ['Test_clCopyImageTest', [['.*']]],
        ['Test_MT_context_retain', [['.*']]],
        ['Test_MT_release', [['.*']]],
        ['Test_clNativeFunctionTest', [['.*']]],
        ['Test_EnqueueNativeKernelTest', [['.*']]],
        ['Test_EventCallbackTest', [['.*']]],
        ['Test_clFinishTest', [['.*']]],
        ['Test_clIntelOfflineCompilerTest', [['.*']]],
        ['Test_clIntelOfflineCompilerThreadsTest', [['.*']]],
        ['Test_clIntelOfflineCompilerBuildOptionsTest', [['.*']]],
        # Disable some subtests because of bug #CSSD100006755
        ['Test_clImageExecuteTest', [['.*', '.*64']]],
        ['Test_opencl_printf_test', [['.*', '.*64']]],
        ['Test_overloadingTest', [['.*', '.*64']]]
        ]
        frameworkSuite = VolcanoReferenceConformanceTest("framework_test_type", config, skipTestList)
        self.addTask(frameworkSuite)

        # CPUDevice
        CPUDeviceSuite = VolcanoReferenceConformanceTest("CPUDevice", config, [])
        self.addTask(CPUDeviceSuite)

        # Half
        halfSuite = VolcanoReferenceConformanceTest("half", config, [])
        self.addTask(halfSuite)

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
    parser.add_option("-t", "--target", dest="target_type", help="target type: " + str(SUPPORTED_TARGETS), default="Win32")
    parser.add_option("-b", "--build", dest="build_type", help="build type: " + str(SUPPORTED_BUILDS), default="Release")
    parser.add_option("-c", "--cpu",     dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size: " + str(SUPPORTED_VECTOR_SIZES), default="0")    
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    Volcano_CmdUtils.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoReferenceConformancePostCommit("Conformance_Test_Suite", config)
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


