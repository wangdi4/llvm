//|
//| TEST: DeviceFissionTest.fissionSubdivisionTest
//|
//| Purpose
//| -------
//|
//| Test that devices can be re-partitioned repeateadly.
//|
//| Method
//| ------
//|
//| 1. Query the number of compute units available in the root-level device.
//| 2. Divide the root-level device in several manners, ensure only the legal
// ones pass. | 3. Attempt to create command queues on overlapping sub-devices
// and ensure failure.
//|
//| Pass criteria
//| -------------
//|
//| Legal subdivisions pass. Illegal subdivisions fail. Attempts to allocate
// more compute units than are available fail. | Return true in case of SUCCESS.

#include "CL/cl_ext.h"
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

bool fission_subdivision_test() {
  printf("---------------------------------------\n");
  printf("fission subdivision test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue[2];
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

  if (numComputeUnits < 3) {
    printf(
        "Not enough compute units to divide twice, tast passing vacuously\n");
    return true;
  }
  size_t actual_size;
  cl_device_partition_property prop[20];
  err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_AFFINITY_DOMAIN,
                        20 * sizeof(cl_device_partition_property), prop,
                        &actual_size);
  bResult = SilentCheck(
      "clGetDeviceInfo for selector CL_DEVICE_PARTITION_AFFINITY_DOMAIN",
      CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_uint next_subdevice_index = 0;
  cl_uint num_entries = 100;
  cl_device_id out_devices[100];
  cl_uint num_devices = 1;
  cl_device_partition_property properties[100];

  // First test: attempt to create a device that's the entire parent. This
  // should fail.
  properties[0] = CL_DEVICE_PARTITION_EQUALLY;
  properties[1] = numComputeUnits;
  properties[2] = 0;

  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices - attempt to create a subdevice "
                        "equal to a root level device",
                        CL_DEVICE_PARTITION_FAILED, err);
  if (!bResult)
    return bResult;

  // Create a sub-device of the root level device
  properties[1] = numComputeUnits - 1;
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices - attempt to create a subdevice "
                        "with one less compute unit",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  ++next_subdevice_index;

  // Second test: attempt to get a one-sized sub-device of the parent device
  properties[0] = CL_DEVICE_PARTITION_BY_COUNTS;
  properties[1] = 1;
  properties[2] = 0;
  properties[3] = 0;
  err = clCreateSubDevices(out_devices[0], properties,
                           num_entries - next_subdevice_index,
                           out_devices + next_subdevice_index, &num_devices);
  bResult = SilentCheck("clCreateSubDevices - attempt to create a child of a "
                        "sub-device with one compute unit",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  ++next_subdevice_index;

  // Third test: attempt to mix BY_NAMES with other partitioning modes.
  properties[0] = CL_DEVICE_PARTITION_BY_NAMES_INTEL;
  properties[1] = 0;
  properties[2] = CL_PARTITION_BY_NAMES_LIST_END_INTEL;
  properties[3] = 0;
  err = clCreateSubDevices(out_devices[0], properties,
                           num_entries - next_subdevice_index,
                           out_devices + next_subdevice_index, &num_devices);
  bResult = SilentCheck("clCreateSubDevices - attempt to create a BY_NAMES "
                        "child of a BY_COUNTS subdevice",
                        CL_DEVICE_PARTITION_FAILED, err);
  if (!bResult)
    return bResult;

  // Release devices acquired in the previous phase of testing
  for (unsigned int subdevice = 0; subdevice < next_subdevice_index;
       ++subdevice) {
    err = clReleaseDevice(out_devices[subdevice]);
    bResult &= SilentCheck("clReleaseDevice", CL_SUCCESS, err);
  }
  if (!bResult)
    return bResult;
  next_subdevice_index = 0;

  // Create two sub-devices that must alias compute units
  num_devices = 1;
  properties[0] = CL_DEVICE_PARTITION_BY_COUNTS;
  properties[1] = (numComputeUnits / 2) + 1;
  properties[2] = 0;
  properties[3] = 0;
  err = clCreateSubDevices(device, properties, num_entries, out_devices,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices - create a device with more than "
                        "half the compute units",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  err = clCreateSubDevices(device, properties, num_entries - 1, out_devices + 1,
                           &num_devices);
  bResult = SilentCheck("clCreateSubDevices - create a device with more than "
                        "half the compute units",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Create a context with the two sub-devices
  context = clCreateContext(NULL, 2, out_devices, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Create a command queue for the first device
  cmd_queue[0] =
      clCreateCommandQueueWithProperties(context, out_devices[0], NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties - first sub-device",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Passed all tests, release resources
  err = clReleaseCommandQueue(cmd_queue[0]);
  bResult &= SilentCheck("clReleaseCommandQueue", CL_SUCCESS, err);
  err = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext", CL_SUCCESS, err);
  err = clReleaseDevice(out_devices[0]);
  bResult &= SilentCheck("clReleaseDevice", CL_SUCCESS, err);
  err = clReleaseDevice(out_devices[1]);
  bResult &= SilentCheck("clReleaseDevice", CL_SUCCESS, err);

  if (!bResult)
    return bResult;

  printf("\n---------------------------------------\n");
  printf("fission subdivision test succeeded!\n");
  printf("---------------------------------------\n");

  return bResult;
}
