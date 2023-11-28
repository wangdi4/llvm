//===- CSAIRReductionOpt.cpp - IR level optimization pass --------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------===//
// This pass identifies additional opportunities to use
// reduction operations
// Pattern
// BB1:
// .....
// BB2:
//  %t1 = fadd double %t2, %t3
//  br label %BB3
//
// BB3:                              ; preds = BB2, BB1
//  %t4 = phi double [ %t1, BB2 ], [ %t2, BB1 ]
// TRANSFORM TO
// BB1:
// .....
// BB2:
//  br label BB3
//
// BB3:                              ; preds = BB2, BB1
//  %t5 = phi double [ %t3, BB2 ], [ 0, BB1 ]
//  %t4 = fadd double %t2, %t5
//===----------------------------------------------------------------===//

#include "CSAIROpt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <set>

using namespace llvm;

cl::opt<bool> DisableIRReductionOpt{
  "csa-disable-IR-reduction-opt", cl::Hidden,
  cl::desc("CSA Specific: disables IR level optimization to help in generating "
           "reduction operations")};

#define DEBUG_TYPE "csa-ir-opt-debug"

namespace llvm {
void initializeCSAIRReductionOptPass(PassRegistry &);
}

struct CSAIRReductionOpt : LoopPass {
  static char ID;
  CSAIRReductionOpt() : LoopPass{ID} {
    llvm::initializeCSAIRReductionOptPass(*PassRegistry::getPassRegistry());
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
  }
  bool runOnLoop(Loop *L, LPPassManager &) override;
  StringRef getPassName() const override {
    return "CSA: IR level reduction optimizations";
  }

private:
  LLVMContext Context;
};

char CSAIRReductionOpt::ID = 0;

INITIALIZE_PASS_BEGIN(CSAIRReductionOpt, DEBUG_TYPE,
                      "IR Reduction optimization", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(CSAIRReductionOpt, DEBUG_TYPE, "IR Reduction optimization",
                    false, false)

bool CSAIRReductionOpt::runOnLoop(Loop *L, LPPassManager &) {
  if (DisableIRReductionOpt)
    return false;
  // This causes isReductionPHI to crash if there is no loop preheader.
  if (!L->getLoopPreheader())
    return false;
  bool changed         = false;
  LLVMContext &context = L->getHeader()->getContext();
  SmallVector<Instruction *, 8> toBeDeleted;
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    if (Phi->getNumIncomingValues() != 2)
      continue;
    RecurrenceDescriptor RedDes;
    // UnDef values mess up isReductionPHI(...)
    if (dyn_cast<UndefValue>(Phi->getIncomingValue(0)) ||
        dyn_cast<UndefValue>(Phi->getIncomingValue(1)))
      continue;
    if (RecurrenceDescriptor::isReductionPHI(Phi, L, RedDes)) {
      Instruction *exitInst = RedDes.getLoopExitInstr();
      if (!exitInst)
        continue;
      PHINode *exitPhi = dyn_cast<PHINode>(exitInst);
      if (!exitPhi)
        continue;
      RecurrenceDescriptor::RecurrenceKind RecKind = RedDes.getRecurrenceKind();
      if (RecKind != RecurrenceDescriptor::RK_FloatAdd)
        continue;
      Value *v0   = exitPhi->getIncomingValue(0);
      Value *v1   = exitPhi->getIncomingValue(1);
      Value *Zero = ConstantFP::get(Type::getDoubleTy(context), 0);
      Value *incrVal;
      Instruction *addInst;
      if (dyn_cast<Instruction>(v0) == &I) {
        addInst = dyn_cast<Instruction>(v1);
        if (addInst->getOpcode() != Instruction::FAdd) continue;
        incrVal = (addInst->getOperand(0) == v0) ? addInst->getOperand(1)
                                                 : addInst->getOperand(0);
        exitPhi->addIncoming(Zero, exitPhi->getIncomingBlock(0));
        exitPhi->addIncoming(incrVal, exitPhi->getIncomingBlock(1));
      } else if (dyn_cast<Instruction>(v1) == &I) {
        addInst = dyn_cast<Instruction>(v0);
        if (addInst->getOpcode() != Instruction::FAdd) continue;
        incrVal = (addInst->getOperand(0) == v1) ? addInst->getOperand(1)
                                                 : addInst->getOperand(0);
        exitPhi->addIncoming(incrVal, exitPhi->getIncomingBlock(0));
        exitPhi->addIncoming(Zero, exitPhi->getIncomingBlock(1));
      } else
        continue;
      if (addInst->hasOneUse())
        toBeDeleted.push_back(addInst);
      exitPhi->removeIncomingValue((unsigned)0);
      exitPhi->removeIncomingValue((unsigned)0);
      Instruction *newAddInst = addInst->clone();
      exitInst->replaceAllUsesWith(newAddInst);
      newAddInst->setOperand(0, &I);
      newAddInst->setOperand(1, exitInst);
      newAddInst->insertBefore(exitInst->getParent()->getFirstNonPHI());
      changed = true;
    }
  }
  for (auto I : toBeDeleted)
    I->eraseFromParent();
  return changed;
}

Pass *llvm::createCSAIRReductionOptPass() { return new CSAIRReductionOpt(); }
