//|
//| TEST: DeviceFissionTest.ReadBufferTest
//|
//| Purpose
//| -------
//|
//| Test read buffer from a sub-device
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Create two buffers | 3. Write to the first buffer on the
// sub-device and read from the second | 4. Verify the second buffer was updated
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

bool fission_read_buffer_test() {
  printf("---------------------------------------\n");
  printf("fission read buffer test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_device_id devices[2];
  cl_mem buf;
  cl_int err;
  cl_platform_id platform = NULL;

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...)
  err = clGetDeviceIDs(platform, gDeviceType, 1, devices, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_uint num_entries = 1;
  cl_uint num_devices = 1;
  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_BY_COUNTS, 1,
                                               0, 0};
  err = clCreateSubDevices(devices[0], properties, num_entries, devices + 1,
                           &num_devices);
  bResult &= SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init context
  context = clCreateContext(NULL, 2, devices, NULL, NULL, &err);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Command Queue
  cmd_queue =
      clCreateCommandQueueWithProperties(context, devices[1], NULL, &err);
  bResult &= SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    clReleaseDevice(devices[1]);
    return bResult;
  }

  // Create buffers
  cl_uint data[2] = {0xABCD, 0x1234};
  // buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
  // sizeof(cl_uint), data, &err);
  buf = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_uint), NULL, &err);
  bResult &= SilentCheck("clCreateBuffer #1", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseCommandQueue(cmd_queue);
    clReleaseContext(context);
    clReleaseDevice(devices[1]);
    return bResult;
  }

  err = clEnqueueWriteBuffer(cmd_queue, buf, CL_FALSE, 0, sizeof(cl_uint), data,
                             0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer", CL_SUCCESS, err);
  err = clEnqueueReadBuffer(cmd_queue, buf, CL_TRUE, 0, sizeof(cl_uint),
                            data + 1, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);

  clReleaseMemObject(buf);
  clReleaseCommandQueue(cmd_queue);
  clReleaseContext(context);
  clReleaseDevice(devices[1]);
  if (!bResult)
    return bResult;

  return data[0] == data[1];
}
