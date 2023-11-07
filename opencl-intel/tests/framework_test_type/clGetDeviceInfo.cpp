#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include "common_utils.h"
#include <cstdio>

extern cl_device_type gDeviceType;
using namespace Intel::OpenCL::Utils;

class GetDeviceInfoTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
};

TEST_F(GetDeviceInfoTest, basic) {
  cl_device_type clDevType;
  cl_uint uiVendorId;
  cl_uint uiMaxComputeUnits;
  cl_device_atomic_capabilities atomicCapabilities;
  cl_device_atomic_capabilities atomicFenceCapabilities;
  size_t size_ret = 0;

  // useless call (both returns are NULL) but allowed.
  cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_TYPE, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo") << "CL_DEVICE_TYPE, invalid args";

  // CL_DEVICE_TYPE
  // return size only
  err = clGetDeviceInfo(m_device, CL_DEVICE_TYPE, 0, NULL, &size_ret);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_TYPE, return size only args";
  ASSERT_EQ(sizeof(cl_device_type), size_ret) << "check value";

  // CL_DRIVER_VERSION
  // return driver version
  // it's set as hardcoded number, see more in doc/library_versioning.rst
  // shall be fixed when it's changed
  char buffer[1024];
  err = clGetDeviceInfo(m_device, CL_DRIVER_VERSION, sizeof(buffer), buffer,
                        nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo") << "CL_DRIVER_VERSION, error";

  // CL_DEVICE_TYPE
  err = clGetDeviceInfo(m_device, CL_DEVICE_TYPE, sizeof(cl_device_type),
                        (void *)(&clDevType), NULL);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo") << "CL_DEVICE_TYPE, error";

  // CL_DEVICE_VENDOR_ID
  err = clGetDeviceInfo(m_device, CL_DEVICE_VENDOR_ID, sizeof(cl_uint),
                        &uiVendorId, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo") << "CL_DEVICE_VENDOR_ID, error";

  // max compute units
  err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
                        &uiMaxComputeUnits, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_MAX_COMPUTE_UNITS error";

  // device version string
  size_t szDeviceVersionStringSize = 0;
  std::string deviceVersionString;
  err = clGetDeviceInfo(m_device, CL_DEVICE_VERSION, 0, NULL,
                        &szDeviceVersionStringSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_VERSION - query size";
  if (0 < szDeviceVersionStringSize) {
    deviceVersionString.resize(szDeviceVersionStringSize);
    err =
        clGetDeviceInfo(m_device, CL_DEVICE_VERSION, szDeviceVersionStringSize,
                        &deviceVersionString[0], NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_VERSION - get string";
    printf("CL_DEVICE_VERSION: %s\n", deviceVersionString.c_str());
  }

  // OpenCL 3.0: device numeric version
  size_t szDeviceNumericVersionSize = 0;
  cl_version deviceNumericVersion;
  err = clGetDeviceInfo(m_device, CL_DEVICE_NUMERIC_VERSION, 0, NULL,
                        &szDeviceNumericVersionSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_NUMERIC_VERSION - query size";
  if (0 < szDeviceVersionStringSize) {
    err = clGetDeviceInfo(m_device, CL_DEVICE_NUMERIC_VERSION,
                          szDeviceNumericVersionSize, &deviceNumericVersion,
                          NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_NUMERIC_VERSION - get number";
    printf("CL_DEVICE_NUMERIC_VERSION: %u.%u.%u\n",
           CL_VERSION_MAJOR(deviceNumericVersion),
           CL_VERSION_MINOR(deviceNumericVersion),
           CL_VERSION_PATCH(deviceNumericVersion));
  }

  // IL version string
  size_t szDeviceILVersionStringSize = 0;
  std::string deviceILVersionString;
  err = clGetDeviceInfo(m_device, CL_DEVICE_IL_VERSION, 0, NULL,
                        &szDeviceILVersionStringSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_IL_VERSION - query size";
  if (0 < szDeviceILVersionStringSize) {
    deviceILVersionString.resize(szDeviceILVersionStringSize);
    err = clGetDeviceInfo(m_device, CL_DEVICE_IL_VERSION,
                          szDeviceILVersionStringSize,
                          &deviceILVersionString[0], NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_IL_VERSION - get string";
    printf("CL_DEVICE_IL_VERSION: %s\n", deviceILVersionString.c_str());
  }

  // OpenCL 3.0: ILS with version
  size_t szDeviceILSVersionSize = 0;
  cl_name_version deviceILSVersion;
  err = clGetDeviceInfo(m_device, CL_DEVICE_ILS_WITH_VERSION, 0, NULL,
                        &szDeviceILSVersionSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_ILS_WITH_VERSION - query size";
  if (0 < szDeviceILSVersionSize) {
    err = clGetDeviceInfo(m_device, CL_DEVICE_ILS_WITH_VERSION,
                          szDeviceILSVersionSize, &deviceILSVersion, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_ILS_WITH_VERSION - get cl_name_version";
    printf("CL_DEVICE_ILS_WITH_VERSION: %s v%u.%u.%u\n", deviceILSVersion.name,
           CL_VERSION_MAJOR(deviceILSVersion.version),
           CL_VERSION_MINOR(deviceILSVersion.version),
           CL_VERSION_PATCH(deviceILSVersion.version));
  }

  // OpenCL C version string
  size_t szOpenCLCVersionStringSize = 0;
  std::string OpenCLCVersionString;
  err = clGetDeviceInfo(m_device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL,
                        &szOpenCLCVersionStringSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_OPENCL_C_VERSION - query size";
  if (0 < szOpenCLCVersionStringSize) {
    OpenCLCVersionString.resize(szOpenCLCVersionStringSize);
    err = clGetDeviceInfo(m_device, CL_DEVICE_OPENCL_C_VERSION,
                          szOpenCLCVersionStringSize, &OpenCLCVersionString[0],
                          NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_OPENCL_C_VERSION - get string";
    printf("CL_DEVICE_OPENCL_C_VERSION: %s\n", OpenCLCVersionString.c_str());
  }

  // OpenCL 3.0: OpenCL C all versions
  size_t szOpenCLCAllVersionsSize = 0;
  err = clGetDeviceInfo(m_device, CL_DEVICE_OPENCL_C_ALL_VERSIONS, 0, NULL,
                        &szOpenCLCAllVersionsSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_OPENCL_C_ALL_VERSIONS - query size";
  if (0 < szOpenCLCAllVersionsSize) {
    const size_t numOfCVer = szOpenCLCAllVersionsSize / sizeof(cl_name_version);
    std::vector<cl_name_version> OpenCLCAllVersions(numOfCVer);
    err = clGetDeviceInfo(m_device, CL_DEVICE_OPENCL_C_ALL_VERSIONS,
                          szOpenCLCAllVersionsSize, OpenCLCAllVersions.data(),
                          NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_OPENCL_C_ALL_VERSIONS";
    printf("CL_DEVICE_OPENCL_C_ALL_VERSIONS:\n");
    for (auto CVer : OpenCLCAllVersions) {
      printf("  %s %u.%u\n", CVer.name, CL_VERSION_MAJOR(CVer.version),
             CL_VERSION_MINOR(CVer.version));
    }
  }

  // Built-in kernels
  size_t szBuiltinsStringSize = 0;
  std::string builtinsString;
  err = clGetDeviceInfo(m_device, CL_DEVICE_BUILT_IN_KERNELS, 0, NULL,
                        &szBuiltinsStringSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_BUILT_IN_KERNELS - query size";
  if (0 < szBuiltinsStringSize) {
    builtinsString.resize(szBuiltinsStringSize);
    err = clGetDeviceInfo(m_device, CL_DEVICE_BUILT_IN_KERNELS,
                          szBuiltinsStringSize, &builtinsString[0], NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_BUILT_IN_KERNELS - get string";
    printf("CL_DEVICE_BUILT_IN_KERNELS: %s\n", builtinsString.c_str());
  }

  // OpenCL 3.0: Built-in kernels with version
  size_t szBuiltinsVersionSize = 0;
  err = clGetDeviceInfo(m_device, CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION, 0,
                        NULL, &szBuiltinsVersionSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION - query size";
  ASSERT_EQ(0, szBuiltinsVersionSize)
      << "CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION";

  // OpenCL 3.0: the latest version of the conformance test suite that device
  // passed
  size_t szPassedCTSVersionSize = 0;
  std::string passedCTSVersion;
  err = clGetDeviceInfo(m_device, CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED,
                        0, NULL, &szPassedCTSVersionSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED - query size";
  if (0 < szPassedCTSVersionSize) {
    passedCTSVersion.resize(szPassedCTSVersionSize);
    err = clGetDeviceInfo(m_device, CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED,
                          szPassedCTSVersionSize, &passedCTSVersion[0], NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED - get string";
    printf("CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED: %s\n",
           passedCTSVersion.c_str());
  }

  cl_uint uiNativeVecWidth = 0;
  err = clGetDeviceInfo(m_device, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
                        sizeof(cl_uint), &uiNativeVecWidth, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT - query size";
  const auto CPUID = CPUDetect::GetInstance();
  if (CPUID->IsFeatureSupported(CFS_AVX512F))
    ASSERT_EQ(16, uiNativeVecWidth) << "check value AVX512";
  else if (CPUID->IsFeatureSupported(CFS_AVX20))
    ASSERT_EQ(8, uiNativeVecWidth) << "check value AVX2";
  else if (CPUID->IsFeatureSupported(CFS_AVX10))
    ASSERT_EQ(8, uiNativeVecWidth) << "check value AVX";
  else
    ASSERT_EQ(4, uiNativeVecWidth) << "check value SSE42";

  // CL_DEVICE_PROFILING_TIMER_RESOLUTION
  // Compare with native code results.
  size_t szResolution;
  size_t szNativeCodeResolution;
  err = clGetDeviceInfo(m_device, CL_DEVICE_PROFILING_TIMER_RESOLUTION,
                        sizeof(size_t), &szResolution, NULL);
#if defined(__linux__)
  struct timespec tp;
  clock_getres(CLOCK_MONOTONIC, &tp);
  szNativeCodeResolution = (size_t)tp.tv_nsec;
#elif defined(_WIN32)
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  szNativeCodeResolution = (size_t)(1e9 / freq.QuadPart);
#endif
  if (gDeviceType != CL_DEVICE_TYPE_ACCELERATOR) {
    ASSERT_EQ(szNativeCodeResolution, szResolution)
        << "CL_DEVICE_PROFILING_TIMER_RESOLUTION";
  }

  if (!deviceVersionString.compare(0, 10, "OpenCL 3.0")) {
    cl_bool nonUniformWGSupport = CL_FALSE;
    err = clGetDeviceInfo(m_device, CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT,
                          sizeof(cl_bool), &nonUniformWGSupport, &size_ret);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "clGetDeviceInfo CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT";
    ASSERT_EQ(sizeof(cl_bool), size_ret)
        << "clGetDeviceInfo CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT - query "
           "size";
    ASSERT_EQ(CL_TRUE, nonUniformWGSupport)
        << "clGetDeviceInfo CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT";

    cl_bool wgCollectiveFuncSupport = CL_FALSE;
    err = clGetDeviceInfo(m_device,
                          CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT,
                          sizeof(cl_bool), &wgCollectiveFuncSupport, &size_ret);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "clGetDeviceInfo CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT";
    ASSERT_EQ(sizeof(cl_bool), size_ret)
        << "clGetDeviceInfo CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT "
           "- query size";
    ASSERT_EQ(CL_TRUE, wgCollectiveFuncSupport)
        << "clGetDeviceInfo CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT";

    cl_bool genericAddressSpaceSupport = CL_FALSE;
    err = clGetDeviceInfo(m_device, CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT,
                          sizeof(cl_bool), &genericAddressSpaceSupport,
                          &size_ret);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "clGetDeviceInfo CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT";
    ASSERT_EQ(sizeof(cl_bool), size_ret)
        << "clGetDeviceInfo CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT - query "
           "size";
    ASSERT_EQ(CL_TRUE, genericAddressSpaceSupport)
        << "clGetDeviceInfo CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT";

    cl_device_device_enqueue_capabilities enqueueCapabilities;
    err = clGetDeviceInfo(m_device, CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES,
                          sizeof(cl_device_device_enqueue_capabilities),
                          &enqueueCapabilities, &size_ret);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "clGetDeviceInfo CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES";
    ASSERT_EQ(sizeof(cl_device_device_enqueue_capabilities), size_ret)
        << "clGetDeviceInfo CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES - query size";
    ASSERT_EQ(CL_DEVICE_QUEUE_SUPPORTED | CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT,
              enqueueCapabilities)
        << "clGetDeviceInfo CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES";

    cl_bool pipeSupport = CL_FALSE;
    err = clGetDeviceInfo(m_device, CL_DEVICE_PIPE_SUPPORT, sizeof(cl_bool),
                          &pipeSupport, &size_ret);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "clGetDeviceInfo CL_DEVICE_PIPE_SUPPORT";
    ASSERT_EQ(sizeof(cl_bool), size_ret)
        << "clGetDeviceInfo CL_DEVICE_PIPE_SUPPORT - query size";
    ASSERT_EQ(CL_TRUE, pipeSupport) << "clGetDeviceInfo CL_DEVICE_PIPE_SUPPORT";

    size_t preferredWGSize;
    err =
        clGetDeviceInfo(m_device, CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                        sizeof(size_t), &preferredWGSize, &size_ret);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
        << "CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE";
    ASSERT_EQ(sizeof(size_t), size_ret)
        << "clGetDeviceInfo CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE - "
           "query size";
    ASSERT_EQ(128, preferredWGSize)
        << "clGetDeviceInfo CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE";
  }
  // max work item dimentions
  // all OK
  // cl_uint uiMaxWorkItemDim;
  // err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
  // sizeof(cl_uint), &uiMaxWorkItemDim, NULL); bResult &=
  // Check("CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, all OK", CL_SUCCESS, err);

  // CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES
  // return size only args
  err = clGetDeviceInfo(m_device, CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES, 0,
                        nullptr, &size_ret);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES, return size only args";
  ASSERT_EQ(sizeof(cl_device_atomic_capabilities), size_ret) << "check value";
  // size < actual size
  err = clGetDeviceInfo(m_device, CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES, 1,
                        &atomicCapabilities, &size_ret);
  ASSERT_EQ(CL_INVALID_VALUE, err)
      << "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES, size < actual size";
  // size ok, no size return
  err = clGetDeviceInfo(m_device, CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES,
                        sizeof(cl_device_atomic_capabilities),
                        &atomicCapabilities, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES";
  cl_device_atomic_capabilities expectedCapabilities =
      CL_DEVICE_ATOMIC_ORDER_RELAXED | CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP;
  if (deviceVersionString >= "OpenCL 2.0")
    expectedCapabilities |=
        CL_DEVICE_ATOMIC_ORDER_ACQ_REL | CL_DEVICE_ATOMIC_ORDER_SEQ_CST |
        CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES | CL_DEVICE_ATOMIC_SCOPE_DEVICE;
  ASSERT_EQ(expectedCapabilities, atomicCapabilities)
      << "CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES";

  // CL_DEVICE_ATOMIC_FENCE_CAPABILITIES
  // return size only args
  err = clGetDeviceInfo(m_device, CL_DEVICE_ATOMIC_FENCE_CAPABILITIES, 0,
                        nullptr, &size_ret);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES, return size only args";
  ASSERT_EQ(sizeof(cl_device_atomic_capabilities), size_ret) << "check value";
  // size < actual size
  err = clGetDeviceInfo(m_device, CL_DEVICE_ATOMIC_FENCE_CAPABILITIES, 1,
                        &atomicCapabilities, &size_ret);
  ASSERT_EQ(CL_INVALID_VALUE, err)
      << "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES, size < actual size";
  // size ok, no size return
  err = clGetDeviceInfo(m_device, CL_DEVICE_ATOMIC_FENCE_CAPABILITIES,
                        sizeof(cl_device_atomic_capabilities),
                        &atomicFenceCapabilities, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo")
      << "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES";
  expectedCapabilities = CL_DEVICE_ATOMIC_ORDER_RELAXED |
                         CL_DEVICE_ATOMIC_ORDER_ACQ_REL |
                         CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP;
  if (deviceVersionString >= "OpenCL 2.0")
    expectedCapabilities |=
        CL_DEVICE_ATOMIC_ORDER_SEQ_CST | CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES |
        CL_DEVICE_ATOMIC_SCOPE_DEVICE | CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM;
  ASSERT_EQ(expectedCapabilities, atomicFenceCapabilities)
      << "CL_DEVICE_ATOMIC_FENCE_CAPABILITIES";

  cl_device_fp_config config;
  err = clGetDeviceInfo(m_device, CL_DEVICE_HALF_FP_CONFIG, sizeof(config),
                        &config, &size_ret);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo") << "CL_DEVICE_HALF_FP_CONFIG";
  ASSERT_EQ(sizeof(config), size_ret);
  ASSERT_EQ(CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST, config);
}
