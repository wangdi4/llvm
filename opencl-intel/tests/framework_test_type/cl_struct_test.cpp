#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

#define BUFFERS_LENGTH 20000

extern cl_device_type gDeviceType;

/*******************************************************************************
 * Native Kernel test
 *
 ******************************************************************************/
typedef struct _TEST_STRUCT {
  char a;
  short b;
  int c;
  float d;
} TEST_STRUCT;

typedef union _TEST_UNION {
  int i;
  float f;
} TEST_UNION;

/*******************************************************************************
 *
 *
 ******************************************************************************/
bool clStructTest() {
  const char *ocl_test_program[] = {"typedef struct _TEST_STRUCT\
    {\
      char a;\
      short b;\
      int c;\
      float d;\
    }TEST_STRUCT;\
    \
    typedef union _TEST_UNION\
    {\
      int i;\
      float f;\
    }TEST_UNION;\
    \
    __kernel void test(TEST_STRUCT src1, TEST_UNION src2, __global TEST_STRUCT \
                       *dst)\
    {\
      size_t tid = get_global_id(0);\
      dst[tid] = src1;\
        dst[tid].c += src2.i;\
    }"};

  bool bResult = true;

  cl_platform_id platform = 0;
  cl_device_id device;
  cl_context context;

  // get platform
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // get device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create context
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // build program
  iRet = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  //
  // From here down it is the program execution implementation
  //
  TEST_STRUCT src1;
  TEST_UNION src2;
  TEST_STRUCT dst1[BUFFERS_LENGTH];
  TEST_STRUCT dst2[BUFFERS_LENGTH];

  src1.a = 1;
  src1.b = 2;
  src1.c = 3;
  src1.d = 4;

  src2.f = 1.0;

  // Create queue
  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
  bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue",
                         CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create Kernel
  cl_kernel kernel = clCreateKernel(program, "test", &iRet);
  bResult &= SilentCheck("clCreateKernel - test", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create buffers
  cl_mem buffer_dst =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                     BUFFERS_LENGTH * sizeof(TEST_STRUCT), NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - dst", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Set arguments
  iRet = clSetKernelArg(kernel, 0, sizeof(TEST_STRUCT), &src1);
  bResult &= SilentCheck("clSetKernelArg - src", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clSetKernelArg(kernel, 1, sizeof(TEST_UNION), &src2);
  bResult &= SilentCheck("clSetKernelArg - src", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_dst);
  bResult &= SilentCheck("clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Execute kernel - test
  size_t global_work_size[1] = {BUFFERS_LENGTH};
  size_t local_work_size[1] = {1};

  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size,
                                local_work_size, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Read results. wait for completion - blocking!
  iRet = clEnqueueReadBuffer(queue, buffer_dst, CL_TRUE, 0,
                             BUFFERS_LENGTH * sizeof(TEST_STRUCT), dst1, 0,
                             NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  src1.a += 1;
  src1.b += 1;
  src1.c += 1;
  src1.d += 1;

  // Set arguments
  iRet = clSetKernelArg(kernel, 0, sizeof(TEST_STRUCT), &src1);
  bResult &= SilentCheck("clSetKernelArg - src", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size,
                                local_work_size, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Read results. wait for completion - blocking!
  iRet = clEnqueueReadBuffer(queue, buffer_dst, CL_TRUE, 0,
                             BUFFERS_LENGTH * sizeof(TEST_STRUCT), dst2, 0,
                             NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  for (int i = 0; i < BUFFERS_LENGTH; ++i) {
    if ((dst1[i].a != src1.a - 1) || (dst1[i].b != src1.b - 1) ||
        (dst1[i].c != (src1.c - 1 + src2.i)) || (dst1[i].d != src1.d - 1)) {
      printf("Result did not validate:\n\
                   \\tdst[%d].a = %c\n\
                   \tdst[%d].b = %d\n\
                   \tdst[%d].c = %d\n\
                   \tdst[%d].d = %f\n",
             i, dst1[i].a, i, dst1[i].b, i, dst1[i].c, i, dst1[i].d);

      return false;
    }

    if ((dst2[i].a != src1.a) || (dst2[i].b != src1.b) ||
        (dst2[i].c != (src1.c + src2.i)) || (dst2[i].d != src1.d)) {
      printf("Result did not validate:\n\
                   \\tdst[%d].a = %c\n\
                   \tdst[%d].b = %d\n\
                   \tdst[%d].c = %d\n\
                   \tdst[%d].d = %f\n",
             i, dst1[i].a, i, dst1[i].b, i, dst1[i].c, i, dst1[i].d);

      return false;
    }
  }

  // Release objects
  iRet = clReleaseMemObject(buffer_dst);
  bResult &= SilentCheck("clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

  iRet = clReleaseKernel(kernel);
  bResult &= SilentCheck("clReleaseKernel - kernel", CL_SUCCESS, iRet);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram - program", CL_SUCCESS, iRet);

  iRet = clReleaseCommandQueue(queue);
  bResult &= SilentCheck("clReleaseCommandQueue - queue", CL_SUCCESS, iRet);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext - context", CL_SUCCESS, iRet);

  return bResult;
}
