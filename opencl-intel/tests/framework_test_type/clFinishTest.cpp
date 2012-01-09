#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_sys_defines.h"


typedef struct 
{
	cl_command_queue queue;
	cl_mem           buf;
	cl_event         evt;
	void*            ptr;
} callback_data_type;

static const size_t bufSize = 4;
static volatile bool canExecute = false;

static void CL_CALLBACK NativeFunc(void *pData)
{
	//just block
	while (!canExecute);
}

static void STDCALL CallbackFunc (cl_event notifier, cl_int status, void *user_data)
{

	callback_data_type* pData = static_cast<callback_data_type*>(user_data);
	clEnqueueNativeKernel(pData->queue, NativeFunc, NULL, 0, 0, NULL, NULL, 0, NULL, &pData->evt);
	//clEnqueueReadBuffer(pData->queue, pData->buf, CL_FALSE, 0, bufSize, pData->ptr, 0, NULL, &pData->evt);
	clFlush(pData->queue);
}


bool clFinishTest()
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

	cl_event readBufferEvent;
	cl_event callbackEvent;
	char data[bufSize];
	cl_mem buf;

	buf = clCreateBuffer(context, CL_MEM_READ_WRITE, bufSize, NULL, &iRet);
	bResult &= SilentCheck(L"clCreateBuffer", CL_SUCCESS, iRet);

	callback_data_type callback_data;
	callback_data.queue = queue;
	callback_data.ptr   = &data;
	callback_data.buf   = buf;
	callback_data.evt   = 0;

	memset(data, 0xF4, bufSize);

	iRet = clEnqueueWriteBuffer(queue, buf, CL_FALSE, 0, bufSize, &data, 0, NULL, &callbackEvent);
	bResult &= SilentCheck(L"clEnqueueWriteBuffer", CL_SUCCESS, iRet);

	iRet = clSetEventCallback(callbackEvent, CL_COMPLETE, CallbackFunc, &callback_data);
	bResult &= SilentCheck(L"clSetEventCallback", CL_SUCCESS, iRet);

	iRet = clFinish(queue);
	bResult &= SilentCheck(L"clFinish", CL_SUCCESS, iRet);

	cl_int eventStatus = CL_COMPLETE;
	readBufferEvent = callback_data.evt;

	iRet = clGetEventInfo(readBufferEvent, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &eventStatus, NULL);
	bResult &= SilentCheck(L"clGetEventInfo", CL_SUCCESS, iRet);
	bool NotYetExecuted = (CL_RUNNING == eventStatus || CL_SUBMITTED == eventStatus);
	if (!NotYetExecuted)
	{
		printf("Error: event either only queued or complete, test failed\n");
		bResult = false;
	}

	clReleaseEvent(callbackEvent);

	canExecute = true;
	iRet = clFinish(queue);
	bResult &= SilentCheck(L"clFinish", CL_SUCCESS, iRet);

	iRet = clGetEventInfo(readBufferEvent, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &eventStatus, NULL);
	bResult &= SilentCheck(L"clGetEventInfo", CL_SUCCESS, iRet);
	bResult &= SilentCheck(L"Event Status", CL_COMPLETE, eventStatus);

	clReleaseEvent(readBufferEvent);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return bResult;
}
