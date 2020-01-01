//===--- kernel_serialization.cpp -                             -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for kernel serialization
//
// ===--------------------------------------------------------------------=== //
#include "simple_fixture.h"

#include <CL/cl.h>
#include <gtest/gtest.h>

#include <numeric>
#include <vector>
#include <string>

class TestKernelSerialization : public OCLFPGASimpleFixture {};

// If test sporadically hangs - that means the serialization isn't working.
TEST_F(TestKernelSerialization, Basic) {
  const std::string program_source = "                                       \n\
    #pragma OPENCL EXTENSION cl_intel_channels : enable                      \n\
    channel int b[3];                                                        \n\
                                                                             \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void host_reader(__global int *data, int size) {                \n\
      for (int i = 0; i < size; ++i) {                                       \n\
        write_channel_intel(b[0], data[i] + 1);                              \n\
      }                                                                      \n\
    }                                                                        \n\
                                                                             \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void host_writer(__global int *data, int size) {                \n\
      for (int i = 0; i < size; ++i) {                                       \n\
        data[i] = read_channel_intel(b[0]);                                  \n\
      }                                                                      \n\
    }                                                                        \n\
    ";

  ASSERT_TRUE(createAndBuildProgram(program_source))
      << "createAndBuildProgram failed";

  cl_int size = 10;
  std::vector<cl_int> data(size);
  std::iota(data.begin(), data.end(), 0);

  cl_mem input_buffer = createBuffer<cl_int>(
      size, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &data.front());
  ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

  cl_mem temp_buffer =
      createBuffer<cl_int>(size, CL_MEM_READ_WRITE);
  ASSERT_NE(nullptr, temp_buffer) << "createBuffer failed";

  cl_mem output_buffer =
      createBuffer<cl_int>(size, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer) << "createBuffer failed";

  for (size_t i = 0; i != 2; ++i) {
    ASSERT_TRUE(
        enqueueOOOTask("host_reader", (i) ? temp_buffer : input_buffer, size))
        << "enqueueTask failed";
    ASSERT_TRUE(
        enqueueOOOTask("host_writer", (i) ? output_buffer : temp_buffer, size))
        << "enqueueTask failed";
  }

  clFinish(getOOOQueue());

  std::vector<cl_int> result(size);
  ASSERT_TRUE(readBuffer<cl_int>("host_writer", output_buffer, size,
                                 &result.front())) << "readBuffer failed";

  for (cl_int i = 0; i < size; ++i)
    ASSERT_EQ(data[i] + 2, result[i]) << " Data is differ at index " << i;
}
