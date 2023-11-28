// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "Buffer.h"
#include "BufferContainer.h"
#include "Comparator.h"
#include "ComparisonResults.h"
#include "IComparisonResults.h"
#include "dxfloat.h"
#include "gtest_wrapper.h"
#include <iostream>
#include <limits>
#include <list>
#include <typeinfo>

using namespace Validation;
using namespace Validation::Utils;

#ifdef _WIN32
#pragma warning(disable : 4723)
#endif

TEST(Common, FloatToFloat16) {
  CFloat16 posInf(std::numeric_limits<float>::infinity());
  EXPECT_TRUE(posInf.IsPInf());
  CFloat16 negInf(-std::numeric_limits<float>::infinity());
  EXPECT_TRUE(negInf.IsNInf());
  CFloat16 x1(0.3f);
  CFloat16 x2(0.5f);
  CFloat16 x3(0.3f);
  EXPECT_FALSE(x1 == x2);
  EXPECT_TRUE(x1 != x2);
  EXPECT_TRUE(x1 == x3);
  EXPECT_FALSE(x1 != x3);
  float zero1 = 0.0f;
  float zero2 = 3.0 - 3.0; /// Use this expression to prevent "potential divide
                           /// by 0" warning from the compiler
  float NaN1 = zero1 / zero2;
  float NaN2 = std::numeric_limits<float>::quiet_NaN();
  float NaN3 = std::numeric_limits<float>::signaling_NaN();
  CFloat16 NaN16_1(NaN1);
  CFloat16 NaN16_2(NaN2);
  CFloat16 NaN16_3(NaN3);
  EXPECT_TRUE(NaN16_1.IsNaN());
  EXPECT_TRUE(NaN16_2.IsNaN());
  EXPECT_TRUE(NaN16_3.IsNaN());
  EXPECT_FALSE(x1.IsNaN());
  EXPECT_FALSE(x2.IsNaN());
  EXPECT_FALSE(posInf.IsNaN());
  EXPECT_FALSE(negInf.IsNaN());

  /// conversion tests
  float fNaN1 = (float)NaN16_1;
  float fNaN2 = (float)NaN16_2;
  float fNaN3 = (float)NaN16_3;
  EXPECT_TRUE(IsNaN(fNaN1));
  EXPECT_TRUE(IsNaN(fNaN2));
  EXPECT_TRUE(IsNaN(fNaN3));
  float fx1 = (float)x1;
  float fx2 = (float)x2;
  float fx3 = (float)x3;
  EXPECT_FALSE(IsNaN(fx1));
  EXPECT_FALSE(IsNaN(fx2));
  EXPECT_FALSE(IsNaN(fx3));
  float fPosInf = (float)posInf;
  float fNegInf = (float)negInf;
  EXPECT_FALSE(IsNaN(fPosInf));
  EXPECT_FALSE(IsNaN(fNegInf));
  EXPECT_TRUE(IsPInf(fPosInf));
  EXPECT_FALSE(IsPInf(fNegInf));
  EXPECT_TRUE(IsNInf(fNegInf));
  EXPECT_FALSE(IsNInf(fPosInf));
  EXPECT_TRUE(IsNInf(CFloat16::GetNInf()));
  EXPECT_FALSE(IsPInf(CFloat16::GetNInf()));
  EXPECT_TRUE(IsPInf(CFloat16::GetPInf()));
  EXPECT_FALSE(IsNInf(CFloat16::GetPInf()));
  CFloat16 maxF16 = CFloat16::GetMax();
  CFloat16 minF16 = CFloat16::GetMin();
  EXPECT_FALSE(IsNInf(maxF16));
  EXPECT_FALSE(IsNInf(minF16));
  EXPECT_FALSE(IsPInf(maxF16));
  EXPECT_FALSE(IsPInf(minF16));
  EXPECT_FALSE(IsNaN(maxF16));
  EXPECT_FALSE(IsNaN(minF16));
  /// TODO: test denormals!!
}
