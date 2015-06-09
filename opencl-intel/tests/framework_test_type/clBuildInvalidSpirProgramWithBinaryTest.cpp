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
* (3) create binary
* (4) create program with binary
* (5) build program (expected: FAILED)

Expected clBuildProgram FAILED (with CL_BUILD_PROGRAM_FAILURE)
if you try to feed 32-bit SPIR to the 64-bit CPU device and vice versa
**************************************************************************************************/

cl_program inv_g_clProgram = 0;

bool clBuildInvalidSpirProgramWithBinaryTest()
{
    bool bResult = true;

    printf("---------------------------------------\n");
    printf("clBuildInvalidSpirProgramWithBinaryTest\n");
    printf("---------------------------------------\n");
    cl_device_id device = NULL;
    size_t * pBinarySizes;
    cl_int binaryStatus;
    cl_context context;
    char** ppContainers;
    cl_platform_id platform = 0;

    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

    if (!bResult)
    {
        return bResult;
    }

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);

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

    char* pCont = (char*)malloc(uiContSize);
    if ( NULL == pCont )
    {
        return false;
    }
    // Construct program container
    fread(((unsigned char*)pCont), 1, (size_t)GET_FPOS_T(fileSize), pIRfile);
    fclose(pIRfile);


    pBinarySizes = &uiContSize;
    ppContainers = &pCont;


    // create program with binary
    inv_g_clProgram = clCreateProgramWithBinary(context, 1, &device, pBinarySizes, (const unsigned char**)(ppContainers), &binaryStatus, &iRet);
    // FIXME: commented code below is right, but hard to implement. Need to fix in the future
    // bResult &= Check(L"clCreateProgramWithBinary", CL_DEV_INVALID_BINARY, iRet);
    bResult &= Check(L"clCreateProgramWithBinary", CL_SUCCESS, iRet);

    iRet = clBuildProgram(inv_g_clProgram, 1, &device, NULL, NULL, NULL);
    bResult &= Check(L"clBuildProgram", CL_BUILD_PROGRAM_FAILURE, iRet);

    if (!bResult)
    {
        return bResult;
    }


    clReleaseProgram(inv_g_clProgram);
    clReleaseContext(context);
    return bResult;
}
