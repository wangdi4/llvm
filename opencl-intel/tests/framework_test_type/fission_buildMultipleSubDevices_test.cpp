//|
//| TEST: DeviceFissionTest.fissionBuildMultipleSubDevicesTest
//|
//| Purpose
//| -------
//|
//| Test build capability on multiple sub devices.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS property from root
// device. | 2. Build a program on both sub devices.
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

#if __GNUC__ > 7
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif

extern cl_device_type gDeviceType;

static volatile unsigned int uiBuildDone = 0;

static void CL_CALLBACK notifyBuildDone(cl_program program, void *user_data) {
  ++uiBuildDone;
}

bool fission_buildMultipleSubDevices_test() {

  const char *ocl_test_program = "\
    __kernel void fissionBuildTest(__global int* pBuff)\n\
    {\n\
        size_t id = get_global_id(0);\n\
        pBuff[id] = id;\n\
    }\n\
    ";

  uiBuildDone = false;

  printf("---------------------------------------\n");
  printf("fission build multiple sub devices test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_device_id device = NULL;
  cl_program program = NULL;
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

  cl_uint num_entries = 2;
  cl_device_id out_devices[2];
  cl_uint num_devices = 0;
  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_BY_COUNTS, 1,
                                               1, 0};
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init context
  context = clCreateContext(NULL, 2, out_devices, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // create program
  program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &err);
  bResult = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    return bResult;
  }

  err = clBuildProgram(program, 2, out_devices, NULL, &notifyBuildDone, NULL);
  bResult = SilentCheck("clBuildProgram", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseProgram(program);
    clReleaseContext(context);
    return bResult;
  }

  while (!uiBuildDone) {
  }

  // Check that program was built for both devices
  cl_build_status build_status;

  for (int i = 0; i < 2; ++i) {
    err =
        clGetProgramBuildInfo(program, out_devices[i], CL_PROGRAM_BUILD_STATUS,
                              MAX_SOURCE_SIZE, &build_status, NULL);
    if (CL_SUCCESS != err || CL_BUILD_SUCCESS != build_status) {
      printf("\n build status for device %d is: %d \n", i, build_status);

      clReleaseProgram(program);
      clReleaseContext(context);

      return false;
    }
  }

  clReleaseProgram(program);
  clReleaseContext(context);
  for (size_t i = 0; i < num_devices; i++) {
    clReleaseDevice(out_devices[i]);
  }

  if (uiBuildDone != 1) {
    printf("\n notify was called more than once (%u)\n", uiBuildDone);

    return false;
  }

  printf("\n------------------------------------------------\n");
  printf("fission build multiple sub devices test succeeded!\n");
  printf("--------------------------------------------------\n");

  fflush(stdout);

  return true;
}
