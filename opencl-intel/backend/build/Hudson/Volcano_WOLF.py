import  sys, platform
import framework.cmdtool
from framework.core import VolcanoTestRunner, VolcanoTestSuite
from optparse import OptionParser
from Volcano_Common import VolcanoRunConfig, SUPPORTED_CPUS, SUPPORTED_TARGETS, SUPPORTED_BUILDS, SUPPORTED_VECTOR_SIZES
from Volcano_Tasks import WOLFTest

WolfPerformanceFull = [
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['wlMatrixMultiplication', 1, 'cfgFile=wlMatrixMultiplication.cfg,eDeviceTypes=CPU'],
['wlATIPrefixSum', 1, 'cfgFile=wlATIPrefixSum.cfg,eDeviceTypes=CPU'],
['wlFrameworkOverhead', 1, 'cfgFile=measure.cfg,length=16777216,wg_size=1024,iterations=1000,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['wlDCT', 1, 'cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=100,groupSize=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['wlHistogram', 1, 'cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU'],
['intel_median', 1, 'cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['wlPrefixSum', 1, 'cfgFile=measure.cfg,iterations=1000,szArray=67108864,szMaxInt=1000,szLocalWork=8,szGlobalWork=1024,bPrintSerialReport=true,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['wlNVMedian', 1, 'cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,eDeviceTypes=CPU'],
['intel_god_rays', 1, 'cfgFile=intel_god_rays.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8'],
['intel_MonteCarlo', 1, 'cfgFile=measure.cfg,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU,WorkGroupSize=8'],
['wlConvolution', 1, 'cfgFile=measure.cfg,iterations=100,CFG_CONVOLUTION_ARRAY_WIDTH=1920,CFG_CONVOLUTION_ARRAY_HEIGHT=1080,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,eDeviceTypes=CPU'],
['wlATINBody', 1, 'cfgFile=measure.cfg,kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=10,eDeviceTypes=CPU'],
['intel_NBody', 1, 'cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_sobel', 1, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8'],
['intel_sobel8u', 1, 'cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_convolution', 1, 'cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU'],
['intel_mandelbrot', 1, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,localSize=128,eDeviceTypes=CPU'],
['intel_rgb2yuv', 1, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU'],
['wlATIAES', 1, 'cfgFile=wlATIAES.cfg,decrypt=true,eDeviceTypes=CPU'],
['wlBitonicSort', 1, 'cfgFile=wlBitonicSort.cfg,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,eDeviceTypes=CPU'],
['wlATIRecGaussian', 1, 'cfgFile=wlATIRecGaussian.cfg,eDeviceTypes=CPU,iterations=100'],
['intel_tcc', 1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,iterations=100,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,eDeviceTypes=CPU'],
['intel_RecGaussian', 1, 'cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_colorbalance', 1, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_crossfade', 1, 'cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_gauss', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['intel_radar', 1, 'cfgFile=measure.cfg,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8'],
['intel_bas', 1, 'cfgFile=measure.cfg,iterations=500,KernelType=ScalarOCL,eDeviceTypes=CPU'],
['wlATIRadixSort', 1, 'cfgFile=measure.cfg,iterations=500,KernelType=ScalarOCL,eDeviceTypes=CPU,elementCount=65536'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,KernelType=VectorOCL,eDeviceTypes=CPU'],
['checkerboard', 1, 'cfgFile=checkerboard.cfg,CFG_checkerboard_LOCAL_WORK_SIZE0=256,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wlDCT', 1, 'cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=100,groupSize=8,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wlHistogram', 1, 'cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU'],
['intel_median', 1, 'cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_god_rays', 1, 'cfgFile=intel_god_rays.cfg,KernelType=VectorOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8'],
['intel_MonteCarlo', 1, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU,WorkGroupSize=8'],
['intel_NBody', 1, 'cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_sobel', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_sobel8u', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_convolution', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_mandelbrot', 1, 'cfgFile=measure.cfg,localSize=8,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,eDeviceTypes=CPU'],
['intel_rgb2yuv', 1, 'cfgFile=measure.cfg,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_RecGaussian', 1, 'cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_tone_mapping', 1, 'cfgFile=intel_tone_mapping.cfg,KernelType=VectorOCL,iterations=100,CFG_intel_tone_mapping_LOCAL_WORK_SIZE=8,eDeviceTypes=CPU'],
['wlTCCACE', 1, 'cfgFile=wlTCCACE.cfg'],
['wlLcs', 1, 'cfgFile=wlLcs.cfg,KernelType=VectorOCL'],
['intel_colorbalance', 1, 'cfgFile=measure.cfg,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wlSimpleBoxBlur', 1, 'cfgFile=measure.cfg,CFG_SIMPLEBOXBLUR_LOCAL_THREAD_0=4,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_crossfade', 1, 'cfgFile=measure.cfg,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_gauss', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_radar', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],
['intel_bas', 1, 'cfgFile=measure.cfg,iterations=500,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wlMersenneTwister', 1, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wl2Landscape', 1, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],
['wlSubdivision', 1, 'cfgFile=subdivision.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU,MeshFileName=bigguy.obj'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=1,KernelType=ScalarOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=1,KernelType=ScalarOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=1,KernelType=VectorOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=1,KernelType=VectorOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=1,KernelType=ScalarOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=1,KernelType=ScalarOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=1,KernelType=VectorOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=1,KernelType=VectorOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=1,BLOCK_SIZE1=1,KernelType=ScalarOCL,USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=1,BLOCK_SIZE1=1,KernelType=ScalarOCL,USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=1,BLOCK_SIZE1=1,KernelType=VectorOCL,USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=1,BLOCK_SIZE1=1,KernelType=VectorOCL,USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=1,KernelType=ScalarOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=1,KernelType=ScalarOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=1,KernelType=VectorOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=1,KernelType=VectorOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE0=8,CFG_intel_radial_blur_LOCAL_WORK_SIZE1=1,KernelType=ScalarOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE0=8,CFG_intel_radial_blur_LOCAL_WORK_SIZE1=1,KernelType=ScalarOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE0=8,CFG_intel_radial_blur_LOCAL_WORK_SIZE1=1,KernelType=VectorOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE0=8,CFG_intel_radial_blur_LOCAL_WORK_SIZE1=1,KernelType=VectorOCL,CFG_intel_radial_blur_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE0=8,CFG_intel_bilateral2D_LOCAL_WORK_SIZE1=1,KernelType=ScalarOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE0=8,CFG_intel_bilateral2D_LOCAL_WORK_SIZE1=1,KernelType=ScalarOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE0=8,CFG_intel_bilateral2D_LOCAL_WORK_SIZE1=1,KernelType=VectorOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE0=8,CFG_intel_bilateral2D_LOCAL_WORK_SIZE1=1,KernelType=VectorOCL,CFG_intel_bilateral2D_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=8,BLOCK_SIZE1=1,KernelType=ScalarOCL,USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=8,BLOCK_SIZE1=1,KernelType=ScalarOCL,USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=8,BLOCK_SIZE1=1,KernelType=VectorOCL,USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,BLOCK_SIZE0=8,BLOCK_SIZE1=1,KernelType=VectorOCL,USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=0,eDeviceTypes=CPU'],
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,CFG_MEDIAN_USE_IMAGE_DATA_TYPE=1,eDeviceTypes=CPU'],
]

WolfPerformance = [
['intel_matrix_mult',       16, 'cfgFile=${intel_matrix_mult}.cfg,KernelType=ScalarOCL'],
['wlMatrixMultiplication',  16, 'cfgFile=wlMatrixMultiplication.cfg'],
['wlFrameworkOverhead',     16, 'length=16777216,wg_size=1024,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_bilateral2D',       16, 'cfgFile=${intel_bilateral2D}.cfg,KernelType=ScalarOCL,iterations=1'],
['wlDCT',                   16, 'KernelType=ScalarOCL,CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,groupSize=4'],
['wlHistogram',             16, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=64,szGlobalWorkStep2=64,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=1024,szMatrix=8388608,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,bPrintSerialReport=true'],
['intel_median',            16, 'KernelType=ScalarOCL,iterations=10,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlPrefixSum',             16, 'iterations=100,bMeasureCreateBuffer=False,szArray=4194304,szMaxInt=1000,szLocalWork=4,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,bPrintSerialReport=true'],
['intel_radial_blur',       16, 'cfgFile=${intel_radial_blur}.cfg,KernelType=ScalarOCL,CFG_intel_radial_blur_LOCAL_WORK_SIZE=4,iterations=6'],
['wlNVMedian',              16, 'iterations=10,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_god_rays',          16, 'cfgFile=${intel_god_rays}.cfg,KernelType=ScalarOCL,CFG_intel_god_rays_LOCAL_WORK_SIZE=4,iterations=10'],
['intel_MonteCarlo',        16, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1'],
['wlConvolution',           16, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMandelbrot',            16, 'CFG_MANDELBROT_WIDTH=2048,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              16, 'kernelName=nbody_sim,numParticles=4096,groupSize=128,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_NBody',             16, 'cfgFile=measure.cfg,KernelType=ScalarOCL,szBodies=4096,szGlobalWork=4096,groupSize=4,iterations=10'],
['intel_sobel',             16, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=100'],
['intel_convolution',       16, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1000,groupSize=4'],
['intel_mandelbrot',        16, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=100,localSize=128'],
['intel_rgb2yuv',           16, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=100,groupSize=4'],
['wlATIAES',                16, 'cfgFile=wlATIAES.cfg,decrypt=true'],
['wlBitonicSort',           16, 'cfgFile=wlBitonicSort.cfg,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128'],
['wlATIRecGaussian',        16, 'cfgFile=wlATIRecGaussian.cfg,iterations=100'],
['wlTCC',                   16, 'cfgFile=measure.cfg,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=10,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64'],
['intel_RecGaussian',       16, 'cfgFile=intel_RecGaussian.cfg,KernelType=ScalarOCL,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,iterations=100'],
['wlATIPrefixSum',          16, 'cfgFile=wlATIPrefixSum.cfg,iterations=100'],
['wl2Landscape',            10, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],
]

WolfPerformanceConf = [
['intel_matrix_mult',       1, 'cfgFile=${intel_matrix_mult}.cfg,KernelType=ScalarOCL'],
['wlMatrixMultiplication',  1, 'cfgFile=wlMatrixMultiplication.cfg'],
['wlFrameworkOverhead',     1, 'length=16777216,wg_size=1024,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_bilateral2D',       1, 'cfgFile=${intel_bilateral2D}.cfg,KernelType=ScalarOCL,iterations=1'],
['wlDCT',                   1, 'KernelType=ScalarOCL,CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,groupSize=4'],
['wlHistogram',             1, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=64,szGlobalWorkStep2=64,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=1024,szMatrix=8388608,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,bPrintSerialReport=true'],
['intel_median',            1, 'KernelType=ScalarOCL,iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlPrefixSum',             1, 'iterations=1,bMeasureCreateBuffer=False,szArray=4194304,szMaxInt=1000,szLocalWork=4,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,bPrintSerialReport=true'],
['intel_radial_blur',       1, 'cfgFile=${intel_radial_blur}.cfg,KernelType=ScalarOCL,CFG_intel_radial_blur_LOCAL_WORK_SIZE=4,iterations=1'],
['wlNVMedian',              1, 'iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_god_rays',          1, 'cfgFile=${intel_god_rays}.cfg,KernelType=ScalarOCL,CFG_intel_god_rays_LOCAL_WORK_SIZE=4,iterations=1'],
['intel_MonteCarlo',        1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1'],
['wlConvolution',           1, 'iterations=1,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMandelbrot',            1, 'CFG_MANDELBROT_WIDTH=2048,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=nbody_sim,numParticles=4096,groupSize=128,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_NBody',             1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,szBodies=4096,szGlobalWork=4096,groupSize=4,iterations=1'],
['intel_sobel',             1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1'],
['intel_convolution',       1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1,groupSize=4'],
['intel_mandelbrot',        1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1,localSize=128'],
['intel_rgb2yuv',           1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,iterations=1,groupSize=4'],
['wlATIAES',                1, 'cfgFile=wlATIAES.cfg,decrypt=true'],
['wlBitonicSort',           1, 'cfgFile=wlBitonicSort.cfg,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128'],
['wlATIRecGaussian',        1, 'cfgFile=wlATIRecGaussian.cfg,iterations=1'],
['wlTCC',                   1, 'cfgFile=measure.cfg,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64'],
['intel_RecGaussian',       1, 'cfgFile=intel_RecGaussian.cfg,KernelType=ScalarOCL,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,iterations=1'],
['wlATIPrefixSum',          1, 'cfgFile=wlATIPrefixSum.cfg,iterations=1'],
['wl2Landscape',            1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],
]

WolfPerformanceNew = [
# ########################################################################################################################################
# 
# Scalar OCL kernels
# Applicable for 15.1 + 15.2.1 + 15.2.2 + 15.3 + 15.6
# Check: 26
# ########################################################################################################################################

# Matrix Multiplication (Intel)
['intel_matrix_mult', 10, 'cfgFile=intel_matrix_mult.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Matrix Multiplication (ATI)
['wlMatrixMultiplication', 10, 'cfgFile=wlMatrixMultiplication.cfg,eDeviceTypes=CPU'],

# Prefix Sum (ATI), pls note that this workload doesn't support WGsize variation (always==length/2)
['wlATIPrefixSum', 10, 'cfgFile=wlATIPrefixSum.cfg,eDeviceTypes=CPU'],

# Framework overhead
['wlFrameworkOverhead', 10, 'cfgFile=measure.cfg,length=16777216,wg_size=1024,iterations=1000,eDeviceTypes=CPU'],

# Bilateral 2D filter (Intel)
['intel_bilateral2D', 10, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# DCT (Intel)
['wlDCT', 10, 'cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=100,groupSize=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Histogram (Intel)
['wlHistogram', 10, 'cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU'],

# Median filter (Intel)
['intel_median', 10, 'cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Median filter bitonic (Intel)
['intel_median_bitonic', 10, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Prefix Sum (Intel)
['wlPrefixSum', 10, 'cfgFile=measure.cfg,iterations=1000,szArray=67108864,szMaxInt=1000,szLocalWork=8,szGlobalWork=1024,bPrintSerialReport=true,eDeviceTypes=CPU'],

# Radial blur (Intel)
['intel_radial_blur', 10, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Median filter (nVidia)
['wlNVMedian', 10, 'cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,eDeviceTypes=CPU'],

# God rays (Intel)
['intel_god_rays', 10, 'cfgFile=intel_god_rays.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8'],

# Monte Carlo (Intel)
['intel_MonteCarlo', 10, 'cfgFile=measure.cfg,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU,WorkGroupSize=8'],

# Convolution (ATI)
['wlConvolution', 10, 'cfgFile=measure.cfg,iterations=100,CFG_CONVOLUTION_ARRAY_WIDTH=1920,CFG_CONVOLUTION_ARRAY_HEIGHT=1080,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,eDeviceTypes=CPU'],

# NBody (ATI)
['wlATINBody', 10, 'cfgFile=measure.cfg,kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=10,eDeviceTypes=CPU'],

# NBody (Intel)
['intel_NBody', 10, 'cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Sobel (Intel)
['intel_sobel', 10, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8'],

# Sobel8U (Intel)
['intel_sobel8u', 10, 'cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Convolution (Intel)
['intel_convolution', 10, 'cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU'],

# Mandelbrot (Intel)
['intel_mandelbrot', 10, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,localSize=128,eDeviceTypes=CPU'],

# Color conversion (Intel)
['intel_rgb2yuv', 10, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU'],

# AES (ATI)
['wlATIAES', 10, 'cfgFile=wlATIAES.cfg,decrypt=true,eDeviceTypes=CPU'],

# Bitonic Sort (ATI)
['wlBitonicSort', 10, 'cfgFile=wlBitonicSort.cfg,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,eDeviceTypes=CPU'],

# Recursive Gausian (ATI)
['wlATIRecGaussian', 10, 'cfgFile=wlATIRecGaussian.cfg,eDeviceTypes=CPU,iterations=100'],

# TCC (Intel)
# WOLF.exe out_tcc.csv wlTCC 10 cfgFile=measure.cfg,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,eDeviceTypes=CPU
['intel_tcc', 10, 'cfgFile=measure.cfg,KernelType=ScalarOCL,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,iterations=100,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,eDeviceTypes=CPU'],

# Recursive Gausian (Intel)
['intel_RecGaussian', 10, 'cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Colorbalance (Intel)
['intel_colorbalance', 10, 'cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Crossfade (Intel)
['intel_crossfade', 10, 'cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Gauss (Intel)
['intel_gauss', 10, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Radar (Intel)
['intel_radar', 10, 'cfgFile=measure.cfg,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8'],

# BAS (Intel)
['intel_bas', 10, 'cfgFile=measure.cfg,iterations=500,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# RadixSort(ATI)
['wlATIRadixSort', 10, 'cfgFile=measure.cfg,iterations=500,KernelType=ScalarOCL,eDeviceTypes=CPU,elementCount=65536'],


# ########################################################################################################################################
# 
# Vector OCL kernels
# Applicable for 15.4
# Check: 15
# ########################################################################################################################################

# Matrix Multiplication (Intel)
['intel_matrix_mult', 10, 'cfgFile=intel_matrix_mult.cfg,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Checkerboard
['checkerboard', 10, 'cfgFile=checkerboard.cfg,CFG_checkerboard_LOCAL_WORK_SIZE0=256,eDeviceTypes=CPU'],

# Bilateral 2D filter (Intel)
['intel_bilateral2D', 10, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# DCT (Intel)
['wlDCT', 10, 'cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=100,groupSize=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Histogram (Intel)
['wlHistogram', 10, 'cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU'],

# Median filter (Intel)
['intel_median', 10, 'cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Median filter bitonic (Intel)
['intel_median_bitonic', 10, 'cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Radial blur (Intel)
['intel_radial_blur', 10, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# God rays (Intel)
['intel_god_rays', 10, 'cfgFile=intel_god_rays.cfg,KernelType=VectorOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8'],

# Monte Carlo (Intel)
['intel_MonteCarlo', 10, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU,WorkGroupSize=8'],

# NBody (Intel)
['intel_NBody', 10, 'cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Sobel (Intel)
['intel_sobel', 10, 'cfgFile=measure.cfg,groupSize=8,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Sobel8U (Intel)
['intel_sobel8u', 10, 'cfgFile=measure.cfg,groupSize=8,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Convolution (Intel)
['intel_convolution', 10, 'cfgFile=measure.cfg,groupSize=8,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Mandelbrot (Intel)
['intel_mandelbrot', 10, 'cfgFile=measure.cfg,localSize=8,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Mandelbrot (ATI)
['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,eDeviceTypes=CPU'],

# Color conversion (Intel)
['intel_rgb2yuv', 10, 'cfgFile=measure.cfg,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Recursive Gausian (Intel)
['intel_RecGaussian', 10, 'cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Tone mapping OpenEXR (Intel)
['intel_tone_mapping', 10, 'cfgFile=intel_tone_mapping.cfg,KernelType=VectorOCL,iterations=100,CFG_intel_tone_mapping_LOCAL_WORK_SIZE=8,eDeviceTypes=CPU'],

# TCC + ACE (Intel)
['wlTCCACE', 1, 'cfgFile=wlTCCACE.cfg'],

# LCS (Intel)
['wlLcs', 10, 'cfgFile=wlLcs.cfg,KernelType=VectorOCL'],

# Colorbalance (Intel)
['intel_colorbalance', 10, 'cfgFile=measure.cfg,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Simple Box Blur (?)
['wlSimpleBoxBlur', 10, 'cfgFile=measure.cfg,CFG_SIMPLEBOXBLUR_LOCAL_THREAD_0=4,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Crossfade (Intel)
['intel_crossfade', 10, 'cfgFile=measure.cfg,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Gauss (Intel)
['intel_gauss', 10, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Radar (Intel)
['intel_radar', 10, 'cfgFile=measure.cfg,groupSize=8,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],

# BAS (Intel)
['intel_bas', 10, 'cfgFile=measure.cfg,iterations=500,KernelType=VectorOCL,eDeviceTypes=CPU'],

# MersenneTwister(ATI)
['wlMersenneTwister', 10, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],

# 2Landscape
['wl2Landscape', 10, 'cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Subdivision
['wlSubdivision', 10, 'cfgFile=subdivision.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU,MeshFileName=bigguy.obj'],

                      
]


WolfPerformanceNewConf = [
# ########################################################################################################################################
# 
# Scalar OCL kernels
# Applicable for 15.1 + 15.2.1 + 15.2.2 + 15.3 + 15.6
# Check: 26
# ########################################################################################################################################

# Matrix Multiplication (Intel)
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Matrix Multiplication (ATI)
['wlMatrixMultiplication', 1, 'cfgFile=wlMatrixMultiplication.cfg,eDeviceTypes=CPU'],

# Prefix Sum (ATI), pls note that this workload doesn't support WGsize variation (always==length/2)
['wlATIPrefixSum', 1, 'cfgFile=wlATIPrefixSum.cfg,eDeviceTypes=CPU'],

# Framework overhead
['wlFrameworkOverhead', 1, 'cfgFile=measure.cfg,length=16777216,wg_size=1024,iterations=1,eDeviceTypes=CPU'],

# Bilateral 2D filter (Intel)
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# DCT (Intel)
['wlDCT', 1, 'cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=1,groupSize=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Histogram (Intel)
['wlHistogram', 1, 'cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU'],

# Median filter (Intel)
['intel_median', 1, 'cfgFile=measure.cfg,iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Median filter bitonic (Intel)
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Prefix Sum (Intel)
['wlPrefixSum', 1, 'cfgFile=measure.cfg,iterations=1,szArray=67108864,szMaxInt=1000,szLocalWork=8,szGlobalWork=1024,bPrintSerialReport=true,eDeviceTypes=CPU'],

# Radial blur (Intel)
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Median filter (nVidia)
['wlNVMedian', 1, 'cfgFile=measure.cfg,iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,eDeviceTypes=CPU'],

# God rays (Intel)
['intel_god_rays', 1, 'cfgFile=intel_god_rays.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8'],

# Monte Carlo (Intel)
['intel_MonteCarlo', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU,WorkGroupSize=8'],

# Convolution (ATI)
['wlConvolution', 1, 'cfgFile=measure.cfg,iterations=1,CFG_CONVOLUTION_ARRAY_WIDTH=1920,CFG_CONVOLUTION_ARRAY_HEIGHT=1080,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,eDeviceTypes=CPU'],

# NBody (ATI)
['wlATINBody', 1, 'cfgFile=measure.cfg,kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=1,eDeviceTypes=CPU'],

# NBody (Intel)
['intel_NBody', 1, 'cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Sobel (Intel)
['intel_sobel', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8'],

# Sobel8U (Intel)
['intel_sobel8u', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Convolution (Intel)
['intel_convolution', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU'],

# Mandelbrot (Intel)
['intel_mandelbrot', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,localSize=128,eDeviceTypes=CPU'],

# Color conversion (Intel)
['intel_rgb2yuv', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU'],

# AES (ATI)
['wlATIAES', 1, 'cfgFile=wlATIAES.cfg,decrypt=true,eDeviceTypes=CPU'],

# Bitonic Sort (ATI)
['wlBitonicSort', 1, 'cfgFile=wlBitonicSort.cfg,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,eDeviceTypes=CPU'],

# Recursive Gausian (ATI)
['wlATIRecGaussian', 1, 'cfgFile=wlATIRecGaussian.cfg,eDeviceTypes=CPU,iterations=1'],

# TCC (Intel)
# WOLF.exe out_tcc.csv wlTCC 10 cfgFile=measure.cfg,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,eDeviceTypes=CPU
['intel_tcc', 1, 'cfgFile=measure.cfg,KernelType=ScalarOCL,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,iterations=1,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,eDeviceTypes=CPU'],

# Recursive Gausian (Intel)
['intel_RecGaussian', 1, 'cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Colorbalance (Intel)
['intel_colorbalance', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Crossfade (Intel)
['intel_crossfade', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Gauss (Intel)
['intel_gauss', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# Radar (Intel)
['intel_radar', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8'],

# BAS (Intel)
['intel_bas', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU'],

# RadixSort(ATI)
['wlATIRadixSort', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU,elementCount=65536'],


# ########################################################################################################################################
# 
# Vector OCL kernels
# Applicable for 15.4
# Check: 15
# ########################################################################################################################################

# Matrix Multiplication (Intel)
['intel_matrix_mult', 1, 'cfgFile=intel_matrix_mult.cfg,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Checkerboard
['checkerboard', 1, 'cfgFile=checkerboard.cfg,CFG_checkerboard_LOCAL_WORK_SIZE0=256,eDeviceTypes=CPU'],

# Bilateral 2D filter (Intel)
['intel_bilateral2D', 1, 'cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# DCT (Intel)
['wlDCT', 1, 'cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=1,groupSize=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Histogram (Intel)
['wlHistogram', 1, 'cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU'],

# Median filter (Intel)
['intel_median', 1, 'cfgFile=measure.cfg,iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Median filter bitonic (Intel)
['intel_median_bitonic', 1, 'cfgFile=measure.cfg,iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Radial blur (Intel)
['intel_radial_blur', 1, 'cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# God rays (Intel)
['intel_god_rays', 1, 'cfgFile=intel_god_rays.cfg,KernelType=VectorOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8'],

# Monte Carlo (Intel)
['intel_MonteCarlo', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU,WorkGroupSize=8'],

# NBody (Intel)
['intel_NBody', 1, 'cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Sobel (Intel)
['intel_sobel', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Sobel8U (Intel)
['intel_sobel8u', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Convolution (Intel)
['intel_convolution', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Mandelbrot (Intel)
['intel_mandelbrot', 1, 'cfgFile=measure.cfg,localSize=8,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Mandelbrot (ATI)
['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,eDeviceTypes=CPU'],

# Color conversion (Intel)
['intel_rgb2yuv', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Recursive Gausian (Intel)
['intel_RecGaussian', 1, 'cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Tone mapping OpenEXR (Intel)
['intel_tone_mapping', 1, 'cfgFile=intel_tone_mapping.cfg,KernelType=VectorOCL,iterations=1,CFG_intel_tone_mapping_LOCAL_WORK_SIZE=8,eDeviceTypes=CPU'],

# TCC + ACE (Intel)
['wlTCCACE', 1, 'cfgFile=wlTCCACE.cfg'],

# LCS (Intel)
['wlLcs', 1, 'cfgFile=wlLcs.cfg,KernelType=VectorOCL'],

# Colorbalance (Intel)
['intel_colorbalance', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Simple Box Blur (?)
['wlSimpleBoxBlur', 1, 'cfgFile=measure.cfg,CFG_SIMPLEBOXBLUR_LOCAL_THREAD_0=4,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Crossfade (Intel)
['intel_crossfade', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Gauss (Intel)
['intel_gauss', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Radar (Intel)
['intel_radar', 1, 'cfgFile=measure.cfg,groupSize=8,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# BAS (Intel)
['intel_bas', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# MersenneTwister(ATI)
['wlMersenneTwister', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# 2Landscape
['wl2Landscape', 1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],

# Subdivision
['wlSubdivision', 1, 'cfgFile=subdivision.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU,MeshFileName=bigguy.obj'],
]


WolfPostCommit = [
['wlFrameworkOverhead',     1, 'length=16,wg_size=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead',     1, 'length=1024,wg_size=1024,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC',                   1, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_vector8_third_optimization,TCC_CFG_GLOBAL_WORKGROUP_SIZE=8,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC',                   1, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC',                   1, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_vector8_third_optimization,TCC_CFG_GLOBAL_WORKGROUP_SIZE=8,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC',                   1, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=1,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBitonicSort',           1, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBitonicSort',           1, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlIntelBitonicSort',      1, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlIntelBitonicSort',      1, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMandelbrot',            1, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_mandelbrot',        1, 'iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlConvolution',           1, 'iterations=1,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIDCT',                1, 'CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlDCT',                   1, 'CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody',              1, 'kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_NBody',             1, 'szBodies=32768,szGlobalWork=32768,groupSize=4,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIPrefixSum',          1, 'length=2048,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlPrefixSum',             1, 'iterations=1,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlPrefixSum',             1, 'iterations=1,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=4,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIHistogram',          1, 'iterations=1,width=1024,height=1024,wg_size=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram',             1, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=1,szGlobalWorkStep2=1,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIAES',                1, 'decrypt=false,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIAES',                1, 'decrypt=true,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlNVMedian',              1, 'iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_median',            1, 'iterations=1,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFFT',                1, 'iterations=1,length=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATISobelFilter',        1, 'iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_sobel',             1, 'iterations=1'],
['wlATIRadixSort',          1, 'iterations=1,elementCount=8192,groupSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction',          1, 'length=256,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction',          1, 'length=1024,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction',          1, 'length=16384,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction',          1, 'length=1048576,GROUP_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIRecGaussian',        1, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_RecGaussian',       1, 'eDeviceTypes=CPU,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=16,GAUSSIAN_CFG_BLOCK_DIM=8'],
['wlATIFastWalsh',          1, 'length=256,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh',          1, 'length=1024,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh',          1, 'length=16384,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh',          1, 'length=524288,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh',          1, 'length=1048576,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister',       1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister',       1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister',       1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMonteCarlo',         1, 'blockSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_MonteCarlo',        1, 'iterations=1,WorkGroupSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication',  1, 'CFG_MATRIX_MUL_WIDTH=128,CFG_MATRIX_MUL_HEIGHT=128,CFG_MATRIX_MUL_DEPTH=128,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication',  1, 'CFG_MATRIX_MUL_WIDTH=512,CFG_MATRIX_MUL_HEIGHT=512,CFG_MATRIX_MUL_DEPTH=512,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication',  1, 'CFG_MATRIX_MUL_WIDTH=1024,CFG_MATRIX_MUL_HEIGHT=1024,CFG_MATRIX_MUL_DEPTH=1024,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_matrix_mult',       1, 'cfgFile=${intel_matrix_mult}.cfg,KernelType=ScalarOCL'],
['intel_matrix_mult',       1, 'cfgFile=${intel_matrix_mult}.cfg,KernelType=VectorOCL'],
['wlATIBinOption',          1, 'CFG_BIN_OPTION_NUM_SAMPLES=64,CFG_BIN_OPTION_NUM_STEPS=64,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans',        1, 'width=128,height=128,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans',        1, 'width=512,height=512,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans',        1, 'width=1024,height=1024,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans',        1, 'width=4096,height=4096,blockSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIQuasiSeq',           1, 'iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIDwtHaar1D',          1, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,signalLength=32768,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFloydWarshall',      1, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,numNodes=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch',          1, 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch',          1, 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch',          1, 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIEigenValue',         1, 'globalSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays',         1, 'iterations=1,length=128,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays',         1, 'iterations=1,length=1024,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays',         1, 'iterations=1,length=16384,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=64,globalSize2=64,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=128,globalSize2=128,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=512,globalSize2=512,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=1024,globalSize2=1024,localSize=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=64,globalSize2=64,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=128,globalSize2=128,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=512,globalSize2=512,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS',                1, 'globalSize1=1024,globalSize2=1024,localSize=2,iterations=1,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_god_rays',          1, '${intel_god_rays}.cfg'],
['AIF_RadialBlur',          1, '${AIF_RadialBlur}.cfg'],
['intel_radial_blur',       1, '${intel_radial_blur}.cfg'],
['invertRGB',               1, '${invertRGB}.cfg'],
['wlSimpleBoxBlur',         1, 'wlSimpleBoxBlur.cfg'],
['crossfade',               1, '${crossfade}.cfg'],
['twirl',                   1, '${twirl}.cfg'],
['checkerboard',            1, '${checkerboard}.cfg'],
['wlSobel',                 1, 'wlSobel.cfg'],
['droste',                  1, '${droste}.cfg'],
['bilateral2D',             1, '${bilateral2D}.cfg'],
['intel_bilateral2D',       1, '${intel_bilateral2D}.cfg'],
['wlAdobe_PB_Pixelate',     1, 'wlAdobe_PB_Pixelate.cfg'],
['wl2Landscape',            1, 'cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU'],
]                                  

WolfBenchmark = [
#########################################################################################################################################
# Framework Overhead 
#########################################################################################################################################
# (Kernel Execution \ 1000)
# -------------------------------------------------
# 	Workgroup Size = 1
# -------------------------------------------------

['wlFrameworkOverhead', 10, 'length=16,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=32,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=64,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=128,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=256,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=512,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=1024,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=2048,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=4096,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=8192,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=16384,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=32768,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=65536,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=131072,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=262144,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=524288,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=1048576,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=2097152,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=4194304,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=8388608,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=16777216,wg_size=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Workgroup Size = 1024
# -------------------------------------------------

['wlFrameworkOverhead', 10, 'length=1024,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=2048,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=4096,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=8192,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=16384,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=32768,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=65536,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=131072,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 10, 'length=262144,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=524288,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=1048576,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=2097152,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=4194304,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=8388608,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlFrameworkOverhead', 1, 'length=16777216,wg_size=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# TCC (Intel)
# ########################################################################################################################################
# execute kernel (Total) \ 1000

# -------------------------------------------------
#     SD (720x480) - Explicit
# -------------------------------------------------

['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_vector8_third_optimization,TCC_CFG_GLOBAL_WORKGROUP_SIZE=8,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     SD (720x480) - Implicit
# -------------------------------------------------

['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=720,TCC_CFG_FRAME_HEIGHT=480,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     HD (1920x1080) - Explicit
# -------------------------------------------------

['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_vector8_third_optimization,TCC_CFG_GLOBAL_WORKGROUP_SIZE=8,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     HD (1920x1080) - Implicit
# -------------------------------------------------

['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlTCC', 10, 'TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,TCC_CFG_USE_RANDOM_DATA=True,TCC_CFG_MAX_NUMBER_OF_FRAMES=10,iterations=100,TCC_CFG_KERNEL_NAME=tcc_scalar_unroll2,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Bitonic Sort (AMD)
# ########################################################################################################################################
# Complete Sorting Execution

# -------------------------------------------------
#     Buffer size = 16K 
# -------------------------------------------------

# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     Buffer size = 1M
# -------------------------------------------------

# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Bitonic Sort (Intel)
# ########################################################################################################################################
# Complete Sorting Execution

# -------------------------------------------------
#     Buffer size = 16K 
# -------------------------------------------------

# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=16384,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     Buffer size = 1M
# -------------------------------------------------

# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlIntelBitonicSort', 10, 'bMeasureNQNDRange=False,BITONIC_CFG_SORTING_DIR=1,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Mandelbrot (AMD)
# ########################################################################################################################################
# Kernel Execution

# -------------------------------------------------
# 	Matrtix size = 64x64 (4K) 
# -------------------------------------------------
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=64,CFG_MANDELBROT_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 256x256 (64K)
# -------------------------------------------------
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=256,CFG_MANDELBROT_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=256,CFG_MANDELBROT_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=256,CFG_MANDELBROT_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=256,CFG_MANDELBROT_LOCAL_WORK_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=256,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 512x512 (256K) 
# -------------------------------------------------
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 1024x1024 (1M)
# -------------------------------------------------
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=1024,CFG_MANDELBROT_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=1024,CFG_MANDELBROT_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=1024,CFG_MANDELBROT_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=1024,CFG_MANDELBROT_LOCAL_WORK_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 10, 'CFG_MANDELBROT_WIDTH=1024,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 4096x4096 (16M)
# -------------------------------------------------
# ['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=4096,CFG_MANDELBROT_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=4096,CFG_MANDELBROT_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=4096,CFG_MANDELBROT_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=4096,CFG_MANDELBROT_LOCAL_WORK_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlMandelbrot', 1, 'CFG_MANDELBROT_WIDTH=4096,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Mandelbrot (Intel)
# ########################################################################################################################################
# Kernel Execution
# -------------------------------------------------
# 	Matrtix size = 512x512 (256K) \100
# -------------------------------------------------
['intel_mandelbrot', 10, 'iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],



# ########################################################################################################################################
# Convolution
# ########################################################################################################################################
# Complete Execution (64X64) \ 1000

# -------------------------------------------------
# 	Input size = 64 X 64 (4K)
# -------------------------------------------------

# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=64,CFG_CONVOLUTION_ARRAY_HEIGHT=64,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Complete Execution (128X128) \ 1000

# -------------------------------------------------
# 	Input size = 128 X 128 (16K)
# -------------------------------------------------

# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=128,CFG_CONVOLUTION_ARRAY_HEIGHT=128,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Complete Execution (256X256) \ 1000

# -------------------------------------------------
# 	Input size = 256 X 256 (64K)
# -------------------------------------------------

# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=256,CFG_CONVOLUTION_ARRAY_HEIGHT=256,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# -------------------------------------------------
# 	Input size = 1024 X 1024 (1M)
# -------------------------------------------------
# ['wlConvolution', 10, 'iterations=1000,CFG_CONVOLUTION_ARRAY_WIDTH=1024,CFG_CONVOLUTION_ARRAY_HEIGHT=1024,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],



# ########################################################################################################################################
# DCT (AMD)
# ########################################################################################################################################
# Kernel Execution / 1000

['wlATIDCT', 10, 'CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# DCT (Intel)
# ########################################################################################################################################
# Kernel Execution (OpenCL vector) / 100

# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU,CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU,CFG_DCT_ARRAY_WIDTH=128,CFG_DCT_ARRAY_HEIGHT=128,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU,CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU,CFG_DCT_ARRAY_WIDTH=1024,CFG_DCT_ARRAY_HEIGHT=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU_VECTOR,CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU_VECTOR,CFG_DCT_ARRAY_WIDTH=128,CFG_DCT_ARRAY_HEIGHT=128,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU_VECTOR,CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_CPU_VECTOR,CFG_DCT_ARRAY_WIDTH=1024,CFG_DCT_ARRAY_HEIGHT=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT,CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT,CFG_DCT_ARRAY_WIDTH=128,CFG_DCT_ARRAY_HEIGHT=128,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT,CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT,CFG_DCT_ARRAY_WIDTH=1024,CFG_DCT_ARRAY_HEIGHT=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_VECTOR,CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_VECTO,CFG_DCT_ARRAY_WIDTH=128,CFG_DCT_ARRAY_HEIGHT=128,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_VECTO,CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlDCT', 10, 'CFG_DCT_KERNEL_NAME=DCT_VECTO,CFG_DCT_ARRAY_WIDTH=1024,CFG_DCT_ARRAY_HEIGHT=1024,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

['wlDCT', 10, 'CFG_DCT_ARRAY_WIDTH=64,CFG_DCT_ARRAY_HEIGHT=64,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlDCT', 10, 'CFG_DCT_ARRAY_WIDTH=128,CFG_DCT_ARRAY_HEIGHT=128,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlDCT', 10, 'CFG_DCT_ARRAY_WIDTH=512,CFG_DCT_ARRAY_HEIGHT=512,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlDCT', 10, 'CFG_DCT_ARRAY_WIDTH=1024,CFG_DCT_ARRAY_HEIGHT=1024,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],



# ########################################################################################################################################
# NBody (Apple)
# ########################################################################################################################################
# Execution of IntegrateSystemNonVectorized \ 10000

# -------------------------------------------------
# 	128 Particles
# -------------------------------------------------

['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=1,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=2,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=4,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=8,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=128,globalThreads=16,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Execution of IntegrateSystemNonVectorized \ 1000

# -------------------------------------------------
# 	1024 Particles
# -------------------------------------------------

['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=2,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=4,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=8,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=1024,globalThreads=16,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Execution of IntegrateSystemNonVectorized \ 100

# -------------------------------------------------
# 	4096 Particles
# -------------------------------------------------

['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=1,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemNonVectorized,numParticles=4096,globalThreads=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# NBody (Apple - Vectorized)
# ########################################################################################################################################

# Execution of IntegrateSystemVectorized \ 10000

# -------------------------------------------------
# 	128 Particles
# -------------------------------------------------

['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=1,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=2,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=4,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=8,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=128,globalThreads=16,iterations=10000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Execution of IntegrateSystemVectorized \ 1000

# -------------------------------------------------
# 	1024 Particles
# -------------------------------------------------

['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=1,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=2,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=4,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=8,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=1024,globalThreads=16,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Execution of IntegrateSystemVectorized \ 100

# -------------------------------------------------
# 	4096 Particles
# -------------------------------------------------

['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=1,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=IntegrateSystemVectorized,numParticles=4096,globalThreads=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# NBody (AMD)
# ########################################################################################################################################

# Execution of nbody_sim \ 1000

# -------------------------------------------------
# 	128 Particles
# -------------------------------------------------

# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=128,groupSize=2,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=128,groupSize=4,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=128,groupSize=8,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=128,groupSize=16,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=128,groupSize=32,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	1024 Particles
# -------------------------------------------------

# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=2,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=4,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=8,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=16,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=32,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=64,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=128,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=1024,groupSize=256,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Execution of nbody_sim \ 100

# -------------------------------------------------
# 	4096 Particles
# -------------------------------------------------

# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=32,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=64,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=128,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=256,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=512,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=4096,groupSize=1024,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Execution of nbody_sim \ 10

# -------------------------------------------------
# 	32768 Particles
# -------------------------------------------------

# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=2,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=4,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=8,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=16,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=32,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=64,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=256,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlATINBody', 10, 'kernelName=nbody_sim,numParticles=32768,groupSize=512,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# NBody (Intel)
# ########################################################################################################################################
# Execution of OCL kernel nBodyVecKernel \ 100

# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=2,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_NBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=4,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=16,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=32,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=64,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=128,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=256,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNBody', 10, 'szBodies=32768,szGlobalWork=32768,groupSize=512,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Prefix Sum (AMD)
# ########################################################################################################################################
# Kernel Execution \ 1000
['wlATIPrefixSum', 10, 'length=2048,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Prefix Sum (Intel)
# ########################################################################################################################################
# Kernel execution (OpenCL scalar) prefixSumStep1 and prefixSumStep2 \ 1000

# -------------------------------------------------
# 	Input size = 4K
# -------------------------------------------------

# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=4096,szMaxInt=1000,szLocalWork=1,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# -------------------------------------------------
# 	Input size = 16K
# -------------------------------------------------

# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=16384,szMaxInt=1000,szLocalWork=1,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1M
# -------------------------------------------------

# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=1,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlPrefixSum', 10, 'iterations=1000,bMeasureCreateBuffer=False,szArray=1048576,szMaxInt=1000,szLocalWork=4,szGlobalWork=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Histogram (AMD)
# ########################################################################################################################################
# Kernel Execution \ 1000

['wlATIHistogram', 10, 'iterations=1000,width=1024,height=1024,wg_size=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Histogram (Intel)
# ########################################################################################################################################
# Kernel execution (OpenCL vector) histogramStep1 and histogramStep2int2 \ 1000

# -------------------------------------------------
# 	Input size = 64 x 64 (4K), Bin size = 256
# -------------------------------------------------

# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=1,szGlobalWorkStep2=1,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=2,szGlobalWorkStep2=2,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=4,szGlobalWorkStep2=4,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=8,szGlobalWorkStep2=8,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=16,szGlobalWorkStep2=16,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=32,szGlobalWorkStep2=32,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=64,szGlobalWorkStep2=64,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=256,szMatrix=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 128 x 128 (16K), Bin size = 1024
# -------------------------------------------------

['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=1,szGlobalWorkStep2=1,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=2,szGlobalWorkStep2=2,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=4,szGlobalWorkStep2=4,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=8,szGlobalWorkStep2=8,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=16,szGlobalWorkStep2=16,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=32,szGlobalWorkStep2=32,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=64,szGlobalWorkStep2=64,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1024 x 1024 (1M), Bin size = 1024
# -------------------------------------------------

# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=1,szGlobalWorkStep2=1,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=2,szGlobalWorkStep2=2,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=4,szGlobalWorkStep2=4,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=8,szGlobalWorkStep2=8,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=16,szGlobalWorkStep2=16,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=32,szGlobalWorkStep2=32,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlHistogram', 10, 'bMeasureCreateBuffer=False,szGlobalWorkStep1=64,szGlobalWorkStep2=64,szLocalWorkStep1=1,szLocalWorkStep2=1,iterations=1000,szBin=1024,szMatrix=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# AES Encrypt / Decrypt
# ########################################################################################################################################
# Kernel Execution / 100

['wlATIAES', 10, 'decrypt=false,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIAES', 10, 'decrypt=true,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Median Filter
# ########################################################################################################################################
# Kernel Execution (OpenCL scalar) / 1000

# ['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=32,CFG_MEDIAN_ARRAY_HEIGHT=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['wlNVMedian', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Median Filter (Intel)
# ########################################################################################################################################
# Kernel Execution (OpenCL vector) / 1000

# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=32,CFG_MEDIAN_ARRAY_HEIGHT=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,CFG_MEDIAN_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,CFG_MEDIAN_LOCAL_WORK_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,CFG_MEDIAN_LOCAL_WORK_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=64,CFG_MEDIAN_ARRAY_HEIGHT=64,CFG_MEDIAN_LOCAL_WORK_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,CFG_MEDIAN_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,CFG_MEDIAN_LOCAL_WORK_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,CFG_MEDIAN_LOCAL_WORK_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=128,CFG_MEDIAN_ARRAY_HEIGHT=128,CFG_MEDIAN_LOCAL_WORK_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_median', 10, 'iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# FFT
# ########################################################################################################################################
# Kernel Execution / 1000

['wlATIFFT', 10, 'iterations=1000,length=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFFT', 10, 'iterations=1000,length=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFFT', 10, 'iterations=1000,length=16384,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Sobel Filter
# ########################################################################################################################################
# Kernel Execution / 1000

['wlATISobelFilter', 10, 'iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Sobel Filter (Intel)
# ########################################################################################################################################
# Execute_OCL_Vector / 10

['intel_sobel', 10, 'iterations=10'],

# ########################################################################################################################################
# Radix Sort
# ########################################################################################################################################
# Kernel Execution / 1000

['wlATIRadixSort', 10, 'iterations=1000,elementCount=8192,groupSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Reduction (AMD)
# ########################################################################################################################################
# Kernel Execution / 100

# -------------------------------------------------
# 	Input size = 256 (64)
# -------------------------------------------------

['wlATIReduction', 10, 'length=256,GROUP_SIZE=1,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=256,GROUP_SIZE=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=256,GROUP_SIZE=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=256,GROUP_SIZE=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=256,GROUP_SIZE=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1K (256)
# -------------------------------------------------

['wlATIReduction', 10, 'length=1024,GROUP_SIZE=1,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1024,GROUP_SIZE=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1024,GROUP_SIZE=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1024,GROUP_SIZE=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1024,GROUP_SIZE=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 16K (4K)
# -------------------------------------------------

['wlATIReduction', 10, 'length=16384,GROUP_SIZE=1,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=16384,GROUP_SIZE=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=16384,GROUP_SIZE=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=16384,GROUP_SIZE=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=16384,GROUP_SIZE=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1M (1M)
# -------------------------------------------------

['wlATIReduction', 10, 'length=1048576,GROUP_SIZE=1,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1048576,GROUP_SIZE=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1048576,GROUP_SIZE=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1048576,GROUP_SIZE=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIReduction', 10, 'length=1048576,GROUP_SIZE=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Recursive Gausian (AMD)
# ########################################################################################################################################
# Kernel Execution

['wlATIRecGaussian', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Recursive Gausian (Intel)
# ########################################################################################################################################
# Kernel Execution (OpenCL vector)

['intel_RecGaussian', 10, 'eDeviceTypes=CPU,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=16,GAUSSIAN_CFG_BLOCK_DIM=8'],

# ########################################################################################################################################
# Fast-Walsh Transform (AMD)
# ########################################################################################################################################
# Kernel Execution

# -------------------------------------------------
# 	Input size = 256
# -------------------------------------------------

['wlATIFastWalsh', 100, 'length=256,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=256,localSize=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1K
# -------------------------------------------------

['wlATIFastWalsh', 100, 'length=1024,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=1024,localSize=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 16K
# -------------------------------------------------

['wlATIFastWalsh', 100, 'length=16384,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 100, 'length=16384,localSize=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 512K
# -------------------------------------------------

['wlATIFastWalsh', 10, 'length=524288,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=524288,localSize=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1M
# -------------------------------------------------

['wlATIFastWalsh', 10, 'length=1048576,localSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=512,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFastWalsh', 10, 'length=1048576,localSize=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Mersenne-Twister
# ########################################################################################################################################
# Kernel Execution / 50

# -------------------------------------------------
#     4K (64 x 64)
# -------------------------------------------------

['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=2,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=4,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=8,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=16,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=16384,CFG_MERSENNE_TWISTER_BLOCK_SIZE=32,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     16K (128 x 128)
# -------------------------------------------------

['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=2,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=4,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=8,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=16,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 10, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=65536,CFG_MERSENNE_TWISTER_BLOCK_SIZE=32,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     1M (1024 x 1024)
# -------------------------------------------------

['wlMersenneTwister', 1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=1,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=2,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=4,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=8,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=16,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMersenneTwister', 1, 'CFG_MERSENNE_TWISTER_ARRAY_SIZE=4194304,CFG_MERSENNE_TWISTER_BLOCK_SIZE=32,iterations=50,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Monte Carlo Asian
# ########################################################################################################################################
# Kernel execution

['wlATIMonteCarlo', 1, 'blockSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMonteCarlo', 1, 'blockSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Monte Carlo vector (Intel)
# ########################################################################################################################################
# intel_MonteCarlo_vector all iterations execution / 10

# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
# ['intel_MonteCarlo', 10, 'iterations=10,WorkGroupSize=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Matrix Multiplication
# ########################################################################################################################################

# Kernel execution / 10

# -------------------------------------------------
#     Matrtix size = 128x128 (16k)
# -------------------------------------------------

['wlMatrixMultiplication', 10, 'CFG_MATRIX_MUL_WIDTH=128,CFG_MATRIX_MUL_HEIGHT=128,CFG_MATRIX_MUL_DEPTH=128,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 10, 'CFG_MATRIX_MUL_WIDTH=128,CFG_MATRIX_MUL_HEIGHT=128,CFG_MATRIX_MUL_DEPTH=128,CFG_MATRIX_MUL_BLOCK_SIZE=4,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 10, 'CFG_MATRIX_MUL_WIDTH=128,CFG_MATRIX_MUL_HEIGHT=128,CFG_MATRIX_MUL_DEPTH=128,CFG_MATRIX_MUL_BLOCK_SIZE=8,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 10, 'CFG_MATRIX_MUL_WIDTH=128,CFG_MATRIX_MUL_HEIGHT=128,CFG_MATRIX_MUL_DEPTH=128,CFG_MATRIX_MUL_BLOCK_SIZE=16,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 512x512 (256k) 
# -------------------------------------------------

['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=512,CFG_MATRIX_MUL_HEIGHT=512,CFG_MATRIX_MUL_DEPTH=512,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=512,CFG_MATRIX_MUL_HEIGHT=512,CFG_MATRIX_MUL_DEPTH=512,CFG_MATRIX_MUL_BLOCK_SIZE=4,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=512,CFG_MATRIX_MUL_HEIGHT=512,CFG_MATRIX_MUL_DEPTH=512,CFG_MATRIX_MUL_BLOCK_SIZE=8,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=512,CFG_MATRIX_MUL_HEIGHT=512,CFG_MATRIX_MUL_DEPTH=512,CFG_MATRIX_MUL_BLOCK_SIZE=16,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Kernel execution / 1

# -------------------------------------------------
# 	Matrtix size = 1024x1024 (1M) 
# -------------------------------------------------

['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=1024,CFG_MATRIX_MUL_HEIGHT=1024,CFG_MATRIX_MUL_DEPTH=1024,CFG_MATRIX_MUL_BLOCK_SIZE=2,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=1024,CFG_MATRIX_MUL_HEIGHT=1024,CFG_MATRIX_MUL_DEPTH=1024,CFG_MATRIX_MUL_BLOCK_SIZE=4,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=1024,CFG_MATRIX_MUL_HEIGHT=1024,CFG_MATRIX_MUL_DEPTH=1024,CFG_MATRIX_MUL_BLOCK_SIZE=8,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlMatrixMultiplication', 1, 'CFG_MATRIX_MUL_WIDTH=1024,CFG_MATRIX_MUL_HEIGHT=1024,CFG_MATRIX_MUL_DEPTH=1024,CFG_MATRIX_MUL_BLOCK_SIZE=16,iterations=1,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Intel_Matrix_Multiplication
# ########################################################################################################################################
# Kernel execution
['intel_matrix_mult', 10, 'cfgFile=${intel_matrix_mult}.cfg,KernelType=ScalarOCL'],
# Kernel execution
['intel_matrix_mult', 10, 'cfgFile=${intel_matrix_mult}.cfg,KernelType=VectorOCL'],

# ########################################################################################################################################
# Binomial option
# ########################################################################################################################################
# Kernel execution / 100

['wlATIBinOption', 10, 'CFG_BIN_OPTION_NUM_SAMPLES=64,CFG_BIN_OPTION_NUM_STEPS=64,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBinOption', 10, 'CFG_BIN_OPTION_NUM_SAMPLES=128,CFG_BIN_OPTION_NUM_STEPS=128,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBinOption', 10, 'CFG_BIN_OPTION_NUM_SAMPLES=128,CFG_BIN_OPTION_NUM_STEPS=64,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Matrix Transpose
# ########################################################################################################################################
# Kernel execution / 100

# -------------------------------------------------
# 	Matrtix size = 128x128 (16K)
# -------------------------------------------------
['wlATIMatrixTrans', 10, 'width=128,height=128,blockSize=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=128,height=128,blockSize=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=128,height=128,blockSize=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 512x512 (256K)
# -------------------------------------------------
['wlATIMatrixTrans', 10, 'width=512,height=512,blockSize=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=512,height=512,blockSize=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=512,height=512,blockSize=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Matrtix size = 1024x1024 (1M)
# -------------------------------------------------
['wlATIMatrixTrans', 10, 'width=1024,height=1024,blockSize=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=1024,height=1024,blockSize=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=1024,height=1024,blockSize=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Kernel execution / 10

# -------------------------------------------------
# 	Matrtix size = 4096x4096 (16M)
# -------------------------------------------------
['wlATIMatrixTrans', 10, 'width=4096,height=4096,blockSize=2,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=4096,height=4096,blockSize=4,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIMatrixTrans', 10, 'width=4096,height=4096,blockSize=8,iterations=10,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Quasi Random Sequence
# ########################################################################################################################################
# Kernel execution - 1000 times / 1000

['wlATIQuasiSeq', 10, 'iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# 1D Haar Wavelet Transform (AMD)
# ########################################################################################################################################
# Kernel Execution

['wlATIDwtHaar1D', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,signalLength=32768,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIDwtHaar1D', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,signalLength=131072,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIDwtHaar1D', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,signalLength=524288,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIDwtHaar1D', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,signalLength=1048576,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# Floyd Warshall
# ########################################################################################################################################
# Complete Execution

['wlATIFloydWarshall', 100, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,numNodes=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFloydWarshall', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,numNodes=256,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIFloydWarshall', 10, 'bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,numNodes=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Binary Search
# ########################################################################################################################################
# Kernel execution / 100

# -------------------------------------------------
#     Buffer size = 256K
# -------------------------------------------------

['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=262144,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     Buffer size = 1M
# -------------------------------------------------

['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=1048576,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
#     Buffer size = 16M
# -------------------------------------------------

['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=2,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlBinarySearch', 10, 'CFG_BINARY_SEARCH_ARRAY_SIZE=16777216,CFG_BINARY_SEARCH_SUBDIVISION_SIZE=32,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Eigen Value
# ########################################################################################################################################
# Kernel execution

['wlATIEigenValue', 1, 'globalSize=64,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIEigenValue', 1, 'globalSize=1024,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIEigenValue', 1, 'globalSize=4096,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# Scan Large Arrays
# ########################################################################################################################################
# Kernel execution / 1000

# -------------------------------------------------
# 	Input size = 128
# -------------------------------------------------

['wlATIScanArrays', 10, 'iterations=1000,length=128,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays', 10, 'iterations=1000,length=128,blockSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays', 10, 'iterations=1000,length=128,blockSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 1024 (1K)
# -------------------------------------------------

['wlATIScanArrays', 10, 'iterations=1000,length=1024,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays', 10, 'iterations=1000,length=1024,blockSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays', 10, 'iterations=1000,length=1024,blockSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	Input size = 16384 (16K)
# -------------------------------------------------

['wlATIScanArrays', 10, 'iterations=1000,length=16384,blockSize=4,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays', 10, 'iterations=1000,length=16384,blockSize=8,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIScanArrays', 10, 'iterations=1000,length=16384,blockSize=16,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


#########################################################################################################################################
########################################################################################################################################
# Kernel execution / 1000

# -------------------------------------------------
# 	4k (64 x 64)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=2,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=4,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=8,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=16,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=32,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	16k (128 x 128)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=2,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=4,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=8,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=16,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=32,iterations=1000,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Kernel execution / 100

# -------------------------------------------------
# 	256k (512 x 512)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=32,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# -------------------------------------------------
# 	1M (1024 x 1024)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=2,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=4,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=8,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=16,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=32,iterations=100,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# ########################################################################################################################################
# ########################################################################################################################################
# Kernel execution / 1000

# -------------------------------------------------
# 	4k (64 x 64)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=2,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=4,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=8,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=16,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=64,globalSize2=64,localSize=32,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# -------------------------------------------------
# 	16k (128 x 128)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=2,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=4,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=8,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=16,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=128,globalSize2=128,localSize=32,iterations=1000,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# Kernel execution / 100

# -------------------------------------------------
# 	256k (512 x 512)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=2,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=4,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=8,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=16,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=512,globalSize2=512,localSize=32,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],


# -------------------------------------------------
# 	1M (1024 x 1024)
# -------------------------------------------------

['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=2,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=4,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=8,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=16,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],
['wlATIBAS', 10, 'globalSize1=1024,globalSize2=1024,localSize=32,iterations=100,vectorized=false,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false'],

# ########################################################################################################################################
# God Rays
# ########################################################################################################################################
# Kernel execution / 100

['intel_god_rays', 10, '${intel_god_rays}.cfg'],


# ########################################################################################################################################
# Adobe Pixel Bender
# ########################################################################################################################################

# Kernel Execution / 100
['AIF_RadialBlur', 10, '${AIF_RadialBlur}.cfg'],
['intel_radial_blur', 10, '${intel_radial_blur}.cfg'],
# Native Execution Time of: invertRGB / 5000
['invertRGB', 10, '${invertRGB}.cfg'],
# Kernel Execution / 1000
['colorbalance2', 10, '${colorbalance2}.cfg'],
# Complete Execution / 1000
['wlSimpleBoxBlur', 10, 'wlSimpleBoxBlur.cfg'],
# Native Execution Time of: crossfade / 5000
['crossfade', 10, '${crossfade}.cfg'],
# Kernel Execution / 1000
['twirl', 10, '${twirl}.cfg'],
# Kernel Execution / 1000
['checkerboard', 10, '${checkerboard}.cfg'],
# iterations == 8
['gauss', 10, '${gauss}.cfg'],
# Native Execution Time of: wlSobel / 1000
['wlSobel', 10, 'wlSobel.cfg'],
# Kernel Execution / 500
['Mandelbrot', 10, '${Mandelbrot}.cfg'],
# Kernel Execution / 1000
# ['droste', 10, '${droste}.cfg'],
# iterations == 10
['bilateral2D', 10, '${bilateral2D}.cfg'],
['intel_bilateral2D', 10, '${intel_bilateral2D}.cfg'],
# Native Execution Time of: wlAdobe_PB_Pixelate / 5000
['wlAdobe_PB_Pixelate', 10, 'wlAdobe_PB_Pixelate.cfg'],
]

class VolcanoWolf(VolcanoTestSuite):
    def __init__(self, name, config, tests, capture_data):
        VolcanoTestSuite.__init__(self, name)

        for test in tests:
            self.addTask( WOLFTest( self.getTestName(test[0]), test[0], test[1], test[2], config, capture_data));

    def getTestName(self, test):
        tname = test
        count = 1
        while tname in self.getTasksNames():
            tname = '_'.join([test, str(count)])
            count += 1
        return tname

class VolcanoWolfPostCommit(VolcanoWolf):
    def __init__(self, name, config, capture_data):
        VolcanoWolf.__init__(self, name, config, WolfPostCommit, capture_data)
        
        self.updateTask('wlATIRadixSort', skiplist=[['.*']])
        self.updateTask('wlHistogram', skiplist=[['.*']])

class VolcanoWolfNightly(VolcanoWolf):
    def __init__(self, name, config, capture_data):
        VolcanoWolf.__init__(self, name, config, WolfBenchmark, capture_data)
        
        self.updateTask('wlATIRadixSort', skiplist=[['.*']])
        self.updateTask('Mandelbrot', skiplist=[['.*']])
        self.updateTask('colorbalance2', skiplist=[['.*']])
        self.updateTask('gauss', skiplist=[['.*']])
        self.updateTask('wlHistogram', skiplist=[['.*']])
        self.updateTask('wlHistogram_1', skiplist=[['.*']])
        self.updateTask('wlHistogram_2', skiplist=[['.*']])
        self.updateTask('wlHistogram_3', skiplist=[['.*']])
        self.updateTask('wlHistogram_4', skiplist=[['.*']])
        self.updateTask('wlHistogram_5', skiplist=[['.*']])
        self.updateTask('wlHistogram_6', skiplist=[['.*']])
        
class VolcanoWolfPerformance(VolcanoWolf):
    def __init__(self, name, config, capture_data):
        VolcanoWolf.__init__(self, name, config, WolfPerformanceConf, capture_data)

        self.updateTask('wlHistogram', skiplist=[['.*']])

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

    suites = {"Performance": WolfPerformance, 
              "PerformanceNew": WolfPerformanceNew,
              "PerformanceConf": WolfPerformanceConf,
              "PerformanceNewConf": WolfPerformanceNewConf, 
              "PerformanceFull": WolfPerformanceFull,
              "Benchmark": WolfBenchmark,
              "Fast": WolfPostCommit
             }

    if(options.suite not in suites):
        print "Unsupported suite '" + options.suite + "'. The suite must be one of: " + str(suites.keys())
        return 1

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              options.cpu,
                              options.cpu_features,
                              options.transpose_size)
    suite  = VolcanoWolf(options.suite, config, suites[options.suite], capture_data = options.capture)
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


