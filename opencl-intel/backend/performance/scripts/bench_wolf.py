import subprocess
import os
import sys
import marshal
import time
	
def runme(command, Env):
	my_env = os.environ
	for key in Env:
		my_env[key] = Env[key]
	p = subprocess.Popen( command ,stdout=subprocess.PIPE,stderr=subprocess.PIPE, env=my_env)
	output, errors = p.communicate()
	return output.split('\n')
	
class Result:
	def __init__(self, name, cmd):
		self.name = name
		self.cmd = cmd
		
	def run(self, arch):
		self.scalar = runme(self.cmd, {"CL_CONFIG_USE_VECTORIZER": "false", "VOLCANO_ARCH": arch})
		self.vector = runme(self.cmd, {"CL_CONFIG_USE_VECTORIZER": "true" , "VOLCANO_ARCH": arch})
		self.scalar_build = "\"FAIL\""
		self.scalar_exec = "\"FAIL\""
		self.vector_build = "\"FAIL\""
		self.vector_exec = "\"FAIL\""
		if Result.didPass(self.scalar):
			#print "parsing ", "".join(self.scalar)
			self.scalar_build = Result.getTimeValue(self.scalar, ['clBuildProgram'])	
			self.scalar_exec =  Result.getTimeValue(self.scalar, ['Kernel Execution (OpenCL scalar)','Kernel Execution','Kernel execution','Complete Execution','Execution of'])
		if Result.didPass(self.vector):
			#print "parsing ", "".join(self.vector)
			self.vector_build = Result.getTimeValue(self.vector, ['clBuildProgram'])
			self.vector_exec =  Result.getTimeValue(self.vector, ['Kernel Execution (OpenCL scalar)','Kernel Execution','Kernel execution','Complete Execution','Execution of'])

	def __str__(self):
		return self.name + "," + \
		str(self.scalar_build) + "," + \
		str(self.scalar_exec) + "," + \
		str(self.vector_build) + "," + \
		str(self.vector_exec) + "," + \
		"\"" + " ".join(self.cmd) + '\",'
			
	@staticmethod
	def didPass(output):
		for line in output:
			if 'TEST SUCCEDDED' in line: return True
		return False
	
	@staticmethod
	def getTimeValue(output, labels):
		for label in labels:
			seen = False
			for line in output:
				if 'SUMMARY' in line: seen=True
				if not seen: continue
				if label.lower() in line.lower():
					tokens = line.split(',')
					return tokens[2] # time 
		return "\"NODATA\""

wolf_commands  =  \
"""WOLF.exe out_intel_matrix_mult.csv intel_matrix_mult 10 cfgFile=intel_matrix_mult.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_ati_matrix_mult.csv wlMatrixMultiplication 10 cfgFile=wlMatrixMultiplication.cfg,eDeviceTypes=CPU
WOLF.exe out_ati_prefix_sum.csv wlATIPrefixSum 10 cfgFile=wlATIPrefixSum.cfg,eDeviceTypes=CPU
WOLF.exe out_framework.csv wlFrameworkOverhead 10 cfgFile=measure.cfg,length=16777216,wg_size=1024,iterations=1000,eDeviceTypes=CPU
WOLF.exe out_pb_intel_bilateral2D.csv intel_bilateral2D 10 cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_dct_intel.csv wlDCT 10 cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=100,groupSize=8,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_histogram_intel.csv wlHistogram 10 cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU
WOLF.exe out_intel_median.csv intel_median 10 cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_median_bitonic.csv intel_median_bitonic 10 cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_prefix_sum_intel.csv wlPrefixSum 10 cfgFile=measure.cfg,iterations=1000,szArray=67108864,szMaxInt=1000,szLocalWork=8,szGlobalWork=1024,bPrintSerialReport=true,eDeviceTypes=CPU
WOLF.exe out_pb_intel_radial_blur.csv intel_radial_blur 10 cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_median.csv wlNVMedian 10 cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=16,eDeviceTypes=CPU
WOLF.exe out_intel_god_rays.csv intel_god_rays 10 cfgFile=intel_god_rays.cfg,KernelType=ScalarOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8
WOLF.exe out_intel_montecarlo.csv intel_MonteCarlo 10 cfgFile=measure.cfg,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU,WorkGroupSize=8
WOLF.exe out_convolution.csv wlConvolution 10 cfgFile=measure.cfg,iterations=100,CFG_CONVOLUTION_ARRAY_WIDTH=1920,CFG_CONVOLUTION_ARRAY_HEIGHT=1080,CFG_CONVOLUTION_LOCAL_WORGROUP_SIZE=128,eDeviceTypes=CPU
WOLF.exe out_nbody_amd.csv wlATINBody 10 cfgFile=measure.cfg,kernelName=nbody_sim,numParticles=32768,groupSize=128,iterations=10,eDeviceTypes=CPU
WOLF.exe out_intel_nbody.csv intel_NBody 10 cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_sobel.csv intel_sobel 10 cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8
WOLF.exe out_intel_sobel8u.csv intel_sobel8u 10 cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_convolution.csv intel_convolution 10 cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU
WOLF.exe out_intel_mandelbrot.csv intel_mandelbrot 10 cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,localSize=128,eDeviceTypes=CPU
WOLF.exe out_intel_rgb2yuv.csv intel_rgb2yuv 10 cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,groupSize=8,eDeviceTypes=CPU
WOLF.exe out_aes.csv wlATIAES 10 cfgFile=wlATIAES.cfg,decrypt=true,eDeviceTypes=CPU
WOLF.exe out_bitonic_sort.csv wlBitonicSort 10 cfgFile=wlBitonicSort.cfg,BITONIC_CFG_BUFFER_SIZE=1048576,BITONIC_CFG_LOCAL_WORKGROUP_SIZE=128,eDeviceTypes=CPU
WOLF.exe out_rec_gaus.csv wlATIRecGaussian 10 cfgFile=wlATIRecGaussian.cfg,eDeviceTypes=CPU,iterations=100
WOLF.exe out_intel_tcc.csv intel_tcc 10 cfgFile=measure.cfg,KernelType=ScalarOCL,TCC_CFG_FRAME_WIDTH=1920,TCC_CFG_FRAME_HEIGHT=1080,iterations=100,TCC_CFG_GLOBAL_WORKGROUP_SIZE=0,TCC_CFG_LOCAL_WORKGROUP_SIZE=64,eDeviceTypes=CPU
WOLF.exe out_intel_rec_gaus.csv intel_RecGaussian 10 cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_colorbalance.csv intel_colorbalance 10 cfgFile=measure.cfg,iterations=100,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_crossfade.csv intel_crossfade 10 cfgFile=measure.cfg,iterations=1000,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_gauss.csv intel_gauss 10 cfgFile=measure.cfg,iterations=1,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_intel_radar.csv intel_radar 10 cfgFile=measure.cfg,iterations=10,KernelType=ScalarOCL,eDeviceTypes=CPU,groupSize=8
WOLF.exe out_intel_bas.csv intel_bas 10 cfgFile=measure.cfg,iterations=500,KernelType=ScalarOCL,eDeviceTypes=CPU
WOLF.exe out_ati_radix_sort.csv wlATIRadixSort 10 cfgFile=measure.cfg,iterations=500,KernelType=ScalarOCL,eDeviceTypes=CPU,elementCount=65536
WOLF.exe out_intel_matrix_mult.csv intel_matrix_mult 10 cfgFile=intel_matrix_mult.cfg,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_pb_checkerboard.csv checkerboard 10 cfgFile=checkerboard.cfg,CFG_checkerboard_LOCAL_WORK_SIZE0=256,eDeviceTypes=CPU
WOLF.exe out_pb_intel_bilateral2D.csv intel_bilateral2D 10 cfgFile=intel_bilateral2D.cfg,CFG_intel_bilateral2D_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_dct_intel.csv wlDCT 10 cfgFile=measure.cfg,CFG_DCT_ARRAY_WIDTH=2048,CFG_DCT_ARRAY_HEIGHT=2048,iterations=100,groupSize=8,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_histogram_intel.csv wlHistogram 10 cfgFile=measure.cfg,szGlobalWorkStep1=1024,szGlobalWorkStep2=1024,szLocalWorkStep1=1,szLocalWorkStep2=1,szBin=131072,szMatrix=16777216,bPrintSerialReport=true,eDeviceTypes=CPU
WOLF.exe out_intel_median.csv intel_median 10 cfgFile=measure.cfg,iterations=100,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_median_bitonic.csv intel_median_bitonic 10 cfgFile=measure.cfg,iterations=1000,CFG_MEDIAN_ARRAY_WIDTH=1024,CFG_MEDIAN_ARRAY_HEIGHT=1024,CFG_MEDIAN_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_pb_intel_radial_blur.csv intel_radial_blur 10 cfgFile=intel_radial_blur.cfg,CFG_intel_radial_blur_LOCAL_WORK_SIZE=8,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_god_rays.csv intel_god_rays 10 cfgFile=intel_god_rays.cfg,KernelType=VectorOCL,eDeviceTypes=CPU,CFG_intel_god_rays_LOCAL_WORK_SIZE=8
WOLF.exe out_intel_montecarlo.csv intel_MonteCarlo 10 cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU,WorkGroupSize=8
WOLF.exe out_intel_nbody.csv intel_NBody 10 cfgFile=measure.cfg,szBodies=32768,szGlobalWork=32768,groupSize=8,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_sobel.csv intel_sobel 10 cfgFile=measure.cfg,groupSize=8,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_sobel8u.csv intel_sobel8u 10 cfgFile=measure.cfg,groupSize=8,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_convolution.csv intel_convolution 10 cfgFile=measure.cfg,groupSize=8,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_mandelbrot.csv intel_mandelbrot 10 cfgFile=measure.cfg,localSize=8,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_mandelbrot.csv wlMandelbrot 10 CFG_MANDELBROT_WIDTH=512,CFG_MANDELBROT_LOCAL_WORK_SIZE=128,bMeasureNQNDRange=false,bMeasureReadBuffer=false,bMeasureWriteBuffer=false,eDeviceTypes=CPU
WOLF.exe out_intel_rgb2yuv.csv intel_rgb2yuv 10 cfgFile=measure.cfg,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_rec_gaus.csv intel_RecGaussian 10 cfgFile=intel_RecGaussian.cfg,GAUSSIAN_CFG_LOCAL_WORKGROUP_SIZE=8,GAUSSIAN_CFG_BLOCK_DIM=8,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_tone_mapping.csv intel_tone_mapping 10 cfgFile=intel_tone_mapping.cfg,KernelType=VectorOCL,iterations=100,CFG_intel_tone_mapping_LOCAL_WORK_SIZE=8,eDeviceTypes=CPU
WOLF.exe out_wltccace.csv wlTCCACE 1 cfgFile=wlTCCACE.cfg
WOLF.exe out_wlLcs.csv wlLcs 10 cfgFile=wlLcs.cfg,KernelType=VectorOCL
WOLF.exe out_intel_colorbalance.csv intel_colorbalance 10 cfgFile=measure.cfg,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_wlSimpleBoxBlur.csv wlSimpleBoxBlur 10 cfgFile=measure.cfg,CFG_SIMPLEBOXBLUR_LOCAL_THREAD_0=4,iterations=100,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_crossfade.csv intel_crossfade 10 cfgFile=measure.cfg,iterations=1000,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_gauss.csv intel_gauss 10 cfgFile=measure.cfg,iterations=1,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_radar.csv intel_radar 10 cfgFile=measure.cfg,groupSize=8,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_intel_bas.csv intel_bas 10 cfgFile=measure.cfg,iterations=500,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_ati_mersenne_twister.csv wlMersenneTwister 10 cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_landscape.csv wl2Landscape 10 cfgFile=measure.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU
WOLF.exe out_subdivision.csv wlSubdivision 10 cfgFile=subdivision.cfg,iterations=10,KernelType=VectorOCL,eDeviceTypes=CPU,MeshFileName=bigguy.obj"""
	
os.environ['VOLCANO_VECTORIZE'] = 'enabled'

class Benchmark(object):

	def __init__(self):
		self.db = {}
		self.results = []
		
	def addResult(self, result):
		self.results.append(result)
		kernel = result.name
		self.db["%s.build.scalar"%kernel] = result.scalar_build
		self.db["%s.build.vector"%kernel] = result.vector_build
		self.db["%s.exec.scalar"%kernel]  = result.scalar_exec
		self.db["%s.exec.vector"%kernel]  = result.vector_exec

	def printResults(self):
		for res in self.results:
			print res

	def load_db(self, arch):
		try:
			filename = "c:\\work\\nrotem\\perf.%s.db"%arch
			print "loading %s" % filename 
			fh = open(filename,"r")
			self.olddb = marshal.loads(fh.read())
		except:
			self.olddb = {}
			
	def save_db(self, arch):
		try:
			filename = "c:\\work\\nrotem\\perf.%s.db"%arch
			print "saving %s" % filename , 
			fh = open(filename,"wb")
			self.db = marshal.dump(self.db, fh)
			fh.close()
			print 'done'
		except Exception, e:
			print "unable to save db", e
	
	def reportDeltas(self, delta):
		for key in self.db:
			try:
				score = float(self.db[key])
			except:
				score = None
			try:
				oldscore = float(self.olddb[key])
			except:
				oldscore = None
			
			if (not bool(score) and bool(oldscore)):
				print key, " - New Failure"
			if ((not bool(oldscore)) and bool(score)):
				print key, " - New Pass"
			if (bool(score) and bool(oldscore)):
				ratio = (oldscore/score)
				if abs(ratio-1) > delta:
					print key, " perf: ", (ratio-1) , ">", delta, "  -- old:%f  new:%f "%(oldscore, score)
			
			
			
			
		
arch = sys.argv[-1]
supported_archs = ["sandybridge","corei7"]

if (not arch in supported_archs):
	print "Unsupported arch:", arch, "from cli:", str(sys.argv)
	sys.exit(1)

bench = Benchmark()	

print ",",arch,","
print "Name, Scalar.Build, Scalar.Exec, Vector.Build, Vector.Exec,Command, Wall clock,"
for cmd in wolf_commands.split('\n'):	
	tokens = cmd.split(' ')
	r = Result(tokens[2], tokens)
	start = time.time()
	r.run(arch)	
	bench.addResult(r)
	end = time.time()
	print r, 
	print end-start
	sys.stdout.flush()
