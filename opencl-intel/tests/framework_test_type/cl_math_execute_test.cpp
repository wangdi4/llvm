#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

#define BUFFER_SIZE 128

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clMathExecuteTest
 * -------------------
 * Implement image access test
 ******************************************************************************/
bool clMathExecuteTest() {
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;
  cl_device_id clDefaultDeviceId;

  printf("=============================================================\n");
  printf("clMathExecuteTest\n");
  printf("=============================================================\n");

  const char *ocl_test_program[] = {
      "__kernel void math_test(__global float* pBuff)\n"
      "{\n"
      "size_t x = get_global_id(0);\n"
      "float cos_val;\n"
      "float sin_val = sincos(pBuff[x], &cos_val);\n"
      "sin_val += cos_val;\n"
      "pBuff[x] = sin_val;\n"
      "}"};

  cl_platform_id platform = 0;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

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

  {
    cl_command_queue queue = clCreateCommandQueueWithProperties(
        context, clDefaultDeviceId, NULL /*no properties*/, &iRet);
    bResult &=
        Check("clCreateCommandQueueWithProperties - queue", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_context;

      // create program with source
#if 0
  cl_program program =
    clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL,
                              &iRet);
  bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult) goto release_queue;
  iRet = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult) goto release_program;
#else
    cl_program program;
    if (!BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                           NULL, &program))
      goto release_queue;
#endif

    {
      cl_kernel kernel = clCreateKernel(program, "math_test", &iRet);
      bResult &= Check("clCreateKernel - math_test", CL_SUCCESS, iRet);
      if (!bResult)
        goto release_program;

      {
        size_t stBuffSize = BUFFER_SIZE;
        cl_float pBuff[BUFFER_SIZE];
        cl_float pDstBuff[BUFFER_SIZE];

        srand(0);

        // fill with random bits no matter what
        for (unsigned ui = 0; ui < BUFFER_SIZE; ui++) {
          pBuff[ui] = (cl_float)(rand() - RAND_MAX / 2) / (cl_float)RAND_MAX;
        }

        {
          cl_mem clBuff =
              clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                             sizeof(pBuff), pBuff, &iRet);
          bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
          if (!bResult) {
            goto release_kernel;
          }

          // Set Kernel Arguments
          iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
          bResult &= Check("clSetKernelArg(0) - clBuff", CL_SUCCESS, iRet);
          if (!bResult)
            goto release_image;

          {
            size_t global_work_size[1] = {stBuffSize};

            // Execute kernel
            iRet = clEnqueueNDRangeKernel(
                queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
          }
          bResult &= Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
          if (!bResult)
            goto release_image;
          //
          // Verification phase
          //
          iRet = clEnqueueReadBuffer(queue, clBuff, CL_TRUE, 0,
                                     sizeof(pDstBuff), pDstBuff, 0, NULL, NULL);
          bResult &= Check("clEnqueueReadBuffer - Dst", CL_SUCCESS, iRet);
          if (!bResult)
            goto release_image;

          for (unsigned y = 0; (y < stBuffSize) && bResult; ++y) {
            if (fabsf(pDstBuff[y] -
                      ((float)sin(pBuff[y]) + (float)cos(pBuff[y]))) > 0.0001) {
              bResult = false;
            }
          }

          if (bResult) {
            printf(
                "*** clImageExecuteTest compare verification succeeded *** \n");
          } else {
            printf("!!!!!! clImageExecuteTest compare verification failed "
                   "!!!!! \n");
          }

        release_image:
          clReleaseMemObject(clBuff);
        }
      }
    release_kernel:
      clReleaseKernel(kernel);
    }
  release_program:
    clReleaseProgram(program);
  release_queue:
    clFinish(queue);
    clReleaseCommandQueue(queue);
  }
release_context:
  clReleaseContext(context);
release_end:
  return bResult;
}
