// Copyright (c) 2020 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "base_fixture.h"
#include "common_utils.h"
#include "gtest/gtest.h"
#include <CL/cl_ext_intel.h>

class UsmTest : public OCLFPGABaseFixture,
                public ::testing::WithParamInterface<int> {
protected:
  typedef OCLFPGABaseFixture parent_t;
  void SetUp() override {
    ASSERT_TRUE(SETENV("CL_CONFIG_CPU_EMULATE_DEVICES",
                       std::to_string(GetParam()).c_str()))
        << "failed to set env CL_CONFIG_CPU_EMULATE_DEVICES";
    parent_t::SetUp();

    // Get USM function address
    cl_platform_id platform = parent_t::platform();
    m_clHostMemAllocINTEL =
        (clHostMemAllocINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clHostMemAllocINTEL");
    ASSERT_NE(m_clHostMemAllocINTEL, nullptr)
        << "failed to get address of clHostMemAllocINTEL";

    m_clSharedMemAllocINTEL =
        (clSharedMemAllocINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clSharedMemAllocINTEL");
    ASSERT_NE(m_clSharedMemAllocINTEL, nullptr)
        << "failed to get address of clSharedMemAllocINTEL";

    m_clEnqueueMemsetINTEL =
        (clEnqueueMemsetINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueMemsetINTEL");
    ASSERT_NE(m_clEnqueueMemsetINTEL, nullptr)
        << "failed to get address of clEnqueueMemsetINTEL";

    m_clEnqueueMemFillINTEL =
        (clEnqueueMemFillINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueMemFillINTEL");
    ASSERT_NE(m_clEnqueueMemFillINTEL, nullptr)
        << "failed to get address of clEnqueueMemFillINTEL";

    m_clEnqueueMemcpyINTEL =
        (clEnqueueMemcpyINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueMemcpyINTEL");
    ASSERT_NE(m_clEnqueueMemcpyINTEL, nullptr)
        << "failed to get address of clEnqueueMemcpyINTEL";

    m_clEnqueueMigrateMemINTEL =
        (clEnqueueMigrateMemINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueMigrateMemINTEL");
    ASSERT_NE(m_clEnqueueMigrateMemINTEL, nullptr)
        << "failed to get address of clEnqueueMigrateMemINTEL";

    m_clEnqueueMemAdviseINTEL =
        (clEnqueueMemAdviseINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clEnqueueMemAdviseINTEL");
    ASSERT_NE(m_clEnqueueMemAdviseINTEL, nullptr)
        << "failed to get address of clEnqueueMemAdviseINTEL";

    m_clMemFreeINTEL =
        (clMemFreeINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clMemFreeINTEL");
    ASSERT_NE(m_clMemFreeINTEL, nullptr)
        << "failed to get address of clMemFreeINTEL";

    m_clSetKernelArgMemPointerINTEL = (clSetKernelArgMemPointerINTEL_fn)
        clGetExtensionFunctionAddressForPlatform(
            platform, "clSetKernelArgMemPointerINTEL");
    ASSERT_NE(m_clSetKernelArgMemPointerINTEL, nullptr)
        << "failed to get address of clSetKernelArgMemPointerINTEL";
  }

  void TearDown() override {
    parent_t::TearDown();
    ASSERT_TRUE(UNSETENV("CL_CONFIG_CPU_EMULATE_DEVICES"))
        << "failed to unset env CL_CONFIG_CPU_EMULATE_DEVICES";
  }

  void BuildKernel(cl_context context, cl_device_id device,
                   const char *source[], int count, const char *kernelName,
                   cl_program &program, cl_kernel &kernel) {
    cl_int err;
    program = clCreateProgramWithSource(context, count, source, nullptr, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clCreateProgramWithSource failed";
    err = clBuildProgram(program, 1, &device, "", nullptr, nullptr);
    ASSERT_EQ(err, CL_SUCCESS) << "clBuildProgram failed";
    kernel = clCreateKernel(program, kernelName, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clCreateKernel failed";
  }

  /// Test enqueue commands.
  void TestCommands(cl_device_id device, cl_context context,
                    cl_command_queue queue, void *buffer, size_t size,
                    bool shouldSucceed) {
    // memset
    cl_int value = 1;
    cl_int err =
        m_clEnqueueMemsetINTEL(queue, buffer, value, size, 0, nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMemsetINTEL should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueMemsetINTEL should fail";

    // memfill
    const char pattern[4] = {'0', '1', '2', '3'};
    int pattern_size = sizeof(pattern) / sizeof(char);
    err = m_clEnqueueMemFillINTEL(queue, buffer, pattern, pattern_size, size, 0,
                                  nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMemFillINTEL should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueMemFillINTEL should fail";

    // Explicit host USM
    void *buffer2 = m_clHostMemAllocINTEL(context, nullptr, size, 0, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clHostMemAllocINTEL failed";
    ASSERT_NE(buffer2, nullptr) << "invalid buffer2";

    // kernel arg
    const char *source[] = {
        "kernel void test(global char *buffer, global char *buffer2){}"};
    cl_program program;
    cl_kernel kernel;
    ASSERT_NO_FATAL_FAILURE(
        BuildKernel(context, device, source, 1, "test", program, kernel));
    err = m_clSetKernelArgMemPointerINTEL(kernel, 0, buffer);
    ASSERT_EQ(err, CL_SUCCESS) << "clSetKernelArgMemPointerINTEL failed";
    err = m_clSetKernelArgMemPointerINTEL(kernel, 1, buffer2);
    ASSERT_EQ(err, CL_SUCCESS) << "clSetKernelArgMemPointerINTEL failed";

    size_t gdim = 1;
    size_t ldim = 1;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                                 nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueNDRangeKernel should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueNDRangeKernel should fail";

    err = clFinish(queue);
    ASSERT_EQ(err, CL_SUCCESS) << "clFinish failed";

    err = clReleaseKernel(kernel);
    ASSERT_EQ(err, CL_SUCCESS) << "clReleaseKernel failed";
    err = clReleaseProgram(program);
    ASSERT_EQ(err, CL_SUCCESS) << "clReleaseProgram failed";
  }

  /// Test memcpy
  void TestMemcpy(cl_device_id device, cl_context context,
                  cl_command_queue queue, void *buffer, size_t size,
                  bool shouldSucceed) {
    cl_int err;
    cl_uint alignment = 0;
    void *buffer2 = m_clSharedMemAllocINTEL(context, device, nullptr, size,
                                            alignment, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clSharedMemAllocINTEL failed";
    ASSERT_NE(buffer2, nullptr) << "invalid buffer2";

    // buffer -> buffer2
    err = m_clEnqueueMemcpyINTEL(queue, CL_TRUE, buffer2, buffer, size, 0,
                                 nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should fail";

    // buffer2 -> buffer
    err = m_clEnqueueMemcpyINTEL(queue, CL_TRUE, buffer, buffer2, size, 0,
                                 nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should fail";

    // Explicit host USM
    void *buffer3 = m_clHostMemAllocINTEL(context, nullptr, size, 0, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clHostMemAllocINTEL failed";
    ASSERT_NE(buffer3, nullptr) << "invalid buffer2";

    // buffer -> buffer3
    err = m_clEnqueueMemcpyINTEL(queue, CL_TRUE, buffer3, buffer, size, 0,
                                 nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should fail";

    // buffer3 -> buffer
    err = m_clEnqueueMemcpyINTEL(queue, CL_TRUE, buffer, buffer3, size, 0,
                                 nullptr, nullptr);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueMemcpyINTEL should fail";

    m_clMemFreeINTEL(context, buffer2);
  }

  /// Test non-argument USM pointer.
  void TestNonArg(cl_device_id device, cl_context context,
                  cl_command_queue queue, void *buffer, size_t size,
                  bool shouldSucceed) {
    cl_int err;
    cl_uint alignment = 0;
    struct Foo {
      cl_int *a;
    };
    Foo *foo = (Foo *)m_clSharedMemAllocINTEL(context, device, nullptr,
                                              sizeof(Foo), alignment, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clSharedMemAllocINTEL failed";
    foo->a = (cl_int *)buffer;
    const char *source[] = {"typedef struct {\n"
                            "  int *a;\n"
                            "} Foo;\n"
                            "kernel void test(global Foo *foo) {}\n"};
    cl_program program;
    cl_kernel kernel;
    ASSERT_NO_FATAL_FAILURE(
        BuildKernel(context, device, source, 1, "test", program, kernel));
    err = m_clSetKernelArgMemPointerINTEL(kernel, 0, foo);
    ASSERT_EQ(err, CL_SUCCESS) << "clSetKernelArgMemPointerINTEL failed";

    err = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                              sizeof(Foo), foo);
    if (shouldSucceed)
      ASSERT_EQ(err, CL_SUCCESS) << "clSetKernelExecInfo should succeed";
    else
      ASSERT_NE(err, CL_SUCCESS) << "clEnqueueNDRangeKernel should fail";

    if (CL_SUCCESS == err) {
      size_t gdim = 1;
      size_t ldim = 1;

      // clEnqueueNDRangeKernel should fail because access to buffer requires
      // CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL being set.
      err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                                   nullptr, nullptr);
      ASSERT_EQ(err, CL_INVALID_OPERATION)
          << "clEnqueueNDRangeKernel should return CL_INVALID_OPERATION";

      cl_bool useIndirectShared = CL_TRUE;
      err = clSetKernelExecInfo(
          kernel, CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL,
          sizeof(useIndirectShared), &useIndirectShared);
      ASSERT_EQ(err, CL_SUCCESS) << "clSetKernelExecInfo failed";

      err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                                   nullptr, nullptr);
      if (shouldSucceed)
        ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueNDRangeKernel should succeed";
      else
        ASSERT_NE(err, CL_SUCCESS) << "clEnqueueNDRangeKernel should fail";

      err = clFinish(queue);
      ASSERT_EQ(err, CL_SUCCESS) << "clFinish failed";
    }

    err = clReleaseKernel(kernel);
    ASSERT_EQ(err, CL_SUCCESS) << "clReleaseKernel failed";
    err = clReleaseProgram(program);
    ASSERT_EQ(err, CL_SUCCESS) << "clReleaseProgram failed";

    err = m_clMemFreeINTEL(context, foo);
    ASSERT_EQ(err, CL_SUCCESS) << "clMemFreeINTEL failed";
  }

protected:
  clHostMemAllocINTEL_fn m_clHostMemAllocINTEL;
  clSharedMemAllocINTEL_fn m_clSharedMemAllocINTEL;
  clEnqueueMemsetINTEL_fn m_clEnqueueMemsetINTEL;
  clEnqueueMemFillINTEL_fn m_clEnqueueMemFillINTEL;
  clEnqueueMemcpyINTEL_fn m_clEnqueueMemcpyINTEL;
  clEnqueueMigrateMemINTEL_fn m_clEnqueueMigrateMemINTEL;
  clEnqueueMemAdviseINTEL_fn m_clEnqueueMemAdviseINTEL;
  clMemFreeINTEL_fn m_clMemFreeINTEL;
  clSetKernelArgMemPointerINTEL_fn m_clSetKernelArgMemPointerINTEL;
};

TEST_P(UsmTest, checkCapacities) {
  cl_device_unified_shared_memory_capabilities_intel capsAll =
      CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_ATOMIC_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ATOMIC_ACCESS_INTEL;
  std::pair<cl_bitfield, cl_device_unified_shared_memory_capabilities_intel>
      capParams[] = {
          {CL_DEVICE_HOST_MEM_CAPABILITIES_INTEL, capsAll},
          {CL_DEVICE_DEVICE_MEM_CAPABILITIES_INTEL, capsAll},
          {CL_DEVICE_SINGLE_DEVICE_SHARED_MEM_CAPABILITIES_INTEL, capsAll},
          {CL_DEVICE_CROSS_DEVICE_SHARED_MEM_CAPABILITIES_INTEL, 0},
          {CL_DEVICE_SHARED_SYSTEM_MEM_CAPABILITIES_INTEL, 0}};

  for (cl_device_id device : devices()) {
    for (auto &pair : capParams) {
      cl_bitfield param_name = pair.first;
      cl_device_unified_shared_memory_capabilities_intel caps;
      cl_int err = clGetDeviceInfo(
          device, param_name,
          sizeof(cl_device_unified_shared_memory_capabilities_intel), &caps,
          nullptr);
      ASSERT_EQ(err, CL_SUCCESS) << "clGetDeviceInfo failed";
      ASSERT_EQ(caps, pair.second) << "capability mismatch";
    }
  }
}

/// FPGA supports explicit USM, i.e. access to USM buffer allocated on host or
/// the same device is allowed.
#ifdef _WIN32
/// FIXME disabled on windows due to flaky fail [CMPLRLLVM-39465]
TEST_P(UsmTest, DISABLED_sameDevice) {
#else
TEST_P(UsmTest, sameDevice) {
#endif
  for (cl_device_id device : devices()) {
    cl_context context = createContext(device);
    ASSERT_NE(nullptr, context) << "createContext failed";

    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(queue, nullptr) << "createCommandQueue failed";

    cl_int err;
    size_t size = 256;
    cl_uint alignment = 0;
    void *buffer = m_clSharedMemAllocINTEL(context, device, nullptr, size,
                                           alignment, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clSharedMemAllocINTEL failed";
    ASSERT_NE(buffer, nullptr) << "invalid buffer";

    ASSERT_NO_FATAL_FAILURE(TestCommands(device, context, queue, buffer, size,
                                         /*shouldSucceed*/ true));
    ASSERT_NO_FATAL_FAILURE(TestMemcpy(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ true));
    ASSERT_NO_FATAL_FAILURE(TestNonArg(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ true));

    // Test migrate at last, since CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED
    // will make content of buffer undefined.
    cl_mem_migration_flags flags = CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED;
    err = m_clEnqueueMigrateMemINTEL(queue, buffer, size, flags, 0, nullptr,
                                     nullptr);
    ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueMigrateMemINTEL failed";

    err = m_clMemFreeINTEL(context, buffer);
    ASSERT_EQ(err, CL_SUCCESS) << "clMemFreeINTEL failed";
  }
}

/// FPGA supports restricted USM, so cross device access is not allowed.
TEST_P(UsmTest, crossDevice) {
  GTEST_SKIP();
  const std::vector<cl_device_id> &devices = parent_t::devices();
  size_t numDevices = devices.size();
  // Early return if there is only a single device.
  if (numDevices <= 1)
    return;

  // Create shared context
  cl_context context = createContext(devices);
  ASSERT_NE(nullptr, context) << "createContext failed";

  // Create USM buffer on the first device
  cl_int err;
  size_t size = 256;
  cl_uint alignment = 0;
  void *buffer = m_clSharedMemAllocINTEL(context, devices[0], nullptr, size,
                                         alignment, &err);
  ASSERT_EQ(err, CL_SUCCESS) << "clSharedMemAllocINTEL failed";
  ASSERT_NE(buffer, nullptr) << "invalid buffer";

  for (size_t i = 1; i < numDevices; ++i) {
    cl_device_id device = devices[i];

    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(queue, nullptr) << "createCommandQueue failed";

    ASSERT_NO_FATAL_FAILURE(TestCommands(device, context, queue, buffer, size,
                                         /*shouldSucceed*/ false));
    ASSERT_NO_FATAL_FAILURE(TestMemcpy(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ false));
    ASSERT_NO_FATAL_FAILURE(TestNonArg(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ false));

    // migrate
    cl_mem_migration_flags flags = CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED;
    err = m_clEnqueueMigrateMemINTEL(queue, buffer, size, flags, 0, nullptr,
                                     nullptr);
    ASSERT_EQ(err, CL_INVALID_VALUE)
        << "clEnqueueMigrateMemINTEL should return CL_INVALID_VALUE";
  }

  err = m_clMemFreeINTEL(context, buffer);
  ASSERT_EQ(err, CL_SUCCESS) << "clMemFreeINTEL failed";
}

/// FPGA doesn't support system USM, so access to system buffer is not
/// allowed.
TEST_P(UsmTest, system) {
  // Allocate a system buffer
  size_t size = 256;
  char *buffer = new char[size];
  for (cl_device_id device : devices()) {
    cl_context context = createContext(device);
    ASSERT_NE(nullptr, context) << "createContext failed";
    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(nullptr, queue) << "createCommandQueue failed";

    ASSERT_NO_FATAL_FAILURE(TestCommands(device, context, queue, buffer, size,
                                         /*shouldSucceed*/ false));
    ASSERT_NO_FATAL_FAILURE(TestMemcpy(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ true));
    ASSERT_NO_FATAL_FAILURE(TestNonArg(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ false));

    // migrate
    cl_mem_migration_flags flags = 0;
    cl_int err = m_clEnqueueMigrateMemINTEL(queue, buffer, size, flags, 0,
                                            nullptr, nullptr);
    ASSERT_EQ(err, CL_INVALID_VALUE)
        << "clEnqueueMigrateMemINTEL should return CL_INVALID_VALUE";
  }
  delete[] buffer;
}

/// FPGA supports host USM allocation.
TEST_P(UsmTest, host) {
  for (cl_device_id device : devices()) {
    cl_context context = createContext(device);
    ASSERT_NE(nullptr, context) << "createContext failed";

    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(queue, nullptr) << "createCommandQueue failed";

    cl_int err;
    size_t size = 256;
    cl_uint alignment = 0;
    void *buffer =
        m_clHostMemAllocINTEL(context, nullptr, size, alignment, &err);
    ASSERT_EQ(err, CL_SUCCESS) << "clHostMemAllocINTEL failed";
    ASSERT_NE(buffer, nullptr) << "invalid buffer";

    ASSERT_NO_FATAL_FAILURE(TestCommands(device, context, queue, buffer, size,
                                         /*shouldSucceed*/ true));
    ASSERT_NO_FATAL_FAILURE(TestMemcpy(device, context, queue, buffer, size,
                                       /*shouldSucceed*/ true));

    err = m_clMemFreeINTEL(context, buffer);
    ASSERT_EQ(err, CL_SUCCESS) << "clMemFreeINTEL failed";
  }
}

/// FPGA supports host USM allocation.
/// Test multiple alloc and free [CMPLRLLVM-22817].
TEST_P(UsmTest, hostAllocFree) {
  for (cl_device_id device : devices()) {
    cl_context context = createContext(device);
    ASSERT_NE(nullptr, context) << "createContext failed";

    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(queue, nullptr) << "createCommandQueue failed";

    cl_int err;
    size_t size = 256;
    cl_uint alignment = 0;
    size_t gdim = 1;
    size_t ldim = 1;

    const char *source[] = {"kernel void test(global char *buffer){}"};
    cl_program program;
    cl_kernel kernel;
    ASSERT_NO_FATAL_FAILURE(
        BuildKernel(context, device, source, 1, "test", program, kernel));

    for (int i = 0; i < 10; i++) {
      void *buffer = m_clSharedMemAllocINTEL(context, device, nullptr, size,
                                             alignment, &err);
      ASSERT_EQ(err, CL_SUCCESS) << "clSharedMemAllocINTEL failed";
      ASSERT_NE(buffer, nullptr) << "invalid buffer";

      // kernel arg
      err = m_clSetKernelArgMemPointerINTEL(kernel, 0, buffer);
      ASSERT_EQ(err, CL_SUCCESS) << "clSetKernelArgMemPointerINTEL failed";
      err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                                   nullptr, nullptr);
      ASSERT_EQ(err, CL_SUCCESS) << "clEnqueueNDRangeKernel should succeed";

      err = clFinish(queue);
      ASSERT_EQ(err, CL_SUCCESS) << "clFinish failed";

      err = m_clMemFreeINTEL(context, buffer);
      ASSERT_EQ(err, CL_SUCCESS) << "clMemFreeINTEL failed";
    }
  }
}

INSTANTIATE_TEST_SUITE_P(UsmTests, UsmTest, ::testing::Values(1, 2, 3, 10));
