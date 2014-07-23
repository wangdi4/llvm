// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <iostream>
#include "CL/cl.h"
#include "test_utils.h"
#include "TestsHelpClasses.h"

#define WORK_SIZE_DIM 16

using namespace std;

extern cl_device_type gDeviceType;

static const char* sProg_prefer2 =
"__kernel void Fan2(__global int *m_dev,\n"
"                  __global int *a_dev,\n"
"                  __global int *b_dev,\n"
"                  const int size,\n"
"                  const int t) {\n"
"                  int globalIdx = get_global_id(0);\n"
"                  int globalIdy = get_global_id(1);\n"
"                  \n"
"                  if (globalIdx < size-1-t && globalIdy < size-t) {\n"
"                      a_dev[size*(globalIdx+1+t)+(globalIdy+t)] -= m_dev[size*(globalIdx+1+t)+t] * a_dev[size*t+(globalIdy+t)];\n"
"                      \n"
"                      if(globalIdy == 0){\n"
"                           b_dev[globalIdx+1+t] -= m_dev[size*(globalIdx+1+t)+(globalIdy+t)] * b_dev[t];\n"
"                      }\n"
"                  } \n"
"}";

static const char* sProg_prefer1 =
"__kernel void Fan2(__global int *m_dev,\n"
"                  __global int *a_dev,\n"
"                  __global int *b_dev,\n"
"                  const int size,\n"
"                  const int t) {\n"
"                int globalIdx = get_global_id(1);\n"
"                int globalIdy = get_global_id(0);\n"
"                \n"
"                if (globalIdx < size-1-t && globalIdy < size-t) {\n"
"                a_dev[size*(globalIdx+1+t)+(globalIdy+t)] -= m_dev[size*(globalIdx+1+t)+t] * a_dev[size*t+(globalIdy+t)];\n"
"                \n"
"                if(globalIdy == 0){\n"
"                   b_dev[globalIdx+1+t] -= m_dev[size*(globalIdx+1+t)+(globalIdy+t)] * b_dev[t];\n"
"                }\n"
"            } \n"
"}";

bool clCheckVectorizingDim1And2AndUniteWG(int progIndex,bool hasLocalWGSize)
{
    cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program prog = NULL;

    std::cout << "=============================================================" << std::endl;
    std::cout << "check_null_huristics" << std::endl;
    std::cout << "=============================================================" << std::endl;

    try
    {
        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException(L"clGetPlatformIDs", CL_SUCCESS, iRet);
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException(L"clGetDeviceIDs", CL_SUCCESS, iRet);

        const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
        context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
        CheckException(L"clCreateContextFromType", CL_SUCCESS, iRet);

        queue = clCreateCommandQueue(context, device, 0, &iRet);
        CheckException(L"clCreateCommandQueue", CL_SUCCESS, iRet);

        const char * program  = (progIndex==0)?sProg_prefer1:sProg_prefer2;
        const size_t szLengths = { strlen(program) };
        cl_program prog = clCreateProgramWithSource(context, 1, (const char**)&program, &szLengths, &iRet);

        CheckException(L"clCreateProgramWithSource", CL_SUCCESS, iRet);
        iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
        CheckException(L"clBuildProgram", CL_SUCCESS, iRet);

        cl_kernel kernel = clCreateKernel(prog, "Fan2", &iRet);
        CheckException(L"clCreateKernel", CL_SUCCESS, iRet);

        //create input
        cl_int iSrcArr1[WORK_SIZE_DIM][WORK_SIZE_DIM];
        cl_int iSrcArr2[WORK_SIZE_DIM][WORK_SIZE_DIM];
        cl_int iDstArr[WORK_SIZE_DIM];
        cl_int iDstArr_correct[WORK_SIZE_DIM];
        for (size_t i = 0; i < WORK_SIZE_DIM; i++)
        {
            for (size_t j = 0; j < WORK_SIZE_DIM; j++)
            {
                iSrcArr1[i][j] = i;
                iSrcArr2[i][j] = j;
            }
            iDstArr[i] = 1;
            iDstArr_correct[i] = 1;
        }
        cl_int size = WORK_SIZE_DIM;
        cl_int t = 0;

        //create buffers
        clMemWrapper srcBuf1 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, sizeof(iSrcArr1), iSrcArr1, &iRet);
        CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);
        clMemWrapper srcBuf2 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, sizeof(iSrcArr2), iSrcArr2, &iRet);
        CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);
        clMemWrapper dstBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, sizeof(iDstArr),iDstArr, &iRet);
        CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);

        //fill the buffers with the input
        iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &srcBuf1);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &srcBuf2);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dstBuf);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 3, sizeof(cl_int), &size);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 4, sizeof(cl_int), &t);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);

        size_t szGlobalWorkSize[2] = {WORK_SIZE_DIM, WORK_SIZE_DIM};

        if (hasLocalWGSize){
            size_t localWGSize[2]={2,2};
            iRet = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, szGlobalWorkSize, localWGSize , 0, NULL, NULL);
        }
        else{
            iRet = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, szGlobalWorkSize, NULL , 0, NULL, NULL);
        }

        CheckException(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

        iRet = clEnqueueReadBuffer(queue, dstBuf, CL_TRUE, 0, sizeof(iDstArr), iDstArr, 0, NULL, NULL);
        CheckException(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);

        // Calculate correct result
        for (size_t i = 0; i < WORK_SIZE_DIM; i++)
        {
            for (size_t j = 0; j < WORK_SIZE_DIM; j++)
            {
                if (i < size-1-t && j < size-t){
                    iSrcArr2[i+1+t][j+t] -= iSrcArr1[i+1+t][t]*iSrcArr2[t][j+t] ;
                    if (j==0){
                        iDstArr_correct[i+1+t] -= iSrcArr1[i+1+t][j+t]*iDstArr_correct[t];
                    }
                }
            }
        }

        //compare results
        for (size_t j = 0; j < WORK_SIZE_DIM; j++)
        {
            if (iDstArr_correct[j] != iDstArr[j])
            {
                cout << "result is not as expected for work group " << j << endl;
                cout << iDstArr_correct[j] << " vs " << iDstArr[j]<< endl;
                throw exception();
            }
        }

        //release buffers
        if (srcBuf1){
           clReleaseMemObject(srcBuf1);
        }
        if (srcBuf2){
           clReleaseMemObject(srcBuf2);
        }
        if (dstBuf){
           clReleaseMemObject(dstBuf);
        }
    }
    catch (const std::exception& exe)
    {
        cerr << exe.what() << endl;
        bResult = false;
    }
    if (context)
    {
        clReleaseContext(context);
    }
    if (queue)
    {
        clReleaseCommandQueue(queue);
    }
    if (prog)
    {
        clReleaseProgram(prog);
    }
    return bResult;
}

