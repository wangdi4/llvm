//===- HIRMVForConstUB.cpp - Multiversioning for constant UB -================//
//
// Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRMVForConstUBPass.h"

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
typedef std::pair<unsigned, int64_t> BlobIdxAndNewValue;

class HIRMVForConstUB {
  HIRFramework &HIRF;
  DDRefUtils &DRU;
  BlobUtils &BU;
  DenseMap<HLLoop *, BlobIdxAndNewValue> MaxTCEstMVCandidates;

public:
  HIRMVForConstUB(HIRFramework &HIRF)
      : HIRF(HIRF), DRU(HIRF.getDDRefUtils()), BU(HIRF.getBlobUtils()) {}

  bool run();

private:
  bool analyzeAndTransformLoop(HLLoop *Loop);
  void transformLoop(HLLoop *Loop, unsigned TempIndex, int64_t Constant);
  bool transformLoopNest(HLLoop *Loop, unsigned BlobIndex, int64_t NewValue);
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
    // Reset Max TC info.
    Loop->setMaxTripCountEstimate(0);
    Loop->setLegalMaxTripCount(0);

    HLNodeUtils::removeRedundantNodes(Loop);
  }
}

// Cost model: handle loop if the new TC will be within [8, 32] range.
// New TC is calculated by replacing the UB blob of \p Blobindex with the
// most likely \p Constant value based on the phi in the IR.
static bool isProfitable(const CanonExpr *UpperBound, unsigned BlobIndex,
 int64_t Constant) {
  std::unique_ptr<CanonExpr> UBCEClone(UpperBound->clone());

  UBCEClone->replaceTempBlobByConstant(BlobIndex, Constant);
  UBCEClone->simplify(true, true);

  int64_t NewTC = 0;
  bool IsConst = UBCEClone->isIntConstant(&NewTC);
  assert(IsConst && "TC should result in Const Value");
  (void) IsConst;
  NewTC++; // normalize TC

  return NewTC >= 8 && NewTC <= 32;
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
    NewLoop->setLegalMaxTripCount(0);

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

  // Try to multivesion loopnest if one of innermost loops has small MAX_TC_EST.
  // Ex.:
  // Before optimization:
  //      DO i1 =
  //        %N = ...;
  //        DO i2 =
  //          DO i3 = 1, %N      <MAX_TC_EST = 5>
  //            ...
  //          ENDDO
  //        ENDDO
  //      ENDDO
  // After optimization:
  //      DO i1 =
  //        %N = ...;
  //        if (%N == 3) {
  //          DO i2 =
  //            DO i3 = 0, 2, 1
  //              ...
  //            ENDDO
  //          ENDDO
  //        else {
  //          DO i2 =
  //            DO i3 = 0, %N -1      <MAX_TC_EST = 5>
  //              ...
  //            ENDDO
  //          ENDDO
  //        }
  //      ENDDO
bool HIRMVForConstUB::transformLoopNest(HLLoop *OuterLoop, unsigned BlobIndex,
                                        int64_t NewValue) {
  // Create multiversioning condition.
  RegDDRef *LHS = DRU.createSelfBlobRef(BlobIndex, OuterLoop->getNestingLevel() - 1);
  RegDDRef *RHS = DRU.createConstDDRef(LHS->getDestType(), NewValue);

  HLIf *If =
      OuterLoop->getHLNodeUtils().createHLIf(PredicateTy::ICMP_EQ, LHS, RHS);

  // Insert original loop nest on the else branch and its clone on the true
  // branch of the multiversioning 'if'.
  HLNodeUtils::insertAfter(OuterLoop, If);
  auto *ThenLoop = OuterLoop->clone();
  HLNodeUtils::insertAsFirstThenChild(If, ThenLoop);
  HLNodeUtils::moveAsFirstElseChild(If, OuterLoop);

  LHS->makeConsistent();

  // Propogate new constant value of TC throught loopnest.
  propagateConstant(ThenLoop, BlobIndex, NewValue);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(If);

  LoopsMultiversioned++;
  return true;
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
      isProfitable(CE, BlobIndex, ConstValue)) {
    transformLoop(Loop, BlobIndex, ConstValue);
    return true;
  }

  // Try to multivesion loopnest if one of innermost loops has small MAX_TC_EST.
  // If innermost loop has single blob in the TC and this blob is defined at
  // some nesting level L, then do loop multivertioning on the base of the blob
  // value calculated from MAX_TC_EST.
  if (!Loop->isInnermost() || (Loop->getNestingLevel() == 1) ||
      (MaxTCEstMVCandidates.size() > 64)) {
    return false;
  }

  // Calculate on which level UB blob is defined to figure out what loop should be
  // multiversioned.
  unsigned DefAtLvl = Ref->getDefinedAtLevel();

  // Bail out for unprofitable TC or no outer loop. Initial target was TC = 3.
  uint64_t TC = Loop->getMaxTripCountEstimate();
  if ((TC < 3) || (TC > 15)) {
    LLVM_DEBUG(dbgs() << "\t Give up: unprofitable TC.\n");
    return false;
  }

  // If we already have this candidate we do not consider it again.
  HLLoop *const ParentL = Loop->getParentLoopAtLevel(DefAtLvl + 1);
  if (ParentL->hasPreheader()) {
    return false;
  }

  if (MaxTCEstMVCandidates.find(ParentL) != MaxTCEstMVCandidates.end()) {
    LLVM_DEBUG(dbgs() << "\t Give up: we already have this candidate.\n");
    return false;
  }

  // We expect that TC canon expr consists of only one blob.
  auto *TCCanonExpr = Loop->getTripCountCanonExpr();

  if (TCCanonExpr->getDenominator() != 1) {
    LLVM_DEBUG(dbgs() << "\t Give up due to denominator in TC canon expr.\n");
    return false;
  }

  int64_t Coeff = TCCanonExpr->getSingleBlobCoeff();
  BlobIndex = TCCanonExpr->getSingleBlobIndex();
  int64_t Const = TCCanonExpr->getConstant();

  // We expect TC to be in the form of C * %x + C1.
  // Also check the case of C * ext(%x) + C1.
  TCCanonExpr->setConstant(0);
  TCCanonExpr->setBlobCoeff(BlobIndex, 1);

  // Only self blob or ext of self blob is allowed.
  BlobTy InnerBlob = nullptr;
 if (!TCCanonExpr->isSelfBlob()) {
    if ((BU.isSignExtendBlob(BU.getBlob(BlobIndex), &InnerBlob) ||
         BU.isZeroExtendBlob(BU.getBlob(BlobIndex), &InnerBlob)) &&
        BU.isTempBlob(InnerBlob)) {
      BlobIndex = BU.findBlob(InnerBlob);
    } else {
      LLVM_DEBUG(dbgs() << "\t Give up due to non-self blob.\n");
      return false;
    }
  }

  // %x = (TC - C1) / C.
  if ((TC - Const) % Coeff) {
    LLVM_DEBUG(dbgs() << "\t Give up due to non-integer blob value.\n");
    // Cannot get integer value for the blob.
    return false;
  }

  int64_t NewBlobValue = (TC - Const) / Coeff;

  LLVM_DEBUG(dbgs() << "\t MAX_TC_EST candidate:  ";
             dbgs() << NewBlobValue << " = "; BU.getBlob(BlobIndex)->dump();
             ParentL->dump(););
  MaxTCEstMVCandidates.insert(
      std::make_pair(ParentL, std::make_pair(BlobIndex, NewBlobValue)));

  return false;
}

bool HIRMVForConstUB::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Multiversioning for constant UB on function "
                    << HIRF.getFunction().getName() << "\n");

  // Multiversion for most probable constant UB.
  LoopVisitor V(*this);
  HLNodeUtils::visitRange(V, HIRF.hir_begin(), HIRF.hir_end());

  for (auto &Cand : MaxTCEstMVCandidates) {
    transformLoopNest(Cand.first, Cand.second.first, Cand.second.second);
  }

  return true;
}

PreservedAnalyses HIRMVForConstUBPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIRMVForConstUB(HIRF).run();
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
