//===--- device_info.cpp -                                      -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for local_mem_size attribute
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"
#include "base_fixture.h"
#include "gtest_wrapper.h"
#include <string>

class TestLocalMemSizeAttr : public OCLFPGABaseFixture {};

TEST_F(TestLocalMemSizeAttr, Basic) {
  std::string program_sources = "                                            \n\
      __kernel void k1(__local __attribute__((local_mem_size(512))) int *a,  \n\
                       __local __attribute__((local_mem_size(512))) int *b,  \n\
                       __local __attribute__((local_mem_size(512))) int *c,  \n\
                       __local int *d,                                       \n\
                       __local int *e,                                       \n\
                       __local int *f) {}                                    \n\
      ";

  cl_context context = createContext(device());
  ASSERT_NE(nullptr, context) << "createContext failed";

  cl_program program = createAndBuildProgram(context, program_sources);
  ASSERT_NE(nullptr, program) << "createAndBuildProgram failed";

  cl_kernel kernel = createKernel(program, "k1");
  ASSERT_NE(nullptr, kernel) << "createKernel failed";

  cl_int error = CL_SUCCESS;
  error = clSetKernelArg(kernel, 0, 256, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  error = clSetKernelArg(kernel, 1, 512, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  error = clSetKernelArg(kernel, 2, 1024, nullptr);
  ASSERT_EQ(CL_INVALID_ARG_SIZE, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  error = clSetKernelArg(kernel, 3, 256, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  error = clSetKernelArg(kernel, 4, 512, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);

  error = clSetKernelArg(kernel, 5, 1024, nullptr);
  ASSERT_EQ(CL_SUCCESS, error)
      << "clSetKernelArg failed with error " << ErrToStr(error);
}
