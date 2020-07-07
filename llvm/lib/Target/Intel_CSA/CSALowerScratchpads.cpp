//===- CSALowerScratchpads.cpp - Streaming operations -*- C++ -*--===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that converts memory operations to streaming
// memory loads and stores where applicable.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsCSA.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;

#define DEBUG_TYPE "csa-lower-scratchpads"
#define PASS_DESC "CSA: Lower scratchpad accesses"

namespace llvm {
void initializeCSALowerScratchpadsPass(PassRegistry &);
}

namespace {
struct CSALowerScratchpads : public ModulePass {
  static char ID;

  explicit CSALowerScratchpads() : ModulePass(ID) {
    initializeCSALowerScratchpadsPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return PASS_DESC; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override;
};
} // namespace

char CSALowerScratchpads::ID = 0;
INITIALIZE_PASS_BEGIN(CSALowerScratchpads, DEBUG_TYPE, PASS_DESC,
                      false, false)
INITIALIZE_PASS_END(CSALowerScratchpads, DEBUG_TYPE, PASS_DESC,
                    false, false)

Pass *llvm::createCSALowerScratchpadsPass() {
  return new CSALowerScratchpads();
}

static bool hasInordEdge(Instruction *MemInst) {
  if (auto II = dyn_cast<IntrinsicInst>(MemInst->getPrevNode())) {
    if (II->getIntrinsicID() == Intrinsic::csa_inord) {
      return true;
    }
  }

  return false;
}

static bool hasOutordEdge(Instruction *MemInst) {
  if (auto II = dyn_cast<IntrinsicInst>(MemInst->getNextNode())) {
    if (II->getIntrinsicID() == Intrinsic::csa_outord) {
      return true;
    }
  }

  return false;
}

bool CSALowerScratchpads::runOnModule(Module &M) {
  bool Changed = false;

  // Find all scratchpad loads/stores in the program.
  SmallVector<LoadInst *, 16> LoadInstsToRewrite;
  SmallVector<StoreInst *, 16> StoreInstsToRewrite;
  for (Function &F: M) {
    if (F.isDeclaration())
      continue;

    for (BasicBlock &BB: F) {
      for (Instruction &I: BB) {
        if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
          if (isScratchpadAddressSpace(LI->getPointerAddressSpace()))
            LoadInstsToRewrite.push_back(LI);
        } else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
          if (isScratchpadAddressSpace(SI->getPointerAddressSpace()))
            StoreInstsToRewrite.push_back(SI);
        }
      }
    }
  }

  SmallVector<Function *, 16> ScratchpadFunctions;
  auto getOrMakeFunction = [&ScratchpadFunctions, &M](unsigned AS, bool Load) {
    for (Function *F: ScratchpadFunctions) {
      if (F->getFunctionType()->getParamType(0)->getPointerAddressSpace() == AS)
        return F;
    }

    Type *ScratchpadTy = nullptr;
    for (GlobalVariable &GV : M.globals()) {
      if (GV.getAddressSpace() == AS) {
        ScratchpadTy = GV.getType()->getElementType();
        while (ScratchpadTy->isArrayTy())
          ScratchpadTy = ScratchpadTy->getArrayElementType();
        break;
      }
    }

    if (ScratchpadTy == nullptr) {
      report_fatal_error("No scratchpad storage defined");
    }
    Type *PointerTy = ScratchpadTy->getPointerTo(AS);
    Type *VoidTy = Type::getVoidTy(PointerTy->getContext());

    // Didn't find one, create it.
    std::string Name = (Twine("__csa_scratchpad_") +
        (Load ? "load_" : "store_") + Twine(AS)).str();
    Function *F = Function::Create(
      Load ? FunctionType::get(ScratchpadTy, {PointerTy}, false)
           : FunctionType::get(VoidTy, {PointerTy, ScratchpadTy}, false),
      GlobalValue::PrivateLinkage, Name, M);
    BasicBlock *BB = BasicBlock::Create(M.getContext(), "", F);
    IRBuilder<> Builder(BB);
    Value *Ord = Builder.CreateIntrinsic(Intrinsic::csa_mementry, {}, {});
    Builder.CreateIntrinsic(Intrinsic::csa_inord, {}, {Ord});
    if (Load) {
      Value *V = Builder.CreateLoad(ScratchpadTy, F->arg_begin());
      Ord = Builder.CreateIntrinsic(Intrinsic::csa_outord, {}, {});
      Builder.CreateIntrinsic(Intrinsic::csa_inord, {}, {Ord});
      Builder.CreateRet(V);
    } else {
      Builder.CreateStore(F->arg_begin() + 1, F->arg_begin());
      Ord = Builder.CreateIntrinsic(Intrinsic::csa_outord, {}, {});
      Builder.CreateIntrinsic(Intrinsic::csa_inord, {}, {Ord});
      Builder.CreateRetVoid();
    }

    ScratchpadFunctions.push_back(F);
    return F;
  };

  // For each load instruction, pull its use into a separate function.
  for (LoadInst *LI : LoadInstsToRewrite) {
    Function *CombinedFunc = getOrMakeFunction(LI->getPointerAddressSpace(), true);
    // If the input has no ordering edges, generate dummy ordering edges to
    // allow the call to be lowered.
    bool hasInorder = hasInordEdge(LI), hasOutorder = hasOutordEdge(LI);
    IRBuilder<> Builder(LI);
    if (!hasInorder)
      Builder.CreateIntrinsic(Intrinsic::csa_inord, {}, {Builder.getFalse()});
    Value *Res = Builder.CreateCall(CombinedFunc, LI->getPointerOperand(),
        LI->getName());
    if (!hasOutorder)
      Builder.CreateIntrinsic(Intrinsic::csa_outord, {}, {});
    LI->replaceAllUsesWith(Res);
    LI->eraseFromParent();
    Changed = true;
  }

  // For each store instruction, pull its use into a separate function.
  ScratchpadFunctions.clear();
  for (StoreInst *SI : StoreInstsToRewrite) {
    Function *CombinedFunc = getOrMakeFunction(SI->getPointerAddressSpace(), false);
    // If the input has no ordering edges, generate dummy ordering edges to
    // allow the call to be lowered.
    bool hasInorder = hasInordEdge(SI), hasOutorder = hasOutordEdge(SI);
    IRBuilder<> Builder(SI);
    if (!hasInorder)
      Builder.CreateIntrinsic(Intrinsic::csa_inord, {}, {Builder.getTrue()});
    Builder.CreateCall(CombinedFunc,
      {SI->getPointerOperand(), SI->getValueOperand()}, SI->getName());
    if (!hasOutorder)
      Builder.CreateIntrinsic(Intrinsic::csa_outord, {}, {});
    SI->eraseFromParent();
    Changed = true;
  }

  return Changed;
}

