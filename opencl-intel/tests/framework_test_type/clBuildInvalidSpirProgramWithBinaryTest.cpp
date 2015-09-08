#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#ifdef _WIN32
#include <windows.h>
#endif

extern cl_device_type gDeviceType;

/**************************************************************************************************
* cclBuildInvalidSpirProgramWithBinaryTest
* -------------------
* (1) get device id (gDeviceType)
* (2) create context
* (3) load binary
* (4) create program with binary (expected: FAILED)

Expected clCreateProgramWithBinary FAILED (with CL_INVALID_BINARY)
if you try to feed 32-bit SPIR to the 64-bit CPU device and vice versa
**************************************************************************************************/

cl_program inv_g_clProgram = 0;

bool clBuildInvalidSpirProgramWithBinaryTest()
{
    if (gDeviceType != CL_DEVICE_TYPE_CPU) return true;

    bool bResult = true;

    printf("---------------------------------------\n");
    printf("clBuildInvalidSpirProgramWithBinaryTest\n");
    printf("---------------------------------------\n");
    cl_device_id device = NULL;
    cl_int binaryStatus;
    cl_context context;
    cl_platform_id platform = 0;

    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

    if (!bResult)
    {
        return bResult;
    }

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);

    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
        return false;
    }

    // create context
    context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateContext = %s\n",ClErrTxt(iRet));
        return false;
    }
    printf("context = %p\n", context);

    // create binary container
    size_t uiContSize = 0;
    FILE* pIRfile = NULL;

    FOPEN(pIRfile, "inv_test.bc", "rb");

    fpos_t fileSize;
    SET_FPOS_T(fileSize, 0);
    if ( NULL == pIRfile )
    {
        printf("Failed open file.\n");
        return false;
    }
    fseek(pIRfile, 0, SEEK_END);
    fgetpos(pIRfile, &fileSize);
    uiContSize += (size_t)GET_FPOS_T(fileSize);
    fseek(pIRfile, 0, SEEK_SET);

    unsigned char* pCont = (unsigned char*)malloc(uiContSize);
    if ( NULL == pCont )
    {
        return false;
    }
    // Construct program container
    fread(((unsigned char*)pCont), 1, (size_t)GET_FPOS_T(fileSize), pIRfile);
    fclose(pIRfile);

    // create program with binary
    inv_g_clProgram = clCreateProgramWithBinary(context, 1, &device, &uiContSize, const_cast<const unsigned char**>(&pCont), &binaryStatus, &iRet);
    bResult &= Check(L"clCreateProgramWithBinary", CL_INVALID_BINARY, iRet);
    bResult &= Check(L"binaryStatus = CL_INVALID_BINARY", CL_INVALID_BINARY, binaryStatus);

    if (!bResult)
    {
        return bResult;
    }


    clReleaseProgram(inv_g_clProgram);
    clReleaseContext(context);
    return bResult;
}
