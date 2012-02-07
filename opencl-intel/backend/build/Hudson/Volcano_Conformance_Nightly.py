import os, sys, platform
from optparse import OptionParser
import framework.cmdtool
from framework.core import VolcanoTestRunner, VolcanoTestSuite, TIMEOUT_MINUTE, TIMEOUT_HALFHOUR, TIMEOUT_HOUR, TIMEOUT_HOURANDHALF, TIMEOUT_DAY
from framework.utils import EnvironmentValue
from framework.tasks import SimpleTest
from Volcano_Common import VolcanoRunConfig, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_WOLF import VolcanoWolfNightly, VolcanoWolfPerformance
from Volcano_Conformance_Framework import VolcanoConformanceFramework
from Volcano_Conformance_Basic import VolcanoConformanceBasic

class VolcanoConformanceNightly(VolcanoTestSuite):
    def __init__(self, config):
        VolcanoTestSuite.__init__(self,"Conformance")
        self.config = config
        self.conformance_dir     = os.path.join(config.bin_dir, 'validation', 'conformance')
        self.cpu_device_test_dir = os.path.join(config.bin_dir, 'validation', 'cpu_device_test_type')

        # Framework
        frameworkTest = VolcanoConformanceFramework(config)
        frameworkTest.timeout = TIMEOUT_HOURANDHALF
        self.addTask(frameworkTest)

        # CPUDevice
        cpuDeviceTest = SimpleTest('CPUDevice', config.bin_dir, os.path.join('validation', 'cpu_device_test_type','cpu_device_test_type'))
        cpuDeviceTest.success_code = 1 
        cpuDeviceTest.timeout = TIMEOUT_HALFHOUR
        self.addTask(cpuDeviceTest)

        # WOLF benchmark
        # wolfNightly = VolcanoWolfNightly("WOLF_Benchmark", config, capture_data =  False)
        # self.addTask(wolfNightly, stop_on_failure = False, always_pass = False, skiplist=[['.*','SLES64'],['.*','RH64']])

        # wolfPerf = VolcanoWolfPerformance("WOLF_Performance", config, capture_data =  False)
        # self.addTask(wolfPerf, stop_on_failure = False, always_pass = False, skiplist=[['.*','SLES64'],['.*','RH64']])

        # #########################################    
        # Basic Information on the compute device    
        # #########################################    
        runTest = SimpleTest('Compute Info', self.conformance_dir, os.path.join('computeinfo','computeinfo'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)
            
        # #########################################    
        # Basic operation tests    
        # #########################################    
        runTest = VolcanoConformanceBasic(config)
        self.addTask(runTest, timeout = TIMEOUT_HOUR)
        
        runTest = SimpleTest('API', self.conformance_dir, os.path.join('api','test_api'))
        self.addTask(runTest, timeout = 2 * TIMEOUT_MINUTE)

        runTest = SimpleTest('Compiler', os.path.join(self.conformance_dir,'compiler'), 'test_compiler')
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        # #########################################    
        # Common mathematical functions    
        # #########################################
        runTest = SimpleTest('Common Functions', self.conformance_dir, os.path.join('commonfns','test_commonfns'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Geometric Functions', self.conformance_dir, os.path.join('geometrics','test_geometrics'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Relationals', self.conformance_dir, os.path.join('relationals','test_relationals'))
        self.addTask(runTest, timeout = TIMEOUT_HOUR)

        # #########################################    
        # General operation    
        # #########################################    
        runTest = SimpleTest('Thread Dimensions', self.conformance_dir, os.path.join('thread_dimensions','test_thread_dimensions') + ' full*')
        self.addTask(runTest, timeout = TIMEOUT_HALFHOUR)

        runTest = SimpleTest('Multiple Device Context', self.conformance_dir, os.path.join('multiple_device_context','test_multiples'))
        self.addTask(runTest, timeout = 5 * TIMEOUT_MINUTE)

        runTest = SimpleTest('Atomics', self.conformance_dir, os.path.join('atomics','test_atomics'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Profiling', self.conformance_dir, os.path.join('profiling','test_profiling'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Events', self.conformance_dir, os.path.join('events','test_events'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Allocations single maximum', self.conformance_dir, os.path.join('allocations','test_allocations') + ' single 5 all')
        self.addTask(runTest, timeout = 6 * TIMEOUT_MINUTE)

        runTest = SimpleTest('Allocations total maximum', self.conformance_dir, os.path.join('allocations','test_allocations') + ' multiple 5 all ')
        self.addTask(runTest, timeout = 20 * TIMEOUT_MINUTE)

        runTest = SimpleTest('VecAlign', self.conformance_dir, os.path.join('vec_align','test_vecalign'))
        self.addTask(runTest, timeout = 10 * TIMEOUT_MINUTE)

        runTest = SimpleTest('VecStep', self.conformance_dir, os.path.join('vec_step','test_vecstep'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

            
        # #########################################    
        # Buffers and images    
        # #########################################    
        runTest = SimpleTest('Buffers', self.conformance_dir, os.path.join('buffers','test_buffers'))
        self.addTask(runTest, timeout = 5 * TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-API Info', self.conformance_dir, os.path.join('images/clGetInfo','test_cl_get_info'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-Kernel Methods', self.conformance_dir, os.path.join('images/kernel_image_methods','test_kernel_image_methods'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-Kernel', self.conformance_dir, os.path.join('images/kernel_read_write','test_image_streams') + ' CL_FILTER_NEAREST')
        self.addTask(runTest, timeout = TIMEOUT_HOURANDHALF)

        runTest = SimpleTest('Images-Kernel pitch', self.conformance_dir, os.path.join('images/kernel_read_write','test_image_streams') + ' use_pitches CL_FILTER_NEAREST')
        self.addTask(runTest, timeout = 2 * TIMEOUT_HOUR)

        runTest = SimpleTest('Images-Kernel max size', self.conformance_dir, os.path.join('images/kernel_read_write','test_image_streams') + ' max_images CL_FILTER_NEAREST')
        self.addTask(runTest, timeout = 5 * TIMEOUT_HOUR, skiplist=[['.*']])

        runTest = SimpleTest('Images-clCopyImage', self.conformance_dir, os.path.join('images/clCopyImage','test_cl_copy_images'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-clCopyImage small', self.conformance_dir, os.path.join('images/clCopyImage','test_cl_copy_images') + ' small_images')
        self.addTask(runTest, timeout = 2 * TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-clCopyImage max size', self.conformance_dir, os.path.join('images/clCopyImage','test_cl_copy_images') + ' max_images ')
        self.addTask(runTest, timeout = 20 * TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-clReadWriteImage', self.conformance_dir, os.path.join('images/clReadWriteImage','test_cl_read_write_images'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-clReadWriteImage pitch', self.conformance_dir, os.path.join('images/clReadWriteImage','test_cl_read_write_images') + ' use_pitches ')
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Images-clReadWriteImage max size', self.conformance_dir, os.path.join('images/clReadWriteImage','test_cl_read_write_images') + ' max_images ')
        self.addTask(runTest, timeout = 20 * TIMEOUT_MINUTE)

            
        # #########################################    
        # Headers    
        # #########################################    
        runTest = SimpleTest('Headers-cl_typen', self.conformance_dir, os.path.join('headers','test_headers'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Headers-cl_h standalone', self.conformance_dir, os.path.join('headers','test_cl_h'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Headers-cl_platform_h standalone', self.conformance_dir, os.path.join('headers','test_cl_platform_h'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Headers-cl_gl_h standalone', self.conformance_dir, os.path.join('headers','test_cl_gl_h'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Headers-opencl_h standalone', self.conformance_dir, os.path.join('headers','test_opencl_h'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

            
        # #########################################    
        # CPU is required to pass linear and normalized image filtering    
        # #########################################    
        runTest = SimpleTest('Images-Kernel CL_FILTER_LINEAR', self.conformance_dir, os.path.join('images/kernel_read_write','test_image_streams') + ' CL_FILTER_LINEAR')
        self.addTask(runTest, timeout = TIMEOUT_HOURANDHALF)

        runTest = SimpleTest('Images-Kernel CL_FILTER_LINEAR pitch', self.conformance_dir, os.path.join('images/kernel_read_write','test_image_streams') + ' use_pitches CL_FILTER_LINEAR')
        self.addTask(runTest, timeout = TIMEOUT_HOURANDHALF)

        runTest = SimpleTest('Images-Kernel CL_FILTER_LINEAR max size', self.conformance_dir, os.path.join('images/kernel_read_write','test_image_streams') + ' max_images CL_FILTER_LINEAR')
        self.addTask(runTest, timeout = 3* TIMEOUT_HOUR, skiplist=[['.*']])


            
        # #########################################    
        # OpenGL/CL interaction    
        # #########################################    
        #OpenCL-GL Sharing    gl/test_gl
            
        # #########################################    
        # Thorough math and conversions tests    
        # #########################################    
        runTest = SimpleTest('Select', self.conformance_dir, os.path.join('select','test_select'))
        self.addTask(runTest, timeout = 4 * TIMEOUT_HOUR)

        runTest = SimpleTest('Conversions', self.conformance_dir, os.path.join('conversions','test_conversions' ) + ' -w')
        self.addTask(runTest, timeout = TIMEOUT_HOURANDHALF)

        runTest = SimpleTest('Contractions', self.conformance_dir, os.path.join('contractions','contractions'))
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

        runTest = SimpleTest('Math', self.conformance_dir, os.path.join('math_brute_force','bruteforce') + ' -w')
        self.addTask(runTest, timeout = TIMEOUT_HALFHOUR)

        runTest = SimpleTest('Integer Ops', self.conformance_dir, os.path.join('integer_ops','test_integer_ops'))
        self.addTask(runTest, timeout = 5 * TIMEOUT_HOUR)

        runTest = SimpleTest('Half Ops', self.conformance_dir, os.path.join('half','test_half') + ' -w')
        self.addTask(runTest, timeout = TIMEOUT_MINUTE)

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
    parser.add_option("-t", "--target",    action="store",      choices=SUPPORTED_TARGETS, dest="target_type",  default="Win32",   help="Target type: " + str(SUPPORTED_TARGETS) + ". [Default: %default]")
    parser.add_option("-b", "--build_type",action="store",      choices=SUPPORTED_BUILDS,  dest="build_type",   default="Release", help="Build type: " + str(SUPPORTED_BUILDS) + ". [Default: %default]")
    parser.add_option("-c", "--cpu",       action="store",      choices=SUPPORTED_CPUS,    dest="cpu",          default="auto",    help="CPU Type: " + str(SUPPORTED_CPUS) + ". [Default: %default]")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features", default="")
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size:0(auto),1,4,8,16", default="0")    
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    framework.cmdtool.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoConformanceNightly(config)
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


