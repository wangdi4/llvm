//===- HIRDummyTransformation.cpp - Implements Dummy Transformation class -===//
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
// This file implementas an empty HIR transformation. It can be used for
// debugging or testing purposes.
//
// Without additional options, the transformation does nothing.
//
// Available options:
//   -hir-dummy-label       inserts a label before each HLInst node
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-dummy"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    InsertLabels("hir-dummy-label", cl::init(false), cl::Hidden,
                 cl::desc("Insert label before each instruction"));

namespace {

class HIRDummyTransformation : public HIRTransformPass {
public:
  static char ID;

  HIRDummyTransformation() : HIRTransformPass(ID) {
    initializeHIRDummyTransformationPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRParser>();
    AU.addRequiredTransitive<DDAnalysis>();
    AU.setPreservesAll();
  }
};

struct NodeVisitor {
  int num;

  NodeVisitor() : num(0) {}

  void visit(HLInst *I) {
    if (InsertLabels) {
      HLLabel *Label = HLNodeUtils::createHLLabel("L" + std::to_string(num++));
      HLNodeUtils::insertBefore(I, Label);

      I->getParentRegion()->setGenCode(true);
    }
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool isDone() { return false; }
  bool skipRecursion(HLNode *Node) { return false; }
};
}

char HIRDummyTransformation::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDummyTransformation, "hir-dummy",
                      "HIR Dummy Transformation Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(HIRDummyTransformation, "hir-dummy",
                    "HIR Dummy Transformation Pass", false, false)

FunctionPass *llvm::createHIRDummyTransformationPass() {
  return new HIRDummyTransformation();
}

bool HIRDummyTransformation::runOnFunction(Function &F) {
  DEBUG(dbgs() << "Dummy Transformation for Function : " << F.getName()
               << "\n");

  NodeVisitor V;
  HLNodeUtils::visitAll<NodeVisitor>(V);

  return false;
}

void HIRDummyTransformation::releaseMemory() {}
