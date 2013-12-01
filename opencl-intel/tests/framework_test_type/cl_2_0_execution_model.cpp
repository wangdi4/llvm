// Copyright (c) 2008-2012 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <iostream>
#include "test_utils.h"

using namespace std;

extern cl_device_type gDeviceType;

static void TestBadProperties(cl_device_id device, cl_context context) /*throw (exception)*/
{
	cl_int iRet = CL_SUCCESS;
	cl_command_queue queue;

	cl_queue_properties props2[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_ON_DEVICE, 0 };	// missing OOO
	queue = clCreateCommandQueueWithProperties(context, device, props2, &iRet);
	CheckException(L"clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
	CHECK_COND(L"clCreateCommandQueueWithProperties", NULL == queue);

	cl_queue_properties props3[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_ON_DEVICE_DEFAULT, 0 };	// default, but not device queue
	queue = clCreateCommandQueueWithProperties(context, device, props3, &iRet);
	CheckException(L"clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
	CHECK_COND(L"clCreateCommandQueueWithProperties", NULL == queue);

	cl_queue_properties props4[] = { 1, 1, 0 };	// invalid name
	queue = clCreateCommandQueueWithProperties(context, device, props4, &iRet);
	CheckException(L"clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
	CHECK_COND(L"clCreateCommandQueueWithProperties", NULL == queue);

	cl_queue_properties props5[] = { CL_QUEUE_PROPERTIES, 0, CL_QUEUE_SIZE, 5, 0 };	// queue size for host queue
	queue = clCreateCommandQueueWithProperties(context, device, props5, &iRet);
	CheckException(L"clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
	CHECK_COND(L"clCreateCommandQueueWithProperties", NULL == queue);
}

bool cl20ExecutionModel()
{
	cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL, defaultQueue = NULL;

	cout << "=============================================================" << endl;
    cout << "cl20ExecutionModel" << endl;
    cout << "=============================================================" << endl;

	try
	{
		// create context
		iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException(L"clGetPlatformIDs", CL_SUCCESS, iRet);        
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException(L"clGetDeviceIDs", CL_SUCCESS, iRet);        
        const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };    
        context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);    
        CheckException(L"clCreateContextFromType", CL_SUCCESS, iRet);    

		// do the real testing		

		cl_queue_properties props[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE, 0 };
		queue = clCreateCommandQueueWithProperties(context, device, props, &iRet);
		CheckException(L"clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
		CHECK_COND(L"clCreateCommandQueueWithProperties", NULL != queue);		

		cl_uint uiQueueSize, uiDevQueuePreferredSize;
		size_t szQueueSizeSize, szDevQueuePreferredSizeSize;
		iRet = clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, sizeof(uiDevQueuePreferredSize), &uiDevQueuePreferredSize, &szDevQueuePreferredSizeSize);
		CheckException(L"clGetDeviceInfo", CL_SUCCESS, iRet);
		CheckException(L"clGetDeviceInfo", sizeof(uiDevQueuePreferredSize), szDevQueuePreferredSizeSize);		

		cl_uint uiOnDeviceQueues;
		size_t szOnDeviceQueuesSize;
		iRet = clGetDeviceInfo(device, CL_DEVICE_MAX_ON_DEVICE_QUEUES, sizeof(uiOnDeviceQueues), &uiOnDeviceQueues, &szOnDeviceQueuesSize);
		CheckException(L"clGetDeviceInfo", CL_SUCCESS, iRet);
		CheckException(L"clGetDeviceInfo", sizeof(uiOnDeviceQueues), szOnDeviceQueuesSize);

		cl_queue_properties defaultProps[] = { CL_QUEUE_PROPERTIES,  CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT, 0 };
		defaultQueue = clCreateCommandQueueWithProperties(context, device, defaultProps, &iRet);
		CheckException(L"clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
		CHECK_COND(L"clCreateCommandQueueWithProperties", NULL != queue);

		// check that a second default queue can't be created for the same device
		clCreateCommandQueueWithProperties(context, device, defaultProps, &iRet);
		CheckException(L"clCreateCommandQueueWithProperties", CL_INVALID_COMMAND_QUEUE, iRet);

		iRet = clGetCommandQueueInfo(queue, CL_QUEUE_SIZE, sizeof(uiQueueSize), &uiQueueSize, &szQueueSizeSize);
		CheckException(L"clGetCommandQueueInfo", CL_SUCCESS, iRet);
		CheckException(L"clGetCommandQueueInfo", sizeof(uiQueueSize), szQueueSizeSize);
		CheckException(L"uiQueueSize == uiDevQueuePreferredSize", uiQueueSize, uiDevQueuePreferredSize);

		cl_command_queue_properties reportedProps;
		size_t szPropsSize;
		iRet = clGetCommandQueueInfo(defaultQueue, CL_QUEUE_PROPERTIES, sizeof(reportedProps), &reportedProps, &szPropsSize);
		CheckException(L"clGetCommandQueueInfo", CL_SUCCESS, iRet);
		CheckException(L"clGetCommandQueueInfo", sizeof(reportedProps), szPropsSize);
		CheckException(L"props == props[1]", defaultProps[1], reportedProps);

		cl_device_id reportedDevice;
		size_t szDevIdSize;
		iRet = clGetCommandQueueInfo(defaultQueue, CL_QUEUE_DEVICE, sizeof(reportedDevice), &reportedDevice, &szDevIdSize);
		CheckException(L"clGetCommandQueueInfo", CL_SUCCESS, iRet);
		CheckException(L"clGetCommandQueueInfo", sizeof(reportedDevice), szDevIdSize);
		CheckException(L"device == reportedDevice", device, reportedDevice);

		TestBadProperties(device, context);
	}
	catch (const std::exception&)
    {
        bResult = false;
    }
	if (NULL != queue)
	{
		iRet = clReleaseCommandQueue(queue);
		if (!SilentCheck(L"clReleaseCommandQueue", CL_SUCCESS, iRet))
		{
			return false;
		}
	}
	if (NULL != defaultQueue)
	{
		iRet = clReleaseCommandQueue(defaultQueue);
		if (!SilentCheck(L"clReleaseCommandQueue", CL_SUCCESS, iRet))
		{
			return false;
		}
	}
	if (NULL != context)
    {
        iRet = clReleaseContext(context);
		if (!SilentCheck(L"clReleaseContext", CL_SUCCESS, iRet))
		{
			return false;
		}
    }
	return bResult;
}
