//===- ValueTrackingTest.cpp - ValueTracking tests ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/ValueTracking.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/KnownBits.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

class MatchSelectPatternTest : public testing::Test {
protected:
  void parseAssembly(const char *Assembly) {
    SMDiagnostic Error;
    M = parseAssemblyString(Assembly, Error, Context);

    std::string errMsg;
    raw_string_ostream os(errMsg);
    Error.print("", os);

    // A failure here means that the test itself is buggy.
    if (!M)
      report_fatal_error(os.str());

    Function *F = M->getFunction("test");
    if (F == nullptr)
      report_fatal_error("Test must have a function named @test");

    A = nullptr;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (I->hasName()) {
        if (I->getName() == "A")
          A = &*I;
      }
    }
    if (A == nullptr)
      report_fatal_error("@test must have an instruction %A");
  }

  void expectPattern(const SelectPatternResult &P) {
    Value *LHS, *RHS;
    Instruction::CastOps CastOp;
    SelectPatternResult R = matchSelectPattern(A, LHS, RHS, &CastOp);
    EXPECT_EQ(P.Flavor, R.Flavor);
    EXPECT_EQ(P.NaNBehavior, R.NaNBehavior);
    EXPECT_EQ(P.Ordered, R.Ordered);
  }

  LLVMContext Context;
  std::unique_ptr<Module> M;
  Instruction *A, *B;
};

#if INTEL_CUSTOMIZATION
// Correct way of inheritance here is to create base class with only
// parseAssembly method and 3 children: MatchSelectPatternTest,
// MatchSaturationDownconvertTest and MatchSaturationAddSubTest. But this
// approach will cause conflicts during pulldown. So inherit new classes from
// MatchSelectPattern.
class MatchSaturationDownconvertTest : public MatchSelectPatternTest {
protected:
  void expectPattern(bool P) {
    const APInt *C1, *C2;
    bool Signed;
    Value *X;
    Type *SrcTy, *DestTy;
    bool R = matchSaturationDownconvert(A, X, C1, C2, SrcTy, DestTy, Signed);
    EXPECT_EQ(P, R);
  }
};

class MatchSaturationAddSubTest : public MatchSelectPatternTest {
protected:
  void expectPattern(bool P) {
    const APInt *C1, *C2;
    bool Signed;
    unsigned InstCode;
    Value *X1, *X2;
    Type *SrcTy, *DestTy;
    bool R = matchSaturationAddSub(A, X1, X2, C1, C2, SrcTy, DestTy, Signed, InstCode);
    EXPECT_EQ(P, R);
  }
};
#endif // INTEL_CUSTOMIZATION
}

TEST_F(MatchSelectPatternTest, SimpleFMin) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp ult float %a, 5.0\n"
      "  %A = select i1 %1, float %a, float 5.0\n"
      "  ret float %A\n"
      "}\n");
  expectPattern({SPF_FMINNUM, SPNB_RETURNS_NAN, false});
}

TEST_F(MatchSelectPatternTest, SimpleFMax) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp ogt float %a, 5.0\n"
      "  %A = select i1 %1, float %a, float 5.0\n"
      "  ret float %A\n"
      "}\n");
  expectPattern({SPF_FMAXNUM, SPNB_RETURNS_OTHER, true});
}

TEST_F(MatchSelectPatternTest, SwappedFMax) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp olt float 5.0, %a\n"
      "  %A = select i1 %1, float %a, float 5.0\n"
      "  ret float %A\n"
      "}\n");
  expectPattern({SPF_FMAXNUM, SPNB_RETURNS_OTHER, false});
}

TEST_F(MatchSelectPatternTest, SwappedFMax2) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp olt float %a, 5.0\n"
      "  %A = select i1 %1, float 5.0, float %a\n"
      "  ret float %A\n"
      "}\n");
  expectPattern({SPF_FMAXNUM, SPNB_RETURNS_NAN, false});
}

TEST_F(MatchSelectPatternTest, SwappedFMax3) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp ult float %a, 5.0\n"
      "  %A = select i1 %1, float 5.0, float %a\n"
      "  ret float %A\n"
      "}\n");
  expectPattern({SPF_FMAXNUM, SPNB_RETURNS_OTHER, true});
}

TEST_F(MatchSelectPatternTest, FastFMin) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp nnan olt float %a, 5.0\n"
      "  %A = select i1 %1, float %a, float 5.0\n"
      "  ret float %A\n"
      "}\n");
  expectPattern({SPF_FMINNUM, SPNB_RETURNS_ANY, false});
}

TEST_F(MatchSelectPatternTest, FMinConstantZero) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp ole float %a, 0.0\n"
      "  %A = select i1 %1, float %a, float 0.0\n"
      "  ret float %A\n"
      "}\n");
  // This shouldn't be matched, as %a could be -0.0.
  expectPattern({SPF_UNKNOWN, SPNB_NA, false});
}

TEST_F(MatchSelectPatternTest, FMinConstantZeroNsz) {
  parseAssembly(
      "define float @test(float %a) {\n"
      "  %1 = fcmp nsz ole float %a, 0.0\n"
      "  %A = select i1 %1, float %a, float 0.0\n"
      "  ret float %A\n"
      "}\n");
  // But this should be, because we've ignored signed zeroes.
  expectPattern({SPF_FMINNUM, SPNB_RETURNS_OTHER, true});
}

TEST_F(MatchSelectPatternTest, VectorFMinNaN) {
  parseAssembly(
      "define <4 x float> @test(<4 x float> %a) {\n"
      "  %1 = fcmp ule <4 x float> %a, \n"
      "    <float 5.0, float 5.0, float 5.0, float 5.0>\n"
      "  %A = select <4 x i1> %1, <4 x float> %a,\n"
      "     <4 x float> <float 5.0, float 5.0, float 5.0, float 5.0>\n"
      "  ret <4 x float> %A\n"
      "}\n");
  // Check that pattern matching works on vectors where each lane has the same
  // unordered pattern.
  expectPattern({SPF_FMINNUM, SPNB_RETURNS_NAN, false});
}

TEST_F(MatchSelectPatternTest, VectorFMinOtherOrdered) {
  parseAssembly(
      "define <4 x float> @test(<4 x float> %a) {\n"
      "  %1 = fcmp ole <4 x float> %a, \n"
      "    <float 5.0, float 5.0, float 5.0, float 5.0>\n"
      "  %A = select <4 x i1> %1, <4 x float> %a,\n"
      "     <4 x float> <float 5.0, float 5.0, float 5.0, float 5.0>\n"
      "  ret <4 x float> %A\n"
      "}\n");
  // Check that pattern matching works on vectors where each lane has the same
  // ordered pattern.
  expectPattern({SPF_FMINNUM, SPNB_RETURNS_OTHER, true});
}

TEST_F(MatchSelectPatternTest, VectorNotFMinNaN) {
  parseAssembly(
      "define <4 x float> @test(<4 x float> %a) {\n"
      "  %1 = fcmp ule <4 x float> %a, \n"
      "    <float 5.0, float 0x7ff8000000000000, float 5.0, float 5.0>\n"
      "  %A = select <4 x i1> %1, <4 x float> %a,\n"
      "     <4 x float> <float 5.0, float 0x7ff8000000000000, float 5.0, float "
      "5.0>\n"
      "  ret <4 x float> %A\n"
      "}\n");
  // The lane that contains a NaN (0x7ff80...) behaves like a
  // non-NaN-propagating min and the other lines behave like a NaN-propagating
  // min, so check that neither is returned.
  expectPattern({SPF_UNKNOWN, SPNB_NA, false});
}

TEST_F(MatchSelectPatternTest, VectorNotFMinZero) {
  parseAssembly(
      "define <4 x float> @test(<4 x float> %a) {\n"
      "  %1 = fcmp ule <4 x float> %a, \n"
      "    <float 5.0, float -0.0, float 5.0, float 5.0>\n"
      "  %A = select <4 x i1> %1, <4 x float> %a,\n"
      "     <4 x float> <float 5.0, float 0.0, float 5.0, float 5.0>\n"
      "  ret <4 x float> %A\n"
      "}\n");
  // Always selects the second lane of %a if it is positive or negative zero, so
  // this is stricter than a min.
  expectPattern({SPF_UNKNOWN, SPNB_NA, false});
}

TEST_F(MatchSelectPatternTest, DoubleCastU) {
  parseAssembly(
      "define i32 @test(i8 %a, i8 %b) {\n"
      "  %1 = icmp ult i8 %a, %b\n"
      "  %2 = zext i8 %a to i32\n"
      "  %3 = zext i8 %b to i32\n"
      "  %A = select i1 %1, i32 %2, i32 %3\n"
      "  ret i32 %A\n"
      "}\n");
  // We should be able to look through the situation where we cast both operands
  // to the select.
  expectPattern({SPF_UMIN, SPNB_NA, false});
}

TEST_F(MatchSelectPatternTest, DoubleCastS) {
  parseAssembly(
      "define i32 @test(i8 %a, i8 %b) {\n"
      "  %1 = icmp slt i8 %a, %b\n"
      "  %2 = sext i8 %a to i32\n"
      "  %3 = sext i8 %b to i32\n"
      "  %A = select i1 %1, i32 %2, i32 %3\n"
      "  ret i32 %A\n"
      "}\n");
  // We should be able to look through the situation where we cast both operands
  // to the select.
  expectPattern({SPF_SMIN, SPNB_NA, false});
}

TEST_F(MatchSelectPatternTest, DoubleCastBad) {
  parseAssembly(
      "define i32 @test(i8 %a, i8 %b) {\n"
      "  %1 = icmp ult i8 %a, %b\n"
      "  %2 = zext i8 %a to i32\n"
      "  %3 = sext i8 %b to i32\n"
      "  %A = select i1 %1, i32 %2, i32 %3\n"
      "  ret i32 %A\n"
      "}\n");
  // The cast types here aren't the same, so we cannot match an UMIN.
  expectPattern({SPF_UNKNOWN, SPNB_NA, false});
}

#if INTEL_CUSTOMIZATION
TEST_F(MatchSaturationAddSubTest, SignedSatAddi8) {
  parseAssembly(
      "define signext i8 @test(i8 signext %a, i8 signext %b) {\n"
      "  %conv = sext i8 %a to i32\n"
      "  %conv1 = sext i8 %b to i32\n"
      "  %add = add nsw i32 %conv1, %conv\n"
      "  %tmp = icmp slt i32 %add, 127\n"
      "  %.phitmp3132 = select i1 %tmp, i32 %add, i32 127\n"
      "  %tmp1 = icmp sgt i32 %.phitmp3132, -128\n"
      "  %retval33 = select i1 %tmp1, i32 %.phitmp3132, i32 -128\n"
      "  %A = trunc i32 %retval33 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, SignedSatAddi16) {
  parseAssembly(
      "define signext i16 @test(i16 signext %a, i16 signext %b) {\n"
      "  %conv = sext i16 %a to i32\n"
      "  %conv1 = sext i16 %b to i32\n"
      "  %add = add nsw i32 %conv1, %conv\n"
      "  %tmp = icmp slt i32 %add, 32767\n"
      "  %.phitmp3132 = select i1 %tmp, i32 %add, i32 32767\n"
      "  %tmp1 = icmp sgt i32 %.phitmp3132, -32768\n"
      "  %retval33 = select i1 %tmp1, i32 %.phitmp3132, i32 -32768\n"
      "  %A = trunc i32 %retval33 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, SignedSatSubi8) {
  parseAssembly(
      "define signext i8 @test(i8 signext %a, i8 signext %b) {\n"
      "  %conv = sext i8 %a to i32\n"
      "  %conv1 = sext i8 %b to i32\n"
      "  %sub = sub nsw i32 %conv, %conv1\n"
      "  %tmp = icmp slt i32 %sub, 127\n"
      "  %.phitmp3132 = select i1 %tmp, i32 %sub, i32 127\n"
      "  %tmp1 = icmp sgt i32 %.phitmp3132, -128\n"
      "  %retval33 = select i1 %tmp1, i32 %.phitmp3132, i32 -128\n"
      "  %A = trunc i32 %retval33 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, SignedSatSubi16) {
  parseAssembly(
      "define signext i16 @test(i16 signext %a, i16 signext %b) {\n"
      "  %conv = sext i16 %a to i32\n"
      "  %conv1 = sext i16 %b to i32\n"
      "  %sub = sub nsw i32 %conv, %conv1\n"
      "  %tmp = icmp slt i32 %sub, 32767\n"
      "  %.phitmp3132 = select i1 %tmp, i32 %sub, i32 32767\n"
      "  %tmp1 = icmp sgt i32 %.phitmp3132, -32768\n"
      "  %retval33 = select i1 %tmp1, i32 %.phitmp3132, i32 -32768\n"
      "  %A = trunc i32 %retval33 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, ShortPatternUnsignedSatAddi8) {
  parseAssembly(
      "define zeroext i8 @test(i8 zeroext %a, i8 zeroext %b) {\n"
      "entry:\n"
      "  %conv = zext i8 %a to i32\n"
      "  %conv1 = zext i8 %b to i32\n"
      "  %add = add nuw nsw i32 %conv1, %conv\n"
      "  %tmp = icmp ult i32 %add, 255\n"
      "  %cond2331 = select i1 %tmp, i32 %add, i32 255\n"
      "  %A = trunc i32 %cond2331 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, ShortPatternUnsignedSatAddi16) {
  parseAssembly(
      "define zeroext i16 @test(i16 zeroext %a, i16 zeroext %b) {\n"
      "entry:\n"
      "  %conv = zext i16 %a to i32\n"
      "  %conv1 = zext i16 %b to i32\n"
      "  %add = add nuw nsw i32 %conv1, %conv\n"
      "  %tmp = icmp ult i32 %add, 65535\n"
      "  %cond2331 = select i1 %tmp, i32 %add, i32 65535\n"
      "  %A = trunc i32 %cond2331 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, ShortPatternUnsignedSatSubi8) {
  parseAssembly(
      "define zeroext i8 @test(i8 zeroext %a, i8 zeroext %b) {\n"
      "entry:\n"
      "  %conv = zext i8 %a to i32\n"
      "  %conv1 = zext i8 %b to i32\n"
      "  %sub = sub nsw i32 %conv1, %conv\n"
      "  %tmp = icmp sgt i32 %sub, 0\n"
      "  %cond2331 = select i1 %tmp, i32 %sub, i32 0\n"
      "  %A = trunc i32 %cond2331 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, ShortPatternUnsignedSatSubi16) {
  parseAssembly(
      "define zeroext i16 @test(i16 zeroext %a, i16 zeroext %b) {\n"
      "entry:\n"
      "  %conv = zext i16 %a to i32\n"
      "  %conv1 = zext i16 %b to i32\n"
      "  %sub = sub nsw i32 %conv1, %conv\n"
      "  %tmp = icmp sgt i32 %sub, 0\n"
      "  %cond2331 = select i1 %tmp, i32 %sub, i32 0\n"
      "  %A = trunc i32 %cond2331 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, UnsignedSatSubi8) {
  parseAssembly(
      "define zeroext i8 @test(i8 zeroext %a, i8 zeroext %b) {\n"
      "entry:\n"
      "  %conv = zext i8 %a to i32\n"
      "  %conv1 = zext i8 %b to i32\n"
      "  %sub = sub nsw i32 %conv, %conv1\n"
      "  %tmp = icmp slt i32 %sub, 255\n"
      "  %.phitmp3132 = select i1 %tmp, i32 %sub, i32 255\n"
      "  %tmp1 = icmp sgt i32 %.phitmp3132, 0\n"
      "  %retval33 = select i1 %tmp1, i32 %.phitmp3132, i32 0\n"
      "  %A = trunc i32 %retval33 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, UnsignedSatSubi16) {
  parseAssembly(
      "define zeroext i16 @test(i16 zeroext %a, i16 zeroext %b) {\n"
      "entry:\n"
      "  %conv = zext i16 %a to i32\n"
      "  %conv1 = zext i16 %b to i32\n"
      "  %sub = sub nsw i32 %conv, %conv1\n"
      "  %tmp = icmp slt i32 %sub, 65535\n"
      "  %.phitmp3132 = select i1 %tmp, i32 %sub, i32 65535\n"
      "  %tmp1 = icmp sgt i32 %.phitmp3132, 0\n"
      "  %retval33 = select i1 %tmp1, i32 %.phitmp3132, i32 0\n"
      "  %A = trunc i32 %retval33 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationDownconvertTest, SignedSatDcnvi8) {
  parseAssembly(
      "define signext i8 @test(i16 signext %a) {\n"
      "  %conv = sext i16 %a to i32\n"
      "  %tmp = icmp slt i32 %conv, 127\n"
      "  %cond = select i1 %tmp, i32 %conv, i32 127\n"
      "  %tmp1 = icmp sgt i32 %cond, -128\n"
      "  %.phitmp21 = select i1 %tmp1, i32 %cond, i32 -128\n"
      "  %A = trunc i32 %.phitmp21 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationDownconvertTest, SignedSatDcnvi16) {
  parseAssembly(
      "define signext i16 @test(i32 %a) {\n"
      "  %tmp = icmp slt i32 %a, 32767\n"
      "  %cond = select i1 %tmp, i32 %a, i32 32767\n"
      "  %tmp1 = icmp sgt i32 %cond, -32768\n"
      "  %.phitmp14 = select i1 %tmp1, i32 %cond, i32 -32768\n"
      "  %A = trunc i32 %.phitmp14 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationDownconvertTest, UnsignedSatDcnvi8) {
  parseAssembly(
      "define zeroext i8 @test(i16 signext %a) {\n"
      "entry:\n"
      "  %conv = sext i16 %a to i32\n"
      "  %tmp = icmp slt i32 %conv, 255\n"
      "  %cond = select i1 %tmp, i32 %conv, i32 255\n"
      "  %tmp1 = icmp sgt i32 %cond, 0\n"
      "  %.phitmp21 = select i1 %tmp1, i32 %cond, i32 0\n"
      "  %A = trunc i32 %.phitmp21 to i8\n"
      "  ret i8 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationDownconvertTest, UnsignedSatDcnvi16) {
  parseAssembly(
      "define zeroext i16 @test(i32 %a) {\n"
      "entry:\n"
      "  %tmp = icmp slt i32 %a, 65535\n"
      "  %cond = select i1 %tmp, i32 %a, i32 65535\n"
      "  %tmp1 = icmp sgt i32 %cond, 0\n"
      "  %.phitmp14 = select i1 %tmp1, i32 %cond, i32 0\n"
      "  %A = trunc i32 %.phitmp14 to i16\n"
      "  ret i16 %A\n"
      "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, SignedSatAddv2i8) {
  parseAssembly(
    "define signext <2 x i8> @test(<2 x i8> signext %a, <2 x i8> signext %b) {\n"
      "  %tmp6 = sext <2 x i8> %a to <2 x i32>\n"
      "  %tmp9 = sext <2 x i8> %b to <2 x i32>\n"
      "  %tmp10 = add nsw <2 x i32> %tmp9, %tmp6\n"
      "  %tmp11 = icmp slt <2 x i32> %tmp10, <i32 127, i32 127>\n"
      "  %tmp12 = select <2 x i1> %tmp11, <2 x i32> %tmp10, <2 x i32> <i32 127, i32 127>\n"
      "  %tmp13 = icmp sgt <2 x i32> %tmp12, <i32 -128, i32 -128>\n"
      "  %tmp14 = select <2 x i1> %tmp13, <2 x i32> %tmp12, <2 x i32> <i32 -128, i32 -128>\n"
      "  %A = trunc <2 x i32> %tmp14 to <2 x i8>\n"
      "  ret <2 x i8> %A\n"
    "}\n");
  expectPattern(true);
}

TEST_F(MatchSaturationAddSubTest, OutOfRangeSignedSatAddv2i8) {
  parseAssembly(
    "define signext <2 x i8> @test(<2 x i8> signext %a, <2 x i8> signext %b) {\n"
      "  %tmp6 = sext <2 x i8> %a to <2 x i32>\n"
      "  %tmp9 = sext <2 x i8> %b to <2 x i32>\n"
      "  %tmp10 = add nsw <2 x i32> %tmp9, %tmp6\n"
      "  %tmp11 = icmp slt <2 x i32> %tmp10, <i32 200, i32 200>\n"
      "  %tmp12 = select <2 x i1> %tmp11, <2 x i32> %tmp10, <2 x i32> <i32 200, i32 200>\n"
      "  %tmp13 = icmp sgt <2 x i32> %tmp12, <i32 -128, i32 -128>\n"
      "  %tmp14 = select <2 x i1> %tmp13, <2 x i32> %tmp12, <2 x i32> <i32 -128, i32 -128>\n"
      "  %A = trunc <2 x i32> %tmp14 to <2 x i8>\n"
      "  ret <2 x i8> %A\n"
    "}\n");
  expectPattern(false);
}
#endif // INTEL_CUSTOMIZATION

TEST(ValueTracking, GuaranteedToTransferExecutionToSuccessor) {
  StringRef Assembly =
      "declare void @nounwind_readonly(i32*) nounwind readonly "
      "declare void @nounwind_argmemonly(i32*) nounwind argmemonly "
      "declare void @throws_but_readonly(i32*) readonly "
      "declare void @throws_but_argmemonly(i32*) argmemonly "
      " "
      "declare void @unknown(i32*) "
      " "
      "define void @f(i32* %p) { "
      "  call void @nounwind_readonly(i32* %p) "
      "  call void @nounwind_argmemonly(i32* %p) "
      "  call void @throws_but_readonly(i32* %p) "
      "  call void @throws_but_argmemonly(i32* %p) "
      "  call void @unknown(i32* %p) nounwind readonly "
      "  call void @unknown(i32* %p) nounwind argmemonly "
      "  call void @unknown(i32* %p) readonly "
      "  call void @unknown(i32* %p) argmemonly "
      "  ret void "
      "} ";

  LLVMContext Context;
  SMDiagnostic Error;
  auto M = parseAssemblyString(Assembly, Error, Context);
  assert(M && "Bad assembly?");

  auto *F = M->getFunction("f");
  assert(F && "Bad assembly?");

  auto &BB = F->getEntryBlock();
  bool ExpectedAnswers[] = {
      true,  // call void @nounwind_readonly(i32* %p)
      true,  // call void @nounwind_argmemonly(i32* %p)
      false, // call void @throws_but_readonly(i32* %p)
      false, // call void @throws_but_argmemonly(i32* %p)
      true,  // call void @unknown(i32* %p) nounwind readonly
      true,  // call void @unknown(i32* %p) nounwind argmemonly
      false, // call void @unknown(i32* %p) readonly
      false, // call void @unknown(i32* %p) argmemonly
      false, // ret void
  };

  int Index = 0;
  for (auto &I : BB) {
    EXPECT_EQ(isGuaranteedToTransferExecutionToSuccessor(&I),
              ExpectedAnswers[Index])
        << "Incorrect answer at instruction " << Index << " = " << I;
    Index++;
  }
}

TEST(ValueTracking, ComputeNumSignBits_PR32045) {
  StringRef Assembly = "define i32 @f(i32 %a) { "
                       "  %val = ashr i32 %a, -1 "
                       "  ret i32 %val "
                       "} ";

  LLVMContext Context;
  SMDiagnostic Error;
  auto M = parseAssemblyString(Assembly, Error, Context);
  assert(M && "Bad assembly?");

  auto *F = M->getFunction("f");
  assert(F && "Bad assembly?");

  auto *RVal =
      cast<ReturnInst>(F->getEntryBlock().getTerminator())->getOperand(0);
  EXPECT_EQ(ComputeNumSignBits(RVal, M->getDataLayout()), 1u);
}

TEST(ValueTracking, ComputeKnownBits) {
  StringRef Assembly = "define i32 @f(i32 %a, i32 %b) { "
                       "  %ash = mul i32 %a, 8 "
                       "  %aad = add i32 %ash, 7 "
                       "  %aan = and i32 %aad, 4095 "
                       "  %bsh = shl i32 %b, 4 "
                       "  %bad = or i32 %bsh, 6 "
                       "  %ban = and i32 %bad, 4095 "
                       "  %mul = mul i32 %aan, %ban "
                       "  ret i32 %mul "
                       "} ";

  LLVMContext Context;
  SMDiagnostic Error;
  auto M = parseAssemblyString(Assembly, Error, Context);
  assert(M && "Bad assembly?");

  auto *F = M->getFunction("f");
  assert(F && "Bad assembly?");

  auto *RVal =
      cast<ReturnInst>(F->getEntryBlock().getTerminator())->getOperand(0);
  auto Known = computeKnownBits(RVal, M->getDataLayout());
  ASSERT_FALSE(Known.hasConflict());
  EXPECT_EQ(Known.One.getZExtValue(), 10u);
  EXPECT_EQ(Known.Zero.getZExtValue(), 4278190085u);
}

TEST(ValueTracking, ComputeKnownMulBits) {
  StringRef Assembly = "define i32 @f(i32 %a, i32 %b) { "
                       "  %aa = shl i32 %a, 5 "
                       "  %bb = shl i32 %b, 5 "
                       "  %aaa = or i32 %aa, 24 "
                       "  %bbb = or i32 %bb, 28 "
                       "  %mul = mul i32 %aaa, %bbb "
                       "  ret i32 %mul "
                       "} ";

  LLVMContext Context;
  SMDiagnostic Error;
  auto M = parseAssemblyString(Assembly, Error, Context);
  assert(M && "Bad assembly?");

  auto *F = M->getFunction("f");
  assert(F && "Bad assembly?");

  auto *RVal =
      cast<ReturnInst>(F->getEntryBlock().getTerminator())->getOperand(0);
  auto Known = computeKnownBits(RVal, M->getDataLayout());
  ASSERT_FALSE(Known.hasConflict());
  EXPECT_EQ(Known.One.getZExtValue(), 32u);
  EXPECT_EQ(Known.Zero.getZExtValue(), 95u);
}
