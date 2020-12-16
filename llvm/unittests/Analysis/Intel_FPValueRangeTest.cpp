//===- Intel_FPValueRangeTest.cpp - FPValueRange tests ----------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_FPValueRange.h"

#include "gtest/gtest.h"

using namespace llvm;

namespace {

class FPValueRangeTest : public testing::Test {
protected:
  LLVMContext Context;
  const fltSemantics &DoubleSemantics =
      Type::getDoubleTy(Context)->getFltSemantics();
};

TEST_F(FPValueRangeTest, ApplyFastMathFlags) {
  FastMathFlags NoFMF;
  FastMathFlags NoNaNs;
  NoNaNs.setNoNaNs(true);
  FastMathFlags NoInfs;
  NoInfs.setNoInfs(true);
  FastMathFlags NoNaNAndInfs;
  NoNaNAndInfs.setNoNaNs(true);
  NoNaNAndInfs.setNoInfs(true);

  FPValueRange Empty = FPValueRange::createEmpty(DoubleSemantics);
  FPValueRange InfinityValue =
      FPValueRange::createEmptyOrSpecialConstant(false, true, DoubleSemantics);
  FPValueRange NaNValue =
      FPValueRange::createEmptyOrSpecialConstant(true, false, DoubleSemantics);
  FPValueRange InfinityAndNaNValue =
      FPValueRange::createEmptyOrSpecialConstant(true, true, DoubleSemantics);
  FPValueRange Unknown =
      FPValueRange::createUnknown(false, false, DoubleSemantics);
  FPValueRange PosConstant =
      FPValueRange::createConstant(APFloat(3.6), false, false);
  FPValueRange NegConstant =
      FPValueRange::createConstant(APFloat(-3.6), false, false);
  FPValueRange ZeroConstant =
      FPValueRange::createConstant(APFloat(0.0), false, false);
  FPValueRange ConstantRange1 = FPValueRange::createConstantOrConstantRange(
      APFloat(-2.2), APFloat(1.7), false, false);
  FPValueRange ConstantRange2 = FPValueRange::createConstantOrConstantRange(
      APFloat(3.3), APFloat(7.1), false, false);
  FPValueRange InfinityRange1 = FPValueRange::createConstantOrConstantRange(
    APFloat(1.2), APFloat::getInf(DoubleSemantics), false, false);

  std::vector<FPValueRange> OriginalRanges{
      Empty,          InfinityValue,  NaNValue,      InfinityAndNaNValue,
      Unknown,        PosConstant,    NegConstant,   ZeroConstant,
      ConstantRange1, ConstantRange2, InfinityRange1};
  std::vector<FPValueRange> DerivedRanges;
  for (FPValueRange Range : OriginalRanges) {
    DerivedRanges.push_back(Range);
    DerivedRanges.push_back(Range.setMaybeInfinity(true));
    DerivedRanges.push_back(Range.setMaybeNaN(true));
    DerivedRanges.push_back(Range.setMaybeInfinity(true).setMaybeNaN(true));
  }

  for (FPValueRange Range : DerivedRanges) {
    FPValueRange RangeWithNoFMF = Range.applyFastMathFlags(NoFMF);
    EXPECT_TRUE(RangeWithNoFMF.contains(Range));
    if (!Range.isEmpty())
      EXPECT_FALSE(RangeWithNoFMF.isEmpty());

    FPValueRange RangeWithNoNaNs = Range.applyFastMathFlags(NoNaNs);
    if (!Range.getMaybeNaN())
      EXPECT_TRUE(RangeWithNoNaNs.contains(Range));
    else
      EXPECT_TRUE(RangeWithNoNaNs.contains(Range.setMaybeNaN(false)));
    if (!Range.isEmpty())
      EXPECT_FALSE(RangeWithNoNaNs.isEmpty());

    FPValueRange RangeWithNoInfs = Range.applyFastMathFlags(NoInfs);
    if (!Range.getMaybeInfinity())
      EXPECT_TRUE(RangeWithNoInfs.contains(Range));
    else
      EXPECT_TRUE(RangeWithNoInfs.contains(Range.setMaybeInfinity(false)));
    if (!Range.isEmpty())
      EXPECT_FALSE(RangeWithNoInfs.isEmpty());

    FPValueRange RangeWithNoNaNAndInfs = Range.applyFastMathFlags(NoNaNAndInfs);
    if (!Range.getMaybeInfinity() && !Range.getMaybeNaN())
      EXPECT_TRUE(RangeWithNoNaNAndInfs.contains(Range));
    else
      EXPECT_TRUE(RangeWithNoNaNAndInfs.contains(
          Range.setMaybeNaN(false).setMaybeInfinity(false)));
    if (!Range.isEmpty())
      EXPECT_FALSE(RangeWithNoNaNAndInfs.isEmpty());
  }
}

TEST_F(FPValueRangeTest, Contains) {
  FPValueRange Empty = FPValueRange::createEmpty(DoubleSemantics);
  FPValueRange Unknown =
      FPValueRange::createUnknown(false, false, DoubleSemantics);
  FPValueRange PosConstant =
      FPValueRange::createConstant(APFloat(3.6), false, false);
  FPValueRange NegConstant =
      FPValueRange::createConstant(APFloat(-3.6), false, false);
  FPValueRange ZeroConstant =
      FPValueRange::createConstant(APFloat(0.0), false, false);

  FPValueRange ConstantRange1 = FPValueRange::createConstantOrConstantRange(
      APFloat(-2.2), APFloat(1.7), false, false);
  FPValueRange ConstantRange2 = FPValueRange::createConstantOrConstantRange(
      APFloat(3.3), APFloat(7.1), false, false);
  FPValueRange ConstantRange3 = FPValueRange::createConstantOrConstantRange(
      APFloat(1.2), APFloat(7.1), false, false);

  std::vector<FPValueRange> AllRanges{
      Empty,        Unknown,        PosConstant,   NegConstant,
      ZeroConstant, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : AllRanges) {
    EXPECT_TRUE(Unknown.contains(Range));
    if (!Range.isUnknown())
      EXPECT_FALSE(Range.contains(Unknown));

    EXPECT_TRUE(Range.contains(Empty));
    if (!Range.isEmpty())
      EXPECT_FALSE(Empty.contains(Range));
  }

  EXPECT_TRUE(ConstantRange1.contains(ZeroConstant));
  EXPECT_TRUE(ConstantRange2.contains(PosConstant));
  EXPECT_TRUE(ConstantRange3.contains(ConstantRange2));
  EXPECT_FALSE(ConstantRange1.contains(ConstantRange2));
  EXPECT_FALSE(ConstantRange2.contains(ZeroConstant));
  EXPECT_FALSE(PosConstant.contains(NegConstant));
}

void createTestRanges(std::vector<std::vector<FPValueRange>> &Ranges,
                      const fltSemantics &Semantics) {
  Ranges.push_back(
      std::vector<FPValueRange>{FPValueRange::createEmpty(Semantics)});

  std::vector<FPValueRange> UnknownRanges;
  for (bool HasNaN : {false, true})
    for (bool HasInfinity : {false, true})
      UnknownRanges.push_back(
          FPValueRange::createUnknown(HasNaN, HasInfinity, Semantics));
  Ranges.push_back(UnknownRanges);

  std::vector<FPValueRange> UndefRanges;
  for (bool HasNaN : {false, true})
    for (bool HasInfinity : {false, true})
      UndefRanges.push_back(
          FPValueRange::createUndef(HasNaN, HasInfinity, Semantics));
  Ranges.push_back(UndefRanges);

  std::vector<FPValueRange> ConstantRanges;
  for (bool HasNaN : {false, true})
    for (bool HasInfinity : {false, true})
      ConstantRanges.push_back(
          FPValueRange::createConstant(APFloat(3.6), HasNaN, HasInfinity));
  Ranges.push_back(ConstantRanges);

  std::vector<FPValueRange> ConstantRangeRanges;
  for (bool HasNaN : {false, true})
    for (bool HasInfinity : {false, true})
      ConstantRangeRanges.push_back(FPValueRange::createConstantOrConstantRange(
          APFloat(-2.2), APFloat(1.7), HasNaN, HasInfinity));
  Ranges.push_back(ConstantRangeRanges);
}

TEST_F(FPValueRangeTest, Equal) {
  std::vector<std::vector<FPValueRange>> Ranges;
  createTestRanges(Ranges, DoubleSemantics);

  // When merging two ranges, the result range must contain both sources
  for (unsigned I = 0; I < Ranges.size(); I++)
    for (unsigned J = I; J < Ranges.size(); J++)
      for (unsigned K = 0; K < Ranges[I].size(); K++)
        for (unsigned L = 0; L < Ranges[J].size(); L++) {
          FPValueRange LHS = Ranges[I][K], RHS = Ranges[J][L];
          bool Equal = (I == J) && (K == L);
          EXPECT_EQ(LHS == RHS, Equal);
          EXPECT_EQ(LHS != RHS, !Equal);
        }
}

TEST_F(FPValueRangeTest, Merge) {
  std::vector<std::vector<FPValueRange>> Ranges;
  createTestRanges(Ranges, DoubleSemantics);

  // When merging two ranges, the result range must contain both sources
  for (unsigned I = 0; I < Ranges.size(); I++)
    for (unsigned J = I; J < Ranges.size(); J++)
      for (unsigned K = 0; K < Ranges[I].size(); K++)
        for (unsigned L = 0; L < Ranges[J].size(); L++) {
          FPValueRange LHS = Ranges[I][K], RHS = Ranges[J][L];
          FPValueRange Merged = FPValueRange::merge(LHS, RHS);
          EXPECT_TRUE(Merged.contains(LHS));
          EXPECT_TRUE(Merged.contains(RHS));

          FPValueRange MergedOtherWay = FPValueRange::merge(RHS, LHS);
          EXPECT_TRUE(MergedOtherWay.contains(LHS));
          EXPECT_TRUE(MergedOtherWay.contains(RHS));

          FPValueRange MergeSelf = FPValueRange::merge(LHS, LHS);
          EXPECT_TRUE((MergeSelf == LHS) && !(MergeSelf != LHS));

          if (!((I == J) && (K == L)) && !LHS.isUndef() && !RHS.isUndef()) {
            if (!LHS.contains(RHS)) {
              EXPECT_NE(LHS, Merged);
              EXPECT_NE(LHS, MergedOtherWay);
            }
            if (!RHS.contains(LHS)) {
              EXPECT_NE(RHS, Merged);
              EXPECT_NE(RHS, MergedOtherWay);
            }
          }
        }
}

TEST_F(FPValueRangeTest, Multiply) {
  FPValueRange PosConstant =
      FPValueRange::createConstant(APFloat(3.6), false, false);
  FPValueRange NegConstant =
      FPValueRange::createConstant(APFloat(-3.6), false, false);
  FPValueRange ZeroConstant =
      FPValueRange::createConstant(APFloat(0.0), false, false);
  FPValueRange BigConstant =
      FPValueRange::createConstant(APFloat(5.8753e170), false, false);
  FPValueRange InfinityValue =
      FPValueRange::createEmptyOrSpecialConstant(false, true, DoubleSemantics);
  FPValueRange NaNValue =
      FPValueRange::createEmptyOrSpecialConstant(true, false, DoubleSemantics);

  FPValueRange ConstantRange1 = FPValueRange::createConstantOrConstantRange(
      APFloat(-2.2), APFloat(1.7), false, false);
  FPValueRange ConstantRange2 = FPValueRange::createConstantOrConstantRange(
      APFloat(3.8), APFloat(7.1), false, false);

  std::vector<FPValueRange> InfinityTestRanges{
      PosConstant, NegConstant, InfinityValue, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : InfinityTestRanges)
    EXPECT_TRUE(
        FPValueRange::multiply(InfinityValue, Range).getMaybeInfinity());
  EXPECT_TRUE(
      FPValueRange::multiply(InfinityValue, ZeroConstant).getMaybeNaN());

  std::vector<FPValueRange> NaNTestRanges = InfinityTestRanges;
  NaNTestRanges.push_back(NaNValue);
  for (const FPValueRange &Range : NaNTestRanges)
    EXPECT_TRUE(
        FPValueRange::multiply(NaNValue, Range).getMaybeNaN());

  EXPECT_TRUE(
      FPValueRange::multiply(BigConstant, BigConstant).contains(InfinityValue));

  FPValueRange BigMultBigRange = FPValueRange::multiply(
      BigConstant, FPValueRange::createConstantOrConstantRange(
                       APFloat(0.0), APFloat(5.8753e170), false, false));
  EXPECT_TRUE(
      BigMultBigRange.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(0.0), APFloat::getInf(DoubleSemantics), false, true)));

  FPValueRange PosMultRange1 =
      FPValueRange::multiply(PosConstant, ConstantRange1);
  EXPECT_TRUE(
      PosMultRange1.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(3.6 * -2.2), APFloat(3.6 * 1.7), false, false)));

  FPValueRange PosMultRange2 =
      FPValueRange::multiply(PosConstant, ConstantRange2);
  EXPECT_TRUE(
      PosMultRange2.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(3.6 * 3.8), APFloat(3.6 * 7.1), false, false)));

  FPValueRange NegMultRange1 =
      FPValueRange::multiply(NegConstant, ConstantRange1);
  EXPECT_TRUE(
      NegMultRange1.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(-3.6 * 1.7), APFloat(-3.6 * -2.2), false, false)));

  FPValueRange NegMultRange2 =
      FPValueRange::multiply(NegConstant, ConstantRange2);
  EXPECT_TRUE(
      NegMultRange2.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(-3.6 * 7.1), APFloat(-3.6 * 3.8), false, false)));

  FPValueRange ZeroMultRange =
      FPValueRange::multiply(ZeroConstant, ConstantRange1);
  EXPECT_TRUE(ZeroMultRange.contains(
      FPValueRange::createConstant(APFloat(0.0), false, false)));

  FPValueRange Range1MultRange2 =
      FPValueRange::multiply(ConstantRange1, ConstantRange2);
  EXPECT_TRUE(
      Range1MultRange2.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(-2.2 * 3.8), APFloat(7.1 * 1.7), false, false)));

  FPValueRange PosMultNeg =
      FPValueRange::multiply(PosConstant, NegConstant);
  EXPECT_TRUE(PosMultNeg.contains(
      FPValueRange::createConstant(APFloat(3.6 * -3.6), false, false)));
  FPValueRange PosMultZero =
      FPValueRange::multiply(PosConstant, ZeroConstant);
  EXPECT_TRUE(PosMultZero.contains(
      FPValueRange::createConstant(APFloat(0.0), false, false)));
}

TEST_F(FPValueRangeTest, Mod) {
  FPValueRange Unknown =
      FPValueRange::createUnknown(false, false, DoubleSemantics);
  FPValueRange PosConstant =
      FPValueRange::createConstant(APFloat(3.6), false, false);
  FPValueRange ZeroConstant =
      FPValueRange::createConstant(APFloat(0.0), false, false);
  FPValueRange InfinityValue =
      FPValueRange::createEmptyOrSpecialConstant(false, true, DoubleSemantics);
  FPValueRange NaNValue =
      FPValueRange::createEmptyOrSpecialConstant(true, false, DoubleSemantics);

  FPValueRange ConstantRange1 = FPValueRange::createConstantOrConstantRange(
      APFloat(-2.2), APFloat(1.7), false, false);
  FPValueRange ConstantRange2 = FPValueRange::createConstantOrConstantRange(
      APFloat(3.8), APFloat(7.1), false, false);

  std::vector<FPValueRange> ZeroDividendTestRanges{
      Unknown, PosConstant, InfinityValue, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : ZeroDividendTestRanges)
    EXPECT_TRUE(FPValueRange::mod(ZeroConstant, Range)
                    .contains(FPValueRange::createConstant(APFloat(0.0), false,
                                                           false)));

  std::vector<FPValueRange> ZeroDivisorTestRanges{
      Unknown, PosConstant, InfinityValue, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : ZeroDivisorTestRanges)
    EXPECT_TRUE(FPValueRange::mod(Range, ZeroConstant).getMaybeNaN());

  std::vector<FPValueRange> InfiniteDividendTestRanges{
      Unknown, PosConstant, InfinityValue, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : InfiniteDividendTestRanges)
    EXPECT_TRUE(FPValueRange::mod(InfinityValue, Range).getMaybeNaN());

  std::vector<FPValueRange> InfiniteDivisorTestRanges{
      Unknown, ZeroConstant, PosConstant, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : InfiniteDivisorTestRanges)
    EXPECT_TRUE(FPValueRange::mod(Range, InfinityValue).contains(Range));

  std::vector<FPValueRange> NaNTestRanges{
      Unknown,  PosConstant,    ZeroConstant,  InfinityValue,
      NaNValue, ConstantRange1, ConstantRange2};
  for (const FPValueRange &Range : NaNTestRanges) {
    EXPECT_TRUE(FPValueRange::mod(NaNValue, Range).getMaybeNaN());
    EXPECT_TRUE(FPValueRange::mod(Range, NaNValue).getMaybeNaN());
  }

  FPValueRange UnkModRange1 = FPValueRange::mod(Unknown, ConstantRange1);
  EXPECT_TRUE(
      UnkModRange1.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(-2.2), APFloat(2.2), true, false)));

  FPValueRange UnkModRange2 = FPValueRange::mod(Unknown, ConstantRange2);
  EXPECT_TRUE(
      UnkModRange2.contains(FPValueRange::createConstantOrConstantRange(
          APFloat(-7.1), APFloat(7.1), false, false)));
}

}
