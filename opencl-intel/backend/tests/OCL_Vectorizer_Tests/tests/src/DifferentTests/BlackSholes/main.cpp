// *******************************************************************************
// Black-Scholes Example
// This sample evaluates fair call and put prices for a
// given set of European options by Black-Scholes formula.
//		Author: Mohammed Agabaria
//		Updates : 01/11/08	Realese
//				  17/12/08  Time Ext. Added
//				  14/01/09	Ported to OCL 1.0
// *******************************************************************************

// *******************************************************************************
// Include Headers
// *******************************************************************************
// Include OpenCL API Libraries
#include "opencl.h"

// for serial calculation
#include "serial.h"

// Some general libraries
#include <math.h>
#include <fstream>
#include <assert.h>
#include <iostream>
#include <cstdlib>

// *******************************************************************************
// Constants & Definitions
// *******************************************************************************

using namespace std;
#define MAX_BUF 64000

// for debug mode, it will proview some extra info.
#define ALLOW_DBG_ 0

// what device you want to do the kernel on
#define CPU_VER 1
#define GPU_VER !CPU_VER

// vector kernel version for the CPU Device.
#define Vector_VER CPU_VER

// what's the measure type you want, 0-Serial, Or 1-OpenCL Kernel
#define MEASURE_TYPE 1

// how many iterations
#define ITERATIONS	128

#define _View_DT	0	// for viewing details (L1, max abs. error)

// The Path for the vector verssion of the kernel file.
// OPTIMAL FOR CPU
#if (Vector_VER == 1)
#define _PATH "/Users/magabari/Documents/NewVersion/Global/BlackScholes/kernel_vec"
#endif

// OPTIMAL FOR GPU
#if (Vector_VER == 0)
#define _PATH "/Users/magabari/Documents/NewVersion/Global/BlackScholes/kernel"
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
// the size of the supported vector in the CPU, float.
const int	VEC_SZ = 4;

// assumpation that OPT_N is multiply of vector size in the vector version
const int OPT_N = 1000000 * VEC_SZ;

const int          OPT_SZ = OPT_N * sizeof(float);

// RISKFREE and VOLATILITY parameters
const float      RISKFREE = 0.02f;
const float    VOLATILITY = 0.30f;

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
// Helper function, returning uniformly distributed
// random float in [low, high] range
// *******************************************************************************
float RandFloat(float low, float high)
{
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
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
	cl_int err;
	cl_program prog = clCreateProgramWithSource(context, 1, (const char**)&clCodeStr, NULL, &err);
	ERROR_CHK(err);
	
	delete clCodeStr;
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
void runKernel(cl_command_queue clWorkQueue, cl_device_id deviceId, cl_context context,
			cl_kernel Kernel, cl_mem arr1, cl_mem arr2, cl_mem arr3, 
			cl_mem arr4, cl_mem arr5)
{
	// printf("Setting and Executing the kernel ... ");
#if (Vector_VER != 0)
	#define VECTOR_SIZE 4
#endif
	
#if (Vector_VER == 0)
	#define VECTOR_SIZE 1 // not vector 
#endif
	
	// Execute kernel
	size_t globalSize = OPT_N / VECTOR_SIZE;
	
	unsigned int maxThreadGroupSize = 0;
	clGetDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, 
					sizeof(unsigned int), &maxThreadGroupSize,  NULL);
	size_t localSize = std::min(maxThreadGroupSize, (unsigned int)globalSize);
	
	if(localSize > 256) localSize = 256;	//GPU DEVICE Will handle 256 packets on every local pool
	if(localSize > 64 && Vector_VER == 1) localSize = 64; // relating to GPU DEV with vectors (not dealed)
	
	// adjusting the global size to divide the local size of threads
	if(globalSize % localSize != 0) globalSize = globalSize + (localSize - globalSize % localSize);	
	
	// Creating arg array
	float arg_h[4]= {RISKFREE, VOLATILITY, OPT_N, globalSize};
	
	cl_int err;
	cl_mem arg_d  = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), 4*sizeof(float), arg_h, &err);	
	ERROR_CHK(err);
	
	// Set args to kernel
	void *args[] = {arr1, arr2, arr3, arr4, arr5, arg_d};
	size_t sizes[] = {sizeof(cl_mem), sizeof(cl_mem), sizeof(cl_mem), sizeof(cl_mem)
						, sizeof(cl_mem), sizeof(cl_mem) };
	
	for(int i=0; i<6; ++i) clSetKernelArg(Kernel, i, sizes[i], &args[i]);

#if ALLOW_DBG_	
	printf("Total Threads : %d, Local Net: %d\n",globalSize,localSize);
#endif
	
	// simple addresing of the threads (simple version) 1-dim
	err = clEnqueueNDRangeKernel(clWorkQueue, Kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Failure\n");
		exit(0);
		}
	// printf("Success\n");
}
// *******************************************************************************
// Main Program
// *******************************************************************************
int main (int argc, char * const argv[]) 
{
	cl_int err;
	
	printf("[Black-Scholes Example] \n");
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
	cl_kernel Kernel = clCreateKernel(prog, "BlackScholes", &err);
	if (!Kernel) {
		printf("Failure\n");
		return 0;
		}	
	printf("Success\n");

	// DEVICE & HOST Array's
	//'h_' prefix - (host) memory space
    float
		//Results calculated by HOST for reference
		*h_CallResultHOST,
		*h_PutResultHOST,
		//HOST copy of DEVICE results
		*h_CallResultDEVICE,
		*h_PutResultDEVICE,
		//HOST instance of input data
		*h_StockPrice,
		*h_OptionStrike,
		*h_OptionYears;	

	//'d_' prefix - (device) memory space
    cl_mem
		//Results calculated by DEVICE
		d_CallResult,
		d_PutResult,
		//DEVICE instance of input data
		d_StockPrice,
		d_OptionStrike,
		d_OptionYears;
	
	double
	delta, ref, sum_delta, sum_ref, max_delta, L1norm;
	int i;
	
	printf("\tInitializing data...\n");
	printf("\t\t...allocating HOST memory for options.\n");
	h_CallResultHOST	= (float *)malloc(OPT_SZ);
	h_PutResultHOST		= (float *)malloc(OPT_SZ);
	h_CallResultDEVICE	= (float *)malloc(OPT_SZ);
	h_PutResultDEVICE	= (float *)malloc(OPT_SZ);
	h_StockPrice		= (float *)malloc(OPT_SZ);
	h_OptionStrike		= (float *)malloc(OPT_SZ);
	h_OptionYears		= (float *)malloc(OPT_SZ);	
	
	printf("\t\t...generating input data in HOST mem.\n");
	srand(5347);
	//Generate options set
	for(i = 0; i < OPT_N; i++){
		h_CallResultHOST[i] = 0.0f;
		h_PutResultHOST[i]  = -1.0f;
		h_StockPrice[i]    = RandFloat(5.0f, 30.0f);
		h_OptionStrike[i]  = RandFloat(1.0f, 100.0f);
		h_OptionYears[i]   = RandFloat(0.25f, 10.0f);
		}
	
	long long total = 0, serial, mem_total, total_mem;	
	
	if(MEASURE_TYPE == 1) {
		printf("\t\t...copying input data to DEVICE mem.\n");
		long long counter_b_mem = rdtsc();		
		
		// Creating device arrays
		d_StockPrice = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), OPT_N*sizeof(float), h_StockPrice, &err);
		ERROR_CHK(err);
		
		d_OptionStrike = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), OPT_N*sizeof(float), h_OptionStrike, &err);
		ERROR_CHK(err);
	
		d_OptionYears = clCreateBuffer(context, (cl_mem_flags)(SRC_FLAG), OPT_N*sizeof(float), h_OptionYears, &err);	
		ERROR_CHK(err);
		
		// Allocate OpenCL empty result arrays on the device
		d_CallResult = clCreateBuffer(context, (cl_mem_flags)(DEST_FLAG), OPT_N*sizeof(float), h_CallResultDEVICE, &err);
		ERROR_CHK(err);
		
		d_PutResult  = clCreateBuffer(context, (cl_mem_flags)(DEST_FLAG), OPT_N*sizeof(float), h_PutResultDEVICE, &err);	
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
		runKernel(WorkQueue, device_id, context, Kernel, d_CallResult, d_PutResult, d_OptionStrike,d_StockPrice, d_OptionYears);
		clFinish(WorkQueue);	
		
		counter_b = rdtsc();	
		for(int i=0; i< NUM ; i++){
			
			// Ready for starting the computation
			runKernel(WorkQueue, device_id, context, Kernel, d_CallResult, d_PutResult, d_OptionStrike,d_StockPrice, d_OptionYears);
			clFinish(WorkQueue);
			}
		err = clReleaseKernel(Kernel);
		ERROR_CHK(err);
		
		counter_a = rdtsc();
		total = counter_a - counter_b;		
	
		long long mem_count_a, mem_count_b;
		mem_count_a = rdtsc();		
		
		// Copying the results to the host memory
		h_CallResultDEVICE= (float*)clEnqueueMapBuffer(WorkQueue, d_CallResult, 
								CL_TRUE, CL_MAP_READ, 0, OPT_N*sizeof(float), 0, NULL, NULL, &err);
		ERROR_CHK(err);			
		
		h_PutResultDEVICE= (float*)clEnqueueMapBuffer(WorkQueue, d_PutResult, 
								CL_TRUE, CL_MAP_READ, 0, OPT_N*sizeof(float), 0, NULL, NULL, &err);
		ERROR_CHK(err);		
		
		mem_count_b = rdtsc();
		mem_total = mem_count_b -mem_count_a;
		}
	
	// checking the results
	printf("running HOST BlackSholes calculation.\n");	
	BlackScholesCPU(
					h_CallResultHOST,
					h_PutResultHOST,
					h_OptionStrike,
					h_StockPrice,
					h_OptionYears,
					RISKFREE,
					VOLATILITY,
					OPT_N
					);	
	
	if(MEASURE_TYPE == 0) {
		//Calculate options values on CPU - Serial 
		// Warming Up
		BlackScholesCPU(
					h_CallResultHOST,
					h_PutResultHOST,
					h_OptionStrike,
					h_StockPrice,
					h_OptionYears,
					RISKFREE,
					VOLATILITY,
					OPT_N
					);
		
		long long serial_a, serial_b;
		serial_a = rdtsc();
		for(i =0; i< NUM; i++) {
			// running serail computation on the host (just for checking the results)
			BlackScholesCPU(
							h_CallResultHOST,
							h_PutResultHOST,
							h_OptionStrike,
							h_StockPrice,
							h_OptionYears,
							RISKFREE,
							VOLATILITY,
							OPT_N
							);
		}
		serial_b = rdtsc();
		serial = (serial_b - serial_a)/NUM;		
	}
	
	printf("Results : \n");
	if(MEASURE_TYPE == 0) printf("CPU Serial Compute: %llu\n",serial);
	else printf("Device OpenCL: Compute %llu, Mem-Dev %llu, Dev-Mem %llu\n",total/NUM, total_mem,mem_total);
	
#if ALLOW_DBG_
	printf("Presenting the HOST/DEVICE Data :\n");
	printf("	ID	CallPrice	\t\tPutPrice	OptionStrike	StockPrice\t\tYears\t	RISK	VOLATILITY\n");
	
	for(int i=0; i< OPT_N; i++) 
	printf("\t%d\t%4.2f/%4.2f\t\t\t%4.2f/%4.2f\t\t%4.2f\t\t\t%4.2f\t\t%.2f\t\t%.2f\t\t%.2f\n"
		   ,i,h_CallResultHOST[i], h_CallResultDEVICE[i], h_PutResultHOST[i],
		   h_PutResultDEVICE[i], h_OptionStrike[i], h_StockPrice[i], h_OptionYears[i],
		   RISKFREE, VOLATILITY);		   
	printf("\n");
#endif
	
	if(MEASURE_TYPE == 1) {
		printf("Comparing the results... ");
		//Calculate max absolute difference and L1 distance
		//between HOST (Serial) and DEVICE results
		sum_delta = 0;
		sum_ref   = 0;
		max_delta = 0;
		for(i = 0; i < OPT_N; i++){
			ref   = h_CallResultHOST[i];
			delta = fabs(h_CallResultHOST[i] - h_CallResultDEVICE[i]);
			if(delta > max_delta) max_delta = delta;
			sum_delta += delta;
			sum_ref   += fabs(ref);
		}
		L1norm = sum_delta / sum_ref;
	
		#if _View_DT
		printf("\nL1 norm: %E\n", L1norm);
		printf("Max absolute error: %E\n", max_delta);
		#endif
	
		printf((L1norm < 1e-7) ? "Success\n" : "Failure\n");
	}
	
	// Realesing the resources (HOST)
	free(h_OptionYears);
	free(h_OptionStrike);
	free(h_StockPrice);
	//free(h_PutResultDEVICE);
	//free(h_CallResultDEVICE);
	free(h_PutResultHOST);
	free(h_CallResultHOST);	
	
	if(MEASURE_TYPE == 1) {
		// Realesing the resources (DEVICE)
		clReleaseMemObject(d_OptionYears);
		clReleaseMemObject(d_OptionStrike);
		clReleaseMemObject(d_StockPrice);
		clReleaseMemObject(d_PutResult);
		clReleaseMemObject(d_CallResult);
		}
	
	clReleaseContext(context);
    return 0;
}
