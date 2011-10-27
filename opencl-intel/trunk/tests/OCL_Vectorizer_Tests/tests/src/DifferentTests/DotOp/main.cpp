// *******************************************************************************
// Matrix Dot Example
// This is simple example demonstrates the usage of program and kernel objects
//
//		Author: Mohammed Agabaria
//		Updates: 31/12/08	Ported to OpenCl 1.0
// *******************************************************************************
#include "opencl.h"
#include "serial.h"

#include <fstream>
#include <assert.h>
#include <iostream>
#include <cstdlib>

using namespace std;
#define MAX_BUF 64000
#define ALLOW_DBG_ 0

// Which device you want to work with
#define CPU_VER 1
#define GPU_VER !CPU_VER

// what's the measure type you want, 0-Serial, Or 1-OpenCL Kernel
#define MEASURE_TYPE 1

#define Vector_VER	CPU_VER

// how many iterations
#define ITERATIONS 64


// OPTIMAL FOR CPU
#if (Vector_VER == 1)
#define _PATH "/Users/magabari/Documents/NewVersion/Global/DotOp/kernel_vec4"
#endif

// OPTIMAL FOR GPU
#if (Vector_VER == 0)
#define _PATH "/Users/magabari/Documents/NewVersion/Global/DotOp/kernel"
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
// For measring the time
unsigned long long int rdtsc()
{
	unsigned long long int x;	
	__asm__ volatile (".byte 0x0f, 0x31" : "=A"  (x));
	return x;
}
// *******************************************************************************
// absolute value function 
float abs(float x)
{
	if(x > 0) return x;
	return -x;
}
// *******************************************************************************
// Load OpenCL source into OpenCL driver from file
// - Returns valid (non-NULL) cl_program if succeeded and 0 otherwise
cl_program createProgramFromFile(cl_context Context, const char* fileName)
{
	// Load OpenCL source file
	FILE* in  =fopen(fileName, "r");
	if(in == NULL) { 
		printf("ERROR: FILE CAN'T OPEN");
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
	cl_program prog = clCreateProgramWithSource(Context, 1, (const char**)&clCodeStr, NULL, NULL);
	if (!prog) {
		cerr << "Failed to load source into OpenCL driver\n";
		return 0;
		}
	
	return prog;
}
// *******************************************************************************
// Bind arguments and execute the kernel [ arr3 = arr1 + arr2 ]
// - NOTE: This is hard-coded just to run this kernel
void runKernel(cl_command_queue clWorkQueue, cl_device_id deviceId,
				  cl_kernel Kernel, cl_mem arr1, cl_mem arr2, cl_mem arr3, unsigned int numElmnts)
{
	//printf("Setting and Executing the kernel ... ");
	// Set args to kernel
	void *args[] = {arr1, arr2, arr3};
	size_t sizes[] = {sizeof(cl_mem), sizeof(cl_mem), sizeof(cl_mem)};

	for(int i=0; i<3; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);
	
#if (Vector_VER == 1)
	#define VECTOR_SIZE 4
#endif
	
#if (Vector_VER == 0)
	#define VECTOR_SIZE 1 // no vector 
#endif
	// Execute kernel
	size_t globalSize = numElmnts / VECTOR_SIZE;
	
	unsigned int maxThreadGroupSize = 0;
	clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, 
					sizeof(unsigned int), &maxThreadGroupSize, NULL);
	size_t localSize = (size_t)std::min((unsigned int)maxThreadGroupSize, (unsigned int)globalSize);
	
	if(globalSize % localSize != 0) globalSize = globalSize + (localSize - globalSize % localSize);
	// simple addresing of the threads (simple version) 1-dim
	int clErr = clEnqueueNDRangeKernel(clWorkQueue, Kernel, 1, NULL, 
								&globalSize, &localSize, 0,
								NULL, NULL);
	
	
	if (clErr != CL_SUCCESS) {
		printf("Failure, %d\n", clErr);
		exit(0);
	}
	//printf("Success\n");
}
// *******************************************************************************
int main (int argc, char * const argv[]) {
	cl_int err;	
	
	printf("[Matrix Dot Example] \n");
	cl_device_id device_id;
	
    // Searching for suitable devices , computation units
	if(CPU_VER) clGetDeviceIDs(CL_DEVICE_TYPE_CPU, 1, &device_id, NULL); 
	else if(GPU_VER) clGetDeviceIDs(CL_DEVICE_TYPE_GPU, 1, &device_id, NULL); 
	
	// Creating Context for the device
	cl_context context;
	context = clCreateContext(0, 1, &device_id, NULL, NULL, NULL);	
	printf("Creating Context ... ");
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
	if(prog == 0) {
		printf("Failure\n");
		return 0;
		}
	printf("Success\n");
	
	// Compile and build the source
	printf("Building the Source that has been loaded ... ");
	
	clErr = clBuildProgram(prog, 1, &device_id, NULL, NULL, NULL);
	if (clErr != CL_SUCCESS) {
		cout << endl << "clBuildProgramExecutable returned = " << clErr << endl;
		cout << "buildLog = \n\t" << "ERROR" << endl;
		return 0;
		}
	printf("Success\n");
	
	// Creating the kernel
	printf("Creating and linking the kernel to the program ... ");
	// Obtain handle to executable OpenCL kernel 
	cl_kernel Kernel = clCreateKernel(prog, "process", NULL);
	if (!Kernel) {
		printf("Failure\n");
		return 0;
		}	
	printf("Success\n");
	
	// Allocate host arrays
	const int M = 1 << 22 , N = 1;
	float *host_data_1 , *host_data_2  , *compute_device= NULL;
	float *host_result;
	
	
	host_data_1 = (float*) malloc(M*N * sizeof(float));
	host_data_2 = (float*) malloc(M*N * sizeof(float));
	host_result = (float*) malloc(M*N * sizeof(float));
	//compute_device = (float*) malloc(M*N * sizeof(float));
	
	// putting some values on the data matrices
	srand(0);
	for (int i = 0; i < N*M; ++i) {host_data_1[i] = (float)std::max((double)(i % 10) + rand()%5 , 0.5);}		
	for (int i = 0; i < N*M; ++i) {host_data_2[i] = (float)std::max((double)(i % 10) + rand()%3 , 0.5);}	
	
	long long counter_a_mem, counter_b_mem, total_mem;
	cl_mem	device_arr_1, device_arr_2, device_result;
	if(MEASURE_TYPE) {
		
		counter_b_mem = rdtsc();	
		// Creating device arrays -- M X N matrices, and copying the host values
		device_arr_1 = clCreateBuffer(context,(cl_mem_flags)(SRC_FLAG), M*N*sizeof(float), host_data_1, &err);
		ERROR_CHK(err);
		//float* mem_map_1 = (float*) clEnqueueMapBuffer(WorkQueue, device_arr_1, CL_TRUE, CL_MAP_WRITE, 0, M*N*sizeof(float), 0, NULL, NULL, &err);
		//ERROR_CHK(err);
		//for(int i=0; i< M*N; i++) mem_map_1[i] = 1;
		//clEnqueueUnmapMemObject(WorkQueue, device_arr_1, (void*) mem_map_1, 0, NULL, NULL);		
		
		device_arr_2 = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), M*N*sizeof(float), host_data_2, &err);
		ERROR_CHK(err);		
		//float* mem_map_2 = (float*) clEnqueueMapBuffer(WorkQueue, device_arr_2, CL_TRUE, CL_MAP_WRITE, 0, M*N*sizeof(float), 0, NULL, NULL, &err);
		//ERROR_CHK(err);
		//for(int i=0; i< M*N; i++) mem_map_2[i] = 1;	
		//clEnqueueUnmapMemObject(WorkQueue, device_arr_2, (void*) mem_map_2, 0, NULL, NULL);
		
		// Allocate OpenCL empty result array on the device
		device_result = clCreateBuffer(context,(cl_mem_flags)(CL_MEM_ALLOC_HOST_PTR), sizeof(float)*M*N, device_result, &err);	
		ERROR_CHK(err);		
		
		counter_a_mem = rdtsc();	
		total_mem = counter_a_mem - counter_b_mem;
		}
	printf("Press Any Key to Start ...");
	char ch;
	ch=getchar();
	long long counter_a, counter_b, total =0, mem_total, serial;
	int NUM = ITERATIONS;
	
	if(MEASURE_TYPE) {
		// Warming Up the device
		runKernel(WorkQueue, device_id, Kernel, device_arr_1, device_arr_2
				  , device_result, N * M);
		clFinish(WorkQueue);
		
		counter_b = rdtsc();	
		for(int i=0; i< NUM ; i++){
			// Ready for starting the computation
			runKernel(WorkQueue, device_id, Kernel, device_arr_1, device_arr_2
			  , device_result, N * M);
			clFinish(WorkQueue);
			}
		counter_a = rdtsc();
		total += counter_a - counter_b;
	
		// printf("Time elapsed [avg] : %llu",total/NUM);
	
		// waiting for an input to end the program
		// printf("\nInsert any char to end the execution for the program ... ");
		// ch=getchar();
	
		long long mem_count_a, mem_count_b;
		mem_count_a = rdtsc();	
		// Copying the results to the host memory
		//clEnqueueReadBuffer(WorkQueue, device_result, CL_TRUE, 0, M*N * sizeof(float), 
		//		compute_device, 0, NULL, NULL);
		compute_device = (float*)clEnqueueMapBuffer(WorkQueue, device_result, 
						 CL_TRUE, CL_MAP_READ, 0, M*N*sizeof(float), 0, NULL, NULL, &err);
		ERROR_CHK(err);	
		
		mem_count_b = rdtsc();
		mem_total = mem_count_b -mem_count_a;
	}
	// Just for checking the results
	runSerail_H(host_data_1, host_data_2, host_result, M*N);
	
	if(MEASURE_TYPE == 0) {
		int i;
		// Warming Up
		runSerail_H(host_data_1, host_data_2, host_result, M*N);
		
		long long serial_a, serial_b;
		serial_a = rdtsc();		
		for(i =0; i< NUM; i++) {
			// running serail computation on the host (just for checking the results)
			runSerail_H(host_data_1, host_data_2, host_result, M*N);
			}
		serial_b = rdtsc();
		serial = (serial_b - serial_a)/NUM;
	}
	printf("Results : \n");
	if(MEASURE_TYPE == 0) printf("CPU Serial Compute: %llu\n",serial);
	else printf("Device OpenCL: Compute %llu, Mem-Dev %llu, Dev-Mem %llu\n",total/NUM, total_mem,mem_total);
	
#if ALLOW_DBG_
	printf("Matrix A[] : ");
	for(int i=0; i< M*N; i++) printf(" %.2f ",host_data_1[i]);
	printf("\n");
	
	printf("Matrix B[] : ");
	for(int i=0; i< M*N; i++) printf(" %.2f ",host_data_2[i]);
	printf("\n");
	
	printf("Host Res[] : ");
	for(int i=0; i< M*N; i++) printf(" %.2f ",host_result[i]);
	printf("\n");
	
	printf("Dev. Res[] : ");
	for(int i=0; i< M*N; i++) printf(" %.2f ",compute_device[i]);
	printf("\n");
#endif
	
	// checking the results
	if(MEASURE_TYPE == 1) {
		printf("Checking the results ... ");
		int cnt = 0;
		float max_error = 0;
		for(int i = 0; i< N*M; ++i) { 
			if(max_error < abs(host_result[i] - compute_device[i]) ) 
				max_error = abs(host_result[i] - compute_device[i]);
			if (abs(host_result[i] - compute_device[i]) < 1e-4) {
				cnt++;
			}
		}
		if(cnt != N*M) printf("Failure, %d from %d, Max Error: %.6f\n", cnt, N*M, max_error);
		else	printf("Success\n");
		
	// Realesing the resources
	clReleaseMemObject(device_arr_1);
	clReleaseMemObject(device_arr_2);
	clReleaseMemObject(device_result);
	}
	
	printf("Press Any Key to Exit ...");
	ch=getchar();
	
	free(host_result);
	free(host_data_1);
	free(host_data_2);
	
	clReleaseContext(context);
    return 0;
}
