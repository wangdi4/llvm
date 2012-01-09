#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "FrameworkTest.h"
#include "cl_objects_map.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/**************************************************************************************************
* clKernelAttributes
* -------------------
* (1) get device ids
* (2) create context
* (3) create binary
* (4) create program with source
* (5) build program
**************************************************************************************************/

#define BUFFERS_LENGTH 4

bool clKernelBarrierTest()
{
	printf("---------------------------------------\n");
	printf("clKernelBarrierTest\n");
	printf("---------------------------------------\n");

	const char *ocl_test_program[] = {\
	"__kernel void barrier_test(__global int *res, __local int* tmp2)"\
	"{"\
	"__local int tmp[10][12];"\
	"int tid = get_global_id(0);"\
	"tmp2[0] = tid;"\
	"tmp[4][5] = tmp2[0];"\
	"barrier(CLK_LOCAL_MEM_FENCE);"\
	"res[tid] = tmp[4][5];"\
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
	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &uiNumDevices);
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);
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
	printf("context = %p\n", context);


	// create program with source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRet);
	bResult &= Check(L"clCreateProgramWithSource", CL_SUCCESS, iRet);

	iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, NULL, NULL);
	bResult &= Check(L"clBuildProgram", CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

    //
    // From here down it is the program execution implementation
    //
    cl_int dst[BUFFERS_LENGTH];

    for(int j = 0; j < BUFFERS_LENGTH; j++)
    {
        dst[j] = -1;
    }

    //
    // Create queue
    //
    cl_command_queue queue1 = clCreateCommandQueue (context, pDevices[0], 0 /*no properties*/, &iRet);
	bResult &= Check(L"clCreateCommandQueue - queue1", CL_SUCCESS, iRet);

    //
    // Create Kernel
    //
    cl_kernel kernel1 = clCreateKernel(program, "barrier_test", &iRet);
	bResult &= Check(L"clCreateKernel - barrier_test", CL_SUCCESS, iRet);

    //
    // Create buffers
    //
    size_t size = sizeof(cl_int);

    cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE, size * BUFFERS_LENGTH, NULL, &iRet);
    bResult &= Check(L"clCreateBuffer - dst", CL_SUCCESS, iRet);

    //
    // Set arguments
    //
    iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_dst);
    bResult &= Check(L"clSetKernelArg - dest", CL_SUCCESS, iRet);
    iRet = clSetKernelArg(kernel1, 1, sizeof(cl_int), NULL);
    bResult &= Check(L"clSetKernelArg - tmp(local)", CL_SUCCESS, iRet);

	//
    // Execute kernel
    //
    size_t global_work_size[1] = { BUFFERS_LENGTH };
    size_t local_work_size[1] = { BUFFERS_LENGTH };

    iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    bResult &= Check(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);    

    //
    // Read results. wait for completion - blocking!
    //
    iRet = clEnqueueReadBuffer (queue1, buffer_dst, CL_TRUE,  0, size*BUFFERS_LENGTH, dst, 0, NULL, NULL);
    bResult &= Check(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);    

    //
    // Print kernel output
    //
    printf("\n ==== \n");
    for (int i=1; i<BUFFERS_LENGTH; i++)
    {
        printf("%d, ", dst[i]);
		bResult = bResult && (dst[0] == dst[i]) && (-1 != dst[i]);
    }
    printf("\n ==== \n");

	//
    // Release objects
    //
    iRet = clReleaseMemObject(buffer_dst);
    bResult &= Check(L"clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

    iRet = clReleaseKernel(kernel1);
    bResult &= Check(L"clReleaseKernel - kernel1", CL_SUCCESS, iRet);

    iRet = clReleaseProgram(program);
    bResult &= Check(L"clReleaseProgram - program", CL_SUCCESS, iRet);

    iRet = clFinish(queue1);
    bResult &= Check(L"clFinish - queue1", CL_SUCCESS, iRet);

    iRet = clReleaseCommandQueue(queue1);
    bResult &= Check(L"clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

    iRet = clReleaseContext(context);
    bResult &= Check(L"clReleaseContext - context", CL_SUCCESS, iRet);

	delete []pDevices;
    return bResult;
}
