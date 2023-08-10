//===--- tbb_num_workers.cpp -                                  -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for OCL_TBB_NUM_WORKERS env variable
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"
#include "base_fixture.h"
#include "common_utils.h"
#include "gtest_wrapper.h"

class TBBNumWorkersBase : public OCLFPGABaseFixture,
                          public ::testing::WithParamInterface<int> {
protected:
  typedef OCLFPGABaseFixture parent_t;

  void SetUp() override {
    SETENV("OCL_TBB_NUM_WORKERS", std::to_string(GetParam()).c_str());
    parent_t::SetUp();
  }

  void TearDown() override {
    parent_t::TearDown();
    UNSETENV("OCL_TBB_NUM_WORKERS");
  }
};

TEST_P(TBBNumWorkersBase, CreateContext) {
  cl_context context = createContext(device());
  ASSERT_NE(nullptr, context) << "createContext failed";
}

INSTANTIATE_TEST_SUITE_P(TestTBBNumWorkers, TBBNumWorkersBase,
                         ::testing::Values(256, 2, 4096, 1, -42));
