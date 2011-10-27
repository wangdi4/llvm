// *******************************************************************************
// Scalar Prod. Example
// This sample calculates scalar products of a given set of input vector pairs
//
//		Author: Mohammed Agabaria
//		Updates : 27/11/08	Realese
//				  25/12/08  Added Time Ext.
//				  25/12/08  GPU kernel added
//				  01/01/09	Proted to OCL 1.0
// *******************************************************************************

// *******************************************************************************
// Include Headers
// *******************************************************************************

// Include OpenCL API Libraries
#include "opencl.h"

// Contains the Serail version of the operation, just for checking the result.
#include "serial.h"

// Some other libraries.
#include <fstream>
#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <math.h>

// *******************************************************************************
// Constants & Definitions
// *******************************************************************************

using namespace std;
#define MAX_BUF 64000

// for debug mode, it will proview some extra info. to the screen.
#define ALLOW_DBG_ 0

// what device you want to do the kernel on
#define CPU_VER 1
#define GPU_VER !CPU_VER

// what's the measure type you want, 0-Serial, Or 1-OpenCL Kernel
#define MEASURE_TYPE 1

// You may choose between the vector version or the normal version to exec.
#define Vector_VER	CPU_VER

// how many iterations
#define ITERATIONS 1024

// The Path for the vector version of the kernel file.
// OPTIMAL FOR CPU
#if (Vector_VER == 1)
#define _PATH "/Users/magabari/Documents/NewVersion/Global/scalarProd/kernel"
#endif

// OPTIMAL FOR GPU
#if (Vector_VER == 0)
#define _PATH "/Users/magabari/Documents/NewVersion/Global/scalarProd/kernel_gpu"
#endif

// ERROR MACRO
#define ERROR_CHK(PARM) if(PARM != CL_SUCCESS) printf("Error: Return with %d\n", PARM);

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

// Total number of input vector pairs; 
const int VECTOR_N = 4096;

// Number of elements per vector; 
const int ELEMENT_N = 4096;

// Total number of data elements
const int    DATA_N = VECTOR_N * ELEMENT_N;

// The size of the input and result arrays.
const int   DATA_SZ = DATA_N * sizeof(float);
const int RESULT_SZ = VECTOR_N  * sizeof(float);

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
// RandFloat -Helper function, returning uniformly distributed
// random float in [low, high] range
// *******************************************************************************
float RandFloat(float low, float high)
{
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}

// *******************************************************************************
// createProgramFromFile -Load OpenCL source into OpenCL driver from file
// - Returns valid (non-NULL) cl_program if succeeded and NULL otherwise
// *******************************************************************************
cl_program createProgramFromFile(cl_context context, const char* fileName)
{
	// Load OpenCL source file
	FILE* in =fopen(fileName, "r");
	if(in == NULL) { 
		printf("ERROR: FILE CAN'T OPEN");
		return NULL;
		}
	
	FILE* out =fopen("Compile.tmp", "w");
	unsigned int clCodeStrSize=0;
	
	// just for count the file characters
	while(!feof(in)) {
		fputc(fgetc(in), out);
		++clCodeStrSize;
		}
	// Closing the files.
	fclose(in);
	fclose(out);
	
	char *clCodeStr = new char[clCodeStrSize + 1];
	assert(clCodeStr);	// if an error happen
	
	ifstream inFile(fileName);
	if (!inFile) {
		cerr << "Failed to open '" << fileName << "'\n" << endl;
		return NULL;
		}
	inFile.read(clCodeStr, clCodeStrSize);
	clCodeStr[clCodeStrSize] = '\0';
	inFile.close();
	
	// Load the source into OpenCL driver
	cl_int err;
	cl_program prog = clCreateProgramWithSource(context, 1, (const char**)&clCodeStr, NULL, &err);
	ERROR_CHK(err);
	
	delete clCodeStr;	
	if (!prog) {
		cerr << "Failed to load source into OpenCL driver\n";
		return NULL;
	}
	return prog;
}
// *******************************************************************************
// Bind arguments and execute the kernel [ scalarProd kernel ]
// - NOTE: This is hard-coded just to run this kernel
// *******************************************************************************
void runKernel(cl_command_queue clWorkQueue, cl_device_id deviceId, cl_context context,
				  cl_kernel Kernel, cl_mem arr1, cl_mem arr2, cl_mem arr3)
{
	//printf("Setting and Executing the kernel ... ");

	// Execute kernel -- Every thread deals with a pair of vectors (dot)
	size_t globalSize = CPU_VER ? 2 : VECTOR_N; // two threads for the CPU
	
	unsigned int maxThreadGroupSize = CPU_VER ? 1 : 256;
	size_t localSize = (size_t)std::min(maxThreadGroupSize, (unsigned int)globalSize);

	if(globalSize % localSize != 0) globalSize = globalSize + (localSize - globalSize % localSize);
	
	//int size_h[1]={globalSize / localSize};
	cl_int err;
	// Set args to kernel
	void* args[] = {arr3, arr1, arr2};
	size_t sizes[] = {sizeof(cl_mem), sizeof(cl_mem), sizeof(cl_mem), localSize*sizeof(float), sizeof(cl_mem)};
#if Vector_VER
	for(int i=0; i<3; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);	
#endif
	
#if (!Vector_VER)
	for(int i=0; i<3; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);
	err=clSetKernelArg(Kernel, 3, sizes[3], NULL); // local
#endif
	
	// simple addresing of the threads (simple version) 1-dim
	err= clEnqueueNDRangeKernel(clWorkQueue, Kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Failure\n");
		exit(0);
		}
	
	//printf("Success\n");
}

// *******************************************************************************
// Main Program
// *******************************************************************************
int main (int argc, char * const argv[]) 
{	
	cl_int err;
	
	printf("[scalarProd. Example] \n");
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
	const char *clSourceName = _PATH;
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
	cl_kernel Kernel = clCreateKernel(prog, "scalarProd", &err);
	if (!Kernel) {
		printf("Failure\n");
		return 0;
		}	
	printf("Success\n");

	// DEVICE & HOST Array's
	float  *h_A, *h_B, *h_C_HOST, *h_C_DEVICE;
    cl_mem d_A, d_B, d_C;
    double delta, ref, sum_delta, sum_ref, L1norm;
    int i;	

	printf("Initializing data...\n");
	printf("\t...allocating HOST memory.\n");
	h_A			= (float *)malloc(DATA_SZ);
	h_B			= (float *)malloc(DATA_SZ);
	h_C_HOST	= (float *)malloc(RESULT_SZ);
	h_C_DEVICE	= (float *)malloc(RESULT_SZ);	

	printf("\t...generating input data in HOST mem.\n");
	srand(123);
	//Generating input data on HOST
	for(i = 0; i < DATA_N; i++){
		h_A[i] = RandFloat(0.0f, 1.0f);
		h_B[i] = RandFloat(0.0f, 1.0f);
		}	
	
	long long total = 0, serial, mem_total, total_mem;

	if(MEASURE_TYPE == 1) {
		long long counter_b_mem = rdtsc();		
		
		// Creating device arrays and copying the host values
		d_A		= clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG ), DATA_N*sizeof(float), h_A, &err);
		ERROR_CHK(err);
	
		d_B		= clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), DATA_N*sizeof(float), h_B, &err);
		ERROR_CHK(err);

		// Allocate OpenCL empty result array on the device
		d_C		= clCreateBuffer(context, (cl_mem_flags)(DEST_FLAG), VECTOR_N*sizeof(float), h_C_DEVICE, &err);	
		ERROR_CHK(err);
		
		long long counter_a_mem = rdtsc();
		total_mem = counter_a_mem - counter_b_mem;
		}
	
	printf("Press Any Key to Start ...");
	char ch;
	ch=getchar();
	long long counter_a, counter_b;
	int NUM = ITERATIONS;
		
	if(MEASURE_TYPE) {
		// Warming Up the Device
		runKernel(WorkQueue, device_id, context, Kernel, d_A, d_B, d_C);
		clFinish(WorkQueue);	

		counter_b = rdtsc();	
		for(int i=0; i< NUM ; i++){
	
			// Ready for starting the computation
			runKernel(WorkQueue, device_id, context, Kernel, d_A, d_B, d_C);
			clFinish(WorkQueue);
			}
		err = clReleaseKernel(Kernel);
		ERROR_CHK(err);
		
		counter_a = rdtsc();
		total = counter_a - counter_b;

		// printf("Time elapsed [avg] : %llu",total/NUM);
		// waiting for an input to end the program
		// printf("\nInsert any char to end the execution for the program ... ");
		// ch=getchar();
	
		long long mem_count_a, mem_count_b;
		mem_count_a = rdtsc();		
		// Copying the results to the host memory
		h_C_DEVICE = (float*)clEnqueueMapBuffer(WorkQueue, d_C, 
						CL_TRUE, CL_MAP_READ, 0, VECTOR_N*sizeof(float), 0, NULL, NULL, &err);
		ERROR_CHK(err);	
		
		mem_count_b = rdtsc();
		mem_total = mem_count_b -mem_count_a;		
		}
	
	// checking the results
	printf("running HOST scalar product calculation.\n");
	scalarProdSerial(h_C_HOST, h_A, h_B, VECTOR_N, ELEMENT_N);
	
	if(MEASURE_TYPE == 0) {
		int i;
		// Warming Up
		scalarProdSerial(h_C_HOST, h_A, h_B, VECTOR_N, ELEMENT_N);
		int NUM = ITERATIONS;
		long long serial_a, serial_b;
		
		serial_a = rdtsc();		
		for(i =0; i< NUM; i++) {
			// running serail computation on the host (just for checking the results)
			scalarProdSerial(h_C_HOST, h_A, h_B, VECTOR_N, ELEMENT_N);
			}
		serial_b = rdtsc();
		serial = (serial_b - serial_a)/NUM;
	}	
	
	printf("Results : \n");
	if(MEASURE_TYPE == 0) printf("CPU Serial Compute: %llu\n",serial);
	else printf("Device OpenCL: Compute %llu, Mem-Dev %llu, Dev-Mem %llu\n", total/NUM, total_mem, mem_total);
	
#if ALLOW_DBG_
	printf("Array A[] : ");
	for(int i=0; i< DATA_N; i++) printf(" %.2f ",h_A[i]);
	printf("\n");
	
	printf("Array B[] : ");
	for(int i=0; i< DATA_N; i++) printf(" %.2f ",h_B[i]);
	printf("\n");
	
	printf("Host Res[] : ");
	for(int i=0; i< VECTOR_N; i++) printf(" %.2f ",h_C_HOST[i]);
	printf("\n");
	
	printf("Dev. Res[] : ");
	for(int i=0; i< VECTOR_N; i++) printf(" %.2f ",h_C_DEVICE[i]);
	printf("\n");
#endif
	
	
	if(MEASURE_TYPE == 1) {
		printf("checking the results ... \n");
		//Calculate max absolute difference and L1 distance
		//between HOST and DEVICE results
		sum_delta = 0;
		sum_ref   = 0;
		for(i = 0; i < VECTOR_N; i++){
			delta = fabs(h_C_DEVICE[i] - h_C_HOST[i]);
			ref   = h_C_HOST[i];
			sum_delta += delta;
			sum_ref   += ref;
			}
		L1norm = sum_delta / sum_ref;
		printf("L1 error: %E\n", L1norm);
		printf((L1norm < 1e-6) ? "TEST PASSED\n" : "TEST FAILED\n");

		// Realesing the resources
		clReleaseMemObject(d_A);
		clReleaseMemObject(d_B);
		clReleaseMemObject(d_C);
		}
	
	printf("Press Any Key to Exit ...");
	ch=getchar();	
	
	//free(h_C_DEVICE);
	free(h_C_HOST);
	free(h_B);
	free(h_A);	
	
	clReleaseContext(context);
    return 0;
}
