//|
//| TEST: MKL integration test
//|
//| Purpose
//| -------
//|
//| Test initial integration with MKL library
//|
//| Method
//| ------
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#ifdef __INCLUDE_MKL__
#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <assert.h>
#include <stdio.h>

#define WORK_SIZE 1
#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

#define MATRIX_DIM_SIZE 2048

bool mkl_test();

TEST(MKL, Test_MKL_BLAS) { EXPECT_TRUE(mkl_test()); }

bool mkl_test() {
  printf("---------------------------------------\n");
  printf("MKL test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_device_id device = NULL;
  cl_program program = NULL;
  cl_kernel kernel = NULL;
  cl_int err;
  cl_uint num_devices = 1;
  cl_platform_id platform = NULL;

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult = SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...) should fail
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, &num_devices);
  bResult = SilentCheck("clGetDeviceIDs - get CPU device", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  size_t ulRetValCheck = -1;
  size_t actual_size = 0;
  err = clGetDeviceInfo(device, CL_DEVICE_BUILT_IN_KERNELS, 0, NULL,
                        &actual_size);
  bResult = SilentCheck("clGetDeviceInfo, CL_DEVICE_BUILT_IN_KERNELS",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  if (0 == actual_size) {
    printf("Device doesn't support built-in functions\nTest skipped\n");
    return true;
  }

  char *szBultInList = (char *)alloca(actual_size);
  assert(NULL != szBultInList && "alloca() can't fail");
  err = clGetDeviceInfo(device, CL_DEVICE_BUILT_IN_KERNELS, actual_size,
                        szBultInList, &actual_size);
  bResult = SilentCheck("clGetDeviceInfo, CL_DEVICE_BUILT_IN_KERNELS",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  static const char *szMKLFunction = "cblas_sgemm";
  // Locate if desired built-in exists in device properties
  if (NULL == strstr(szBultInList, szMKLFunction)) {
    printf("Device doesn't support %s\nTest skipped\n", szMKLFunction);
    return true;
  }

  // init context should succeed
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  program = clCreateProgramWithBuiltInKernels(context, 1, &device,
                                              szMKLFunction, &err);
  bResult = SilentCheck("clCreateProgramWithBuiltInKernels", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    return bResult;
  }
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  bResult = SilentCheck("clBuildProgram", CL_INVALID_OPERATION, err);

  kernel = clCreateKernel(program, szMKLFunction, &err);
  bResult = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    clReleaseProgram(program);
    return false;
  }

  cl_uint num_args;
  err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &num_args,
                        NULL);
  bResult = SilentCheck("clGetKernelInfo(CL_KERNEL_NUM_ARGS)", CL_SUCCESS, err);
  bResult |= SilentCheck("clGetKernelInfo(CL_KERNEL_NUM_ARGS)=8", 8, num_args);

  size_t arg_size;
  err = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_TYPE_NAME, 0, NULL,
                           &arg_size);
  char *szParamStr = (char *)alloca(arg_size);
  err = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_TYPE_NAME, arg_size,
                           szParamStr, NULL);
  bResult = SilentCheck("clGetKernelArgInfo(1, CL_KERNEL_ARG_TYPE_NAME)",
                        CL_SUCCESS, err);
  bResult = SilentCheckStr("clGetKernelArgInfo(1, CL_KERNEL_ARG_TYPE_NAME)",
                           "CBLAS_ORDER", szParamStr);
  if (!bResult) {
    clReleaseContext(context);
    clReleaseProgram(program);
    return false;
  }

  // create buffer - should succeed
  cl_mem buffA = clCreateBuffer(
      context, CL_MEM_READ_WRITE,
      MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float), NULL, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    return false;
  }

  cl_mem buffB = clCreateBuffer(
      context, CL_MEM_READ_WRITE,
      MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float), NULL, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseMemObject(buffA);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    return false;
  }

  cl_mem buffC = clCreateBuffer(
      context, CL_MEM_READ_WRITE,
      MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float), NULL, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseMemObject(buffA);
    clReleaseMemObject(buffB);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    return false;
  }

  int MNK = MATRIX_DIM_SIZE;
  int order = 101; // CblasRowMajor;
  int trans = 111; // CblasNoTrans;
  float alpha = 1.0f, beta = 0.0f;
  clSetKernelArg(kernel, 0, sizeof(int), &order);
  clSetKernelArg(kernel, 1, sizeof(int), &trans);
  clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffA);
  clSetKernelArg(kernel, 3, sizeof(int), &trans);
  clSetKernelArg(kernel, 4, sizeof(cl_mem), &buffB);
  clSetKernelArg(kernel, 5, sizeof(cl_mem), &buffC);
  clSetKernelArg(kernel, 6, sizeof(float), &alpha);
  clSetKernelArg(kernel, 7, sizeof(float), &beta);

  // init Command Queue - should succeed
  cmd_queue = clCreateCommandQueue(
      context, device, 0 /*CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE*/, &err);
  bResult = SilentCheck("clCreateCommandQueue", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseMemObject(buffA);
    clReleaseMemObject(buffB);
    clReleaseMemObject(buffC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    return bResult;
  }

  float aVal = 1.0f;
  err = clEnqueueFillBuffer(cmd_queue, buffA, &aVal, sizeof(aVal), 0,
                            MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float),
                            0, NULL, NULL);
  bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
  float bVal = 2.0f;
  err = clEnqueueFillBuffer(cmd_queue, buffB, &bVal, sizeof(bVal), 0,
                            MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float),
                            0, NULL, NULL);
  bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
  float cVal = 0.0f;
  err = clEnqueueFillBuffer(cmd_queue, buffC, &cVal, sizeof(cVal), 0,
                            MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float),
                            0, NULL, NULL);
  bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);

  size_t dim[] = {MNK, MNK, MNK};
  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 3, NULL, dim, NULL, 0, NULL,
                               NULL);
  bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);

  float *pResult = (float *)clEnqueueMapBuffer(
      cmd_queue, buffC, CL_TRUE, CL_MAP_READ, 0,
      MATRIX_DIM_SIZE * MATRIX_DIM_SIZE * sizeof(float), 0, NULL, NULL, &err);

  printf("pResult[0]=%.f\n", pResult[0]);
  bResult = (pResult[0] == (MATRIX_DIM_SIZE * 2.0f));

  clEnqueueUnmapMemObject(cmd_queue, buffC, pResult, 0, NULL, NULL);
  clFinish(cmd_queue);
  fflush(stdout);
  clReleaseCommandQueue(cmd_queue);
  clReleaseMemObject(buffA);
  clReleaseMemObject(buffB);
  clReleaseMemObject(buffC);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);

  printf("\n---------------------------------------\n");
  printf("MKL test %s !\n", bResult ? "succeeded" : "failed");
  printf("---------------------------------------\n");

  return bResult;
}

#endif // __IMPORT_MKL__
