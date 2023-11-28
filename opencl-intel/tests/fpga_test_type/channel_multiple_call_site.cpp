//===--- channel_multiple_call_site.cpp -                       -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for channels multiple call site feature
//
// ===--------------------------------------------------------------------=== //
#include "CL/cl.h"
#include "gtest_wrapper.h"
#include "simple_fixture.h"

#include <numeric>
#include <string>
#include <vector>

class TestChannelMultipleCallSite : public OCLFPGASimpleFixture {};

TEST_F(TestChannelMultipleCallSite, Basic) {
  const std::string program_source = "                                       \n\
    #pragma OPENCL EXTENSION cl_intel_channels : enable                      \n\
    channel int a;                                                           \n\
    channel int b[3];                                                        \n\
                                                                             \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void host_reader(__global int *data, int size) {                \n\
      for (int i = 0; i < size; ++i) {                                       \n\
        write_channel_intel(a, data[i]);                                     \n\
        while (!write_channel_nb_intel(a, data[i]));                         \n\
        write_channel_intel(b[0], data[i]);                                  \n\
        write_channel_intel(b[1], data[i]);                                  \n\
        write_channel_intel(b[2], data[i]);                                  \n\
        while (!write_channel_nb_intel(b[0], data[i]));                      \n\
        while (!write_channel_nb_intel(b[1], data[i]));                      \n\
        while (!write_channel_nb_intel(b[2], data[i]));                      \n\
      }                                                                      \n\
    }                                                                        \n\
                                                                             \n\
    __attribute__((max_global_work_dim(0)))                                  \n\
    __kernel void host_writer(__global int *data, int size) {                \n\
      bool valid = false;                                                    \n\
      for (int i = 0; i < size; ++i) {                                       \n\
        data[8 * i + 0] = read_channel_intel(a);                             \n\
        do {                                                                 \n\
          data[8 * i + 1] = read_channel_nb_intel(a, &valid);                \n\
        } while (!valid);                                                    \n\
        data[8 * i + 2] = read_channel_intel(b[0]);                          \n\
        data[8 * i + 3] = read_channel_intel(b[1]);                          \n\
        data[8 * i + 4] = read_channel_intel(b[2]);                          \n\
        do {                                                                 \n\
          data[8 * i + 5] = read_channel_nb_intel(b[0], &valid);             \n\
        } while (!valid);                                                    \n\
        do {                                                                 \n\
          data[8 * i + 6] = read_channel_nb_intel(b[1], &valid);             \n\
        } while (!valid);                                                    \n\
        do {                                                                 \n\
          data[8 * i + 7] = read_channel_nb_intel(b[2], &valid);             \n\
        } while (!valid);                                                    \n\
      }                                                                      \n\
    }                                                                        \n\
    ";

  ASSERT_TRUE(createAndBuildProgram(program_source))
      << "createAndBuildProgram failed";

  cl_int size = 1000;
  std::vector<cl_int> data(size);
  std::iota(data.begin(), data.end(), 0);

  cl_mem input_buffer = createBuffer<cl_int>(
      size, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &data.front());
  ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

  cl_mem output_buffer = createBuffer<cl_int>(size * 8, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer) << "createBuffer failed";

  ASSERT_TRUE(enqueueTask("host_reader", input_buffer, size))
      << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("host_writer", output_buffer, size))
      << "enqueueTask failed";

  std::vector<cl_int> result(size * 8);
  ASSERT_TRUE(readBuffer<cl_int>("host_writer", output_buffer, size * 8,
                                 &result.front()))
      << "readBuffer failed";

  for (cl_int i = 0; i < size; ++i) {
    ASSERT_EQ(data[i], result[8 * i + 0]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 1]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 2]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 3]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 4]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 5]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 6]) << " Data is differ at index " << i;
    ASSERT_EQ(data[i], result[8 * i + 7]) << " Data is differ at index " << i;
  }
}
