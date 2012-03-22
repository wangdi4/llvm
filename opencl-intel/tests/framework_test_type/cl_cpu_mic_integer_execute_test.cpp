#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

#define BUFFER_SIZE 128

#define TITLE( prefix ) get_title( title, prefix, sizeof(prefix), dev_name, (4*sizeof(wchar_t)) )
    

inline wchar_t* get_title( wchar_t* buffer, 
                           const wchar_t* prefix, unsigned int psize, 
                           const wchar_t* suffix, unsigned int ssize  )
{
    memcpy( buffer, prefix, psize );    
    memcpy( buffer + (psize-1)/sizeof( wchar_t ), suffix, ssize );
    return buffer;
}

/**************************************************************************************************
 * cl_CPU_MIC_IntegerExecuteTest
 * -------------------
 * Implement image access test
 **************************************************************************************************/
bool cl_CPU_MIC_IntegerExecuteTest()
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;
    cl_device_id clCpuDeviceId;
    cl_device_id clMicDeviceId;
    cl_command_queue clCpuQueue = NULL;
    cl_command_queue clMicQueue = NULL;
    
    cl_device_id   devices[3];
    const wchar_t* dev_names[] = { L"Cpu", L"Mic", L"Cpu" };    
    cl_command_queue dev_queue[3];
    
    wchar_t title[1024];
    size_t global_work_size[1];

    size_t  stBuffSize = BUFFER_SIZE;
    cl_long pBuff[BUFFER_SIZE];
    cl_long pDstBuff[BUFFER_SIZE];
    cl_mem clBuff;
    
    cl_program program;
    cl_kernel  kernel;

	printf("=============================================================\n");
	printf("cl_CPU_MIC_IntegerExecuteTest\n");
	printf("=============================================================\n");

	const char *ocl_test_program[] = {\
	"__kernel void int_test(__global long* pValues)\n"\
	"{\n"\
	"size_t x = get_global_id(0);\n"\
	"long val = pValues[x];\n"\
	"long res = val + 10;\n"\
	"pValues[x] = res;\n"\
	"}"
	};

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &clCpuDeviceId, NULL);
    bResult &= Check(L"clGetDeviceIDs(CL_DEVICE_TYPE_CPU)", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &clMicDeviceId, NULL);
    if (CL_DEVICE_NOT_FOUND == iRet)
    {
        // CL_DEVICE_TYPE_ACCELERATOR not found - skipping the test
        bResult &= Check(L"clGetDeviceIDs(CL_DEVICE_TYPE_ACCELERATOR)", CL_DEVICE_NOT_FOUND, iRet);    
        return bResult;
    }
    
    bResult &= Check(L"clGetDeviceIDs(CL_DEVICE_TYPE_ACCELERATOR)", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    // For CPU-only uncomment
    //clCpuDeviceId = clMicDeviceId;

    // For MIC-only uncomment
    //clMicDeviceId = clCpuDeviceId;
    
    //
    // Initiate test infrastructure:
    // Create context, Queue
    //
    cl_device_id all_devices[] = { clCpuDeviceId, clMicDeviceId };
	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    cl_context context = clCreateContext( prop, sizeof( all_devices ) / sizeof( all_devices[0] ), all_devices, NULL, NULL, &iRet);
    bResult &= Check(L"clCreateContext", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    srand( 0 );
    
    // fill with random bits no matter what
    for( unsigned ui = 0; ui < BUFFER_SIZE; ui++ )
    {
        pBuff[ui] = (cl_long)(rand() - RAND_MAX/2) ;
    }
    
    clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(pBuff), pBuff, &iRet);
    bResult &= Check(L"clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult)
    {
        goto release_queues;
    }

    // create program with source
    if ( !BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, NULL, &program) )
    {
        goto release_context;
    }
    
    kernel = clCreateKernel(program, "int_test", &iRet);
    bResult &= Check(L"clCreateKernel - int_test", CL_SUCCESS, iRet);
    if (!bResult) goto release_program;

    // Set Kernel Arguments
    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
    bResult &= Check(L"clSetKernelArg - clBuff", CL_SUCCESS, iRet);
    if (!bResult) goto release_kernel;
    
    global_work_size[0] = stBuffSize;
    
    devices[0] = clCpuDeviceId;
    devices[1] = clMicDeviceId;
    devices[2] = clCpuDeviceId;

    clCpuQueue = clCreateCommandQueue (context, clCpuDeviceId, 0 /*no properties*/, &iRet);
    bResult &= Check(L"clCreateCommandQueue - queue for Cpu", CL_SUCCESS, iRet);
    if (!bResult) goto release_queues;

    clMicQueue = clCreateCommandQueue (context, clMicDeviceId, 0 /*no properties*/, &iRet);
    bResult &= Check(L"clCreateCommandQueue - queue for Mic", CL_SUCCESS, iRet);
    if (!bResult) goto release_queues;

    dev_queue[0] = clCpuQueue;
    dev_queue[1] = clMicQueue;
    dev_queue[2] = clCpuQueue;

    for (unsigned int curr_dev = 0; curr_dev < sizeof(devices)/sizeof(devices[0]); ++curr_dev )
	{
        cl_device_id     device   = devices[curr_dev];
        const wchar_t*   dev_name = dev_names[curr_dev];
        cl_command_queue queue    = dev_queue[curr_dev];
        
        
		// Execute kernel
		iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
		bResult &= Check(TITLE(L"clEnqueueNDRangeKernel for "), CL_SUCCESS, iRet);    
        if (!bResult) break;
        
		iRet = clFinish(queue);
		bResult &= Check(TITLE(L"clFinish for "), CL_SUCCESS, iRet);    
        if (!bResult) break;
    }

    if (!bResult) goto main_error_exit;
        
	//
	// Verification phase
	//
	iRet = clEnqueueReadBuffer( clCpuQueue, clBuff, CL_TRUE, 0, sizeof(pDstBuff), pDstBuff, 0, NULL, NULL );
	bResult &= Check(L"clEnqueueReadBuffer - Dst for Cpu", CL_SUCCESS, iRet);
	if (!bResult) goto main_error_exit;

	for( unsigned y=0; y < stBuffSize; ++y )
	{
		if ( pDstBuff[y] != (pBuff[y] + 10 * sizeof(devices)/sizeof(devices[0])))
		{
			bResult = false;
		}
	}

	if ( bResult )
	{
	    printf ("*** cl_CPU_MIC_IntegerExecuteTest compare verification succeeded *** \n");
	}
	else
	{
		printf ("!!!!!! cl_CPU_MIC_IntegerExecuteTest compare verification failed !!!!! \n");
	}

main_error_exit:
release_queues:
    if (NULL != clCpuQueue)
    {
		clFinish(clCpuQueue);
		clReleaseCommandQueue(clCpuQueue);
    }
    if (NULL != clMicQueue)
    {
		clFinish(clMicQueue);
		clReleaseCommandQueue(clMicQueue);
    }
release_kernel:
    clReleaseKernel(kernel);
release_program:
	clReleaseProgram(program);
release_context:
    clReleaseContext(context);
release_end:
    return bResult;
}

