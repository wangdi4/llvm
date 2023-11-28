//|
//| TEST: DeviceFissionTest.fissionBasicTest
//|
//| Purpose
//| -------
//|
//| Test basic usage of a device fission.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Validate the number of compute units in the sub device. | 3.
// Enqueue execution of a kernel on the sub device. | 4. read the results.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

#define WORK_SIZE 1
#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

struct preferences {
  unsigned int likes_ice_cream;
  unsigned int plays_golf;
  unsigned int watches_tv;
  unsigned int reads_books;
};

struct foo {
  char flag;
  char counter;
};

bool run_kernel(cl_context &context, cl_device_id &device,
                cl_command_queue &cmd_queue, const char *prog,
                struct preferences *out1, struct foo *out2) {
  cl_int err;
  cl_kernel kernel;
  cl_program program;
  bool res;

  program =
      clCreateProgramWithSource(context, 1, (const char **)&prog, NULL, &err);
  res = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!res)
    return res;

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  cl_build_status build_status;
  err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                               MAX_SOURCE_SIZE, &build_status, NULL);
  if (CL_SUCCESS != err || CL_BUILD_ERROR == build_status) {
    printf("\n build status is: %d \n", build_status);
    char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
    char *err_str_ptr = err_str;
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                MAX_SOURCE_SIZE, err_str_ptr, NULL);
    if (err != CL_SUCCESS)
      printf("Build Info error: %d \n", err);
    printf("%s \n", err_str_ptr);
    return res;
  }

  kernel = clCreateKernel(program, "fissionBasicTest", &err);
  res = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseProgram(program);
    return res;
  }

  cl_mem buff1 = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                sizeof(struct preferences), NULL, &err);
  res = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!res) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  cl_mem buff2 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(struct foo),
                                NULL, &err);
  res = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!res) {
    clReleaseMemObject(buff1);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buff1);
  res = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!res) {
    clReleaseMemObject(buff1);
    clReleaseMemObject(buff2);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&buff2);
  res = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!res) {
    clReleaseMemObject(buff1);
    clReleaseMemObject(buff2);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  size_t global_work_size[1];
  global_work_size[0] = WORK_SIZE;

  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                               NULL, 0, NULL, NULL);
  res = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseMemObject(buff1);
    clReleaseMemObject(buff2);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }
  out1->likes_ice_cream = 0;
  out1->plays_golf = 0;
  out1->reads_books = 0;
  out1->watches_tv = 0;
  out2->counter = 0;
  out2->flag = 0;
  err = clEnqueueReadBuffer(cmd_queue, buff1, CL_TRUE, 0,
                            sizeof(struct preferences), out1, 0, NULL, NULL);
  err = clEnqueueReadBuffer(cmd_queue, buff2, CL_TRUE, 0, sizeof(struct foo),
                            out2, 0, NULL, NULL);

  clFinish(cmd_queue);
  clReleaseMemObject(buff1);
  clReleaseMemObject(buff2);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  return ((CL_SUCCESS == err) ? true : false);
}

bool fission_basic_test() {
  printf("---------------------------------------\n");
  printf("fission basic test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_device_id device = NULL;
  cl_int err;
  cl_platform_id platform = NULL;

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult = SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...)
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult = SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_uint numComputeUnits;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
                        &numComputeUnits, NULL);
  bResult = SilentCheck("clGetDeviceInfo(CL_DEVICE_MAX_COMPUTE_UNITS)",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  if (numComputeUnits < 4) {
    printf("Not enough compute units, tast passing vacuously\n");
    return true;
  }

  err = clRetainDevice(device);
  bResult = SilentCheck("clRetainDevice", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clReleaseDevice(device);
  bResult = SilentCheck("clReleaseDevice", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_devices = 2;
  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_BY_COUNTS, 2,
                                               1, 0, 0};
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_uint param;
  size_t actual_size;
  err = clGetDeviceInfo(out_devices[0], CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(cl_uint), &param, &actual_size);
  bResult = SilentCheck("clGetDeviceInfo", CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (2 != param) {
    printf("FAIL: clGetDeviceInfo\n");
    printf("\t\texpected = %d, result = %d\n", 2, param);
    return false;
  }

  // init context
  context = clCreateContext(NULL, 1, &out_devices[0], NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  size_t pSize;
  char param_val[100];
  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, 100, param_val, &pSize);
  bResult = SilentCheck("clGetContextInfo", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Command Queue
  cmd_queue =
      clCreateCommandQueueWithProperties(context, out_devices[0], NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    return bResult;
  }

  clGetCommandQueueInfo(cmd_queue, CL_QUEUE_DEVICE, 100, param_val, &pSize);
  bResult = SilentCheck("clGetCommandQueueInfo", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    return bResult;
  }

  const char *ocl_test_program = {"struct preferences {\
    unsigned int likes_ice_cream;\
    unsigned int plays_golf;\
    unsigned int watches_tv;\
    unsigned int reads_books;\
    };\
    struct foo {\
    char flag;\
    char counter;\
    };\
    __kernel void fissionBasicTest(__global struct preferences  *fred, __global struct foo *my_foo)\
    {size_t s = 7;\
        my_foo->flag = 1;}"};

  struct preferences p;
  struct foo f;

  bResult =
      run_kernel(context, out_devices[0], cmd_queue, ocl_test_program, &p, &f);
  printf("\n---------------------------------------\n");
  printf("fission basic test succeeded!\n");
  printf("---------------------------------------\n");

  clFinish(cmd_queue);
  fflush(stdout);
  clReleaseCommandQueue(cmd_queue);
  clReleaseContext(context);
  for (size_t i = 0; i < num_devices; i++) {
    clReleaseDevice(out_devices[i]);
  }
  return bResult;
}
