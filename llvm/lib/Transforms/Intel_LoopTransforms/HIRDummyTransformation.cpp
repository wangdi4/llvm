//===- HIRDummyTransformation.cpp - Implements Dummy Transformation class -===//
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
// This file implementas an empty HIR transformation. It can be used for
// debugging or testing purposes.
//
// Without additional options, the transformation does nothing.
//
// Available options:
//   -hir-dummy-label       inserts a label before each HLInst node
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/ForEach.h"

#define DEBUG_TYPE "hir-dummy"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    InsertLabels("hir-dummy-labels", cl::init(false), cl::Hidden,
                 cl::desc("Insert label before each instruction"));

static cl::opt<bool> MarkModified("hir-dummy-cg", cl::init(false), cl::Hidden,
                                  cl::desc("Mark all HIR regions as modified"));

static cl::opt<bool> RemoveIntelDirectives("hir-dummy-remove-intel-directives",
                                           cl::init(false), cl::Hidden,
                                           cl::desc("Remove Intel directives"));

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
    AU.addRequiredTransitive<HIRFramework>();
    AU.setPreservesAll();
  }
};

struct NodeVisitor final : public HLNodeVisitorBase {
  int Num;

  NodeVisitor() : Num(0) {}

  void insertLabel(HLInst *I) {
    HLLabel *Label =
        I->getHLNodeUtils().createHLLabel("L" + std::to_string(Num++));
    I->getHLNodeUtils().insertBefore(I, Label);
  }

  void visit(HLInst *I) {
    if (InsertLabels) {
      insertLabel(I);

      I->getParentRegion()->setGenCode(true);
    }
  }

  void visit(HLRegion *R) {
    if (MarkModified) {
      R->setGenCode(true);
    }
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
};
}

char HIRDummyTransformation::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDummyTransformation, "hir-dummy",
                      "HIR Dummy Transformation Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRDummyTransformation, "hir-dummy",
                    "HIR Dummy Transformation Pass", false, false)

FunctionPass *llvm::createHIRDummyTransformationPass() {
  return new HIRDummyTransformation();
}

bool HIRDummyTransformation::runOnFunction(Function &F) {
  DEBUG(dbgs() << "Dummy Transformation for Function : " << F.getName()
               << "\n");
  auto HIRF = &getAnalysis<HIRFramework>();
  NodeVisitor V;
  HIRF->getHLNodeUtils().visitAll(V);

  if (RemoveIntelDirectives) {
    ForEach<HLInst>::visitRange(
        HIRF->hir_begin(), HIRF->hir_end(), [](HLInst *Inst) {
          Intrinsic::ID IntrinID;

          if (!Inst->isIntrinCall(IntrinID) ||
              !vpo::VPOAnalysisUtils::isIntelDirective(IntrinID)) {
            return;
          }

          HLNodeUtils::remove(Inst);
        });
  }

  return false;
}

void HIRDummyTransformation::releaseMemory() {}
