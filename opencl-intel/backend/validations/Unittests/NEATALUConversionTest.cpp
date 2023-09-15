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

// \brief Tests for OpenCL vector load and store built-in functions (see
// spec. 6.11.7.) in NEATALU

#include "ALUTest.h"
#include "DGHelper.h"
#include "DataGenerator.h"
#include "NEATALU.h"
#include "NEATALUUtils.h"
#include "NEATValue.h"
#include "NEATVector.h"
#include "RefALU.h"
#include "gtest_wrapper.h" // Test framework

using namespace Validation;

template <typename T> class NEATAluTypedConversion : public ALUTest {
public:
  // Number of different random inputs to test.
  static const int NUM_TESTS = 500;
  // Vector data type length. WARNING! should be aligned with convert_float*
  // function call!
  static const int vectorWidth = 4;

  // Random values for the first argument.
  T src[NUM_TESTS * vectorWidth];

  // Parameters for random data generator.
  VectorWidth currWidth;
  DataTypeVal dataTypeVal;

  NEATAluTypedConversion() {
    currWidth = VectorWidthWrapper::ValueOf(vectorWidth);
    switch (sizeof(T)) {
    case 1:
      dataTypeVal = I8;
      break;
    case 2:
      dataTypeVal = I16;
      break;
    case 4:
      dataTypeVal = I32;
      break;
    case 8:
      dataTypeVal = I64;
      break;
    default:
      dataTypeVal = UNSPECIFIED_DATA_TYPE;
    }

    // Fill up data
    this->seed =
        GenerateRandomVectors(dataTypeVal, &src[0], currWidth, NUM_TESTS);
  }
};

typedef ::testing::Types<char, short, int, long, long long> IntTypesConversion;
TYPED_TEST_SUITE(NEATAluTypedConversion, IntTypesConversion, );

TYPED_TEST(NEATAluTypedConversion, convert_float) {
  // Scalar test
  for (int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx) {
    NEATValue result = NEATALU::convert_float<TypeParam>(&this->src[testIdx]);
    EXPECT_TRUE(TestAccValue<float>(result, float(this->src[testIdx])));
  }

  // Vector test
  for (int testIdx = 0; testIdx < this->NUM_TESTS; ++testIdx) {
    NEATVector result = NEATALU::convert_float<TypeParam, 4>(
        this->src + (this->vectorWidth) * testIdx);
    for (int i = 0; i < this->vectorWidth; ++i) {
      EXPECT_TRUE(TestAccValue<float>(
          result[i], float(this->src[this->vectorWidth * testIdx + i])));
    }
  }
}
