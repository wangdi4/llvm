//===----- HIRFramework.cpp - public interface for HIR framework ----------===//
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
// This file implements the HIRFramework pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HIRVerifier.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRScalarSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-framework"

static cl::opt<bool>
    HIRVerify("hir-verify",
              cl::desc("Verify HIR after each transformation (default=true)"),
              cl::init(true));

INITIALIZE_PASS_BEGIN(HIRFramework, "hir-framework", "HIR Framework", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_DEPENDENCY(HIRSymbaseAssignment)
INITIALIZE_PASS_DEPENDENCY(HIRScalarSymbaseAssignment)
INITIALIZE_PASS_END(HIRFramework, "hir-framework", "HIR Framework", false, true)

char HIRFramework::ID = 0;

FunctionPass *llvm::createHIRFrameworkPass() { return new HIRFramework(); }

HIRFramework::HIRFramework() : FunctionPass(ID) {
  initializeHIRFrameworkPass(*PassRegistry::getPassRegistry());
}

void HIRFramework::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRParser>();
  AU.addRequiredTransitive<HIRSymbaseAssignment>();
  AU.addRequiredTransitive<HIRScalarSymbaseAssignment>();
}

bool HIRFramework::runOnFunction(Function &F) {
  HIRP = &getAnalysis<HIRParser>();

  getHLNodeUtils().HIRF = this;
  getHLNodeUtils().removeEmptyNodesRange(hir_begin(), hir_end());
  getHLNodeUtils().initTopSortNum();

  estimateMaxTripCounts();

#ifndef NDEBUG
  verifyAnalysis();
#endif

  return false;
}

struct HIRFramework::MaxTripCountEstimator final : public HLNodeVisitorBase {
  const HIRFramework *HIRF;

  MaxTripCountEstimator(const HIRFramework *HIRF) : HIRF(HIRF) {}

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  void visit(HLLoop *Lp);
  void visit(HLDDNode *Node);

  void visit(RegDDRef *Ref, HLDDNode *Node);
  void visit(CanonExpr *CE, ArrayType *ArrTy, HLDDNode *Node);
};

void HIRFramework::MaxTripCountEstimator::visit(HLLoop *Lp) {
  // This can set trip count estimate for triangular loops.
  // DO i1 = 0, 10
  // DO i2 = 0, i1 - 1  <MAX_TC_EST = 10>
  int64_t MaxVal;

  auto UpperCE = Lp->getUpperDDRef()->getSingleCanonExpr();

  if (!UpperCE->isIntConstant() &&
      Lp->getHLNodeUtils().getMaxValue(UpperCE, Lp, MaxVal)) {
    Lp->setMaxTripCountEstimate(MaxVal + 1);
  }
}

void HIRFramework::MaxTripCountEstimator::visit(HLDDNode *Node) {

  auto ParentLoop = Node->getLexicalParentLoop();

  if (!ParentLoop) {
    return;
  }

  for (auto RefIt = Node->op_ddref_begin(), E = Node->op_ddref_end();
       RefIt != E; ++RefIt) {
    visit(*RefIt, Node);
  }
}

void HIRFramework::MaxTripCountEstimator::visit(RegDDRef *Ref, HLDDNode *Node) {
  if (!Ref->hasGEPInfo()) {
    return;
  }

  // We cannot rely on information from non-inbounds gep.
  if (!Ref->isInBounds()) {
    return;
  }

  unsigned NumDims = Ref->getNumDimensions();

  // Highest dimension is intentionally skipped as it doesn't contain
  // information about number of elements.
  for (unsigned I = 1; I < NumDims; ++I) {
    visit(Ref->getDimensionIndex(I), cast<ArrayType>(Ref->getDimensionType(I)),
          Node);
  }

  // We try getting the information about number of elements in the highest
  // dimension by tracing the base value back to an array type from which it may
  // have been extracted.
  auto HighestCE = Ref->getDimensionIndex(NumDims);

  if (HighestCE->isNonLinear() || !HighestCE->hasIV()) {
    return;
  }

  auto BaseCE = Ref->getBaseCE();

  if (!BaseCE->isSelfBlob()) {
    return;
  }

  auto BaseVal =
      BaseCE->getBlobUtils().getTempBlobValue(BaseCE->getSingleBlobIndex());

  auto ArrTy = HIRF->HIRP->traceBackToArrayType(BaseVal);

  if (ArrTy) {
    visit(HighestCE, ArrTy, Node);
  }
}

void HIRFramework::MaxTripCountEstimator::visit(CanonExpr *CE, ArrayType *ArrTy,
                                                HLDDNode *Node) {

  // We cannot estimate the iteration space of the IV with a varying blob.
  if (CE->isNonLinear() || !CE->hasIV()) {
    return;
  }

  uint64_t NumElements = ArrTy->getNumElements();

  if (!NumElements) {
    return;
  }

  auto Denom = CE->getDenominator();
  auto &HNU = Node->getHLNodeUtils();

  // The analysis is carried out based on the assumption that the range of the
  // subscript is [0, NumElements].

  for (auto Lp = Node->getParentLoop(); Lp != nullptr;
       Lp = Lp->getParentLoop()) {
    unsigned Index;
    int64_t Coeff, BlobVal = 1, NonIVVal = 0;
    uint64_t MaxTC = 0;

    if (Lp->getUpperDDRef()->getSingleCanonExpr()->isIntConstant()) {
      continue;
    }

    unsigned Level = Lp->getNodeLevel();
    CE->getIVCoeff(Level, &Index, &Coeff);

    if (!Coeff) {
      continue;
    }

    bool PositiveIVCoeff = (Coeff > 0);

    if (Index != InvalidBlobIndex) {
      if (!HNU.isKnownPositiveOrNegative(Index, Node, BlobVal)) {
        // Blob can be zero so we conservatively bail out.
        continue;
      } else if (BlobVal < 0) {
        PositiveIVCoeff = !PositiveIVCoeff;
      }
    }

    // Remove IV to calculate min/max for the remaining part.
    CE->removeIV(Level);

    // Note: Avoiding post-domination check here to save compile time. Since
    // this is just an estimate, it is okay to not be very accurate.

    // The max value of the rest of the CE gives a better estimate on max
    // trip count so we check max value first. For example, A[i + j] will
    // give a tighter bound on j for max value of i. Similarly, for A[i - j]
    // max value of i gives max estimate for j.
    if (!HNU.getMaxValue(CE, Node, NonIVVal) &&
        !HNU.getMinValue(CE, Node, NonIVVal)) {
      // This gets us the most conservative estimate.
      NonIVVal = 0;
    }

    if (NonIVVal < 0) {
      NonIVVal = 0;
    }

    // If we encounter a reference like A[5 - i], we can estimate the trip
    // count
    // to be 6.
    if (!PositiveIVCoeff && NonIVVal) {
      MaxTC = ((Denom * NonIVVal) / std::llabs(Coeff * BlobVal)) + 1;
    } else {
      MaxTC = ((Denom * (NumElements - NonIVVal - 1)) /
               std::llabs(Coeff * BlobVal)) +
              1;
    }

    // Restore CE to original state.
    CE->setIVCoeff(Level, Index, Coeff);

    auto CurMaxTC = Lp->getMaxTripCountEstimate();

    if (MaxTC && (!CurMaxTC || (MaxTC < CurMaxTC))) {
      Lp->setMaxTripCountEstimate(MaxTC);
    }
  }
}

void HIRFramework::estimateMaxTripCounts() const {
  MaxTripCountEstimator MTCE(this);
  getHLNodeUtils().visitAll(MTCE);
}

void HIRFramework::print(raw_ostream &OS, const Module *M) const {
  print(true, OS, M);
}

void HIRFramework::print(bool FrameworkDetails, raw_ostream &OS,
                         const Module *M) const {
  HIRP->print(FrameworkDetails, OS, M);
}

void HIRFramework::verifyAnalysis() const {
  if (HIRVerify) {
    HIRVerifier::verifyAll(*this);
    DEBUG(dbgs() << "Verification of HIR done"
                 << "\n");
  }
}
