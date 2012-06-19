#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#ifdef _WIN32
#include <windows.h>
#endif

/**************************************************************************************************
* clBuildProgram
* -------------------
* (1) get device ids
* (2) create context
* (3) create program with source
* (4) build program
* (5) build program again
**************************************************************************************************/

bool clBuildProgramTwiceTest()
{
	bool bResult = true;
	const char *ocl_test_program[] = {\
	"__kernel void test_kernel(__global char16* pBuff0, __global char* pBuff1, __global char* pBuff2, image2d_t __read_only test_image)"\
	"{"\
	"	size_t id = get_global_id(0);"\
	"	pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"\
	"}"
	};

	printf("clBuildProgramTwiceTest\n");
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
		printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
		return false;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];
	pBinarySizes = new size_t[uiNumDevices];
	pBinaryStatus = new cl_int[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, uiNumDevices, pDevices, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return false;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContext = %s\n",ClErrTxt(iRet));
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return false;
	}
	printf("context = %p\n", context);

    clProg = clCreateProgramWithSource(context, 1, ocl_test_program, NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateProgramWithSource = %s\n",ClErrTxt(iRet));
        delete []pDevices;
	    delete []pBinarySizes;
	    delete []pBinaryStatus;
        clReleaseContext(context);

		return false;
	}
	printf("program id = %p\n", clProg);

	iRet = clBuildProgram(clProg, uiNumDevices, pDevices, "-cl-denorms-are-zero", NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
		printf("first clBuildProgram = %s\n",ClErrTxt(iRet));
        delete []pDevices;
	    delete []pBinarySizes;
	    delete []pBinaryStatus;
        clReleaseContext(context);
        clReleaseProgram(clProg);

		return false;
	}

    iRet = clBuildProgram(clProg, uiNumDevices, pDevices, "-cl-denorms-are-zero", NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
		printf("second clBuildProgram = %s\n",ClErrTxt(iRet));
        delete []pDevices;
	    delete []pBinarySizes;
	    delete []pBinaryStatus;
        clReleaseContext(context);
        clReleaseProgram(clProg);

		return false;
	}

    // Release objects
	delete []pDevices;
	delete []pBinarySizes;
	delete []pBinaryStatus;
    clReleaseProgram(clProg);
    clReleaseContext(context);
	return bResult;
}
