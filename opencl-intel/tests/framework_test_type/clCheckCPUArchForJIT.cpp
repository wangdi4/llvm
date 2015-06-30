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

using namespace Intel::OpenCL::Utils;

extern cl_device_type gDeviceType;

/**************************************************************************************************
  clCheckCPUArchForJIT
  --------------------
  test load invalid binaries and try to create program with these binaries
  binary file was created before for invalid CPU architecture
  Expected fail: clCreateProgramWithBinary
                 clBuildProgram
**************************************************************************************************/

/*
bool TestCheckRun(cl_program& program, cl_context cxContext, cl_device_id device)
{
    bool bResult = true;
    size_t szGlobalWorkSize = 8;
    cl_int iRet;
    cl_kernel kern = clCreateKernel(program, "test_kernel", &iRet);
    bResult = Check(L"clCreateKernel", CL_INVALID_PROGRAM, iRet);

    std::auto_ptr<int> pDst(new int[szGlobalWorkSize]);
    for (size_t i = 0; i < szGlobalWorkSize; ++i) pDst.get()[i] = 0;
    std::auto_ptr<int> pSrcA(new int[szGlobalWorkSize]);
    for (size_t i = 0; i < szGlobalWorkSize; ++i) pSrcA.get()[i] = i % 2;
    std::auto_ptr<int> pSrcB(new int[szGlobalWorkSize]);
    for (size_t i = 0; i < szGlobalWorkSize; ++i) pSrcB.get()[i] = 10;

    cl_int ciErrNum;
    cl_command_queue cqCommandQueue = clCreateCommandQueue(cxContext, device, 0, &ciErrNum);
    bResult &= Check(L"clCreateCommandQueue(...)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevDst = clCreateBuffer(cxContext, CL_MEM_READ_WRITE, sizeof(cl_int)* szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
        cmDevDst,
        CL_TRUE,
        0,
        sizeof(cl_int)* szGlobalWorkSize,
        pDst.get(),
        0,
        NULL,
        NULL);
    bResult &= Check(L"clEnqueueWriteBuffer(Dst)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevSrcA = clCreateBuffer(cxContext, CL_MEM_READ_ONLY, sizeof(cl_int)* szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
        cmDevSrcA,
        CL_TRUE,
        0,
        sizeof(cl_int)* szGlobalWorkSize,
        pSrcA.get(),
        0,
        NULL,
        NULL);
    bResult &= Check(L"clEnqueueWriteBuffer(SrcA)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevSrcB = clCreateBuffer(cxContext, CL_MEM_READ_ONLY, sizeof(cl_int)* szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
        cmDevSrcB,
        CL_TRUE,
        0,
        sizeof(cl_int)* szGlobalWorkSize,
        pSrcB.get(),
        0,
        NULL,
        NULL);
    bResult &= Check(L"clEnqueueWriteBuffer(SrcB)", CL_SUCCESS, ciErrNum);


    if (bResult)
    {
        iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), (void*)&cmDevDst);
        bResult &= Check(L"clSetKernelArg(0)", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kern, 1, sizeof(cl_mem), (void*)&cmDevSrcA);
        bResult &= Check(L"clSetKernelArg(1)", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kern, 2, sizeof(cl_mem), (void*)&cmDevSrcB);
        bResult &= Check(L"clSetKernelArg(2)", CL_SUCCESS, iRet);
    }

    ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue,
        kern,
        1,
        NULL,
        &szGlobalWorkSize,
        &szGlobalWorkSize,
        0,
        NULL,
        NULL);
    bResult &= Check(L"clEnqueueNDRangeKernel()", CL_INVALID_KERNEL, ciErrNum);

    std::auto_ptr<int> pRead(new int[szGlobalWorkSize]);
    ciErrNum = clEnqueueReadBuffer(cqCommandQueue,
        cmDevDst, CL_TRUE, 0, sizeof(cl_int)* szGlobalWorkSize,
        pRead.get(), 0, NULL, NULL);
    bResult &= Check(L"clEnqueueReadBuffer()", CL_SUCCESS, ciErrNum);

    bool bCheck = true;
    for (size_t i = 0; i < szGlobalWorkSize; ++i)
    {
        bCheck &= (pRead.get()[i] == ((i % 2) * -5 + 10));
        printf(" {%d} ", pRead.get()[i]);
    }
    printf("validation check: %s\n", bCheck ? "PASS" : "FAIL");
    bResult &= bCheck;

    if (cmDevSrcA)clReleaseMemObject(cmDevSrcA);
    if (cmDevSrcB)clReleaseMemObject(cmDevSrcB);
    if (cmDevDst)clReleaseMemObject(cmDevDst);
    clReleaseKernel(kern);
    return bResult;
}
*/
bool clCheckCPUArchForJIT() {
    cl_context context;
    cl_device_id device;

    bool bResult = true;

    printf("clCheckCPUArchForJIT\n");
    cl_uint uiNumDevices = 0;

    cl_platform_id platform = 0;

    cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetPlatformIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    // get device(s)
    iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
        return false;
    }

    // initialize arrays
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
        return false;
    }
    fseek(fout, 0, SEEK_END);
    fgetpos(fout, &fileSize);
    uiContSize += (unsigned int)GET_FPOS_T(fileSize);
    fseek(fout, 0, SEEK_SET);

    char* pCont = (char*)malloc(uiContSize);
    if (NULL == pCont)
    {
        return false;
    }
    // construct program container
    fread(((unsigned char*)pCont), 1, (size_t)GET_FPOS_T(fileSize), fout);
    fclose(fout);

    size_t * pBinarySizes = new size_t[uiNumDevices];
    char ** ppContainers = new char*[uiNumDevices];

    for (unsigned int i = 0; i < uiNumDevices; i++)
    {
        pBinarySizes[i] = uiContSize;
        ppContainers[i] = pCont;
    }

    // create program with binary
    cl_int * pBinaryStatus = new cl_int[uiNumDevices];
    cl_program program = clCreateProgramWithBinary(context, uiNumDevices, &device, pBinarySizes, (const unsigned char**)(ppContainers), pBinaryStatus, &iRet);
    bResult &= Check(L"clCreateProgramWithBinary", CL_INVALID_BINARY, iRet);

    iRet = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    bResult &= Check(L"clBuildProgram", CL_INVALID_PROGRAM, iRet);

//    iRet = TestCheckRun(program, context, device);
//    printf("test: binary progarm run %s\n", iRet ? "PASS" : "FAIL");

    // Release objects
    clReleaseProgram(program);
    clReleaseContext(context);

    return bResult;
}