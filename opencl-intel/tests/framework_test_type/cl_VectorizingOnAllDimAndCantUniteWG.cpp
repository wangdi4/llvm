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

#define WORK_SIZE_EVEN 16
#define WORK_SIZE_ODD 15

using namespace std;

extern cl_device_type gDeviceType;

static const char* sProg_prefer3 =
"__kernel void Fan(__global int *m_dev,\n"
"                  __global int *a_dev,\n"
"                  __global int *b_dev,\n"
"                  const int size) {\n"
"                 int x = get_global_id(2);\n"
"                 int y = get_global_id(1);\n"
"                 int z = get_global_id(0);\n"
"                 int index = x + y * size + z * size * size;\n"
"                 b_dev[index] = a_dev[index]"
"                 +m_dev[index];\n"
"}";

static const char* sProg_prefer1 =
"__kernel void Fan(__global int *m_dev,\n"
"                  __global int *a_dev,\n"
"                  __global int *b_dev,\n"
"                  const int size) {\n"
"                int x = get_global_id(0);\n"
"                int y = get_global_id(2);\n"
"                int z = get_global_id(1);\n"
"                int index = x + y * size + z * size * size;\n"
"                b_dev[index] = a_dev[index]"
"                +m_dev[index];\n"
"}";

static const char* sProg_prefer2 =
"__kernel void Fan(__global int *m_dev,\n"
"                  __global int *a_dev,\n"
"                  __global int *b_dev,\n"
"                  const int size) {\n"
"                int x = get_global_id(1);\n"
"                int y = get_global_id(0);\n"
"                int z = get_global_id(2);\n"
"                int index = x + y * size + z * size * size;\n"
"                b_dev[index] = a_dev[index]"
"                +m_dev[index];\n"
"}";

static const char* noWGUnite =
"__kernel void Fan(__global int *m_dev,\n"
"                  __global int *a_dev,\n"
"                  __global int *b_dev,\n"
"                  const int size) {\n"
"                int num = get_local_id(0);\n"
"                int x = get_global_id(1);\n"
"                int y = get_global_id(0);\n"
"                int z = get_global_id(2);\n"
"                int index = x + y * size + z * size * size;\n"
"                b_dev[index] = a_dev[index]+m_dev[index];\n"
"                a_dev[index] = num;\n"
"}";

cl_int getByIndex(cl_int * arr, int x, int y, int z,int size){
    return *(arr+x*size*size+y*size+z);
}

void setByIndex(cl_int * arr, int x, int y, int z,int size,cl_int val){
    *(arr+x*size*size+y*size+z) = val;
}

bool clCheckVectorizingOnAllDimAndCantUniteWG(int progIndex, bool oddDimention)
{
    cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program prog = NULL;

    std::cout << "=============================================================" << std::endl;
    std::cout << "matric_add" << std::endl;
    std::cout << "=============================================================" << std::endl;

    try
    {
        if (progIndex>3 || progIndex<0){
            std::cout << "Invalid program index" << std::endl;
            throw exception();
        }

        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException(L"clGetPlatformIDs", CL_SUCCESS, iRet);
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException(L"clGetDeviceIDs", CL_SUCCESS, iRet);

        const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
        context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
        CheckException(L"clCreateContextFromType", CL_SUCCESS, iRet);

        queue = clCreateCommandQueue(context, device, 0, &iRet);
        CheckException(L"clCreateCommandQueue", CL_SUCCESS, iRet);

        const char * program  = sProg_prefer3;
        if (progIndex == 0){
            program = sProg_prefer1;
        }
        else if (progIndex == 1){
            program = sProg_prefer2;
        }
        else if (progIndex == 3){
            program = noWGUnite;
        }

        cl_int workSize = WORK_SIZE_EVEN;
        if (oddDimention){
            workSize = WORK_SIZE_ODD;
        }

        const size_t szLengths = { strlen(program) };
        cl_program prog = clCreateProgramWithSource(context, 1, (const char**)&program, &szLengths
            , &iRet);
        CheckException(L"clCreateProgramWithSource", CL_SUCCESS, iRet);
        iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
        CheckException(L"clBuildProgram", CL_SUCCESS, iRet);

        cl_kernel kernel = clCreateKernel(prog, "Fan", &iRet);
        CheckException(L"clCreateKernel", CL_SUCCESS, iRet);

        //create input
        int memorySize = workSize * workSize * workSize * sizeof(cl_int);
        cl_int * iSrcArr1 = (cl_int *) malloc (memorySize);
        cl_int * iSrcArr2 = (cl_int *) malloc (memorySize);
        cl_int * iDstArr = (cl_int *) malloc (memorySize);
        cl_int * iDstArr_correct = (cl_int *) malloc (memorySize);
        for (cl_int i = 0; i < workSize; i++)
        {
            for (cl_int j = 0; j < workSize; j++)
            {
                for (cl_int k = 0; k < workSize; k++)
                {
                    setByIndex(iSrcArr1,i,j,k,workSize,i+j+k);
                    setByIndex(iSrcArr2,i,j,k,workSize,j);
                    setByIndex(iDstArr,i,j,k,workSize,1);
                    setByIndex(iDstArr_correct,i,j,k,workSize,1);
                }
            }
        }

        //create buffers
        clMemWrapper srcBuf1 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, memorySize, iSrcArr1, &iRet);
        CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);
        clMemWrapper srcBuf2 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, memorySize, iSrcArr2, &iRet);
        CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);
        clMemWrapper dstBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, memorySize,iDstArr, &iRet);
        CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);

        //fill the buffers with the input
        iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &srcBuf1);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &srcBuf2);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dstBuf);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
        iRet = clSetKernelArg(kernel, 3, sizeof(cl_int), &workSize);
        CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);

        size_t szGlobalWorkSize[3] = {workSize, workSize, workSize};

        iRet = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, szGlobalWorkSize, NULL, 0, NULL, NULL);
        CheckException(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

        iRet = clEnqueueReadBuffer(queue, dstBuf, CL_TRUE, 0, memorySize, iDstArr, 0, NULL, NULL);
        CheckException(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);

        // Calculate correct result
        for (size_t i = 0; i < workSize; i++)
        {
            for (size_t j = 0; j < workSize; j++)
            {
                for (size_t  k= 0; k < workSize; k++)
                {
                    setByIndex(iDstArr_correct,i,j,k,workSize,getByIndex(iSrcArr1,i,j,k,workSize)+getByIndex(iSrcArr2,i,j,k,workSize));
                }
            }
        }

        //compare results
        for (size_t i = 0; i < workSize; i++)
        {
            for (size_t j = 0; j < workSize; j++)
            {
                for (size_t  k= 0; k < workSize; k++)
                {
                    if (getByIndex(iDstArr_correct,i,j,k,workSize) != getByIndex(iDstArr,i,j,k,workSize))
                    {
                        cout << "result is not as expected for work item " << i<<" "<<j<<" "<< k << endl;
                        cout << getByIndex(iDstArr_correct,i,j,k,workSize) << " vs " << getByIndex(iDstArr,i,j,k,workSize)<< endl;
                        throw exception();
                    }
                }
            }
        }

        //free dynamically allocated structures
        if (iSrcArr1!=NULL)
        {
            free(iSrcArr1);
        }
        if (iSrcArr2!=NULL)
        {
            free(iSrcArr2);
        }
        if (iDstArr!=NULL)
        {
            free(iDstArr);
        }
        if (iDstArr_correct!=NULL)
        {
            free(iDstArr_correct);
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

