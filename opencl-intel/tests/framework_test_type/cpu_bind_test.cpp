#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "cl_sys_info.h"
#include "gtest_wrapper.h"
#include "hwloc.h"

extern cl_device_type gDeviceType;

class CPUBindTest : public ::testing::Test {
public:
  CPUBindTest() : platform(nullptr), device(nullptr), topology(nullptr) {}

protected:
  void SetUp() override {
    int err = hwloc_topology_init(&topology);
    ASSERT_EQ(err, 0) << "failed to init topology\n";

    err = hwloc_topology_load(topology);
    ASSERT_EQ(err, 0) << "failed to load topology\n";

#if _WIN32
    // There are a lot of restrictions on processor affinity on Windows, so we
    // will not run this test on Windows.
    GTEST_SKIP();
#endif
  }

  void TearDown() override {
    if (topology)
      hwloc_topology_destroy(topology);
    if (device) {
      cl_int err = clReleaseDevice(device);
      ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
    }
  }

protected:
  cl_platform_id platform;
  cl_device_id device;
  hwloc_topology_t topology;
};

TEST_F(CPUBindTest, BindCoresToOneNode) {
  int numNodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NUMANODE);
  if (numNodes <= 1)
    GTEST_SKIP();

  hwloc_obj_t node_obj = hwloc_get_numanode_obj_by_os_index(topology, 0);
  ASSERT_NE(node_obj, nullptr) << "failed to get numa node obj\n";

  int err =
      hwloc_set_cpubind(topology, node_obj->cpuset, HWLOC_CPUBIND_PROCESS);
  ASSERT_EQ(err, 0) << "failed to bind cpu cores to node 0\n";

  err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

  const cl_device_partition_property props[3] = {
      CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, CL_DEVICE_AFFINITY_DOMAIN_NUMA,
      0};

  // After binding cpu cores to one numa node, we cannot partition by numa.
  cl_uint numDevices = 0;
  err = clCreateSubDevices(device, props, 8, nullptr, &numDevices);
  ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
  ASSERT_EQ(numDevices, 1) << "The device should not be partitioned\n";
}
