//===----- SSADeconstruction.cpp - Deconstructs SSA for HIR -----*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file Deconstructs SSA for HIR. It performs several functions-
//
// 1) Creates a list of region livein/liveout variables.
// 2) Inserts copies for SCC live-in values and non-SCC phi operands.
// 3) Groups scalars(temps) into sets representing a single value(symbase) using
//    information from SCCFormation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"

#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SCCFormation.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#define DEBUG_TYPE "hir-de-ssa"

using namespace llvm;
using namespace llvm::loopopt;

namespace {

class SSADeconstruction : public FunctionPass {
public:
  static char ID;

  SSADeconstruction() : FunctionPass(ID), ModifiedIR(false) {
    initializeSSADeconstructionPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<RegionIdentification>();
    AU.addRequired<SCCFormation>();

    // We need to preserve all the analysis computed for HIR.
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addPreserved<ScalarEvolution>();
    AU.addPreserved<RegionIdentification>();
  }

private:
  /// \brief Returns a copy of Val.
  Instruction *createCopy(Value *Val, StringRef Name) const;
  /// \brief Inserts copy of Val at the end of BB.
  Instruction *insertCopyAsLastInst(Value *Val, BasicBlock *BB, StringRef Name);
  /// \brief Returns the SCC this phi belongs to, if any, otherwise returns
  /// null.
  SCCFormation::SCCTy *getPhiSCC(const PHINode *Phi) const;
  /// \brief Deconstructs phi by inserting copies.
  void deconstructPhi(const PHINode *Phi);
  /// \brief Returns true is this instruction has a use outside the region.
  bool isRegionLiveOut(const Instruction *Inst) const;
  /// \brief Performs SSA deconstruction on the regions.
  void deconstructSSAForRegions();

private:
  LoopInfo *LI;
  ScalarEvolution *SE;
  RegionIdentification *RI;
  SCCFormation *SCCF;

  bool ModifiedIR;
  RegionIdentification::iterator CurRegIt;
  SmallPtrSet<SCCFormation::SCCTy *, 32> ProcessedSSCs;
};
}

char SSADeconstruction::ID = 0;
INITIALIZE_PASS_BEGIN(SSADeconstruction, "hir-de-ssa", "HIR SSA Deconstruction",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(RegionIdentification)
INITIALIZE_PASS_DEPENDENCY(SCCFormation)
INITIALIZE_PASS_END(SSADeconstruction, "hir-de-ssa", "HIR SSA Deconstruction",
                    false, false)

FunctionPass *llvm::createSSADeconstructionPass() {
  return new SSADeconstruction();
}

Instruction *SSADeconstruction::createCopy(Value *Val, StringRef Name) const {
  return CastInst::Create(Instruction::BitCast, Val, Val->getType(),
                          Name + ".de.ssa");
}

Instruction *SSADeconstruction::insertCopyAsLastInst(Value *Val, BasicBlock *BB,
                                                     StringRef Name) {
  // Create a copy.
  auto CopyInst = createCopy(Val, Name);

  // Insert at the end of predecessor.
  CopyInst->insertBefore(BB->getTerminator());

  // Indicate that we have modified the IR by inserting a copy.
  ModifiedIR = true;

  return CopyInst;
}

SCCFormation::SCCTy *SSADeconstruction::getPhiSCC(const PHINode *Phi) const {
  for (auto SCCIt = SCCF->begin(CurRegIt), E = SCCF->end(CurRegIt); SCCIt != E;
       ++SCCIt) {

    // Present in this SCC.
    if ((*SCCIt)->count(Phi)) {
      return *SCCIt;
    }
  }

  return nullptr;
}

void SSADeconstruction::deconstructPhi(const PHINode *Phi) {
  unsigned Symbase;

  // Phi is part of SCC
  if (auto PhiSCC = getPhiSCC(Phi)) {

    // Return if this SCC has been processed already.
    if (ProcessedSSCs.count(PhiSCC)) {
      return;
    } else {
      // Insert phi lval as a base temp.
      Symbase = RI->insertBaseTemp(Phi);
      ProcessedSSCs.insert(PhiSCC);
    }

    bool SCCRegionLiveInProcessed = false;

    for (auto const &SCCInst : *PhiSCC) {

      RI->insertTempSymbase(SCCInst, Symbase);

      if (auto SCCPhiInst = dyn_cast<PHINode>(SCCInst)) {
        // Insert a copy in the predecessor bblock for each phi operand which
        // lies outside the SCC(incoming values) and assign it the same symbase.
        for (unsigned I = 0, E = SCCPhiInst->getNumIncomingValues(); I != E;
             ++I) {
          auto PhiOp = SCCPhiInst->getIncomingValue(I);

          // Constant operand is assumed to lie outside SCC.
          if (!isa<Constant>(PhiOp) && isa<Instruction>(PhiOp) &&
              PhiSCC->count(cast<Instruction>(PhiOp))) {
            continue;
          }

          // Mark this value as region live-in and move on.
          if (!(*CurRegIt)->containsBBlock(SCCPhiInst->getIncomingBlock(I))) {
            assert(!SCCRegionLiveInProcessed &&
                   "Multiple region live-in values found for the same SCC!");

            (*CurRegIt)->addLiveInTemp(Symbase, PhiOp);
            SCCRegionLiveInProcessed = true;
            continue;
          }

          // Insert copy and assign it the same symbase.
          auto CopyInst = insertCopyAsLastInst(
              PhiOp, SCCPhiInst->getIncomingBlock(I), SCCPhiInst->getName());
          RI->insertTempSymbase(CopyInst, Symbase);
        }
      }
    }
  } else {
    // This is a standalone phi such as the one which occurs at an if-else join.
    // Deconstruct all the operands and assign a symbase.
    //
    // Shown below is an example of a standalone phi case where output will have
    // a phi at the if-else join.
    //
    // if (cond) {
    //   output = a;
    // }
    // else {
    //   output = b;
    // }
    assert(!LI->isLoopHeader(const_cast<BasicBlock *>(Phi->getParent())) &&
           "Phi in loop header cannot be stand alone!");

    // Insert phi lval as a base temp.
    Symbase = RI->insertBaseTemp(Phi);
    RI->insertTempSymbase(Phi, Symbase);

    for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
      // Insert copy and assign it the same symbase.
      auto CopyInst = insertCopyAsLastInst(
          Phi->getIncomingValue(I), Phi->getIncomingBlock(I), Phi->getName());
      RI->insertTempSymbase(CopyInst, Symbase);
    }
  }
}

bool SSADeconstruction::isRegionLiveOut(const Instruction *Inst) const {
  // Check if the Inst is used outside the region.
  for (auto UserIt = Inst->user_begin(), EndIt = Inst->user_end();
       UserIt != EndIt; ++UserIt) {
    assert(isa<Instruction>(*UserIt) && "Use is not an instruction!");

    if ((*CurRegIt)->containsBBlock(cast<Instruction>(*UserIt)->getParent())) {
      continue;
    }

    return true;
  }

  return false;
}

void SSADeconstruction::deconstructSSAForRegions() {
  // Traverse regions.
  for (auto RegIt = RI->begin(), EndRegIt = RI->end(); RegIt != EndRegIt;
       ++RegIt) {

    // Set current region
    CurRegIt = RegIt;

    // Traverse region basic blocks.
    for (auto BBIt = (*RegIt)->bb_begin(), EndBBIt = (*RegIt)->bb_end();
         BBIt != EndBBIt; ++BBIt) {

      // Process instructions inside the basic blocks.
      for (auto Inst = (*BBIt)->begin(), EndI = (*BBIt)->end(); Inst != EndI;
           ++Inst) {
        if (isa<PHINode>(Inst) && !SCCF->isLinear(Inst)) {
          deconstructPhi(cast<PHINode>(Inst));
        }

        if (isRegionLiveOut(Inst)) {
          (*RegIt)->addLiveOutTemp(const_cast<Value *>(cast<Value>(Inst)));
        }
      }
    }
  }
}

bool SSADeconstruction::runOnFunction(Function &F) {
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolution>();
  RI = &getAnalysis<RegionIdentification>();
  SCCF = &getAnalysis<SCCFormation>();

  deconstructSSAForRegions();

  return ModifiedIR;
}
