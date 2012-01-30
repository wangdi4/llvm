#include <CL/cl.h>
#include <stdio.h>
#include "FrameworkTest.h"
//|
//| TEST: Memoryleak.CreateReleaseOOOQueueTest
//|
//| Purpose 
//| -------
//|
//| Test all memory is released when releasing an out of order command queue
//|
//| Method
//| ------
//|
//| 1. Create a context with a single device
//| 2. Repeatedly create and release an OOO queue
//|
//| Pass criteria
//| -------------
//|
//| No crash due to running out of memory

bool CreateReleaseOOOQueueTest()
{
	printf("CreateReleaseOOOQueueTest\n");
	cl_uint                     uiNumDevices = 0;
	cl_device_id*               pDevices;
	cl_context                  context;
	cl_command_queue            queue;
	cl_command_queue_properties queue_properties;
	cl_device_id                device_id;

	const size_t                ITERATION_COUNT = 500000;

	bool bResult = true;
	cl_int iRet = CL_SUCCESS;

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &uiNumDevices);
	bResult &= SilentCheck(L"clGetDeviceIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}


	pDevices = new cl_device_id[uiNumDevices];
	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);
	bResult &= SilentCheck(L"clGetDeviceIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	device_id = pDevices[0];

	iRet = clGetDeviceInfo(device_id, CL_DEVICE_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queue_properties, NULL);
	bResult &= SilentCheck(L"clGetDeviceInfo (CL_DEVICE_QUEUE_PROPERTIES)", CL_SUCCESS, iRet);

	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	if (!(queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE))
	{
		printf("Device doesn't support OOO queues. Passing vacuously\n");
		delete[] pDevices;
		return true;
	}

	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
    delete []pDevices;
	bResult &= SilentCheck(L"clCreateContext", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	for (size_t i = 0; i < ITERATION_COUNT; ++i)
	{
    	queue = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &iRet);
	    bResult &= SilentCheck(L"clCreateCommandQueue", CL_SUCCESS, iRet);

	    if (!bResult)
	    {
            clReleaseContext(context);
		    return bResult;
	    }

		iRet = clReleaseCommandQueue(queue);
	    bResult &= SilentCheck(L"clReleaseCommandQueue", CL_SUCCESS, iRet);

	    if (!bResult)
	    {
		    clReleaseContext(context);
		    return bResult;
	    }
	}

    clReleaseContext(context);

	return bResult;
}
