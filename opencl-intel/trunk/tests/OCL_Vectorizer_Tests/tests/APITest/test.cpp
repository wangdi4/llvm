#include <libc.h>
#include <stdbool.h>
#include <OpenCL/opencl.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>

using namespace std;


#define DIMENSION_SIZE 1024
#define DATA_SET_SIZE DIMENSION_SIZE * DIMENSION_SIZE

int              err;
cl_device_id	 device_id;
cl_context       context;
cl_command_queue queue;
cl_program       program;
cl_kernel        kernel;
cl_mem           dst, counter;
int *			 dump_buffer;
size_t           groupsize;
size_t           global[2], local[2];
int				 magicNumber = 0xaaa;

const char * testKernel="   \
	__kernel void program(__global int4 * out, __global int * count, int magic_num)\
	{\
		int tid_0 = get_global_id(0);\
		int tid_1 = get_global_id(1);\
		int global_x = get_global_size(0);\
		int index = tid_1 * global_x + tid_0;\
		int grp_0 = get_local_size(0);\
		int grp_1 = get_local_size(1);\
		atomic_inc(count);\
		int4 outputVal = (int4)(magic_num, 0, grp_0, grp_1);\
		out[index] = outputVal;\
	}";

void clearBuffer()
{
	for (unsigned i = 0; i < 4 * DATA_SET_SIZE * 2; i++)
	{
		dump_buffer[i] = 0;
	}
}

int countEntriesInBuffer(int* buf, int magic_num)
{
	int allCount = 0;
	for (unsigned i = 0; i < DATA_SET_SIZE * 4 * 2; i+=4)
	{
		if (dump_buffer[i] == magic_num) allCount++;
	}
	return allCount;
}


int runTest(int global_0, int global_1, bool hasLocal, int local_0, int local_1, bool shouldPass, int expected_grp_0)
{
	int retVal = 0;
	// --------- Start test --------
	printf("\nNext Test:\n");
	printf("\tInputs: Global size is %d X %d ", global_0, global_1);
	if (!hasLocal)
	{
		printf("  Local size is NULL\n");
	}
	else
	{
		printf("  Local size is %d X %d\n", local_0, local_1);
	}
	if (shouldPass)
	{
		printf("\tExpected result: Group size %d X 1 (executing %s kernel). Expected work items:%d\n", expected_grp_0, expected_grp_0 > 1? "vectorized" : "scalar", global_0*global_1);
	}
	else
	{
		printf("\tExpected result: Error (CL_INVALID_WORK_GROUP_SIZE)\n");
	}
	
	// Clear buffers
	int counter_val = 0;
	err = clEnqueueWriteBuffer(queue, counter, CL_TRUE, 0, sizeof(int), &counter_val, 0, NULL, NULL); 
	if(err)
	{
		printf("clEnqueueWriteBuffer() failed.\n");
		return -1;
	}
	clearBuffer();
	err = clEnqueueWriteBuffer(queue, dst, CL_TRUE, 0, DATA_SET_SIZE * sizeof(cl_int4) * 2, dump_buffer, 0, NULL, NULL); 
	if(err)
	{
		printf("clEnqueueWriteBuffer() failed.\n");
		return -1;
	}

	// Set magic value
	magicNumber += 0xabc;
	err = clSetKernelArg(kernel, 2, sizeof(int), &magicNumber);
	if (err)
	{
		printf("clSetKernelArg() failed (%d).\n", err);
		return -1;
	}
	

	global[0] = global_0;
	global[1] = global_1;
	local[0] = local_0;
	local[1] = local_1;
	if (hasLocal)
	{
		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
	}
	else
	{
		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);
	}
	clFinish(queue);
	if (!shouldPass)
	{
		// Test was expected to fail. Did it?
		if(err == CL_INVALID_WORK_GROUP_SIZE)
		{
			printf("Test Passed!\n");
			return 0;
		}	
		else if (err)
		{
			printf("Test failed: Unexpected error code (%d)\n", err);
			return -1;
		}
		else
		{
			printf("Failure: Execution apparently succeeded somehow... \n");
			err = clEnqueueReadBuffer(queue, dst, CL_TRUE, 0, global[0] * global[1] * sizeof(cl_int4), dump_buffer, 0, NULL, NULL); 
			err = clEnqueueReadBuffer(queue, counter, CL_TRUE, 0, sizeof(int), &counter_val, 0, NULL, NULL); 
			printf("\tActual execution values: \n\t\tGroup size: %d X %d\n", dump_buffer[2],dump_buffer[3]);		
			int magic_count = countEntriesInBuffer(dump_buffer, magicNumber);
			printf("\tExecuted threads: \n\t\tAtomic increment:%d    \n\t\tModified buffer lines:%d\n", counter_val, magic_count);
			return -1;
		}
	}
	// Getting here, test should have passed
	if(err)
	{
		printf("clEnqueueNDRangeKernel() failed (%d).\n", err);
		return -1;
	}			
	
	// Read all outputs
	err = clEnqueueReadBuffer(queue, dst, CL_TRUE, 0, DATA_SET_SIZE * sizeof(cl_int4) * 2, dump_buffer, 0, NULL, NULL); 
	if(err)
	{
		printf("clEnqueueReadBuffer() failed.\n");
		return -1;
	}
	err = clEnqueueReadBuffer(queue, counter, CL_TRUE, 0, sizeof(int), &counter_val, 0, NULL, NULL); 
	if(err)
	{
		printf("clEnqueueReadBuffer() failed.\n");
		return -1;
	}
	int magic_count = countEntriesInBuffer(dump_buffer, magicNumber);
	
	// Check results	
	printf("\tActual results:");
	printf("\n\t\tGroup size: %d X %d", dump_buffer[2],dump_buffer[3]);
	printf("\n\t\tWork items (counted by Atomic increment):%d",counter_val);
	printf("\n\t\tWork items (counted Modified buffer lines):%d\n",magic_count);
	if (dump_buffer[2] != expected_grp_0)
	{
		printf("Failure: Actual group size is incorrect\n");
		retVal = -1;
	}
	if (dump_buffer[3] != 1)
	{
		printf("Failure: Group size in Y-dimension is not 1!\n");
		retVal = -1;
	}
	if (counter_val != global_0*global_1 || magic_count != counter_val)
	{
		printf("Failure: Work-items counts are incorrect! Ususally this means executing scalar kernel with vector group-size of vice-versa!\n");
		retVal = -1;
	}
	if (retVal == 0)
	{
		printf("Test Passed!\n");
	}
	return retVal;
}



int main()
{
	int retVal = 0;

	printf("\nTesting the Runtime-vectorizer interface...\n");
	
	// Initialize OpenCL stuff
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
	if(err)
	{
		printf("clGetComputeDevices() failed.\n");
		return -1;
	}
	
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
	if(!context || err)
	{
		printf("clCreateContext() failed.\n");
		return -1;
	}
	
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (!queue || err)
	{
		printf("clCreateCommandQueue() failed: %d\n", err);
		return -1;
	}	
	
	program = clCreateProgramWithSource(context, 1, (const char**)&testKernel, NULL, &err);
	if (!program || err)
	{
		printf("clCreateProgramWithSource() failed.\n");
		return -1;
	}

	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if(err != CL_SUCCESS)
	{
		char buildLog[ 1024 * 16 ];
		buildLog[0] = 0;
		
		clGetProgramBuildInfo( program, device_id, CL_PROGRAM_BUILD_LOG, sizeof( buildLog ), buildLog, NULL );
		printf("clBuildProgram() failed:%s\n", buildLog);
		return -1;
	}
	
	kernel = clCreateKernel(program, "program", &err);
	if(!kernel)
	{
		printf("clCreateKernel() failed creating a kernel. %d\n", err);
		return -1;
	}
	
	// Check that supported work-group size is bigger than 4 
	err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(groupsize), &groupsize, NULL);
	if(err)
	{
		printf("clGetKernelWorkGroupInfo() failed (%d).\n", err);
		return -1;
	}	
	
	printf("Maximum supported Kernel WorkGroup size is: %d\n", (int)groupsize);

 	if (groupsize == 1)
	{
		printf("Error: Work group size 1 probably means vectorizer is not initiated!\n");
		return -1;
	}
	if (groupsize < 16)
	{
		printf("Error: Vectorizer must provide work group size >= 16 for this test to work!\n");
		return -1;
	}
	
	printf("-----------------\n\n");	
	
	
	// Prep buffers
	dst = clCreateBuffer(context, CL_MEM_READ_WRITE, DATA_SET_SIZE * sizeof(cl_int4) * 2, NULL, &err);
	if (err)
	{
		printf("clCreateBuffer() failed (%d).\n", err);
		return -1;
	}
	counter = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
	if (err)
	{
		printf("clCreateBuffer() failed (%d).\n", err);
		return -1;
	}
	
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dst);
	if (err)
	{
		printf("clSetKernelArg() failed (%d).\n", err);
		return -1;
	}
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &counter);
	if (err)
	{
		printf("clSetKernelArg() failed (%d).\n", err);
		return -1;
	}
	
	dump_buffer = (int *)malloc(sizeof(cl_int4) * DATA_SET_SIZE * 2);
	if(!dump_buffer)
	{
		printf("malloc failed.\n");
		return -1;
	}			

	
	/* Run the actual test */

	/* Test #1:
	 Local size is NULL
	 Vectorized max_width fits inside global size. 
     Vectorized code is expected to run, with max_width work group size 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, false, 0, 0, true, (int)groupsize);
	
	/* Test #2:
	 Local size is NULL
	 Vectorized max_width Doesnt fit inside global size. However, min_width does fit. 
	 Vectorized code is expected to run, with min_width work group size 
	 */
	retVal += runTest(4, DIMENSION_SIZE, false, 0, 0, true, 4);

	/* Test #3:
	 Local size is NULL
	 Vectorized max_width Doesnt fit inside global size. However, min_width*Const does fit. 
	 Vectorized code is expected to run, with min_width*Const work group size 
	 */
	retVal += runTest((int)groupsize/2, DIMENSION_SIZE, false, 0, 0, true, (int)groupsize/2);

	/* Test #4:
	 Local size is NULL
	 Global size (dimension 0) doesnt divide by min_width.  
	 Scalar code is expected to run (work group size 1x1). 
	 */
	retVal += runTest((int)groupsize-1, (int)groupsize, false, 0, 0, true, 1);
	
	/* Test #5:
	 Local size is NULL
	 Vectorized width Doesnt fit inside global size.  
	 Scalar code is expected to run (work group size 1x1). 
	 */
	retVal += runTest(2, 1, false, 0, 0, true, 1);
	
	/* Test #6:
	 Local size is max_width
	 Global size divides by max_width. 
     Vectorized code is expected to run, with max_width work group size 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, (int)groupsize, 1, true, (int)groupsize);

	/* Test #7:
	 Local size is min_width
	 Global size divides by min_width. 
     Vectorized code is expected to run, with min_width work group size 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, 4, 1, true, 4);
	
	/* Test #8:
	 Local size is min_width*Const
	 Global size divides by min_width*Const. 
     Vectorized code is expected to run, with min_width*Const work group size 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, (int)groupsize/4, 1, true, (int)groupsize/4);

	/* Test #9:
	 Local size is 1
	 Scalar code is expected to run (work group size 1x1). 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, 1, 1, true, 1);

	/* Test #10:
	 Local size has dimension(1) = 4
	 Kernel is expected to fail. 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, (int)groupsize/4, 4, false, 0);

	/* Test #11:
	 Local size is smaller than min_width (but bigger than 1)
	 Kernel is expected to fail. 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, 2, 1, false, 0);

	/* Test #12:
	 Local size doesnt divide by min_width
	 Kernel is expected to fail. 
	 */
	retVal += runTest(DIMENSION_SIZE, DIMENSION_SIZE, true, (int)groupsize-1, 1, false, 0);
	

	
	
	// Summary report
	
	if (retVal == 0)
	{
		printf("\n\nAll tests Passed!\n");
	}
	else
	{
		printf("\n\n%d tests Failed!\n", retVal * -1);
	}
	return retVal;
}





