//===--- infinite_loops.cpp -                                   -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for infinite loops feature
//
// ===--------------------------------------------------------------------=== //
#include "simple_fixture.h"

#include "gtest_wrapper.h"
#include <CL/cl.h>

#include <string>

class TestInfiniteLoops : public OCLFPGASimpleFixture {};

TEST_F(TestInfiniteLoops, Basic) {
  const std::string programSources = "                                       \n\
      global atomic_int counter = ATOMIC_VAR_INIT(0);                        \n\
      __kernel void infinite_loop_test() {                                   \n\
        int val = atomic_load(&counter);                                     \n\
        while (true) {                                                       \n\
          if (val < 999) {                                                   \n\
            val = atomic_fetch_add(&counter, 1);                             \n\
          }                                                                  \n\
        }                                                                    \n\
      }                                                                      \n\
                                                                             \n\
      __kernel void waiter(__global int* value) {                            \n\
        int val = atomic_load(&counter);                                     \n\
        while (val != 1000) {                                                \n\
          val = atomic_load(&counter);                                       \n\
        }                                                                    \n\
        *value = val;                                                        \n\
      }                                                                      \n\
      ";

  ASSERT_TRUE(createAndBuildProgram(programSources, "-cl-std=CL2.0"))
      << "createAndBuildProgram failed";

  cl_mem buffer = createBuffer<cl_int>(1, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, buffer) << "createBuffer failed";

  ASSERT_TRUE(enqueueTask("infinite_loop_test")) << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("waiter", buffer)) << "enqueueTask failed";

  cl_int result = 0;
  ASSERT_TRUE(readBuffer<cl_int>("waiter", buffer, 1, &result))
      << "readBuffer failed";
  ASSERT_EQ(1000, result) << "verification failed";
}
