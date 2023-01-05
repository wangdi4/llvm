#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

#define BUFFER_SIZE 128

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clRelaxedFunctionTest
 * -------------------
 * Implement relaxed functions test
 ******************************************************************************/
bool clRelaxedFunctionTest() {
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;
  cl_device_id clDefaultDeviceId;

  printf("=============================================================\n");
  printf("clRelaxedFunctionTest\n");
  printf("=============================================================\n");

  const char *ocl_test_program[] = {
      "__kernel void relaxed_test(__global float* pIn1, __global float* pIn2, "
      "__global float* pOut)\n"
      "{\n"
      "size_t index = get_global_id(0);\n"
      "float t1 = sqrt(pIn1[index]);\n"
      "float t2 = rsqrt(pIn2[index]);\n"
      "float t3 = hypot(t1, t2);\n"
      "float t4 = fract(t1, &t2);\n"
      "float t5 = fmax(t3, t4);\n"
      "pOut[index] = powr(t5, 2.3f);\n"
      "}"};

  cl_platform_id platform = 0;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  size_t stBuffSize = BUFFER_SIZE;

  cl_mem clBuff1, clBuff2, clBuffDst, clBuffDstRelaxed;
  cl_kernel kernel, relaxedKernel;
  cl_command_queue queue;

  size_t global_work_size[1] = {stBuffSize};

  //
  // Initiate test infrastructure:
  // Create context, Queue
  //
  cl_context context =
      clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  bResult &= Check("clCreateContextFromType", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_end;

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_context;

  queue = clCreateCommandQueueWithProperties(context, clDefaultDeviceId,
                                             NULL /*no properties*/, &iRet);
  bResult &=
      Check("clCreateCommandQueueWithProperties - queue", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_context;

  // create program with source
  cl_program program, relaxedProgram;
  if (!BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                         "-cl-fast-relaxed-math", &relaxedProgram)) {
    bResult = false;
    goto release_queue;
  }

  if (!BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                         NULL, &program)) {
    bResult = false;
    goto release_relaxed_program;
  }

  kernel = clCreateKernel(program, "relaxed_test", &iRet);
  bResult &= Check("clCreateKernel - relaxed_test", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_program;

  relaxedKernel = clCreateKernel(relaxedProgram, "relaxed_test", &iRet);
  bResult &= Check("clCreateKernel - relaxed_test", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_kernel;

  cl_float pBuff1[BUFFER_SIZE];
  cl_float pBuff2[BUFFER_SIZE];
  cl_float pDstBuff[BUFFER_SIZE];
  cl_float pDstBuffRelaxed[BUFFER_SIZE];

  srand(0);

  // fill with random bits no matter what
  for (unsigned ui = 0; ui < BUFFER_SIZE; ui++) {
    pBuff1[ui] = (cl_float)(rand()) / (cl_float)RAND_MAX;
    pBuff2[ui] = (cl_float)(rand()) / (cl_float)RAND_MAX;
    pDstBuff[ui] = -1.0f;
    pDstBuffRelaxed[ui] = -1.0f;
  }

  clBuff1 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                           sizeof(pBuff1), pBuff1, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    goto release_relaxed_kernel;
  }

  clBuff2 = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                           sizeof(pBuff2), pBuff2, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    goto release_clBuff1;
  }

  clBuffDst = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                             sizeof(pDstBuff), pDstBuff, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    goto release_clBuff2;
  }

  clBuffDstRelaxed =
      clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     sizeof(pDstBuffRelaxed), pDstBuffRelaxed, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    goto release_clBuffDst;
  }

  // Set Kernel Arguments
  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff1);
  bResult &= Check("clSetKernelArg(0) - clBuff1", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &clBuff2);
  bResult &= Check("clSetKernelArg(1) - clBuff2", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &clBuffDst);
  bResult &= Check("clSetKernelArg(2) - clBuffDst", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clSetKernelArg(relaxedKernel, 0, sizeof(cl_mem), &clBuff1);
  bResult &= Check("clSetKernelArg(0) - clBuff1", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clSetKernelArg(relaxedKernel, 1, sizeof(cl_mem), &clBuff2);
  bResult &= Check("clSetKernelArg(1) - clBuff2", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clSetKernelArg(relaxedKernel, 2, sizeof(cl_mem), &clBuffDstRelaxed);
  bResult &= Check("clSetKernelArg(2) - clBuffDst", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  // Execute kernel
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL,
                                0, NULL, NULL);
  bResult &= Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clEnqueueNDRangeKernel(queue, relaxedKernel, 1, NULL, global_work_size,
                                NULL, 0, NULL, NULL);
  bResult &= Check("clEnqueueNDRangeKernel relaxed", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;
  //
  // Verification phase
  //
  iRet = clEnqueueReadBuffer(queue, clBuffDst, CL_TRUE, 0, sizeof(pDstBuff),
                             pDstBuff, 0, NULL, NULL);
  bResult &= Check("clEnqueueReadBuffer - Dst", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  iRet = clEnqueueReadBuffer(queue, clBuffDstRelaxed, CL_TRUE, 0,
                             sizeof(pDstBuffRelaxed), pDstBuffRelaxed, 0, NULL,
                             NULL);
  bResult &= Check("clEnqueueReadBuffer - DstRelaxed", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_DstRelaxed;

  bResult = false;

  for (unsigned y = 0; (y < stBuffSize) && !bResult; ++y) {
    if (pDstBuff[y] != pDstBuffRelaxed[y]) {
      bResult = true; // relaxed version is less accurate
    }
  }

  for (unsigned y = 0; (y < stBuffSize) && bResult; ++y) {
    if (0 == fabs(pDstBuff[y])) {
      if (fabs(pDstBuff[y] - pDstBuffRelaxed[y]) > 0.15) {
        bResult = false;
        break;
      }
    } else {
      if (fabs((pDstBuff[y] - pDstBuffRelaxed[y]) / pDstBuff[y]) > 0.15) {
        bResult = false;
        break;
      }
    }
  }

  if (bResult) {
    printf("*** clRelaxedFunction compare verification succeeded *** \n");
  } else {
    printf("!!!!!! clRelaxedFunction compare verification failed !!!!! \n");
  }

release_DstRelaxed:
  clReleaseMemObject(clBuffDstRelaxed);
release_clBuffDst:
  clReleaseMemObject(clBuffDst);
release_clBuff2:
  clReleaseMemObject(clBuff2);
release_clBuff1:
  clReleaseMemObject(clBuff1);
release_relaxed_kernel:
  clReleaseKernel(relaxedKernel);
release_kernel:
  clReleaseKernel(kernel);
release_program:
  clReleaseProgram(program);
release_relaxed_program:
  clReleaseProgram(relaxedProgram);
release_queue:
  clFinish(queue);
  clReleaseCommandQueue(queue);
release_context:
  clReleaseContext(context);
release_end:
  return bResult;
}
