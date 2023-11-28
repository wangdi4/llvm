//|
//| TEST: DeviceFissionTest.fissionByNamesTest
//|
//| Purpose
//| -------
//|
//| Test the ability to execute commands on two command queues for the same
// sub-device.
//|
//| Method
//| ------
//|
//| 1. Create a sub device with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Create two command queues on the sub-device. | 3. Execute a
// kernel on the first command queue and release it. Do the same for the second
// command queue. | 4. read the results.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl_ext.h"
#include "FrameworkTest.h"
#include <stdio.h>

#define WORK_SIZE 1
#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

bool fission_by_names_test() {
  printf("---------------------------------------\n");
  printf("fission by names test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_device_id device = NULL;
  cl_platform_id platform = NULL;
  cl_int err;

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
    printf("Not enough compute units to create sub-device, tast passing "
           "vacuously\n");
    return true;
  }
  cl_device_id subdevice_id;
  cl_uint num_entries = 1;
  cl_uint num_devices = 1;
  cl_device_partition_property properties[] = {
      (cl_device_partition_property)CL_DEVICE_PARTITION_BY_NAMES_INTEL, 0,
      (cl_device_partition_property)CL_PARTITION_BY_NAMES_LIST_END_INTEL, 0};

  err = clCreateSubDevices(device, properties, num_entries, &subdevice_id,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Create a context with the sub-device
  context = clCreateContext(NULL, 1, &subdevice_id, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Create a command queue for the device
  cmd_queue =
      clCreateCommandQueueWithProperties(context, subdevice_id, NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties - first queue",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  const char *ocl_test_program = "__kernel void writeSeven(__global char "
                                 "*a) { a[get_global_id(0)] = 7; }";

  cl_kernel kernel;
  cl_program program;

  program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &err);
  bResult = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  cl_build_status build_status;
  err |= clGetProgramBuildInfo(program, subdevice_id, CL_PROGRAM_BUILD_STATUS,
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
    return false;
  }

  kernel = clCreateKernel(program, "writeSeven", &err);
  bResult = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  char input[WORK_SIZE];
  memset(input, 0, WORK_SIZE);
  cl_mem buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                              WORK_SIZE, input, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
  bResult = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  size_t globalSize = WORK_SIZE;
  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, &globalSize, NULL, 0,
                               NULL, NULL);
  bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clEnqueueReadBuffer(cmd_queue, buf, CL_TRUE, 0, WORK_SIZE, input, 0,
                            NULL, NULL);
  bResult = SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  for (size_t i = 0; i < WORK_SIZE; ++i) {
    if (7 != input[i]) {
      return false;
    }
  }
  err = clReleaseCommandQueue(cmd_queue);
  bResult &= SilentCheck("clReleaseCommandQueue", CL_SUCCESS, err);
  err = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram", CL_SUCCESS, err);
  err = clReleaseKernel(kernel);
  bResult &= SilentCheck("clReleaseKernel", CL_SUCCESS, err);
  err = clReleaseMemObject(buf);
  bResult &= SilentCheck("clReleaseMemObject", CL_SUCCESS, err);
  err = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext", CL_SUCCESS, err);
  err = clReleaseDevice(subdevice_id);
  bResult &= SilentCheck("clReleaseDevice", CL_SUCCESS, err);

  if (bResult) {
    printf("---------------------------------------\n");
    printf("fission by names test passed\n");
    printf("---------------------------------------\n");
  }

  return bResult;
}
