#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include "cl_device_api.h"
#ifdef _WIN32
#include <windows.h>
#endif
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/**************************************************************************************************
* clBuildProgram
* -------------------
* (1) get device ids
* (2) create context
* (3) create binary
* (4) create program with source
* (5) build program
**************************************************************************************************/

bool clBuildProgramWithSourceTest()
{
	bool bResult = true;
	const char *ocl_test_program[] = {\
	"__kernel void test_kernel(__global char16* pBuff0, __global char* pBuff1, __global char* pBuff2, image2d_t __read_only test_image)"\
	"{"\
	"	size_t id = get_global_id(0);"\
	"	pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"\
	"}"
	};

	printf("clBuildProgramFromSourcesTest\n");
	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	size_t * pBinarySizes;
	cl_int * pBinaryStatus; 
	cl_context context;
	cl_program clProg;

	cl_platform_id platform = 0;

	cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	// get device(s)
	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &uiNumDevices);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		return false;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];
	pBinarySizes = new size_t[uiNumDevices];
	pBinaryStatus = new cl_int[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, uiNumDevices, pDevices, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return false;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContext = %ws\n",ClErrTxt(iRet));
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return false;
	}
	printf("context = %p\n", context);


	bResult &= BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, "-cl-denorms-are-zero", &clProg);
	if (!bResult)
	{
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return bResult;
	}


	if (bResult)
	{
		size_t szSize = 0;
		// get the binary
		iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &szSize, NULL);
		bResult &= Check(L"clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
		if (bResult)
		{
			char * pBinaries = new char[szSize];
			iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, szSize, &pBinaries, NULL);
			bResult &= Check(L"clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);
#if __STORE_BINARY__
			if (bResult)
			{
				FILE * fout;
				fout = fopen("C:\\dot.bin", "wb");
				fwrite(pBinaries, 1, szSize, fout);
				fclose(fout);
			}
#endif
			delete []pBinaries;
		}

		// CSSD100011901
		cl_kernel kern = clCreateKernel(clProg, "test_kernel", &iRet);
		bResult = SilentCheck(L"clCreateKernel", CL_SUCCESS, iRet);
		if ( bResult )
		{
			iRet = clSetKernelArg(kern, 2, sizeof(cl_mem), NULL);
			bResult &= Check(L"clSetKernelArg()", CL_SUCCESS, iRet);
			iRet = clSetKernelArg(kern, 3, sizeof(cl_mem), NULL);
			bResult &= Check(L"clSetKernelArg(C)", CL_INVALID_ARG_VALUE, iRet);
			clReleaseKernel(kern);
		}
	}

    // Release objects
	delete []pDevices;
	delete []pBinarySizes;
	delete []pBinaryStatus;
    clReleaseProgram(clProg);
    clReleaseContext(context);
	return bResult;
}
