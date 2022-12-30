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
  std::unique_ptr<VPlanVector> Plan;

  static BasicBlock *findBlockByName(Function *F, StringRef Name) {
    for (BasicBlock &BB : F->getBasicBlockList())
      if (BB.getName() == Name)
        return &BB;
    return nullptr;
  }

  void buildVPlanFromString(const char *ModuleString) {
    Module &M = parseModule(ModuleString);
    Function *F = M.getFunction("foo");

    BasicBlock *LoopHeader = findBlockByName(F, "for.body");
    ASSERT_TRUE(LoopHeader) << "No block named 'for.body'!";

    Plan = buildHCFG(LoopHeader);
  }

  KnownBits getKnownBitsForStoreAddressBase(const char *ModuleString) {
    buildVPlanFromString(ModuleString);
    VPLoadStoreInst *Store = nullptr;
    for (auto &BB : *Plan)
      for (auto &VPInst : BB) {
        if (VPInst.getOpcode() == Instruction::Store) {
          EXPECT_EQ(Store, nullptr) << "Multiple stores in the module";
          Store = &cast<VPLoadStoreInst>(VPInst);
        }
      }

    VPlanSCEV *Scev = Store->getAddressSCEV();
    auto Induction = Plan->getVPSE()->asConstStepInduction(Scev);
    return Plan->getVPVT()->getKnownBits(Induction->InvariantBase, Store);
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

struct VPlanComputeKnownBitsTest : public VPlanValueTrackingTest {
  VPInstruction *findInstructionByName(StringRef Name) const {
    for (VPInstruction &I : vpinstructions(Plan.get()))
      if (I.getOrigName() == Name)
        return &I;
    return nullptr;
  }

  template <typename InstTy>
  InstTy *
  findFirstInstruction(llvm::function_ref<bool(const InstTy *)> P) const {
    for (VPInstruction &I : vpinstructions(Plan.get()))
      if (auto *Inst = dyn_cast<InstTy>(&I))
        if (P(Inst))
          return Inst;
    return nullptr;
  }

  template <typename InstTy> InstTy *findFirstInstruction() const {
    return findFirstInstruction<InstTy>([](const auto *) { return true; });
  }

  VPLoadStoreInst *findFirstLoad() const {
    return findFirstInstruction<VPLoadStoreInst>([](const VPLoadStoreInst *LS) {
      return LS->getOpcode() == Instruction::Load;
    });
  }

  VPLoadStoreInst *findFirstStore() const {
    return findFirstInstruction<VPLoadStoreInst>([](const VPLoadStoreInst *LS) {
      return LS->getOpcode() == Instruction::Store;
    });
  }

  void expectKnownBits(Twine Name, const VPValue *V, const VPInstruction *CtxI,
                       uint64_t Zero, uint64_t One) {
    KnownBits KB = Plan->getVPVT()->getKnownBits(V, CtxI);
    EXPECT_EQ(KB.Zero.getZExtValue(), Zero) << "KB('%" << Name << "')";
    EXPECT_EQ(KB.One.getZExtValue(), One) << "KB('%" << Name << "')";
  }

  void expectKnownBits(StringRef Name, uint64_t Zero, uint64_t One) {
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";
    expectKnownBits(Name, I, I, Zero, One);
  }

  void expectKnownBitsForOperand(StringRef Name, unsigned Idx, uint64_t Zero,
                                 uint64_t One) {
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";
    expectKnownBits(Twine(Name) + "." + std::to_string(Idx), I->getOperand(Idx),
                    I, Zero, One);
  }
};

TEST_F(VPlanComputeKnownBitsTest, Arithmetic) {
  buildVPlanFromString(R"(
    define void @foo(i64 %a, i32 %b) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      %a.mul = mul i64 %a, 4
      %a.add = add i64 %a.mul, 7
      %a.and = and i64 %a.add, 127
      %b.shl = shl i32 %b, 3
      %b.or  = or  i32 %b.shl, 63
      %b.sub = sub i32 %b.or, 1
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");
  // clang-format off
  expectKnownBits("a.mul", /*Zero:*/ 0b00000011, /*One:*/ 0b00000000);
  expectKnownBits("a.add", /*Zero:*/ 0b00000000, /*One:*/ 0b00000011);
  expectKnownBits("a.and", /*Zero:*/ ~0b1111111, /*One:*/ 0b00000011);
  expectKnownBits("b.shl", /*Zero:*/ 0b00000111, /*One:*/ 0b00000000);
  expectKnownBits("b.or",  /*Zero:*/ 0b00000000, /*One:*/ 0b00111111);
  expectKnownBits("b.sub", /*Zero:*/ 0b00000001, /*One:*/ 0b00111110);
  // clang-format on
}

TEST_F(VPlanComputeKnownBitsTest, Constants) {
  buildVPlanFromString(R"(
    define void @foo(i64 %a) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      %mul = mul i64 %a, 0
      %add = add i64 %a, 127
      %gep = getelementptr i64, i64* null, i64 42
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");
  // clang-format off
  expectKnownBitsForOperand("mul", /*Idx:*/ 1, /*Zero:*/   ~0, /*One:*/   0);
  expectKnownBitsForOperand("add", /*Idx:*/ 1, /*Zero:*/ ~127, /*One:*/ 127);
  expectKnownBitsForOperand("gep", /*Idx:*/ 0, /*Zero:*/   ~0, /*One:*/   0);
  // clang-format on
}

TEST_F(VPlanComputeKnownBitsTest, ExternalDef) {
  buildVPlanFromString(R"(
    declare void @llvm.assume(i1)
    define void @foo(i64* %p) {
    entry:
      call void @llvm.assume(i1 true) [ "align"(i64* %p, i32 64) ]
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      store i64 0, i64* %p, align 8
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");
  const auto *Store = findFirstInstruction<VPLoadStoreInst>();
  ASSERT_TRUE(Store);

  KnownBits ArgKB =
      Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
  EXPECT_EQ(ArgKB.Zero, 63);
  EXPECT_EQ(ArgKB.One, 0);
}

TEST_F(VPlanComputeKnownBitsTest, GEP) {
  buildVPlanFromString(R"(
    %base.ty = type { [4 x i32], [4 x i32] } ; sizeof(%base.ty) == 32

    declare void @llvm.assume(i1)
    define void @foo([256 x %base.ty]* %p, [256 x %base.ty]* %q) {
    entry:
      call void @llvm.assume(i1 true) [ "align"([256 x %base.ty]* %p, i32 64) ]
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]

      ; Test constant (and known-stride) offsets from aligned base
      %gep1 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 0
      %gep2 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 0, i64 %iv
      %gep3 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 0, i64 %iv, i32 0
      %gep4 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 0, i64 %iv, i32 0, i64 4
      %gep5 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 2
      %gep6 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 2, i64 %iv
      %gep7 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 2, i64 %iv, i32 1
      %gep8 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 2, i64 %iv, i32 1, i64 2

      ; Test that we recurse on instruction operands
      %mul.iv = mul i64 %iv, 4
      %gep9 = getelementptr [256 x %base.ty], [256 x %base.ty]* %p, i64 0, i64 %iv, i32 0, i64 %mul.iv

      ; Test that offsets from unknown base always result in unknown
      %gep10 = getelementptr [256 x %base.ty], [256 x %base.ty]* %q
      %gep11 = getelementptr [256 x %base.ty], [256 x %base.ty]* %q, i64 0
      %gep12 = getelementptr [256 x %base.ty], [256 x %base.ty]* %q, i64 %mul.iv, i64 %iv
      %gep13 = getelementptr [256 x %base.ty], [256 x %base.ty]* %q, i64 2, i64 %iv, i32 1
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");
  // clang-format off
  expectKnownBits("gep1", /*Zero:*/ 63, /*One:*/  0);
  expectKnownBits("gep2", /*Zero:*/ 31, /*One:*/  0);
  expectKnownBits("gep3", /*Zero:*/ 31, /*One:*/  0);
  expectKnownBits("gep4", /*Zero:*/ 15, /*One:*/ 16);
  expectKnownBits("gep5", /*Zero:*/ 63, /*One:*/  0);
  expectKnownBits("gep6", /*Zero:*/ 31, /*One:*/  0);
  expectKnownBits("gep7", /*Zero:*/ 15, /*One:*/ 16);
  expectKnownBits("gep8", /*Zero:*/  7, /*One:*/ 24);

  expectKnownBits("gep9", /*Zero:*/ 15, /*One:*/  0);

  expectKnownBits("gep10", /*Zero:*/ 0, /*One:*/ 0);
  expectKnownBits("gep11", /*Zero:*/ 0, /*One:*/ 0);
  expectKnownBits("gep12", /*Zero:*/ 0, /*One:*/ 0);
  expectKnownBits("gep13", /*Zero:*/ 0, /*One:*/ 0);
  // clang-format on
}

TEST_F(VPlanComputeKnownBitsTest, ToggleUseUnderlyingValues) {
  buildVPlanFromString(R"(
    define void @foo(i64 %a) {
    entry:
      %a.mul = mul i64 %a, 8
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      %add = add i64 %a.mul, 0
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  // Expect underlying values to be used by default
  EXPECT_TRUE(VPlanValueTracking::getUseUnderlyingValues());
  expectKnownBitsForOperand("add", /*Idx: */ 0, /*Zero: */ 7, /*One: */ 0);

  // Expect that we can toggle underlying values to not be used.
  VPlanValueTracking::setUseUnderlyingValues(false);
  expectKnownBitsForOperand("add", /*Idx: */ 0, /*Zero: */ 0, /*One: */ 0);

  // Expect that we can toggle underlying values to be used.
  VPlanValueTracking::setUseUnderlyingValues(true);
  expectKnownBitsForOperand("add", /*Idx: */ 0, /*Zero: */ 7, /*One: */ 0);
}

TEST_F(VPlanComputeKnownBitsTest, AffectedByInternalAssumption) {
  buildVPlanFromString(R"(
    declare void @llvm.assume(i1)
    define void @foo(i64* %p) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      call void @llvm.assume(i1 true) [ "align"(i64* %p, i32 64) ]
      store i64 0, i64* %p, align 8
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const auto *Store = findFirstInstruction<VPLoadStoreInst>();
  ASSERT_TRUE(Store);

  // Ensure alignment comes from the VPAssumptionCache and not from underlying
  // LLVM ValueTracking.
  VPlanValueTracking::setUseUnderlyingValues(false);

  KnownBits ArgKB =
      Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
  EXPECT_EQ(ArgKB.Zero, 63);
  EXPECT_EQ(ArgKB.One, 0);
}

TEST_F(VPlanComputeKnownBitsTest, AffectedByExternalAssumption) {
  buildVPlanFromString(R"(
    declare void @llvm.assume(i1)
    define void @foo(i64* %p) {
    entry:
      call void @llvm.assume(i1 true) [ "align"(i64* %p, i32 64) ]
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      store i64 0, i64* %p, align 8
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");
  const auto *Store = findFirstInstruction<VPLoadStoreInst>();
  ASSERT_TRUE(Store);

  // Ensure alignment comes from the VPAssumptionCache and not from underlying
  // LLVM ValueTracking.
  VPlanValueTracking::setUseUnderlyingValues(false);

  KnownBits ArgKB =
      Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
  // TODO: uncomment once we have support for external assumptions
  // EXPECT_EQ(ArgKB.Zero, 63);
  EXPECT_EQ(ArgKB.Zero, 0);
  EXPECT_EQ(ArgKB.One, 0);
}

TEST_F(VPlanComputeKnownBitsTest, UnaffectedByInvalidInternalAssumption) {
  buildVPlanFromString(R"(
    declare i1 @cond()
    declare void @llvm.assume(i1)
    define void @foo(i64* %p) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.after ]
      %if.cond = call i1 @cond()
      br i1 %if.cond, label %if.then, label %if.else
    if.then:
      call void @llvm.assume(i1 true) [ "align"(i64* %p, i32 64) ]
      br label %if.after
    if.else:
      br label %if.after
    if.after:
      store i64 0, i64* %p, align 8
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const auto *Store = findFirstInstruction<VPLoadStoreInst>();
  ASSERT_TRUE(Store);

  // Ensure alignment comes from the VPAssumptionCache and not from underlying
  // LLVM ValueTracking.
  VPlanValueTracking::setUseUnderlyingValues(false);

  KnownBits ArgKB =
      Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
  EXPECT_EQ(ArgKB.Zero, 0);
  EXPECT_EQ(ArgKB.One, 0);
}

TEST_F(VPlanComputeKnownBitsTest, UnaffectedByInvalidExternalAssumption) {
  buildVPlanFromString(R"(
    declare i1 @cond()
    declare void @llvm.assume(i1)
    define void @foo(i64* %p) {
    entry:
      %if.cond = call i1 @cond()
      br i1 %if.cond, label %if.then, label %if.else
    if.then:
      call void @llvm.assume(i1 true) [ "align"(i64* %p, i32 64) ]
      br label %if.after
    if.else:
      br label %if.after
    if.after:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %if.after ], [ %iv.next, %for.body ]
      store i64 0, i64* %p, align 8
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const auto *Store = findFirstInstruction<VPLoadStoreInst>();
  ASSERT_TRUE(Store);

  // Ensure alignment comes from the VPAssumptionCache and not from underlying
  // LLVM ValueTracking.
  VPlanValueTracking::setUseUnderlyingValues(false);

  KnownBits ArgKB =
      Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
  EXPECT_EQ(ArgKB.Zero, 0);
  EXPECT_EQ(ArgKB.One, 0);
}
} // namespace
