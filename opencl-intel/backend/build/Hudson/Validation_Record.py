
'''
Captures OpenCL conformance suites. 
For given binaries path, test executable path and the name of conformance tests
it creates folder with captured data.
List of subtests of conformance test is specified inside the script
'''

from Volcano_Common import VolcanoRunConfig, VolcanoCmdTask, VolcanoTestSuite, \
    VolcanoTestRunner, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES, EnvironmentValue
from optparse import OptionParser
from subprocess import call
from Volcano_Conformance_Framework import FrameworkTestsNames
from Volcano_Conformance_Basic import BasicTestsNames
import csv
import glob
import shutil
import sys
import platform
import re
import os

'''
    Sample of line of CSV input file with tests 
    testName, testExe, testCmdLineParams   
'''

CPUDevice = [
'CPUDevice'
]

half = [
'vload_half', 
'vloada_half',
'vstore_half',
'vstorea_half',
'vstore_half_rte',
'vstorea_half_rte',
'vstore_half_rtz',
'vstorea_half_rtz',
'vstore_half_rtp',
'vstorea_half_rtp',
'vstore_half_rtn',
'vstorea_half_rtn',
'roundTrip'
]

images = [
 ['test_cl_copy_images', 'test_cl_copy_images', 'small_images', 0],
 ['test_cl_get_info', 'test_cl_get_info', 'small_images', 0],
 ['test_cl_read_write_images', 'test_cl_read_write_images', 'small_images', 0],
 ['test_kernel_image_methods', 'test_kernel_image_methods', 'small_images', 19968],
 ['test_image_streams', 'test_image_streams', 'small_images', 306]]

basic_images = [
 ['readimage', 'validation/conformance/basic/test_basic', 'readimage', 2],
 ['readimage_int16', 'validation/conformance/basic/test_basic', 'readimage_int16', 1],
 ['readimage_fp32', 'validation/conformance/basic/test_basic', 'readimage_fp32', 1],
 ['writeimage', 'validation/conformance/basic/test_basic', 'writeimage', 2],
 ['writeimage_int16', 'validation/conformance/basic/test_basic', 'writeimage_int16', 1],
 ['writeimage_fp32', 'validation/conformance/basic/test_basic', 'writeimage_fp32', 1],
 ['image_r8', 'validation/conformance/basic/test_basic', 'image_r8', 0],
 ['imagereadwrite', 'validation/conformance/basic/test_basic', 'imagereadwrite', 0],
 ['imagereadwrite3d', 'validation/conformance/basic/test_basic', 'imagereadwrite3d', 0],
 ['readimage3d', 'validation/conformance/basic/test_basic', 'readimage3d', 2],
 ['readimage3d_int16', 'validation/conformance/basic/test_basic', 'readimage3d_int16', 1],
 ['readimage3d_fp32', 'validation/conformance/basic/test_basic', 'readimage3d_fp32', 1],
 ['imagearraycopy', 'validation/conformance/basic/test_basic', 'imagearraycopy', 0],
 ['imagearraycopy3d', 'validation/conformance/basic/test_basic', 'imagearraycopy3d', 0],
 ['imagecopy', 'validation/conformance/basic/test_basic', 'imagecopy', 0],
 ['imagecopy3d', 'validation/conformance/basic/test_basic', 'imagecopy3d', 0],
 ['imagerandomcopy', 'validation/conformance/basic/test_basic', 'imagerandomcopy', 0],
 ['arrayimagecopy', 'validation/conformance/basic/test_basic', 'arrayimagecopy', 0],
 ['arrayimagecopy3d', 'validation/conformance/basic/test_basic', 'arrayimagecopy3d', 0],
 ['imagenpot', 'validation/conformance/basic/test_basic', 'imagenpot', 4],
 ['image_param', 'validation/conformance/basic/test_basic', 'image_param', 1],
 ['image_multipass_integer_coord', 'validation/conformance/basic/test_basic', 'image_multipass_integer_coord', 2],
 ['image_multipass_float_coord', 'validation/conformance/basic/test_basic', 'image_multipass_float_coord', 2],
 ['enqueue_map_image', 'validation/conformance/basic/test_basic', 'enqueue_map_image', 0],
 ['imagedim_pow2', 'validation/conformance/basic/test_basic', 'imagedim_pow2', 1],
 ['imagedim_non_pow2', 'validation/conformance/basic/test_basic', 'imagedim_non_pow2', 1],
 ['mri_one', 'validation/conformance/basic/test_basic', 'mri_one', 1],
 ['mri_multiple', 'validation/conformance/basic/test_basic', 'mri_multiple', 1]]

# Some tests in basic should be skipped due to NEAT limitations
basic_common_to_skip = {"hiloeo":[113, 114, 123, 124, 153, 154, 163, 164, 193, 194, 203, 204, 233, 234,   # list of subtest indexes which will be skipped because these subtests returns "undefined" integer values.
                                                                                                          # At the moment stand-alone doesn't support tests that produces "undefined" integer values.
                         243, 244, 273, 274, 283, 284, 3, 313, 314, 33, 34, 365, 4, 43, 44, 73, 74, 83, 84],
                     "astype":[ # list of tests that can't be passed due to integer 3-element vectors
                                114, 157, 199, 245, 283, 317, 325, 363, 371, 38, 7, 77] } 

# Some tests in basic_images should be skipped due to NEAT limitations
basic_images_to_skip = {
                        "imagedim_pow2":[1], # fail at very BIG images
                        "imagedim_non_pow2":[1]} # fail at very BIG images

# Some tests in test_half have to be skipped because of bug in Clang/Back-end 
# interoperability. See more about details at the description of the bug: CSSD100006755
# Here is the list with the indexes of sub-tests which are not supported by stand-alone because of bug mentioned above.
# ALL THESE TESTS FAIL BECAUSE OF CSS100006755
half_to_skip = {"vload_half":[5, 6, 7, 8, 21, 22, 23, 24], 
                "vloada_half":[5, 6, 7, 8, 21, 22, 23, 24], 
                "vstore_half":[7, 8, 9, 10, 31, 32, 33, 34, 35, 36], 
                "vstorea_half":[1, 2, 3, 25, 26, 27], 
                "vstore_half_rte":[7, 8, 9, 10, 31, 32, 33, 34, 35, 36], 
                "vstorea_half_rte":[1, 2, 3, 25, 26, 27], 
                "vstore_half_rtz":[7, 8, 9, 10, 31, 32, 33, 34, 35, 36], 
                "vstorea_half_rtz":[1, 2, 3, 25, 26, 27], 
                "vstore_half_rtp":[7, 8, 9, 10, 31, 32, 33, 34, 35, 36], 
                "vstorea_half_rtp":[1, 2, 3, 25, 26, 27], 
                "vstore_half_rtn":[7, 8, 9, 10, 31, 32, 33, 34, 35, 36], 
                "vstorea_half_rtn":[1, 2, 3, 25, 26, 27], 
                "roundTrip":[3, 4, 11, 12], 
}

framework_test_type_to_skip = {
                        "Test_clExecutionTest":[1], # CSSD100006765
                        "Test_clNativeFunctionTest":[1, # NEAT fails because NEAT interval bitcasts to the integer.
                        ]
                        }

CPUDevice_to_skip = {"CPUDevice":[1]} # Empty config file - there are no OpenCL kernels to capture.

class ConfTestDesc:
    def __init__(self, test_list, skip_list, success_code):
        self.test_list = test_list
        self.skip_list = skip_list
        self.success_code = success_code

# For each test there is list of tests and exceptions list
# Exceptions were created because some tests can't be passed due to NEAT limitations  
OclRefSuites = {"basic_common" : ConfTestDesc(BasicTestsNames, basic_common_to_skip, 0),
                "basic_images" : ConfTestDesc(basic_images, basic_images_to_skip, 0),
                "half" : ConfTestDesc(half, half_to_skip, 0),
                "framework_test_type" : ConfTestDesc(FrameworkTestsNames, framework_test_type_to_skip, 1),
                "CPUDevice" : ConfTestDesc(CPUDevice, CPUDevice_to_skip, 1),
                "images" : ConfTestDesc(images, [], 0)}

class RecordTest(VolcanoCmdTask):
    def __init__(self, name, cmdline, work_dir, output_dir):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = work_dir 
        self.output_dir = output_dir
        self.command = cmdline
        
    def startUp(self):
        VolcanoCmdTask.startUp(self)
        if platform.system() != 'Windows':
            os.environ['OCLBACKEND_PLUGINS'] = os.path.join('libOclRecorder.so')
        else:
            os.environ['OCLBACKEND_PLUGINS'] = os.path.join('OCLRecorder.dll')
        os.environ['OCLRECORDER_DUMPPREFIX'] = self.name
        os.environ['OCLRECORDER_LOGDIR'] = self.output_dir
        
    def tearDown(self):
        VolcanoCmdTask.tearDown(self)
        print '---------------------------------------------------'
        os.environ['OCLBACKEND_PLUGINS'] = ''
        os.environ['OCLRECORDER_DUMPPREFIX'] = ''
        os.environ['OCLRECORDER_LOGDIR'] = ''
        
class CaptureConformanceSuite(VolcanoTestSuite):
    def __init__(self, name, config, suite_desc, output_dir, test_dir):
        VolcanoTestSuite.__init__(self, name)
        self.config = config
        self.output_dir = os.path.abspath(output_dir)
        self.test_dir = os.path.abspath(test_dir)
        for test_desc in suite_desc.test_list:
            if(len(test_desc) < 3):
                print "Test description is invalid: " + str(test_desc)
                continue;
            testName = test_desc[0]
            testExe = test_desc[1] 
            testCmdLineParams = test_desc[2]
            if platform.system() != 'Windows':
                testCmdLine = testExe + ' ' + testCmdLineParams
            else:
                testCmdLine = os.path.join(self.test_dir, testExe) + ' ' + testCmdLineParams
            rec = RecordTest(testName, testCmdLine, self.test_dir, self.output_dir)
            rec.success_code = suite_desc.success_code
            self.addTask(rec, stop_on_failure=False, always_pass=False)

    def startUp(self):
        print '[CaptureConformance]: Creating output dir ' + self.output_dir + '\n'
        # create output dir if it doesn't exist
        if not os.path.exists(self.output_dir):
            os.mkdir(self.output_dir)
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
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32, Win64, Linux64", default='Win32')
    parser.add_option("-c", "--cpu", dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features:+avx,-avx256", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size: 0(auto),1,4,8,16", default="0")
    parser.add_option('-b', '--build_type', dest='build_type', help='Build type. Possible values. Allowed values: ' + str(SUPPORTED_BUILDS))
    parser.add_option("-o", "--out_dir", dest="out_dir", help="Output directory", default='Recorded_data')
    parser.add_option("-d", "--test_dir", dest="test_dir", help="Directory with conformance tests executable", default=None)
    parser.add_option("-s", "--suite", dest="suite", help="Suite to capture.", default=None)

    (options, _notUsed) = parser.parse_args()
    
    config = VolcanoRunConfig(options.root_dir,
                              options.target_type,
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    runner = VolcanoTestRunner()
    suite = CaptureConformanceSuite("Capture_Conformance_Suite",
                                    config,
                                    OclRefSuites[options.suite],
                                    options.out_dir,
                                    options.test_dir)
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
        sys, exit(1)
    main_result = main()
    sys.exit(main_result)
