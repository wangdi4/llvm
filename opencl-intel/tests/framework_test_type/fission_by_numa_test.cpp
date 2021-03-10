//|
//| TEST: DeviceFissionTest.fissionByNumaTest
//|
//| Purpose
//| -------
//|
//| Test the ability to plit the device into sub-devices comprised of compute units that share a NUMA node.
//|
//| Method
//| ------
//|
//| 1. Create two subdevices with CL_DEV_PARTITION_AFFINITY_NUMA property from root device.
//| 2. Get which processor the current thread is running on in the kernel.
//| 3. Read results from kernel.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "FrameworkTest.h"
#include "cl_sys_info.h"
#include <CL/cl.h>
#include <CL/cl_ext_intel.h>
#include <gtest/gtest.h>
#include <stdio.h>

extern cl_device_type gDeviceType;

static const char *source = R"(
  extern int sched_getcpu (void);

  __kernel void test_kernel(__global int *p, long n, __global int volatile *counter)
  {
    int i = get_global_id(0);
    int core = sched_getcpu();
    p[i] = core;
    atomic_inc(counter);
    while(*counter < n)
      ;
  }
)";

void fission_by_numa_test() {
  printf("---------------------------------------\n");
  printf("fission by numa test\n");
  printf("---------------------------------------\n");
  cl_device_id device;
  cl_platform_id platform;
  cl_int err;

  cl_uint numNodes = Intel::OpenCL::Utils::GetMaxNumaNode();
  cl_uint maxComputeUnits;
  if (numNodes == 1) {
    printf("This machine has only one numa node, skip fission by name test\n");
    return;
  }

  // init platform
  err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << "clGetPlatformIDs failed.";

  // init device
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << " clGetDeviceIDs failed on trying to obtain "
                             << gDeviceType << " device type.";

  err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(maxComputeUnits), &maxComputeUnits, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << "failed to get device info.";

  cl_uint numDevices;
  const cl_device_partition_property props[3] = {
      CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, CL_DEVICE_AFFINITY_DOMAIN_NUMA,
      0};

  // try to get number of subdevices may be partitioned
  err = clCreateSubDevices(device, props, 8, nullptr, &numDevices);
  ASSERT_EQ(err, CL_SUCCESS) << "Can not get number of devices info.";
  ASSERT_EQ(numDevices, numNodes)
      << "The value of number of devices is incorrect.";

  // create subdevice
  cl_device_id *subDevIds = new cl_device_id[numDevices];
  err = clCreateSubDevices(device, props, 8, subDevIds, &numDevices);
  ASSERT_EQ(err, CL_SUCCESS) << "failed to create subdevice.";

  // cl_context *context = new cl_context[numDevices];
  cl_context *context = new cl_context[numDevices];
  cl_command_queue *command = new cl_command_queue[numDevices];
  cl_program *program = new cl_program[numDevices];
  size_t globalSize = maxComputeUnits / numDevices;
  size_t localSize = 1;
  std::vector<int *> res(numDevices, nullptr);
  for (int i = 0; i < numDevices; i++)
    res[i] = new int[globalSize];
  cl_mem *buf = new cl_mem[numDevices];
  cl_kernel *kernel = new cl_kernel[numDevices];
  for (int i = 0; i < numDevices; i++) {
    cl_uint computeUnits;
    err = clGetDeviceInfo(subDevIds[i], CL_DEVICE_MAX_COMPUTE_UNITS,
                          sizeof(computeUnits), &computeUnits, nullptr);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to get device info.";
    ASSERT_EQ(computeUnits, maxComputeUnits / numDevices)
        << "The number of compute units of subdevice is not correct.";

    // create context
    context[i] =
        clCreateContext(nullptr, 1, &subDevIds[i], nullptr, nullptr, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to create context.";

    // create command queue
    command[i] = clCreateCommandQueue(context[i], subDevIds[i], 0, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to create command queue.";

    // build program
    err = BuildProgramSynch(context[i], 1, (const char **)&source, nullptr,
                            nullptr, &program[i]);

    ASSERT_TRUE(err) << "BuildProgramSynch failed";

    // create buffer
    buf[i] =
        clCreateBuffer(context[i], CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sizeof(int) * globalSize, res[i], &err);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to create buffer.";

    int counter[1] = {0};
    cl_mem counterBuf =
        clCreateBuffer(context[i], CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sizeof(int), counter, &err);

    // create kernel
    kernel[i] = clCreateKernel(program[i], "test_kernel", &err);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to create kernel.";
    clSetKernelArg(kernel[i], 0, sizeof(cl_mem), &buf[i]);
    clSetKernelArg(kernel[i], 1, sizeof(size_t), &globalSize);
    clSetKernelArg(kernel[i], 2, sizeof(cl_ulong), &counterBuf);

    // enqueue kernel
    err = clEnqueueNDRangeKernel(command[i], kernel[i], 1, nullptr, &globalSize,
                                 &localSize, 0, nullptr, nullptr);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to run kernel.";
    err = clEnqueueReadBuffer(command[i], buf[i], CL_FALSE, 0,
                              sizeof(int) * globalSize, res[i], 0, nullptr,
                              nullptr);
    ASSERT_EQ(err, CL_SUCCESS) << "failed to read buffer";
  }

  for (int i = 0; i < numDevices; i++) {
    clFinish(command[i]);
  }

  // validate result
  for (int i = 0; i < numDevices; i++) {
    std::vector<cl_uint> nodes;
    Intel::OpenCL::Utils::GetProcessorIndexFromNumaNode(i, nodes);
    std::set<cl_uint> index(nodes.begin(), nodes.end());
    for (int core = 0; core < globalSize; core++) {
      if (index.find(res[i][core]) == index.end()) {
        ASSERT_TRUE(false) << "thread is not bound to same numa node.";
      }
    }
  }

  clReleaseDevice(device);
  for (int i = 0; i < numDevices; i++) {
    clReleaseDevice(subDevIds[i]);
    clReleaseContext(context[i]);
    clReleaseProgram(program[i]);
    clReleaseCommandQueue(command[i]);
    clReleaseKernel(kernel[i]);
    clReleaseMemObject(buf[i]);
    delete[] res[i];
  }
  delete[] subDevIds;
  delete[] context;
  delete[] program;
  delete[] command;
  delete[] kernel;
  delete[] buf;
}
