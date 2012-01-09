#include "CL/cl.h"
#include "FrameworkTest.h"

typedef struct 
{
	cl_command_queue queue;
	bool flag;
} callback_data_type;

void STDCALL SystemEventCallbackFunc (cl_event notifier, cl_int status, void *user_data)
{

	callback_data_type* pData = static_cast<callback_data_type*>(user_data);
	clFlush(pData->queue);
	pData->flag = true;
}


bool EventCallbackTest()
{
	bool bResult = true;
	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	cl_context context;

	cl_platform_id platform = 0;

	cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	// get device(s)
	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &uiNumDevices);
	bResult &= SilentCheck(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);
	bResult &= SilentCheck(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	bResult &= SilentCheck(L"clCreateContext",CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}


	// Create queue
	cl_command_queue queue = clCreateCommandQueue (context, pDevices[0], 0 /*no properties*/, &iRet);
	bResult &= SilentCheck(L"clCreateCommandQueue", CL_SUCCESS, iRet);

	delete[] pDevices;

	callback_data_type data;
	data.queue = queue;
	data.flag  = false;
	cl_event marker_event;
	iRet = clEnqueueMarker(queue, &marker_event);
	bResult &= SilentCheck(L"clEnqueueMarker", CL_SUCCESS, iRet);

	iRet = clSetEventCallback(marker_event, CL_COMPLETE, SystemEventCallbackFunc, &data);
	bResult &= SilentCheck(L"clSetEventCallback", CL_SUCCESS, iRet);

	iRet = clFinish(queue);
	bResult &= SilentCheck(L"clFinish", CL_SUCCESS, iRet);

	clReleaseEvent(marker_event);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return data.flag;
}

