#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#ifdef _WIN32
#include <windows.h>
#endif
 
#define BUFFERS_LENGTH 10000
#define REPEATS 5

extern cl_device_type gDeviceType;

/**************************************************************************************************
* cl_execution_test - Out of order test
* -------------------
* (1) Test out of order execution
*
**************************************************************************************************/


bool clOutOfOrderTest()
{
	printf("---------------------------------------\n");
	printf("clOutOfOrderTest\n");
	printf("---------------------------------------\n");
	const char *ocl_test_program[] = {\
	"__kernel void dot_product (__global const float4 *a, __global const float4 *b, __global float *c)"\
	"{"\
	"int tid = get_global_id(0);"\
	"c[tid] = dot(a[tid], b[tid]);"\
	"}"
	};

    bool bResult = true;

	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	cl_context context;

	// get device(s)
	cl_int iRet = clGetDeviceIDs(NULL, gDeviceType, 0, NULL, &uiNumDevices);
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];
	iRet = clGetDeviceIDs(NULL, gDeviceType, uiNumDevices, pDevices, NULL);
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	// create context
	context = clCreateContext(0, uiNumDevices, pDevices, NULL, NULL, &iRet);
	bResult &= Check(L"clCreateContext",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}
	
	// create program with source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRet);
	bResult &= Check(L"clCreateProgramWithSource", CL_SUCCESS, iRet);

	iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, NULL, NULL);
	bResult &= Check(L"clBuildProgram", CL_SUCCESS, iRet);


    //
    // From here down it is the program execution implementation
    //
    printf("\n************************\n Out Of Order Test Started \n***************\n");

    int j;
    cl_int dst[BUFFERS_LENGTH];
    for( j = 0; j < BUFFERS_LENGTH; j++)
    {
        dst[j] = 0;
    }
    //
    // Create queue
    //
    cl_command_queue queue1 = clCreateCommandQueue (context, pDevices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &iRet);
	bResult &= Check(L"clCreateCommandQueue - queue1 - Out of order", CL_SUCCESS, iRet);

    //
    // Create buffers
    //
    size_t size = sizeof(cl_int);


    cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - dst", CL_SUCCESS, iRet);


    //
    // Execute commands - Write buffers
    //
	cl_int src[BUFFERS_LENGTH];

	int numIter = BUFFERS_LENGTH/REPEATS;
    for( j =0; j<numIter; j++)
    {     
        for (int i=0; i<REPEATS; i++)
        {
            src[j*REPEATS + i] = j;
        }
	}
    
    for( j =0; j<numIter; j++)
    {               		
        iRet = clEnqueueWriteBuffer (queue1, buffer_dst, false, j*size*REPEATS, size*REPEATS, src + j*REPEATS, 0, NULL, NULL);
        bResult &= Check(L"clEnqueueWriteBuffer - repeat", CL_SUCCESS, iRet);
    }

    iRet = clEnqueueBarrier(queue1);
	bResult &= Check(L"clEnqueueBarrier", CL_SUCCESS, iRet);	

    // Read content and print    
    cl_event readBufferEvent;
    iRet = clEnqueueReadBuffer (queue1, buffer_dst, CL_FALSE, 0, size*BUFFERS_LENGTH, dst, 0, NULL, &readBufferEvent);
    bResult &= Check(L"clEnqueueReadBuffer - dst", CL_SUCCESS, iRet);

    cl_event markerEvent;
    iRet = clEnqueueMarkerWithWaitList(queue1, 1, &readBufferEvent, &markerEvent);
    bResult &= Check(L"clEnqueueMarkerWithWaitList", CL_SUCCESS, iRet);

    iRet = clWaitForEvents(1, &markerEvent);
    bResult &= Check(L"clWaitForEvents", CL_SUCCESS, iRet);

    clReleaseEvent(readBufferEvent);
    clReleaseEvent(markerEvent);

    //Print output
    printf("********** Output: \n");
	bool PASSED = true;
    for( j =0; j<BUFFERS_LENGTH; j++)
    {             
        if (dst[j] != src[j])
		{
			PASSED = false;
		}        
	}
	bResult &= PASSED;
        
    //
    // Release objects
    //
    iRet = clReleaseMemObject(buffer_dst);
    bResult &= Check(L"clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

    iRet = clReleaseProgram(program);
    bResult &= Check(L"clReleaseProgram - program", CL_SUCCESS, iRet);

    iRet = clFinish(queue1);
    bResult &= Check(L"clFinish - queue1", CL_SUCCESS, iRet);

    iRet = clReleaseCommandQueue(queue1);
    bResult &= Check(L"clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

    iRet = clReleaseContext(context);
    bResult &= Check(L"clReleaseContext - context", CL_SUCCESS, iRet);

    return bResult;
}


bool clOODotProductTest( int iNumLoops )
{
	printf("---------------------------------------\n");
    printf("clOODotProductTest (loops: %d) \n", iNumLoops);
	printf("---------------------------------------\n");
	const char *ocl_test_program[] = {\
	"__kernel void dot_product (__global const float4 *a, __global const float4 *b, __global float *c)"\
	"{"\
	"int tid = get_global_id(0);"\
	"c[tid] = dot(a[tid], b[tid]);"\
	"}"
	};

	bool bResult = true;

	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	cl_context context;

	cl_platform_id platform = 0;

	cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	// get device(s)
	iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];
	iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	bResult &= Check(L"clCreateContext",CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	// create program with source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRet);
	bResult &= Check(L"clCreateProgramWithSource", CL_SUCCESS, iRet);

	iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, NULL, NULL);
	bResult &= Check(L"clBuildProgram", CL_SUCCESS, iRet);

    //
    // From here down it is the program execution implementation
    //
    cl_float* srcA = new cl_float[BUFFERS_LENGTH]; 
    cl_float* srcB = new cl_float[BUFFERS_LENGTH]; 
    cl_float* dst  = new cl_float[BUFFERS_LENGTH];

    for(int j = 0; j < BUFFERS_LENGTH; j++)
    {
        srcA[j] = (cl_float)j;
        srcB[j] = 1;
        dst[j] = 0;
    }

    //
    // Create Kernel
    //
    cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
	bResult &= Check(L"clCreateKernel - dot_product", CL_SUCCESS, iRet);

    //
    // Create buffers
    //
    size_t size = sizeof(cl_float);

    cl_mem buffer_srcA = clCreateBuffer(context, CL_MEM_READ_ONLY, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - srcA", CL_SUCCESS, iRet);

    cl_mem buffer_srcB = clCreateBuffer(context, CL_MEM_READ_ONLY, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - srcB", CL_SUCCESS, iRet);

    cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - dst", CL_SUCCESS, iRet);

    //
    // Set arguments
    //
    iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_srcA);
    bResult &= Check(L"clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);

    iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem), &buffer_srcB);
    bResult &= Check(L"clSetKernelArg - buffer_srcB", CL_SUCCESS, iRet);

    iRet = clSetKernelArg(kernel1, 2, sizeof(cl_mem), &buffer_dst);
    bResult &= Check(L"clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);

    for( int i=0; i< iNumLoops; i++)
    {

        // Create queue
        //
        cl_command_queue queue1 = clCreateCommandQueue (context, pDevices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &iRet);
	    bResult &= Check(L"clCreateCommandQueue - queue1", CL_SUCCESS, iRet);

        //
        // Execute commands - Write buffers
        //
        iRet = clEnqueueWriteBuffer (queue1, buffer_srcA, false, 0, size* BUFFERS_LENGTH, srcA, 0, NULL, NULL);
        bResult &= Check(L"clEnqueueWriteBuffer - srcA", CL_SUCCESS, iRet);

        iRet = clEnqueueWriteBuffer (queue1, buffer_srcB, false, 0, size* BUFFERS_LENGTH, srcB, 0, NULL, NULL);
        bResult &= Check(L"clEnqueueWriteBuffer - srcB", CL_SUCCESS, iRet);
        

        // Enqueue Barrier to continue only when buffers are ready
        iRet = clEnqueueBarrier(queue1);
        bResult &= Check(L"clEnqueueBarrier", CL_SUCCESS, iRet);

        //
        // Execute kernel - dot_product
        //
        size_t global_work_size[1] = { BUFFERS_LENGTH/4 };
        size_t local_work_size[1] = { 1 };

        cl_event waitEvent[2];
        for (int index =0; index < 4; index++)
        {
            iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size, local_work_size, 0, NULL, &waitEvent[0]);
            bResult &= Check(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

            iRet = clEnqueueReadBuffer (queue1, buffer_dst, CL_FALSE,  0, size*BUFFERS_LENGTH, dst, 1, &waitEvent[0], NULL);
            bResult &= Check(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);

            // Wait with Marker...
            iRet = clEnqueueMarker(queue1, &waitEvent[1]);
            bResult &= Check(L"clEnqueueMarker", CL_SUCCESS, iRet);

            // Wait on marker
            iRet = clWaitForEvents(1, &waitEvent[1]);
            bResult &= Check(L"clWaitForEvents", CL_SUCCESS, iRet);


            //
            // Print kernel output
            //
            printf("\n ==== \n");
            for (int i=0; i<10; i++)
            {
                printf("%lf, ", dst[i]);
            }
            printf("\n ==== \n");

            iRet = clReleaseEvent(waitEvent[0]);
            bResult &= Check(L"clReleaseEvent - waitEvent[0]", CL_SUCCESS, iRet);

            iRet = clReleaseEvent(waitEvent[1]);
            bResult &= Check(L"clReleaseEvent - waitEvent[1]", CL_SUCCESS, iRet);
        }
        iRet = clFinish(queue1);
        bResult &= Check(L"clFinish - queue1", CL_SUCCESS, iRet);        

        iRet = clReleaseCommandQueue(queue1);
        bResult &= Check(L"clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);
    }
    //
    // Release objects
    //
    iRet = clReleaseMemObject(buffer_dst);
    bResult &= Check(L"clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

    iRet = clReleaseMemObject(buffer_srcA);
    bResult &= Check(L"clReleaseBuffer - buffer_srcA", CL_SUCCESS, iRet);

    iRet = clReleaseMemObject(buffer_srcB);
    bResult &= Check(L"clReleaseBuffer - buffer_srcB", CL_SUCCESS, iRet);

    iRet = clReleaseKernel(kernel1);
    bResult &= Check(L"clReleaseKernel - kernel1", CL_SUCCESS, iRet);

    iRet = clReleaseProgram(program);
    bResult &= Check(L"clReleaseProgram - program", CL_SUCCESS, iRet);

    iRet = clReleaseContext(context);
    bResult &= Check(L"clReleaseContext - context", CL_SUCCESS, iRet);

    delete[] srcA;
    delete[] srcB;
    delete[] dst;
	delete []pDevices;

    return bResult;
}


bool clQuickExecutionTest()
{
	printf("---------------------------------------\n");
	printf("clQuickExecutionTest\n");
	printf("---------------------------------------\n");
	bool bResult = true;

	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	cl_context context;

	// get device(s)
	cl_int iRet = clGetDeviceIDs(NULL, gDeviceType, 0, NULL, &uiNumDevices);

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];

	iRet = clGetDeviceIDs(NULL, gDeviceType, uiNumDevices, pDevices, NULL);

	// create context
	context = clCreateContext(0, uiNumDevices, pDevices, NULL, NULL, &iRet);
	
    //
    // From here down it is the program execution implementation
    //
    printf("\n************************\n Quick Execution Test Started \n***************\n");

    cl_float* srcA = new cl_float[BUFFERS_LENGTH]; 
    cl_float* srcB = new cl_float[BUFFERS_LENGTH]; 
    cl_float* dst  = new cl_float[BUFFERS_LENGTH];
    int j;

    for( j = 0; j < BUFFERS_LENGTH; j++)
    {
        srcA[j] = (cl_float)j;
        srcB[j] = 1;
        dst[j] = 0;
    }

    //
    // Create queue
    //
    cl_command_queue queue1 = clCreateCommandQueue (context, pDevices[0], CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &iRet);

    //
    // Create buffers
    //
    size_t size = sizeof(cl_float);

    cl_mem buffer_srcA = clCreateBuffer(context, CL_MEM_READ_ONLY, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - srcA", CL_SUCCESS, iRet);

    cl_mem buffer_srcB = clCreateBuffer(context, CL_MEM_READ_ONLY, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - srcB", CL_SUCCESS, iRet);

    cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - dst", CL_SUCCESS, iRet);


    //
    // Execute commands - Write buffers
    //
    int numIter = 100;
    for( j =0; j<numIter; j++)
    {
        iRet = clEnqueueWriteBuffer (queue1, buffer_srcA, false, 0, size*BUFFERS_LENGTH, srcA, 0, NULL, NULL);
        iRet = clEnqueueWriteBuffer (queue1, buffer_srcB, false, 0, size*BUFFERS_LENGTH, srcB, 0, NULL, NULL);
    }

    iRet = clEnqueueBarrier(queue1);
    bResult &= Check(L"clEnqueueBarrier", CL_SUCCESS, iRet);

    cl_event waitEvent;
    iRet = clEnqueueReadBuffer (queue1, buffer_srcB, false, 0, size*BUFFERS_LENGTH, dst, 0, NULL, &waitEvent);
    bResult &= Check(L"clEnqueueReadBuffer - to dst", CL_SUCCESS, iRet);

    iRet = clWaitForEvents(1, &waitEvent);
    bResult &= Check(L"clWaitForEvents", CL_SUCCESS, iRet);
    
    
    //
    // Release objects
    //
    iRet = clReleaseEvent(waitEvent);
    iRet = clReleaseMemObject(buffer_srcA);
    iRet = clReleaseMemObject(buffer_srcB);
    iRet = clReleaseMemObject(buffer_dst);
    iRet = clFinish(queue1);
    iRet = clReleaseCommandQueue(queue1);
    iRet = clReleaseContext(context);
    bResult &= Check(L"clReleaseContext - context", CL_SUCCESS, iRet);

    delete[] srcA;
    delete[] srcB;
    delete[] dst;

    return bResult;
}

