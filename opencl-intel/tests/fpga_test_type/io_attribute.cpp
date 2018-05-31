//===--- io_attribute.cpp -                                -*- OpenCL C -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for I/O channels and pipes
//
// ===--------------------------------------------------------------------=== //
#include "simple_fixture.h"

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace {
  const int SIZE = 10;
}

class TestIOAttribute : public OCLFPGASimpleFixture {
protected:
  void fillInput(const std::string &filename) {
    std::vector<cl_int> arr(SIZE);
    std::iota(arr.begin(), arr.end(), 0);
    m_reference = arr;

    FILE *test_in = fopen(filename.c_str(), "wb");
    ASSERT_TRUE(test_in != nullptr) << "Cannot create input file";
    size_t write = fwrite(&arr[0], sizeof(cl_int), arr.size(), test_in);
    ASSERT_EQ(write, arr.size()) << "didn't write all elements!";
    fclose(test_in);
  }

  void validateOutput(const std::string &filename) {
    FILE *test_out = fopen(filename.c_str(), "rb");
    ASSERT_TRUE(test_out != nullptr) << "Cannot open out file for reading";

    std::vector<cl_int> data(SIZE);
    int read = fread(&data.front(), sizeof(cl_int), data.size(), test_out);
    ASSERT_GT(read, 0) << "Read 0 bytes from file";
    fclose(test_out);

    for (size_t i = 0; i < data.size(); ++i) {
      ASSERT_EQ(m_reference[i], data[i])
          << " invalid value of " << i << "-th element of array";
    }
  }

  std::vector<cl_int> m_reference;
};

TEST_F(TestIOAttribute, Pipes) {
  const std::string programSource = "                                        \n\
    __kernel void test_io(                                                   \n\
        read_only pipe int p1 __attribute__((io(\"test_pipes_in\"))),        \n\
        write_only pipe int p2 __attribute__((io(\"test_pipes_out\")))) {    \n\
      for (int i = 0; i < 10; ++i) {                                         \n\
        int data;                                                            \n\
        int ret = read_pipe(p1, &data);                                      \n\
        if (ret > 0)                                                         \n\
          write_pipe(p2, &data);                                             \n\
      }                                                                      \n\
    }                                                                        \n\
    ";

  ASSERT_NO_FATAL_FAILURE(fillInput("test_pipes_in"));
  ASSERT_TRUE(createAndBuildProgram(programSource))
      << "createAndBuildProgram failed";
  cl_mem readPipe = createPipe<cl_int>(SIZE);
  ASSERT_NE(nullptr, readPipe) << "createPipe failed";
  cl_mem writePipe = createPipe<cl_int>(SIZE);
  ASSERT_NE(nullptr, writePipe) << "createPipe failed";
  ASSERT_TRUE(enqueueTask("test_io", readPipe, writePipe))
      << "enqueueTask failed";
  finish("test_io");
  // FIXME: looks like we need to release writePipe first to get access to
  // the output file
  ASSERT_NO_FATAL_FAILURE(validateOutput("test_pipes_out"));
}

TEST_F(TestIOAttribute, Channels) {
  const std::string programSource = "                                        \n\
      #pragma OPENCL EXTENSION cl_intel_channels : enable                    \n\
                                                                             \n\
      channel int ichIn __attribute__((io(\"test_channels_in\")));           \n\
      channel int ichOut __attribute__((io(\"test_channels_out\")));         \n\
                                                                             \n\
      __kernel void test_io() {                                              \n\
        for (int i = 0; i < 10; ++i) {                                       \n\
          int data = read_channel_intel(ichIn);                              \n\
          write_channel_intel(ichOut, data);                                 \n\
        }                                                                    \n\
      }                                                                      \n\
      ";

  ASSERT_NO_FATAL_FAILURE(fillInput("test_channels_in"));
  ASSERT_TRUE(createAndBuildProgram(programSource))
      << "createAndBuildProgram failed";
  ASSERT_TRUE(enqueueTask("test_io")) << "enqueueTask failed";
  finish("test_io");
  ASSERT_NO_FATAL_FAILURE(validateOutput("test_channels_out"));
}
