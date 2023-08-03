//|
//| TEST: DeviceFissionTest.fissionContextTest
//|
//| Purpose
//| -------
//|
//| Test device fission after context.
//|
//| Method
//| ------
//|
//| 1. Create context on the root device.
//| 2. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 3. Release the context and the sub devices.
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

bool fission_context_test() {
  printf("---------------------------------------\n");
  printf("fission context test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
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

  // init context
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_devices = 0;

  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_BY_COUNTS,
                                               (int)numComputeUnits - 1, 0, 0};
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  fflush(stdout);
  err = clReleaseContext(context);
  bResult = SilentCheck("clReleaseContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  for (size_t i = 0; i < num_devices; i++) {
    err = clReleaseDevice(out_devices[i]);
    bResult = SilentCheck("clReleaseDevice", CL_SUCCESS, err);
  }
  printf("\n---------------------------------------\n");
  printf("fission context test succeeded!\n");
  printf("---------------------------------------\n");
  return bResult;
}