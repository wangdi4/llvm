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

bool clKernelAttributesTest()
{
	bool bResult = true;
	const char *sample_attributes_kernel[] = {
		"__kernel __attribute__((reqd_work_group_size(2, 3, 4))) void sample_test_reqrd(__global long *result)\n"
		"{\n"
		"result[get_global_id(0)] = 0;\n"
		"}\n"
		"__kernel void sample_test_prefered(__global long *result)\n"
		"{\n"
		"result[get_global_id(0)] = 0;\n"
		"}\n"
	};

	printf("clKernelAttributesTest\n");
	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	cl_context context;
	cl_program prog;
	cl_kernel kernel;

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
	if (CL_SUCCESS != iRet)
	{
		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		return false;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		return false;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	bResult &= Check(L"clCreateContext", CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		return false;
	}

	prog = clCreateProgramWithSource(context, 1, (const char**)&sample_attributes_kernel, NULL, &iRet);
	bResult &= Check(L"clCreateProgramWithSource", CL_SUCCESS, iRet);
	if (!bResult)
	{
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}

	iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
	bResult &= Check(L"clBuildProgram", CL_SUCCESS, iRet);
	if (!bResult)
	{
		clReleaseProgram(prog);
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}

	kernel = clCreateKernel(prog, "sample_test_reqrd", &iRet);
	if ( CL_FAILED(iRet))
	{
		clReleaseProgram(prog);
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}

	// Get kernel extended attributes
	size_t wgSizeInfo[3];
	iRet = clGetKernelWorkGroupInfo(kernel, pDevices[0], CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(wgSizeInfo), &wgSizeInfo, NULL);
	if ( CL_FAILED(iRet))
	{
		clReleaseKernel(kernel);
		clReleaseProgram(prog);
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}

	bool bRes = (wgSizeInfo[0] == 2) && (wgSizeInfo[1] == 3) && (wgSizeInfo[2] == 4);

	size_t wgMaxSize = 0;
	iRet = clGetKernelWorkGroupInfo(kernel, pDevices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgMaxSize, NULL);
	if ( CL_FAILED(iRet))
	{
		clReleaseKernel(kernel);
		clReleaseProgram(prog);
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}

	iRet = clGetKernelWorkGroupInfo(kernel, pDevices[0], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &wgMaxSize, NULL);
	if ( CL_FAILED(iRet))
	{
		clReleaseKernel(kernel);
		clReleaseProgram(prog);
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}
	bRes &= (wgSizeInfo[0]*wgSizeInfo[1]*wgSizeInfo[2] == wgMaxSize);

	cl_ulong ulPrSize;
	iRet = clGetKernelWorkGroupInfo(kernel, pDevices[0], CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(cl_ulong), &ulPrSize, NULL);
	if ( CL_FAILED(iRet))
	{
		clReleaseKernel(kernel);
		clReleaseProgram(prog);
		clReleaseContext(context);
		delete []pDevices;
		return false;
	}
	bRes &= ( 0 != ulPrSize );

	clReleaseKernel(kernel);
	clReleaseProgram(prog);
	clReleaseContext(context);
	delete []pDevices;

	return bRes;
}
