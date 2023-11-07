// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
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
#include "TestsHelpClasses.h"
#include "bi_tests.h"
#include "common_utils.h"
#include <math.h>

extern cl_device_type gDeviceType;

class ImplicitConversionsTest : public BITest {
public:
  ImplicitConversionsTest() {}

protected:
  void SetUp() override {
    // Make sure we will call the soft math builtins
    ASSERT_TRUE(SETENV("CL_CONFIG_CPU_TARGET_ARCH", "corei7"));
    m_options = "-cl-opt-disable -g";

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  void TearDown() override {
    cl_int err;
    if (m_kernel) {
      err = clReleaseKernel(m_kernel);
      ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
    }
    if (m_queue) {
      err = clReleaseCommandQueue(m_queue);
      ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    }
    if (m_program) {
      err = clReleaseProgram(m_program);
      ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      ASSERT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

protected:
  std::string m_options;
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_program m_program;
  cl_command_queue m_queue;
  cl_kernel m_kernel;
};

TEST_F(ImplicitConversionsTest, ConversionHalfFloat) {
  cl_int err;

  uint32_t nan32 = 0x7fc00000U;
  uint32_t inf32 = 0x7f800000U;

  const char *src = R"(
    #pragma OPENCL EXTENSION cl_khr_fp16 : enable

    kernel void test(global float * src, global float *dst) {
      size_t gid = get_global_id(0);
      float sf1 = src[gid];
      half h2 = sf1;
      dst[gid] = h2;
    }
  )";

  ASSERT_TRUE(BuildProgramSynch(m_context, 1, &src, nullptr, m_options.c_str(),
                                &m_program));

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  float input[4] = {*(float *)&nan32, *(float *)&inf32, 0.0f, 3.1415926535f},
        output[4];
  cl_mem arg1 =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(input), input, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  cl_mem arg2 =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(output), output, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &arg1);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  err = clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &arg2);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  size_t global_size = 4, local_size = 1;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &global_size,
                               &local_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clEnqueueReadBuffer(m_queue, arg2, CL_TRUE, 0, sizeof(output), &output,
                            0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  clFinish(m_queue);

  ASSERT_TRUE(isnan(output[0]));
  ASSERT_TRUE(isinf(output[1]));
  ASSERT_TRUE(output[2] == 0.0f);
  ASSERT_TRUE(fabs(input[3] - output[3]) < 0.01f);
}

TEST_F(ImplicitConversionsTest, ConversionHalfDouble) {
  cl_int err;

  uint64_t nan64 = 0x7ff8000000000000UL;
  uint64_t inf64 = 0x7ff0000000000000UL;

  const char *src = R"(
    #pragma OPENCL EXTENSION cl_khr_fp16 : enable

    kernel void test(global double * src, global double *dst) {
      size_t gid = get_global_id(0);
      double df1 = src[gid];
      half h2 = df1;
      dst[gid] = h2;
    }
  )";

  ASSERT_TRUE(BuildProgramSynch(m_context, 1, &src, nullptr, m_options.c_str(),
                                &m_program));

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  double input[4] = {*(double *)&nan64, *(double *)&inf64, 0.0, 3.1415926535},
         output[4];
  cl_mem arg1 =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(input), input, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  cl_mem arg2 =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(output), output, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &arg1);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  err = clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &arg2);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  size_t global_size = 4, local_size = 1;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &global_size,
                               &local_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clEnqueueReadBuffer(m_queue, arg2, CL_TRUE, 0, sizeof(output), &output,
                            0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  clFinish(m_queue);

  ASSERT_TRUE(isnan(output[0]));
  ASSERT_TRUE(isinf(output[1]));
  ASSERT_TRUE(output[2] == 0.0);
  ASSERT_TRUE(fabs(input[3] - output[3]) < 0.01);
}

#ifndef _WIN32
TEST_F(ImplicitConversionsTest, ConversionLongDoubleFloat) {
  cl_int err;

  uint32_t nan32 = 0x7fc00000U;
  uint32_t inf32 = 0x7f800000U;

  const char *src = R"(
    #pragma OPENCL EXTENSION cl_khr_fp16 : enable

    kernel void test(global float * src, global float *dst) {
      size_t gid = get_global_id(0);
      float sf1 = src[gid];
      long double tf2 = sf1;
      dst[gid] = tf2;
    }
  )";

  ASSERT_TRUE(BuildProgramSynch(m_context, 1, &src, nullptr, m_options.c_str(),
                                &m_program));

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  float input[4] = {*(float *)&nan32, *(float *)&inf32, 0.0f, 3.1415926535f},
        output[4];
  cl_mem arg1 =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(input), input, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  cl_mem arg2 =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(output), output, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &arg1);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  err = clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &arg2);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  size_t global_size = 4, local_size = 1;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &global_size,
                               &local_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clEnqueueReadBuffer(m_queue, arg2, CL_TRUE, 0, sizeof(output), &output,
                            0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  clFinish(m_queue);

  ASSERT_TRUE(isnan(output[0]));
  ASSERT_TRUE(isinf(output[1]));
  ASSERT_TRUE(output[2] == 0.0f);
  ASSERT_TRUE(input[3] == output[3]);
}

TEST_F(ImplicitConversionsTest, ConversionLongDoubleDouble) {
  cl_int err;

  uint64_t nan64 = 0x7ff8000000000000UL;
  uint64_t inf64 = 0x7ff0000000000000UL;

  const char *src = R"(
    #pragma OPENCL EXTENSION cl_khr_fp16 : enable

    kernel void test(global double * src, global double *dst) {
      size_t gid = get_global_id(0);
      double df1 = src[gid];
      long double tf2 = df1;
      dst[gid] = tf2;
    }
  )";

  ASSERT_TRUE(BuildProgramSynch(m_context, 1, &src, nullptr, m_options.c_str(),
                                &m_program));

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  double input[4] = {*(double *)&nan64, *(double *)&inf64, 0.0, 3.1415926535},
         output[4];
  cl_mem arg1 =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(input), input, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  cl_mem arg2 =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                     sizeof(output), output, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &arg1);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  err = clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &arg2);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  size_t global_size = 4, local_size = 1;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &global_size,
                               &local_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clEnqueueReadBuffer(m_queue, arg2, CL_TRUE, 0, sizeof(output), &output,
                            0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  clFinish(m_queue);

  ASSERT_TRUE(isnan(output[0]));
  ASSERT_TRUE(isinf(output[1]));
  ASSERT_TRUE(output[2] == 0.0);
  ASSERT_TRUE(input[3] == output[3]);
}
#endif
