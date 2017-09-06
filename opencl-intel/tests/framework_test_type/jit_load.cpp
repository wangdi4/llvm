#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

static const char* g_BINFILENAME = "jit.bin";

extern cl_device_type gDeviceType;

bool TestBinaryRun(cl_program& program, cl_context cxContext, cl_device_id device)
{
    bool bResult = true;
    size_t szGlobalWorkSize = 8;
    cl_int iRet;
    cl_kernel kern = clCreateKernel(program, "test_kernel", &iRet);
    bResult = SilentCheck("clCreateKernel", CL_SUCCESS, iRet);

    std::vector<int> pDst(szGlobalWorkSize);
    for(size_t i = 0; i < szGlobalWorkSize; ++i) pDst[i] = 0;
    std::vector<int> pSrcA(szGlobalWorkSize);
    for(size_t i = 0; i < szGlobalWorkSize; ++i) pSrcA[i] = 90*(i % 2);
    std::vector<int> pSrcB(szGlobalWorkSize);
    for(size_t i = 0; i < szGlobalWorkSize; ++i) pSrcB[i] = 10;

    cl_int ciErrNum;
    cl_command_queue cqCommandQueue = clCreateCommandQueue(cxContext, device, 0, &ciErrNum);
    bResult &= Check("clCreateCommandQueue(...)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevDst = clCreateBuffer(cxContext, CL_MEM_READ_WRITE , sizeof(cl_int) * szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
                                  cmDevDst,
                                  CL_TRUE,
                                  0,
                                  sizeof(cl_int) * szGlobalWorkSize,
                                  &(pDst[0]),
                                  0,
                                  NULL,
                                  NULL);
    bResult &= Check("clEnqueueWriteBuffer(Dst)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevSrcA = clCreateBuffer(cxContext, CL_MEM_READ_ONLY, sizeof(cl_int) * szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
                                  cmDevSrcA,
                                  CL_TRUE,
                                  0,
                                  sizeof(cl_int) * szGlobalWorkSize,
                                  &(pSrcA[0]),
                                  0,
                                  NULL,
                                  NULL);
    bResult &= Check("clEnqueueWriteBuffer(SrcA)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevSrcB = clCreateBuffer(cxContext, CL_MEM_READ_ONLY, sizeof(cl_int) * szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
                                  cmDevSrcB,
                                  CL_TRUE,
                                  0,
                                  sizeof(cl_int) * szGlobalWorkSize,
                                  &(pSrcB[0]),
                                  0,
                                  NULL,
                                  NULL);
    bResult &= Check("clEnqueueWriteBuffer(SrcB)", CL_SUCCESS, ciErrNum);


    if ( bResult )
    {
        iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), (void*)&cmDevDst);
        bResult &= Check("clSetKernelArg(0)", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kern, 1, sizeof(cl_mem), (void*)&cmDevSrcA);
        bResult &= Check("clSetKernelArg(1)", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kern, 2, sizeof(cl_mem), (void*)&cmDevSrcB);
        bResult &= Check("clSetKernelArg(2)", CL_SUCCESS, iRet);
    }

    ciErrNum = clEnqueueNDRangeKernel (cqCommandQueue,
            kern,
            1,
            NULL,
            &szGlobalWorkSize,
            &szGlobalWorkSize,
            0,
            NULL,
            NULL);
    bResult &= Check("clEnqueueNDRangeKernel()", CL_SUCCESS, ciErrNum);

    std::vector<int> pRead(szGlobalWorkSize);
    ciErrNum = clEnqueueReadBuffer(cqCommandQueue, 
              cmDevDst, CL_TRUE, 0, sizeof(cl_int) * szGlobalWorkSize, 
              &(pRead[0]), 0, NULL, NULL);
    bResult &= Check("clEnqueueReadBuffer()", CL_SUCCESS, ciErrNum);

    bool bCheck = true;
    for(size_t i = 0; i < szGlobalWorkSize; ++i)
    {
        bCheck &= (((size_t)pRead[i]) == 8*(i % 2));
        printf(" {%d} ", pRead[i]);
    }
    printf("validation check: %s\n", bCheck ? "PASS" : "FAIL");
    bResult &= bCheck;

    if (cmDevSrcA)clReleaseMemObject(cmDevSrcA);
    if (cmDevSrcB)clReleaseMemObject(cmDevSrcB);
    if (cmDevDst)clReleaseMemObject(cmDevDst);
    clReleaseKernel(kern);
    return bResult;
}

bool clCheckJITLoadTest()
{
    bool bResult = true;
    printf("clCheckJITSaveTest\n");

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
    cl_device_id* devices = new cl_device_id[uiNumDevices];
    iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, &devices[0], NULL);
    if (CL_SUCCESS != iRet)
    {
        printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
        return false;
    }

    // create context
    cl_context context = clCreateContext(prop, uiNumDevices, &devices[0], NULL, NULL, &iRet);
    if (CL_SUCCESS != iRet)
    {
        printf("clCreateContext = %s\n",ClErrTxt(iRet));
        return false;
    }
    printf("context = %p\n", (void*)context);

	
    // read the cached binary from a file
    FILE * pFile = fopen(g_BINFILENAME, "rb");
    
    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    size_t size = ftell (pFile);
    rewind (pFile);

    // allocate buffer
    char* pBuffer = new char[size];
	
    // copy the file content to the buffer
    fread(pBuffer, 1, size, pFile);
    fclose(pFile);


    char** ppBuffers = new char*[uiNumDevices];

    size_t* sizes = new size_t[uiNumDevices];
    for(size_t i = 0; i < uiNumDevices; ++i)
    {
        ppBuffers[i] = pBuffer;
        sizes[i] = size;
    }


    // create program with binary
    cl_int* status = new cl_int[uiNumDevices];
    cl_program clBinaryProg = clCreateProgramWithBinary(context,uiNumDevices , &devices[0], sizes, (const unsigned char**)ppBuffers, &status[0], &iRet);
    bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

    iRet = clBuildProgram(clBinaryProg, uiNumDevices, &devices[0], NULL, NULL, NULL);
    bResult &= Check("clBuildProgram", CL_SUCCESS, iRet); 

    printf("Testing the loaded binary .. \n");
    bResult &= TestBinaryRun(clBinaryProg, context, devices[0]);
	delete[] devices;
	delete[] pBuffer;
	delete[] ppBuffers;
	delete[] sizes;
	delete[] status;
	
    return bResult;
}

