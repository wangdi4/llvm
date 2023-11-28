//|
//| TEST: DeviceFissionTest.fissionDeviceInfoSelectorsTest
//|
//| Purpose
//| -------
//|
//| Test clGetDeviceInfo selectors correctness.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_EQUALLY property from root
// device. | 2. Check for support and correctness of all relevant
//"clGetDeviceInfo" selectors, for sub devices.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl_ext.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

bool fission_deviceInfoSelectors_test() {
  printf("---------------------------------------\n");
  printf("fission device info selectors test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
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

  if (numComputeUnits < 5) {
    printf("Not enough compute units, tast passing vacuously\n");
    return true;
  }

  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_devices = 0;
  cl_device_partition_property properties[] = {CL_DEVICE_PARTITION_EQUALLY, 4,
                                               0};
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_device_id param;
  size_t actual_size;
  cl_uint ref;

  err = clGetDeviceInfo(out_devices[0], CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(cl_uint), &ref, &actual_size);
  bResult = SilentCheck("clGetDeviceInfo", CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (4 != ref) {
    printf("FAIL: clGetDeviceInfo\n");
    printf("\t\texpected = %d, result = %d\n", 4, ref);
    return false;
  }

  // CL_DEVICE_PARENT_DEVICE for sub device
  err = clGetDeviceInfo(out_devices[0], CL_DEVICE_PARENT_DEVICE,
                        sizeof(cl_device_id), &param, &actual_size);
  bResult = SilentCheck("clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (device != param) {
    printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE\n");
    printf("\t\texpected = 0x%p, result = 0x%p\n", (void *)device,
           (void *)param);
    return false;
  }

  // CL_DEVICE_PARENT_DEVICE for root device
  err = clGetDeviceInfo(device, CL_DEVICE_PARENT_DEVICE, sizeof(cl_device_id),
                        &param, &actual_size);
  bResult = SilentCheck("clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (NULL != param) {
    printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE\n");
    printf("\t\texpected = 0x0, result = 0x%p\n", (void *)param);
    return false;
  }

  // CL_DEVICE_PARTITION_PROPERTIES
  cl_device_partition_property prop[20];
  err = clGetDeviceInfo(
      out_devices[num_devices - 1], CL_DEVICE_PARTITION_PROPERTIES,
      20 * sizeof(cl_device_partition_property), &prop, &actual_size);
  bResult =
      SilentCheck("clGetDeviceInfo for selector CL_DEVICE_PARTITION_PROPERTIES",
                  CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  if (1 > actual_size) {
    printf(
        "FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_PROPERTIES\n");
    printf("\t\texpected at least one supported property\n");
    return false;
  }
  for (size_t i = 0; i < actual_size / sizeof(cl_device_partition_property);
       i++) {
    switch (prop[i]) {
    case CL_DEVICE_PARTITION_EQUALLY:
    case CL_DEVICE_PARTITION_BY_COUNTS:
    case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
    case CL_DEVICE_PARTITION_BY_NAMES_INTEL:
      break;
    default:
      printf("FAIL: clGetDeviceInfo for selector "
             "CL_DEVICE_PARTITION_PROPERTIES\n");
      printf("\t\tinvalid property\n");
      return false;
    }
  }

  // CL_DEVICE_REFERENCE_COUNT_EXT for sub device
  err = clRetainDevice(out_devices[0]);
  bResult = SilentCheck("clRetainDevice", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clGetDeviceInfo(out_devices[0], CL_DEVICE_REFERENCE_COUNT,
                        sizeof(cl_uint), &ref, &actual_size);
  bResult =
      SilentCheck("clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT",
                  CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  clReleaseDevice(out_devices[0]);

  if (2 != ref) {
    printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT\n");
    printf("\t\texpected = 2, result = %d\n", ref);
    return false;
  }

  // CL_DEVICE_REFERENCE_COUNT for root device
  err = clRetainDevice(device);
  bResult = SilentCheck("clRetainDevice", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clGetDeviceInfo(device, CL_DEVICE_REFERENCE_COUNT, sizeof(cl_uint),
                        &ref, &actual_size);
  bResult =
      SilentCheck("clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT",
                  CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  if (1 != ref) {
    printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT\n");
    printf("\t\texpected = 1, result = %d\n", ref);
    return false;
  }

  // CL_DEVICE_PARTITION_TYPE for root device
  err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_TYPE,
                        20 * sizeof(cl_device_partition_property), prop,
                        &actual_size);
  bResult = SilentCheck("clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPE",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  if (!((0 == actual_size) ||
        ((sizeof(cl_device_partition_property) == actual_size) &&
         (0 == prop[0])))) {
    printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPE\n");
    printf("\t\texpected size of zero or a single NULL property: "
           "%" PRIiPTR ", result: %zu properties - %" PRIiPTR "\n",
           properties[0], actual_size, prop[0]);

    return false;
  }

  // CL_DEVICE_PARTITION_TYPE for sub device
  err = clGetDeviceInfo(out_devices[0], CL_DEVICE_PARTITION_TYPE,
                        20 * sizeof(cl_device_partition_property), prop,
                        &actual_size);
  bResult = SilentCheck("clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPE",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  for (size_t i = 0; i < actual_size / sizeof(cl_device_partition_property);
       ++i) {
    if (prop[i] != properties[i]) {
      printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPE\n");
      printf("\t\texpected = %" PRIiPTR ", result = %" PRIiPTR "\n",
             properties[i], prop[i]);
      return false;
    }
  }
  printf("\n---------------------------------------\n");
  printf("fission device info selectors test succeeded!\n");
  printf("---------------------------------------\n");

  for (size_t i = 0; i < num_devices; i++) {
    clReleaseDevice(out_devices[i]);
  }

  return bResult;
}
