// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

/*******************************************************************************
 * clBuildProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create program with source
 * (4) build program
 * (5) build program again
 ******************************************************************************/

#include "CL/cl.h"
#include "FrameworkTestThreads.h"
#include "TestsHelpClasses.h"

extern cl_device_type gDeviceType;

class BuildProgramTwiceTest : public ::testing::Test {
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

    source = R"(kernel void test(global int *dst)
    {
      dst[0] = VALUE;
    })";
  }

  virtual void TearDown() override {
    cl_int err;
    if (m_program) {
      err = clReleaseProgram(m_program);
      EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
    }
    if (m_queue) {
      err = clReleaseCommandQueue(m_queue);
      EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      EXPECT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

protected:
  void testProgram(int value);

  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context = nullptr;
  cl_command_queue m_queue = nullptr;
  cl_program m_program = nullptr;
  const char *source;
};

void BuildProgramTwiceTest::testProgram(int value) {
  cl_int err;
  cl_kernel kernel = clCreateKernel(m_program, "test", &err);
  EXPECT_OCL_SUCCESS(err, "clCreateKernel");

  int dst = 0;

  err = clSetKernelArgMemPointerINTEL(kernel, 0, &dst);
  EXPECT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  size_t gdims[1] = {1};
  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, gdims, nullptr, 0,
                               nullptr, nullptr);
  EXPECT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  EXPECT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(dst, value);

  err = clReleaseKernel(kernel);
  EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
}

TEST_F(BuildProgramTwiceTest, sameOptions) {
  cl_int err;
  m_program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  int value = 1;
  std::string buildOption = "-DVALUE=" + std::to_string(value);
  err = clBuildProgram(m_program, 1, &m_device, buildOption.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "first clBuildProgram");

  ASSERT_NO_FATAL_FAILURE(testProgram(value));

  err = clBuildProgram(m_program, 1, &m_device, buildOption.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "second clBuildProgram");

  ASSERT_NO_FATAL_FAILURE(testProgram(value));
}

TEST_F(BuildProgramTwiceTest, differentOptions) {
  cl_int err;
  m_program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  int value = 1;
  std::string buildOption = "-cl-opt-disable -DVALUE=" + std::to_string(value);
  err = clBuildProgram(m_program, 1, &m_device, buildOption.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "first clBuildProgram");

  ASSERT_NO_FATAL_FAILURE(testProgram(value));

  value = 2;
  std::string buildOption2 = "-cl-opt-disable -DVALUE=" + std::to_string(value);
  err = clBuildProgram(m_program, 1, &m_device, buildOption2.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "second clBuildProgram");

  ASSERT_NO_FATAL_FAILURE(testProgram(value));
}
