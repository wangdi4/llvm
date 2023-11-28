//===--- test_pipes.cpp -                                       -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for pipes on FPGA
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"
#include "gtest_wrapper.h"
#include "simple_fixture.h"
#include <string>

class TestPipes : public OCLFPGASimpleFixture {};

TEST_F(TestPipes, Simple) {
  std::string programSources = "                                             \n\
      __kernel void write(write_only pipe int p, int ptr) {                  \n\
        write_pipe(p, &ptr);                                                 \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void read(read_only pipe int p, global int *ptr) {            \n\
        read_pipe(p, ptr);                                                   \n\
      }                                                                      \n\
    ";

  ASSERT_TRUE(createAndBuildProgram(programSources))
      << "createAndBuildProgram failed";

  cl_mem pipe = createPipe<cl_int>(1);
  ASSERT_NE(nullptr, pipe) << "createPipe failed";

  cl_mem outBuf = createBuffer<cl_int>(1, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, outBuf) << "createBuffer failed";

  const cl_int val = 10;

  // Write pipe
  ASSERT_TRUE(enqueueTask("write", pipe, val)) << "enqueueTask failed";
  finish("write");

  // Read pipe
  ASSERT_TRUE(enqueueTask("read", pipe, outBuf)) << "enqueueTask failed";

  cl_int out = 0;
  ASSERT_TRUE(readBuffer<cl_int>("read", outBuf, 1, &out))
      << "readBuffer failed";

  ASSERT_EQ(val, out) << "Verification failed";
}
