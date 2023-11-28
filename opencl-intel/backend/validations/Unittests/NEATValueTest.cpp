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

#include "NEATValue.h"
#include "NEATALUUtils.h"
#include "RefALU.h"
#include "gtest_wrapper.h"
#include <fstream>
#include <iostream>

using namespace Validation;
using namespace std;

/// this test checks serialization of NEATValue structure into standard streams
/// variable of type NEATValue is created and filled with reference data
/// then it is written to output stream.
/// Next it is read from the same stream into new NEATValue variable
/// Then checking we read the same as we written before
TEST(NEATValue, SerializationCheck) {

  /// ACCURATE CHECK
  {
    stringstream ref_str(stringstream::in | stringstream::out);
    NEATValue ref;
    ref.SetAccurateVal<float>(1.0f);
    ref_str << ref;

    NEATValue val;
    ref_str >> val;
    EXPECT_EQ(val.IsAcc(), true);
    EXPECT_EQ(*val.GetAcc<float>(), 1.0f);
  }

  /// UNKNOWN CHECK
  {
    stringstream ref_str(stringstream::in | stringstream::out);
    NEATValue ref;
    ref.SetStatus(NEATValue::UNKNOWN);
    ref_str << ref;

    NEATValue val;
    ref_str >> val;
    EXPECT_EQ(val.IsUnknown(), true);
  }

  /// ANY CHECK
  {
    stringstream ref_str(stringstream::in | stringstream::out);
    NEATValue ref;
    ref.SetStatus(NEATValue::ANY);
    ref_str << ref;

    NEATValue val;
    ref_str >> val;
    EXPECT_EQ(val.IsAny(), true);
  }
  /// UNWRITTEN CHECK
  {
    stringstream ref_str(stringstream::in | stringstream::out);
    NEATValue ref;
    ref.SetStatus(NEATValue::UNWRITTEN);
    ref_str << ref;

    NEATValue val;
    ref_str >> val;
    EXPECT_EQ(val.IsUnwritten(), true);
  }

  /// INTERVAL CHECK
  {
    stringstream ref_str(stringstream::in | stringstream::out);
    NEATValue ref;
    ref.SetIntervalVal<double>(1.0, 88.34);
    ref_str << ref;

    NEATValue val;
    ref_str >> val;
    EXPECT_EQ(val.IsInterval(), true);
    EXPECT_EQ(*val.GetMin<double>(), 1.0);
    EXPECT_EQ(*val.GetMax<double>(), 88.34);
  }
}

TEST(NEATValue, ExpandIntervalTest) {
  double fmin = -32.4901;
  double fmax = -12.345;

  double fmin_out = fmin, fmax_out = fmax;
  IntervalError<float> error(3.0f);

  IntervalError<float>::ExpandFPInterval(&fmin_out, &fmax_out, error);
  NEATValue test_int((float)fmin_out, (float)fmax_out);

  bool res_int = TestIntExpanded<double>(fmin, fmax, test_int, error);
  EXPECT_TRUE(res_int);

  double facc = 13.14873;

  double facc_min = facc, facc_max = facc;

  IntervalError<float>::ExpandFPInterval(&facc_min, &facc_max, error);
  NEATValue test_acc((float)facc_min, (float)facc_max);

  bool res_acc =
      TestNeatAcc<double>(NEATValue(0.0f), test_acc, facc, error, 0, 0);
  EXPECT_TRUE(res_acc);
}
