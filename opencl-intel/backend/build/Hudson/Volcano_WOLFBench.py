from optparse import OptionParser
import  sys, platform
import framework.cmdtool
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestSuite, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Tasks import WOLFBenchTest

Wolfbench = [
['intel_NBody', 1, 'cfgFile=intel_NBody.cfg,Platform=0,DeviceType=CPU'],
['intel_bitonic_sort', 1, 'cfgFile=intel_bitonic_sort.cfg,Platform=0,DeviceType=CPU'],
['intel_convolution', 1, 'cfgFile=intel_convolution.cfg,Platform=0,DeviceType=CPU'],
['intel_mandelbrot', 1, 'cfgFile=intel_Mandelbrot.cfg,Platform=0,DeviceType=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,Platform=0,DeviceType=CPU'],
['intel_median_bitonic', 1, 'cfgFile=intel_median_bitonic.cfg,Platform=0,DeviceType=CPU'],
['intel_radar', 1, 'cfgFile=intel_radar.cfg,Platform=0,DeviceType=CPU'],
['intel_sobel', 1, 'cfgFile=intel_sobel.cfg,Platform=0,DeviceType=CPU'],
['intel_DCT', 1, 'cfgFile=intel_DCT.cfg,Platform=0,DeviceType=CPU'],
['intel_MonteCarlo', 1, 'cfgFile=intel_MonteCarlo.cfg,Platform=0,DeviceType=CPU'],
['intel_crossfade', 1, 'cfgFile=intel_crossfade.cfg,Platform=0,DeviceType=CPU'],
['intel_tone_mapping', 1, 'cfgFile=intel_tone_mapping.cfg,Platform=0,DeviceType=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,Platform=0,DeviceType=CPU'],
['ati_NBody', 1, 'cfgFile=ati_NBody.cfg,Platform=0,DeviceType=CPU'],
['ati_matrix_mult', 1, 'cfgFile=ati_matrix_mult.cfg,Platform=0,DeviceType=CPU'],
['ati_sobel', 1, 'cfgFile=ati_sobel.cfg,Platform=0,DeviceType=CPU'],
['ati_mandelbrot', 1, 'cfgFile=ati_Mandelbrot.cfg,Platform=0,DeviceType=CPU'],
['ati_bitonic_sort', 1, 'cfgFile=ati_bitonic_sort.cfg,Platform=0,DeviceType=CPU'],
['ati_DCT', 1, 'cfgFile=ati_DCT.cfg,Platform=0,DeviceType=CPU'],
['ati_aes', 1, 'cfgFile=ati_aes.cfg,Platform=0,DeviceType=CPU'],
['ati_monte_carlo_asian', 1, 'cfgFile=ati_monte_carlo_asian.cfg,Platform=0,DeviceType=CPU'],
['ati_RecGaussian', 1, 'Platform=0,DeviceType=CPU'],
['intel_RecGaussian', 1, 'Platform=0,DeviceType=CPU'],
['intel_tcc', 1, 'cfgFile=intel_tcc.cfg,Platform=0,DeviceType=CPU'],
['intel_ace', 1, 'cfgFile=intel_ace.cfg,Platform=0,DeviceType=CPU'],
['intel_god_rays', 1, 'cfgFile=intel_god_rays.cfg,Platform=0,DeviceType=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,Platform=0,DeviceType=CPU'],
['nvidia_median_filter', 1, 'cfgFile=nvidia_median_filter.cfg,Platform=0,DeviceType=CPU'],
['nvidia_bitonic_sort', 1, 'cfgFile=nvidia_bitonic_sort.cfg,Platform=0,DeviceType=CPU'],
['ati_BinOption', 1, 'cfgFile=ati_BinOption.cfg,Platform=0,DeviceType=CPU'],
['ati_FFT', 1, 'Platform=0,DeviceType=CPU'],
['ati_SobelFilterImage', 1, 'Platform=0,DeviceType=CPU'],
['ati_MersenneTwister', 1, 'cfgFile=ati_MersenneTwister.cfg,Platform=0,DeviceType=CPU'],
['ati_ScanLargeArrays', 1, 'length=16777216,Platform=0,DeviceType=CPU'],
['ati_HistogramAtomics', 1, 'Platform=0,DeviceType=CPU'],
['ati_BlackScholes', 1, 'cfgFile=ati_BlackScholes.cfg,Platform=0,DeviceType=CPU'],
['ati_DwtHaar1D', 1, 'cfgFile=ati_DwtHaar1D.cfg,Platform=0,DeviceType=CPU'],
['ati_MatrixTranspose', 1, 'cfgFile=ati_MatrixTranspose.cfg,Platform=0,DeviceType=CPU'],
['ati_matrix_mult_image', 1, 'cfgFile=ati_matrix_mult_image.cfg,Platform=0,DeviceType=CPU'],
]

class VolcanoWolfBench(VolcanoTestSuite):
    def __init__(self, name, config, tests, capture_data):
        VolcanoTestSuite.__init__(self, name)

        for test in tests:
            self.addTask( WOLFBenchTest( self.getTestName(test[0]), test[0], test[1], test[2], config, capture_data));

    def getTestName(self, test):
        tname = test
        count = 1
        while tname in self.getTasksNames():
            tname = '_'.join([test, str(count)])
            count += 1
        return tname

def main():
    parser = OptionParser()
    parser.add_option("-r", "--root", dest="root_dir", help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32/64,Linux64", default="Win32")
    parser.add_option("-b", "--build", dest="build_type", help="build type: Debug, Release", default="Release")
    parser.add_option("-c", "--cpu",  dest="cpu", help="CPU Type: " + str(SUPPORTED_CPUS), default="auto")
    parser.add_option("-f", "--cpu-features", dest="cpu_features", help="CPU features", default="")
    parser.add_option("-s", "--suite", dest="suite", help="Suite to run: Performance[New,Conf,NewConf], Benchmark, Fast", default=None)
    parser.add_option("-v", "--vec", dest="transpose_size", help="Tranpose Size:0(auto),1,4,8,16", default="0")
    parser.add_option("-p", "--capture", action="store_true", dest="capture", help="Capture the data", default=False)
    parser.add_option("-d", "--demo", action="store_true", dest="demo_mode", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()
    framework.cmdtool.demo_mode = options.demo_mode

    suites = {"Wolfbench": Wolfbench}

    if(options.suite not in suites):
        print "Unsupported suite '" + options.suite + "'. The suite must be Wolfbench."
        return 1

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoWolfBench(options.suite, config, suites[options.suite], capture_data = options.capture)
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


