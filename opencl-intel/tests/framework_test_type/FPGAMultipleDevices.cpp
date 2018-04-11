//==--- FPGAMultipleDevices.cpp                            - OpenCL C -*-=====//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include <gtest/gtest.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

#include "FrameworkTest.h"
#include "common_utils.h"

#include <vector>
#include <string>
#include <numeric>

extern cl_device_type gDeviceType;

class FPGAMultiDeviceTestBase : public ::testing::TestWithParam<int> {
protected:
  virtual void SetUp();
  virtual void TearDown() {}

  cl_platform_id            m_platform;
  std::vector<cl_device_id> m_devices;
};

class FPGAMultiDevice : public FPGAMultiDeviceTestBase {
public:
  typedef FPGAMultiDeviceTestBase parent_t;
protected:
  virtual void SetUp();
  virtual void TearDown();

  void setupContextForEachDevice();
  void setupSharedContext();
  void setupQueueForEachDevice();

  std::vector<cl_command_queue> m_queues;
  std::vector<cl_context>       m_contexts;
};

void FPGAMultiDeviceTestBase::SetUp() {
  SETENV("CL_CONFIG_CPU_EMULATE_DEVICES", std::to_string(GetParam()).c_str());

  cl_int error = CL_SUCCESS;
  error = clGetPlatformIDs(1, &m_platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetPlatformIDs failed.";

  cl_uint numDevices = 0;
  error = clGetDeviceIDs(m_platform, gDeviceType, 0, nullptr, &numDevices);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clGetDeviceIDs failed on trying to obtain number of devices of "
      << gDeviceType << " device type.";
  ASSERT_EQ(GetParam(), numDevices)
      << "clGetDeviceIDs returned invalid number of devices.";

  m_devices.assign(numDevices, nullptr);
  error = clGetDeviceIDs(m_platform, gDeviceType, numDevices,
                         &m_devices.front(), nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetDeviceIDs failed on trying to obtain "
                              << gDeviceType << " device type.";
}

void FPGAMultiDevice::setupContextForEachDevice() {
  m_contexts.assign(m_devices.size(), nullptr);

  cl_int error = CL_SUCCESS;
  const cl_context_properties props[5] = {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)m_platform,
      CL_CONTEXT_FPGA_EMULATOR_INTEL,
      CL_TRUE,
      0
  };

  for (size_t i = 0; i < m_devices.size(); ++i) {
    m_contexts[i] =
        clCreateContext(props, 1, &m_devices[i], nullptr, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clCreateContext failed for " << i << "-th device";
  }
}

void FPGAMultiDevice::setupSharedContext() {
  const cl_context_properties props[5] = {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)m_platform,
      CL_CONTEXT_FPGA_EMULATOR_INTEL,
      CL_TRUE,
      0
  };

  m_contexts.resize(1);
  cl_int error = CL_SUCCESS;
  m_contexts[0] = clCreateContext(props, m_devices.size(), &m_devices.front(),
                                  nullptr, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clCreateContext failed for shared context";
}

void FPGAMultiDevice::setupQueueForEachDevice() {
  m_queues.assign(m_devices.size(), nullptr);

  cl_int error = CL_SUCCESS;
  for (size_t i = 0; i < m_devices.size(); ++i) {
    cl_context context =
        (m_contexts.size() == 1) ? m_contexts.front() : m_contexts[i];
    m_queues[i] = clCreateCommandQueueWithProperties(
        context, m_devices[i], nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << "clCreateCommanndQueueWithProperties failed for " << i
        << "-th device";
  }
}

void FPGAMultiDevice::SetUp() {
  parent_t::SetUp();
}

void FPGAMultiDevice::TearDown() {
  for (size_t i = 0; i < m_queues.size(); ++i) {
    clReleaseCommandQueue(m_queues[i]);
  }
  for (size_t i = 0; i < m_contexts.size(); ++i) {
    clReleaseContext(m_contexts[i]);
  }

  parent_t::TearDown();
}

TEST_P(FPGAMultiDevice, Simple) {
  ASSERT_NO_FATAL_FAILURE(setupContextForEachDevice());
  ASSERT_NO_FATAL_FAILURE(setupQueueForEachDevice());
  for (size_t i = 0; i < m_devices.size(); ++i) {
    cl_mem bufA, bufB, bufC;
    std::vector<cl_int> inputA(128), inputB(128);
    std::iota(inputA.begin(), inputA.end(), 0);
    std::iota(inputB.rbegin(), inputB.rend(), 0);

    const char* programSources = "                                             \n\
      __kernel void plus(__global int *a, __global int *b, __global int *c) {  \n\
        size_t gid = get_global_id(0);                                         \n\
        c[gid] = a[gid] + b[gid];                                              \n\
      }";

    cl_int error = CL_SUCCESS;
    cl_program program = clCreateProgramWithSource(
        m_contexts[i], 1, &programSources, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed";

    error = clBuildProgram(program, 1, &m_devices[i], "", nullptr, nullptr);
    EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
    if (CL_SUCCESS != error) {
      size_t logSize = 0;
      error = clGetProgramBuildInfo(program, m_devices[i], CL_PROGRAM_BUILD_LOG,
                                    0, nullptr, &logSize);
      ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
      std::string log("", logSize);
      error = clGetProgramBuildInfo(program, m_devices[i], CL_PROGRAM_BUILD_LOG,
                                    logSize, &log[0], nullptr);
      ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
      std::cout << log << std::endl;
      return;
    }

    bufA =
        clCreateBuffer(m_contexts[i], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       sizeof(cl_int) * inputA.size(), &inputA.front(), &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";
    bufB =
        clCreateBuffer(m_contexts[i], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                       sizeof(cl_int) * inputB.size(), &inputB.front(), &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";
    bufC = clCreateBuffer(m_contexts[i], CL_MEM_WRITE_ONLY,
                          sizeof(cl_int) * inputA.size(), nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";

    cl_kernel kernel = clCreateKernel(program, "plus", &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
    ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";
    error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
    ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";
    error = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
    ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";

    cl_uint ndims = 1;
    size_t ws = 128;
    error = clEnqueueNDRangeKernel(m_queues[i], kernel, ndims, nullptr, &ws,
                                   nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed.";
    std::vector<cl_int> output(128);
    error = clEnqueueReadBuffer(m_queues[i], bufC, CL_TRUE, 0,
                                sizeof(cl_int) * output.size(), &output.front(),
                                0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueReadBuffer failed.";

    for (size_t i = 0; i < output.size(); ++i) {
      ASSERT_EQ(inputA[i] + inputB[i], output[i])
          << i << "-th element of result array is incorrect.";
    }
  }
}

TEST_P(FPGAMultiDevice, SharedContext) {
  ASSERT_NO_FATAL_FAILURE(setupSharedContext());
  ASSERT_NO_FATAL_FAILURE(setupQueueForEachDevice());

  const char* programSources = "                                             \n\
    __kernel void plus(__global int *a, __global int *b) {                   \n\
      size_t gid = get_global_id(0);                                         \n\
      b[gid] += a[gid];                                                      \n\
    }";

  cl_int error = CL_SUCCESS;
  cl_program program = clCreateProgramWithSource(
      m_contexts.front(), 1, &programSources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed.";

  error = clBuildProgram(program, m_devices.size(), &m_devices.front(), "",
                         nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
  if (CL_SUCCESS != error) {
    size_t logSize = 0;
    error = clGetProgramBuildInfo(program, m_devices[0], CL_PROGRAM_BUILD_LOG,
                                  0, nullptr, &logSize);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::string log("", logSize);
    error = clGetProgramBuildInfo(program, m_devices[0], CL_PROGRAM_BUILD_LOG,
                                  logSize, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::cout << log << std::endl;
    return;
  }

  cl_mem bufA, bufB;
  std::vector<cl_int> inputA(128, 1), inputB(128, 0);
  bufA =
      clCreateBuffer(m_contexts[0], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(cl_int) * inputA.size(), &inputA.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";
  bufB =
      clCreateBuffer(m_contexts[0], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(cl_int) * inputB.size(), &inputB.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";

  cl_kernel kernel = clCreateKernel(program, "plus", &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

  error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
  ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";
  error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
  ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";

  for (size_t i = 0; i < m_devices.size(); ++i) {
    cl_uint ndims = 1;
    size_t ws = 128;
    error = clEnqueueNDRangeKernel(m_queues[i], kernel, ndims, nullptr, &ws,
                                   nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed.";
    clFinish(m_queues[i]);
  }

  std::vector<cl_int> output(128);
  error = clEnqueueReadBuffer(m_queues[0], bufB, CL_TRUE, 0,
                              sizeof(cl_int) * output.size(), &output.front(),
                              0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueReadBuffer failed.";

  for (size_t i = 0; i < output.size(); ++i) {
    ASSERT_EQ((cl_int)m_devices.size(), output[i])
        << i << "-th element of result array is wrong.";
  }
}

TEST_P(FPGAMultiDevice, WithAutorun) {
  ASSERT_NO_FATAL_FAILURE(setupSharedContext());
  ASSERT_NO_FATAL_FAILURE(setupQueueForEachDevice());

  const char* programSources = "                                             \n\
    #pragma OPENCL EXTENSION cl_intel_channels : enable                      \n\
    channel int in;                                                          \n\
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

  cl_int error = CL_SUCCESS;
  cl_program program = clCreateProgramWithSource(
      m_contexts.front(), 1, &programSources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed.";

  error = clBuildProgram(program, m_devices.size(), &m_devices.front(), "",
                         nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
  if (CL_SUCCESS != error) {
    size_t logSize = 0;
    error = clGetProgramBuildInfo(program, m_devices[0], CL_PROGRAM_BUILD_LOG,
                                  0, nullptr, &logSize);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::string log("", logSize);
    error = clGetProgramBuildInfo(program, m_devices[0], CL_PROGRAM_BUILD_LOG,
                                  logSize, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::cout << log << std::endl;
    return;
  }

  cl_mem bufA, bufB;
  std::vector<cl_int> inputA(128, 1), inputB(128, 0);
  bufA =
      clCreateBuffer(m_contexts[0], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(cl_int) * inputA.size(), &inputA.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";
  bufB =
      clCreateBuffer(m_contexts[0], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(cl_int) * inputB.size(), &inputB.front(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";

  cl_kernel kernel = clCreateKernel(program, "plus", &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

  error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
  ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";
  error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
  ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";

  for (size_t i = 0; i < m_devices.size(); ++i) {
    cl_uint ndims = 1;
    size_t ws = 128;
    error = clEnqueueNDRangeKernel(m_queues[i], kernel, ndims, nullptr, &ws,
                                   nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed.";
    clFinish(m_queues[i]);
  }

  std::vector<cl_int> output(128);
  error = clEnqueueReadBuffer(m_queues[0], bufB, CL_TRUE, 0,
                              sizeof(cl_int) * output.size(), &output.front(),
                              0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueReadBuffer failed.";

  for (size_t i = 0; i < output.size(); ++i) {
    ASSERT_EQ((cl_int)m_devices.size() * 2, output[i])
        << i << "-th element of result array is invalid.";
  }
}

TEST_P(FPGAMultiDevice, CheckThatChannelsAreNotShared) {
  ASSERT_NO_FATAL_FAILURE(setupSharedContext());
  ASSERT_NO_FATAL_FAILURE(setupQueueForEachDevice());

  const char* programSources = "                                             \n\
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

  cl_int error = CL_SUCCESS;
  cl_program program = clCreateProgramWithSource(
      m_contexts.front(), 1, &programSources, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed.";

  error = clBuildProgram(program, m_devices.size(), &m_devices.front(), "",
                         nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
  if (CL_SUCCESS != error) {
    size_t logSize = 0;
    error = clGetProgramBuildInfo(program, m_devices[0], CL_PROGRAM_BUILD_LOG,
                                  0, nullptr, &logSize);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::string log("", logSize);
    error = clGetProgramBuildInfo(program, m_devices[0], CL_PROGRAM_BUILD_LOG,
                                  logSize, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
    std::cout << log << std::endl;
    return;
  }

  cl_mem bufR;
  bufR = clCreateBuffer(m_contexts[0], CL_MEM_WRITE_ONLY, sizeof(cl_int) * 2,
                        nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";

  cl_kernel plus = clCreateKernel(program, "plus", &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

  cl_kernel test = clCreateKernel(program, "test", &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

  error = clSetKernelArg(test, 0, sizeof(cl_mem), &bufR);
  ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";

  cl_uint ndims = 1;
  size_t ws = 1;
  for (size_t i = 0; i < m_devices.size(); ++i) {
    cl_int num = i + 1;
    error = clSetKernelArg(plus, 0, sizeof(cl_int), &num);
    ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed.";

    error = clEnqueueNDRangeKernel(m_queues[i], plus, ndims, nullptr, &ws,
                                   nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed.";
    clFinish(m_queues[i]);
  }

  for (size_t i = 0; i < m_devices.size(); ++i) {
    cl_int pattern = -1;
    error = clEnqueueFillBuffer(m_queues[i], bufR, &pattern, sizeof(cl_int), 0,
                                sizeof(cl_int) * 2, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueFillBuffer failed.";
    error = clEnqueueNDRangeKernel(m_queues[i], test, ndims, nullptr, &ws,
                                   nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed.";
    cl_int result[2];
    error = clEnqueueReadBuffer(m_queues[i], bufR, CL_TRUE, 0,
                                sizeof(cl_int) * 2, result,
                                0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueReadBuffer failed.";
    ASSERT_EQ(i + 1, result[0]);
    ASSERT_EQ(0, result[1]);
  }
}

#ifdef BUILD_FPGA_EMULATOR
INSTANTIATE_TEST_CASE_P(Test, FPGAMultiDevice, ::testing::Values(1, 2, 3, 10));
#endif // BUILD_FPGA_EMULATOR
