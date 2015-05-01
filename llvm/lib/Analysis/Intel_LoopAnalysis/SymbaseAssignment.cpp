//===- SymbaseAssignment.cpp - Assigns symbase to ddrefs *- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// TODO License for Intel
//
//===----------------------------------------------------------------------===//
//
// This file implements the Symbase assignment pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include <map>
#include <vector>

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "symbase-assignment"

FunctionPass *llvm::createSymbaseAssignmentPass() {
  return new SymbaseAssignment();
}

namespace {
class SymbaseAssignmentVisitor {

private:
  SymbaseAssignment *SA;

  void addToAST(RegDDRef *Ref);
  Value *getRefPtr(RegDDRef *Ref);

public:
  AliasSetTracker AST;
  HIRParser *HIRP;
  std::map<Value *, std::vector<DDRef *>> PtrToRefs;

  SymbaseAssignmentVisitor(SymbaseAssignment *CurSA, AliasAnalysis *AA,
                           HIRParser *CurHIRP)
      : SA(CurSA), AST(*AA), HIRP(CurHIRP) {}
  void visit(HLNode *Node) {}
  void visit(HLDDNode *Node);
  void postVisit(HLNode *) {}
  void postVisit(HLDDNode *) {}
  bool isDone() { return false; }
};
}

// Returns a value* for base ptr of ref
Value *SymbaseAssignmentVisitor::getRefPtr(RegDDRef *Ref) {
  if (CanonExpr *CE = Ref->getBaseCE()) {
    assert(CE->hasBlob());
    for (auto I = CE->blob_cbegin(), E = CE->blob_cend(); I != E; ++I) {
      // Even if there are multiple ptr blobs, will AA make correct choice?
      const SCEV *Blob = HIRP->getBlob(I->Index);
      if (Blob->getType()->isPointerTy()) {
        const SCEVUnknown *PtrSCEV = cast<const SCEVUnknown>(Blob);
        return PtrSCEV->getValue();
      }
    }
  } else {
    // assert isScalarRef and has symbase TODO
  }
  return nullptr;
}

void SymbaseAssignmentVisitor::addToAST(RegDDRef *Ref) {
  // TODO if ref is not mem load/str return
  Value *Ptr = getRefPtr(Ref);
  assert(Ptr && "Could not find Value* ptr for mem load store ref");
  DEBUG(dbgs() << "Got ptr " << *Ptr << "\n");

  PtrToRefs[Ptr].push_back(Ref);

  // TODO eventually want restrict/tbaa associated with refs
  AAMDNodes AAInfo;
  // we want loop carried disam, so use a store of unknown size
  // to simulate read/write of all mem accessed by loop
  AST.add(Ptr, AliasAnalysis::UnknownSize, AAInfo);
}

void SymbaseAssignmentVisitor::visit(HLDDNode *Node) {
  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    if ((*I)->isConstant()) {
      (*I)->setSymBase(SA->getSymBaseForConstants());
    } else {
      addToAST(*I);
    }
  }
}

char SymbaseAssignment::ID = 0;

INITIALIZE_PASS_BEGIN(SymbaseAssignment, "symbase", "Symbase Assignment", false,
                      true)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(SymbaseAssignment, "symbase", "Symbase Assignment", false,
                    true)

void SymbaseAssignment::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<HIRParser>();
  AU.addRequired<AliasAnalysis>();
}

bool SymbaseAssignment::runOnFunction(Function &F) {

  this->F = &F;
  auto AA = &getAnalysis<AliasAnalysis>();
  auto HIRP = &getAnalysis<HIRParser>();
  SymbaseAssignmentVisitor SV(this, AA, HIRP);

  HLNodeUtils::visitAll(&SV, HIRP);
  AliasSetTracker &AST = SV.AST;

  // Each ref in a set gets the same symbase
  for (auto &AliasSet : AST) {
    int CurSymBase = getNewSymBase();
    DEBUG(dbgs() << "Assigned following refs to Symbase " << CurSymBase
                 << "\n");

    for (auto AV : AliasSet) {
      Value *Ptr = AV.getValue();
      auto &Refs = SV.PtrToRefs[Ptr];
      for (auto CurRef : Refs) {
        DEBUG(CurRef->dump());
        DEBUG(dbgs() << "\n");
        CurRef->setSymBase(CurSymBase);
      }
    }
  }

  return false;
}
