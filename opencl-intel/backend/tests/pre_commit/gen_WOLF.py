cases = [
['wlFrameworkOverhead', '1', 'length=16,wg_size=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlFrameworkOverhead', '2', 'length=1024,wg_size=1024,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlTCC', '1', 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_vector8_third_optimization,TCC_CFG_GLOBAL_WORKGROUP_SIZE=8,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlTCC', '2', 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlTCC', '3', 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_vector8_third_optimization,TCC_CFG_GLOBAL_WORKGROUP_SIZE=8,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlTCC', '4', 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlBitonicSort', '1', 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlBitonicSort', '2', 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlIntelBitonicSort', '1', 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlIntelBitonicSort', '2', 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMandelbrot', '', 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_mandelbrot', '', 'iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlConvolution', '', 'iterations=1,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIDCT', '', 'CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlDCT', '', 'CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '1', 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '2', 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '3', 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '4', 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '5', 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '6', 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATINBody', '7', 'kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_NBody', '', 'szBodies=32768,szGlobalWork=32768,groupSize=4,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIPrefixSum', '', 'length=2048,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlPrefixSum', '1', 'iterations=1,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlPrefixSum', '2', 'iterations=1,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=4,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIHistogram', '', 'iterations=1,width=1024,height=1024,wg_size=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlHistogram', '', 'bMeasureCreateBuffer=False,szGlobalWorkStep1=1,szGlobalWorkStep2=1,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIAES', '1', 'decrypt=false,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIAES', '2', 'decrypt=true,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlNVMedian', '', 'iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_median', '', 'iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIFFT', '', 'iterations=1,length=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATISobelFilter', '', 'iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_sobel', '', 'iterations=1' ],
['wlATIRadixSort', '', 'iterations=1,elementCount=8192,groupSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIReduction', '1', 'length=256,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIReduction', '2', 'length=1024,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIReduction', '3', 'length=16384,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIReduction', '4', 'length=1048576,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIRecGaussian', '', 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_RecGaussian', '', 'eDeviceTypes=CPU,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=16,GAUSSIAN_CFG_BLOCK_DIM=8' ],
['wlATIFastWalsh', '1', 'length=256,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIFastWalsh', '2', 'length=1024,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIFastWalsh', '3', 'length=16384,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIFastWalsh', '4', 'length=524288,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIFastWalsh', '5', 'length=1048576,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMersenneTwister', '1', 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMersenneTwister', '2', 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMersenneTwister', '3', 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIMonteCarlo', '', 'blockSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_MonteCarlo', '', 'iterations=1,WorkGroupSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMatrixMultiplication', '1', 'CFG_MATRIX_MUL_WIDTH=128,CFG_MATRIX_MUL_HEIGHT=128,CFG_MATRIX_MUL_DEPTH=128,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMatrixMultiplication', '2', 'CFG_MATRIX_MUL_WIDTH=512,CFG_MATRIX_MUL_HEIGHT=512,CFG_MATRIX_MUL_DEPTH=512,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlMatrixMultiplication', '3', 'CFG_MATRIX_MUL_WIDTH=1024,CFG_MATRIX_MUL_HEIGHT=1024,CFG_MATRIX_MUL_DEPTH=1024,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_matrix_mult', '1', 'cfgFile=intel_matrix_mult.cfg,KernelType=ScalarOCL' ],
['intel_matrix_mult', '2', 'cfgFile=intel_matrix_mult.cfg,KernelType=VectorOCL' ],
['wlATIBinOption', '', 'CFG_BIN_OPTION_NUM_SAMPLES=64,CFG_BIN_OPTION_NUM_STEPS=64,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIMatrixTrans', '1', 'width=128,height=128,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIMatrixTrans', '2', 'width=512,height=512,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIMatrixTrans', '3', 'width=1024,height=1024,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIMatrixTrans', '4', 'width=4096,height=4096,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIQuasiSeq', '', 'iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIDwtHaar1D', '', 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,signalLength=32768,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIFloydWarshall', '', 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,numNodes=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlBinarySearch', '1', 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlBinarySearch', '2', 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlBinarySearch', '3', 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIEigenValue', '', 'globalSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIScanArrays', '1', 'iterations=1,length=128,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIScanArrays', '2', 'iterations=1,length=1024,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIScanArrays', '3', 'iterations=1,length=16384,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '1', 'globalSize1=64,globalSize2=64,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '2', 'globalSize1=128,globalSize2=128,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '3', 'globalSize1=512,globalSize2=512,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '4', 'globalSize1=1024,globalSize2=1024,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '5', 'globalSize1=64,globalSize2=64,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '6', 'globalSize1=128,globalSize2=128,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '7', 'globalSize1=512,globalSize2=512,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['wlATIBAS', '8', 'globalSize1=1024,globalSize2=1024,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false' ],
['intel_god_rays', '', 'intel_god_rays.cfg' ],
#['AIF_RadialBlur', '', 'AIF_RadialBlur.cfg' ],
['intel_radial_blur', '', 'intel_radial_blur.cfg' ],
['invertRGB', '', 'invertRGB.cfg' ],
['wlSimpleBoxBlur', '', 'wlSimpleBoxBlur.cfg' ],
['crossfade', '', 'crossfade.cfg' ],
['twirl', '', 'twirl.cfg' ],
['checkerboard', '', 'checkerboard.cfg' ],
#['wlSobel', '', 'wlSobel.cfg' ],
#['droste', '', 'droste.cfg' ],
['bilateral2D', '', 'bilateral2D.cfg' ],
['intel_bilateral2D', '', 'intel_bilateral2D.cfg' ],
['wlAdobe_PB_Pixelate', '', 'wlAdobe_PB_Pixelate.cfg' ],
['wl2Landscape', '', 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU']
]

# include content of common file
# by including instead of import we can have a self contained script
with open('my_unittest.py', 'r') as f:
    print f.read()

print '''
class WOLFTestSuite(MyTestCase):
    def __init__(self, methodName='runTest'):        
        MyTestCase.__init__(self, methodName, 'WOLF.exe', 600)

    def helper_run_workload(self, workload, config):
        """Runs a test suite"""
        command = ' '.join([self.UUT_bin, 'out/'+workload+'.out', workload, '1', config])
        
        returncode, out = self.CLT.runCommand(command)
        self.assertEqual(returncode, 0, out)'''

for wl, num, opt in cases:
    print """
    def test_"""+wl+num+"""(self):
        '''"""+wl+"""'''
        self.helper_run_workload(
                '"""+wl+"""', 
                '"""+opt+"""'
            )
"""


print '''if __name__ == '__main__':
    import sys

    unittest.main()
'''


