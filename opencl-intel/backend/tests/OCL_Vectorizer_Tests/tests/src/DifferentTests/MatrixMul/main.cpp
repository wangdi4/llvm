// *******************************************************************************
// Matrix Multiply Example
// This is an example demonstrates the usage of program and kernel objects
// This example implements matrix multiplication
//
//		Author: Mohammed Agabaria
//		Updates : 27/09/08	Realese
//				  18/12/08  Time Ext. Added
//				  07/01/09	Ported to OCL 1.0
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
#define MAX_BUF		64000
#define ALLOW_DBG_	0
#define _EXT_INFO	0

// what device you want to do the kernel on
#define CPU_VER 1
#define GPU_VER !CPU_VER

// how many iterations
#define ITERATIONS 128

#define _USE_BLOCK_SIZE	0
#define _VECTOR_VER	CPU_VER
#define VECTOR_SIZE	4

// what's the measure type you want, 0-Serial, Or 1-OpenCL Kernel
#define MEASURE_TYPE 1

// The Path for the vector version of the kernel file.
// OPTIMAL FOR CPU
#if (_VECTOR_VER)
#define _PATH		"/Users/magabari/Documents/NewVersion/Global/MatrixMul/kernel_vec"
#endif

// OPTIMAL FOR GPU
#if (!_VECTOR_VER)
#define _PATH		"/Users/magabari/Documents/NewVersion/Global/MatrixMul/kernel"
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
// Data Configration
// *******************************************************************************

// block size of the matrix
#define BLOCK_SIZE 4

// Matrix dimensions
// (chosen as multiples of the thread block size for simplicity)
#define WA (251 * BLOCK_SIZE) // Matrix A width
#define HA (142 * BLOCK_SIZE) // Matrix A height
#define WB (51 * BLOCK_SIZE) // Matrix B width

// don't change
#define HB WA  // Matrix B height
#define WC WB  // Matrix C width 
#define HC HA  // Matrix C height

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
// Finds the greatest value which divides the divider and lower from upvalue
// *******************************************************************************
int find_suitable_dim(int divider1, int divider2,int upvalue)
{
	int i;
	for(i=upvalue; i > 1; --i) {
		if(divider1 % i == 0 && divider2 % i == 0) return i;
	}
	return i;
}
// *******************************************************************************
// ABS function - returns the absolute value.
// *******************************************************************************
float abs(float x){
	float res = x;
	if (x < 0) res = -x;
	return res;
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
	ERROR_CHK(err);
	if (!prog) {
		cerr << "Failed to load source into OpenCL driver\n";
		return 0;
	}
	
	return prog;
}
// *******************************************************************************
// Bind arguments and execute the kernel [ arr3 = arr1 * arr2 ]
// - NOTE: This is hard-coded just to run this kernel
// *******************************************************************************
void runKernel(cl_command_queue clWorkQueue, cl_context context, cl_device_id deviceId,
				  cl_kernel Kernel, cl_mem arr1, cl_mem arr2, cl_mem arr3, 
				  unsigned int M, unsigned int N, unsigned int R)
{
	// printf("Setting and Executing the kernel ... ");
	
	// Execute kernel
	int vector = 1;
	if(_VECTOR_VER) vector = VECTOR_SIZE;
	size_t globalSize[2] = {M / vector ,R / vector};
	
	unsigned int maxThreadGroupSize = 0;
	clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, 
						  sizeof(unsigned int), &maxThreadGroupSize, NULL);
	
	float root = std::min(maxThreadGroupSize, M * R);
	if(GPU_VER) root = 256; // suits the G80
	root = sqrt(root);
	
	int M_dim = find_suitable_dim(M, R, floor(root));
	int R_dim = find_suitable_dim(M, R, floor(root));	
	size_t localSize[2] = {(int)M_dim, (int)R_dim}; // for local mem.
	
	if(_USE_BLOCK_SIZE == 1 && _VECTOR_VER == 0) {	
	// assuming the BLOCK_SIZE^2 < maxThreadGroupSize
	localSize[0] = BLOCK_SIZE;
	localSize[1] = BLOCK_SIZE;
	}
	
	cl_int err;
	// Creating an array of three values which contains the number of the cell's in the matrix
	//	- A will be M X N , B is N X R thus C the result will be M X R
	int arg_h[5] = {M, N, R, localSize[0], localSize[1]};
	cl_mem arg = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), 5*sizeof(int), arg_h, &err);
	ERROR_CHK(err);
	
	// Set args to kernel 
	void *args[] = {arg, arr1, arr2, arr3, NULL, NULL};
	size_t sizes[] = {sizeof(cl_mem), sizeof(cl_mem), sizeof(cl_mem), sizeof(cl_mem)
			, localSize[0] * localSize[1] * sizeof(float)
			, localSize[0] * localSize[1] * sizeof(float)};
	
	for (int i=0; i<4; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);
	if(_VECTOR_VER == 0) {
		clSetKernelArg(Kernel, 4, sizes[4], NULL);
		clSetKernelArg(Kernel, 5, sizes[4], NULL);
	}
	
#if _EXT_INFO	
	printf("\n\tExtention of the Thread Net ...");
	printf("\n\tBefore extending:  GL [%d , %d]   LC [%d , %d]\n",globalSize[0],globalSize[1],localSize[0],localSize[1]);
#endif
	// adjusting the global size to divide the local size of threads
	if(globalSize[0] % localSize[0] != 0) 
			globalSize[0] = globalSize[0] + (localSize[0] - globalSize[0] % localSize[0]);
	if(globalSize[1] % localSize[1] != 0) 
			globalSize[1] = globalSize[1] + (localSize[1] - globalSize[1] % localSize[1]);

#if _EXT_INFO
	printf("\tAfter  extention:  GL [%d , %d]   LC [%d , %d]\n",globalSize[0],globalSize[1],localSize[0],localSize[1]);	
#endif
	// simple addresing of the threads  2-dim
	int clErr = clEnqueueNDRangeKernel(clWorkQueue, Kernel, 2, NULL, globalSize, localSize, 0, NULL,  NULL);
	
	if (clErr != CL_SUCCESS) {
		printf("Execute Failure\n");
		exit(0);
	}
	// printf("Execute Success\n");
}
// *******************************************************************************
// Main Program
// *******************************************************************************
int main (int argc, char * const argv[]) 
{
	cl_int err;
	
	printf("[Matrix Multiply Example] \n");
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
	cl_kernel Kernel = clCreateKernel(prog, "multiply", &err);
	if (!Kernel) {
		printf("Failure\n");
		return 0;
		}	
	printf("Success\n");

	// Allocate host arrays [ HOST ARRAYS ]
	const int M = HA , N = WA , R = WB;	// A[M X N] * B[N X R] = C[M X R]
	float *host_data_1 , *host_data_2 , *host_result , *compute_device;
	
	host_data_1		= (float*) malloc(M*N*sizeof(float));
	host_data_2		= (float*) malloc(N*R*sizeof(float));
	compute_device	= (float*) malloc(M*R*sizeof(float)); // for the device return
	host_result		= (float*) malloc(M*R*sizeof(float)); // refrence compute
	
	// putting some values on the data matrices
	srand(0);
	for (int i = 0; i < N*M; ++i) host_data_1[i] = (float)(i%7);
	for (int i = 0; i < N*R; ++i) host_data_2[i] = (float)(i%7) -3;	
	
	// DEVICE MEMORY
	cl_mem device_arr_1, device_arr_2, device_result;
	long long total = 0, serial, mem_total, total_mem;	
	
	if(MEASURE_TYPE == 1) {
		long long counter_b_mem = rdtsc();		
		// Creating device arrays -- 2D matrices, and copying the host values
		device_arr_1 = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), M*N*sizeof(float), host_data_1, &err);
		ERROR_CHK(err);
	
		device_arr_2 = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), N*R*sizeof(float), host_data_2, &err);
		ERROR_CHK(err);
	
		// Allocate OpenCL empty result array on the device
		device_result = clCreateBuffer(context, (cl_mem_flags)(DEST_FLAG), M*R*sizeof(float), compute_device, &err);
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
		// Warming Up the device
		runKernel(WorkQueue, context, device_id, Kernel, device_arr_1, device_arr_2, device_result, M , N, R);
		clFinish(WorkQueue);	
		
		counter_b = rdtsc();	
		for(int i=0; i< NUM ; i++){
			// Ready for starting the computation
			runKernel(WorkQueue, context, device_id, Kernel, device_arr_1, device_arr_2, device_result, M , N, R);
			clFinish(WorkQueue);
			}
		err = clReleaseKernel(Kernel);
		ERROR_CHK(err);
		
		counter_a = rdtsc();
		total = counter_a - counter_b;		
		
		long long mem_count_a, mem_count_b;
		mem_count_a = rdtsc();		
		// Copying the results to the host memory
		compute_device = 	(float*)clEnqueueMapBuffer(WorkQueue, device_result, 
								   CL_TRUE, CL_MAP_READ, 0, M*R*sizeof(float), 0, NULL, NULL, &err);	
		mem_count_b = rdtsc();
		mem_total = mem_count_b -mem_count_a;
		}
	
	// checking the results
	printf("running HOST MatrixMul calculation.\n");
	runSerail_MUL(host_result, host_data_1, host_data_2, M, N, R);
	
	if(MEASURE_TYPE == 0) {
		int i;
		// Warming UP
		runSerail_MUL(host_result, host_data_1, host_data_2, M, N, R);
		
		int NUM = ITERATIONS;
		long long serial_a, serial_b;
		serial_a = rdtsc();		
		for(i =0; i< NUM; i++) {
			// running serail computation on the host (just for checking the results)
			runSerail_MUL(host_result, host_data_1, host_data_2, M, N, R);
			}
		serial_b = rdtsc();
		serial = (serial_b - serial_a)/NUM;		
		}
	
	
	printf("Results : \n");
	if(MEASURE_TYPE == 0) printf("CPU Serial Compute: %llu\n",serial);
	else printf("Device OpenCL: Compute %llu, Mem-Dev %llu, Dev-Mem %llu\n",total/NUM, total_mem,mem_total);
	
#if ALLOW_DBG_
	printf("Matrix A[M X N] : \n");
	for(int i=0; i< M; i++) { 
		for(int j=0; j< N; j++)
			printf(" %3.0f ",host_data_1[i*N+j]);
		printf("\n");
	}
	
	printf("Matrix B[N X R] : \n");
	for(int i=0; i< N; i++) { 
		for(int j=0; j< R; j++)
			printf(" %3.0f ",host_data_2[i*R+j]);
		printf("\n");
	}
	
	printf("Host Res[M X R] : \n");
	for(int i=0; i< M; i++) { 
		for(int j=0; j< R; j++)
			printf(" %3.0f ",host_result[i*R+j]);
		printf("\n");
	}
	
	printf("Dev. Res[M X R] : \n");
	for(int i=0; i< M; i++) { 
		for(int j=0; j< R; j++)
			printf(" %3.0f ",compute_device[i*R+j]);
		printf("\n");
	}
#endif
	
	if(MEASURE_TYPE == 1) {
		// checking the results
		float Threshold = 1e-6f;
		printf("Checking the results ... ");
		int cnt = 0;
		for(int i = 0; i< M*R; ++i) 
			if (abs(host_result[i] - compute_device[i]) < Threshold) cnt++;
	
		if(cnt != M*R) printf("Failure\n %d from %d",cnt, M*R);
		else	printf("Success, %d passed from %d\n",cnt,M*R);

		// Realesing the resources
		clReleaseMemObject(device_arr_1);
		clReleaseMemObject(device_arr_2);
		clReleaseMemObject(device_result);
		}
	
	printf("Press Any Key to Exit ...");
	ch=getchar();	
	
	free(host_data_1);
	free(host_data_2);
	free(host_result);
	//free(compute_device);
	
	clReleaseContext(context);
    return 0;
}
