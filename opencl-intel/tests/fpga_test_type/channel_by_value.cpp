//===--- channel_by_value.cpp -                                 -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for Channel-by-value feature
//
// ===--------------------------------------------------------------------=== //
#include "CL/cl.h"
#include "gtest_wrapper.h"
#include "simple_fixture.h"
#include <string>

class TestChannelByValue : public OCLFPGASimpleFixture {};

TEST_F(TestChannelByValue, Simple) {
  std::string programSources = "                                             \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int a __attribute__((depth(512)));                             \n\
      channel int b __attribute__((depth(512)));                             \n\
      channel int c[2] __attribute__((depth(1024)));                         \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void sendOne(channel int ch) {                                         \n\
        write_channel_intel(ch, 1);                                          \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void sendTwo(channel int ch) {                                         \n\
        write_channel_intel(ch, 2);                                          \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      int getFlag(channel int a, channel int b) {                            \n\
        int i = read_channel_intel(a);                                       \n\
        int j = read_channel_intel(b);                                       \n\
        return i + j;                                                        \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void send(channel int a, channel int b, int flag) {                    \n\
        switch (flag) {                                                      \n\
          case 0: sendOne(a); break;                                         \n\
          case 1: sendTwo(a); break;                                         \n\
          case 2: sendOne(b); break;                                         \n\
          case 3: sendTwo(b); break;                                         \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void producer(int size) {                                     \n\
        for (int i = 0; i < size; ++i) {                                     \n\
          int flag = getFlag(c[0], c[1]);                                    \n\
          send(a, b, flag);                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void host_reader(__global int2* data, int size) {             \n\
        for (int i = 0; i < size; ++i) {                                     \n\
          write_channel_intel(c[0], data[i].x);                              \n\
          write_channel_intel(c[1], data[i].y);                              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void host_writer(__global int* data1, __global int* data2,    \n\
          int size) {                                                        \n\
        for (int i = 0; i < size; ++i) {                                     \n\
          data1[i] = read_channel_intel(a);                                  \n\
          data2[i] = read_channel_intel(b);                                  \n\
        }                                                                    \n\
      }                                                                      \n\
    ";

  ASSERT_TRUE(createAndBuildProgram(programSources))
      << "createAndBuildProgram failed";

  const cl_int numElements = 1024;
  cl_int2 input_data[numElements];
  for (cl_int i = 0, x = 0; i < numElements; ++i) {
    int a = 0, b = 0;
    switch (x) {
    case 0:
      a = 0;
      b = 0;
      break;
    case 1:
      a = 1;
      b = 0;
      break;
    case 2:
      a = 1;
      b = 1;
      break;
    case 3:
      a = 1;
      b = 2;
      break;
    }

    input_data[i].s[0] = a;
    input_data[i].s[1] = b;

    ++x;
    if (x == 4) {
      x = 0;
    }
  }

  cl_mem input_buffer = createBuffer<cl_int2>(
      numElements, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, input_data);
  ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

  const cl_int halfNumElements = numElements / 2;
  cl_mem output_buffer1 =
      createBuffer<cl_int2>(halfNumElements, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer1) << "createBuffer failed";
  cl_mem output_buffer2 =
      createBuffer<cl_int2>(halfNumElements, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer2) << "createBuffer failed";

  ASSERT_TRUE(enqueueTask("host_reader", input_buffer, numElements))
      << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("host_writer", output_buffer1, output_buffer2,
                          halfNumElements))
      << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("producer", numElements)) << "enqueueTask failed";

  cl_int output_data1[halfNumElements];
  ASSERT_TRUE(readBuffer<cl_int>("host_writer", output_buffer1, halfNumElements,
                                 output_data1))
      << "readBuffer failed";
  cl_int output_data2[halfNumElements];
  ASSERT_TRUE(readBuffer<cl_int>("host_writer", output_buffer2, halfNumElements,
                                 output_data2))
      << "readBuffer failed";

  for (cl_int i = 0; i < halfNumElements; ++i) {
    cl_int reference = i % 2 == 0 ? 1 : 2;
    ASSERT_EQ(reference, output_data1[i])
        << "invalid value of " << i << "-th element of data1 array";
  }

  for (cl_int i = 0; i < halfNumElements; ++i) {
    cl_int reference = i % 2 == 0 ? 1 : 2;
    ASSERT_EQ(reference, output_data2[i])
        << "invalid value of " << i << "-th element of data2 array";
  }
}

TEST_F(TestChannelByValue, O0Regression) {
  std::string programSources = "                                             \n\
      #pragma OPENCL EXTENSION cl_intel_channels: enable                     \n\
      channel int a __attribute__((depth(512)));                             \n\
      channel int b __attribute__((depth(512)));                             \n\
      channel int c[2] __attribute__((depth(1024)));                         \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void sendOne(channel int ch) {                                         \n\
        write_channel_intel(ch, 1);                                          \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void sendTwo(channel int ch) {                                         \n\
        write_channel_intel(ch, 2);                                          \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void sendOneHelper(channel int ch) {                                   \n\
        sendOne(ch);                                                         \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void sendTwoHelper(channel int ch) {                                   \n\
        sendTwo(ch);                                                         \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      int getFlag(channel int a, channel int b) {                            \n\
        int i = read_channel_intel(a);                                       \n\
        int j = read_channel_intel(b);                                       \n\
        return i + j;                                                        \n\
      }                                                                      \n\
                                                                             \n\
      __attribute__((noinline))                                              \n\
      void send(channel int a, channel int b, int flag) {                    \n\
        switch (flag) {                                                      \n\
          case 0: sendOneHelper(a); break;                                   \n\
          case 1: sendTwoHelper(a); break;                                   \n\
          case 2: sendOneHelper(b); break;                                   \n\
          case 3: sendTwoHelper(b); break;                                   \n\
          // just to trigger segfault in ChannelPipeTransformation           \n\
          case 4: sendOneHelper(a); break;                                   \n\
          case 5: sendTwoHelper(a); break;                                   \n\
          case 6: sendOneHelper(b); break;                                   \n\
          case 7: sendTwoHelper(b); break;                                   \n\
          case 8: sendOneHelper(a); break;                                   \n\
          case 9: sendTwoHelper(a); break;                                   \n\
          case 10: sendOneHelper(b); break;                                  \n\
          case 11: sendTwoHelper(b); break;                                  \n\
          case 12: sendOneHelper(a); break;                                  \n\
          case 13: sendTwoHelper(a); break;                                  \n\
          case 14: sendOneHelper(b); break;                                  \n\
          case 15: sendTwoHelper(b); break;                                  \n\
          case 16: sendOneHelper(a); break;                                  \n\
          case 17: sendTwoHelper(a); break;                                  \n\
          case 18: sendOneHelper(b); break;                                  \n\
          case 19: sendTwoHelper(b); break;                                  \n\
          case 20: sendOneHelper(a); break;                                  \n\
          case 21: sendTwoHelper(a); break;                                  \n\
          case 22: sendOneHelper(b); break;                                  \n\
          case 23: sendTwoHelper(b); break;                                  \n\
          case 24: sendTwoHelper(b); break;                                  \n\
          case 25: sendTwoHelper(b); break;                                  \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void producer(int size) {                                     \n\
        for (int i = 0; i < size; ++i) {                                     \n\
          int flag = getFlag(c[0], c[1]);                                    \n\
          send(a, b, flag);                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void host_reader(__global int2* data, int size) {             \n\
        for (int i = 0; i < size; ++i) {                                     \n\
          write_channel_intel(c[0], data[i].x);                              \n\
          write_channel_intel(c[1], data[i].y);                              \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void host_writer(__global int* data1, __global int* data2,    \n\
          int size) {                                                        \n\
        for (int i = 0; i < size; ++i) {                                     \n\
          data1[i] = read_channel_intel(a);                                  \n\
          data2[i] = read_channel_intel(b);                                  \n\
        }                                                                    \n\
      }                                                                      \n\
    ";

  ASSERT_TRUE(createAndBuildProgram(programSources, "-cl-opt-disable"))
      << "createAndBuildProgram failed";

  const cl_int numElements = 1024;
  cl_int2 input_data[numElements];
  for (cl_int i = 0, x = 0; i < numElements; ++i) {
    int a = 0, b = 0;
    switch (x) {
    case 0:
      a = 0;
      b = 0;
      break;
    case 1:
      a = 1;
      b = 0;
      break;
    case 2:
      a = 1;
      b = 1;
      break;
    case 3:
      a = 1;
      b = 2;
      break;
    }

    input_data[i].s[0] = a;
    input_data[i].s[1] = b;

    ++x;
    if (x == 4) {
      x = 0;
    }
  }

  cl_mem input_buffer = createBuffer<cl_int2>(
      numElements, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, input_data);
  ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

  const cl_int halfNumElements = numElements / 2;
  cl_mem output_buffer1 =
      createBuffer<cl_int2>(halfNumElements, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer1) << "createBuffer failed";
  cl_mem output_buffer2 =
      createBuffer<cl_int2>(halfNumElements, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer2) << "createBuffer failed";

  ASSERT_TRUE(enqueueTask("host_reader", input_buffer, numElements))
      << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("host_writer", output_buffer1, output_buffer2,
                          halfNumElements))
      << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("producer", numElements)) << "enqueueTask failed";

  cl_int output_data1[halfNumElements];
  ASSERT_TRUE(readBuffer<cl_int>("host_writer", output_buffer1, halfNumElements,
                                 output_data1))
      << "readBuffer failed";
  cl_int output_data2[halfNumElements];
  ASSERT_TRUE(readBuffer<cl_int>("host_writer", output_buffer2, halfNumElements,
                                 output_data2))
      << "readBuffer failed";

  for (cl_int i = 0; i < halfNumElements; ++i) {
    cl_int reference = i % 2 == 0 ? 1 : 2;
    ASSERT_EQ(reference, output_data1[i])
        << "invalid value of " << i << "-th element of data1 array";
  }

  for (cl_int i = 0; i < halfNumElements; ++i) {
    cl_int reference = i % 2 == 0 ? 1 : 2;
    ASSERT_EQ(reference, output_data2[i])
        << "invalid value of " << i << "-th element of data2 array";
  }
}
