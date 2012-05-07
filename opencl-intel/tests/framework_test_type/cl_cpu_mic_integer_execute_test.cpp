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

bool run_the_test(const char* test_name, bool include_migration)
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;

    const unsigned int number_of_rounds = 2;
    const wchar_t* dev_names[2] = { L"Cpu", L"Mic" };
    cl_device_type dev_types[2] = { CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_ACCELERATOR };
    cl_device_id   devices[2];
    cl_command_queue dev_queue[2] = {NULL, NULL};

	const unsigned int verification_device = 0;
    cl_device_id     device;
    const wchar_t*   dev_name;
    cl_command_queue queue;
    
    wchar_t title[1024];
    size_t global_work_size[1];

    size_t  stBuffSize = BUFFER_SIZE;
    cl_long pBuff[BUFFER_SIZE];
    cl_long pDstBuff[BUFFER_SIZE];
    cl_mem clBuff;
    
    cl_program program;
    cl_kernel  kernel;

	printf("=============================================================\n");
	printf("%s\n", test_name);
	printf("=============================================================\n");

	const char *ocl_test_program[] = {\
	"__kernel void int_test(__global long* pValues)\n"\
	"{\n"\
	"   size_t x = get_global_id(0);\n"\
	"   long val = pValues[x];\n"\
	"   long res = val + 10;\n"\
	"   pValues[x] = res;\n"\
	"}"
	};

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

    for (unsigned int curr_dev = 0; curr_dev < sizeof(dev_types)/sizeof(dev_types[0]); ++curr_dev )
    {
        dev_name = dev_names[curr_dev];
        
        iRet = clGetDeviceIDs(platform, dev_types[curr_dev], 1, &(devices[curr_dev]), NULL);

        if ((CL_DEVICE_NOT_FOUND == iRet) && (CL_DEVICE_TYPE_CPU != dev_types[curr_dev]))
        {
            // CL_DEVICE_TYPE_ACCELERATOR not found - skipping the test
            bResult &= SilentCheck(TITLE(L"clGetDeviceIDs for "), CL_DEVICE_NOT_FOUND, iRet);    
            return bResult;
        }
        
        bResult &= SilentCheck(TITLE(L"clGetDeviceIDs for "), CL_SUCCESS, iRet);    
        if (!bResult) return bResult;
    }
    
    //
    // Initiate test infrastructure:
    // Create context, Queue
    //
	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    cl_context context = clCreateContext( prop, sizeof( devices ) / sizeof( devices[0] ), devices, NULL, NULL, &iRet);
    bResult &= SilentCheck(L"clCreateContext", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    srand( 0 );
    
    // fill with random bits no matter what
    for( unsigned ui = 0; ui < BUFFER_SIZE; ui++ )
    {
        pBuff[ui] = (cl_long)(ui + 0x1000);
    }
    
    clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(pBuff), pBuff, &iRet);
    bResult &= SilentCheck(L"clCreateBuffer", CL_SUCCESS, iRet);
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
    bResult &= SilentCheck(L"clCreateKernel - int_test", CL_SUCCESS, iRet);
    if (!bResult) goto release_program;

    // Set Kernel Arguments
    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
    bResult &= SilentCheck(L"clSetKernelArg - clBuff", CL_SUCCESS, iRet);
    if (!bResult) goto release_kernel;
    
    global_work_size[0] = stBuffSize;

    for (unsigned int curr_dev = 0; curr_dev < sizeof(devices)/sizeof(devices[0]); ++curr_dev )
	{
        device   = devices[curr_dev];
        dev_name = dev_names[curr_dev];

        dev_queue[curr_dev] = clCreateCommandQueue (context, device, 0 /*no properties*/, &iRet);
        bResult &= SilentCheck(TITLE(L"clCreateCommandQueue - queue for "), CL_SUCCESS, iRet);
        if (!bResult) goto release_queues;
    }

    for (unsigned int round = 0; round < number_of_rounds; ++round)
    {
        for (unsigned int curr_dev = 0; curr_dev < sizeof(devices)/sizeof(devices[0]); ++curr_dev )
    	{
            device   = devices[curr_dev];
            dev_name = dev_names[curr_dev];
            queue    = dev_queue[curr_dev];

            if (include_migration)
            {
                printf("...Migrate from HOST to %ls...\n",  dev_name );
                iRet = clEnqueueMigrateMemObjects(queue, 1, &clBuff, 0, 0, NULL, NULL);
                bResult &= SilentCheck(TITLE(L"clEnqueueMigrateMemObjects from HOST to "), CL_SUCCESS, iRet);    
                if (!bResult) break;
            }
            
    		// Execute kernel
    		printf("...NDRange(%ls)...\n",  dev_name );
    		iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    		bResult &= SilentCheck(TITLE(L"clEnqueueNDRangeKernel for "), CL_SUCCESS, iRet);    
            if (!bResult) break;

            if (include_migration)
            {
                printf("...Migrate from %ls to HOST...\n",  dev_name );
                iRet = clEnqueueMigrateMemObjects(queue, 1, &clBuff, CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL);
                bResult &= SilentCheck(TITLE(L"clEnqueueMigrateMemObjects to HOST from "), CL_SUCCESS, iRet);    
                if (!bResult) break;
            }

    		iRet = clFinish(queue);
    		bResult &= SilentCheck(TITLE(L"clFinish for "), CL_SUCCESS, iRet);    
            if (!bResult) break;
        }

        if (!bResult) break;
    }

    if (!bResult) goto main_error_exit;
        
	//
	// Verification phase
	//
	device   = devices[verification_device];
    dev_name = dev_names[verification_device];
    queue    = dev_queue[verification_device];
    
    printf("...ReadBuffer(%ls)...\n", dev_name );
	iRet = clEnqueueReadBuffer( queue, clBuff, CL_TRUE, 0, sizeof(pDstBuff), pDstBuff, 0, NULL, NULL );
	bResult &= SilentCheck(TITLE(L"clEnqueueReadBuffer - Dst for "), CL_SUCCESS, iRet);
	if (!bResult) goto main_error_exit;

	for( unsigned y=0; y < stBuffSize; ++y )
	{
		if ( pDstBuff[y] != (pBuff[y] + 10 * number_of_rounds * sizeof(devices)/sizeof(devices[0])))
		{
			bResult = false;
            break;
		}
	}

	if ( bResult )
	{
	    printf ("*** %s compare verification succeeded *** \n", test_name);
	}
	else
	{
		printf ("!!!!!! %s compare verification failed !!!!! \n", test_name);
	}

main_error_exit:
release_queues:
    for (unsigned int curr_dev = 0; curr_dev < sizeof(dev_queue)/sizeof(dev_queue[0]); ++curr_dev )
    {
        queue = dev_queue[curr_dev];
        if (NULL == queue)
        {
            continue;
        }
		clFinish(queue);
		clReleaseCommandQueue(queue);
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

/**************************************************************************************************
 * cl_CPU_MIC_IntegerExecuteTest
 * -------------------
 * Implement image access test
 **************************************************************************************************/
bool cl_CPU_MIC_IntegerExecuteTest()
{
    return run_the_test(__FUNCTION__, false);
}

/**************************************************************************************************
 * cl_CPU_MIC_MigrateTest
 * -------------------
 * Implement image access test
 **************************************************************************************************/
bool cl_CPU_MIC_MigrateTest()
{
    return run_the_test(__FUNCTION__, true);
}


