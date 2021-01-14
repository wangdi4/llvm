//===- Intel_FPValueRangeAnalysisTest.cpp -----------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/Intel_FPValueRangeAnalysis.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "gtest/gtest.h"

using namespace llvm;

static Instruction *findInstructionByNameOrNull(Function *F, StringRef Name) {
  for (Instruction &I : instructions(F))
    if (I.getName() == Name)
      return &I;

  return nullptr;
}

static Instruction &findInstructionByName(Function *F, StringRef Name) {
  auto *I = findInstructionByNameOrNull(F, Name);
  if (I)
    return *I;

  llvm_unreachable("Expected value not found");
}

struct FPValueRangeAnalysisTestParams {
  const char *IR;
  // Each pair in Checks refers to an instruction inside the test() function of
  // IR by name, and stores an expected FPValueRange. The tests will make sure
  // that the analysis infers the same range as specified.
  std::vector<std::pair<const char *, FPValueRange>> Checks;
};

class FPValueRangeAnalysisTest
    : public testing::Test,
      public ::testing::WithParamInterface<FPValueRangeAnalysisTestParams> {
protected:
  std::unique_ptr<Module> parseModule(StringRef Assembly) {
    SMDiagnostic Error;
    std::unique_ptr<Module> M = parseAssemblyString(Assembly, Error, Context);

    std::string errMsg;
    raw_string_ostream os(errMsg);
    EXPECT_TRUE(M) << os.str();

    return M;
  }

  LLVMContext Context;
};

const FPValueRangeAnalysisTestParams FPValueRangeAnalysisTests[] = {
    {R"(
    define double @test() {
      %a = fmul double 4.0, 3.0
      ret double %a
    })",
     {{"a", FPValueRange::createConstant(APFloat(12.0), /*MaybeNaN=*/false,
                                         /*MaybeInfinity=*/false)}}},
    {R"(
    define double @test() {
    entry:
      br label %loop

    loop:
      %i0 = phi double [ 1.415900e+04, %entry ], [ %b, %loop ]
      %i1 = phi i32 [ 1, %entry ], [ %i9, %loop ]
      %i2 = phi double [ 0.000000e+00, %entry ], [ %i8, %loop ]
      %a = fmul double %i0, 1.680700e+04
      %b = frem double %a, 0x41DFFFFFFFC00000
      %i5 = fmul double %b, 0x3E00000000200000
      %i6 = fptrunc double %i5 to float
      %i7 = fpext float %i6 to double
      %i8 = fadd double %i2, %i7
      %i9 = add nuw nsw i32 %i1, 1
      %i10 = icmp eq i32 %i9, 501
      br i1 %i10, label %exit, label %loop

    exit:
      ret double %i8
    })",
     {{"a", FPValueRange::createConstantOrConstantRange(
                APFloat(-36092757655129.0), APFloat(36092757655129.0),
                /*MaybeNaN=*/false, /*MaybeInfinity=*/false)},
      {"b", FPValueRange::createConstantOrConstantRange(
                APFloat(-2147483647.0), APFloat(2147483647.0),
                /*MaybeNaN=*/false, /*MaybeInfinity=*/false)}}},
    {R"(
    define double @test() {
    entry:
      br label %loop

    loop:
      %i0 = phi double [ 1.415900e+04, %entry ], [ %b, %loop ]
      %i1 = phi i32 [ 1, %entry ], [ %i9, %loop ]
      %i2 = phi double [ 0.000000e+00, %entry ], [ %i8, %loop ]
      %a = fmul double %i0, 0x41F0000000000000
      %b = frem double %a, 0x41F0000000000000
      %i5 = fmul double %b, 0x3E00000000200000
      %i6 = fptrunc double %i5 to float
      %i7 = fpext float %i6 to double
      %i8 = fadd double %i2, %i7
      %i9 = add nuw nsw i32 %i1, 1
      %i10 = icmp eq i32 %i9, 501
      br i1 %i10, label %exit, label %loop

    exit:
      ret double %i8
    })",
     {{"a",
       FPValueRange::createConstantOrConstantRange(
           APFloat(-18446744073709551616.0), APFloat(18446744073709551616.0),
           /*MaybeNaN=*/false, /*MaybeInfinity=*/false)},
      {"b", FPValueRange::createConstantOrConstantRange(
                APFloat(-4294967296.0), APFloat(4294967296.0),
                /*MaybeNaN=*/false, /*MaybeInfinity=*/false)}}},
    {R"(
    define double @test(i32 %n) {
    entry:
      %cmp22 = icmp sgt i32 %n, 0
      br i1 %cmp22, label %for.cond2.preheader, label %for.cond.cleanup

    for.cond2.preheader:
      %i1.024 = phi i32 [ %inc10, %for.cond2.preheader ], [ 0, %entry ]
      %v.023 = phi double [ %fmod7.2, %for.cond2.preheader ], [ 0x41AC5E4712000000, %entry ]
      %mul6 = fmul fast double %v.023, 1.680700e+04
      %fmod7 = frem fast double %mul6, 0x41DFFFFFFFC00000
      %mul6.1 = fmul fast double %fmod7, 1.680700e+04
      %fmod7.1 = frem fast double %mul6.1, 0x41DFFFFFFFC00000
      %mul6.2 = fmul fast double %fmod7.1, 1.680700e+04
      %fmod7.2 = frem fast double %mul6.2, 0x41DFFFFFFFC00000
      %inc10 = add nuw nsw i32 %i1.024, 1
      %exitcond = icmp eq i32 %inc10, %n
      br i1 %exitcond, label %for.cond.cleanup, label %for.cond2.preheader

    for.cond.cleanup:
      %v.0.lcssa = phi double [ 0x41AC5E4712000000, %entry ], [ %fmod7.2, %for.cond2.preheader ]
      ret double %v.0.lcssa
    })",
     {
         {"mul6", FPValueRange::createConstantOrConstantRange(
                      APFloat(-36092757655129.0), APFloat(36092757655129.0),
                      /*MaybeNaN=*/false, /*MaybeInfinity=*/false)},
         {"mul6.1", FPValueRange::createConstantOrConstantRange(
                        APFloat(-36092757655129.0), APFloat(36092757655129.0),
                        /*MaybeNaN=*/false, /*MaybeInfinity=*/false)},
         {"mul6.2", FPValueRange::createConstantOrConstantRange(
                        APFloat(-36092757655129.0), APFloat(36092757655129.0),
                        /*MaybeNaN=*/false, /*MaybeInfinity=*/false)},
     }},
    {R"(
    declare zeroext i1 @do_i_continue()
    define float @test() {
    entry:
      %call11 = tail call zeroext i1 @do_i_continue()
      br i1 %call11, label %while.body, label %while.end

    while.body:
      %b.012 = phi float [ %conv4, %while.body ], [ 1.000000e+00, %entry ]
      %conv4 = fmul fast float %b.012, 0x3FF028F5C0000000
      %call = tail call zeroext i1 @do_i_continue()
      br i1 %call, label %while.body, label %while.end

    while.end:
      %b.0.lcssa = phi float [ 1.000000e+00, %entry ], [ %conv4, %while.body ]
      %a.0.lcssa = phi float [ 1.100000e+01, %entry ], [ 0x4024666660000000, %while.body ]
      %mul5 = fmul fast float %a.0.lcssa, %b.0.lcssa
      ret float %mul5
    })",
     {{"mul5", FPValueRange::createConstantOrConstantRange(
                   APFloat(10.1999998f), APFloat::getInf(APFloat::IEEEsingle()),
                   /*MaybeNaN=*/false, /*MaybeInfinity=*/false)}}},
    {R"(
    define double @test(i32 %n, double %l, double %p) {
    entry:
      %cmp43 = icmp sgt i32 %n, 0
      br i1 %cmp43, label %for.body, label %for.cond.cleanup

    for.cond.cleanup:
      %v.0.lcssa = phi double [ 1.979000e+03, %entry ], [ %v.3, %while.end ]
      ret double %v.0.lcssa

    for.body:
      %v.045 = phi double [ %v.3, %while.end ], [ 1.979000e+03, %entry ]
      %i1.044 = phi i32 [ %inc20, %while.end ], [ 0, %entry ]
      %mul2 = fmul fast double %v.045, 2.401000e+03
      %fmod.i39 = frem fast double %mul2, 8.191000e+03
      br label %while.cond

    while.cond:
      %j.0 = phi i32 [ 0, %for.body ], [ %dec, %cleanup15 ]
      %v.1 = phi double [ %fmod.i39, %for.body ], [ %v.3, %cleanup15 ]
      %0 = add i32 %j.0, 4
      br label %land.rhs

    land.rhs:
      %v.242 = phi double [ %v.1, %while.cond ], [ %fmod.i, %for.inc ]
      %j.140 = phi i32 [ %j.0, %while.cond ], [ %inc14, %for.inc ]
      %call7 = tail call zeroext i1 @_Z9unknown_fv()
      br i1 %call7, label %cleanup15, label %for.body9

    for.body9:
      %mul10 = fmul fast double %v.242, 2.401000e+03
      %fmod.i = frem fast double %mul10, 8.191000e+03
      %div12 = fmul fast double %fmod.i, 0x3F20008004002001
      %cmp13 = fcmp fast ogt double %div12, %p
      br i1 %cmp13, label %cleanup15, label %for.inc

    for.inc:
      %inc14 = add i32 %j.140, 1
      %exitcond = icmp eq i32 %inc14, %0
      br i1 %exitcond, label %cleanup15, label %land.rhs

    cleanup15:
      %j.1.lcssa = phi i32 [ %j.140, %land.rhs ], [ %0, %for.inc ], [ %j.140, %for.body9 ]
      %v.3 = phi double [ %v.242, %land.rhs ], [ %fmod.i, %for.inc ], [ %fmod.i, %for.body9 ]
      %cmp16 = icmp eq i32 %j.1.lcssa, 0
      %dec = add nsw i32 %j.1.lcssa, -1
      br i1 %cmp16, label %while.end, label %while.cond

    while.end:
      %inc20 = add nuw nsw i32 %i1.044, 1
      %exitcond46 = icmp eq i32 %inc20, %n
      br i1 %exitcond46, label %for.cond.cleanup, label %for.body
    }
    declare zeroext i1 @_Z9unknown_fv())",
     {{"mul2", FPValueRange::createConstantOrConstantRange(
                   APFloat(-19666591.0), APFloat(19666591.0),
                   /*MaybeNaN=*/false, /*MaybeInfinity=*/false)},
      {"mul10", FPValueRange::createConstantOrConstantRange(
                    APFloat(-19666591.0), APFloat(19666591.0),
                    /*MaybeNaN=*/false, /*MaybeInfinity=*/false)}}}};

TEST_P(FPValueRangeAnalysisTest, ComputeValueRange) {
  auto M = parseModule(GetParam().IR);
  Function *F = M->getFunction("test");

  TargetLibraryInfoImpl TLII;
  TargetLibraryInfo TLI(TLII);
  AssumptionCache AC(*F);
  LazyValueInfo LVI(&AC, &M->getDataLayout(), &TLI);
  FPValueRangeAnalysis RangeAnalysis(&LVI);

  for (const std::pair<const char *, FPValueRange> &Check :
       GetParam().Checks) {
    Instruction *I = &findInstructionByName(F, Check.first);
    FPValueRange Range = RangeAnalysis.computeRange(I);
    EXPECT_EQ(Range, Check.second);
  }
}

INSTANTIATE_TEST_CASE_P(
    FPValueRangeAnalysisTest, FPValueRangeAnalysisTest,
    ::testing::ValuesIn(FPValueRangeAnalysisTests), );
