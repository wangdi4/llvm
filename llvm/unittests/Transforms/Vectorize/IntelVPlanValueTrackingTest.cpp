//===- IntelVPlanValueTracking.cpp ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanTestBase.h"

#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValueTracking.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

class VPlanValueTrackingTest : public vpo::VPlanTestBase {
protected:
  KnownBits getKnownBitsForStoreAddressBase(const char *ModuleString) {
    Module &M = parseModule(ModuleString);
    Function *F = M.getFunction("foo");
    BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
    std::unique_ptr<VPlan> Plan = buildHCFG(LoopHeader);

    VPlanScalarEvolutionLLVM VPSE(*SE, *LI->begin());
    VPlanValueTrackingLLVM VT(VPSE, *DL, &*AC, &*DT);

    VPInstruction *Store = nullptr;
    for (auto &BB : *Plan)
      for (auto &VPInst : BB) {
        if (VPInst.getOpcode() == Instruction::Store) {
          EXPECT_EQ(Store, nullptr) << "Multiple stores in the module";
          Store = &VPInst;
        }
      }

    VPValue *Pointer = Store->getOperand(1);
    VPlanSCEV *Scev = VPSE.getVPlanSCEV(*Pointer);
    auto Induction = VPSE.asConstStepInduction(Scev);
    return VT.getKnownBits(Induction->InvariantBase, Store);
  }
};

TEST_F(VPlanValueTrackingTest, Bitwise) {
  // ptr = (buf & -64) + 56;
  // ptr[6 + i] = ...
  const char *ModuleString =
      "define void @foo(i64* %buf) {\n"
      "entry:\n"
      "  %0 = ptrtoint i64* %buf to i64\n"
      "  %and = and i64 %0, -64\n"
      "  %add = or i64 %and, 56\n"
      "  %1 = inttoptr i64 %add to i64*\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %i.08 = phi i64 [ 0, %entry ], [ %inc, %for.body ]\n"
      "  %add1 = add nuw nsw i64 %i.08, 6\n"
      "  %arrayidx = getelementptr inbounds i64, i64* %1, i64 %add1\n"
      "  store i64 %i.08, i64* %arrayidx, align 8\n"
      "  %inc = add nuw nsw i64 %i.08, 1\n"
      "  %exitcond = icmp eq i64 %inc, 1024\n"
      "  br i1 %exitcond, label %for.cond.cleanup, label %for.body\n"
      "for.cond.cleanup:\n"
      "  ret void\n"
      "}\n";

  KnownBits KB = getKnownBitsForStoreAddressBase(ModuleString);
  EXPECT_EQ(KB.Zero, 0b010111);
  EXPECT_EQ(KB.One,  0b101000);
}

TEST_F(VPlanValueTrackingTest, BitwiseMul) {
  const char *ModuleString =
    "define void @foo(i64* %buf, i64 %x) {\n"
    "entry:\n"
    "  %buf.asInt = ptrtoint i64* %buf to i64\n"
    "  %ptr.asInt = and i64 %buf.asInt, -1024\n" // 10 trailing zero bits.
    "  %ptr = inttoptr i64 %ptr.asInt to i64*\n"
    // %y = 2 * %x
    "  %y = mul i64 %x, 2\n"
    "  br label %for.body\n"
    "for.body:\n"
    "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
    // %offset = 72 * (2 * %x) = 16 * (9 * %x)
    "  %offset = mul i64 %y, 72\n"
    "  %add1 = add nuw nsw i64 %counter, %offset\n"
    // %arrayidx = %ptr + (16 * sizeof(i64)) * (9 * %x) + %counter
    "  %arrayidx = getelementptr inbounds i64, i64* %ptr, i64 %add1\n"
    "  store i64 %counter, i64* %arrayidx, align 8\n"
    "  %counter.next = add nuw nsw i64 %counter, 1\n"
    "  %exitcond = icmp eq i64 %counter.next, 1024\n"
    "  br i1 %exitcond, label %for.cond.cleanup, label %for.body\n"
    "for.cond.cleanup:\n"
    "  ret void\n"
    "}\n";

  // Store address base is a multiple of (16 * sizeof(i64) = 2^7),
  // which means 7 trailing zero bits.
  KnownBits KB = getKnownBitsForStoreAddressBase(ModuleString);
  EXPECT_EQ(KB.Zero, 0b01111111);
  EXPECT_EQ(KB.One, 0);
}

TEST_F(VPlanValueTrackingTest, SimpleAssumeAlignedLegacy) {
  // __builtin_assume_aligned(buf, 64);
  // buf[3 + i] = ...
  const char *ModuleString =
      "declare void @llvm.assume(i1)\n"
      "define void @foo(i64* %buf) {\n"
      "entry:\n"
      "  %ptrint = ptrtoint i64* %buf to i64\n"
      "  %maskedptr = and i64 %ptrint, 63\n"
      "  %maskcond = icmp eq i64 %maskedptr, 0\n"
      "  tail call void @llvm.assume(i1 %maskcond)\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %i.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]\n"
      "  %add = add nuw nsw i64 %i.06, 3\n"
      "  %arrayidx = getelementptr inbounds i64, i64* %buf, i64 %add\n"
      "  store i64 %i.06, i64* %arrayidx, align 8\n"
      "  %inc = add nuw nsw i64 %i.06, 1\n"
      "  %exitcond = icmp eq i64 %inc, 1024\n"
      "  br i1 %exitcond, label %for.cond.cleanup, label %for.body\n"
      "for.cond.cleanup:\n"
      "  ret void\n"
      "}\n";

  KnownBits KB = getKnownBitsForStoreAddressBase(ModuleString);
  EXPECT_EQ(KB.Zero, 0b100111);
  EXPECT_EQ(KB.One,  0b011000);
}

} // namespace
