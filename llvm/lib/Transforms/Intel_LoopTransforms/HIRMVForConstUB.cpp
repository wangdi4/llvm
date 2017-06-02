//===- HIRMVForConstUB.cpp - Multiversioning for constant UB -================//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements multiversioning of the loops with probable tripcount
// value.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/ForEach.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define OPT_SWITCH "hir-mv-const-ub"
#define OPT_DESCR "HIR Multiversioning for constant UB"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESCR "."));

STATISTIC(LoopsMultiversioned,
          "Number of loops multiversioned by MV for const UB");

namespace {

class HIRMVForConstUB : public HIRTransformPass {
  DDRefUtils *DRU;
  HLNodeUtils *HNU;
  BlobUtils *BU;

public:
  static char ID;

  HIRMVForConstUB() : HIRTransformPass(ID) {
    initializeHIRMVForConstUBPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFramework>();
    AU.setPreservesAll();
  }

private:
  bool analyzeAndTransformLoop(HLLoop *Loop);
  void transformLoop(HLLoop *Loop, unsigned TempIndex, int64_t Constant);

  class LoopVisitor final : public HLNodeVisitorBase {
    HIRMVForConstUB &Pass;
    HLNode *SkipNode;

  public:
    LoopVisitor(HIRMVForConstUB &Pass) : Pass(Pass), SkipNode(nullptr) {}

    void visit(HLLoop *Loop) {
      if (Pass.analyzeAndTransformLoop(Loop)) {
        SkipNode = Loop;
      }
    }

    void postVisit(const HLNode *) {}
    void visit(const HLNode *) {}

    bool skipRecursion(const HLNode *Node) const override {
      return Node == SkipNode;
    }
  };
};
}

char HIRMVForConstUB::ID = 0;
INITIALIZE_PASS_BEGIN(HIRMVForConstUB, OPT_SWITCH, OPT_DESCR, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRMVForConstUB, OPT_SWITCH, OPT_DESCR, false, false)

FunctionPass *llvm::createHIRMVForConstUBPass() {
  return new HIRMVForConstUB();
}

static void propagateConstant(HLLoop *Loop, unsigned TempIndex,
                              int64_t Constant) {
  bool LoopChanged = false;

  ForEach<RegDDRef>::visit(Loop, [&](RegDDRef *Ref) {
    if (Ref->isConstant()) {
      return;
    }

    bool SimplifyCast = Ref->isTerminalRef();

    bool Changed = false;
    for (CanonExpr *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
      if (CE->replaceTempBlobByConstant(TempIndex, Constant)) {
        CE->simplify(SimplifyCast);
        Changed = true;
      }
    }

    if (Changed) {
      Ref->makeConsistent();
      LoopChanged = true;
    }
  });

  if (LoopChanged) {
    // Reset Max TC estimation.
    Loop->setMaxTripCountEstimate(0);

    HLNodeUtils::removeRedundantNodes(Loop);
  }
}

// Cost model: handle loop if the new TC will be within [8, 32] range.
static bool isProfitable(const CanonExpr *UpperBound, int64_t Constant) {
  unsigned SrcWidth = UpperBound->getSrcType()->getPrimitiveSizeInBits();
  unsigned DstWidth = UpperBound->getSrcType()->getPrimitiveSizeInBits();

  APInt NewUpper(SrcWidth, Constant, true);
  NewUpper += UpperBound->getConstant();

  auto Denom = UpperBound->getDenominator();
  if (Denom != 1) {
    if (UpperBound->isSignedDiv()) {
      NewUpper = NewUpper.sdiv(APInt(SrcWidth, Denom, true));
    } else {
      NewUpper = NewUpper.udiv(APInt(SrcWidth, Denom, true));
    }
  }

  if (UpperBound->isSExt()) {
    NewUpper = NewUpper.sextOrTrunc(DstWidth);
  } else {
    NewUpper = NewUpper.zextOrTrunc(DstWidth);
  }

  int64_t NewTCVal = NewUpper.getSExtValue() + 1;
  return NewTCVal >= 8 && NewTCVal <= 32;
}

void HIRMVForConstUB::transformLoop(HLLoop *Loop, unsigned TempIndex,
                                    int64_t Constant) {
  RegDDRef *LHS =
      DRU->createSelfBlobRef(TempIndex, Loop->getNestingLevel() - 1);
  RegDDRef *RHS = DRU->createConstDDRef(LHS->getDestType(), Constant);

  HLIf *If = HNU->createHLIf(PredicateTy::ICMP_EQ, LHS, RHS);

  HNU->insertAfter(Loop, If);
  HNU->insertAsFirstChild(If, Loop->clone(), false);
  HNU->moveAsFirstChild(If, Loop, true);

  propagateConstant(Loop, TempIndex, Constant);

  SmallVector<const RegDDRef *, 1> Aux = {Loop->getUpperDDRef()};
  LHS->makeConsistent(&Aux);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(If);

  LoopsMultiversioned++;
}

bool HIRMVForConstUB::analyzeAndTransformLoop(HLLoop *Loop) {
  if (!Loop->isNormalized()) {
    return false;
  }

  RegDDRef *Ref = Loop->getUpperDDRef();
  CanonExpr *CE = Ref->getSingleCanonExpr();

  if (CE->hasIV() || CE->numBlobs() != 1 || Ref->numBlobDDRefs() != 1) {
    return false;
  }

  const BlobDDRef *BlobRef = *Ref->blob_cbegin();

  int64_t ConstValue;
  unsigned BlobIndex = BlobRef->getBlobIndex();

  if (BU->getTempBlobMostProbableConstValue(BlobIndex, ConstValue) &&
      isProfitable(CE, ConstValue)) {
    transformLoop(Loop, BlobIndex, ConstValue);
    return true;
  }

  return false;
}

bool HIRMVForConstUB::runOnFunction(Function &F) {
  if (DisablePass || skipFunction(F)) {
    return false;
  }

  auto &HIRF = getAnalysis<HIRFramework>();
  BU = &HIRF.getBlobUtils();
  HNU = &HIRF.getHLNodeUtils();
  DRU = &HIRF.getDDRefUtils();

  DEBUG(dbgs() << "HIRRuntimeDD for function: " << F.getName() << "\n");

  // Multiversion for most probable constant UB.
  LoopVisitor V(*this);
  HLNodeUtils::visitRange(V, HIRF.hir_begin(), HIRF.hir_end());

  return true;
}

void HIRMVForConstUB::releaseMemory() {}
