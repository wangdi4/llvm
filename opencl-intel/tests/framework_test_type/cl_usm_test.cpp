// Copyright (c) 2019-2020 Intel Corporation
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

#include "FrameworkTestThreads.h"
#include "TestsHelpClasses.h"
#include <CL/cl.h>
#include <tbb/parallel_for.h>

#include <thread>

extern cl_device_type gDeviceType;

class USMTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES,
                                                CL_QUEUE_PROFILING_ENABLE, 0};
    m_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                 properties, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  virtual void TearDown() override {
    cl_int err;
    if (m_kernel) {
      err = clReleaseKernel(m_kernel);
      EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
    }
    if (m_program) {
      err = clReleaseProgram(m_program);
      EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
    }
    err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  cl_program m_program = nullptr;
  cl_kernel m_kernel = nullptr;
};

static void BuildProgram(cl_context context, cl_device_id device,
                         const char *source[], int count, cl_program &program) {
  cl_int err;
  program = clCreateProgramWithSource(context, count, source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");
}

TEST_F(USMTest, checkExtensions) {
  void *ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                       "clHostMemAllocINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clDeviceMemAllocINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clSharedMemAllocINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform, "clMemFreeINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clMemBlockingFreeINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clGetMemAllocInfoINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(
      m_platform, "clSetKernelArgMemPointerINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clEnqueueMemsetINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clEnqueueMemFillINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clEnqueueMemcpyINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clEnqueueMigrateMemINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clEnqueueMemAdviseINTEL");
  ASSERT_TRUE(nullptr != ptr);
}

TEST_F(USMTest, checkCapacities) {
  cl_bitfield cap_params[] = {
      CL_DEVICE_HOST_MEM_CAPABILITIES_INTEL,
      CL_DEVICE_DEVICE_MEM_CAPABILITIES_INTEL,
      CL_DEVICE_SINGLE_DEVICE_SHARED_MEM_CAPABILITIES_INTEL,
      CL_DEVICE_CROSS_DEVICE_SHARED_MEM_CAPABILITIES_INTEL,
      CL_DEVICE_SHARED_SYSTEM_MEM_CAPABILITIES_INTEL};
  cl_device_unified_shared_memory_capabilities_intel caps_expected =
      CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_ATOMIC_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ATOMIC_ACCESS_INTEL;
  for (cl_bitfield param_name : cap_params) {
    cl_device_unified_shared_memory_capabilities_intel caps;
    cl_int err = clGetDeviceInfo(
        m_device, param_name,
        sizeof(cl_device_unified_shared_memory_capabilities_intel), &caps,
        nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
    ASSERT_EQ(caps, caps_expected);
  }
}

TEST_F(USMTest, memAlloc) {
  cl_int err;
  size_t size = 256;
  cl_uint alignment = 0;

  // Test correct flags.
  const cl_mem_alloc_flags_intel correctFlags[] = {
      CL_MEM_ALLOC_WRITE_COMBINED_INTEL};
  for (cl_mem_flags flag : correctFlags) {
    cl_mem_properties_intel properties[] = {CL_MEM_ALLOC_FLAGS_INTEL, flag, 0};

    void *buf0 =
        clHostMemAllocINTEL(m_context, properties, size, alignment, &err);
    ASSERT_OCL_SUCCESS(err, "clHostMemAllocINTEL");
    err = clMemFreeINTEL(m_context, buf0);
    ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");

    void *buf1 = clDeviceMemAllocINTEL(m_context, m_device, properties, size,
                                       alignment, &err);
    ASSERT_OCL_SUCCESS(err, "clDeviceMemAllocINTEL");
    err = clMemFreeINTEL(m_context, buf1);
    ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");

    void *buf2 = clSharedMemAllocINTEL(m_context, m_device, properties, size,
                                       alignment, nullptr);
    ASSERT_TRUE(nullptr != buf2);
    err = clMemFreeINTEL(m_context, buf2);
    ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");

    void *buf3 = clSharedMemAllocINTEL(m_context, nullptr, properties, size,
                                       alignment, nullptr);
    ASSERT_TRUE(nullptr != buf3);
    err = clMemFreeINTEL(m_context, buf3);
    ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  }

  // Test wrong flags.
  const cl_mem_alloc_flags_intel wrongFlags[] = {0};
  for (cl_mem_flags flag : wrongFlags) {
    cl_mem_properties_intel properties[] = {CL_MEM_ALLOC_FLAGS_INTEL, flag, 0};

    void *buf = clDeviceMemAllocINTEL(m_context, m_device, properties, size,
                                      alignment, &err);
    ASSERT_EQ(CL_INVALID_PROPERTY, err);
    ASSERT_TRUE(nullptr == buf);
  }

  // Test wrong alignments. The only requirement for alignment is power of 2
  // value.
  int alignments[] = {7, sizeof(cl_double16) * 8 + 1};
  for (int alignment : alignments) {
    void *buf = clDeviceMemAllocINTEL(m_context, m_device, nullptr, size,
                                      alignment, &err);
    ASSERT_TRUE(nullptr == buf);
    ASSERT_EQ(CL_INVALID_VALUE, err);
  }

  // Test one valid alignment value.
  void *buf = clDeviceMemAllocINTEL(m_context, m_device, nullptr, size,
                                    sizeof(cl_double16) * 16, &err);
  ASSERT_TRUE(nullptr != buf);

  err = clMemFreeINTEL(m_context, buf);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
}

static const char *getBlockingFreeTestKernel() {
  return R"(
      __kernel void blocking_test(__global int *buf,
                                  volatile __global int *result) {
        atomic_add(result, buf[get_global_id(0)]);
      }
  )";
}

/// Test clMemBlockingFreeINTEL when enqueue failed.
TEST_F(USMTest, memBlockingFreeAfterEnqueueFail) {
  // Build program and kernel.
  const char *source = getBlockingFreeTestKernel();
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, &source, 1, m_program));
  cl_int err;
  m_kernel = clCreateKernel(m_program, "blocking_test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel blocking_test");

  cl_int *buffer =
      (cl_int *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                      /*size*/ 1024, /*alignment*/ 0, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  err = clSetKernelArgMemPointerINTEL(m_kernel, 0, buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  cl_int result;
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 0, nullptr, nullptr, nullptr,
                               0, nullptr, nullptr);
  ASSERT_OCL_EQ(CL_INVALID_WORK_DIMENSION, err, "clEnqueueNDRangeKernel");

  err = clMemBlockingFreeINTEL(m_context, buffer);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
}

TEST_F(USMTest, memBlockingFree) {
  // Build program and kernel.
  const char *source = getBlockingFreeTestKernel();
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, &source, 1, m_program));
  cl_int err;
  m_kernel = clCreateKernel(m_program, "blocking_test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel blocking_test");

  // Allocate USM buffers.
  // Workload should be large enough so that kernel is not finished when
  // clMemBlockingFreeINTEL is called.
  size_t num = 16384;
  size_t size = num * sizeof(int);
  cl_int *buffer = (cl_int *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                                   size, /*alignment*/ 0, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  std::fill(buffer, buffer + num, 1);
  cl_int result = 0;

  /// Test clMemBlockingFreeINTEL when enqueue succeed.
  err = clSetKernelArgMemPointerINTEL(m_kernel, 0, buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  size_t gdim = num;
  size_t ldim = 64;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  // Test clMemBlockingFreeINTEL when USM buffer passed as argument of kernel.
  err = clMemBlockingFreeINTEL(m_context, buffer);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");

  // Assert result is correct if kernel did complete before buffer is freed.
  EXPECT_EQ(num, result);
}

/// Test clMemBlockingFreeINTEL with event released in advance.
TEST_F(USMTest, memBlockingFreeAfterReleaseEvent) {
  // Build program and kernel.
  const char *source = getBlockingFreeTestKernel();
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, &source, 1, m_program));
  cl_int err;
  m_kernel = clCreateKernel(m_program, "blocking_test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel blocking_test");

  // Workload should be large enough so that kernel is not finished when
  // clMemBlockingFreeINTEL is called.
  size_t num = 16384;
  size_t size = num * sizeof(int);
  cl_int *buffer = (cl_int *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                                   size, /*alignment*/ 0, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  std::fill(buffer, buffer + num, 1);
  cl_int result = 0;

  err = clSetKernelArgMemPointerINTEL(m_kernel, 0, buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  size_t gdim = num;
  size_t ldim = 64;
  cl_event event;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  // Release event before clMemBlockingFreeINTEL.
  err = clReleaseEvent(event);
  EXPECT_OCL_SUCCESS(err, "clReleaseEvent");

  err = clMemBlockingFreeINTEL(m_context, buffer);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");

  EXPECT_EQ(num, result);
}

TEST_F(USMTest, getMemAllocInfo) {
  cl_int err;
  // Allocate USM buffers.
  cl_mem_properties_intel properties[] = {CL_MEM_ALLOC_FLAGS_INTEL,
                                          CL_MEM_ALLOC_WRITE_COMBINED_INTEL, 0};
  size_t size = 1024 * sizeof(char);
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, m_device, properties,
                                               size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  char *ptr = buffer + (size / 2);
  void *basePtr;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_BASE_PTR_INTEL,
                               sizeof(basePtr), &basePtr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(buffer, basePtr);

  size_t bufferSize;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_SIZE_INTEL,
                               sizeof(bufferSize), &bufferSize, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(size, bufferSize);

  cl_device_id device;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_DEVICE_INTEL,
                               sizeof(device), &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(m_device, device);

  cl_mem_alloc_flags_intel flags;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_FLAGS_INTEL,
                               sizeof(flags), &flags, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_ALLOC_WRITE_COMBINED_INTEL, flags);

  err = clMemFreeINTEL(m_context, buffer);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
}

TEST_F(USMTest, enqueueMemset) {
  cl_int err;
  // Allocate USM buffers.
  size_t num = 1024;
  size_t size = num * sizeof(char);
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                               size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  cl_int value1 = 1;
  err =
      clEnqueueMemsetINTEL(m_queue, buffer, value1, size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");
  cl_int value2 = 2;
  err = clEnqueueMemsetINTEL(m_queue, buffer, value2, size / 2, 0, nullptr,
                             nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  int countErrors = 0;
  for (size_t i = 0; i < num / 2; i++)
    if (buffer[i] != (char)value2)
      countErrors++;
  for (size_t i = num / 2; i < num; i++)
    if (buffer[i] != (char)value1)
      countErrors++;
  ASSERT_EQ(countErrors, 0);

  err = clMemBlockingFreeINTEL(m_context, buffer);
  ASSERT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");

  // Memset host ptr.
  char *data = new char[size];
  err = clEnqueueMemsetINTEL(m_queue, data, value1, size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  countErrors = 0;
  for (size_t i = 0; i < num; i++)
    if (data[i] != (char)value1)
      countErrors++;
  ASSERT_EQ(countErrors, 0);
  delete[] data;
}

TEST_F(USMTest, enqueueMemFill) {
  cl_int err;

  const char pattern1[4] = {'0', '1', '2', '3'};
  int pattern1_size = sizeof(pattern1) / sizeof(char);
  const char pattern2[2] = {'A', 'B'};
  int pattern2_size = sizeof(pattern2) / sizeof(char);

  // Allocate USM buffers.
  size_t size1 = pattern1_size * 256;
  size_t size2 = pattern2_size * 256;
  size_t size = size1 + size2;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                               size, 0, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  err = clEnqueueMemFillINTEL(m_queue, buffer, pattern1, pattern1_size, size1,
                              0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemFillINTEL");
  err = clEnqueueMemFillINTEL(m_queue, buffer + size1, pattern2, pattern2_size,
                              size2, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemFillINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  int countErrors = 0;
  for (size_t i = 0; i < size1; i += pattern1_size)
    if (0 != strncmp(&buffer[i], pattern1, pattern1_size))
      countErrors++;
  for (size_t i = size1; i < size; i += pattern2_size)
    if (0 != strncmp(&buffer[i], pattern2, pattern2_size))
      countErrors++;
  ASSERT_EQ(countErrors, 0);

  err = clMemBlockingFreeINTEL(m_context, buffer);
  ASSERT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");

  // Fill host ptr.
  char *data = new char[size1];
  cl_event event;
  err = clEnqueueMemFillINTEL(m_queue, data, pattern1, pattern1_size, size1, 0,
                              nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemFillINTEL");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  // Verify result.
  countErrors = 0;
  for (size_t i = 0; i < size1; i += pattern1_size)
    if (0 != strncmp(&data[i], pattern1, pattern1_size))
      countErrors++;
  ASSERT_EQ(countErrors, 0);

  // Check elapsed time.
  cl_ulong start, end;
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                sizeof(cl_ulong), &start, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo CL_PROFILING_COMMAND_START");
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                sizeof(cl_ulong), &end, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo CL_PROFILING_COMMAND_END");
  EXPECT_NE(start, end) << "Command elapsed time should not be zero";

  delete[] data;
}

TEST_F(USMTest, enqueueMemcpy) {
  cl_int err;

  // Allocate USM buffers.
  size_t num = 1024;
  size_t size = num * sizeof(int);
  cl_uint alignment = sizeof(cl_long);
  cl_mem_properties_intel *properties = nullptr;
  int *buffer =
      (int *)clHostMemAllocINTEL(m_context, properties, size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clHostMemAllocINTEL");
  alignment = 0;
  int *buffer2 = (int *)clDeviceMemAllocINTEL(m_context, m_device, properties,
                                              size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clDeviceMemAllocINTEL");
  int *buffer3 = (int *)clSharedMemAllocINTEL(m_context, m_device, properties,
                                              size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  // Initialize buffer.
  for (size_t i = 0; i < num; i++)
    buffer[i] = i;

  // Copy from host USM to device USM.
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, buffer2, buffer, size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Copy from device USM to shared USM.
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, buffer3, buffer2, size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  for (size_t i = 0; i < num; i++)
    ASSERT_EQ(buffer3[i], i);

  // Copy from device USM to arbitrary host memory.
  int *bufferHost = new int[num];
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, bufferHost, buffer2, size, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  for (size_t i = 0; i < num; i++) {
    ASSERT_EQ(bufferHost[i], i);
    bufferHost[i] = i + 1;
  }

  // Copy from arbitrary host memory to arbitrary host memory.
  int *bufferHost2 = new int[num];
  cl_event event;
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, bufferHost2, bufferHost, size, 0,
                             nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  for (size_t i = 0; i < num; i++)
    ASSERT_EQ(bufferHost2[i], i + 1);

  // Check elapsed time.
  cl_ulong start, end;
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                sizeof(cl_ulong), &start, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo CL_PROFILING_COMMAND_START");
  err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                sizeof(cl_ulong), &end, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventProfilingInfo CL_PROFILING_COMMAND_END");
  EXPECT_NE(start, end) << "Command elapsed time should not be zero";

  // Test overlap.
  size_t offset = size / 4;
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, (char *)buffer + offset, buffer,
                             size / 2, 0, nullptr, nullptr);
  EXPECT_OCL_EQ(CL_MEM_COPY_OVERLAP, err, "clEnqueueMemcpyINTEL");

  delete[] bufferHost;
  delete[] bufferHost2;
  err = clMemBlockingFreeINTEL(m_context, buffer);
  ASSERT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
  err = clMemBlockingFreeINTEL(m_context, buffer2);
  ASSERT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
  err = clMemBlockingFreeINTEL(m_context, buffer3);
  ASSERT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
}

TEST_F(USMTest, enqueueMigrateMem) {
  cl_int err;
  // Allocate USM buffers.
  size_t size = 1024;
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, nullptr, nullptr,
                                               size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  cl_unified_shared_memory_type_intel memType;
  err = clGetMemAllocInfoINTEL(m_context, buffer, CL_MEM_ALLOC_TYPE_INTEL,
                               sizeof(memType), &memType, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_TYPE_SHARED_INTEL, memType);

  cl_mem_migration_flags flags = 0;
  err = clEnqueueMigrateMemINTEL(m_queue, buffer, size, flags, 0, nullptr,
                                 nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, err) << "clEnqueueMigrateMemINTEL failed";

  flags = CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED;
  err = clEnqueueMigrateMemINTEL(m_queue, buffer, size, flags, 0, nullptr,
                                 nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMigrateMemINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  err = clGetMemAllocInfoINTEL(m_context, buffer, CL_MEM_ALLOC_TYPE_INTEL,
                               sizeof(memType), &memType, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_TYPE_DEVICE_INTEL, memType);

  cl_device_id device;
  err = clGetMemAllocInfoINTEL(m_context, buffer, CL_MEM_ALLOC_DEVICE_INTEL,
                               sizeof(device), &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(m_device, device);

  err = clMemBlockingFreeINTEL(m_context, buffer);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
}

TEST_F(USMTest, enqueueAdviseMem) {
  cl_int err;
  // Allocate USM buffers.
  size_t size = 1024;
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                               size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  cl_mem_advice_intel advice = 0;
  err = clEnqueueMemAdviseINTEL(m_queue, buffer, size, advice, 0, nullptr,
                                nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, err) << "clEnqueueMemAdviseINTEL failed";

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  err = clMemBlockingFreeINTEL(m_context, buffer);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
}

static void testSetKernelArgMemPointer(cl_context context, cl_device_id device,
                                       cl_command_queue queue,
                                       cl_program program) {
  cl_int err;
  // Allocate USM buffers.
  cl_uint alignment = 0;
  int *buffer = (int *)clSharedMemAllocINTEL(context, device, nullptr,
                                             sizeof(int), alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  int *result = (int *)clSharedMemAllocINTEL(context, device, nullptr,
                                             sizeof(int), alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  int value = 16;
  buffer[0] = value;

  // Create kernel.
  cl_kernel kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArgMemPointerINTEL(kernel, 0, buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  err = clSetKernelArgMemPointerINTEL(kernel, 1, result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  err = clSetKernelArgMemPointerINTEL(kernel, 2, &value);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  err = clSetKernelArgMemPointerINTEL(kernel, 3, buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  size_t gdim = 1;
  size_t ldim = 1;
  cl_event e;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, &e);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clWaitForEvents(1, &e);
  ASSERT_OCL_SUCCESS(err, "clWaitForEvents");
  err = clReleaseEvent(e);
  ASSERT_OCL_SUCCESS(err, "clReleaseEvent");

  ASSERT_EQ(*result, 1);

  // Check the handling of null usm pointer won't be affected
  // by the first enqueue
  err = clSetKernelArgMemPointerINTEL(kernel, 3, nullptr);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, &e);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clWaitForEvents(1, &e);
  ASSERT_OCL_SUCCESS(err, "clWaitForEvents");
  err = clReleaseEvent(e);
  ASSERT_OCL_SUCCESS(err, "clReleaseEvent");

  err = clMemFreeINTEL(context, buffer);
  EXPECT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clMemFreeINTEL(context, result);
  EXPECT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clReleaseKernel(kernel);
  EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
}

TEST_F(USMTest, setKernelArgMemPointer) {
  // Build program.
  const char *source[] = {"__kernel void test(const __global int *data,\n"
                          "  __global int *result, const __global int *val,\n"
                          "  __global void *foo) {\n"
                          "  *result = (int)(data[get_global_id(0)] == *val);\n"
                          "}\n"};
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, source, 1, m_program));
  ASSERT_NO_FATAL_FAILURE(
      testSetKernelArgMemPointer(m_context, m_device, m_queue, m_program));
}

TEST_F(USMTest, setKernelArgMemPointerMultiThreads) {
  // Build program.
  const char *source[] = {"__kernel void test(const __global int *data,\n"
                          "  __global int *result, const __global int *val,\n"
                          "  __global void *foo) {\n"
                          "  *result = (int)(data[get_global_id(0)] == *val);\n"
                          "}\n"};
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, source, 1, m_program));

  // Create out-of-order queue for this sub-test since it is probably not
  // thread-safe to enqueue concurrently into an in-order command queue.
  cl_int err;
  cl_command_queue_properties properties[] = {
      CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
  cl_command_queue queue =
      clCreateCommandQueueWithProperties(m_context, m_device, properties, &err);
  EXPECT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

  int numThreads = getMaxNumExternalThreads();
  tbb::parallel_for(tbb::blocked_range<int>(0, numThreads),
                    [&](tbb::blocked_range<int>(range)) {
                      for (int i = range.begin(); i < range.end(); ++i) {
                        ASSERT_NO_FATAL_FAILURE(testSetKernelArgMemPointer(
                            m_context, m_device, queue, m_program));
                      }
                    });

  err = clReleaseCommandQueue(queue);
  EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
}

TEST_F(USMTest, setKernelExecInfo) {
  cl_int err;
  // Allocate USM buffers.
  // Use large workload to test clMemBlockingFreeINTEL as well.
  size_t num = 16384;
  size_t size = num * sizeof(int);
  cl_uint alignment = 0;
  cl_int *bufA =
      (cl_int *)clHostMemAllocINTEL(m_context, nullptr, size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clHostMemAllocINTEL");

  cl_int *bufB = (cl_int *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                                 size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  // Shared system allocation.
  cl_int *bufC = (cl_int *)new char[size];

  struct Foo {
    cl_int *a;
    cl_int *b;
    cl_int *c;
  };
  Foo *foo = (Foo *)clSharedMemAllocINTEL(m_context, m_device, nullptr,
                                          sizeof(Foo), alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  for (size_t i = 0; i < num; i++) {
    bufA[i] = i;
    bufB[i] = i;
    bufC[i] = i;
  }
  foo->a = bufA;
  foo->b = bufB;
  foo->c = bufC;
  cl_int result = 0;

  // Build program and kernel.
  const char *source[] = {
      "typedef struct {\n"
      "  int *a;\n"
      "  int *b;\n"
      "  int *c;\n"
      "} Foo;\n"
      "kernel void test(const global Foo *foo, global int *result) {\n"
      "  size_t tid = get_global_id(0);\n"
      "  if (foo->a[tid] == foo->b[tid] && foo->a[tid] == foo->c[tid])\n"
      "    atomic_inc(result);\n"
      "}\n"};
  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, source, 1, m_program));
  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel test");

  err = clSetKernelArgMemPointerINTEL(m_kernel, 0, foo);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  // At this point, clEnqueueNDRangeKernel should fail as well. However, we
  // don't know if the kernel accesses indirect pointers.

  err = clSetKernelExecInfo(m_kernel, CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                            sizeof(Foo), foo);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");

  // clEnqueueNDRangeKernel should fail because access to bufA requires
  // CL_KERNEL_EXEC_INFO_INDIRECT_HOST_ACCESS_INTEL being set.
  size_t gdim = num;
  size_t ldim = 64;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_EQ(CL_INVALID_OPERATION, err, "clEnqueueNDRangeKernel");

  // bufA is host USM.
  cl_bool useIndirectHost = CL_TRUE;
  err = clSetKernelExecInfo(m_kernel,
                            CL_KERNEL_EXEC_INFO_INDIRECT_HOST_ACCESS_INTEL,
                            sizeof(useIndirectHost), &useIndirectHost);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");

  // clEnqueueNDRangeKernel should fail because access to bufB/bufC requires
  // CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL being true.
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_EQ(CL_INVALID_OPERATION, err, "clEnqueueNDRangeKernel");

  // bufB and bufC is shared USM.
  cl_bool useIndirectShared = CL_TRUE;
  err = clSetKernelExecInfo(m_kernel,
                            CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL,
                            sizeof(useIndirectShared), &useIndirectShared);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");

  // clEnqueueNDRangeKernel should succeed.
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  // Test clMemBlockingFreeINTEL when kernel indirectly accesses USM buffers.
  err = clMemBlockingFreeINTEL(m_context, bufA);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");
  err = clMemBlockingFreeINTEL(m_context, bufB);
  EXPECT_OCL_SUCCESS(err, "clMemBlockingFreeINTEL");

  // Check result
  EXPECT_EQ(result, num);

  // Test CL_FALSE for CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL.
  useIndirectShared = CL_FALSE;
  err = clSetKernelExecInfo(m_kernel,
                            CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL,
                            sizeof(useIndirectShared), &useIndirectShared);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");

  // clEnqueueNDRangeKernel should fail because access to bufB/bufC requires
  // CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL being true.
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_EQ(CL_INVALID_OPERATION, err, "clEnqueueNDRangeKernel");

  // Shared system allocations can't be released by blocking USM free.
  delete[] bufC;
  err = clMemFreeINTEL(m_context, foo);
  EXPECT_OCL_SUCCESS(err, "clMemFreeINTEL");
}
