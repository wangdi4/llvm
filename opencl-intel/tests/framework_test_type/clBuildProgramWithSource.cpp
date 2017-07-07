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
* (3) create binary
* (4) create program with source
* (5) build program
**************************************************************************************************/

extern cl_device_type gDeviceType;

bool clBuildProgramWithSourceTest()
{
    bool bResult = true;
    const char *ocl_test_program[] = {\
    "__kernel void test_kernel(__global char16* pBuff0, __global char* pBuff1, __global char* pBuff2, image2d_t __read_only test_image)"\
    "{"\
    "    size_t id = get_global_id(0);"\
    "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"\
    "}"
    };

    printf("clBuildProgramFromSourcesTest\n");

    cl_platform_id platform = 0;
    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);
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
        printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
        return false;
    }

    std::vector<cl_device_id> devices(uiNumDevices);
    iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, &devices[0], NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
        return false;
    }

    // check if all devices support images
    cl_bool isImagesSupported = CL_TRUE;
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        iRet = clGetDeviceInfo(devices[i], CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &isImagesSupported, NULL);
        bResult = Check("clGetDeviceInfo(CL_DEVICE_IMAGE_SUPPORT)", CL_SUCCESS, iRet);
        if (!bResult)
        {
            return bResult;
        }
        // We build program on all the devices, so build is expected to fail
        // if at least one of them doesn't support images.
        if (isImagesSupported == CL_FALSE) break;
    }

    // create context
    cl_context context = clCreateContext(prop, uiNumDevices, &devices[0], NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateContext = %s\n",ClErrTxt(iRet));
        return false;
    }
    printf("context = %p\n", (void*)context);

    cl_program clProg;
    bResult &= BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, "-cl-denorms-are-zero", &clProg);
    if (!bResult)
    {
        clReleaseContext(context);
        return bResult || isImagesSupported == CL_FALSE;
    }


    if (bResult)
    {
        std::vector<size_t> binarySizes(uiNumDevices);
        // get the binary
        iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * uiNumDevices, &binarySizes[0], NULL);
        bResult &= Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
        if (bResult)
        {
            size_t sumBinariesSize = 0;
            char ** pBinaries = new char*[uiNumDevices];
            for (unsigned int i = 0; i < uiNumDevices; ++i)
            {
                pBinaries[i] = new char[binarySizes[i]];
                sumBinariesSize += binarySizes[i];
            }
            iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, sumBinariesSize, pBinaries, NULL);
            bResult &= Check("clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);
#if __STORE_BINARY__
            if (bResult)
            {
                FILE * fout;
                fout = fopen("C:\\dot.bin", "wb");
                fwrite(pBinaries, 1, sumBinariesSize, fout);
                fclose(fout);
            }
#endif
            for (unsigned int i = 0; i < uiNumDevices; i++)
            {
                delete [](pBinaries[i]);
            }
            delete []pBinaries;
        }

        // CSSD100011901
        cl_kernel kern = clCreateKernel(clProg, "test_kernel", &iRet);
        bResult = SilentCheck("clCreateKernel", CL_SUCCESS, iRet);
        if ( bResult )
        {
            iRet = clSetKernelArg(kern, 2, sizeof(cl_mem), NULL);
            bResult &= Check("clSetKernelArg()", CL_SUCCESS, iRet);
            iRet = clSetKernelArg(kern, 3, sizeof(cl_mem), NULL);
            bResult &= Check("clSetKernelArg(C)", CL_INVALID_ARG_VALUE, iRet);
            clReleaseKernel(kern);
        }
    }

    // Release objects
    clReleaseProgram(clProg);
    clReleaseContext(context);
    return bResult;
}
