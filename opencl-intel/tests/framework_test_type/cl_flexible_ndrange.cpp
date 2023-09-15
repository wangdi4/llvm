// Copyright (c) 2006 Intel Corporation
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

#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "test_utils.h"
#include <iostream>

using namespace std;

extern cl_device_type gDeviceType;

static constexpr unsigned GLOBAL_WORK_SIZE = 10;
static constexpr unsigned LOCAL_WORK_SIZE = 4;

static const char *sProg =
    "kernel void FlexibleNdrange(global const int* pSrc, global int* pDst)\n"
    "{\n"
    "  printf(\"work item %d at work group %d\\n\", get_global_id(0), "
    "get_local_id(0));\n"
    "  local int iNums[4];\n"
    "\n"
    "  iNums[get_local_id(0)] = pSrc[get_global_id(0)];\n"
    "  barrier(CLK_LOCAL_MEM_FENCE);\n"
    "  if (get_local_id(0) == 0)\n"
    "  {\n"
    "    int iSum = 0;\n"
    "\n"
    "    for (size_t i = 0; i < get_local_size(0); i++)\n"
    "    {\n"
    "      iSum += iNums[i];\n"
    "    }\n"
    "    pDst[get_group_id(0)] = iSum;\n"
    "  }\n"
    "}\n";

bool clFlexibleNdrange() {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id device = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL;
  cl_program prog = NULL;

  std::cout << "============================================================="
            << std::endl;
  std::cout << "clFlexibleNdrange" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  try {
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
    CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

    queue = clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

    const size_t szLengths = {strlen(sProg)};
    cl_program prog =
        clCreateProgramWithSource(context, 1, &sProg, &szLengths, &iRet);
    CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);
    iRet = clBuildProgram(prog, 1, &device, "-cl-std=CL2.0", NULL, NULL);
    CheckException("clBuildProgram", CL_SUCCESS, iRet);
    cl_kernel kernel = clCreateKernel(prog, "FlexibleNdrange", &iRet);
    CheckException("clCreateKernel", CL_SUCCESS, iRet);

    assert(GLOBAL_WORK_SIZE % LOCAL_WORK_SIZE != 0);
    const size_t szDstArrSize = GLOBAL_WORK_SIZE / LOCAL_WORK_SIZE + 1;
    cl_int iSrcArr[GLOBAL_WORK_SIZE], iDstArr[szDstArrSize];
    for (size_t i = 0; i < GLOBAL_WORK_SIZE; i++) {
      iSrcArr[i] = rand();
    }
    clMemWrapper srcBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR,
                                         sizeof(iSrcArr), iSrcArr, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);
    clMemWrapper dstBuf =
        clCreateBuffer(context, 0, sizeof(iDstArr), NULL, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);

    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &srcBuf);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dstBuf);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);

    const size_t szGlobalWorkSize = GLOBAL_WORK_SIZE,
                 szLocalWorkSize = LOCAL_WORK_SIZE;
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &szGlobalWorkSize,
                                  &szLocalWorkSize, 0, NULL, NULL);
    CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

    iRet = clEnqueueReadBuffer(queue, dstBuf, CL_TRUE, 0, sizeof(iDstArr),
                               iDstArr, 0, NULL, NULL);
    CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);

    // Check result
    for (size_t i = 0; i < szDstArrSize; i++) {
      cl_int iSum = 0;
      for (size_t j = 0;
           j < (i < szDstArrSize - 1 ? LOCAL_WORK_SIZE
                                     : GLOBAL_WORK_SIZE % LOCAL_WORK_SIZE);
           j++) {
        iSum += iSrcArr[i * LOCAL_WORK_SIZE + j];
      }
      if (iSum != iDstArr[i]) {
        cout << "result is not as expected for work group " << i << endl;
        throw exception();
      }
    }
  } catch (const std::exception &) {
    bResult = false;
  }
  if (context) {
    clReleaseContext(context);
  }
  if (queue) {
    clReleaseCommandQueue(queue);
  }
  if (prog) {
    clReleaseProgram(prog);
  }
  return bResult;
}
