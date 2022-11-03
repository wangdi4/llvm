//===--- multiple_devices.cpp -                                 -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for CL_CONFIG_CPU_EMULATE_DEVICES env variable
//
// ===--------------------------------------------------------------------=== //

#include "base_fixture.h"
#include "common_utils.h"

#include "gtest_wrapper.h"
#include <CL/cl.h>

#include <numeric>
#include <string>
#include <vector>

class MultipleDevicesBase : public OCLFPGABaseFixture,
                            public ::testing::WithParamInterface<int> {
protected:
  typedef OCLFPGABaseFixture parent_t;
  void SetUp() override {
    SETENV("CL_CONFIG_CPU_EMULATE_DEVICES", std::to_string(GetParam()).c_str());
    parent_t::SetUp();
  }

  void TearDown() override {
    parent_t::TearDown();
    UNSETENV("CL_CONFIG_CPU_EMULATE_DEVICES");
  }
};

TEST_P(MultipleDevicesBase, Simple) {
  std::vector<cl_context> contexts;
  std::vector<cl_command_queue> queues;

  for (cl_device_id device : devices()) {
    cl_context context = createContext(device);
    ASSERT_NE(nullptr, context) << "createContext failed";
    contexts.push_back(context);

    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(nullptr, queue) << "createCommandQueue failed";
    queues.push_back(queue);
  }

  const std::string program_sources = "                                      \n\
      __kernel void plus(__global int *a, __global int *b, __global int *c) {\n\
        size_t gid = get_global_id(0);                                       \n\
        c[gid] = a[gid] + b[gid];                                            \n\
      }";

  std::vector<cl_int> data_a(128), data_b(128);
  std::iota(data_a.begin(), data_a.end(), 0);
  std::iota(data_b.begin(), data_b.end(), 0);

  for (size_t i = 0; i < contexts.size(); ++i) {
    cl_program program = createAndBuildProgram(contexts[i], program_sources);
    ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

    cl_mem buf_a =
        createBuffer(contexts[i], sizeof(cl_int) * data_a.size(),
                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &data_a.front());
    ASSERT_NE(nullptr, buf_a) << "createBuffer failed";

    cl_mem buf_b =
        createBuffer(contexts[i], sizeof(cl_int) * data_b.size(),
                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &data_b.front());
    ASSERT_NE(nullptr, buf_b) << "createBuffer failed";

    cl_mem buf_c = createBuffer(contexts[i], sizeof(cl_int) * data_b.size(),
                                CL_MEM_WRITE_ONLY);
    ASSERT_NE(nullptr, buf_c) << "createBuffer failed";

    cl_kernel kernel = createKernel(program, "plus");
    ASSERT_NE(nullptr, kernel) << "createKernel failed";

    cl_int error = CL_SUCCESS;
    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clSetKernelArg failed with error " << ErrToStr(error);
    error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clSetKernelArg failed with error " << ErrToStr(error);
    error = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_c);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clSetKernelArg failed with error " << ErrToStr(error);

    const size_t num_dims = 1;
    const size_t global_size[num_dims] = {data_a.size()};

    error = clEnqueueNDRangeKernel(queues[i], kernel, num_dims, nullptr,
                                   global_size, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);

    std::vector<cl_int> data_c(data_a.size(), 0);
    error = clEnqueueReadBuffer(queues[i], buf_c, CL_TRUE, 0,
                                sizeof(cl_int) * data_c.size(), &data_c.front(),
                                0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueReadBuffer failed with error " << ErrToStr(error);

    for (size_t i = 0; i < data_c.size(); ++i) {
      ASSERT_EQ(data_a[i] + data_b[i], data_c[i])
          << "invalid value of " << i << "-th element of the array";
    }
  }
}

TEST_P(MultipleDevicesBase, DeviceMismatch) {
  if (GetParam() > 1) { // There are multi devices.
    cl_context context0 = createContext(devices()[0]);
    cl_int error = CL_SUCCESS;
    const char *sources_str = "";
    cl_program program_with_source =
        clCreateProgramWithSource(context0, 1, &sources_str, nullptr, &error);
    EXPECT_EQ(CL_SUCCESS, error)
        << "clCreateProgramWithSource failed with error " << ErrToStr(error);
    cl_device_id seconddevice = devices()[1];
    // Invoke clBuildProgram with another device which is not associated with
    // this program, CL_INVALID_DEVICE should be returned.
    error = clBuildProgram(program_with_source, 1, &seconddevice, nullptr,
                           nullptr, nullptr);
    EXPECT_EQ(CL_INVALID_DEVICE, error)
        << "clBuildProgram does not return expected error " << ErrToStr(error);
  }
}

TEST_P(MultipleDevicesBase, SharedContext) {
  cl_context context = createContext(devices());
  ASSERT_NE(nullptr, context) << "createContext failed";

  std::vector<cl_command_queue> queues;

  for (cl_device_id device : devices()) {
    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(nullptr, queue) << "createCommandQueue failed";
    queues.push_back(queue);
  }

  const std::string program_sources = "                                      \n\
    __kernel void plus(__global int *a, __global int *b) {                   \n\
      size_t gid = get_global_id(0);                                         \n\
      b[gid] += a[gid];                                                      \n\
    }";

  cl_program program = createAndBuildProgram(context, program_sources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  std::vector<cl_int> data_a(128, 1), data_b(128, 0);

  cl_mem buf_a =
      createBuffer(context, sizeof(cl_int) * data_a.size(),
                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &data_a.front());
  ASSERT_NE(nullptr, buf_a) << "createBuffer failed";

  cl_mem buf_b =
      createBuffer(context, sizeof(cl_int) * data_b.size(),
                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &data_b.front());
  ASSERT_NE(nullptr, buf_b) << "createBuffer failed";

  cl_kernel kernel = createKernel(program, "plus");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);
  error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  const size_t num_dims = 1;
  const size_t global_size[num_dims] = {data_a.size()};

  for (size_t i = 0; i < queues.size(); ++i) {
    error = clEnqueueNDRangeKernel(queues[i], kernel, num_dims, nullptr,
                                   global_size, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
    clFinish(queues[i]);
  }

  std::vector<cl_int> data_c(data_a.size(), 0);
  error = clEnqueueReadBuffer(queues[0], buf_b, CL_TRUE, 0,
                              sizeof(cl_int) * data_c.size(), &data_c.front(),
                              0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueReadBuffer failed with error " << ErrToStr(error);

  for (size_t i = 0; i < data_c.size(); ++i) {
    ASSERT_EQ((cl_int)num_devices(), data_c[i])
        << "invalid value of " << i << "-th element of the array";
  }
}

TEST_P(MultipleDevicesBase, WithAutorun) {
  cl_context context = createContext(devices());
  ASSERT_NE(nullptr, context) << "createContext failed";

  std::vector<cl_command_queue> queues;

  for (cl_device_id device : devices()) {
    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(nullptr, queue) << "createCommandQueue failed";
    queues.push_back(queue);
  }

  const std::string program_sources = "                                      \n\
    #pragma OPENCL EXTENSION cl_intel_channels : enable                      \n\
    __attribute__((depth(128)))                                              \n\
    channel int in;                                                          \n\
    __attribute__((depth(128)))                                              \n\
    channel int out;                                                         \n\
    __attribute__((autorun))                                                 \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void autoplus() {                                               \n\
      int i = read_channel_intel(in);                                        \n\
      write_channel_intel(out, i + 1);                                       \n\
    }                                                                        \n\
                                                                             \n\
    __kernel void plus(__global int *a, __global int *b) {                   \n\
      size_t gid = get_global_id(0);                                         \n\
      write_channel_intel(in, a[gid]);                                       \n\
      b[gid] += read_channel_intel(out);                                     \n\
    }";

  cl_program program = createAndBuildProgram(context, program_sources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  std::vector<cl_int> data_a(128, 1), data_b(128, 0);

  cl_mem buf_a =
      createBuffer(context, sizeof(cl_int) * data_a.size(),
                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &data_a.front());
  ASSERT_NE(nullptr, buf_a) << "createBuffer failed";

  cl_mem buf_b =
      createBuffer(context, sizeof(cl_int) * data_b.size(),
                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &data_b.front());
  ASSERT_NE(nullptr, buf_b) << "createBuffer failed";

  cl_kernel kernel = createKernel(program, "plus");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);
  error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  const size_t num_dims = 1;
  const size_t global_size[num_dims] = {data_a.size()};

  for (size_t i = 0; i < queues.size(); ++i) {
    error = clEnqueueNDRangeKernel(queues[i], kernel, num_dims, nullptr,
                                   global_size, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
    clFinish(queues[i]);
  }

  std::vector<cl_int> data_c(data_a.size(), 0);
  error = clEnqueueReadBuffer(queues[0], buf_b, CL_TRUE, 0,
                              sizeof(cl_int) * data_c.size(), &data_c.front(),
                              0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueReadBuffer failed with error " << ErrToStr(error);

  for (size_t i = 0; i < data_c.size(); ++i) {
    ASSERT_EQ(2 * (cl_int)num_devices(), data_c[i])
        << "invalid value of " << i << "-th element of the array";
  }
}

TEST_P(MultipleDevicesBase, CheckThatChannelsAreNotShared) {
  cl_context context = createContext(devices());
  ASSERT_NE(nullptr, context) << "createContext failed";

  std::vector<cl_command_queue> queues;

  for (cl_device_id device : devices()) {
    cl_command_queue queue = createCommandQueue(context, device);
    ASSERT_NE(nullptr, queue) << "createCommandQueue failed";
    queues.push_back(queue);
  }

  const std::string program_sources = "                                      \n\
    #pragma OPENCL EXTENSION cl_intel_channels : enable                      \n\
    channel int in;                                                          \n\
    __kernel void plus(int a) {                                              \n\
      write_channel_intel(in, a);                                            \n\
    }                                                                        \n\
                                                                             \n\
    __kernel void test(__global int* res) {                                  \n\
      res[0] = read_channel_intel(in);                                       \n\
      bool valid = true;                                                     \n\
      int tmp = read_channel_nb_intel(in, &valid);                           \n\
      res[1] = valid ? 1 : 0;                                                \n\
    }";

  cl_program program = createAndBuildProgram(context, program_sources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_mem buf_r = createBuffer(context, sizeof(cl_int) * 2, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, buf_r) << "createBuffer failed";

  cl_kernel plus = createKernel(program, "plus");
  ASSERT_NE(nullptr, plus) << "createKernel failed";

  cl_kernel test = createKernel(program, "test");
  ASSERT_NE(nullptr, test) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clSetKernelArg(test, 0, sizeof(cl_mem), &buf_r);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  const size_t num_dims = 1;
  const size_t global_size[num_dims] = {1};

  for (size_t i = 0; i < queues.size(); ++i) {
    cl_int num = i + 1;
    error = clSetKernelArg(plus, 0, sizeof(cl_int), &num);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clSetKernelArg failed with error " << ErrToStr(error);

    error = clEnqueueNDRangeKernel(queues[i], plus, num_dims, nullptr,
                                   global_size, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
    clFinish(queues[i]);
  }

  for (size_t i = 0; i < queues.size(); ++i) {
    cl_int pattern = -1;
    error = clEnqueueFillBuffer(queues[i], buf_r, &pattern, sizeof(cl_int), 0,
                                sizeof(cl_int) * 2, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueFillBuffer failed with error " << ErrToStr(error);

    error = clEnqueueNDRangeKernel(queues[i], test, num_dims, nullptr,
                                   global_size, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);

    cl_int result[2];
    error =
        clEnqueueReadBuffer(queues[i], buf_r, CL_TRUE, 0, sizeof(cl_int) * 2,
                            result, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clEnqueueReadBuffer failed with error " << ErrToStr(error);
    ASSERT_EQ((cl_int)i + 1, result[0])
        << "invalid result for " << i << "-th device";
    ASSERT_EQ(0, result[1]) << "invlid result for " << i << "-th device";
  }
}

INSTANTIATE_TEST_SUITE_P(TestFPGAMultiDevice, MultipleDevicesBase,
                         ::testing::Values(1, 2, 3, 10));
