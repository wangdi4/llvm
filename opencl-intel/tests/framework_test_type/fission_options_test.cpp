//|
//| TEST: DeviceFissionTest.fissionOptionsTest
//|
//| Purpose
//| -------
//|
//| Test different options of usage of a device fission.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_EQUALLY property from root
// device. | 2. Check that the number of devices creates <= max number of
// devices (num_entries >= num_devices) | 3. Create context which includes sub
// devices as part of the context devices list. | 4. Calling
// clgetProgramBuildInfo on the created sub devices. | 5. Enqueue execution of a
// kernel on all the created sub devices. | 6. read the results.
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

struct dummy {
  unsigned int ui;
  char c;
};

bool run_kernel(cl_context &context, cl_device_id &device,
                cl_command_queue &cmd_queue, const char *prog,
                struct dummy *out1) {
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

  kernel = clCreateKernel(program, "fissionOptionsTest", &err);
  res = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseProgram(program);
    return res;
  }

  cl_mem buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(struct dummy),
                               NULL, &err);
  res = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!res) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buff);
  res = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!res) {
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
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }
  out1->c = 0;
  out1->ui = 0;

  err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(struct dummy),
                            out1, 0, NULL, NULL);

  clFinish(cmd_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseMemObject(buff);
  return ((CL_SUCCESS == err) ? true : false);
}

bool fission_options_test() {
  printf("---------------------------------------\n");
  printf("fission options test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue[10] = {NULL};
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

  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_devices = 2;
  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_EQUALLY, 3,
                                               0};
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  // requirement: num_entries >= num_devices
  err = ((num_entries >= num_devices) ? CL_SUCCESS : CL_INVALID_VALUE);
  bResult = SilentCheck("clCreateSubDevices: num_entries < num_devices",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init context
  context = clCreateContext(NULL, num_devices, out_devices, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Command Queue
  for (size_t i = 0; i < num_devices; i++) {
    cmd_queue[i] =
        clCreateCommandQueueWithProperties(context, out_devices[i], NULL, &err);
    bResult =
        SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
    if (!bResult) {
      for (size_t j = 0; j < i; j++)
        clReleaseCommandQueue(cmd_queue[j]);
      clReleaseContext(context);
      return bResult;
    }
  }

  const char *ocl_test_program = {"struct dummy {\
    unsigned int ui;\
    char c;\
    };\
    __kernel void fissionOptionsTest(__global struct dummy *d)\
    {d->ui++;\
     d->c = 1;}"};

  struct dummy p;

  for (size_t i = 0; i < num_devices; i++) {
    bResult |=
        run_kernel(context, out_devices[i], cmd_queue[i], ocl_test_program, &p);
  }
  const char *res = ((bResult == true) ? "succeeded" : "failed");
  printf("\n---------------------------------------\n");
  printf("fission options test %s!\n", res);
  printf("---------------------------------------\n");

  for (size_t i = 0; i < num_devices; i++) {
    clFinish(cmd_queue[i]);
    clReleaseCommandQueue(cmd_queue[i]);
  }
  clReleaseContext(context);
  for (size_t i = 0; i < num_devices; i++) {
    clReleaseDevice(out_devices[i]);
  }
  return bResult;
}
