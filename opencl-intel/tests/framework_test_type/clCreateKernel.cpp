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
* clCreateKernel
* -------------------
* (1) get device ids
* (2) create context
* (3) create binary
* (4) create program with binary
* (5) build program
* (6) create context
**************************************************************************************************/

bool g_bBuildFinished = false;

static void CL_CALLBACK pfn_notify(cl_program program, void *user_data)
{
	g_bBuildFinished = true;
}

bool clCreateKernelTest()
{
	printf("---------------------------------------\n");
	printf("clCreateKernel\n");
	printf("---------------------------------------\n");
	bool bResult = true;

	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	size_t * pBinarySizes;
	cl_int * pBinaryStatus; 
	cl_context context;

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
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		return bResult;
	}

	// initialize arrays
	pDevices = new cl_device_id[uiNumDevices];
	pBinarySizes = new size_t[uiNumDevices];
	pBinaryStatus = new cl_int[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);
	bResult &= Check(L"clGetDeviceIDs",CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return bResult;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	bResult &= Check(L"clCreateContext",CL_SUCCESS, iRet);
	if (!bResult)
	{
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return bResult;
	}
	printf("context = %p\n", context);

	// create binary container
	unsigned int uiContSize = sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header);
	FILE* pIRfile = NULL;
	FOPEN(pIRfile, "test.bc", "rb");
	fpos_t fileSize;
	SET_FPOS_T(fileSize, 0);
	if ( NULL == pIRfile )
	{
		printf("Failed open file.\n");
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return false;
	}
	fseek(pIRfile, 0, SEEK_END);
	fgetpos(pIRfile, &fileSize);
	uiContSize += (unsigned int)GET_FPOS_T(fileSize);
	fseek(pIRfile, 0, SEEK_SET);

	cl_prog_container_header* pCont = (cl_prog_container_header*)malloc(uiContSize);
	if ( NULL == pCont )
	{
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		return false;
	}
	// Construct program container
	memset(pCont, 0, sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header));
	// Container mask
	memcpy((void*)pCont->mask, _CL_CONTAINER_MASK_, sizeof(pCont->mask));

	pCont->container_type = CL_PROG_CNT_PRIVATE;
	pCont->description.bin_type = CL_PROG_BIN_EXECUTABLE_LLVM;
	pCont->description.bin_ver_major = 1;
	pCont->description.bin_ver_minor = 1;
	pCont->container_size = (unsigned int)GET_FPOS_T(fileSize)+sizeof(cl_llvm_prog_header);
	fread(((unsigned char*)pCont)+sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header), 1, (size_t)GET_FPOS_T(fileSize), pIRfile);
	fclose(pIRfile);

	pBinarySizes[0] = uiContSize;

	// create program with binary
	cl_program program  = clCreateProgramWithBinary(context, uiNumDevices, pDevices, pBinarySizes,  (const unsigned char**)(&pCont), pBinaryStatus, &iRet);
	bResult &= Check(L"clCreateProgramWithBinary", CL_SUCCESS, iRet);

	if (!bResult)
	{
		delete []pDevices;
		delete []pBinarySizes;
		delete []pBinaryStatus;
		free(pCont);
		return bResult;
	}

	iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, pfn_notify, NULL);
	bResult &= Check(L"clBuildProgram", CL_SUCCESS, iRet);

	while (!g_bBuildFinished)
	{
		SLEEP(1);
	}

	cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
	bResult &= Check(L"clCreateKernel - dot_product", CL_SUCCESS, iRet);

	//cl_kernel kernel2 = clCreateKernel(program, "dot_product_test", &iRet);
	//bResult &= Check(L"clCreateKernel - dot_product_test", CL_SUCCESS, iRet);

	//cl_kernel kernel3 = clCreateKernel(program, "foo", &iRet);
	//bResult &= Check(L"clCreateKernel - foo", CL_SUCCESS, iRet);

	//cl_uint uiNumKernels = 0;
	//iRet = clCreateKernelsInProgram(program, 0, NULL, &uiNumKernels);
	//bResult &= Check(L"clCreateKernelsInProgram - get numbers of kernels", CL_SUCCESS, iRet);
	//bResult &= CheckInt(L"clCreateKernelsInProgram - check numbers kernels", 14, uiNumKernels);

	//cl_kernel pKernels[14];
	//iRet = clCreateKernelsInProgram(program, uiNumKernels, pKernels, NULL);
	//bResult &= Check(L"clCreateKernelsInProgram - get kernels", CL_SUCCESS, iRet);
	//if (bResult)
	//{
	//	size_t szKernelNameLength = 0;
	//	char psKernelName[256] = {0};
	//	printf("Print kernels: ");
	//	for (cl_uint ui=0; ui<uiNumKernels; ++ui)
	//	{
	//		printf("%d: ",pKernels[ui]);
	//		iRet = clGetKernelInfo(pKernels[ui], CL_KERNEL_FUNCTION_NAME, 256, psKernelName, NULL);
	//		bResult &= Check(L"clGetKernelInfo (function's name)", CL_SUCCESS, iRet);
	//		printf ("%s\n", psKernelName);
	//	}
	//	printf("\n");
	//}

    // Release object
    clReleaseKernel(kernel1);
    clReleaseProgram(program);
    clReleaseContext(context);
	delete []pDevices;
	delete []pBinarySizes;
	delete []pBinaryStatus;
	free(pCont);
	return bResult;
}
