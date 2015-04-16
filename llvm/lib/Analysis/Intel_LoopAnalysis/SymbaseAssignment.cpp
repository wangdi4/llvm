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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"
using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "symbase-assignment"

FunctionPass *llvm::createSymbaseAssignmentPass() {
  return new SymbaseAssignment();
}

namespace {
class SymbaseAssignmentVisitor {

private:
  void visitDDNodeRefs(HLDDNode *Node);
  SymbaseAssignment *SA;

public:
  SymbaseAssignmentVisitor(SymbaseAssignment *CurSA) : SA(CurSA) {}
  void visit(HLNode* Node) {}
  void visit(HLDDNode* Node); 
  void postVisit(HLNode*) {}
  void postVisit(HLDDNode*) {}
  bool isDone() { return false; }
};
}

void SymbaseAssignmentVisitor::visit(HLDDNode *Node) {
  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; I++) {
    // TODO implement a less conservative assignment algorithm
    DEBUG((*I)->dump());
    DEBUG(dbgs() << "\n");
    // Everyone goes into the same symbase...except constants
    if (isa<ConstDDRef>(*I))
      (*I)->setSymBase(SA->getSymbaseForConstants());
    else
      (*I)->setSymBase(SA->getSymbaseForConstants() + 1);
  }
}

char SymbaseAssignment::ID = 0;

INITIALIZE_PASS_BEGIN(SymbaseAssignment,"symbase", "Symbase Assigment", false,
                                         true)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(SymbaseAssignment,"symbase", "Symbase Assigment", false,
                                         true)
void SymbaseAssignment::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<HIRCreation>();
  AU.addRequired<HIRParser>();
  AU.addRequired<AliasAnalysis>();
}
bool SymbaseAssignment::runOnFunction(Function &F) {

  this->F = &F;
  AA = &getAnalysis<AliasAnalysis>();
  auto HIR = &getAnalysis<HIRCreation>();
  SymbaseAssignmentVisitor SV(this);

  HLNodeUtils::visitAll(&SV, HIR);

  // create a visitor for each region?
  //  for (auto I = HIR->begin(), E = HIR->end(); I != E; I++) {
  //    processRegion(I);
  //  }

  return false;
}
