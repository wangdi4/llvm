// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

class CPUForceMaxWGSizeTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    // Set env
    m_cpuMaxWGSize = 1 << 20;
    std::string cpuMaxWGSizeStr = std::to_string(m_cpuMaxWGSize);
    m_envName = "CL_CONFIG_CPU_FORCE_MAX_WORK_GROUP_SIZE";
    ASSERT_TRUE(SETENV(m_envName.c_str(), cpuMaxWGSizeStr.c_str()))
        << ("Failed to set env " + m_envName);

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

  virtual void TearDown() override {
    // Unset env
    ASSERT_TRUE(UNSETENV(m_envName.c_str()))
        << ("Failed to unset env " + m_envName);
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;

  std::string m_envName;
  size_t m_cpuMaxWGSize;
};

TEST_F(CPUForceMaxWGSizeTest, deviceQuery) {
  size_t maxWGSize;
  cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                               sizeof(size_t), &maxWGSize, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
  EXPECT_EQ(m_cpuMaxWGSize, maxWGSize);
}

TEST_F(CPUForceMaxWGSizeTest, kernel) {
  cl_int err;
  cl_context context =
      clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, m_device, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

  // Create program
  const char *source = "kernel void test(global int* dst) {\n"
                       "  dst[get_global_id(0)] = 1;\n"
                       "}";
  cl_program program =
      clCreateProgramWithSource(context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  // Build program
  err = clBuildProgram(program, 1, &m_device, nullptr, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Create kernel
  cl_kernel kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  // Check maximum work-group size that can be used for the kernel
  size_t kernelWGInfoSize;
  err = clGetKernelWorkGroupInfo(kernel, m_device, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(size_t), &kernelWGInfoSize, nullptr);
  ASSERT_EQ(m_cpuMaxWGSize, kernelWGInfoSize);

  // Run kernel
  std::vector<int> buffer(m_cpuMaxWGSize);
  err = clSetKernelArgMemPointerINTEL(kernel, 0, &buffer[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  size_t gdim = m_cpuMaxWGSize;
  size_t ldim = m_cpuMaxWGSize;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, &ldim, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  for (size_t i = 0; i < buffer.size(); ++i)
    ASSERT_EQ(buffer[i], 1);

  // Release resources
  err = clReleaseKernel(kernel);
  ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
  err = clReleaseCommandQueue(queue);
  ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
  err = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");
}
