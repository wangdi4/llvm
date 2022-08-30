//===- Intel_FoldLoadsToGather.cpp - Fold loads to Gather -----------------===//
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
// This Pass try to transform simulated gather to gather intrinsic:
// %E0 = extractelement <4 x i32> %offset, i64 0
// %GEP0 = getelementptr inbounds i32, i32 addrspace(1)* %base, i32 %E0
// %V0 = load i32, i32 addrspace(1)* %GEP0, align 4
// %I0 = insertelement <4 x i32> undef, i32 %V0, i32 0
// %E1 = extractelement <4 x i32> %offset, i64 1
// %GEP1 = getelementptr inbounds i32, i32 addrspace(1)* %base, i32 %E1
// %V1 = load i32, i32 addrspace(1)* %GEP1, align 4
// %I1 = insertelement <4 x i32> %I0, i32 %V1, i32 1
// %E2 = extractelement <4 x i32> %offset, i64 2
// %GEP2 = getelementptr inbounds i32, i32 addrspace(1)* %base, i32 %E2
// %V2 = load i32, i32 addrspace(1)* %GEP2, align 4
// %I2 = insertelement <4 x i32> %I1, i32 %V2, i32 2
// %E3 = extractelement <4 x i32> %offset, i64 3
// %GEP3 = getelementptr inbounds i32, i32 addrspace(1)* %base, i32 %E3
// %V3 = load i32, i32 addrspace(1)* %GEP3, align 4
// %I3 = insertelement <4 x i32> %I2, i32 %V3, i32 3
// ==>
// %GEP = getelementptr inbounds i32, i32 addrspace(1)* %base, <4 x i32> %offset
// %I3 = call <4 x i32> @llvm.masked.gather.v4i32.v4p1i32(
//                                  <4 x i32 addrspace(1)*> %GEP, i32 4,
//                                  <4 x i1> <i1 0xf>, <4 x i32> undef)
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "fold-loads-to-gather"

namespace {

class FoldLoadsToGather : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  FoldLoadsToGather() : FunctionPass(ID) {
    initializeFoldLoadsToGatherPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool runOnFunction(Function &F) override;

private:
  bool run(Instruction &I);
  TargetTransformInfo *TTI;
};

} // end anonymous namespace

char FoldLoadsToGather::ID = 0;

char &llvm::FoldLoadsToGatherID = FoldLoadsToGather::ID;

INITIALIZE_PASS(FoldLoadsToGather, DEBUG_TYPE,
                "Fold loads to gather instruction", false, false)

FunctionPass *llvm::createFoldLoadsToGatherPass() {
  return new FoldLoadsToGather();
}

static void replaceValue(Value &Old, Value &New) {
  Old.replaceAllUsesWith(&New);
  New.takeName(&Old);
}

struct LoadInfo {
  // Gather address's index.
  Value *AddrIndex;
  // Gather address's base.
  Value *AddrBase;
  // The dest type if there is zext.
  Type *ZExtDstTy;
  // The gather's index.
  uint64_t Pos;
  // The inserted base vector.
  Value *Base;
  LoadInst *Load;
  // The gather's vector elements number.
  unsigned VF;

  LoadInfo() {
    AddrIndex = nullptr;
    AddrBase = nullptr;
    ZExtDstTy = nullptr;
    Pos = 0;
    Base = nullptr;
    Load = nullptr;
    VF = 0;
  }
};

// Try to match the following pattern:
// %E1 = extractelement <4 x i32> %offset, i64 1
// %GEP1 = getelementptr inbounds i32, i32 addrspace(1)* %base, i32 %E1
// %V1 = load i32, i32 addrspace(1)* %GEP1, align 4
// %I1 = insertelement <4 x i32> %I0, i32 %V1, i32 1
static Optional<LoadInfo> findLoad(Value *I, const BasicBlock *BB) {
  LoadInfo L;

  Value *IEVal = nullptr;
  uint64_t IEIdx = 0;
  if (!match(I, m_InsertElt(m_Value(L.Base), m_Value(IEVal),
                            m_ConstantInt(IEIdx))))
    return None;

  // For the first insertelement, the base value may be 'Undef'.
  if (!isa<UndefValue>(L.Base) && !L.Base->hasOneUse())
    return None;
  L.Pos = IEIdx;

  auto Load = dyn_cast<LoadInst>(IEVal);
  if (!Load || !Load->hasOneUser() || !Load->isSimple())
    return None;
  L.Load = Load;

  auto GEP = dyn_cast<GetElementPtrInst>(Load->getOperand(0));
  if (!GEP || !GEP->hasOneUser())
    return None;

  // Only support one index currently.
  if (GEP->getNumIndices() != 1)
    return None;

  L.AddrBase = GEP->getPointerOperand();

  // Handle the zext instruction.
  Value *LastV = *GEP->idx_begin();
  if (auto ZExt = dyn_cast<ZExtInst>(LastV)) {
    L.ZExtDstTy = ZExt->getDestTy();
    if (!ZExt->hasOneUser())
      return None;
    LastV = ZExt->getOperand(0);
  }

  // Must be the same basic block.
  if (auto I = dyn_cast<Instruction>(LastV))
    if (I->getParent() != BB)
      return None;

  Value *EEVec = nullptr;
  uint64_t EEIdx = 0;
  if (!match(LastV,
             m_OneUse(m_ExtractElt(m_Value(EEVec), m_ConstantInt(EEIdx)))))
    return None;

  if (EEIdx != IEIdx)
    return None;

  auto IEVecTy = dyn_cast<FixedVectorType>(L.Base->getType());
  auto EEVecTy = dyn_cast<FixedVectorType>(EEVec->getType());

  if (!IEVecTy || !EEVecTy ||
      IEVecTy->getNumElements() != EEVecTy->getNumElements())
    return None;

  L.AddrIndex = EEVec;
  L.VF = IEVecTy->getNumElements();
  return L;
}

bool FoldLoadsToGather::run(Instruction& I) {
  SmallVector<LoadInfo, 8> Loads;

  auto BB = I.getParent();
  auto LoadOpt = findLoad(&I, BB);
  if (!LoadOpt)
    return false;

  Loads.push_back(LoadOpt.value());
  auto Load0 = Loads[0];
  if (Load0.VF != Load0.Pos + 1)
    return false;

  if (!TTI->isLegalMaskedGather(Load0.Base->getType(), Load0.Load->getAlign()))
    return false;

  for (uint64_t I = 1, E = Load0.VF; I < E; ++I) {
    auto PreGather = Loads[I - 1];
    LoadOpt = findLoad(PreGather.Base, BB);
    if (!LoadOpt)
      return false;

    auto Load = LoadOpt.value();
    if (Load0.AddrIndex != Load.AddrIndex)
      return false;
    if (Load0.AddrBase != Load.AddrBase)
      return false;
    if (Load0.ZExtDstTy != Load.ZExtDstTy)
      return false;
    if (Load.Pos != (E - I - 1))
      return false;
    if (Load0.Load->getAlign() != Load.Load->getAlign())
      return false;
    if (Load0.VF != Load.VF)
      return false;
    Loads.push_back(Load);
  }

  // VF should be >= 4 and power of 2
  if (Loads.size() < 4 || !isPowerOf2_64(Loads.size()))
    return false;

  // A quick way to check the alias between these loads.
  for (auto I = Loads[Load0.VF - 1].Load->getIterator(),
    E = Load0.Load->getIterator();
    I != E; ++I) {
    Instruction& Inst = *I;
    if (Inst.mayWriteToMemory())
      return false;
  }

  LLVM_DEBUG(
    dbgs() << "Transform:\n";
    for (int64_t I = Load0.VF - 1; I >= 0; --I)
      Loads[I].Load->dump();
    dbgs() << "To:\n";
  );

  IRBuilder<> Builder(&I);
  Value *AddrIndex = Load0.AddrIndex;
  if (Load0.ZExtDstTy) {
    auto VecDestTy = FixedVectorType::get(Load0.ZExtDstTy, Load0.VF);
    AddrIndex = Builder.CreateZExt(Load0.AddrIndex, VecDestTy);
    LLVM_DEBUG(AddrIndex->dump());
  }

  // Generate the GEP with 'AddrBase' and 'AddrIndex'.
  auto AddrBaseTy = Load0.AddrBase->getType()->getScalarType();
  auto GEPTy = AddrBaseTy->isOpaquePointerTy()
                   ? AddrBaseTy
                   : AddrBaseTy->getPointerElementType();
  auto GEP =
      Builder.CreateGEP(GEPTy, Load0.AddrBase, ArrayRef<Value *>({AddrIndex}));

  auto Gather = Builder.CreateMaskedGather(Load0.Base->getType(), GEP,
                                           Load0.Load->getAlign());

  replaceValue(I, *Gather);

  LLVM_DEBUG(
    GEP->dump();
    Gather->dump();
  );

  return true;
}

bool FoldLoadsToGather::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

  // Don't attempt vectorization if the target does not support vectors.
  if (!TTI->getNumberOfRegisters(TTI->getRegisterClassForType(/*Vector*/ true)))
    return false;

  bool MadeChange = false;
  for (BasicBlock &BB : F) {
    for (Instruction &I : make_early_inc_range(BB)) {
      if (isa<DbgInfoIntrinsic>(I))
        continue;
      MadeChange |= run(I);
    }
  }

  // We're done with transforms, so remove dead instructions.
  if (MadeChange)
    for (BasicBlock &BB : F)
      SimplifyInstructionsInBlock(&BB);

  return MadeChange;
}
