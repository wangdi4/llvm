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
/*
bool TestRun(cl_program& program, cl_context cxContext, cl_device_id device)
{
    bool bResult = true;
    size_t szGlobalWorkSize = 8;
    cl_int iRet;
    cl_kernel kern = clCreateKernel(program, "test_kernel", &iRet);
    bResult = SilentCheck("clCreateKernel", CL_SUCCESS, iRet);

    std::auto_ptr<int> pDst(new int[szGlobalWorkSize]);
    for(size_t i = 0; i < szGlobalWorkSize; ++i) pDst.get()[i] = 0;
    std::auto_ptr<int> pSrcA(new int[szGlobalWorkSize]);
    for(size_t i = 0; i < szGlobalWorkSize; ++i) pSrcA.get()[i] = i % 2;
    std::auto_ptr<int> pSrcB(new int[szGlobalWorkSize]);
    for(size_t i = 0; i < szGlobalWorkSize; ++i) pSrcB.get()[i] = 10;

    cl_int ciErrNum;
    cl_command_queue cqCommandQueue = clCreateCommandQueue(cxContext, device, 0, &ciErrNum);
    bResult &= Check("clCreateCommandQueue(...)", CL_SUCCESS, ciErrNum);

    cl_mem cmDevDst = clCreateBuffer(cxContext, CL_MEM_READ_WRITE , sizeof(cl_int) * szGlobalWorkSize, NULL, &ciErrNum);
    ciErrNum = clEnqueueWriteBuffer(cqCommandQueue,
                                  cmDevDst,
                                  CL_TRUE,
                                  0,
                                  sizeof(cl_int) * szGlobalWorkSize,
                                  pDst.get(),
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
                                  pSrcA.get(),
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
                                  pSrcB.get(),
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

    std::auto_ptr<int> pRead(new int[szGlobalWorkSize]);
    ciErrNum = clEnqueueReadBuffer(cqCommandQueue, 
              cmDevDst, CL_TRUE, 0, sizeof(cl_int) * szGlobalWorkSize, 
              pRead.get(), 0, NULL, NULL);
    bResult &= Check("clEnqueueReadBuffer()", CL_SUCCESS, ciErrNum);

    bool bCheck = true;
    for(size_t i = 0; i < szGlobalWorkSize; ++i)
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
bool clCheckJITSaveTest()
{
    bool bResult = true;
    const char *ocl_test_program[] = {\
    "__kernel void test_kernel_0(__global int* pBuff0, __global int* pBuff1, __global int* pBuff2)"\
    "{"\
    "    size_t id = get_global_id(0);"\
    "    pBuff0[id] = (int)(10.0*(cos((float)pBuff1[id])));"\
    "}" \
    "__kernel void test_kernel(__global int* pBuff0, __global int* pBuff1, __global int* pBuff2)"\
    "{"\
    "    size_t id = get_global_id(0);"\
    "    pBuff0[id] = (int)(10.0*(sin((float)pBuff1[id])));"\
    "}"
    };

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

    std::vector<cl_device_id> devices(uiNumDevices);
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
        iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * uiNumDevices, &binarySizes[0], NULL);
        bResult &= Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
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
            bResult &= Check("clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);
            if (bResult)
            {
                FILE * fout;
                fout = fopen(g_BINFILENAME, "wb");
                fwrite(pBinaries[0], 1, binarySizes[0], fout);
                fclose(fout);
                printf("Saved successfully!! [size = %zu] \n", sumBinariesSize);
            }
        }
    }
    return bResult;
}

