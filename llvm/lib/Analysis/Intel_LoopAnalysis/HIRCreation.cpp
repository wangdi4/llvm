//===------- HIRCreation.cpp - Creates HIR Nodes --------*- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR creation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-creation"

static RegisterPass<HIRCreation> X("hir-creation", "HIR Creation", false, true);

char HIRCreation::ID = 0;

FunctionPass *llvm::createHIRCreationPass() { return new HIRCreation(); }

HIRCreation::HIRCreation() : FunctionPass(ID) {}

void HIRCreation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTree>();
  AU.addRequiredTransitive<RegionIdentification>();
}

void HIRCreation::populateRegion(HLRegion *Region, BasicBlock *EntryBB) {

  /// TODO: Extract this code out in populateSequence() later.
  HLLabel *Label = HLNodeUtils::createHLLabel(EntryBB);
  Instruction *Terminator = EntryBB->getTerminator();

  HLNodeUtils::insertAsFirstChild(Region, Label);
  HLContainerTy::iterator insertionPos(Label);

  for (auto I = EntryBB->getFirstInsertionPt(), E = std::prev(EntryBB->end());
       I != E; I++, insertionPos++) {
    HLInst *Inst = HLNodeUtils::createHLInst(I);
    HLNodeUtils::insertAfter(insertionPos, Inst);
  }

  /// TODO: Extract this code out in populateIf() later.

  if (BranchInst *BI = dyn_cast<BranchInst>(Terminator)) {
    assert(BI->isConditional() && "Unconditional branch encountered!");

    /// Create dummy if condition for now. Later on the compare instruction
    /// operands will be substituted here and eliminated. If this is a bottom
    /// test, it will be eliminated anyway.
    HLIf *If = HLNodeUtils::createHLIf(CmpInst::Predicate::FCMP_TRUE, nullptr,
                                       nullptr);
    HLNodeUtils::insertAfter(insertionPos, If);

    /// Keep it simple for now so we don't have to invert the predicate!
    assert((Region->getOrigBBlocks().find(BI->getSuccessor(0)) !=
            Region->getOrigBBlocks().end()) &&
           "True successor must be a backedge!");

    /// HLLabel targets should be assigned in a later pass.
    HLGoto *ThenGoto = HLNodeUtils::createHLGoto(nullptr, Label);
    HLNodeUtils::insertAsFirstIfChild(If, ThenGoto);

    /// TODO: Uncomment this later. This should ideally be cleaned up during
    /// lexlink cleanup.
    ///
    /// HLGoto *ElseGoto = HLNodeUtils::createHLGoto(BI->getSuccessor(1),
    /// nullptr);
    /// insertAsFirstIfChild(If, ElseGoto, false);

  } else {
    assert(false && "Terminator is not a branch instruction!");
  }
}

void HIRCreation::create(RegionIdentification *RI) {

  for (auto &I : RI->getIRRegions()) {

    /// TODO: We need to update exit bblock after lexical link creation.
    HLRegion *Region =
        HLNodeUtils::createHLRegion(I->BasicBlocks, I->EntryBB, I->EntryBB);

    populateRegion(Region, I->EntryBB);
    Regions.push_back(Region);
  }
}

bool HIRCreation::runOnFunction(Function &F) {
  this->Func = &F;

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTree>();
  auto RI = &getAnalysis<RegionIdentification>();

  create(RI);

  return false;
}

void HIRCreation::releaseMemory() {
  Regions.clear();

  /// Destroy all HLNodes.
  HLNodeUtils::destroyAll();
}

void HIRCreation::print(raw_ostream &OS, const Module *M) const {
  /// TODO: implement later
}

void HIRCreation::verifyAnalysis() const {
  /// TODO: implement later
}
