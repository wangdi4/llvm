#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>

#define BUFFER_SIZE 128

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clIntegerExecuteTest
 * -------------------
 * Implement image access test
 ******************************************************************************/
bool clIntegerExecuteTest() {
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;
  cl_device_id clDefaultDeviceId;

  printf("=============================================================\n");
  printf("clIntegerExecuteTest\n");
  printf("=============================================================\n");

  const char *ocl_test_program[] = {
      "__kernel void int_test(__global long* pValues)\n"
      "{\n"
      "size_t x = get_global_id(0);\n"
      "long val = pValues[x];\n"
      "long res = abs(val);\n"
      "pValues[x] = res;\n"
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
      cl_kernel kernel = clCreateKernel(program, "int_test", &iRet);
      bResult &= Check("clCreateKernel - int_test", CL_SUCCESS, iRet);
      if (!bResult)
        goto release_program;

      {
        size_t stBuffSize = BUFFER_SIZE;
        cl_long pBuff[BUFFER_SIZE];
        cl_long pDstBuff[BUFFER_SIZE];

        srand(0);

        // fill with random bits no matter what
        for (unsigned ui = 0; ui < BUFFER_SIZE; ui++) {
          pBuff[ui] = (cl_long)(rand() - RAND_MAX / 2);
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
          bResult &= Check("clSetKernelArg - clBuff", CL_SUCCESS, iRet);
          if (!bResult)
            goto release_image;

          {
            size_t global_work_size[1] = {stBuffSize};

            // Execute kernel
            iRet = clEnqueueNDRangeKernel(
                queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
          }
          bResult &= Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
          //
          // Verification phase
          //
          iRet = clEnqueueReadBuffer(queue, clBuff, CL_TRUE, 0,
                                     sizeof(pDstBuff), pDstBuff, 0, NULL, NULL);
          bResult &= Check("clEnqueueReadBuffer - Dst", CL_SUCCESS, iRet);
          if (!bResult)
            goto release_image;

          for (unsigned y = 0; y < stBuffSize; ++y) {
            if (pDstBuff[y] != ABS64(pBuff[y])) {
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
