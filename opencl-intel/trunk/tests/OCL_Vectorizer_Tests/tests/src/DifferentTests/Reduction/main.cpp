// *******************************************************************************
// Reduction Example
// This is an example of sum reduction 
//
//		Author: Mohammed Agabaria
//		Updates : 07/11/08	Realese
//				  17/12/08  Time Ext.
//				  14/01/09	Ported to OCL 1.0
// *******************************************************************************

// *******************************************************************************
// Include Headers
// *******************************************************************************
// include some opencl API libraries 
#include "opencl.h"
// for the refernce computation
#include "serial.h"

// genernal libraries
#include <fstream>
#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <math.h>

// *******************************************************************************
// Constants & Definitions
// *******************************************************************************
// some Constants
using namespace std;
#define MAX_BUF 64000

// what device you want to do the kernel on
#define CPU_VER 0
#define GPU_VER !CPU_VER

// what's the measure type you want, 0-Serial, Or 1-OpenCL Kernel
#define MEASURE_TYPE 1

// You may choose between the vector version or the normal version to exec.
#define Vector_VER	CPU_VER

// how many iterations
#define ITERATIONS 2048

// for viewing some extra debug info.
#define ALLOW_DBG_ 0

#define PATH_0		"/Users/magabari/Documents/NewVersion/Global/Reduction/kernel0"
#define PATH_1		"/Users/magabari/Documents/NewVersion/Global/Reduction/kernel1"
#define PATH_2		"/Users/magabari/Documents/NewVersion/Global/Reduction/kernel2"

// The Path for the vector version of the kernel file.
// OPTIMAL FOR GPU
#define PATH_3		"/Users/magabari/Documents/NewVersion/Global/Reduction/kernel3"
// OPTIMAL FOR CPU
#define PATH_4		"/Users/magabari/Documents/NewVersion/Global/Reduction/kernel_vec"

// ERROR MACRO
#define ERROR_CHK(PARM) if(PARM != CL_SUCCESS) printf("Error: Return with %d\n", PARM);

// choose the default kernel versions
#if CPU_VER 
	#define	DEFAULT_VER 4
#endif
#if GPU_VER
	#define DEFAULT_VER 3
#endif
// *******************************************************************************
// Memory Object Flags
// *******************************************************************************
#if CPU_VER
#define SRC_FLAG CL_MEM_USE_HOST_PTR
#define DEST_FLAG CL_MEM_USE_HOST_PTR
#endif

#if GPU_VER
#define SRC_FLAG CL_MEM_COPY_HOST_PTR
#define DEST_FLAG 0
#endif

// *******************************************************************************
// Data configuration
// *******************************************************************************
// how many elements in the input array to reduction
const int ELEMENT_N = 1 << 23;

// some data over the cpu
const int num_of_cpu_threads = 2;
const int vector_sz = 4;

// the threshold of the CPU DEVICE
const int Threshold_cpu = num_of_cpu_threads * vector_sz;

// the size
const int ELEMENT_SZ = ELEMENT_N * sizeof(int);

// *******************************************************************************
// Function Implementations
// *******************************************************************************

// *******************************************************************************
// Rdtsc - this function measures how many clock's has been passed while doing
//		the computaion, it's needed for measuring the time that has been passed
// *******************************************************************************
unsigned long long int rdtsc()
{
	unsigned long long int x;	
	__asm__ volatile (".byte 0x0f, 0x31" : "=A"  (x));
	return x;
}

// *******************************************************************************
// ABS Function
// *******************************************************************************
float abs(float x)
{
	if( x > 0 ) return x;
	return -x;
}
// *******************************************************************************
// Load OpenCL source into OpenCL driver from file
// - Returns valid (non-NULL) cl_program if succeeded and 0 otherwise
// *******************************************************************************
cl_program createProgramFromFile(cl_context context, const char* fileName)
{
	// Load OpenCL source file
	FILE* in  =fopen(fileName, "r");
	if(in == NULL){ 
		printf("ERROR: FILE CAN'T OPEN ");
		return 0;
	}
	
	FILE* out =fopen("Compile.tmp", "w");
	int i=0;
	
	// just for count the file characters
	while(!feof(in)){
		++i;
		fputc(fgetc(in), out);
		}
	fclose(in);
	fclose(out);
	
	int clCodeStrSize = i;
	char *clCodeStr = new char[clCodeStrSize + 1];
	assert(clCodeStr);	// if an error happen
	
	ifstream inFile(fileName);
	if (!inFile) {
		cerr << "Failed to open '" << fileName << "'\n" << endl;
		return 0;
	}
	inFile.read(clCodeStr, clCodeStrSize);
	clCodeStr[clCodeStrSize] = '\0';
	inFile.close();
	
	// Load the source into OpenCL driver
	cl_int err;
	cl_program prog = clCreateProgramWithSource(context, 1, (const char**)&clCodeStr, NULL, &err);
	if (!prog) {
		cerr << "Failed to load source into OpenCL driver\n";
		return 0;
	}
	return prog;
}
// *******************************************************************************
// Bind arguments and execute the kernel 
// - NOTE: This is hard-coded just to run this kernel
// *******************************************************************************
void runKernel(int type, cl_command_queue clWorkQueue, cl_device_id deviceId, cl_context context
			   , cl_kernel Kernel, cl_mem arr1, cl_mem arr2, unsigned int numElmnts, unsigned int Threshold)
{
	// printf("Setting and Executing the kernel ... ");
	
	// Execute kernel
	size_t globalSize = numElmnts;
	
	// Currect just for the vector version
	if(Vector_VER) {
		clGetDeviceInfo(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(unsigned int), &globalSize, NULL);	
		}
	
	unsigned int maxThreadGroupSize = 0;
	clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(unsigned int), &maxThreadGroupSize, NULL);
	size_t localSize = std::min(maxThreadGroupSize, (unsigned int)globalSize);
	
#if GPU_VER
	localSize = 256;
#endif
	
	if(type == 3) localSize /= 2;
	
	// Set args to kernel
	void *args[]	=	{NULL, arr1, arr2};
	size_t sizes[3] = 	{localSize * sizeof(float), sizeof(cl_mem), sizeof(cl_mem)};
	
	if(type != 4) {
		clSetKernelArg(Kernel, 0, sizes[0], NULL);	
		for(int i=1; i<3; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);
		}
	else	// type 4 which means vector kernel
		{
		unsigned int arg_d[2] = {numElmnts, globalSize};
		
		cl_int err;
		cl_mem device_arg = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), 2*sizeof(unsigned int), arg_d, &err);
		ERROR_CHK(err);
			
		args[0]		= device_arg;
		sizes[0]	= sizeof(cl_mem);
		for(int i=0; i<3; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);
		}
	
	// Check if we can invoke the kernel, just kernel 4 don't assume there's more than one thread in local pool
	if(localSize == 1 && type != 4) {	// req. min 2 local threads in order to execute the parallel version
		printf("[You may execute the serial program]\n");
		exit(0);
		}
	
	cl_int clErr = clEnqueueNDRangeKernel(clWorkQueue, Kernel, 1, NULL ,&globalSize, &localSize, 0, NULL, NULL);
	ERROR_CHK(clErr);
	
	if (ALLOW_DBG_)
	printf("LC %d / GL %d / Blocks %d ... ", (int)localSize, (int)globalSize, (int)globalSize / (int)localSize);
	
	// Re-invoke the kernel to continue calculating, till we will get to the 
	// suitable break point
	
	if(type != 4) { // Not relevant for vector version
		unsigned int s = globalSize / localSize;
		if(type == 3) s /= 2;

		while (s > Threshold) {
			
			localSize = std::min(maxThreadGroupSize, s);		
			if(type == 3) localSize /= 2;
		
			// Set args to kernel
			void *args[]	= {NULL, arr2, arr2};
			size_t sizes[]	= {localSize * sizeof(float), sizeof(cl_mem), sizeof(cl_mem)};
		
			clSetKernelArg(Kernel, 0, sizes[0], NULL);	
			for(int i=1; i<3; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);
		
			clEnqueueNDRangeKernel(clWorkQueue, Kernel, 1, NULL ,(size_t*)&s, &localSize, 0, NULL, NULL);		
		
			if(type == 3) s = s / (2 * localSize);
			else s = s / localSize;
			}
		}
	//printf("Success\n");
}
// *******************************************************************************
//	Main Program
// *******************************************************************************
int main (int argc, char * const argv[]) 
{
	cl_int err;
	
	printf("[Reduction Example] \n");
	int ker_v = DEFAULT_VER;
	cl_device_id device_id;
	
	// Searching for suitable devices , computation units
	if(CPU_VER) err=clGetDeviceIDs(CL_DEVICE_TYPE_CPU, 1, &device_id, NULL); 
	else if(GPU_VER) err=clGetDeviceIDs(CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
	ERROR_CHK(err);
	
	// Creating Context for the device
	cl_context context;
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);		
	printf("Creating Context ... ");
	ERROR_CHK(err);
	if(context != NULL) printf("Success\n");
	else {
		printf("Failure\n");
		return 0;
	}

	cl_command_queue WorkQueue= clCreateCommandQueue(context, device_id, 0, &err);
	ERROR_CHK(err);		
	
	// after initailizing we may link the openCL source to our program
	const char *clSourceName = NULL;
	switch (ker_v) {
		case	0: clSourceName = PATH_0;
				break;
		case	1: clSourceName = PATH_1;
				break;			
		case	2: clSourceName = PATH_2;
				break;
		case	3: clSourceName = PATH_3;
				break;
		case	4: clSourceName = PATH_4;
				break;
		}
    int clErr = CL_SUCCESS;	
	
	// Load and compile OpenCL code
	printf("Linking openCL source file to the project (%s) ... ",clSourceName);
	cl_program prog = createProgramFromFile(context, clSourceName);
	if(prog == 0){
		printf("Failure\n");
		return 0;
		}
	printf("Success\n");
	
	// Compile and build the source
	printf("Building the Source that has been loaded ... ");
	
	clErr = clBuildProgram(prog, 0, NULL, NULL, NULL, NULL);
	if (clErr != CL_SUCCESS) {
		cout << "clBuildProgramExecutable returned = " << err << endl;
		char buildLog[1024];
		cl_device_id dev;
		if(CPU_VER) err = clGetDeviceIDs(CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
		else err = clGetDeviceIDs(CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
		ERROR_CHK(err);
		err = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, 1024, buildLog, NULL);
		ERROR_CHK(err);
		cout << "buildLog = \n\t" << buildLog << endl;
		exit (-1);
		}
	printf("Success\n");
	
	// Creating the kernel
	printf("Creating and linking the kernel to the program ... ");
	// Obtain handle to executable OpenCL kernel 
	cl_kernel Kernel = clCreateKernel(prog, "reduce", &err);
	if (!Kernel) {
		printf("Failure\n");
		return 0;
		}	
	printf("Success\n");
	
	// DEVICE & HOST Array's
	const unsigned int M = ELEMENT_N ;		// assume that is power of 2 
	int Threshold = 1;
	if(ker_v == 4) Threshold = Threshold_cpu;
	float *host_data_1 , host_result , h_device_result, *compute_device;
	
	compute_device	= (float*) malloc(Threshold * sizeof(float));
	host_data_1		= (float*) malloc(M * sizeof(float));
	
	// putting some values on the data input array
	for (unsigned int i = 0; i < M; ++i) {host_data_1[i] = 1 /*(float)(i % 33)*/;} 
	
	cl_mem device_arr_1, device_result;
	long long total = 0, serial, mem_total, total_mem;	
	
	if(MEASURE_TYPE == 1) {
		long long counter_b_mem = rdtsc();		
		// Creating device arrays -- [M], and copying the host values
		device_arr_1 = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), M*sizeof(float), host_data_1, &err);
		ERROR_CHK(err);
	
		// Allocate OpenCL empty result array on the device		
	#if !Vector_VER
		device_result = clCreateBuffer(context, (cl_mem_flags)(0), M*sizeof(float), NULL, &err);
		ERROR_CHK(err);
	#endif
	#if Vector_VER
		device_result = clCreateBuffer(context, (cl_mem_flags)(0), vector_sz*sizeof(float), NULL, &err);
		ERROR_CHK(err);
	#endif
		
		long long counter_a_mem = rdtsc();
		total_mem = counter_a_mem - counter_b_mem;
		}

	printf("Press Any Key to Start ...");
	char ch;
	ch=getchar();
	long long counter_a, counter_b;
	int NUM = ITERATIONS;
	
	if(MEASURE_TYPE == 1) {
		
		// Warming UP the Device 
		runKernel(ker_v, WorkQueue, device_id, context, Kernel, device_arr_1, device_result, M, Threshold);
		clFinish(WorkQueue);	
		
		counter_b = rdtsc();	
		for(int i=0; i< NUM ; i++){
			
			// Ready for starting the computation
			runKernel(ker_v, WorkQueue, device_id, context, Kernel, device_arr_1, device_result, M, Threshold);
			clFinish(WorkQueue);
			}
		err = clReleaseKernel(Kernel);
		ERROR_CHK(err);
		
		counter_a = rdtsc();
		total = counter_a - counter_b;
		
		long long mem_count_a, mem_count_b;
		mem_count_a = rdtsc();			
		
		// Copying the results to the host memory
#if CPU_VER
		compute_device = (float*)clEnqueueMapBuffer(WorkQueue, device_result, 
							CL_TRUE, CL_MAP_READ, 0, Threshold*sizeof(float), 0, NULL, NULL, &err);
#endif
#if GPU_VER
		// Copying the results to the host memory
		clEnqueueReadBuffer(WorkQueue, device_result, CL_TRUE, 0, sizeof(float), 
					compute_device, 0, NULL, NULL);
		h_device_result = compute_device[0];
#endif
		ERROR_CHK(err);	
		mem_count_b = rdtsc();
		mem_total = mem_count_b -mem_count_a;
	
		counter_b = rdtsc();
		// if Thresold > 1 we need to continue the sum serially
		if(Threshold > 1) h_device_result = runSerial_Reduce(compute_device, Threshold);
		counter_a = rdtsc();
		total += NUM*(counter_a - counter_b);
	}
	
	// checking the results
	printf("running HOST Reduction calculation.\n");
	host_result = runSerial_Reduce(host_data_1, M);
	
	if(MEASURE_TYPE == 0) {	
		int i;
		// Warming Up
		host_result = runSerial_Reduce(host_data_1, M);
		
		int NUM = ITERATIONS;
		long long serial_a, serial_b;
		
		serial_a = rdtsc();		
		for(i =0; i< NUM; i++) {
			host_result = runSerial_Reduce(host_data_1, M);
			}
		serial_b = rdtsc();
		serial = (serial_b - serial_a)/NUM;		
		}
	
	printf("Results : \n");
	if(MEASURE_TYPE == 0) printf("CPU Serial Compute: %llu\n",serial);
	else printf("Device OpenCL: Compute %llu, Mem-Dev %llu, Dev-Mem %llu\n",total/NUM, total_mem,mem_total);
	
#if ALLOW_DBG_
	printf("Array Data[] : ");
	for(int i=0; i< M; i++) printf(" %.2f ",host_data_1[i]);
	printf("\n");
	
	printf("Host Res : ");
	printf(" %.2f ",host_result);
	printf("\n");
	
	printf("Dev. Out[] : ");
	for(int i=0; i<Threshold; i++) printf(" %.2f ",compute_device[i]);
	printf("\n");
	
	printf("Dev. Res : ");
	printf(" %.2f ",h_device_result);
	printf("\n");
#endif
	
	if(MEASURE_TYPE == 1) {
		// checking the results
		printf("Checking the results ... ");
		int cnt = 0;
		if (abs(host_result - h_device_result) < 1e-7) cnt++;
		if(cnt != 1) printf("Failure\n");
		else	printf("Success\n");

		// Realesing the resources
		clReleaseMemObject(device_arr_1);
		clReleaseMemObject(device_result);
		}
	
	//free(compute_device);
	free(host_data_1);
	
	clReleaseContext(context);
    return 0;
}
