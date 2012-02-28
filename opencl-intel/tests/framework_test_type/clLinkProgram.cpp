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

#define BUFFER_SIZE 100

bool clLinkProgramTest()
{
	bool bResult = true;

    const char *ocl_test_program_1 = "\
    #include \"help1.h\"\n\
    __kernel void test_kernel1(__global int* pBuff)\n\
    {\n\
        size_t id = get_global_id(0);\n\
        pBuff[id] = helper1() + helper2();\n\
    }\n\
    ";

    const char *ocl_test_help_1 = "\
    #include \"help2.h\"\n\
    \n\
    int helper1();\n\
    ";

    const char *ocl_test_help_2 = "\
    int helper2();\n\
    ";

    const char *ocl_test_lib_1 = "\
	int helper1()\n\
    {\n\
        return 40;\n\
    }\n\
    ";

    const char *ocl_test_lib_2 = "\
    int helper2()\n\
    {\n\
        return 2;\n\
    }\n\
    ";

	printf("---------------------------------------\n");
	printf("clLinkProgramTest\n");
	printf("---------------------------------------\n");
    
	cl_platform_id      platform = 0;
    cl_context          context = 0;
    cl_command_queue    queue = 0;

    cl_uint             uiNumDevices = 0;
	cl_device_id*       pDevices;

    cl_program clHeaders[2];
    cl_program clLibs[2];

    cl_program clTestBinary;
    cl_program clTestLib;

    cl_program clTestProg;

    const char* pszIncludeNames[2] = {"help1.h", "help2.h"};

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
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContext = %ws\n",ClErrTxt(iRet));
		delete []pDevices;
		return false;
	}
	printf("context = %p\n", context);

    queue = clCreateCommandQueue(context, pDevices[0], 0, &iRet);
	if (CL_SUCCESS != iRet)
	{
        clReleaseContext(context);

        delete []pDevices;

		printf("clCreateCommandQueue = %ws\n",ClErrTxt(iRet));
		return false;
	}

    // create program with source
    clTestBinary = clCreateProgramWithSource (context, 1, (const char**)&ocl_test_program_1, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        
		delete []pDevices;

		printf("clCreateProgramWithSource = %ws\n",ClErrTxt(iRet));
		return false;
	}

    clHeaders[0] = clCreateProgramWithSource (context, 1, (const char**)&ocl_test_help_1, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateProgramWithSource = %ws\n",ClErrTxt(iRet));
		return false;
	}

    clHeaders[1] = clCreateProgramWithSource (context, 1, (const char**)&ocl_test_help_2, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateProgramWithSource = %ws\n",ClErrTxt(iRet));
		return false;
	}

    iRet = clCompileProgram(clTestBinary, uiNumDevices, pDevices, NULL, 2, clHeaders, pszIncludeNames, NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCompileProgram = %ws\n",ClErrTxt(iRet));
		return false;
	}

    clLibs[0] = clCreateProgramWithSource (context, 1, (const char**)&ocl_test_lib_1, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateProgramWithSource = %ws\n",ClErrTxt(iRet));
		return false;
	}

    clLibs[1] = clCreateProgramWithSource (context, 1, (const char**)&ocl_test_lib_2, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateProgramWithSource = %ws\n",ClErrTxt(iRet));
		return false;
	}

    iRet = clCompileProgram(clLibs[0], uiNumDevices, pDevices, NULL, 0, NULL, NULL, NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCompileProgram = %ws\n",ClErrTxt(iRet));
		return false;
	}

    iRet = clCompileProgram(clLibs[1], uiNumDevices, pDevices, NULL, 0, NULL, NULL, NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCompileProgram = %ws\n",ClErrTxt(iRet));
		return false;
	}

    clTestLib = clLinkProgram(context, uiNumDevices, pDevices, "-create-library", 2, clLibs, NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCompileProgram = %ws\n",ClErrTxt(iRet));
		return false;
	}

    cl_program clLibBin[2];
    clLibBin[0] = clTestLib;
    clLibBin[1] = clTestBinary;

    clTestProg = clLinkProgram(context, uiNumDevices, pDevices, NULL, 2, clLibBin, NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);
        clReleaseProgram(clTestLib);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCompileProgram = %ws\n",ClErrTxt(iRet));
		return false;
	}

    cl_kernel kernel = clCreateKernel(clTestProg, "test_kernel1", &iRet);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);
        clReleaseProgram(clTestLib);
        clReleaseProgram(clTestProg);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateKernel = %ws\n",ClErrTxt(iRet));
		return false;
	}

    size_t uiBuffSize = BUFFER_SIZE;
    cl_int pBuff[BUFFER_SIZE];

    cl_mem clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE, BUFFER_SIZE * sizeof(cl_int), NULL, &iRet);
	if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);
        clReleaseProgram(clTestLib);
        clReleaseProgram(clTestProg);

        clReleaseKernel(kernel);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateBuffer = %ws\n",ClErrTxt(iRet));
		return false;
	}
    
    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);
        clReleaseProgram(clTestLib);
        clReleaseProgram(clTestProg);

        clReleaseKernel(kernel);

        clReleaseMemObject(clBuff);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clCreateBuffer = %ws\n",ClErrTxt(iRet));
		return false;
	}

	size_t global_work_size[1] = { uiBuffSize };

	// Execute kernel
	iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);
        clReleaseProgram(clTestLib);
        clReleaseProgram(clTestProg);

        clReleaseKernel(kernel);

        clReleaseMemObject(clBuff);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clEnqueueNDRangeKernel = %ws\n",ClErrTxt(iRet));
		return false;
	}
   
	//
	// Verification phase
	//
	iRet = clEnqueueReadBuffer( queue, clBuff, CL_TRUE, 0, sizeof(pBuff), pBuff, 0, NULL, NULL );
    if (CL_SUCCESS != iRet)
	{
        clReleaseProgram(clHeaders[0]);
        clReleaseProgram(clHeaders[1]);
        clReleaseProgram(clLibs[0]);
        clReleaseProgram(clLibs[1]);
        clReleaseProgram(clTestBinary);
        clReleaseProgram(clTestLib);
        clReleaseProgram(clTestProg);

        clReleaseKernel(kernel);

        clReleaseMemObject(clBuff);

        clReleaseCommandQueue(queue);
        clReleaseContext(context);

		delete []pDevices;

		printf("clEnqueueReadBuffer = %ws\n",ClErrTxt(iRet));
		return false;
	}

    for (unsigned int i = 0; i < uiBuffSize; ++i)
    {
        if (42 != pBuff[i])
        {
            clReleaseProgram(clHeaders[0]);
            clReleaseProgram(clHeaders[1]);
            clReleaseProgram(clLibs[0]);
            clReleaseProgram(clLibs[1]);
            clReleaseProgram(clTestBinary);
            clReleaseProgram(clTestLib);
            clReleaseProgram(clTestProg);

            clReleaseKernel(kernel);

            clReleaseMemObject(clBuff);

            clReleaseCommandQueue(queue);
            clReleaseContext(context);

            delete []pDevices;

		    printf("varification failed = %ws\n",ClErrTxt(iRet));
		    return false;
        }
    }

	delete []pDevices;

    clReleaseProgram(clHeaders[0]);
    clReleaseProgram(clHeaders[1]);
    clReleaseProgram(clLibs[0]);
    clReleaseProgram(clLibs[1]);
    clReleaseProgram(clTestBinary);
    clReleaseProgram(clTestLib);
    clReleaseProgram(clTestProg);

    clReleaseKernel(kernel);

    clReleaseMemObject(clBuff);
    
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return bResult;
}
