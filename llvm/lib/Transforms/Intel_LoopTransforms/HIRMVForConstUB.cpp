//===- HIRMVForConstUB.cpp - Multiversioning for constant UB -================//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRMVForConstUB.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

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

class HIRMVForConstUB {
  HIRFramework &HIRF;
  DDRefUtils &DRU;
  BlobUtils &BU;

public:
  HIRMVForConstUB(HIRFramework &HIRF)
      : HIRF(HIRF), DRU(HIRF.getDDRefUtils()), BU(HIRF.getBlobUtils()) {}

  bool run();

private:
  bool analyzeAndTransformLoop(HLLoop *Loop);
  void transformLoop(HLLoop *Loop, unsigned TempIndex, int64_t Constant);
  void transformLoop(HLLoop *Loop, SmallVectorImpl<unsigned> &TripCounts);

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

    bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
  };
};
} // namespace

static void propagateConstant(HLLoop *Loop, unsigned TempIndex,
                              int64_t Constant) {
  bool LoopChanged = false;

  auto ReplaceBlobByConstant = [=](CanonExpr *CE, bool &Changed) {
    if (CE->replaceTempBlobByConstant(TempIndex, Constant)) {
      CE->simplify(true, true);
      Changed = true;
    }
  };

  ForEach<RegDDRef>::visit(Loop, [&](RegDDRef *Ref) {
    if (Ref->isConstant()) {
      return;
    }

    bool HasGEPInfo = Ref->hasGEPInfo();
    bool Changed = false;
    for (unsigned I = 1, NumDims = Ref->getNumDimensions(); I <= NumDims; ++I) {
      ReplaceBlobByConstant(Ref->getDimensionIndex(I), Changed);

      if (HasGEPInfo) {
        ReplaceBlobByConstant(Ref->getDimensionLower(I), Changed);
        ReplaceBlobByConstant(Ref->getDimensionStride(I), Changed);
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

void HIRMVForConstUB::transformLoop(HLLoop *Loop,
                                    SmallVectorImpl<unsigned> &TripCounts) {
  Loop->removeLoopMetadata("llvm.loop.intel.loopcount");

  // Do nothing if no valid trip count specified.
  if (std::all_of(std::begin(TripCounts), std::end(TripCounts),
                  [](unsigned TC) { return TC == 0; })) {
    return;
  }

  Loop->extractZtt();

  unsigned Level = Loop->getNestingLevel();
  RegDDRef *OrigUpperRef = Loop->getUpperDDRef();
  SmallVector<const RegDDRef *, 1> Aux = {OrigUpperRef};

  // Maintain the last If so that we can insert other versions as the else case
  // into the last If
  HLIf *LastIf = nullptr;

  for (unsigned I = 0; I < TripCounts.size(); ++I) {
    if (TripCounts[I] == 0) {
      continue;
    }

    RegDDRef *LHS = OrigUpperRef->clone();
    RegDDRef *RHS = DRU.createConstDDRef(LHS->getDestType(), TripCounts[I] - 1);

    HLIf *If =
        Loop->getHLNodeUtils().createHLIf(PredicateTy::ICMP_EQ, LHS, RHS);

    LHS->makeConsistent(Aux, Level - 1);

    if (!LastIf) {
      HLNodeUtils::insertAfter(Loop, If);
    } else {
      HLNodeUtils::insertAsFirstElseChild(LastIf, If);
    }

    HLLoop *NewLoop = Loop->clone();
    HLNodeUtils::insertAsFirstThenChild(If, NewLoop);
    NewLoop->setMaxTripCountEstimate(0);

    RegDDRef *UpperRef = NewLoop->getUpperDDRef();
    CanonExpr *CE = UpperRef->getSingleCanonExpr();
    UpperRef->clear();
    CE->setConstant(static_cast<int64_t>(TripCounts[I] - 1));

    LastIf = If;
  }

  HLNodeUtils::moveAsFirstElseChild(LastIf, Loop);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(LastIf);
  LoopsMultiversioned++;
}

void HIRMVForConstUB::transformLoop(HLLoop *Loop, unsigned TempIndex,
                                    int64_t Constant) {
  unsigned Level = Loop->getNestingLevel();

  RegDDRef *LHS = DRU.createSelfBlobRef(TempIndex, 0);
  RegDDRef *RHS = DRU.createConstDDRef(LHS->getDestType(), Constant);

  HLIf *If = Loop->getHLNodeUtils().createHLIf(PredicateTy::ICMP_EQ, LHS, RHS);

  HLNodeUtils::insertAfter(Loop, If);
  HLNodeUtils::insertAsFirstElseChild(If, Loop->clone());
  HLNodeUtils::moveAsFirstThenChild(If, Loop);

  SmallVector<const RegDDRef *, 1> Aux = {Loop->getUpperDDRef()};
  LHS->makeConsistent(Aux, Level - 1);

  propagateConstant(Loop, TempIndex, Constant);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(If);

  LoopsMultiversioned++;
}

bool HIRMVForConstUB::analyzeAndTransformLoop(HLLoop *Loop) {
  if (!Loop->isNormalized() || Loop->isUnknown()) {
    return false;
  }

  SmallVector<unsigned, 8> TripCounts;

  if (Loop->isInnermost() && Loop->getPragmaBasedLikelyTripCounts(TripCounts)) {
    transformLoop(Loop, TripCounts);
    return true;
  }

  RegDDRef *Ref = Loop->getUpperDDRef();
  CanonExpr *CE = Ref->getSingleCanonExpr();

  if (CE->hasIV() || CE->numBlobs() != 1 || Ref->numBlobDDRefs() != 1) {
    return false;
  }

  const BlobDDRef *BlobRef = *Ref->blob_begin();

  int64_t ConstValue;
  unsigned BlobIndex = BlobRef->getBlobIndex();

  if (BU.getTempBlobMostProbableConstValue(BlobIndex, ConstValue) &&
      isProfitable(CE, ConstValue)) {
    transformLoop(Loop, BlobIndex, ConstValue);
    return true;
  }

  return false;
}

bool HIRMVForConstUB::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRRuntimeDD for function: "
                    << HIRF.getFunction().getName() << "\n");

  // Multiversion for most probable constant UB.
  LoopVisitor V(*this);
  HLNodeUtils::visitRange(V, HIRF.hir_begin(), HIRF.hir_end());

  return true;
}

PreservedAnalyses HIRMVForConstUBPass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  HIRMVForConstUB(AM.getResult<HIRFrameworkAnalysis>(F)).run();
  return PreservedAnalyses::all();
}

class HIRMVForConstUBLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRMVForConstUBLegacyPass() : HIRTransformPass(ID) {
    initializeHIRMVForConstUBLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRMVForConstUB(getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIRMVForConstUBLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRMVForConstUBLegacyPass, OPT_SWITCH, OPT_DESCR, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRMVForConstUBLegacyPass, OPT_SWITCH, OPT_DESCR, false,
                    false)

FunctionPass *llvm::createHIRMVForConstUBPass() {
  return new HIRMVForConstUBLegacyPass();
}
