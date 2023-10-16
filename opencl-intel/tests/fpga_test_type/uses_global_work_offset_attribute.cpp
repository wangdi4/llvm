//===--- uses_global_work_offset_attribute.cpp -                -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for uses_global_work_offset(0) attribute check
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"
#include "base_fixture.h"
#include "gtest_wrapper.h"
#include <string>

const size_t num_dims = 1;
const size_t work_size[num_dims] = {128};
const size_t work_offset[num_dims] = {1};

class TestCanUseGlobalWorkOffsetBase : public OCLFPGABaseFixture {
protected:
  void SetUp() override {
    OCLFPGABaseFixture::SetUp();

    context = createContext(device());
    ASSERT_NE(nullptr, context) << "createContext failed";

    queue = createCommandQueue(context, device());
    ASSERT_NE(nullptr, queue) << "createCommandQueue failed";
  }

  void TearDown() override {
    if (queue) {
      cl_int err = clFinish(queue);
      ASSERT_EQ(CL_SUCCESS, err)
          << "clFinish failed with error " << ErrToStr(err);
    }

    OCLFPGABaseFixture::TearDown();
  }

protected:
  cl_context context = nullptr;
  cl_command_queue queue = nullptr;
};

TEST_F(TestCanUseGlobalWorkOffsetBase, AttributeNotSpecifiedParameterNull) {
  const std::string programSources = "\n\
  __kernel void k1() {}               \n\
  ";

  cl_program program = createAndBuildProgram(context, programSources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clEnqueueNDRangeKernel(queue, kernel, num_dims, nullptr, work_size,
                                 nullptr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
}

TEST_F(TestCanUseGlobalWorkOffsetBase, AttributeNotSpecifiedParameterNonNull) {
  const std::string programSources = "\n\
  __kernel void k1() {}               \n\
  ";

  cl_program program = createAndBuildProgram(context, programSources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clEnqueueNDRangeKernel(queue, kernel, num_dims, work_offset,
                                 work_size, nullptr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
}

TEST_F(TestCanUseGlobalWorkOffsetBase, AttributeFalseParameterNull) {
  const std::string programSources = "        \n\
  __attribute__((uses_global_work_offset(0))) \n\
  __kernel void k1() {}                       \n\
  ";

  cl_program program = createAndBuildProgram(context, programSources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clEnqueueNDRangeKernel(queue, kernel, num_dims, nullptr, work_size,
                                 nullptr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
}

TEST_F(TestCanUseGlobalWorkOffsetBase, AttributeFalseParameterNonNull) {
  const std::string programSources = "        \n\
  __attribute__((uses_global_work_offset(0))) \n\
  __kernel void k1() {}                       \n\
  ";

  cl_program program = createAndBuildProgram(context, programSources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clEnqueueNDRangeKernel(queue, kernel, num_dims, work_offset,
                                 work_size, nullptr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_GLOBAL_OFFSET, error)
      << "clEnqueueNDRangeKernel should fail with CL_INVALID_GLOBAL_OFFSET. "
         "Instead returns "
      << ErrToStr(error);
}

class CanUseGlobalWorkOffsetParamBase
    : public TestCanUseGlobalWorkOffsetBase,
      public ::testing::WithParamInterface<int> {
protected:
  typedef TestCanUseGlobalWorkOffsetBase parent_t;
  void SetUp() override {
    attribute_value = GetParam();
    parent_t::SetUp();
  }
  size_t attribute_value;
};

TEST_P(CanUseGlobalWorkOffsetParamBase, AttributeTrueParameterNull) {
  const std::string programSources = "\n\
  __attribute__((uses_global_work_offset(" +
                                     std::to_string(attribute_value) +
                                     ")))                                \n\
  __kernel void k1() {}               \n\
  ";

  cl_program program = createAndBuildProgram(context, programSources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clEnqueueNDRangeKernel(queue, kernel, num_dims, nullptr, work_size,
                                 nullptr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
}

TEST_P(CanUseGlobalWorkOffsetParamBase, AttributeTrueParameterNonNull) {
  const std::string programSources = "\n\
  __attribute__((uses_global_work_offset(" +
                                     std::to_string(attribute_value) +
                                     ")))                                \n\
  __kernel void k1() {}               \n\
  ";

  cl_program program = createAndBuildProgram(context, programSources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clEnqueueNDRangeKernel(queue, kernel, num_dims, work_offset,
                                 work_size, nullptr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clEnqueueNDRangeKernel failed with error " << ErrToStr(error);
}

INSTANTIATE_TEST_SUITE_P(TestCanUseGlobalWorkOffsetParam,
                         CanUseGlobalWorkOffsetParamBase,
                         ::testing::Values(1, 31));
