// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __ALUTEST_H__
#define __ALUTEST_H__

/// Common class for ALU tests. Each NEAT ALU test can use it as a base class
/// It prints seed if the test fails

#include "DGHelper.h"
#include "gtest_wrapper.h" // Test framework

// seed taken from command line
extern uint64_t seedForValidation;

class ALUTest : public ::testing::Test {
public:
  uint64_t seed;

  ALUTest() {
    // take external seed from TestMain.cpp, if 0, produce seed inside generator
    seed = Validation::SetSeed(seedForValidation);
  }

  ~ALUTest() {
    // if some test fails, print seed
    if (HasFailure()) {
      std::cout << "Seed: " << seed << std::endl;
    }
  }
};
#endif // #ifndef __ALUTEST_H__
