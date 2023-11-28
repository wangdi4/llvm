//|
//| TEST: DeviceFissionTest.fissionLogicTest
//|
//| Purpose
//| -------
//|
//| Test logic usage of a device fission.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Create sub devices with CL_DEVICE_PARTITION_EQUALLY property
// from the first sub device. | 3. Enqueue execution of 2 different kernels:
// first kernel on one sub device, |    second kernel on the rest sub devices.
// | 4. read and validate the results.
//|
//| Pass criteria
//| -------------
//|
//| The data was correctly copied into the output buffer in each kernel.
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

#define WORK_SIZE 1
#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

bool run_kernel1(cl_context &context, cl_device_id &device,
                 cl_command_queue &cmd_queue, const char *prog, int *out) {
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

  kernel = clCreateKernel(program, "fissionLogic1Test", &err);
  res = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseProgram(program);
    return res;
  }

  cl_mem buff =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
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
  *out = 0;

  err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(int), out, 0,
                            NULL, NULL);
  err += (*out == 7);
  clFinish(cmd_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  return ((CL_SUCCESS == err) ? true : false);
}

bool run_kernel2(cl_context &context, cl_device_id &device,
                 cl_command_queue &cmd_queue, const char *prog, int *out) {
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

  kernel = clCreateKernel(program, "fissionLogic2Test", &err);
  res = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseProgram(program);
    return res;
  }

  cl_mem buff =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
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
  *out = 0;

  err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(int), out, 0,
                            NULL, NULL);
  err += (*out == 3);
  clFinish(cmd_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  return ((CL_SUCCESS == err) ? true : false);
}
#include <iostream>
using namespace std;
bool fission_logic_test() {
  printf("---------------------------------------\n");
  printf("fission logic test\n");
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

  if (numComputeUnits < 7) {
    printf("Not enough compute units, tast passing vacuously\n");
    return true;
  }

  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_level1_devices = 2;
  cl_uint num_level2_devices = 2;

  cl_device_partition_property properties1[] = {
      CL_DEVICE_PARTITION_BY_COUNTS, 4, 2,
      CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0};
  cl_device_partition_property properties2[] = {CL_DEVICE_PARTITION_EQUALLY, 2,
                                                0};

  err = clCreateSubDevices(device, properties1, num_entries, out_devices,
                           &num_level1_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clCreateSubDevices(out_devices[0], properties2, num_entries,
                           (out_devices + num_level1_devices),
                           &num_level2_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init context
  context = clCreateContext(NULL, num_level1_devices + num_level2_devices,
                            out_devices, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  const char *ocl_test1_program = {
      "__kernel void fissionLogic1Test(__global int *d)\
    {d = 7;}"};

  const char *ocl_test2_program = {
      "__kernel void fissionLogic2Test(__global int *d)\
    {d = 3;}"};

  // init Command Queue for the lowest level devices
  for (size_t i = 0; i < num_level2_devices; i++) {
    cmd_queue[i] = clCreateCommandQueueWithProperties(
        context, out_devices[num_level1_devices + i], NULL, &err);
    bResult =
        SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
    if (!bResult) {
      for (size_t j = 0; j < i; ++j) {
        clReleaseCommandQueue(cmd_queue[i]);
      }
      for (size_t j = 0; j < num_level1_devices + num_level2_devices; ++j) {
        clReleaseDevice(out_devices[j]);
      }
      clReleaseContext(context);
      return bResult;
    }
  }

  int res1, res2;

  bResult |= run_kernel1(context, out_devices[num_level1_devices], cmd_queue[0],
                         ocl_test1_program, &res1);

  for (size_t i = 1; i < num_level2_devices; i++) {
    bResult |= run_kernel2(context, out_devices[num_level1_devices + i],
                           cmd_queue[i], ocl_test2_program, &res2);
  }
  const char *res = ((bResult == true) ? "succeed" : "failed");
  printf("\n---------------------------------------\n");
  printf("fission logic test %s!\n", res);
  printf("---------------------------------------\n");

  for (size_t i = 0; i < num_level2_devices; i++) {
    clFinish(cmd_queue[i]);
    clReleaseCommandQueue(cmd_queue[i]);
  }
  clReleaseContext(context);
  for (size_t i = 0; i < num_level1_devices + num_level2_devices; i++) {
    clReleaseDevice(out_devices[i]);
  }
  return bResult;
}
