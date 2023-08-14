//|
//| TEST: FrameworkTest.predictablePartitionTest
//|
//| Purpose
//| -------
//|
//| Test correctness of execution in "predictable partitioning" mode
//|
//| Method
//| ------
//|
//| 1. Create sub-devices of size 1..#cores
//| 2. On each sub-device, execute a kernel several times and validate the
// results
//|
//| Pass criteria
//| -------------
//|
//| The kernel returns the correct results during all iterations on all
// sub-devices

#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

#define ITERATIONS_PER_SUBDEVICE 50
#define MAX_SOURCE_SIZE 2048
#define MAX_PARTITION_PROPS 50

extern cl_device_type gDeviceType;

bool test_subdevice(cl_device_id subdevice_id, cl_uint subdevice_size,
                    bool useFinish) {
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_int err;
  bool bResult;

  size_t globalSize = (size_t)subdevice_size;
  size_t localSize = 1;

  // Create a context with the sub-device
  context = clCreateContext(NULL, 1, &subdevice_id, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Create a command queue for the device
  cmd_queue =
      clCreateCommandQueueWithProperties(context, subdevice_id, NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Todo: should extend this test to also ensure which cores work groups
  // execute on. Relies on a debug built-in
  const char *ocl_test_program = "__kernel void writeTID(__global int *a) { "
                                 "a[get_global_id(0)] = get_global_id(0); }";

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
    err = clGetProgramBuildInfo(program, subdevice_id, CL_PROGRAM_BUILD_LOG,
                                MAX_SOURCE_SIZE, err_str_ptr, NULL);
    if (err != CL_SUCCESS)
      printf("Build Info error: %d \n", err);
    printf("%s \n", err_str_ptr);
    return false;
  }

  kernel = clCreateKernel(program, "writeTID", &err);
  bResult = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  int *input = new int[subdevice_size];
  memset(input, 0xFF, sizeof(int) * subdevice_size);
  cl_mem buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                              sizeof(int) * subdevice_size, input, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult) {
    delete[] input;
    return bResult;
  }

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
  bResult = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!bResult) {
    delete[] input;
    return bResult;
  }

  for (cl_uint i = 0; i < ITERATIONS_PER_SUBDEVICE; ++i) {
    cl_event evt;
    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, &globalSize,
                                 &localSize, 0, NULL, &evt);
    bResult = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
    if (!bResult) {
      delete[] input;
      return bResult;
    }
    if (useFinish) {
      err = clFinish(cmd_queue);
      bResult = SilentCheck("clFinish", CL_SUCCESS, err);
    } else {
      err = clWaitForEvents(1, &evt);
      bResult = SilentCheck("clWaitForEvents", CL_SUCCESS, err);
    }
    clReleaseEvent(evt);
    if (!bResult) {
      delete[] input;
      return bResult;
    }
  }

  err = clEnqueueReadBuffer(cmd_queue, buf, CL_TRUE, 0,
                            sizeof(int) * subdevice_size, input, 0, NULL, NULL);
  bResult = SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);
  if (!bResult) {
    delete[] input;
    return bResult;
  }

  for (cl_uint i = 0; i < subdevice_size; ++i) {
    if ((int)i != input[i]) {
      printf("Error encountered at index %u\n", i);
      delete[] input;
      return false;
    }
  }
  delete[] input;

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

  return bResult;
}

bool predictable_partition_test() {
  printf("---------------------------------------\n");
  printf("predictable partition test\n");
  printf("---------------------------------------\n");

  cl_int err;
  bool bResult;
  cl_platform_id platform = NULL;
  cl_device_id device, sub_device;
  size_t paramSize;

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult = SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init device
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

  err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_PROPERTIES, 0, NULL,
                        &paramSize);
  bResult = SilentCheck("clGetDeviceInfo(CL_DEVICE_PARTITION_PROPERTIES)",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (0 == paramSize) {
    printf("Device doesn't support fission. Test passing vacuously\n");
    return true;
  }

  cl_device_partition_property props[MAX_PARTITION_PROPS] = {0};
  err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_PROPERTIES, sizeof(props),
                        props, NULL);
  bResult = SilentCheck("clGetDeviceInfo(CL_DEVICE_PARTITION_PROPERTIES)",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (0 == props[0]) {
    printf("Device doesn't support fission. Test passing vacuously\n");
    return true;
  }

  bool bCountsSupported = false;
  for (size_t prop = 0; prop < paramSize / sizeof(cl_device_partition_property);
       ++prop) {
    if (CL_DEVICE_PARTITION_BY_COUNTS == props[prop]) {
      bCountsSupported = true;
      break;
    }
  }

  if (false == bCountsSupported) {
    printf(
        "Device doesn't support fission BY_COUNTS. Test passing vacuously\n");
    return true;
  }

  // Start the actual test
  props[0] = CL_DEVICE_PARTITION_BY_COUNTS;
  props[2] = CL_DEVICE_PARTITION_BY_COUNTS_LIST_END;
  props[3] = 0;

  bool bUseFinish = false;

  for (cl_uint device_size = 1; device_size <= numComputeUnits; ++device_size) {
    props[1] = device_size;
    err = clCreateSubDevices(device, props, 1, &sub_device, NULL);
    bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
    if (!bResult)
      return bResult;

    bResult = test_subdevice(sub_device, device_size, bUseFinish);
    if (!bResult) {
      printf("Test failed for size %u, using %s\n", device_size,
             bUseFinish ? "clFinish" : "clWaitForEvents");
    }
    err = clReleaseDevice(sub_device);
    bResult &= SilentCheck("clReleaseDevice", CL_SUCCESS, err);
    if (!bResult)
      return bResult;

    // Alternate between with and without finish
    bUseFinish = !bUseFinish;
  }

  if (bResult) {
    printf("---------------------------------------\n");
    printf("predictable partition test passed\n");
    printf("---------------------------------------\n");
  }

  return bResult;
}
