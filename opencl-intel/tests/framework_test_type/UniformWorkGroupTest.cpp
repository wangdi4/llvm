//===-- UniformWorkGroupTest.cpp - Uniform Work Group Test -----*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "CL.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"

extern cl_device_type gDeviceType;

static constexpr unsigned N = 3;
static const char *source[] = {"kernel void dummy_kernel() {}"};

static const char *compileFlags[N] = {
    "", "-cl-std=CL2.0", "-cl-std=CL2.0 -cl-uniform-work-group-size"};
static constexpr cl_int expectedResults[N] = {
    CL_INVALID_WORK_GROUP_SIZE, CL_SUCCESS, CL_INVALID_WORK_GROUP_SIZE};
static constexpr size_t localSize[] = {4};
static constexpr size_t globalSize[] = {7};

class UniformWorkGroupTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_platform_id platform;
    cl_int err = clGetPlatformIDs(1, &platform, nullptr);
    ASSERT_OCL_SUCCESS(err, clGetPlatformIDs);

    err = clGetDeviceIDs(platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, clGetDeviceIDs);

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, clCreateContext);

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, clCreateCommandQueueWithProperties);
  }

  virtual void TearDown() override {
    cl_int err;
    err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, clReleaseCommandQueue);
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, clReleaseContext);
  }

protected:
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
};

TEST_F(UniformWorkGroupTest, CompileAndLink) {
  cl_int err;
  cl_program program[N] = {nullptr};
  cl_program program2[N] = {nullptr};

  for (unsigned i = 0; i < N; ++i) {
    std::string errMsg =
        "Compile flag '" + std::string(compileFlags[i]) + "' failed";

    // Create program
    program[i] = clCreateProgramWithSource(m_context, 1, source, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, clCreateProgramWithSource);

    // Build program
    err = clCompileProgram(program[i], 1, &m_device, compileFlags[i], 0,
                           nullptr, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, clCompileProgram) << errMsg;

    program2[i] = clLinkProgram(m_context, 1, &m_device, nullptr, 1,
                                &program[i], nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, clLinkProgram) << errMsg;

    // Create kernel
    cl_kernel kernel = clCreateKernel(program2[i], "dummy_kernel", &err);
    ASSERT_OCL_SUCCESS(err, clCreateKernel) << errMsg;

    // Enqueue kernel
    err = clEnqueueNDRangeKernel(m_queue, kernel, 1, NULL, globalSize,
                                 localSize, 0, NULL, NULL);
    ASSERT_OCL_EQ(expectedResults[i], err, clEnqueueNDRangeKernel) << errMsg;

    err = clFinish(m_queue);
    ASSERT_OCL_SUCCESS(err, clFinish) << errMsg;

    err = clReleaseKernel(kernel);
    ASSERT_OCL_SUCCESS(err, clReleaseKernel) << errMsg;
  }

  for (unsigned i = 0; i < N; ++i) {
    err = clReleaseProgram(program[i]);
    EXPECT_OCL_SUCCESS(err, clReleaseProgram);
    err = clReleaseProgram(program2[i]);
    EXPECT_OCL_SUCCESS(err, clReleaseProgram);
  }
}

TEST_F(UniformWorkGroupTest, Build) {
  cl_int err;
  // Create program
  cl_program program =
      clCreateProgramWithSource(m_context, 1, source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, clCreateProgramWithSource);

  for (unsigned i = 0; i < N; ++i) {
    std::string errMsg =
        "Compile flag '" + std::string(compileFlags[i]) + "' failed";

    // Build program
    err =
        clBuildProgram(program, 0, nullptr, compileFlags[i], nullptr, nullptr);
    ASSERT_OCL_SUCCESS(err, clBuildProgram) << errMsg;

    // Create kernel
    cl_kernel kernel = clCreateKernel(program, "dummy_kernel", &err);
    ASSERT_OCL_SUCCESS(err, clCreateKernel) << errMsg;

    // Enqueue kernel
    err = clEnqueueNDRangeKernel(m_queue, kernel, 1, NULL, globalSize,
                                 localSize, 0, NULL, NULL);
    ASSERT_OCL_EQ(expectedResults[i], err, clEnqueueNDRangeKernel) << errMsg;

    err = clFinish(m_queue);
    ASSERT_OCL_SUCCESS(err, clFinish) << errMsg;

    err = clReleaseKernel(kernel);
    ASSERT_OCL_SUCCESS(err, clReleaseKernel) << errMsg;
  }

  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, clReleaseProgram);
}
