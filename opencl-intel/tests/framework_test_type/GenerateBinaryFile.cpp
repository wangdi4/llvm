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

    // check CPU architecture
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

    cl_uint uiNumDevices = 0;
    // get device(s)
    iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    std::vector<cl_device_id> devices(uiNumDevices);
    iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, &devices[0], NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    // create context
    cl_context context = clCreateContext(prop, uiNumDevices, &devices[0], NULL, NULL, &iRet);
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
        clReleaseContext(context);
        return bResult;
    }

    std::vector<size_t> binarySizes(uiNumDevices);
    char ** pBinaries = NULL;
    if (bResult)
    {
        // get the binary
        iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t)* uiNumDevices, &binarySizes[0], NULL);
        bResult &= Check(L"clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
        if (bResult)
        {
            size_t sumBinariesSize = 0;
            pBinaries = new char*[uiNumDevices];
            for (unsigned int i = 0; i < uiNumDevices; ++i)
            {
                pBinaries[i] = new char[binarySizes[i]];
                sumBinariesSize += binarySizes[i];
            }
            iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, sumBinariesSize, pBinaries, NULL);
            bResult &= Check(L"clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);
            if (bResult)
            {
                FILE * fout;
                fout = fopen(filename, "wb");
                fwrite(pBinaries[0], 1, binarySizes[0], fout);
                fclose(fout);
                printf("Saved successfully!! [size = %d] \n", sumBinariesSize);
            }
        }
    }
    return bResult;

    if (!UNSETENV("VOLCANO_CPU_ARCH")) {
        printf("ERROR GenerateBinaryFile: Can't unset VOLCANO_CPU_ARCH environment variable. Test FAILED\n");
        return false;
    }

    return bResult;
}