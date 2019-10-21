// Copyright (c) 2019 Intel Corporation
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

#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include <CL/cl.h>
#include <gtest/gtest.h>

extern cl_device_type gDeviceType;

class USMTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, NULL, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  virtual void TearDown() {
    cl_int err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

  void BuildKernel(const char *source[], int count, const char *kernelName,
                   cl_program &program, cl_kernel &kernel) {
    cl_int err;
    program = clCreateProgramWithSource(m_context, count, source, NULL, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
    err = clBuildProgram(program, 1, &m_device, "", NULL, NULL);
    ASSERT_OCL_SUCCESS(err, "clBuildProgram");
    kernel = clCreateKernel(program, kernelName, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateKernel");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
};

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
                                                 "clGetMemAllocInfoINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(
      m_platform, "clSetKernelArgMemPointerINTEL");
  ASSERT_TRUE(nullptr != ptr);

  ptr = clGetExtensionFunctionAddressForPlatform(m_platform,
                                                 "clEnqueueMemsetINTEL");
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
  cl_unified_shared_memory_capabilities_intel caps_expected =
      CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_ATOMIC_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ACCESS_INTEL |
      CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ATOMIC_ACCESS_INTEL;
  for (cl_bitfield param_name : cap_params) {
    cl_unified_shared_memory_capabilities_intel caps;
    cl_int err = clGetDeviceInfo(
        m_device, param_name,
        sizeof(cl_unified_shared_memory_capabilities_intel), &caps, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
    ASSERT_EQ(caps, caps_expected);
  }
}

TEST_F(USMTest, memAlloc) {
  cl_int err;
  size_t size = 256;
  cl_uint alignment = 0;

  // Test correct flags
  const cl_mem_alloc_flags_intel correctFlags[] = {
      CL_MEM_ALLOC_WRITE_COMBINED_INTEL};
  for (cl_mem_flags flag : correctFlags) {
    cl_mem_properties_intel properties[] = {CL_MEM_ALLOC_FLAGS_INTEL, flag, 0};

    const void *buf0 =
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
                                       alignment, NULL);
    ASSERT_TRUE(nullptr != buf2);
    err = clMemFreeINTEL(m_context, buf2);
    ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");

    void *buf3 = clSharedMemAllocINTEL(m_context, nullptr, properties, size,
                                       alignment, NULL);
    ASSERT_TRUE(nullptr != buf3);
    err = clMemFreeINTEL(m_context, buf3);
    ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  }

  // Test wrong flags
  const cl_mem_alloc_flags_intel wrongFlags[] = {0};
  for (cl_mem_flags flag : wrongFlags) {
    cl_mem_properties_intel properties[] = {CL_MEM_ALLOC_FLAGS_INTEL, flag, 0};

    void *buf = clDeviceMemAllocINTEL(m_context, m_device, properties, size,
                                      alignment, &err);
    ASSERT_EQ(CL_INVALID_PROPERTY, err);
    ASSERT_TRUE(nullptr == buf);
  }
}

TEST_F(USMTest, getMemAllocInfo) {
  cl_int err;
  // Allocate USM buffers
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
                               sizeof(basePtr), &basePtr, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(buffer, basePtr);

  size_t bufferSize;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_SIZE_INTEL,
                               sizeof(bufferSize), &bufferSize, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(size, bufferSize);

  cl_device_id device;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_DEVICE_INTEL,
                               sizeof(device), &device, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(m_device, device);

  cl_mem_alloc_flags_intel flags;
  err = clGetMemAllocInfoINTEL(m_context, ptr, CL_MEM_ALLOC_FLAGS_INTEL,
                               sizeof(flags), &flags, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_ALLOC_WRITE_COMBINED_INTEL, flags);
}

TEST_F(USMTest, enqueueMemset) {
  cl_int err;
  // Allocate USM buffers
  size_t num = 1024;
  size_t size = num * sizeof(char);
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, m_device, NULL, size,
                                               alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  cl_int value1 = 1;
  err = clEnqueueMemsetINTEL(m_queue, buffer, value1, size, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");
  cl_int value2 = 2;
  err = clEnqueueMemsetINTEL(m_queue, buffer, value2, size / 2, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemsetINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  int countFillErrors = 0;
  for (size_t i = 0; i < num / 2; i++)
    if (buffer[i] != (char)value2)
      countFillErrors++;
  for (size_t i = num / 2; i < num; i++)
    if (buffer[i] != (char)value1)
      countFillErrors++;
  ASSERT_EQ(countFillErrors, 0);

  err = clMemFreeINTEL(m_context, buffer);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
}

TEST_F(USMTest, enqueueMemcpy) {
  cl_int err;

  // Allocate USM buffers
  size_t num = 1024;
  size_t size = num * sizeof(int);
  cl_uint alignment = sizeof(cl_long);
  cl_mem_properties_intel *properties = NULL;
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

  // Initialize buffer
  for (size_t i = 0; i < num; i++)
    buffer[i] = i;

  // Copy from host USM to device USM
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, buffer2, buffer, size, 0, NULL,
                             NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");

  // Copy from device USM to shared USM
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, buffer3, buffer2, size, 0, NULL,
                             NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  for (size_t i = 0; i < num; i++)
    ASSERT_EQ(buffer3[i], i);

  // Copy from device USM to arbitrary host memory
  int *bufferHost = new int[num];
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, bufferHost, buffer2, size, 0,
                             NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  for (size_t i = 0; i < num; i++) {
    ASSERT_EQ(bufferHost[i], i);
    bufferHost[i] = i + 1;
  }

  // Copy from arbitrary host memory to arbitrary host memory
  int *bufferHost2 = new int[num];
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, bufferHost2, bufferHost, size, 0,
                             NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemcpyINTEL");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  for (size_t i = 0; i < num; i++)
    ASSERT_EQ(bufferHost2[i], i + 1);

  // Test overlap
  size_t offset = size / 4;
  err = clEnqueueMemcpyINTEL(m_queue, CL_TRUE, (char *)buffer + offset, buffer,
                             size / 2, 0, NULL, NULL);
  EXPECT_OCL_EQ(CL_MEM_COPY_OVERLAP, err, "clEnqueueMemcpyINTEL");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  delete[] bufferHost;
  delete[] bufferHost2;
  err = clMemFreeINTEL(m_context, buffer);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clMemFreeINTEL(m_context, buffer2);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
  err = clMemFreeINTEL(m_context, buffer3);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");
}

TEST_F(USMTest, enqueueMigrateMem) {
  cl_int err;
  // Allocate USM buffers
  size_t size = 1024;
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, NULL, NULL, size,
                                               alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  cl_unified_shared_memory_type_intel memType;
  err = clGetMemAllocInfoINTEL(m_context, buffer, CL_MEM_ALLOC_TYPE_INTEL,
                               sizeof(memType), &memType, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_TYPE_SHARED_INTEL, memType);

  cl_mem_migration_flags flags = CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED;
  err = clEnqueueMigrateMemINTEL(m_queue, buffer, size, flags, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMigrateMemINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  err = clGetMemAllocInfoINTEL(m_context, buffer, CL_MEM_ALLOC_TYPE_INTEL,
                               sizeof(memType), &memType, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(CL_MEM_TYPE_DEVICE_INTEL, memType);

  cl_device_id device;
  err = clGetMemAllocInfoINTEL(m_context, buffer, CL_MEM_ALLOC_DEVICE_INTEL,
                               sizeof(device), &device, NULL);
  ASSERT_OCL_SUCCESS(err, "clGetMemAllocInfoINTEL");
  ASSERT_EQ(m_device, device);

  clMemFreeINTEL(m_context, buffer);
}

TEST_F(USMTest, enqueueAdviseMem) {
  cl_int err;
  // Allocate USM buffers
  size_t size = 1024;
  cl_uint alignment = 0;
  char *buffer = (char *)clSharedMemAllocINTEL(m_context, m_device, NULL, size,
                                               alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  cl_mem_advice_intel advice = CL_MEM_ADVICE_TBD0_INTEL;
  err = clEnqueueMemAdviseINTEL(m_queue, buffer, size, advice, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMemAdviseINTEL");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  clMemFreeINTEL(m_context, buffer);
}

TEST_F(USMTest, setKernelArgMemPointer) {
  cl_int err;
  // Allocate USM buffers
  size_t num = 1024;
  size_t size = num * sizeof(int);
  cl_uint alignment = 0;
  int *buffer = (int *)clSharedMemAllocINTEL(m_context, m_device, NULL, size,
                                             alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  int *result = (int *)clSharedMemAllocINTEL(m_context, m_device, NULL,
                                             sizeof(int), alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  for (int i = 0; i < num; i++) {
    buffer[i] = i;
  }

  // Build program and kernel
  const char *source[] = {"__kernel void test(const __global int *data,\n"
                          "  __global int *result, const __global int *val,\n"
                          "  __global void *foo) {\n"
                          "  *result = (int)(data[get_global_id(0)] == *val);\n"
                          "}\n"};
  cl_program program;
  cl_kernel kernel;
  BuildKernel(source, 1, "test", program, kernel);
  err = clSetKernelArgMemPointerINTEL(kernel, 1, result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  for (cl_int i = 0; i < num; i++) {
    err = clSetKernelArgMemPointerINTEL(kernel, 0, &buffer[i]);
    ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

    err = clSetKernelArgMemPointerINTEL(kernel, 2, &i);
    ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

    err = clSetKernelArgMemPointerINTEL(kernel, 3, nullptr);
    ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

    size_t gdim = 1;
    size_t ldim = 1;
    err = clEnqueueNDRangeKernel(m_queue, kernel, 1, NULL, &gdim, &ldim, 0,
                                 NULL, NULL);
    ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
    err = clFinish(m_queue);
    ASSERT_OCL_SUCCESS(err, "clFinish");

    ASSERT_EQ(*result, 1);
  }

  clMemFreeINTEL(m_context, buffer);
  clMemFreeINTEL(m_context, result);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
}

TEST_F(USMTest, setKernelExecInfo) {
  cl_int err;
  // Allocate USM buffers
  size_t num = 1024;
  size_t size = num * sizeof(int);
  cl_uint alignment = 0;
  cl_int *bufA =
      (cl_int *)clHostMemAllocINTEL(m_context, NULL, size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clHostMemAllocINTEL");

  cl_int *bufB = (cl_int *)clSharedMemAllocINTEL(m_context, m_device, NULL,
                                                 size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  struct Foo {
    cl_int *a;
    cl_int *b;
  };
  Foo *foo = (Foo *)clSharedMemAllocINTEL(m_context, m_device, NULL,
                                          sizeof(Foo), alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clSharedMemAllocINTEL");

  for (int i = 0; i < num; i++) {
    bufA[i] = i;
    bufB[i] = i;
  }
  foo->a = bufA;
  foo->b = bufB;
  cl_int result = 0;

  // Build program and kernel
  const char *source[] = {
      "typedef struct {\n"
      "  int *a;\n"
      "  int *b;\n"
      "} Foo;\n"
      "__kernel void test(const __global Foo *foo, __global int *result) {\n"
      "  size_t tid = get_global_id(0);\n"
      "  if (foo->a[tid] == foo->b[tid])\n"
      "    atomic_inc(result);\n"
      "}\n"};
  cl_program program;
  cl_kernel kernel;
  BuildKernel(source, 1, "test", program, kernel);

  err = clSetKernelArgMemPointerINTEL(kernel, 0, foo);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  err = clSetKernelArgMemPointerINTEL(kernel, 1, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  err = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL,
                            sizeof(Foo), foo);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");
  // bufA is host USM
  cl_bool useIndirectHost = CL_TRUE;
  err = clSetKernelExecInfo(kernel,
                            CL_KERNEL_EXEC_INFO_INDIRECT_HOST_ACCESS_INTEL,
                            sizeof(useIndirectHost), &useIndirectHost);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");
  // bufB is shared USM
  cl_bool useIndirectShared = CL_TRUE;
  err = clSetKernelExecInfo(kernel,
                            CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL,
                            sizeof(useIndirectShared), &useIndirectShared);
  ASSERT_OCL_SUCCESS(err, "clSetKernelExecInfo");

  size_t gdim = num;
  size_t ldim = 16;
  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, NULL, &gdim, &ldim, 0, NULL,
                               NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(result, num);

  clMemFreeINTEL(m_context, bufA);
  clMemFreeINTEL(m_context, bufB);
  clMemFreeINTEL(m_context, foo);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
}
