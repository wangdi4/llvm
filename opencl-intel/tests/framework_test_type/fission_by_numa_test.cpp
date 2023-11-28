//|
//| TEST: DeviceFissionTest.fissionByNumaTest
//|
//| Purpose
//| -------
//|
//| Test the ability to plit the device into sub-devices comprised of compute
// units that share a NUMA node.
//|
//| Method
//| ------
//|
//| 1. Create two subdevices with CL_DEV_PARTITION_AFFINITY_NUMA property from
// root device. | 2. Get which processor the current thread is running on in the
// kernel. | 3. Read results from kernel.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl_ext.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_sys_info.h"
#include "gtest_wrapper.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

class SubDevicesByNumaTest : public ::testing::Test {
public:
  SubDevicesByNumaTest() : platform(nullptr), device(nullptr) {
    // clCreateSubDevice is not supported on windows,
    // skip this test temporarily.
    //  TODO: Re-enable this test
    skipTests = 1;
  }

protected:
  void SetUp() override {
    numNodes = Intel::OpenCL::Utils::GetMaxNumaNode();
    if ((numNodes == 1) || skipTests) {
      GTEST_SKIP();
    }

    cl_int err = clGetPlatformIDs(1, &platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,
                          sizeof(maxComputeUnits), &maxComputeUnits, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
  }

  void TearDown() override {
    cl_int err;
    if (device) {
      err = clReleaseDevice(device);
      ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
    }
  }

protected:
  cl_platform_id platform;
  cl_device_id device;
  std::vector<cl_device_id> subDevIds;
  std::vector<cl_context> context;
  std::vector<cl_program> program;
  std::vector<cl_command_queue> queue;
  std::vector<cl_mem> buf;
  std::vector<cl_kernel> kernel;
  cl_uint numDevices;
  cl_uint maxComputeUnits;
  cl_uint numNodes;
  bool skipTests = 0;
};

TEST_F(SubDevicesByNumaTest, createSubDevicesAndRunKernel) {
  const char *source = R"(
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

  const cl_device_partition_property props[3] = {
      CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, CL_DEVICE_AFFINITY_DOMAIN_NUMA,
      0};

  // try to get number of subdevices can be partitioned
  cl_int err = clCreateSubDevices(device, props, 8, nullptr, &numDevices);
  ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");
  ASSERT_EQ(numDevices, numNodes)
      << "The value of number of devices is incorrect.";

  // create subdevice
  subDevIds.resize(numDevices);
  err = clCreateSubDevices(device, props, 8, subDevIds.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");

  context.resize(numDevices);
  queue.resize(numDevices);
  program.resize(numDevices);
  buf.resize(numDevices);
  kernel.resize(numDevices);
  size_t globalSize = maxComputeUnits / numDevices;
  size_t localSize = 1;
  std::vector<int *> res(numDevices, nullptr);
  for (cl_uint i = 0; i < numDevices; i++)
    res[i] = new int[globalSize];
  for (cl_uint i = 0; i < numDevices; i++) {
    cl_uint computeUnits;
    err = clGetDeviceInfo(subDevIds[i], CL_DEVICE_MAX_COMPUTE_UNITS,
                          sizeof(computeUnits), &computeUnits, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
    ASSERT_EQ(maxComputeUnits / numDevices, computeUnits)
        << "The number of compute units of subdevice is not correct.";

    cl_uint numSlices;
    err = clGetDeviceInfo(subDevIds[i], CL_DEVICE_NUM_SLICES_INTEL,
                          sizeof(numSlices), &numSlices, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
    ASSERT_EQ(1, numSlices)
        << "Each subdevice should be bond to one NUMA node.";

    // create context
    context[i] =
        clCreateContext(nullptr, 1, &subDevIds[i], nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    // create command queue
    queue[i] = clCreateCommandQueueWithProperties(context[i], subDevIds[i],
                                                  nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

    // build program
    err = BuildProgramSynch(context[i], 1, (const char **)&source, nullptr,
                            nullptr, &program[i]);
    ASSERT_TRUE(err) << "BuildProgramSynch failed";

    // create buffer
    buf[i] =
        clCreateBuffer(context[i], CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sizeof(int) * globalSize, res[i], &err);
    ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

    int counter[1] = {0};
    cl_mem counterBuf =
        clCreateBuffer(context[i], CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sizeof(int), counter, &err);

    // create kernel
    kernel[i] = clCreateKernel(program[i], "test_kernel", &err);
    ASSERT_OCL_SUCCESS(err, "clCreateKernel");
    clSetKernelArg(kernel[i], 0, sizeof(cl_mem), &buf[i]);
    clSetKernelArg(kernel[i], 1, sizeof(size_t), &globalSize);
    clSetKernelArg(kernel[i], 2, sizeof(cl_ulong), &counterBuf);

    // enqueue kernel
    err = clEnqueueNDRangeKernel(queue[i], kernel[i], 1, nullptr, &globalSize,
                                 &localSize, 0, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
    err = clEnqueueReadBuffer(queue[i], buf[i], CL_FALSE, 0,
                              sizeof(int) * globalSize, res[i], 0, nullptr,
                              nullptr);
    ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");
  }

  for (auto &q : queue)
    clFinish(q);

  // validate result
  for (cl_uint i = 0; i < numDevices; i++) {
    std::vector<cl_uint> nodes;
    Intel::OpenCL::Utils::GetProcessorIndexFromNumaNode(i, nodes);
    std::set<cl_uint> index(nodes.begin(), nodes.end());
    for (size_t core = 0; core < globalSize; core++) {
      if (index.find(res[i][core]) == index.end()) {
        ASSERT_TRUE(false) << "thread is not bound to same numa node.";
      }
    }
  }

  for (auto &k : kernel) {
    err = clReleaseKernel(k);
    ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
  }
  for (auto &mem : buf) {
    err = clReleaseMemObject(mem);
    ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
  }
  for (auto &q : queue) {
    err = clReleaseCommandQueue(q);
    ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
  }
  for (auto &prog : program) {
    err = clReleaseProgram(prog);
    ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
  }
  for (auto &cxt : context) {
    err = clReleaseContext(cxt);
    ASSERT_OCL_SUCCESS(err, "clReleaseContext");
  }
  for (auto &subDev : subDevIds) {
    err = clReleaseDevice(subDev);
    ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
  }
}

TEST_F(SubDevicesByNumaTest, queryDeviceInfo) {
  size_t paramSize;
  // query the size of device partition properties
  std::vector<cl_device_partition_property> supportedProps;
  cl_int err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_PROPERTIES, 0,
                               nullptr, &paramSize);
  ASSERT_OCL_SUCCESS(err, clGetDeviceInfo);

  supportedProps.resize(paramSize / sizeof(cl_device_partition_property));
  // query device info -> CL_DEVICE_PARTITION_PROPERTIES
  err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_PROPERTIES, paramSize,
                        supportedProps.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, clGetDeviceInfo);
  // device should support partition by affinity domain
  bool canPartitionByAffinityDomain =
      std::find(supportedProps.begin(), supportedProps.end(),
                CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN) != supportedProps.end();
  ASSERT_EQ(true, canPartitionByAffinityDomain);

  cl_device_affinity_domain supportedAffinityDomains;
  err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_AFFINITY_DOMAIN,
                        sizeof(cl_device_affinity_domain),
                        &supportedAffinityDomains, nullptr);
  ASSERT_OCL_SUCCESS(err, clGetDeviceInfo);
  ASSERT_EQ(CL_DEVICE_AFFINITY_DOMAIN_NUMA |
                CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE,
            supportedAffinityDomains &
                (CL_DEVICE_AFFINITY_DOMAIN_NUMA |
                 CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE));

  const cl_device_partition_property props[3] = {
      CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, CL_DEVICE_AFFINITY_DOMAIN_NUMA,
      0};

  // try to get number of subdevices can be partitioned
  err = clCreateSubDevices(device, props, 8, nullptr, &numDevices);
  ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");
  // create subdevice
  subDevIds.resize(numDevices);
  err = clCreateSubDevices(device, props, 8, subDevIds.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");

  std::vector<cl_device_partition_property> partitionType;
  // query the size of device partition type
  err = clGetDeviceInfo(subDevIds[0], CL_DEVICE_PARTITION_TYPE, 0, nullptr,
                        &paramSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  partitionType.resize(paramSize / sizeof(cl_device_partition_property));
  // query device info -> CL_DEVICE_PARTITION_TYPE
  err = clGetDeviceInfo(subDevIds[0], CL_DEVICE_PARTITION_TYPE, paramSize,
                        partitionType.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
  // subdevice should be partitioned by affinity domain numa
  bool partitionByNuma =
      std::find(partitionType.begin(), partitionType.end(),
                CL_DEVICE_AFFINITY_DOMAIN_NUMA) != partitionType.end();
  ASSERT_EQ(true, partitionByNuma);

  // subdevice created by affinity domain is not partitionable
  cl_device_partition_property subDevSupportedProp;
  err = clGetDeviceInfo(subDevIds[0], CL_DEVICE_PARTITION_PROPERTIES,
                        sizeof(cl_device_partition_property),
                        &subDevSupportedProp, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
  ASSERT_EQ((cl_device_partition_property)0, subDevSupportedProp);

  cl_device_affinity_domain subDevSupportedDomain;
  err = clGetDeviceInfo(subDevIds[0], CL_DEVICE_PARTITION_AFFINITY_DOMAIN,
                        sizeof(cl_device_affinity_domain),
                        &subDevSupportedDomain, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
  ASSERT_EQ((cl_device_affinity_domain)0, subDevSupportedDomain);

  for (auto &subDev : subDevIds) {
    err = clReleaseDevice(subDev);
    ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
  }
}

TEST_F(SubDevicesByNumaTest, createSubDevicesAndCommandQueues) {
  std::vector<std::vector<cl_device_id>> createdSubDevices;
  std::vector<cl_context> createdContext;
  std::vector<cl_command_queue> createdQueues;

  cl_int err;

  for (int i = 0; i < 2; i++) {
    const cl_device_partition_property props[3] = {
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, CL_DEVICE_AFFINITY_DOMAIN_NUMA,
        0};

    // try to get number of subdevices can be partitioned
    err = clCreateSubDevices(device, props, 8, nullptr, &numDevices);
    ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");
    ASSERT_EQ(numDevices, numNodes)
        << "The value of number of devices is incorrect.";

    // create subdevice
    std::vector<cl_device_id> subDevices(numDevices, nullptr);
    err = clCreateSubDevices(device, props, 8, subDevices.data(), nullptr);
    ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");

    for (auto &dev : subDevices) {
      cl_context context =
          clCreateContext(nullptr, 1, &dev, nullptr, nullptr, &err);
      ASSERT_OCL_SUCCESS(err, "clCreateContext");
      createdContext.push_back(context);

      cl_command_queue queue =
          clCreateCommandQueueWithProperties(context, dev, nullptr, &err);
      ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
      createdQueues.push_back(queue);
    }
  }

  for (auto &q : createdQueues) {
    err = clReleaseCommandQueue(q);
    ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
  }

  for (auto &ctx : createdContext) {
    err = clReleaseContext(ctx);
    ASSERT_OCL_SUCCESS(err, "clReleaseContext");
  }

  for (auto &devices : createdSubDevices)
    for (auto &dev : devices) {
      err = clReleaseDevice(dev);
      ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
    }
}
