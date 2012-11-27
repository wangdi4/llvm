#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"

#define BUFFER_SIZE 1024

bool validateData(unsigned int numDevices, unsigned int numRounds, unsigned int numIncInRound, unsigned int round, unsigned int deviceRound, unsigned int readNum, unsigned int expectedFrom, unsigned int bufferFrom, unsigned int size, cl_long* buff)
{
	bool result = true;
	for (unsigned int ui = 0; ui < size; ui++)
	{
		cl_long initialValue = ((expectedFrom + ui) * numIncInRound * numDevices * numRounds) + (expectedFrom + ui + 1);
		cl_long expectedValue = initialValue + ((round * numDevices * numIncInRound) + (deviceRound * numIncInRound) + readNum);
		if (buff[bufferFrom+ ui] != expectedValue)
		{
			printf("Validation error: Buff[%d] = %d, expected %d\n", (bufferFrom + ui), buff[bufferFrom + ui], expectedValue);
			result = false;
			break;
		}
	}
	return result;
}


bool run_common_rt_sub_buffers_async_test(const char* test_name, bool releaseBuffTest)
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;

    const unsigned int number_of_rounds = 512;
	unsigned int num_inc_in_round = 2;
	if (releaseBuffTest)
	{
		num_inc_in_round = 16;
	}
    cl_device_type dev_types[2] = { CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_ACCELERATOR };
    cl_device_id   devices[2];
    cl_command_queue dev_queue[2] = {NULL, NULL};

    cl_device_id     device;
    cl_command_queue queue, queueA, queueB;
    
    size_t global_work_size[1];

    const size_t  stBuffSize = BUFFER_SIZE;
	const size_t  stSubBuffSize = BUFFER_SIZE / 2;
    cl_long pBuff[stBuffSize];
	cl_long pDestBuff[stBuffSize];
	cl_long pDstSubBuffA[stSubBuffSize];
	cl_long pDstSubBuffB[stSubBuffSize];

    cl_mem clParentBuff = NULL;
	cl_mem clSubBuffA = NULL;
	cl_mem clSubBuffB = NULL;
    
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
	"   long res = val + 1;\n"\
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
        iRet = clGetDeviceIDs(platform, dev_types[curr_dev], 1, &(devices[curr_dev]), NULL);

        if ((CL_DEVICE_NOT_FOUND == iRet) && (CL_DEVICE_TYPE_CPU != dev_types[curr_dev]))
        {
            // CL_DEVICE_TYPE_ACCELERATOR not found - skipping the test
            bResult &= SilentCheck(L"clGetDeviceIDs", CL_DEVICE_NOT_FOUND, iRet);    
            return bResult;
        }
        
        bResult &= SilentCheck(L"clGetDeviceIDs", CL_SUCCESS, iRet);    
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

	unsigned int numDevices = sizeof(devices)/sizeof(devices[0]);
    
    // fill with data.
    for( unsigned ui = 0; ui < stBuffSize; ui++ )
    {
        pBuff[ui] = (cl_long)((ui * num_inc_in_round * numDevices * number_of_rounds) + (ui + 1));
    }
    
    clParentBuff = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(pBuff), pBuff, &iRet);
    bResult &= SilentCheck(L"clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult)
    {
        goto release_queues;
    }

	cl_buffer_region region;
    region.origin = 0;
    region.size = stSubBuffSize * sizeof(cl_long);
	clSubBuffA = clCreateSubBuffer( clParentBuff, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet );
	bResult &= SilentCheck(L"clCreateSubBuffer", CL_SUCCESS, iRet);
    if (!bResult)
    {
        goto release_queues;
    }
	region.origin = stSubBuffSize * sizeof(cl_long);
	clSubBuffB = clCreateSubBuffer( clParentBuff, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet );
	bResult &= SilentCheck(L"clCreateSubBuffer", CL_SUCCESS, iRet);
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

    for (unsigned int curr_dev = 0; curr_dev < sizeof(devices)/sizeof(devices[0]); ++curr_dev )
	{
        device   = devices[curr_dev];

        dev_queue[curr_dev] = clCreateCommandQueue (context, device, 0 /*no properties*/, &iRet);
        bResult &= SilentCheck(L"clCreateCommandQueue", CL_SUCCESS, iRet);
        if (!bResult) goto release_queues;
    }

	if (releaseBuffTest)
	{
		// cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release test.
		global_work_size[0] = stBuffSize / 2;

		for (unsigned int round = 0; round < number_of_rounds; ++round)
		{
			for (unsigned int curr_dev = 0; curr_dev < numDevices; ++curr_dev )
    		{
				queueA    = dev_queue[curr_dev];
				queueB    = dev_queue[(curr_dev + 1) % numDevices];

				cl_event ev = clCreateUserEvent(context, &iRet);
				bResult &= SilentCheck(L"clCreateUserEvent", CL_SUCCESS, iRet);    
				if (!bResult) break;
				cl_event* pEv = &ev;
				unsigned int numOfEvents = 1;

				for (unsigned int i = 0; i < num_inc_in_round; i++)
				{
					// Set Kernel Arguments
					iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clSubBuffA);
					bResult &= SilentCheck(L"clSetKernelArg - clSubBuffA", CL_SUCCESS, iRet);
					if (!bResult) break;
       				// Execute kernel
    				iRet = clEnqueueNDRangeKernel(queueA, kernel, 1, NULL, global_work_size, NULL, numOfEvents, pEv, NULL);
    				bResult &= SilentCheck(L"clEnqueueNDRangeKernel for queueA", CL_SUCCESS, iRet);    
					if (!bResult) break;
					// Set Kernel Arguments
					iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clSubBuffB);
					bResult &= SilentCheck(L"clSetKernelArg - clSubBuffB", CL_SUCCESS, iRet);
					if (!bResult) break;
       				// Execute kernel
    				iRet = clEnqueueNDRangeKernel(queueB, kernel, 1, NULL, global_work_size, NULL, numOfEvents, pEv, NULL);
    				bResult &= SilentCheck(L"clEnqueueNDRangeKernel for queueB", CL_SUCCESS, iRet);    
					if (!bResult) break;
					pEv = NULL;
					numOfEvents = 0;
				}

				if (!bResult) break;

				// Release the subBuffer before executing all the commands with this subBuffer and create new subBuffer on the same region --> it change the memory mode to overlapping until all the commands will complete.
				clReleaseMemObject(clSubBuffB);
				clSubBuffB = NULL;
				clSubBuffB = clCreateSubBuffer( clParentBuff, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet );
				bResult &= SilentCheck(L"clCreateSubBuffer", CL_SUCCESS, iRet);
				if (!bResult) break;

				// triger the batch ndranges.
				clSetUserEventStatus (ev, CL_COMPLETE);
				iRet = clFinish(queueA);
				bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);    
				if (!bResult) break;
				iRet = clFinish(queueB);
				bResult &= SilentCheck(L"clFinish for queueB", CL_SUCCESS, iRet);    
				if (!bResult) break;

				if (round % 2 == 0)
				{
					// map both subBuffers and validate the content.
					cl_event ev1 = clCreateUserEvent(context, &iRet);
					bResult &= SilentCheck(L"clCreateUserEvent", CL_SUCCESS, iRet);    
					if (!bResult) break;

					cl_long* mapPtr1 = (cl_long*)clEnqueueMapBuffer(queueA, clSubBuffA, CL_FALSE, CL_MAP_READ, 0, sizeof(pDstSubBuffA), 1, &ev1, NULL, &iRet);
					bResult &= SilentCheck(L"clEnqueueMapBuffer from queueB, clSubBuffA to mapPtr1", CL_SUCCESS, iRet);
					if (!bResult) break;
					cl_long* mapPtr2 = (cl_long*)clEnqueueMapBuffer(queueB, clSubBuffB, CL_FALSE, CL_MAP_READ, 0, sizeof(pDstSubBuffB), 1, &ev1, NULL, &iRet);
					bResult &= SilentCheck(L"clEnqueueMapBuffer from queueB, clSubBuffB to mapPtr2", CL_SUCCESS, iRet);
					if (!bResult) break;

					clSetUserEventStatus (ev1, CL_COMPLETE);
					iRet = clFinish(queueA);
					bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);    
					if (!bResult) break;
					iRet = clFinish(queueB);
					bResult &= SilentCheck(L"clFinish for queueB", CL_SUCCESS, iRet);    
					if (!bResult) break;
					if ((!validateData(numDevices, number_of_rounds, num_inc_in_round, round, curr_dev, num_inc_in_round, 0, 0, stSubBuffSize, mapPtr1)) ||
						(!validateData(numDevices, number_of_rounds, num_inc_in_round, round, curr_dev, num_inc_in_round, stSubBuffSize, 0, stSubBuffSize, mapPtr2)))
					{
						bResult = false;
						break;
					}
					iRet = clEnqueueUnmapMemObject(queueA, clSubBuffA, mapPtr1, 0, NULL, NULL);
					bResult &= SilentCheck(L"clEnqueueUnMapBuffer - clSubBuffA", CL_SUCCESS, iRet);
					iRet = clEnqueueUnmapMemObject(queueB, clSubBuffB, mapPtr2, 0, NULL, NULL);
					bResult &= SilentCheck(L"clEnqueueUnMapBuffer - clSubBuffB", CL_SUCCESS, iRet);
					if (!bResult) break;
				}
				else
				{
					// Read parent buffer. device B
					iRet = clEnqueueReadBuffer( queueB, clParentBuff, CL_TRUE, 0, sizeof(pBuff), pDestBuff, 0, NULL, NULL );
					bResult &= SilentCheck(L"clEnqueueReadBuffer - clParentBuff to pDestBuff", CL_SUCCESS, iRet);
					if (!bResult) break;
					if (!validateData(numDevices, number_of_rounds, num_inc_in_round, round, curr_dev, num_inc_in_round, 0, 0, stBuffSize, pDestBuff))
					{
						bResult = false;
						break;
					}
				}
				iRet = clFinish(queueA);
				bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);    
				if (!bResult) break;
				iRet = clFinish(queueB);
				bResult &= SilentCheck(L"clFinish for queueB", CL_SUCCESS, iRet);

			}

			if (!bResult) break;
		}
	}
	else
	{
		// cl_CPU_MIC_Common_RT_SubBuffers_Async test
		for (unsigned int round = 0; round < number_of_rounds; ++round)
		{
			for (unsigned int curr_dev = 0; curr_dev < numDevices; ++curr_dev )
    		{
				queueA    = dev_queue[curr_dev];
				queueB    = dev_queue[(curr_dev + 1) % numDevices];

				// Write to Parent buffer on device A.

				// Set Kernel Arguments
				iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clParentBuff);
				bResult &= SilentCheck(L"clSetKernelArg - clParentBuff", CL_SUCCESS, iRet);
				if (!bResult) break;
				global_work_size[0] = stBuffSize;
    			// Execute kernel
    			iRet = clEnqueueNDRangeKernel(queueA, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    			bResult &= SilentCheck(L"clEnqueueNDRangeKernel for queueA", CL_SUCCESS, iRet);    
				if (!bResult) break;
    			iRet = clFinish(queueA);
    			bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);    
				if (!bResult) break;

				cl_event ev1 = clCreateUserEvent(context, &iRet);
				bResult &= SilentCheck(L"clCreateUserEvent", CL_SUCCESS, iRet);    
				if (!bResult) break;

				// Read parent from child buffers. will triger when ev1 signal. (on both devices simultaneously)
				iRet = clEnqueueReadBuffer( queueA, clSubBuffA, CL_FALSE, 0, sizeof(pDstSubBuffA), pDstSubBuffA, 1, &ev1, NULL );
				bResult &= SilentCheck(L"clEnqueueReadSubBuffer - clSubBuffA to pDstSubBuffA", CL_SUCCESS, iRet);
				if (!bResult) break;
				iRet = clEnqueueReadBuffer( queueB, clSubBuffB, CL_FALSE, 0, sizeof(pDstSubBuffB), pDstSubBuffB, 1, &ev1, NULL );
				bResult &= SilentCheck(L"clEnqueueReadSubBuffer - clSubBuffB to pDstSubBuffB", CL_SUCCESS, iRet);
				if (!bResult) break;
				clSetUserEventStatus (ev1, CL_COMPLETE);
				iRet = clFinish(queueA);
				bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);    
				if (!bResult) break;
				iRet = clFinish(queueB);
				bResult &= SilentCheck(L"clFinish for queueB", CL_SUCCESS, iRet);    
				if (!bResult) break;
				if ((!validateData(numDevices, number_of_rounds, num_inc_in_round, round, curr_dev, 1, 0, 0, stSubBuffSize, pDstSubBuffA)) ||
					(!validateData(numDevices, number_of_rounds, num_inc_in_round, round, curr_dev, 1, stSubBuffSize, 0, stSubBuffSize, pDstSubBuffB)))
				{
					bResult = false;
					break;
				}

				// write to child buffers. (on both devices simultaneously)
				cl_event ev2 = clCreateUserEvent(context, &iRet);
				bResult &= SilentCheck(L"clCreateUserEvent", CL_SUCCESS, iRet);    
				if (!bResult) break;
				// Set Kernel Arguments
				iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clSubBuffA);
				bResult &= SilentCheck(L"clSetKernelArg - clSubBuffA", CL_SUCCESS, iRet);
				if (!bResult) break;
    			global_work_size[0] = stBuffSize / 2;
       			// Execute kernel
    			iRet = clEnqueueNDRangeKernel(queueB, kernel, 1, NULL, global_work_size, NULL, 1, &ev2, NULL);
    			bResult &= SilentCheck(L"clEnqueueNDRangeKernel for queueB", CL_SUCCESS, iRet);    
				if (!bResult) break;
				// Set Kernel Arguments
				iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clSubBuffB);
				bResult &= SilentCheck(L"clSetKernelArg - clParentBuff", CL_SUCCESS, iRet);
				if (!bResult) break;
       			// Execute kernel
    			iRet = clEnqueueNDRangeKernel(queueA, kernel, 1, NULL, global_work_size, NULL, 1, &ev2, NULL);
    			bResult &= SilentCheck(L"clEnqueueNDRangeKernel for queueA", CL_SUCCESS, iRet);    
				if (!bResult) break;
				clSetUserEventStatus (ev2, CL_COMPLETE);
				iRet = clFinish(queueB);
				bResult &= SilentCheck(L"clFinish for queueB", CL_SUCCESS, iRet);    
				if (!bResult) break;
				iRet = clFinish(queueA);
				bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);

				// Map parent buffer. On device B
				cl_long* mapPtr = (cl_long*)clEnqueueMapBuffer(queueB, clParentBuff, CL_TRUE, CL_MAP_READ, 0, sizeof(pBuff), 0, NULL, NULL, &iRet);
				bResult &= SilentCheck(L"clEnqueueMapBuffer from queueB, clParentBuff to mapPtr", CL_SUCCESS, iRet);
				if (!bResult) break;
				if (!validateData(numDevices, number_of_rounds, num_inc_in_round, round, curr_dev, 2, 0, 0, stBuffSize, mapPtr))
				{
					bResult = false;
					break;
				}

				iRet = clEnqueueUnmapMemObject(queueB, clParentBuff, mapPtr, 0, NULL, NULL);
				bResult &= SilentCheck(L"clEnqueueMapBuffer - clParentBuff", CL_SUCCESS, iRet);
				if (!bResult) break;
				iRet = clFinish(queueB);
				bResult &= SilentCheck(L"clFinish for queueB", CL_SUCCESS, iRet);    
				if (!bResult) break;
				iRet = clFinish(queueA);
				bResult &= SilentCheck(L"clFinish for queueA", CL_SUCCESS, iRet);

			}

			if (!bResult) break;
		}
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
release_buffers:
	if (NULL != clSubBuffA)
	{
		clReleaseMemObject(clSubBuffA);
	}
	if (NULL != clSubBuffB)
	{
		clReleaseMemObject(clSubBuffB);
	}
	if (NULL != clParentBuff)
	{
		clReleaseMemObject(clParentBuff);
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
 * cl_CPU_MIC_Common_RT_SubBuffers_Async
 * -------------------------------------
 * This test create shared context of CPU and MIC device.
 * Create Parent buffer and 2 subBuffers that cover the parent buffer.
 * create 2 command queues.
 * Execute in loop X rounds:
 *     for each device:
 *         (1) Run NDRange on Parent buffer that change the parent content.
 *         (2) Read the parent content by the two subBuffers simultaneously. (each on different device)
 *         (3) Run NDRange simultaneously on the two subBuffers that change the content (each subBuffer run on different device that it ran on previous stage).
 *         (4) Map The parent buffer from the second device. (Not the device that it was in stage 1).
 * After each Read / Map operation (by parent buffer or subBuffers) validating the content of the buffers.
 **************************************************************************************************/
bool cl_CPU_MIC_Common_RT_SubBuffers_Async()
{
    return run_common_rt_sub_buffers_async_test(__FUNCTION__, false);
}

/**************************************************************************************************
 * cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release
 * -------------------------------------
 * This test create shared context of CPU and MIC device.
 * Create Parent buffer and 2 subBuffers that cover the parent buffer.
 * create 2 command queues.
 * Execute in loop X rounds:
 *     for each device:
 *			for 1000 times:
 *		         (1) Enqueue NDRange simultaneously with the two subBuffers that will change the content.(It wait for event triggering)
 *         (2) Release one of the subBuffers
 *         (3) Create new subBuffer instead of the deleted subBuffer --> change the memory mode to Overlapping.
 *         (4) Triger the event.
 *		   (5) Read the data from parent buffer or the subBuffers and validate the content.
 **************************************************************************************************/
bool cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release()
{
	 return run_common_rt_sub_buffers_async_test(__FUNCTION__, true);
}

