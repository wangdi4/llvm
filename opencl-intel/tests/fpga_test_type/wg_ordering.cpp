//===--- wg_ordering.cpp -                                      -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for work-groups ordering
//
// ===--------------------------------------------------------------------=== //
#include "simple_fixture.h"

#include "gtest_wrapper.h"
#include <CL/cl.h>

#include <string>
#include <vector>

class TestWGOrdering : public OCLFPGASimpleFixture {};

TEST_F(TestWGOrdering, Channels) {
  const std::string programSources = "                                       \n\
      #pragma OPENCL EXTENSION cl_intel_channels : enable                    \n\
                                                                             \n\
      atomic_int counter = ATOMIC_VAR_INIT(0);                               \n\
      channel int4 ch;                                                       \n\
                                                                             \n\
      __kernel void channel_writer() {                                       \n\
        size_t gid = get_global_linear_id();                                 \n\
        int4 wiid;                                                           \n\
        wiid.x = get_local_id(0);                                            \n\
        wiid.y = get_local_id(1);                                            \n\
        wiid.z = get_local_id(2);                                            \n\
        write_channel_intel(ch, wiid);                                       \n\
        int4 wgid;                                                           \n\
        wgid.x = get_group_id(0);                                            \n\
        wgid.y = get_group_id(1);                                            \n\
        wgid.z = get_group_id(2);                                            \n\
        write_channel_intel(ch, wgid);                                       \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void channel_reader(__global int4 *out_buf) {                 \n\
        int ind = atomic_fetch_add(&counter, 2);                             \n\
        int4 wiid = read_channel_intel(ch);                                  \n\
        int4 wgid = read_channel_intel(ch);                                  \n\
        out_buf[ind + 0] = wiid;                                             \n\
        out_buf[ind + 1] = wgid;                                             \n\
      }                                                                      \n\
      ";

  ASSERT_TRUE(createAndBuildProgram(programSources, "-cl-std=CL2.0"))
      << "createAndBuildProgram failed";

  const size_t numDims = 3;
  size_t globalSize[numDims] = {128, 64, 32};
  size_t localSize[numDims] = {32, 16, 8};
  size_t numElements = globalSize[0] * globalSize[1] * globalSize[2] * 2;

  cl_mem buffer = createBuffer<cl_int4>(numElements, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, buffer) << "createBuffer failed";

  ASSERT_TRUE(enqueueNDRange("channel_writer", numDims, globalSize, localSize,
                             /*event*/ nullptr))
      << "enqueueNDRange failed";
  ASSERT_TRUE(enqueueNDRange("channel_reader", numDims, globalSize, localSize,
                             /*event*/ nullptr, buffer))
      << "enqueueNDRange failed";

  std::vector<cl_int4> data(numElements);
  ASSERT_TRUE(
      readBuffer<cl_int4>("channel_reader", buffer, numElements, &data.front()))
      << "readBuffer failed";

  size_t offset = 0;
  for (size_t wgz = 0; wgz < globalSize[2] / localSize[2]; ++wgz) {
    for (size_t wgy = 0; wgy < globalSize[1] / localSize[1]; ++wgy) {
      for (size_t wgx = 0; wgx < globalSize[0] / localSize[0]; ++wgx) {
        for (size_t wiz = 0; wiz < localSize[2]; ++wiz) {
          for (size_t wiy = 0; wiy < localSize[1]; ++wiy) {
            for (size_t wix = 0; wix < localSize[0]; ++wix) {
              ASSERT_EQ(wix, (size_t)data[offset + 0].s[0])
                  << " incorrect value of get_local_id(0) at offset " << offset;
              ASSERT_EQ(wiy, (size_t)data[offset + 0].s[1])
                  << " incorrect value of get_local_id(1) at offset " << offset;
              ASSERT_EQ(wiz, (size_t)data[offset + 0].s[2])
                  << " incorrect value of get_local_id(2) at offset " << offset;
              ASSERT_EQ(wgx, (size_t)data[offset + 1].s[0])
                  << " incorrect value of get_group_id(0) at offset " << offset;
              ASSERT_EQ(wgy, (size_t)data[offset + 1].s[1])
                  << " incorrect value of get_group_id(1) at offset " << offset;
              ASSERT_EQ(wgz, (size_t)data[offset + 1].s[2])
                  << " incorrect value of get_group_id(2) at offset " << offset;
              offset += 2;
            }
          }
        }
      }
    }
  }
}

TEST_F(TestWGOrdering, Pipes) {
  GTEST_SKIP();
  const std::string programSources = "                                       \n\
      atomic_int counter = ATOMIC_VAR_INIT(0);                               \n\
                                                                             \n\
      __kernel void pipe_writer(write_only pipe int4 a) {                    \n\
        size_t gid = get_global_linear_id();                                 \n\
        int4 wiid;                                                           \n\
        wiid.x = get_local_id(0);                                            \n\
        wiid.y = get_local_id(1);                                            \n\
        wiid.z = get_local_id(2);                                            \n\
        while (0 != write_pipe(a, &wiid));                                   \n\
        int4 wgid;                                                           \n\
        wgid.x = get_group_id(0);                                            \n\
        wgid.y = get_group_id(1);                                            \n\
        wgid.z = get_group_id(2);                                            \n\
        while(0 != write_pipe(a, &wgid));                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void pipe_reader(read_only pipe int4 a,                       \n\
                                __global int4 *out_buf) {                    \n\
        int ind = atomic_fetch_add(&counter, 2);                             \n\
        int4 wiid;                                                           \n\
        int4 wgid;                                                           \n\
        while (0 != read_pipe(a, &wiid));                                    \n\
        while (0 != read_pipe(a, &wgid));                                    \n\
        out_buf[ind + 0] = wiid;                                             \n\
        out_buf[ind + 1] = wgid;                                             \n\
      }                                                                      \n\
      ";

  ASSERT_TRUE(createAndBuildProgram(programSources, "-cl-std=CL2.0"))
      << "createAndBuildProgram failed";

  const size_t numDims = 3;
  size_t globalSize[numDims] = {128, 64, 32};
  size_t localSize[numDims] = {32, 16, 8};
  size_t numElements = globalSize[0] * globalSize[1] * globalSize[2] * 2;

  cl_mem buffer = createBuffer<cl_int4>(numElements, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, buffer) << "createBuffer failed";

  cl_mem pipe = createPipe<cl_int4>(numElements);
  ASSERT_NE(nullptr, pipe) << "createPipe failed";

  ASSERT_TRUE(enqueueNDRange("pipe_writer", numDims, globalSize, localSize,
                             /*event*/ nullptr, pipe))
      << "enqueueNDRange failed";
  ASSERT_TRUE(enqueueNDRange("pipe_reader", numDims, globalSize, localSize,
                             /*event*/ nullptr, pipe, buffer))
      << "enqueueNDRange failed";

  std::vector<cl_int4> data(numElements);
  ASSERT_TRUE(
      readBuffer<cl_int4>("pipe_reader", buffer, numElements, &data.front()))
      << "readBuffer failed";

  size_t offset = 0;
  for (size_t wgz = 0; wgz < globalSize[2] / localSize[2]; ++wgz) {
    for (size_t wgy = 0; wgy < globalSize[1] / localSize[1]; ++wgy) {
      for (size_t wgx = 0; wgx < globalSize[0] / localSize[0]; ++wgx) {
        for (size_t wiz = 0; wiz < localSize[2]; ++wiz) {
          for (size_t wiy = 0; wiy < localSize[1]; ++wiy) {
            for (size_t wix = 0; wix < localSize[0]; ++wix) {
              ASSERT_EQ(wix, (size_t)data[offset + 0].s[0])
                  << " incorrect value of get_local_id(0) at offset " << offset;
              ASSERT_EQ(wiy, (size_t)data[offset + 0].s[1])
                  << " incorrect value of get_local_id(1) at offset " << offset;
              ASSERT_EQ(wiz, (size_t)data[offset + 0].s[2])
                  << " incorrect value of get_local_id(2) at offset " << offset;
              ASSERT_EQ(wgx, (size_t)data[offset + 1].s[0])
                  << " incorrect value of get_group_id(0) at offset " << offset;
              ASSERT_EQ(wgy, (size_t)data[offset + 1].s[1])
                  << " incorrect value of get_group_id(1) at offset " << offset;
              ASSERT_EQ(wgz, (size_t)data[offset + 1].s[2])
                  << " incorrect value of get_group_id(2) at offset " << offset;
              offset += 2;
            }
          }
        }
      }
    }
  }
}

TEST_F(TestWGOrdering, Unordered) {
  const std::string programSources = "                                       \n\
      atomic_int counter = ATOMIC_VAR_INIT(0);                               \n\
                                                                             \n\
      __kernel void test() {                                                 \n\
        atomic_fetch_add(&counter, 1);                                       \n\
        int n;                                                               \n\
        do {                                                                 \n\
          n = atomic_load(&counter);                                         \n\
        } while (n != get_global_size(0));                                   \n\
        return;                                                              \n\
      }                                                                      \n\
      ";

  ASSERT_TRUE(createAndBuildProgram(programSources, "-cl-std=CL2.0"))
      << "createAndBuildProgram failed";

  const size_t numDims = 1;
  size_t globalSize[numDims] = {8};
  size_t localSize[numDims] = {1};

  ASSERT_TRUE(
      enqueueNDRange("test", numDims, globalSize, localSize, /*event*/ nullptr))
      << "enqueueNDRange failed";

  finish("test");
}
