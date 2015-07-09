#include "CL/cl.h"
#include "cl_types.h"
#include "cl_cpu_detect.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "FrameworkTest.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(_WIN32)
#define SETENV(NAME,VALUE)      (SetEnvironmentVariableA(NAME,VALUE) != 0)
#define UNSETENV(NAME)          (SetEnvironmentVariableA(NAME,NULL) != 0)
#else
#define SETENV(NAME,VALUE)      (setenv(NAME,VALUE,1) == 0)
#define UNSETENV(NAME)          (unsetenv(NAME) == 0)
#endif

using namespace Intel::OpenCL::Utils;

extern cl_device_type gDeviceType;

/**************************************************************************************************
  clCheckCPUArchForJIT
  --------------------
  load invalid binaries and try to create program with these binaries
  binary file must be saved before for invalid CPU architecture (by GenerateBinaryFile() test)
  Expect fail: clCreateProgramWithBinary
  **************************************************************************************************/

bool clCheckCPUArchForJIT() {
    if (gDeviceType != CL_DEVICE_TYPE_CPU) return true;

    if (!UNSETENV("VOLCANO_CPU_ARCH")) {
        printf("ERROR clCheckCPUArchForJIT: Can't unset VOLCANO_CPU_ARCH environment variable. Test FAILED\n");
        return false;
    }

    cl_context context;
    cl_device_id device;

    bool bResult = true;

    printf("clCheckCPUArchForJIT\n");

    cl_platform_id platform = 0;

    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetPlatformIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    // initialize
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    // create context
    context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateContext = %s\n", ClErrTxt(iRet));
        return false;
    }
    printf("Initialized context\n");

    unsigned int uiContSize = 0;

    // open binary file
    FILE* fout = NULL;
    if (!CPUDetect::GetInstance()->IsFeatureSupported(CFS_AVX10))
        fout = fopen("avx1.bin", "rb");
    else
        fout = fopen("sse4.bin", "rb");
    fpos_t fileSize;
    SET_FPOS_T(fileSize, 0);
    if (NULL == fout)
    {
        printf("Failed open file.\n");
        clReleaseContext(context);
        return false;
    }
    fseek(fout, 0, SEEK_END);
    fgetpos(fout, &fileSize);
    uiContSize += (unsigned int)GET_FPOS_T(fileSize);
    fseek(fout, 0, SEEK_SET);

    assert(uiContSize > 0 && "the input file must not be empty");
    unsigned char* pCont = (unsigned char*)malloc(uiContSize);
    if (NULL == pCont)
    {
        fclose(fout);
        clReleaseContext(context);
        return false;
    }
    // construct program container
    fread(((unsigned char*)pCont), 1, (size_t)GET_FPOS_T(fileSize), fout);
    fclose(fout);

    size_t binarySize = uiContSize;

    // create program with binary
    cl_int binaryStatus;
    cl_program program = clCreateProgramWithBinary(context, 1, &device, &binarySize, const_cast<const unsigned char**>(&pCont), &binaryStatus, &iRet);
    bResult &= Check(L"clCreateProgramWithBinary", CL_INVALID_BINARY, iRet);
    bResult &= Check(L"binaryStatus = CL_INVALID_BINARY", CL_INVALID_BINARY, binaryStatus);

    // Release objects
    clReleaseProgram(program);
    clReleaseContext(context);

    return bResult;
}