#if INTEL_COLLAB
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
//===------ VPOUtilsTest.cpp - Unit tests for VPOUtils --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "gtest/gtest.h"

using namespace llvm;

TEST(VPOUtils, Simple) {
  LLVMContext Ctx;
  SMDiagnostic Err;
  std::unique_ptr<Module> M(parseAssemblyString(R"invalid(
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() {
entry:
  %a = alloca [2 x [2 x i32]]
  br label %begin

begin:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %region

region:
  br label %exit

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
  )invalid",
                                                Err, Ctx));

  Function *Func = M->getFunction("foo");
  BasicBlock *Region = &*std::next(Func->begin(), 2);
  DominatorTree DT(*Func);
  auto *AI = cast<AllocaInst>(Func->begin()->begin());
  vpo::VPOUtils::addPrivateToEnclosingRegion(
      AI, Region, DT, false /* SimdOnly */, true /* ForceTypedClause */);
  Region->begin()->dump();

  BasicBlock *Begin = &*std::next(Func->begin());
  auto *Directive = cast<IntrinsicInst>(Begin->begin());

  // Expecting to see the following
  // %tok = call token @llvm.directive.region.entry() [
  //     "DIR.OMP.PARALLEL"(),
  //     "QUAL.OMP.PRIVATE:TYPED"([2 x [2 x i32]]* %a, i32 0, i64 4) ]
  EXPECT_EQ(Directive->getNumOperandBundles(), 2u);
  OperandBundleUse Priv = Directive->getOperandBundleAt(1);
  EXPECT_EQ(Priv.Inputs.size(), 3u);
  auto *TypeParam = dyn_cast<ConstantInt>(Priv.Inputs[1]);
  auto *SizeParam = dyn_cast<ConstantInt>(Priv.Inputs[2]);
  EXPECT_TRUE(TypeParam && SizeParam);
  EXPECT_TRUE(SizeParam->getValue() == 4);
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#endif
