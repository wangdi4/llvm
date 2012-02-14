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
* clLinkProgram
* -------------------
* (1) get device ids
* (2) create context
* (3) create binary
* (4) create program with binary
* (5) build program
**************************************************************************************************/

bool clLinkProgramTest()
{
	bool bResult = true;

    const char *ocl_test_program = "\
	__kernel void test_kernel1(__global int* pIn1, __global int* pIn2, __global int* pSelect, __global int* pOut)\n\
	{\n\
		size_t id = get_global_id(0);\n\
		pOut[id] = pSelect[id] ? pIn1[id] : pIn2[id];\n\
	}\n\
    \n\
    __kernel void test_kernel2(__global int* pIn1, __global int* pIn2, __global int* pSelect, __global int* pOut)\n\
	{\n\
		size_t id = get_global_id(0);\n\
		pOut[id] = pSelect[id] ? pIn1[id] : pIn2[id];\n\
	}\n\
    \n\
    __kernel void test_kernel3(__global int* pIn1, __global int* pIn2, __global int* pSelect, __global int* pOut)\n\
	{\n\
		size_t id = get_global_id(0);\n\
		pOut[id] = pSelect[id] ? pIn1[id] : pIn2[id];\n\
	}\n\
    \n\
    ";

	printf("---------------------------------------\n");
	printf("clLinkProgramTest\n");
	printf("---------------------------------------\n");
	cl_uint uiNumDevices = 0;
	cl_device_id * pDevices;
	size_t * pBinarySizes;
    char** ppBinaries;
	cl_int * pBinaryStatus; 
	cl_context context;
    cl_program programOrig;
    cl_program programFromBinary;

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
	pBinarySizes = new size_t[uiNumDevices];
    ppBinaries = new char*[uiNumDevices];
	pBinaryStatus = new cl_int[uiNumDevices];

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clGetDeviceIDs = %ws\n",ClErrTxt(iRet));
		return false;
	}

	// create context
	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContext = %ws\n",ClErrTxt(iRet));
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;
		return false;
	}
	printf("context = %p\n", context);

    const char* ssProgram[] = {ocl_test_program};
    size_t lines[1];
    lines[0] = strlen(ocl_test_program);
    const char* invssProgram[] = { NULL, NULL, NULL, NULL }; 
    size_t nullines[] = {0, 0, 0, 0};
    cl_program prog;

    //Opencl-1.1 specification says: "CL_INVALID_VALUE if count is zero or if strings or any entry in strings is NULL" and "If lengths is NULL, all strings in the strings argument are considered null-terminated."
         //clCreateProgramWithSource(context,   num_lines,  sources,    lengths,    ret)
    prog = clCreateProgramWithSource(context,   NULL,       ssProgram,  lines,      &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }
    prog = clCreateProgramWithSource(context,   0,          ssProgram,  lines,      &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }
    prog = clCreateProgramWithSource(context,   NULL,       NULL,       NULL,       &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }
    prog = clCreateProgramWithSource(context,   1,           NULL,      lines,      &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }
    prog = clCreateProgramWithSource(context,   4,          invssProgram,   lines,  &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }
    prog = clCreateProgramWithSource(context,   4,          invssProgram,   nullines,   &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }
    prog = clCreateProgramWithSource(context,   4,          invssProgram,  NULL,        &iRet);
    if (CL_SUCCESS == iRet)
    {
        if (prog)
        {
            clReleaseProgram(prog);
        }
    }

    // create program with source
    programOrig = clCreateProgramWithSource (context, 1, (const char**)&ocl_test_program,NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clCreateProgramWithSource = %ws\n",ClErrTxt(iRet));
		return false;
	}

    //build the program
    iRet = clBuildProgram (programOrig, uiNumDevices, pDevices, NULL, NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clBuildProgram = %ws\n",ClErrTxt(iRet));
		return false;
	}

    size_t uiNumKernels = 0;
    iRet = clGetProgramInfo(programOrig, CL_PROGRAM_NUM_KERNELS, sizeof(size_t), &uiNumKernels, NULL);

    size_t uiKernelNamesLength = 0;
    char* szKernelNames = NULL;
    iRet = clGetProgramInfo(programOrig, CL_PROGRAM_KERNEL_NAMES, NULL, NULL, &uiKernelNamesLength);

    szKernelNames = new char[uiKernelNamesLength];
    iRet = clGetProgramInfo(programOrig, CL_PROGRAM_KERNEL_NAMES, uiKernelNamesLength, szKernelNames, &uiKernelNamesLength);

    //retrieve the binary
    iRet = clGetProgramInfo(programOrig, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * uiNumDevices, (void*)pBinarySizes, NULL);
    if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clGetProgramInfo = %ws\n",ClErrTxt(iRet));
		return false;
	}

    int totalSize = 0;
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        ppBinaries[i] = new char[pBinarySizes[i]];
        totalSize += pBinarySizes[i];
    }

    iRet = clGetProgramInfo(programOrig, CL_PROGRAM_BINARIES, totalSize, (void*)ppBinaries, NULL);
    if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clGetProgramInfo = %ws\n",ClErrTxt(iRet));
		return false;
	}

    //dump binary to file
    FILE *f = fopen("D:\\\\out.bc", "wb");
    fwrite(ppBinaries[0] + sizeof(cl_prog_container_header) + 4, 1, pBinarySizes[0], f);
    fflush(f);
    fclose(f);

	// create program with binary
	programFromBinary = clCreateProgramWithBinary(context, uiNumDevices, pDevices, pBinarySizes, (const unsigned char**)ppBinaries, pBinaryStatus, &iRet);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clCreateProgramWithBinary = %ws\n",ClErrTxt(iRet));
		return false;
	}

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (pBinaryStatus[i] != CL_SUCCESS)
        {
            delete []pDevices;
		    delete []pBinarySizes;
            delete []ppBinaries;
		    delete []pBinaryStatus;

		    printf("pBinaryStatus[%d] = %ws\n",i, ClErrTxt(iRet));
		    return false;
        }
    }

    // build program from binary
	iRet = clBuildProgram(programFromBinary, uiNumDevices, pDevices, NULL, NULL, NULL);
	if (CL_SUCCESS != iRet)
	{
		delete []pDevices;
		delete []pBinarySizes;
        delete []ppBinaries;
		delete []pBinaryStatus;

		printf("clBuildProgram from binary = %ws\n",ClErrTxt(iRet));
		return false;
	}

	cl_build_status clBuildStatus = CL_BUILD_NONE;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		iRet = clGetProgramBuildInfo(programFromBinary, pDevices[ui],CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &clBuildStatus, NULL);
		bResult &= Check(L"clGetProgramBuildInfo(CL_PROGRAM_BUILD_STATUS)", CL_SUCCESS, iRet);
		bResult &= CheckInt(L"check status", (int)clBuildStatus, CL_BUILD_SUCCESS);
	}

	delete []pDevices;
	delete []pBinarySizes;
    delete []ppBinaries;
	delete []pBinaryStatus;
	clReleaseProgram(programOrig);
    clReleaseProgram(programFromBinary);
    clReleaseContext(context);
	return bResult;
}
