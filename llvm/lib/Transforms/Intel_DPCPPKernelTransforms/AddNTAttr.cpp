//===- AddNTAttr.cpp - Add NT Attr  ----------------------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddNTAttr.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

#define DEBUG_TYPE "dpcpp-kernel-add-nt-attr"

using namespace llvm;

namespace {

class AddNTAttrLegacy : public FunctionPass {
public:
  static char ID;

  AddNTAttrLegacy();

  StringRef getPassName() const override { return "AddNTAttrLegacy"; }

  bool runOnFunction(Function &Func) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  AddNTAttrPass Impl;
};

} // namespace

char AddNTAttrLegacy::ID = 0;

AddNTAttrLegacy::AddNTAttrLegacy() : FunctionPass(ID) {
  initializeAddNTAttrLegacyPass(*PassRegistry::getPassRegistry());
}

void AddNTAttrLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AAResultsWrapperPass>();
}

bool AddNTAttrLegacy::runOnFunction(Function &Func) {
  auto &AaRet = getAnalysis<AAResultsWrapperPass>().getAAResults();
  return Impl.runImpl(Func, AaRet);
}

bool AddNTAttrPass::setNTAttr(StoreInst *SI) {
  if (SI->getMetadata("nontemporal")) {
    LLVM_DEBUG(dbgs() << "Already have NT attr in " << SI->getName() << "\n");
    return false;
  }
  SmallVector<Metadata *, 1> Metas;
  LLVMContext &Context = F->getContext();
  Metas.push_back(
      ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(Context), 1)));
  MDNode *Node = MDNode::get(Context, Metas);
  SI->setMetadata(F->getParent()->getMDKindID("nontemporal"), Node);
  return true;
}

PreservedAnalyses AddNTAttrPass::run(Function &Func,
                                     FunctionAnalysisManager &FAM) {
  AAResults &AaRet = FAM.getResult<AAManager>(Func);
  if (!runImpl(Func, AaRet))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool AddNTAttrPass::runImpl(Function &Func, AAResults &AaRet) {
  F = &Func;
  if (Func.hasOptNone())
    return false;
  bool Changed = false;

  SmallVector<StoreInst *, 4> SiVec;
  SmallVector<Instruction *, 4> RiVec;
  for (auto &II : instructions(Func)) {
    StoreInst *SI = dyn_cast<StoreInst>(&II);
    if (SI)
      SiVec.push_back(SI);
    else if (II.mayReadFromMemory())
      RiVec.push_back(&II);
  }
  // A simple and strict pattern recognition for enabling NT safely,
  // If "Store" and "Read" are "mayAlias", it is not safe to enable NT
  // Traverse the function to make sure there isn't below pattern,
  //
  // No read operation on the %addr
  // %1 = store i32 val, %addr
  // No read operation on the %addr
  //
  // TODO: this recognition is strict and maybe lost the opportunity
  // so it is probably to use "Dominance" to analyze "RAW" mode, e.g.
  //
  // %0 = store i32 val, %addr
  // %1 = load %addr
  //
  // seems NT enabling is reasonable for above case, but there isn't this case
  // now.
  //
  for (StoreInst *SI : SiVec) {
    // Enable NT if no Read/Call exist
    bool MayAlias = false;
    for (Instruction *RI : RiVec) {
      // Disable NT by default if Read or Call exist
      MayAlias = true;
      // if the Call and the Store are "mayAlias"
      if (auto *Call = dyn_cast<CallBase>(RI))
        MayAlias = isModOrRefSet(AaRet.getModRefInfo(SI, Call));
      // if the Load and the Store are "mayAlias"
      else if (LoadInst *LI = dyn_cast<LoadInst>(RI)) {
        MemoryLocation Loc = MemoryLocation::get(LI);
        MayAlias = isModOrRefSet(AaRet.getModRefInfo(SI, Loc));
      } else {
        // For Read instruction which are not Load and Call,
        // Check Alias info between its operands and the Store's pointer.
        // Disable NT If operands are empty, e.g. Fence.
        for (Value *Op : RI->operands()) {
          if (!Op->getType()->isPointerTy())
            continue;
          MayAlias = !AaRet.isNoAlias(SI->getPointerOperand(), Op);
          if (MayAlias)
            break;
        }
      }
      // If Read(Call) and Store are "mayAlias", early bail out
      if (MayAlias)
        break;
    }
    if (!MayAlias)
      Changed |= setNTAttr(SI);
  }
  if (Changed)
    LLVM_DEBUG(dbgs() << "Add NT attr for " << Func.getName() << "\n");
  return Changed;
}

INITIALIZE_PASS_BEGIN(AddNTAttrLegacy, DEBUG_TYPE,
                      "add non-temporal attribute", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(AddNTAttrLegacy, DEBUG_TYPE,
                    "add non-temporal attribute", false, false)

FunctionPass *llvm::createAddNTAttrLegacyPass() { return new AddNTAttrLegacy(); }

