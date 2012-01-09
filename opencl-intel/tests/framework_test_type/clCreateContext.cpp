#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/**************************************************************************************************
* clCreateContextTest
* -------------------
* Get device ids (CL_DEVICE_TYPE_CPU)
* Create context
* Create context from type (CL_DEVICE_TYPE_CPU)
* Retain context
* Release context
* Get context info (CL_CONTEXT_REFERENCE_COUNT)
**************************************************************************************************/
bool clCreateContextTest()
{
	printf("clCreateContextTest\n");
	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	cl_context context;

	bool bResult = true;
	cl_int iRet = CL_SUCCESS;

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 },
        badProps1[] = { CL_CONTEXT_PLATFORM, 0xffffffff, 0 },
        badProps2[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0},
        badProps3[] = { 1, 0, 0};   // unsupported property name

	cl_context context_default = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &iRet);
	bResult &= Check(L"Create Context from type (CL_DEVICE_TYPE_DEFAULT)", CL_SUCCESS, iRet);

    // bad properties
    clCreateContextFromType(badProps1, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &iRet);
    bResult &= Check(L"Create Context from type (bad platform)", CL_INVALID_PLATFORM, iRet);

    clCreateContextFromType(badProps2, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &iRet);
    bResult &= Check(L"Create Context from type (duplicate property name)", CL_INVALID_PROPERTY, iRet);

    clCreateContextFromType(badProps3, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &iRet);
    bResult &= Check(L"Create Context from type (unsupported property name", CL_INVALID_PROPERTY, iRet);

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		return false;
	}

	pDevices = new cl_device_id[uiNumDevices];
	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, uiNumDevices, pDevices, NULL);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		return false;
	}

	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		printf("clCreateContext = %ws\n",ClErrTxt(iRet));
		return false;
	}
	printf("context = %p\n", context);

	cl_context context2 = clCreateContextFromType(prop, CL_DEVICE_TYPE_CPU, NULL, NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		printf("clCreateContextFromType = %ws\n",ClErrTxt(iRet));
		return false;
	}
	printf("context2 = %p\n", context2);

	// Query for number of devices in a context, should be 1
	cl_uint uiCntxCnt = 0;
	iRet = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &uiCntxCnt, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetContextInfo = %ws\n",ClErrTxt(iRet));
		delete []pDevices;
		return false;
	}
	printf("Number of devices in a context is %d\n", (int)uiCntxCnt);

	cl_uint uiCntxRefCnt = 0;
	iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &uiCntxRefCnt, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetContextInfo = %ws\n",ClErrTxt(iRet));
		delete []pDevices;
		return false;
	}
	printf("context ref count = %d\n",uiCntxRefCnt);
	iRet = clRetainContext(context);
	if (CL_SUCCESS != iRet)
	{
		printf("clRetainContext = %ws\n",ClErrTxt(iRet));
		return false;
	}
	iRet = clGetContextInfo(context, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &uiCntxRefCnt, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetContextInfo = %ws\n",ClErrTxt(iRet));
		return false;
	}
	printf("context ref count (after retain) = %d\n",uiCntxRefCnt);

    // Release contexts
    for(cl_uint ui = 0; ui < uiCntxRefCnt; ui++)
    {
        clReleaseContext(context);
    }
    clReleaseContext(context2);
    clReleaseContext(context_default);
	delete []pDevices;
	return bResult;
}
