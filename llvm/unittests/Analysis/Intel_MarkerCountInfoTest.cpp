#if INTEL_FEATURE_MARKERCOUNT
//===- Intel_MarkerCountInfoTest.cpp - parser for marker count ------------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_MarkerCountInfo.h"
#include "llvm/Support/CodeGen.h"
#include "gtest/gtest.h"

using namespace llvm;

TEST(MarkerCountInfoTest, StringParsing) {
  auto Str =
      R"(
	[
	{
	    "name":"f1",
	    "function":"never",
	    "loop":"never",
	    "comment":"indeterministic times of function call"
	},
	{
	    "name":"f2",
	    "loop":"never",
	    "comment":"hash-related loop"
	},
	{
	    "name":"f3",
	    "function":"never",
	    "comment":"function marker count affect optimization a lot"
	},
	{
	    "name":"f4",
	    "function":"me",
	    "loop":"me",
	    "comment":"different loop unroll and inline decision due to new ISA"
	},
	{
	    "name":"f5",
	    "function":"me",
	    "comment":"different inline decision due to new ISA"
	},
	{
	    "name":"f6",
	    "loop":"me",
	    "comment":"different loop unroll due to new ISA"
	},
	{
	    "name":"f7",
	    "function":"be",
	    "loop":"be",
	    "comment":"new ISA does not affect marker in this function"
	},
	{
	    "name":"f8",
	    "function":"be",
	    "comment":"new ISA does not affect function marker in this function"
	},
	{
	    "name":"f9",
	    "loop":"be",
	    "comment":"new ISA does not affect loop marker in this function"
	}
	]
      )";

  std::map<std::string, unsigned> Map;
  // Default: no marker
  unsigned MarkerCountKind =
      MarkerCount::Function_Never | MarkerCount::Loop_Never;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_Never | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_Never | MarkerCount::Loop_BE);
  // Default: mid-end function marker + mid-end loop marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_ME | MarkerCount::Loop_ME;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"), MarkerCount::Function_ME | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"), MarkerCount::Function_Never | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_ME | MarkerCount::Loop_BE);
  // Default: back-end function marker + back-end loop marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_BE | MarkerCount::Loop_BE;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"), MarkerCount::Function_BE | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"), MarkerCount::Function_Never | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_BE | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  // Default: mid-end function marker + back-end loop marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_ME | MarkerCount::Loop_BE;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"), MarkerCount::Function_ME | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"), MarkerCount::Function_Never | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_ME | MarkerCount::Loop_BE);
  // Default: back-end function marker + mid-end loop marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_BE | MarkerCount::Loop_ME;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"), MarkerCount::Function_BE | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"), MarkerCount::Function_Never | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_BE | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  // Default: mid-end function marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_ME | MarkerCount::Loop_Never;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"), MarkerCount::Function_ME | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_ME | MarkerCount::Loop_BE);
  // Default: back-end function marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_BE | MarkerCount::Loop_Never;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"), MarkerCount::Function_BE | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_BE | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  // Default: mid-end loop marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_Never | MarkerCount::Loop_ME;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"), MarkerCount::Function_Never | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_Never | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_Never | MarkerCount::Loop_BE);
  // Default: back-end loop marker
  Map.clear();
  MarkerCountKind = MarkerCount::Function_Never | MarkerCount::Loop_BE;
  MarkerCount::parseMarkerCountString(Map, MarkerCountKind, Str);
  EXPECT_EQ(Map.size(), 9U);
  EXPECT_EQ(Map.at("f1"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f2"),
            MarkerCount::Function_Never | MarkerCount::Loop_Never);
  EXPECT_EQ(Map.at("f3"), MarkerCount::Function_Never | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f4"), MarkerCount::Function_ME | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f5"), MarkerCount::Function_ME | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f6"), MarkerCount::Function_Never | MarkerCount::Loop_ME);
  EXPECT_EQ(Map.at("f7"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f8"), MarkerCount::Function_BE | MarkerCount::Loop_BE);
  EXPECT_EQ(Map.at("f9"), MarkerCount::Function_Never | MarkerCount::Loop_BE);
}
#endif // INTEL_FEATURE_MARKERCOUNT
