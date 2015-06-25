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
// This file Deconstructs SSA for HIR. It inserts copies for SCC livein/liveout
// values and non-SCC phi operands. Metadata nodes are attached to the
// livein/liveout copies, to the SCC root node and non-SCC phi node. The livein
// copies are assigned the same metadata kind node as the root/phi node so that 
// they can all be assigned the same symbase by ScalarSymbaseAssignment pass. 
//
// Liveout copies require metadata to indicate to ScalarEvolution analysis to 
// not trace through them.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Metadata.h"

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
  /// \brief Attaches a string metadata node to instruction. This will be used
  /// by ScalarSymbaseAssignment to assign symbases. The metadata kind used for
  /// livein/liveout values is different because livein copies need to be
  /// assigned the same aymbase as other values in SCC whereas liveout copies
  /// don't.
  void attachMetadata(Instruction *Inst, StringRef Name, bool IsLivein) const;
  /// \brief Returns a copy of Val.
  Instruction *createCopy(Value *Val, StringRef Name, bool IsLivein) const;
  /// \brief Inserts copy of Val at the end of BB.
  void insertCopyAsLastInst(Value *Val, BasicBlock *BB, StringRef Name);
  /// \brief Inserts copy of Inst at the first insertion point of BB.
  Instruction *insertCopyAsFirstInst(Instruction *Inst, BasicBlock *BB,
                                     StringRef Name);
  /// \brief Returns the SCC this phi belongs to, if any, otherwise returns
  /// null.
  SCCFormation::SCCTy *getPhiSCC(const PHINode *Phi) const;
  /// \brief Inserts copies of Phi operands livein to the SCC.
  void processPhiLiveins(PHINode *Phi, SCCFormation::SCCNodesTy &SCCNodes,
                         StringRef Name);
  /// \brief Inserts copies of Phi if it has uses live outside the SCC and
  /// replaces the liveout uses with the copy.
  void processPhiLiveouts(PHINode *Phi, SCCFormation::SCCNodesTy SCCNodes,
                          StringRef Name);
  /// \brief Deconstructs phi by inserting copies.
  void deconstructPhi(const PHINode *Phi);
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

void SSADeconstruction::attachMetadata(Instruction *Inst, StringRef Name,
                                       bool IsLivein) const {

  Metadata *Args[] = {
      MDString::get(Inst->getContext(), Twine(Name, ".de.ssa").str())};
  MDNode *Node = MDNode::get(Inst->getContext(), Args);

  if (IsLivein) {
    Inst->setMetadata("scc.livein.de.ssa", Node);
  } else {
    Inst->setMetadata("scc.liveout.de.ssa", Node);
  }
}

Instruction *SSADeconstruction::createCopy(Value *Val, StringRef Name,
                                           bool IsLivein) const {

  auto CInst = CastInst::Create(Instruction::BitCast, Val, Val->getType(),
                                Name + ".de.ssa");

  attachMetadata(CInst, Name, IsLivein);

  return CInst;
}

void SSADeconstruction::insertCopyAsLastInst(Value *Val, BasicBlock *BB,
                                             StringRef Name) {
  // Create a copy.
  auto CopyInst = createCopy(Val, Name, true);

  // Insert at the end of BB.
  CopyInst->insertBefore(BB->getTerminator());

  // Indicate that we have modified the IR by inserting a copy.
  ModifiedIR = true;
}

Instruction *SSADeconstruction::insertCopyAsFirstInst(Instruction *Inst,
                                                      BasicBlock *BB,
                                                      StringRef Name) {
  // Create a copy.
  auto CopyInst = createCopy(Inst, Name, false);

  // Insert at the first insertion point of BB.
  CopyInst->insertBefore(BB->getFirstInsertionPt());

  // Indicate that we have modified the IR by inserting a copy.
  ModifiedIR = true;

  return CopyInst;
}

SCCFormation::SCCTy *SSADeconstruction::getPhiSCC(const PHINode *Phi) const {
  for (auto SCCIt = SCCF->begin(CurRegIt), E = SCCF->end(CurRegIt); SCCIt != E;
       ++SCCIt) {

    // Present in this SCC.
    if (((*SCCIt)->Nodes).count(Phi)) {
      return *SCCIt;
    }
  }

  return nullptr;
}

void SSADeconstruction::processPhiLiveouts(PHINode *Phi,
                                           SCCFormation::SCCNodesTy SCCNodes,
                                           StringRef Name) {

  Instruction *CopyInst = nullptr;

  for (auto UserIt = Phi->user_begin(), EndIt = Phi->user_end();
       UserIt != EndIt; ++UserIt) {
    assert(isa<Instruction>(*UserIt) && "Use is not an instruction!");

    auto UserInst = cast<Instruction>(*UserIt);

    // Ignore if this value is region live-out.
    if (!(*CurRegIt)->containsBBlock(UserInst->getParent())) {
      continue;
    }

    // Check if the use is outside SCC.
    if (SCCNodes.count(UserInst)) {
      continue;
    }

    // Insert copy, if it doesn't exist.
    if (!CopyInst) {
      CopyInst = insertCopyAsFirstInst(Phi, Phi->getParent(), Name);
    }

    // Replace liveout use by copy.
    UserIt.getUse().set(CopyInst);
  }
}

void SSADeconstruction::processPhiLiveins(PHINode *Phi,
                                          SCCFormation::SCCNodesTy &SCCNodes,
                                          StringRef Name) {

  // Insert a copy in the predecessor bblock for each phi operand which
  // lies outside the SCC(incoming values).
  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
    auto PhiOp = Phi->getIncomingValue(I);

    // Constant operand is assumed to lie outside SCC.
    if (!isa<Constant>(PhiOp) && isa<Instruction>(PhiOp) &&
        SCCNodes.count(cast<Instruction>(PhiOp))) {
      continue;
    }

    // Ignore if this value is region live-in.
    if (!(*CurRegIt)->containsBBlock(Phi->getIncomingBlock(I))) {
      continue;
    }

    // Insert copy.
    insertCopyAsLastInst(PhiOp, Phi->getIncomingBlock(I), Name);
  }
}

void SSADeconstruction::deconstructPhi(const PHINode *Phi) {
  // Phi is part of SCC
  if (auto PhiSCC = getPhiSCC(Phi)) {

    // Return if this SCC has been processed already.
    if (ProcessedSSCs.count(PhiSCC)) {
      return;
    }

    ProcessedSSCs.insert(PhiSCC);
    StringRef Name = PhiSCC->Root->getName();

    // Attach metadata to the root node to connect the SCC to its livein copies.
    attachMetadata(const_cast<Instruction *>(PhiSCC->Root), Name, true);

    for (auto const &SCCInst : PhiSCC->Nodes) {

      if (auto SCCPhiInst = dyn_cast<PHINode>(SCCInst)) {
        processPhiLiveins(const_cast<PHINode *>(SCCPhiInst), PhiSCC->Nodes,
                          Name);
        processPhiLiveouts(const_cast<PHINode *>(SCCPhiInst), PhiSCC->Nodes,
                           Name);
      }
    }
  } else {
    // This is a standalone phi such as the one which occurs at an if-else join.
    // Deconstruct all the operands.
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

    // Attach metadata to Phi to connect it to its copies.
    attachMetadata(const_cast<PHINode *>(Phi), Phi->getName(), true);

    for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
      // Insert copy.
      insertCopyAsLastInst(Phi->getIncomingValue(I), Phi->getIncomingBlock(I),
                           Phi->getName());
    }
  }
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
        if (auto Phi = dyn_cast<PHINode>(Inst)) {
          if (!SCCF->isLinear(Phi)) {
            deconstructPhi(Phi);
          }
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
