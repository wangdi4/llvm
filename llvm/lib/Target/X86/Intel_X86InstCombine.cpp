//===--------- Intel_X86InstCombine.cpp - X86 Instruction Combine ---------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass is mainly anti middle end optimization which is not really good
// for X86 target, at the same time, it is not a good choice to disable it
// at middle end. Just put these optimizations here.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "x86-inst-combine"

namespace {

class X86InstCombine : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  X86InstCombine() : FunctionPass(ID) {
    initializeX86InstCombinePass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool doInitialization(Module &M) override {
    auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
    if (!TPC)
      return false;

    TM = &TPC->getTM<X86TargetMachine>();
    return false;
  }
  bool runOnFunction(Function &F) override;

private:
  bool replaceOrToAdd(Instruction &I);
  bool replaceX86IntrinsicToIR(Instruction &I);
  bool replaceLibmToSVML(Instruction &I);
  bool replaceCall(Instruction &I);

  X86TargetMachine *TM = nullptr;
  const X86Subtarget *ST = nullptr;
  DominatorTree *DT = nullptr;
  TargetTransformInfo *TTI = nullptr;
  AssumptionCache *AC = nullptr;
};

} // end anonymous namespace

char X86InstCombine::ID = 0;
char &llvm::X86InstCombineID = X86InstCombine::ID;

INITIALIZE_PASS_BEGIN(X86InstCombine, DEBUG_TYPE,
                    "Instruction Combine for X86 Target", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(X86InstCombine, DEBUG_TYPE,
                    "Instruction Combine for X86 Target", false, false)

FunctionPass *llvm::createX86InstCombinePass() {
  return new X86InstCombine();
}

static void replaceValue(Instruction &Old, Value &New) {
  Old.replaceAllUsesWith(&New);
  New.takeName(&Old);
}

static bool isBitwiseInst(Value *V) {
  if (auto BinOper = dyn_cast<BinaryOperator>(V))
    if (BinOper->getOpcode() == Instruction::Or ||
        BinOper->getOpcode() == Instruction::And ||
        BinOper->getOpcode() == Instruction::Xor)
      return true;

  return false;
}

bool X86InstCombine::replaceOrToAdd(Instruction &I) {
  Value *LHS = I.getOperand(0), *RHS = I.getOperand(1);
  const DataLayout &DL = I.getModule()->getDataLayout();

  // Check if it is profitable to replace:
  // For avx512, 'Or' with other bitwise instructions may can be
  // transformed to VPTERNLOG.
  if ((ST && ST->hasAVX512()) && I.getType()->isVectorTy()) {
    for (auto User : I.users())
      if (isBitwiseInst(User))
        return false;

    for (Value *V : I.operands())
      if (isBitwiseInst(V))
          return false;
  }

  // Check if 'or' can be transformed to 'add'.
  if (!haveNoCommonBitsSet(LHS, RHS, DL, AC, &I, DT))
    return false;

  IRBuilder<> Builder(&I);
  auto Add = Builder.CreateAdd(LHS, RHS);
  replaceValue(I, *Add);

  return true;
}

static Value* replaceX86GatherToGather(IntrinsicInst* II) {

  switch (II->getIntrinsicID()) {
  case Intrinsic::x86_avx2_gather_d_d:
  case Intrinsic::x86_avx2_gather_d_d_256:
  case Intrinsic::x86_avx2_gather_d_pd:
  case Intrinsic::x86_avx2_gather_d_pd_256:
  case Intrinsic::x86_avx2_gather_d_ps:
  case Intrinsic::x86_avx2_gather_d_ps_256:
  case Intrinsic::x86_avx2_gather_d_q:
  case Intrinsic::x86_avx2_gather_d_q_256:
  case Intrinsic::x86_avx2_gather_q_d:
  case Intrinsic::x86_avx2_gather_q_d_256:
  case Intrinsic::x86_avx2_gather_q_pd:
  case Intrinsic::x86_avx2_gather_q_pd_256:
  case Intrinsic::x86_avx2_gather_q_ps:
  case Intrinsic::x86_avx2_gather_q_ps_256:
  case Intrinsic::x86_avx2_gather_q_q:
  case Intrinsic::x86_avx2_gather_q_q_256:
    break;
  default:
    return nullptr;
  }

  LLVMContext &C = II->getContext();
  Align Align;

  auto *RetTy = cast<FixedVectorType>(II->getType());

  Value *Mask = II->getOperand(3);
  auto *MaskTy = cast<FixedVectorType>(Mask->getType());

  auto *ScaleInt = cast<ConstantInt>(II->getOperand(4));
  uint64_t Scale = ScaleInt->getLimitedValue();

  Value *PassThru = II->getOperand(0);

  Value *Index = II->getOperand(2);
  auto *IndexTy = cast<FixedVectorType>(Index->getType());

  unsigned MaskNumElement = MaskTy->getNumElements();

  IRBuilder<> Builder(II);
  Value *NewMask = nullptr;

  // X86 gather check the MSB of mask, should right shift MSB to LSB for
  // matching llvm.masked.gather's definition.
  if (auto *CV = dyn_cast<ConstantVector>(Mask)) {
    SmallVector<Constant*> Masks;
    for (unsigned I = 0; I < MaskNumElement; ++I) {
      const APInt &APC = CV->getAggregateElement(I)->getUniqueInteger();
      auto *MaskElem =
          ConstantInt::getIntegerValue(Type::getInt1Ty(C), APC.getHiBits(1));
      Masks.push_back(MaskElem);
    }
    NewMask = ConstantVector::get(Masks);
  } else {
    // X86 gather's mask type is same with data type, need extra handling:
    // <8 x float> %mask
    // ==>
    // %IntMask = bitcast %mask to <8 x i32>
    // %ElemHiBit = 31
    // %ShiftV = <8 x i32> <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, ...>
    // %ShiftMask = lshr <8 x i32> %IntMask, %ShiftV
    // %NewMask = trunc <8 x i32> %ShiftMask to <8 x i1>
    unsigned ElemBits = MaskTy->getElementType()->getScalarSizeInBits();
    Value *IntMask = Builder.CreateBitCast(Mask,
        FixedVectorType::get(Type::getIntNTy(C, ElemBits), MaskNumElement));

    auto *ElemHiBit =
        ConstantInt::get(Type::getIntNTy(C, ElemBits), ElemBits - 1);
    Value *ShiftV = Builder.CreateVectorSplat(MaskNumElement, ElemHiBit);
    Value *ShiftMask = Builder.CreateLShr(IntMask, ShiftV);
    NewMask = Builder.CreateTrunc(
        ShiftMask, FixedVectorType::get(Type::getInt1Ty(C), MaskNumElement));
  }

  Value *Base = II->getOperand(1);
  auto *BaseTy = Base->getType();
  auto *NewBaseTy = Type::getIntNTy(C, Scale * 8)
                       ->getPointerTo(BaseTy->getPointerAddressSpace());
  Value *NewBase = Builder.CreateBitCast(Base, NewBaseTy);

  unsigned IndexElemCount = IndexTy->getNumElements();
  unsigned RetElemCount = RetTy->getNumElements();

  // For X86 gather, IndexTy and RetTy's minimal size is 128-bit,
  // If any of them is smaller than 128-bit, it should be extended to 128-bit by
  // increase element count, the result is IndexTy's element count is different
  // with RetTy's element count. Need shrink or extend it.
  if (IndexElemCount > RetElemCount) {
    // Example:
    // <2 x i64> @x86.gather.d.q(<2 x i64> %src, i8* @base, <4 x i32> %index)
    // Shrink %index from <4 x i32> to <2 x i32>.
    SmallVector<int> ShufMask;
    for (unsigned I = 0, E = RetElemCount; I < E; ++I)
      ShufMask.push_back(I);
    Index = Builder.CreateShuffleVector(Index, ShufMask);
    IndexTy = cast<FixedVectorType>(Index->getType());
  } else if (IndexElemCount < RetElemCount) {
    // Example:
    // <4 x i32> @x86.gather.q.d(<4 x i32> %src, i8* @base, <2 x i64> %index, <4
    // x i32> %mask)
    // Shrink mask/passthru from <4 x i32> to <2 x i32>.
    RetTy = FixedVectorType::get(RetTy->getElementType(), IndexElemCount);
    SmallVector<int> ShufMask;
    for (unsigned I = 0, E = IndexElemCount; I < E; ++I)
      ShufMask.push_back(I);
    NewMask = Builder.CreateShuffleVector(NewMask, ShufMask);
    PassThru = Builder.CreateShuffleVector(PassThru, ShufMask);
  }

  Value *GEP =
      Builder.CreateInBoundsGEP(Type::getIntNTy(C, Scale * 8), NewBase, Index);

  auto *PtrsTy = FixedVectorType::get(
      RetTy->getElementType()->getPointerTo(BaseTy->getPointerAddressSpace()),
      IndexTy->getNumElements());
  Value *Ptrs = Builder.CreateBitCast(GEP, PtrsTy);

  Value *NewGather =
      Builder.CreateMaskedGather(RetTy, Ptrs, Align, NewMask, PassThru);

  // When IndexElemCount < RetElemCount, it means we have shrink the result type,
  // need to extend it to original type.
  if (IndexElemCount < RetElemCount) {
    SmallVector<int> ShufMask;
    for (unsigned I = 0, E = IndexElemCount; I < E; ++I)
      ShufMask.push_back(I);
    for (unsigned I = IndexElemCount, E = RetElemCount; I < E; ++I)
      ShufMask.push_back(IndexElemCount);

    auto *ZeroInitializer = ConstantDataVector::getSplat(
        IndexElemCount, Constant::getNullValue(RetTy->getElementType()));
    NewGather = Builder.CreateShuffleVector(NewGather, ZeroInitializer, ShufMask);
  }

  return NewGather;
}

bool X86InstCombine::replaceCall(Instruction &I) {
  bool Changed = false;
  Changed |= replaceLibmToSVML(I);
  Changed |= replaceX86IntrinsicToIR(I);
  return Changed;
}

// SVML's performance is better than libm for some math functions.
// Try to use SVML version if possible.
bool X86InstCombine::replaceLibmToSVML(Instruction &I) {
  auto *II = dyn_cast<IntrinsicInst>(&I);
  if (!II)
    return false;

  switch (II->getIntrinsicID()) {
  case Intrinsic::log2:
  case Intrinsic::log10:
    break;
  default:
    return false;
  }

  if (!II->isFast())
    return false;

  II->addFnAttr(Attribute::get(I.getContext(), "imf-use-svml", "true"));

  return true;
}

bool X86InstCombine::replaceX86IntrinsicToIR(Instruction &I) {
  auto II = dyn_cast<IntrinsicInst>(&I);
  if (!II)
    return false;

  if (TTI->isAdvancedOptEnabled(
    TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2) &&
    (!TTI->isAdvancedOptEnabled(
    TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX512))) {
    if (auto V = replaceX86GatherToGather(II)) {
      replaceValue(I, *V);
      return true;
    }
  }

  return false;
}

bool X86InstCombine::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  if (TM)
    ST = &TM->getSubtarget<X86Subtarget>(F);
  AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);

  bool MadeChange = false;

  for (Instruction &I : make_early_inc_range(instructions(F))) {
    switch (I.getOpcode()) {
      case Instruction::Or:
        MadeChange |= replaceOrToAdd(I);
        break;
      case Instruction::Call:
        MadeChange |= replaceCall(I);
        break;
    }
  }

  // We're done with transforms, so remove dead instructions.
  if (MadeChange)
    for (BasicBlock &BB : F)
      SimplifyInstructionsInBlock(&BB);

  return MadeChange;
}

