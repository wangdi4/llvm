#include "CL/cl.h"
#include "cl_types.h"
#include "cl_cpu_detect.h"
#include "cl_config.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include <memory>
#include <stdlib.h>

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
GenerateBinaryFile
--------------------
generate and save invalid binary file (for invalid CPU architecture)
to load it in clCheckCPUArchForJIT test
GenerateBinaryFile before clCheckCPUArchForJIT
**************************************************************************************************/

bool GenerateBinaryFile()
{
    bool bResult = true;

    const char *ocl_test_program[] = { \
        "__kernel void test_kernel(__global int* pBuff0, __global int* pBuff1, __global int* pBuff2)"\
        "{"\
        "    size_t id = get_global_id(0);"\
        "    pBuff0[id] = pBuff1[id] ? 5 : pBuff2[id];"\
        "    float cosAngle, sinAngle;"\
        "    cosAngle = cos(convert_float(pBuff1[id]));"\
        "    sinAngle = sin(convert_float(pBuff2[id]));"\
        "}"\
    };

    char * filename;
    printf("GenerateBinaryFile\n");

    if (!CPUDetect::GetInstance()->IsFeatureSupported(CFS_AVX10))
    {
        if (!SETENV("VOLCANO_CPU_ARCH", "corei7-avx"))
        {
            printf("ERROR: GenerateBinaryFile: Can't set environment variable. Test FAILED\n");
            return false;
        }
        filename = "avx1.bin";
    }
    else
    {
        if (!SETENV("VOLCANO_CPU_ARCH", "corei7"))
        {
            printf("ERROR: GenerateBinaryFile: Can't set environment variable. Test FAILED\n");
            return false;
        }
        filename = "sse4.bin";
    }

    cl_platform_id platform = 0;
    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);
    if (!bResult)
    {
        return bResult;
    }

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    cl_device_id device;
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    // create context
    cl_context context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateContext = %s\n", ClErrTxt(iRet));
        return false;
    }
    printf("context = %p\n", context);

    cl_program clProg;
    bResult &= BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, "-cl-denorms-are-zero", &clProg);
    if (!bResult)
    {
        clReleaseProgram(clProg);
        clReleaseContext(context);
        return bResult;
    }

    size_t binarySize = 0;
    char * pBinaries = NULL;
    if (bResult)
    {
        // get the binary
        iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &binarySize, NULL);
        bResult &= Check(L"clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
        if (bResult)
        {
            pBinaries = new char[binarySize];
            iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, binarySize, &pBinaries, NULL);
            bResult &= Check(L"clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);
            if (bResult)
            {
                FILE * fout;
                fout = fopen(filename, "wb");
                if (NULL == fout)
                {
                    printf("Failed open file.\n");
                    clReleaseProgram(clProg);
                    clReleaseContext(context);
                    return false;
                }
                fwrite(pBinaries, 1, binarySize, fout);
                fclose(fout);
                printf("Saved successfully!! [size = %d] \n", binarySize);
            }
        }
    }

    clReleaseProgram(clProg);
    clReleaseContext(context);

    if (!UNSETENV("VOLCANO_CPU_ARCH")) {
        printf("ERROR GenerateBinaryFile: Can't unset VOLCANO_CPU_ARCH environment variable. Test FAILED\n");
        return false;
    }

    return bResult;
}