#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#ifdef _WIN32
#include <windows.h>
#endif

/**************************************************************************************************
* clGetKernelArgInfoTest
* -------------------
* (1) get device ids
* (2) create context
* (3) create programs with source
* (4) build programs
* (5) query and test kernel arg info
**************************************************************************************************/

extern cl_device_type gDeviceType;

bool clGetKernelArgInfoTest()
{
    bool bResult = true;
    const char *ocl_test_program[] = {\
    "__kernel void test_kernel1(__global char16 pBuff0[], __global char* pBuff1, __global const char* pBuff2, image2d_t __read_only test_image) "\
    "__attribute__((vec_type_hint(uint8))) __attribute__((reqd_work_group_size(8,8,8)))"\
    "{"\
    "    size_t id = get_global_id(0);"\
    "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"\
    "}"\
    ""\
    "__kernel void test_kernel2(__global int4* pBuff0, __global int* pBuff1, __global const volatile int* pBuff2)"\
    "{"\
    "    size_t id = get_global_id(0);"\
    "    pBuff0[id] = pBuff1[id] ? pBuff0[id] : pBuff2[id];"\
    "}"
    };

    printf("clGetKernelArgInfoTest\n");
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
    bResult &= BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, "-cl-kernel-arg-info", &clProg);
    if (!bResult)
    {
        clReleaseContext(context);
        return bResult || isImagesSupported == CL_FALSE;
    }

    cl_kernel clKernel1 = clCreateKernel (clProg, "test_kernel1", &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateKernel = %s\n",ClErrTxt(iRet));

        clReleaseContext(context);
        clReleaseProgram(clProg);

        return false;
    }

    cl_kernel clKernel2 = clCreateKernel (clProg, "test_kernel2", &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateKernel = %s\n",ClErrTxt(iRet));

        clReleaseContext(context);
        clReleaseProgram(clProg);
        clReleaseKernel(clKernel1);

        return false;
    }

    cl_kernel_arg_address_qualifier adressQualifier;
    cl_kernel_arg_access_qualifier accessQualifier;
    cl_kernel_arg_type_qualifier typeQualifier;

    iRet |= clGetKernelArgInfo (clKernel1, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
        sizeof(cl_kernel_arg_address_qualifier), &adressQualifier, NULL);

    if (CL_KERNEL_ARG_ADDRESS_GLOBAL != adressQualifier)
    {
        iRet = -1;
    }

    char szTypeName[255];
    iRet |= clGetKernelArgInfo (clKernel1, 0, CL_KERNEL_ARG_TYPE_NAME,
        sizeof(szTypeName), szTypeName, NULL);

    if (0 != strcmp(szTypeName, "char16*"))
    {
        iRet = -1;
    }

    iRet |= clGetKernelArgInfo (clKernel1, 1, CL_KERNEL_ARG_ACCESS_QUALIFIER,
        sizeof(cl_kernel_arg_address_qualifier), &accessQualifier, NULL);

    if (CL_KERNEL_ARG_ACCESS_NONE != accessQualifier)
    {
        iRet = -1;
    }

    iRet |= clGetKernelArgInfo (clKernel1, 2, CL_KERNEL_ARG_TYPE_QUALIFIER,
        sizeof(cl_kernel_arg_type_qualifier), &typeQualifier, NULL);

    if (CL_KERNEL_ARG_TYPE_CONST != typeQualifier)
    {
        iRet = -1;
    }

    iRet |= clGetKernelArgInfo (clKernel1, 3, CL_KERNEL_ARG_ACCESS_QUALIFIER,
        sizeof(cl_kernel_arg_address_qualifier), &accessQualifier, NULL);

    if (CL_KERNEL_ARG_ACCESS_READ_ONLY != accessQualifier)
    {
        iRet = -1;
    }

    iRet |= clGetKernelArgInfo (clKernel2, 0, CL_KERNEL_ARG_TYPE_NAME,
        sizeof(szTypeName), szTypeName, NULL);

    if (0 != strcmp(szTypeName, "int4*"))
    {
        iRet = -1;
    }

    char szName[255];
    iRet |= clGetKernelArgInfo (clKernel2, 1, CL_KERNEL_ARG_NAME,
        sizeof(szName), szName, NULL);

    if (0 != strcmp(szName, "pBuff1"))
    {
        iRet = -1;
    }

    iRet |= clGetKernelArgInfo (clKernel2, 2, CL_KERNEL_ARG_TYPE_QUALIFIER,
        sizeof(cl_kernel_arg_type_qualifier), &typeQualifier, NULL);

    if ((CL_KERNEL_ARG_TYPE_CONST | CL_KERNEL_ARG_TYPE_VOLATILE) != typeQualifier)
    {
        iRet = -1;
    }

    if (CL_SUCCESS != iRet)
    {
        printf("clGetKernelArgInfo = %s\n",ClErrTxt(iRet));

        clReleaseContext(context);
        clReleaseProgram(clProg);
        clReleaseKernel(clKernel1);
        clReleaseKernel(clKernel2);

        return false;
    }

    iRet |= clGetKernelInfo(clKernel1, CL_KERNEL_ATTRIBUTES, sizeof(szName), szName, NULL);

    if (0 == strstr(szName, "reqd_work_group_size(8,8,8)"))
    {
        iRet = -1;
    }

    if (0 == strstr(szName, "vec_type_hint(uint8)"))
    {
        iRet = -1;
    }

    // Release objects
    clReleaseContext(context);
    clReleaseProgram(clProg);
    clReleaseKernel(clKernel1);
    clReleaseKernel(clKernel2);

    return bResult;
}
