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

extern cl_device_type gDeviceType;

bool clBuildProgramTwiceTest()
{
    bool bResult = true;
    const char *ocl_test_program[] = {\
    "__kernel void test_kernel(__global char16* pBuff0, __global char* pBuff1, __global char* pBuff2, image2d_t __read_only test_image)"\
    "{"\
    "    size_t id = get_global_id(0);"\
    "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"\
    "}"
    };

    printf("clBuildProgramTwiceTest\n");

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

    cl_program clProg = clCreateProgramWithSource(context, 1, ocl_test_program, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateProgramWithSource = %s\n",ClErrTxt(iRet));
        clReleaseContext(context);

        return false;
    }
    printf("program id = %p\n", (void*)clProg);

    iRet = clBuildProgram(clProg, uiNumDevices, &devices[0], "-cl-denorms-are-zero", NULL, NULL);
    if (((CL_SUCCESS != iRet) && (CL_TRUE == isImagesSupported)) || ((CL_BUILD_PROGRAM_FAILURE != iRet) && (CL_FALSE == isImagesSupported)))
    {
        printf("first clBuildProgram = %s\n",ClErrTxt(iRet));
        clReleaseContext(context);
        clReleaseProgram(clProg);

        return false;
    }

    iRet = clBuildProgram(clProg, uiNumDevices, &devices[0], "-cl-denorms-are-zero", NULL, NULL);
    if (((CL_SUCCESS != iRet) && (CL_TRUE == isImagesSupported)) || ((CL_BUILD_PROGRAM_FAILURE != iRet) && (CL_FALSE == isImagesSupported)))
    {
        printf("second clBuildProgram = %s\n",ClErrTxt(iRet));
        clReleaseContext(context);
        clReleaseProgram(clProg);

        return false;
    }

    // Release objects
    clReleaseProgram(clProg);
    clReleaseContext(context);
    return bResult;
}
