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

#define WORK_SIZE_EVEN 16
#define WORK_SIZE_ODD 15
#define WORK_SIZE_X 100;
#define WORK_SIZE_Y 200;
#define WORK_SIZE_Z 245;

using namespace std;

extern cl_device_type gDeviceType;

static const char *sProg_prefer3 =
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

static const char *sProg_prefer1 =
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

static const char *sProg_prefer2 =
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

static const char *noWGUnite =
    "__kernel void Fan(__global int *m_dev,\n"
    "                  __global int *a_dev,\n"
    "                  __global int *b_dev,\n"
    "                  const int size) {\n"
    "                int num = get_local_id(0);\n"
    "                int x = get_global_id(1);\n"
    "                int y = get_global_id(0);\n"
    "                int z = get_global_id(2);\n"
    "                int index = x + y * size + z * size * size;\n"
    "                a_dev[index] = num;\n"
    "                b_dev[index] = a_dev[index]+m_dev[index];\n"
    "}";

static const char *differentSizes =
    "__kernel void Fan(__global int *m_dev,\n"
    "                  __global int *a_dev,\n"
    "                  __global int *b_dev,\n"
    "                  const int sizeX,\n"
    "                  const int sizeY,\n"
    "                  const int sizeZ){\n"
    "                int x = get_global_id(1);\n"
    "                int y = get_global_id(0);\n"
    "                int z = get_global_id(2);\n"
    "                int index = x*sizeX*sizeY + y * sizeY + z;\n"
    "                b_dev[index] = a_dev[index]+m_dev[index];\n"
    "}";

cl_int getByIndex(cl_int *arr, int x, int y, int z, int sizeX, int sizeY,
                  int sizeZ) {
  return *(arr + x * sizeX * sizeY + y * sizeY + z);
}

void setByIndex(cl_int *arr, int x, int y, int z, int sizeX, int sizeY,
                int sizeZ, cl_int val) {
  *(arr + x * sizeX * sizeY + y * sizeY + z) = val;
}

bool clCheckVectorizingOnAllDimAndCantUniteWG(int progIndex, bool oddDimention,
                                              bool hasLocalWGSize) {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id device = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL;
  cl_program prog = NULL;

  std::cout << "============================================================="
            << std::endl;
  std::cout << "matric_add" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  try {
    if (progIndex > 4 || progIndex < 0) {
      std::cout << "Invalid program index" << std::endl;
      throw exception();
    }

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

    bool additionalSizes = false;
    const char *program = sProg_prefer3;
    if (progIndex == 0) {
      program = sProg_prefer1;
    } else if (progIndex == 1) {
      program = sProg_prefer2;
    } else if (progIndex == 3) {
      program = noWGUnite;
    } else if (progIndex == 4) {
      program = differentSizes;
      additionalSizes = true;
    }

    cl_int workSize = WORK_SIZE_EVEN;
    if (oddDimention) {
      workSize = WORK_SIZE_ODD;
    }

    const size_t szLengths = {strlen(program)};
    cl_program prog = clCreateProgramWithSource(
        context, 1, (const char **)&program, &szLengths, &iRet);
    CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);
    iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
    CheckException("clBuildProgram", CL_SUCCESS, iRet);

    cl_kernel kernel = clCreateKernel(prog, "Fan", &iRet);
    CheckException("clCreateKernel", CL_SUCCESS, iRet);

    // decide about the work size
    int workSizeX = workSize;
    int workSizeY = workSize;
    int workSizeZ = workSize;
    if (additionalSizes) {
      workSizeX = WORK_SIZE_X;
      workSizeY = WORK_SIZE_Y;
      workSizeZ = WORK_SIZE_Z;
    }

    // create input
    int memorySize = workSizeX * workSizeY * workSizeZ * sizeof(cl_int);
    cl_int *iSrcArr1 = (cl_int *)malloc(memorySize);
    cl_int *iSrcArr2 = (cl_int *)malloc(memorySize);
    cl_int *iDstArr = (cl_int *)malloc(memorySize);
    cl_int *iDstArr_correct = (cl_int *)malloc(memorySize);
    for (cl_int i = 0; i < workSizeX; i++) {
      for (cl_int j = 0; j < workSizeY; j++) {
        for (cl_int k = 0; k < workSizeZ; k++) {
          setByIndex(iSrcArr1, i, j, k, workSizeX, workSizeY, workSizeZ,
                     i + j + k);
          setByIndex(iSrcArr2, i, j, k, workSizeX, workSizeY, workSizeZ, j);
          setByIndex(iDstArr, i, j, k, workSizeX, workSizeY, workSizeZ, 1);
          setByIndex(iDstArr_correct, i, j, k, workSizeX, workSizeY, workSizeZ,
                     1);
        }
      }
    }

    // create buffers
    clMemWrapper srcBuf1 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR,
                                          memorySize, iSrcArr1, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);
    clMemWrapper srcBuf2 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR,
                                          memorySize, iSrcArr2, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);
    clMemWrapper dstBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR,
                                         memorySize, iDstArr, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);

    // fill the buffers with the input
    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &srcBuf1);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &srcBuf2);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dstBuf);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    if (additionalSizes) {
      iRet = clSetKernelArg(kernel, 3, sizeof(cl_int), &workSizeX);
      CheckException("clSetKernelArg", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel, 4, sizeof(cl_int), &workSizeY);
      CheckException("clSetKernelArg", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel, 5, sizeof(cl_int), &workSizeZ);
      CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    } else {
      iRet = clSetKernelArg(kernel, 3, sizeof(cl_int), &workSize);
      CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    }

    size_t szGlobalWorkSize[3] = {(size_t)workSizeX, (size_t)workSizeY,
                                  (size_t)workSizeZ};

    if (additionalSizes && hasLocalWGSize) {
      size_t localWGSize[3] = {1, 1, 5};
      iRet = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, szGlobalWorkSize,
                                    localWGSize, 0, NULL, NULL);
      CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
    } else {
      iRet = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, szGlobalWorkSize,
                                    NULL, 0, NULL, NULL);
      CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
    }

    iRet = clEnqueueReadBuffer(queue, dstBuf, CL_TRUE, 0, memorySize, iDstArr,
                               0, NULL, NULL);
    CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);

    // Calculate correct result
    for (int i = 0; i < workSizeX; i++) {
      for (int j = 0; j < workSizeY; j++) {
        for (int k = 0; k < workSizeZ; k++) {
          setByIndex(
              iDstArr_correct, i, j, k, workSizeX, workSizeY, workSizeZ,
              getByIndex(iSrcArr1, i, j, k, workSizeX, workSizeY, workSizeZ) +
                  getByIndex(iSrcArr2, i, j, k, workSizeX, workSizeY,
                             workSizeZ));
        }
      }
    }

    // compare results
    for (int i = 0; i < workSizeX; i++) {
      for (int j = 0; j < workSizeY; j++) {
        for (int k = 0; k < workSizeZ; k++) {
          if (getByIndex(iDstArr_correct, i, j, k, workSizeX, workSizeY,
                         workSizeZ) !=
              getByIndex(iDstArr, i, j, k, workSizeX, workSizeY, workSizeZ)) {
            cout << "result is not as expected for work item " << i << " " << j
                 << " " << k << endl;
            cout << getByIndex(iDstArr_correct, i, j, k, workSizeX, workSizeY,
                               workSizeZ)
                 << " vs "
                 << getByIndex(iDstArr, i, j, k, workSizeX, workSizeY,
                               workSizeZ)
                 << endl;
            throw exception();
          }
        }
      }
    }

    // free dynamically allocated structures
    if (iSrcArr1 != NULL) {
      free(iSrcArr1);
    }
    if (iSrcArr2 != NULL) {
      free(iSrcArr2);
    }
    if (iDstArr != NULL) {
      free(iDstArr);
    }
    if (iDstArr_correct != NULL) {
      free(iDstArr_correct);
    }
  } catch (const std::exception &exe) {
    cerr << exe.what() << endl;
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
