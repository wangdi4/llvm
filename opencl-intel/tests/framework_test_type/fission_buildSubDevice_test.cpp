//|
//| TEST: DeviceFissionTest.fissionBuildSubDeviceTest
//|
//| Purpose
//| -------
//|
//| Test build capability on sub devices.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Build a program on one of the sub devices.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

bool run_kernel(cl_context &context, cl_device_id &device, const char *prog,
                int *out) {
  cl_int err;
  cl_program program;
  bool res;

  program =
      clCreateProgramWithSource(context, 1, (const char **)&prog, NULL, &err);
  res = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!res)
    return res;

  err = clBuildProgram(program, 0, &device, NULL, NULL, NULL);
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
  clReleaseProgram(program);
  return res;
}

bool fission_buildSubDevice_test() {
  printf("---------------------------------------\n");
  printf("fission build sub device test\n");
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

  if (numComputeUnits < 2) {
    printf("Not enough compute units, tast passing vacuously\n");
    return true;
  }

  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_devices = 0;
  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_BY_COUNTS, 1,
                                               0, 0};
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init context
  context = clCreateContext(NULL, 1, &out_devices[0], NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
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

  const char *ocl_test_program = {
      "__kernel void fissionBuildTest(__global int *dummy)\
    {size_t s = 7;}"};

  int p;

  bResult = run_kernel(context, out_devices[0], ocl_test_program, &p);
  printf("\n---------------------------------------\n");
  printf("fission build sub device test succeeded!\n");
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
