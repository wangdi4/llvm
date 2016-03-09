//===- SymbaseAssignment.cpp - Assigns symbase to ddrefs ------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Symbase assignment pass.
//
//===----------------------------------------------------------------------===//

#include <map>

#include "llvm/Pass.h"

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "symbase-assignment"

FunctionPass *llvm::createSymbaseAssignmentPass() {
  return new SymbaseAssignment();
}

namespace {
class SymbaseAssignmentVisitor final : public HLNodeVisitorBase {

private:
  SymbaseAssignment *SA;

  void addToAST(RegDDRef *Ref);
  Value *getRefPtr(RegDDRef *Ref);

public:
  AliasSetTracker AST;
  HIRParser *HIRP;
  std::map<Value *, SmallVector<DDRef *, 16>> PtrToRefs;

  SymbaseAssignmentVisitor(SymbaseAssignment *CurSA, AliasAnalysis *AA,
                           HIRParser *CurHIRP)
      : SA(CurSA), AST(*AA), HIRP(CurHIRP) {}
  void visit(HLNode *Node) {}
  void visit(HLDDNode *Node);
  void postVisit(HLNode *) {}
  void postVisit(HLDDNode *) {}
};
}

// Returns a value* for base ptr of ref
Value *SymbaseAssignmentVisitor::getRefPtr(RegDDRef *Ref) {
  if (CanonExpr *CE = Ref->getBaseCE()) {
    assert(CE->hasBlob());
    for (auto I = CE->blob_begin(), E = CE->blob_end(); I != E; ++I) {
      // Even if there are multiple ptr blobs, will AA make correct choice?
      const SCEV *Blob = BlobUtils::getBlob(I->Index);
      if (Blob->getType()->isPointerTy()) {
        const SCEVUnknown *PtrSCEV = cast<const SCEVUnknown>(Blob);
        return PtrSCEV->getValue();
      }
    }
  } else {
    assert(Ref->isTerminalRef() && "DDRef is in an inconsistent state!");
    assert(Ref->getSymbase() && "Scalar DDRef was not assigned a symbase!");
  }
  return nullptr;
}

// TODO: add special handling for memrefs with undefined base pointers.
void SymbaseAssignmentVisitor::addToAST(RegDDRef *Ref) {
  if (Ref->isTerminalRef()) {
    return;
  }

  Value *Ptr = getRefPtr(Ref);
  assert(Ptr && "Could not find Value* ptr for mem load store ref");
  DEBUG(dbgs() << "Got ptr " << *Ptr << "\n");

  PtrToRefs[Ptr].push_back(Ref);

  // TODO eventually want restrict/tbaa associated with refs
  AAMDNodes AAInfo;
  // we want loop carried disam, so use a store of unknown size
  // to simulate read/write of all mem accessed by loop
  AST.add(Ptr, MemoryLocation::UnknownSize, AAInfo);
}

void SymbaseAssignmentVisitor::visit(HLDDNode *Node) {
  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    if ((*I)->hasGEPInfo()) {
      addToAST(*I);
    }
  }
}

char SymbaseAssignment::ID = 0;

INITIALIZE_PASS_BEGIN(SymbaseAssignment, "symbase", "Symbase Assignment", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(SymbaseAssignment, "symbase", "Symbase Assignment", false,
                    true)

void SymbaseAssignment::initializeMaxSymbase() {
  MaxSymbase = HIRP->getMaxScalarSymbase();
  DEBUG(dbgs() << "Initialized max symbase to " << MaxSymbase << " \n");
}

void SymbaseAssignment::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<HIRParser>();
  AU.addRequired<AAResultsWrapperPass>();
}

bool SymbaseAssignment::runOnFunction(Function &F) {

  this->F = &F;
  auto AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  HIRP = &getAnalysis<HIRParser>();

  initializeMaxSymbase();

  SymbaseAssignmentVisitor SV(this, AA, HIRP);

  // Cannot use visitAll() here as HIRFramework pointer isn't set yet.
  HLNodeUtils::visitRange(SV, HIRP->hir_begin(), HIRP->hir_end());
  AliasSetTracker &AST = SV.AST;

  // Each ref in a set gets the same symbase
  for (auto &AliasSet : AST) {
    unsigned CurSymbase = getNewSymbase();
    DEBUG(dbgs() << "Assigned following refs to Symbase " << CurSymbase
                 << "\n");

    for (auto AV : AliasSet) {
      Value *Ptr = AV.getValue();
      auto &Refs = SV.PtrToRefs[Ptr];
      for (auto CurRef : Refs) {
        DEBUG(CurRef->dump());
        DEBUG(dbgs() << "\n");
        CurRef->setSymbase(CurSymbase);
      }
    }
  }

  return false;
}

void SymbaseAssignment::print(raw_ostream &OS, const Module *M) const {
  NonConstantRefGatherer::MapTy SymToRefs;
  NonConstantRefGatherer::gatherRange(HIRP->hir_cbegin(), HIRP->hir_cend(),
                                      SymToRefs);

  formatted_raw_ostream FOS(OS);
  FOS << "Symbase Reference Vector:";
  FOS << "\n";

  for (auto SymVecPair = SymToRefs.begin(), Last = SymToRefs.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    FOS << "Symbase ";
    FOS << SymVecPair->first;
    FOS << ":\n";
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      (*Ref)->print(FOS, true);
      FOS << "\n";
    }
  }
}
