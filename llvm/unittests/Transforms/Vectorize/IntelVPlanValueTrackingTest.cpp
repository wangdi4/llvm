//===- IntelVPlanValueTrackingTest.cpp --------------------------*- C++ -*-===//
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
    for (BasicBlock &BB : *F)
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
      "define void @foo(ptr %buf) {\n"
      "entry:\n"
      "  %0 = ptrtoint ptr %buf to i64\n"
      "  %and = and i64 %0, -64\n"
      "  %add = or i64 %and, 56\n"
      "  %1 = inttoptr i64 %add to ptr\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %i.08 = phi i64 [ 0, %entry ], [ %inc, %for.body ]\n"
      "  %add1 = add nuw nsw i64 %i.08, 6\n"
      "  %arrayidx = getelementptr inbounds i64, ptr %1, i64 %add1\n"
      "  store i64 %i.08, ptr %arrayidx, align 8\n"
      "  %inc = add nuw nsw i64 %i.08, 1\n"
      "  %exitcond = icmp eq i64 %inc, 1024\n"
      "  br i1 %exitcond, label %for.cond.cleanup, label %for.body\n"
      "for.cond.cleanup:\n"
      "  ret void\n"
      "}\n";

  KnownBits KB = getKnownBitsForStoreAddressBase(ModuleString);
  EXPECT_EQ(KB.Zero, 0b010111);
  EXPECT_EQ(KB.One, 0b101000);
}

TEST_F(VPlanValueTrackingTest, BitwiseMul) {
  const char *ModuleString =
      "define void @foo(ptr %buf, i64 %x) {\n"
      "entry:\n"
      "  %buf.asInt = ptrtoint ptr %buf to i64\n"
      "  %ptr.asInt = and i64 %buf.asInt, -1024\n" // 10 trailing zero bits.
      "  %ptr = inttoptr i64 %ptr.asInt to ptr\n"
      // %y = 2 * %x
      "  %y = mul i64 %x, 2\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]\n"
      // %offset = 72 * (2 * %x) = 16 * (9 * %x)
      "  %offset = mul i64 %y, 72\n"
      "  %add1 = add nuw nsw i64 %counter, %offset\n"
      // %arrayidx = %ptr + (16 * sizeof(i64)) * (9 * %x) + %counter
      "  %arrayidx = getelementptr inbounds i64, ptr %ptr, i64 %add1\n"
      "  store i64 %counter, ptr %arrayidx, align 8\n"
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
      "define void @foo(ptr %buf) {\n"
      "entry:\n"
      "  %ptrint = ptrtoint ptr %buf to i64\n"
      "  %maskedptr = and i64 %ptrint, 63\n"
      "  %maskcond = icmp eq i64 %maskedptr, 0\n"
      "  tail call void @llvm.assume(i1 %maskcond)\n"
      "  br label %for.body\n"
      "for.body:\n"
      "  %i.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]\n"
      "  %add = add nuw nsw i64 %i.06, 3\n"
      "  %arrayidx = getelementptr inbounds i64, ptr %buf, i64 %add\n"
      "  store i64 %i.06, ptr %arrayidx, align 8\n"
      "  %inc = add nuw nsw i64 %i.06, 1\n"
      "  %exitcond = icmp eq i64 %inc, 1024\n"
      "  br i1 %exitcond, label %for.cond.cleanup, label %for.body\n"
      "for.cond.cleanup:\n"
      "  ret void\n"
      "}\n";

  KnownBits KB = getKnownBitsForStoreAddressBase(ModuleString);
  EXPECT_EQ(KB.Zero, 0b100111);
  EXPECT_EQ(KB.One, 0b011000);
}

struct VPlanComputeKnownBitsTest : public VPlanValueTrackingTest {
  template <typename Fn>
  static void withSetUseUnderlyingValues(bool V, Fn &&F) {
    const auto OldVal = VPlanValueTracking::getUseUnderlyingValues();
    VPlanValueTracking::setUseUnderlyingValues(V);
    F();
    VPlanValueTracking::setUseUnderlyingValues(OldVal);
  }

  const VPInstruction *findInstructionByName(StringRef Name) const {
    return VPlanTestBase::findInstructionByName(*Plan.get(), Name);
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

  void expectKnownBits(const std::string &Name, const VPValue *V,
                       const VPInstruction *CtxI, uint64_t Zero, uint64_t One,
                       unsigned VF = 1) {
    KnownBits KB = Plan->getVPVT()->getKnownBits(V, CtxI, VF);
    EXPECT_EQ(KB.Zero.getSExtValue(), int64_t(Zero))
        << "KB('%" << Name << "') (VF = " << VF << ")";
    EXPECT_EQ(KB.One.getSExtValue(), int64_t(One))
        << "KB('%" << Name << "') (VF = " << VF << ")";
  }

  void expectKnownBits(const std::string &Name, uint64_t Zero, uint64_t One,
                       unsigned VF = 1) {
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";
    expectKnownBits(Name, I, I, Zero, One, VF);
  }

  void expectKnownBitsForOperand(std::string Name, unsigned Idx, uint64_t Zero,
                                 uint64_t One) {
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";
    expectKnownBits(std::move(Name) + "." + std::to_string(Idx),
                    I->getOperand(Idx), I, Zero, One);
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
      %gep = getelementptr i64, ptr null, i64 42
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
    define void @foo(ptr %p) {
    entry:
      call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 64) ]
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      store i64 0, ptr %p, align 8
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
    define void @foo(ptr %p, ptr %q) {
    entry:
      call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 64) ]
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]

      ; Test constant (and known-stride) offsets from aligned base
      %gep1 = getelementptr [256 x %base.ty], ptr %p, i64 0
      %gep2 = getelementptr [256 x %base.ty], ptr %p, i64 0, i64 %iv
      %gep3 = getelementptr [256 x %base.ty], ptr %p, i64 0, i64 %iv, i32 0
      %gep4 = getelementptr [256 x %base.ty], ptr %p, i64 0, i64 %iv, i32 0, i64 4
      %gep5 = getelementptr [256 x %base.ty], ptr %p, i64 2
      %gep6 = getelementptr [256 x %base.ty], ptr %p, i64 2, i64 %iv
      %gep7 = getelementptr [256 x %base.ty], ptr %p, i64 2, i64 %iv, i32 1
      %gep8 = getelementptr [256 x %base.ty], ptr %p, i64 2, i64 %iv, i32 1, i64 2

      ; Test that we recurse on instruction operands
      %mul.iv = mul i64 %iv, 4
      %gep9 = getelementptr [256 x %base.ty], ptr %p, i64 0, i64 %iv, i32 0, i64 %mul.iv

      ; Test that offsets from unknown base always result in unknown
      %gep10 = getelementptr [256 x %base.ty], ptr %q
      %gep11 = getelementptr [256 x %base.ty], ptr %q, i64 0
      %gep12 = getelementptr [256 x %base.ty], ptr %q, i64 %mul.iv, i64 %iv
      %gep13 = getelementptr [256 x %base.ty], ptr %q, i64 2, i64 %iv, i32 1
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
  VPlanComputeKnownBitsTest::withSetUseUnderlyingValues(false, [this]() {
    expectKnownBitsForOperand("add", /*Idx: */ 0, /*Zero: */ 0, /*One: */ 0);
  });

  // Expect that we can toggle underlying values to be used.
  VPlanComputeKnownBitsTest::withSetUseUnderlyingValues(true, [this]() {
    expectKnownBitsForOperand("add", /*Idx: */ 0, /*Zero: */ 7, /*One: */ 0);
  });
}

TEST_F(VPlanComputeKnownBitsTest, AffectedByInternalAssumption) {
  buildVPlanFromString(R"(
    declare void @llvm.assume(i1)
    define void @foo(ptr %p) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 64) ]
      store i64 0, ptr %p, align 8
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
  VPlanComputeKnownBitsTest::withSetUseUnderlyingValues(false, [&]() {
    KnownBits ArgKB =
        Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
    EXPECT_EQ(ArgKB.Zero, 63);
    EXPECT_EQ(ArgKB.One, 0);
  });
}

TEST_F(VPlanComputeKnownBitsTest, AffectedByExternalAssumption) {
  buildVPlanFromString(R"(
    declare void @llvm.assume(i1)
    define void @foo(ptr %p) {
    entry:
      call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 64) ]
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      store i64 0, ptr %p, align 8
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
  VPlanComputeKnownBitsTest::withSetUseUnderlyingValues(false, [&]() {
    KnownBits ArgKB =
        Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
    EXPECT_EQ(ArgKB.Zero, 63);
    EXPECT_EQ(ArgKB.One, 0);
  });
}

TEST_F(VPlanComputeKnownBitsTest, UnaffectedByInvalidInternalAssumption) {
  buildVPlanFromString(R"(
    declare i1 @cond()
    declare void @llvm.assume(i1)
    define void @foo(ptr %p) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.after ]
      %if.cond = call i1 @cond()
      br i1 %if.cond, label %if.then, label %if.else
    if.then:
      call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 64) ]
      br label %if.after
    if.else:
      br label %if.after
    if.after:
      store i64 0, ptr %p, align 8
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
  VPlanComputeKnownBitsTest::withSetUseUnderlyingValues(false, [&]() {
    KnownBits ArgKB =
        Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
    EXPECT_EQ(ArgKB.Zero, 0);
    EXPECT_EQ(ArgKB.One, 0);
  });
}

TEST_F(VPlanComputeKnownBitsTest, UnaffectedByInvalidExternalAssumption) {
  buildVPlanFromString(R"(
    declare i1 @cond()
    declare void @llvm.assume(i1)
    define void @foo(ptr %p) {
    entry:
      %if.cond = call i1 @cond()
      br i1 %if.cond, label %if.then, label %if.else
    if.then:
      call void @llvm.assume(i1 true) [ "align"(ptr %p, i32 64) ]
      br label %if.after
    if.else:
      br label %if.after
    if.after:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %if.after ], [ %iv.next, %for.body ]
      store i64 0, ptr %p, align 8
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
  VPlanComputeKnownBitsTest::withSetUseUnderlyingValues(false, [&]() {
    KnownBits ArgKB =
        Plan->getVPVT()->getKnownBits(Store->getPointerOperand(), Store);
    EXPECT_EQ(ArgKB.Zero, 0);
    EXPECT_EQ(ArgKB.One, 0);
  });
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_PositiveStride) {
  buildVPlanFromString(R"(
    define void @foo() {
    entry:
      br label %for.body
    for.body:
      %iv.1 = phi i64 [ 0, %entry ], [ %iv.1.next, %for.body ]
      %iv.2 = phi i64 [ 0, %entry ], [ %iv.2.next, %for.body ]
      %iv.3 = phi i64 [ 0, %entry ], [ %iv.3.next, %for.body ]
      %iv.4 = phi i64 [ 0, %entry ], [ %iv.4.next, %for.body ]
      %iv.5 = phi i64 [ 0, %entry ], [ %iv.5.next, %for.body ]
      %iv.6 = phi i64 [ 0, %entry ], [ %iv.6.next, %for.body ]
      %iv.7 = phi i64 [ 0, %entry ], [ %iv.7.next, %for.body ]
      %iv.8 = phi i64 [ 0, %entry ], [ %iv.8.next, %for.body ]
      %iv.1.next = add nuw nsw i64 %iv.1, 1
      %iv.2.next = add nuw nsw i64 %iv.2, 2
      %iv.3.next = add nuw nsw i64 %iv.3, 3
      %iv.4.next = add nuw nsw i64 %iv.4, 4
      %iv.5.next = add nuw nsw i64 %iv.5, 5
      %iv.6.next = add nuw nsw i64 %iv.6, 6
      %iv.7.next = add nuw nsw i64 %iv.7, 7
      %iv.8.next = add nuw nsw i64 %iv.8, 8
      %exitcond = icmp eq i64 %iv.1.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  for (const uint64_t Step : {1, 2, 3, 4, 5, 6, 7, 8}) {
    const auto Name = std::string("iv.") + std::to_string(Step);
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";

    for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
      const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
      EXPECT_TRUE(KB.isNonNegative());
      EXPECT_EQ(KB.countMinTrailingZeros(),
                (unsigned)llvm::countr_zero(Step * VF));
    }
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_NegativeStride) {
  buildVPlanFromString(R"(
    define void @foo() {
    entry:
      br label %for.body
    for.body:
      %iv.1 = phi i64 [ %iv.1.next, %for.body ], [ 256, %entry ]
      %iv.2 = phi i64 [ %iv.2.next, %for.body ], [ 512, %entry ]
      %iv.3 = phi i64 [ %iv.3.next, %for.body ], [ 768, %entry ]
      %iv.4 = phi i64 [ %iv.4.next, %for.body ], [ 1024, %entry ]
      %iv.5 = phi i64 [ %iv.5.next, %for.body ], [ 1280, %entry ]
      %iv.6 = phi i64 [ %iv.6.next, %for.body ], [ 1536, %entry ]
      %iv.7 = phi i64 [ %iv.7.next, %for.body ], [ 1792, %entry ]
      %iv.8 = phi i64 [ %iv.8.next, %for.body ], [ 2048, %entry ]
      %iv.1.next = add nuw nsw i64 %iv.1, -1
      %iv.2.next = add nuw nsw i64 %iv.2, -2
      %iv.3.next = add nuw nsw i64 %iv.3, -3
      %iv.4.next = add nuw nsw i64 %iv.4, -4
      %iv.5.next = add nuw nsw i64 %iv.5, -5
      %iv.6.next = add nuw nsw i64 %iv.6, -6
      %iv.7.next = add nuw nsw i64 %iv.7, -7
      %iv.8.next = add nuw nsw i64 %iv.8, -8
      %exitcond = icmp eq i64 %iv.1.next, 0
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  for (const uint64_t Step : {1, 2, 3, 4, 5, 6, 7}) {
    const auto Name = std::string("iv.") + std::to_string(Step);
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";

    for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
      const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
      EXPECT_TRUE(KB.isNonNegative());
      EXPECT_EQ(KB.countMinTrailingZeros(),
                (unsigned)llvm::countr_zero(Step * VF));
    }
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_KnownUpperLowerBound) {
  buildVPlanFromString(R"(
    define void @foo() {
    entry:
      br label %for.body
    for.body:
      %iv.1 = phi i64 [ 0, %entry ], [ %iv.1.next, %for.body ]
      %iv.2 = phi i64 [ 0, %entry ], [ %iv.2.next, %for.body ]
      %iv.7 = phi i64 [ 0, %entry ], [ %iv.7.next, %for.body ]
      %iv.1.next = add nuw nsw i64 %iv.1, 1
      %iv.2.next = add nuw nsw i64 %iv.2, 2
      %iv.7.next = add nuw nsw i64 %iv.7, 7
      %exitcond = icmp eq i64 %iv.1.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  for (const uint64_t Step : {1, 2, 7}) {
    const auto Name = std::string("iv.") + std::to_string(Step);
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '" << Name << "'";

    for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
      const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
      EXPECT_TRUE(KB.isNonNegative());
      EXPECT_EQ(KB.countMinLeadingZeros(),
                (unsigned)(llvm::countl_zero(Step * 256uLL) +
                           isPowerOf2_64(Step * 256)));
    }
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_UnknownUpperBound) {
  buildVPlanFromString(R"(
    define void @foo(i64 %UB) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, %UB
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const VPInstruction *I = findInstructionByName("iv");
  ASSERT_TRUE(I) << "No such instruction: '%iv'";

  for (const unsigned VF : {1, 2, 4, 8, 16, 32}) {
    const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
    EXPECT_TRUE(KB.isNonNegative());
    EXPECT_EQ(KB.countMinLeadingZeros(), 1u);
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_UnknownLowerBound) {
  buildVPlanFromString(R"(
    define void @foo(i64 %LB) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ %LB, %entry ], [ %iv.next, %for.body ]
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const VPInstruction *I = findInstructionByName("iv");
  ASSERT_TRUE(I) << "No such instruction: '%iv'";

  for (const unsigned VF : {1, 2, 4, 8, 16, 32}) {
    const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
    EXPECT_TRUE(KB.isUnknown());
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_NonConstLoopInvariantStep) {
  buildVPlanFromString(R"(
    define void @foo(i64 %n) {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      %n.ind.1 = phi i64 [ 0, %entry ], [ %n.ind.1.next, %for.body ]
      %n.ind.2 = phi i64 [ 0, %entry ], [ %n.ind.2.next, %for.body ]
      %n.ind.4 = phi i64 [ 0, %entry ], [ %n.ind.4.next, %for.body ]
      %n.ind.8 = phi i64 [ 0, %entry ], [ %n.ind.8.next, %for.body ]
      %n.2 = mul i64 %n, 2
      %n.4 = mul i64 %n, 4
      %n.8 = mul i64 %n, 8
      %n.ind.1.next = add nuw nsw i64 %n.ind.1, %n
      %n.ind.2.next = add nuw nsw i64 %n.ind.2, %n.2
      %n.ind.4.next = add nuw nsw i64 %n.ind.4, %n.4
      %n.ind.8.next = add nuw nsw i64 %n.ind.8, %n.8
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  using StrideAndName = std::pair<uint64_t, const char *>;
  for (const auto &[Stride, Name] : std::initializer_list<StrideAndName>{
           {1, "n.ind.1"}, {2, "n.ind.2"}, {4, "n.ind.4"}, {8, "n.ind.8"}}) {
    const VPInstruction *I = findInstructionByName(Name);
    ASSERT_TRUE(I) << "No such instruction: '%" << Name << "'";

    for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
      const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
      EXPECT_EQ(KB.countMinTrailingZeros(),
                (unsigned)llvm::countr_zero(Stride * VF));
    }
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_NonZeroStart) {
  buildVPlanFromString(R"(
    define void @foo() {
    entry:
      br label %for.body
    for.body:
      %iv.1.1 = phi i64 [ 1, %entry ], [ %iv.1.1.next, %for.body ]
      %iv.2.1 = phi i64 [ 2, %entry ], [ %iv.2.1.next, %for.body ]
      %iv.3.1 = phi i64 [ 3, %entry ], [ %iv.3.1.next, %for.body ]
      %iv.4.1 = phi i64 [ 4, %entry ], [ %iv.4.1.next, %for.body ]
      %iv.5.1 = phi i64 [ 5, %entry ], [ %iv.5.1.next, %for.body ]
      %iv.6.1 = phi i64 [ 6, %entry ], [ %iv.6.1.next, %for.body ]
      %iv.7.1 = phi i64 [ 7, %entry ], [ %iv.7.1.next, %for.body ]
      %iv.1.1.next = add nuw nsw i64 %iv.1.1, 1
      %iv.2.1.next = add nuw nsw i64 %iv.2.1, 1
      %iv.3.1.next = add nuw nsw i64 %iv.3.1, 1
      %iv.4.1.next = add nuw nsw i64 %iv.4.1, 1
      %iv.5.1.next = add nuw nsw i64 %iv.5.1, 1
      %iv.6.1.next = add nuw nsw i64 %iv.6.1, 1
      %iv.7.1.next = add nuw nsw i64 %iv.7.1, 1
      %exitcond = icmp eq i64 %iv.1.1.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const auto Expect = [this](const std::string &Name, unsigned Start,
                             uint64_t LowZero, uint64_t One, unsigned VF = 1) {
    // Mask the given zero value to add an appropriate number of leading zeros
    // for the upper bound.
    const uint64_t UpperZeroMask = -PowerOf2Ceil(255 + Start);
    expectKnownBits(Name, UpperZeroMask | LowZero, One, VF);
  };

  using StartAndName = std::pair<unsigned, const char *>;
  for (auto [Start, Name] : std::initializer_list<StartAndName>{{1, "iv.1.1"},
                                                                {2, "iv.2.1"},
                                                                {3, "iv.3.1"},
                                                                {4, "iv.4.1"},
                                                                {5, "iv.5.1"},
                                                                {6, "iv.6.1"},
                                                                {7, "iv.7.1"}})
    Expect(Name, Start, /*LowZero:*/ 0, /*One:*/ 0);

  // clang-format off
  Expect("iv.1.1", 1, /*LowZero:*/ 0b0000, /*One:*/ 0b0001, /*VF:*/  2);

  Expect("iv.2.1", 2, /*LowZero:*/ 0b0001, /*One:*/ 0b0000, /*VF:*/  2);
  Expect("iv.2.1", 2, /*LowZero:*/ 0b0101, /*One:*/ 0b0010, /*VF:*/  8);

  Expect("iv.3.1", 3, /*LowZero:*/ 0b0000, /*One:*/ 0b0001, /*VF:*/  2);
  Expect("iv.3.1", 3, /*LowZero:*/ 0b0000, /*One:*/ 0b0011, /*VF:*/  4);

  Expect("iv.4.1", 4, /*LowZero:*/ 0b0001, /*One:*/ 0b0000, /*VF:*/  2);
  Expect("iv.4.1", 4, /*LowZero:*/ 0b0011, /*One:*/ 0b0000, /*VF:*/  4);
  Expect("iv.4.1", 4, /*LowZero:*/ 0b0011, /*One:*/ 0b0100, /*VF:*/  8);

  Expect("iv.5.1", 5, /*LowZero:*/ 0b0000, /*One:*/ 0b0001, /*VF:*/  2);
  Expect("iv.5.1", 5, /*LowZero:*/ 0b0010, /*One:*/ 0b0001, /*VF:*/  4);
  Expect("iv.5.1", 5, /*LowZero:*/ 0b1010, /*One:*/ 0b0101, /*VF:*/ 16);

  Expect("iv.6.1", 6, /*LowZero:*/ 0b0001, /*One:*/ 0b0000, /*VF:*/  2);
  Expect("iv.6.1", 6, /*LowZero:*/ 0b0001, /*One:*/ 0b0010, /*VF:*/  4);
  Expect("iv.6.1", 6, /*LowZero:*/ 0b0001, /*One:*/ 0b0110, /*VF:*/  8);

  Expect("iv.7.1", 7, /*LowZero:*/ 0b0000, /*One:*/ 0b0011, /*VF:*/  4);
  Expect("iv.7.1", 7, /*LowZero:*/ 0b1000, /*One:*/ 0b0111, /*VF:*/ 16);
  // clang-format on
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_NotFirstOp) {
  buildVPlanFromString(R"(
    define void @foo() {
    entry:
      br label %for.body
    for.body:
      %iv = phi i64 [ %iv.next, %for.body ], [ 0, %entry ]
      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body
    exit:
      ret void
    }
  )");

  const VPInstruction *I = findInstructionByName("iv");
  ASSERT_TRUE(I) << "No such instruction: '%iv'";

  for (const uint64_t VF : {1, 2, 4, 8, 16}) {
    const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
    EXPECT_TRUE(KB.isNonNegative());
    EXPECT_EQ(KB.countMinTrailingZeros(), (unsigned)llvm::countr_zero(VF));
    EXPECT_EQ(KB.countMinLeadingZeros(),
              (unsigned)llvm::countl_zero(256uLL) + 1u);
  }
}

TEST_F(VPlanComputeKnownBitsTest, InductionPHI_NestedLoops) {
  buildVPlanFromString(R"(
    define void @foo(ptr %elems) {
    entry:
      br label %for.body

    for.body:
      %outer.iv = phi i64 [ 0, %entry ], [ %outer.iv.next, %for.latch ]
      %outer.gep = getelementptr i64, ptr %elems, i64 %outer.iv
      %outer.elem = load i64, ptr %outer.gep, align 8
      br label %inner.for.body

    inner.for.body:
      %inner.iv = phi i64 [ 0, %for.body ], [ %inner.iv.next, %inner.for.body ]
      %inner.iv2 = phi i64 [ 0, %for.body ], [ %inner.iv2.next, %inner.for.body ]
      %inner.iv.next = add nuw nsw i64 %inner.iv, 2
      %inner.iv2.next = add nuw nsw i64 %inner.iv2, %outer.elem
      %inner.exitcond = icmp eq i64 %inner.iv.next, 256
      br i1 %inner.exitcond, label %for.latch, label %inner.for.body

    for.latch:
      %outer.iv.next = add nuw nsw i64 %outer.iv, 1
      %exitcond = icmp eq i64 %outer.iv.next, 128
      br i1 %exitcond, label %exit, label %for.body

    exit:
      ret void
    }
  )");

  // Check all values in the context of the outer terminator, as well as their
  // own context.
  const VPInstruction *OuterTerminator =
      Plan->getMainLoop(true)->getLoopLatch()->getTerminator();

  // Test inner IV with constant step.
  const auto *InnerIV = findInstructionByName("inner.iv");
  ASSERT_TRUE(InnerIV) << "No such instruction: '%inner.iv'";
  for (const unsigned VF : {1, 2, 4, 8, 16}) {
    for (const auto *CtxI : {InnerIV, OuterTerminator}) {
      const auto KB = Plan->getVPVT()->getKnownBits(InnerIV, CtxI, VF);
      EXPECT_TRUE(KB.isNonNegative());
      EXPECT_EQ(KB.countMinTrailingZeros(), (unsigned)1);
    }
  }

  // Test inner IV with loop-invariant step.
  const auto *InnerIV2 = findInstructionByName("inner.iv2");
  ASSERT_TRUE(InnerIV2) << "No such instruction: '%inner.iv2'";
  for (const unsigned VF : {1, 2, 4, 8, 16}) {
    for (const auto *CtxI : {InnerIV2, OuterTerminator}) {
      const auto KB = Plan->getVPVT()->getKnownBits(InnerIV2, CtxI, VF);
      EXPECT_EQ(KB.countMinTrailingZeros(), (unsigned)0);
    }
  }

  // Test outer IV.
  const auto *OuterIV = findInstructionByName("outer.iv");
  ASSERT_TRUE(OuterIV) << "No such instruction: '%outer.iv'";
  for (const uint64_t VF : {1, 2, 4, 8, 16}) {
    for (const VPInstruction *CtxI : {OuterIV, InnerIV, OuterTerminator}) {
      const auto KB = Plan->getVPVT()->getKnownBits(OuterIV, CtxI, VF);
      EXPECT_TRUE(KB.isNonNegative());
      EXPECT_EQ(KB.countMinTrailingZeros(), (unsigned)llvm::countr_zero(VF));
      EXPECT_EQ(KB.countMinLeadingZeros(),
                (unsigned)llvm::countl_zero(128uLL) + 1u);
    }
  }
}

TEST_F(VPlanComputeKnownBitsTest, ReductionPHI) {
  buildVPlanFromString(R"(
    define void @foo(ptr %elems) {
    entry:
      br label %for.body

    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
      %red = phi i64 [ 0, %entry ], [ %red.next, %for.body ]

      %gep = getelementptr i64, ptr %elems, i64 %iv
      %elem = load i64, ptr %gep, align 8
      %red.next = add nuw nsw i64 %red, %elem

      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 256
      br i1 %exitcond, label %exit, label %for.body

    exit:
      ret void
    }
  )");

  const VPInstruction *I = findInstructionByName("red");
  ASSERT_TRUE(I) << "No such instruction: '%red";

  for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
    const auto KB = Plan->getVPVT()->getKnownBits(I, I, VF);
    EXPECT_TRUE(KB.isUnknown());
  }
}

TEST_F(VPlanComputeKnownBitsTest, ReductionPHI_NestedLoops) {
  buildVPlanFromString(R"(
    define void @foo(ptr %elems) {
    entry:
      br label %for.body

    for.body:
      %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.latch ]
      %outer.red = phi i64 [ 0, %entry ], [ %outer.red.next, %for.latch ]
      br label %inner.for.body

    inner.for.body:
      %inner.iv = phi i64 [ 0, %for.body ], [ %inner.iv.next, %inner.for.body ]
      %inner.red = phi i64 [ 0, %for.body ], [ %inner.red.next, %inner.for.body ]

      %inner.gep = getelementptr i64, ptr %elems, i64 %inner.iv
      %inner.elem = load i64, ptr %inner.gep, align 8
      %inner.red.next = add nuw nsw i64 %inner.red, %inner.elem

      %inner.iv.next = add nuw nsw i64 %inner.iv, 1
      %inner.exitcond = icmp eq i64 %inner.iv.next, 256
      br i1 %inner.exitcond, label %for.latch, label %inner.for.body

    for.latch:
      %outer.gep = getelementptr i64, ptr %elems, i64 %iv
      %outer.elem = load i64, ptr %outer.gep, align 8
      %outer.red.next = add nuw nsw i64 %outer.red, %outer.elem

      %iv.next = add nuw nsw i64 %iv, 1
      %exitcond = icmp eq i64 %iv.next, 128
      br i1 %exitcond, label %exit, label %for.body

    exit:
      ret void
    }
  )");

  // Check all values in the context of the outer terminator, as well as their
  // own context.
  const VPInstruction *OuterTerminator =
      Plan->getMainLoop(true)->getLoopLatch()->getTerminator();

  // Inner reduction.
  const VPInstruction *InnerRed = findInstructionByName("inner.red");
  ASSERT_TRUE(InnerRed) << "No such instruction: '%inner.red";
  for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
    for (const VPInstruction *CtxI : {InnerRed, OuterTerminator}) {
      const auto KB = Plan->getVPVT()->getKnownBits(InnerRed, CtxI, VF);
      EXPECT_TRUE(KB.isUnknown());
    }
  }

  // Outer reduction.
  const VPInstruction *OuterRed = findInstructionByName("outer.red");
  ASSERT_TRUE(OuterRed) << "No such instruction: '%outer.red";
  for (const uint64_t VF : {1, 2, 4, 8, 16, 32}) {
    for (const VPInstruction *CtxI : {InnerRed, OuterRed, OuterTerminator}) {
      const auto KB = Plan->getVPVT()->getKnownBits(OuterRed, CtxI, VF);
      EXPECT_TRUE(KB.isUnknown());
    }
  }
}

} // namespace
