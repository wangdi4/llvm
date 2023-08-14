//|
//| TEST: DeviceFissionTest.fissionReadBufferBetweenDevicesTest
//|
//| Purpose
//| -------
//|
//| Test reading from one sub-device when the buffer is located on another
// sub-device
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Validate the number of compute units in the sub device. | 3.
// Enqueue execution of a kernel on one sub device. | 4. read the result from
// the other sub-device.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

#define WORK_SIZE 1
#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

bool fission_read_buffer_between_device_test() {
  printf("---------------------------------------\n");
  printf("fission read buffer between devices test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue[2] = {NULL, NULL};
  cl_device_id device = NULL;
  cl_program program = NULL;
  cl_kernel kernel = NULL;
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
  if (!bResult) {
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }
  if (2 != param) {
    printf("FAIL: clGetDeviceInfo\n");
    printf("\t\texpected = %d, result = %d\n", 2, param);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return false;
  }

  // init context
  context = clCreateContext(NULL, 2, out_devices, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult) {
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }

  // init Command Queues
  cmd_queue[0] =
      clCreateCommandQueueWithProperties(context, out_devices[0], NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }
  cmd_queue[1] =
      clCreateCommandQueueWithProperties(context, out_devices[1], NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }

  const char *ocl_test_program[] = {
      "__kernel void writeThree(__global int *a) { a[0] = 3; }"};

  program = clCreateProgramWithSource(
      context, 1, (const char **)ocl_test_program, NULL, &err);
  bResult = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseCommandQueue(cmd_queue[1]);
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  cl_build_status build_status;
  err |= clGetProgramBuildInfo(program, out_devices[0], CL_PROGRAM_BUILD_STATUS,
                               MAX_SOURCE_SIZE, &build_status, NULL);
  if (CL_SUCCESS != err || CL_BUILD_ERROR == build_status) {
    printf("\n build status is: %d \n", build_status);
    char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
    char *err_str_ptr = err_str;
    err = clGetProgramBuildInfo(program, out_devices[0], CL_PROGRAM_BUILD_LOG,
                                MAX_SOURCE_SIZE, err_str_ptr, NULL);
    if (err != CL_SUCCESS)
      printf("Build Info error: %d \n", err);
    printf("%s \n", err_str_ptr);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmd_queue[1]);
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return false;
  }

  kernel = clCreateKernel(program, "writeThree", &err);
  bResult = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseProgram(program);
    clReleaseCommandQueue(cmd_queue[1]);
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }

  cl_mem buf =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmd_queue[1]);
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buf);
  bResult = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseMemObject(buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmd_queue[1]);
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }

  size_t global_work_size[] = {WORK_SIZE};

  err = clEnqueueNDRangeKernel(cmd_queue[0], kernel, 1, NULL, global_work_size,
                               NULL, 0, NULL, NULL);
  bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseMemObject(buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmd_queue[1]);
    clReleaseCommandQueue(cmd_queue[0]);
    clReleaseContext(context);
    for (size_t i = 0; i < num_devices; i++) {
      clReleaseDevice(out_devices[i]);
    }
    return bResult;
  }
  clFinish(cmd_queue[0]);
  int three;
  err = clEnqueueReadBuffer(cmd_queue[1], buf, CL_TRUE, 0, sizeof(int), &three,
                            0, NULL, NULL);
  bResult = SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);

  clReleaseMemObject(buf);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(cmd_queue[1]);
  clReleaseCommandQueue(cmd_queue[0]);
  clReleaseContext(context);

  bResult &= (3 == three);
  if (!bResult)
    return bResult;

  printf("\n---------------------------------------\n");
  printf("fission basic test succeeded!\n");
  printf("---------------------------------------\n");

  for (size_t i = 0; i < num_devices; i++) {
    clReleaseDevice(out_devices[i]);
  }
  return bResult;
}
