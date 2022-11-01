//===----- HIRFramework.cpp - public interface for HIR framework ----------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"

#include "llvm/InitializePasses.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HIRVerifier.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"

#include "HIRCleanup.h"
#include "HIRCreation.h"
#include "HIRLoopFormation.h"
#include "HIRScalarSymbaseAssignment.h"
#include "HIRSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-framework"

static cl::opt<bool>
    HIRVerify("hir-verify",
              cl::desc("Verify HIR after each transformation (default=true)"),
              cl::init(true));

static cl::opt<bool> HIRPrinterDetails("hir-details",
                                       cl::desc("Show HIR with dd_ref details"),
                                       cl::init(false));

static cl::opt<bool>
    HIRFrameworkDetails("hir-framework-details",
                        cl::desc("Show framework detail in print"),
                        cl::init(false));

static cl::opt<bool>
    HIRPrintModified("hir-print-modified",
                     cl::desc("Show modified HIR Regions only"),
                     cl::init(false));

static cl::list<unsigned>
    HIRPrintOnly("hir-print-only", cl::desc("Show specified HIR Regions only"));

enum HIRFrameworkDebugEnum {
  P0_None = 0,
  P1_Creation,
  P2_Cleanup,
  P3_LoopFormation,
  P4_ScalarSA,
  P5_Parsing,
  P6_SA
};

static cl::opt<HIRFrameworkDebugEnum> HIRFrameworkDebugPhase(
    "hir-framework-debug", cl::init(HIRFrameworkDebugEnum::P0_None),
    cl::desc("Debug HIR Framework phase"),
    cl::values(clEnumValN(P0_None, "none", "Run in normal mode"),
               clEnumValN(P1_Creation, "creation", "Debug creation phase"),
               clEnumValN(P2_Cleanup, "cleanup", "Debug cleanup phase"),
               clEnumValN(P3_LoopFormation, "loop-formation",
                          "Debug loop formation phase"),
               clEnumValN(P4_ScalarSA, "scalar-symbase-assignment",
                          "Debug scalar symbase assignment phase"),
               clEnumValN(P5_Parsing, "parser", "Debug parsing phase"),
               clEnumValN(P6_SA, "symbase-assignment",
                          "Debug symbase assignment phase")));

AnalysisKey HIRFrameworkAnalysis::Key;

HIRFramework HIRFrameworkAnalysis::run(Function &F,
                                       FunctionAnalysisManager &AM) {
  // All the real work is done in the constructor for the HIRFramework.
  return HIRFramework(
      F, AM.getResult<DominatorTreeAnalysis>(F),
      AM.getResult<PostDominatorTreeAnalysis>(F), AM.getResult<LoopAnalysis>(F),
      AM.getResult<AAManager>(F),
      AM.getResult<HIRRegionIdentificationAnalysis>(F),
      AM.getResult<HIRSCCFormationAnalysis>(F),
      AM.getResult<OptReportOptionsAnalysis>(F).getVerbosity(),
      HIRAnalysisProvider(
          [&]() { return AM.getCachedResult<HIRDDAnalysisPass>(F); },
          [&]() { return AM.getCachedResult<HIRLoopLocalityAnalysis>(F); },
          [&]() { return AM.getCachedResult<HIRLoopResourceAnalysis>(F); },
          [&]() { return AM.getCachedResult<HIRLoopStatisticsAnalysis>(F); },
          [&]() { return AM.getCachedResult<HIRParVecAnalysisPass>(F); },
          [&]() { return AM.getCachedResult<HIRSafeReductionAnalysisPass>(F); },
          [&]() {
            return AM.getCachedResult<HIRSparseArrayReductionAnalysisPass>(F);
          },
          [&]() {
            return AM.getCachedResult<HIRArraySectionAnalysisPass>(F);
          }));
}

INITIALIZE_PASS_BEGIN(HIRFrameworkWrapperPass, "hir-framework", "HIR Framework",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)

INITIALIZE_PASS_DEPENDENCY(HIRSCCFormationWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentificationWrapperPass)

INITIALIZE_PASS_END(HIRFrameworkWrapperPass, "hir-framework", "HIR Framework",
                    false, true)

char HIRFrameworkWrapperPass::ID = 0;

FunctionPass *llvm::createHIRFrameworkWrapperPass() {
  return new HIRFrameworkWrapperPass();
}

HIRFrameworkWrapperPass::HIRFrameworkWrapperPass() : FunctionPass(ID) {
  initializeHIRFrameworkWrapperPassPass(*PassRegistry::getPassRegistry());
}

void HIRFrameworkWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();

  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<AAResultsWrapperPass>();

  AU.addRequiredTransitive<HIRRegionIdentificationWrapperPass>();
  AU.addRequiredTransitive<HIRSCCFormationWrapperPass>();

  AU.addRequiredTransitive<OptReportOptionsPass>();
}

bool HIRFrameworkWrapperPass::runOnFunction(Function &F) {
  // All the real work is done in the constructor for the HIRFramework.
  HIRF.reset(new HIRFramework(
      F, getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
      getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree(),
      getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
      getAnalysis<AAResultsWrapperPass>().getAAResults(),
      getAnalysis<HIRRegionIdentificationWrapperPass>().getRI(),
      getAnalysis<HIRSCCFormationWrapperPass>().getSCCF(),
      getAnalysis<OptReportOptionsPass>().getVerbosity(),
      HIRAnalysisProvider(
          [&]() {
            auto *Wrapper = getAnalysisIfAvailable<HIRDDAnalysisWrapperPass>();
            return Wrapper ? &Wrapper->getDDA() : nullptr;
          },
          [&]() {
            auto *Wrapper =
                getAnalysisIfAvailable<HIRLoopLocalityWrapperPass>();
            return Wrapper ? &Wrapper->getHLL() : nullptr;
          },
          [&]() {
            auto *Wrapper =
                getAnalysisIfAvailable<HIRLoopResourceWrapperPass>();
            return Wrapper ? &Wrapper->getHLR() : nullptr;
          },
          [&]() {
            auto *Wrapper =
                getAnalysisIfAvailable<HIRLoopStatisticsWrapperPass>();
            return Wrapper ? &Wrapper->getHLS() : nullptr;
          },
          [&]() {
            auto *Wrapper =
                getAnalysisIfAvailable<HIRParVecAnalysisWrapperPass>();
            return Wrapper ? &Wrapper->getHPVA() : nullptr;
          },
          [&]() {
            auto *Wrapper =
                getAnalysisIfAvailable<HIRSafeReductionAnalysisWrapperPass>();
            return Wrapper ? &Wrapper->getHSR() : nullptr;
          },
          [&]() {
            auto *Wrapper = getAnalysisIfAvailable<
                HIRSparseArrayReductionAnalysisWrapperPass>();
            return Wrapper ? &Wrapper->getHSAR() : nullptr;
          },
          [&]() {
            auto *Wrapper =
                getAnalysisIfAvailable<HIRArraySectionAnalysisWrapperPass>();
            return Wrapper ? &Wrapper->getASA() : nullptr;
          })));
  return false;
}

#if INTEL_FEATURE_SHARED_SW_ADVANCED
void HIRFramework::processDeferredZtts() {

  for (auto &LoopZttPair : PhaseLoopFormation->getDeferredZtts()) {
    auto *Lp = LoopZttPair.first;
    auto *Ztt = LoopZttPair.second;

    // Loop could have been removed if it was empty.
    // Loop could have become unknown if parsing of upper failed.
    if (!Lp->isAttached() || Lp->isUnknown()) {
      continue;
    }
    // HLNodeUtils::removeEmptyNodesRange() inverts the condiiton if the
    // children only exist in else case. If we still have children in else case,
    // it means we either have children on both sides or the inversion failed.
    // We have to give up in either case.
    if (Ztt->hasElseChildren()) {
      continue;
    }

    // The parent/child relationship sometimes gets broken by HLIf restructuring
    // during parsing.
    if (Lp->getParent() != Ztt) {
      continue;
    }

    if (!PhaseLoopFormation->setRecognizedZtt(Lp, Ztt, false)) {
      continue;
    }

    // If ztt was successfuly recognized, it moved from (LoopLevel-1) to
    // LoopLevel so we will need to update level of non-linear blobs.
    unsigned LoopLevel = Lp->getNestingLevel();

    for (auto *ZttRef :
         make_range(Lp->ztt_ddref_begin(), Lp->ztt_ddref_end())) {

      if (ZttRef->isSelfBlob()) {
        Lp->addLiveInTemp(ZttRef->getSymbase());

        if (ZttRef->isNonLinear()) {
          ZttRef->getSingleCanonExpr()->setDefinedAtLevel(LoopLevel - 1);
        }
      } else {
        bool UpdateDefLevel = false;
        for (auto *BlobRef :
             make_range(ZttRef->blob_begin(), ZttRef->blob_end())) {
          Lp->addLiveInTemp(BlobRef->getSymbase());

          if (BlobRef->isNonLinear()) {
            UpdateDefLevel = true;
            BlobRef->setDefinedAtLevel(LoopLevel - 1);
          }
        }

        if (UpdateDefLevel) {
          ZttRef->updateDefLevel(LoopLevel);
        }
      }
    }
  }
}

static void cleanupRefLowerBounds(HLRegion &Reg) {
  // The function merges lower bounds of GEP RegDDRefs into the Index part.
  //
  //  (%a)[1:i1 + 1:8] -> (%a)[0:i1:8]
  //  (%a)[1:i1:8] -> (%a)[0:i1 - 1:8]
  //
  // Merging is happening per dimension. The lower bound of particular dimension
  // will be merged only if it's possible to do for all references with the same
  // base.
  //
  // If there is a RegDDRef dimension where LB may not be merged into the Index
  // then no dimension of references with that base will be transformed.

  // Only fortran frontend may generate non-zero lower bounds.
  // if (!Reg.getHLNodeUtils().getFunction().isFortran()) {
  //   return;
  // }

  using Gatherer =
      DDRefGatherer<RegDDRef, MemRefs | IsAddressOfRefs | FakeRefs>;
  Gatherer::VectorTy Refs;
  Gatherer::gather(&Reg, Refs);

  DDRefGrouping::RefGroupVecTy<RegDDRef *> Groups;
  DDRefIndexGrouping(Groups, Refs);

  // Each group has references with the same base.
  for (auto &Group : Groups) {
    bool AnyNonZeroLB = false;

    // Store "CanMerge" result across references in the group.
    // Dimensions here are from outermost to innermost.
    SmallVector<bool, MaxLoopNestLevel> CanMergeDimension;

    // First check if it's possible to do the merge for all references.
    for (auto *Ref : Group) {
      auto NumDims = Ref->getNumDimensions();
      if (NumDims > CanMergeDimension.size()) {
        CanMergeDimension.resize(Ref->getNumDimensions(), true);
      }

      for (unsigned DimI = 1; DimI <= NumDims; ++DimI) {
        auto *LBCE = Ref->getDimensionLower(DimI);
        if (LBCE->isZero()) {
          continue;
        }

        bool &CanMerge = CanMergeDimension[NumDims - DimI];

        CanMerge =
            CanMerge && LBCE->isIntConstant() &&
            CanonExprUtils::canSubtract(Ref->getDimensionIndex(DimI), LBCE);

        AnyNonZeroLB = true;
      }
    }

    // Bail out early if it's impossible to transform any dimension of
    // references in the Group.
    if (!AnyNonZeroLB ||
        std::all_of(CanMergeDimension.begin(), CanMergeDimension.end(),
                    [](bool CanMerge) { return !CanMerge; })) {
      continue;
    }

    // Then transform references if it's possible.
    for (auto *Ref : Group) {
      unsigned NumDims = Ref->getNumDimensions();
      for (unsigned DimI = 1; DimI <= NumDims; ++DimI) {
        auto *LBCE = Ref->getDimensionLower(DimI);
        if (CanMergeDimension[NumDims - DimI] && !LBCE->isZero()) {
          CanonExprUtils::subtract(Ref->getDimensionIndex(DimI), LBCE);
          LBCE->clear();
        }
      }
    }
  }
}

// If some nodes were eliminated in cleanup phase the number of loop exits may
// have changed so we need to recompute them.
void HIRFramework::updateNumLoopExits() {
  for (auto &RegIt : make_range(hir_begin(), hir_end())) {
    auto *Reg = &cast<HLRegion>(RegIt);

    if (PhaseCleanup->isOptimizedRegion(Reg)) {
      HLNodeUtils::updateNumLoopExits(Reg);
    }
  }
}
#endif // INTEL_FEATURE_SHARED_SW_ADVANCED

void HIRFramework::runImpl() {
#if INTEL_FEATURE_SHARED_SW_ADVANCED
  // TODO: Refactor code of the framework phases to make them local objects by
  // moving persistent data structures from individual phases to the
  // HIRFramework class.
  HIRSymbaseAssignment PhaseSA(AA, *this, *PhaseParser);

  PhaseCreation->run(Regions);

  if (HIRFrameworkDebugPhase == P1_Creation) {
    return;
  }

  PhaseCleanup->run();

  if (HIRFrameworkDebugPhase == P2_Cleanup) {
    return;
  }

  PhaseLoopFormation->run();

  if (HIRFrameworkDebugPhase == P3_LoopFormation) {
    return;
  }

  PhaseScalarSA->run();

  if (HIRFrameworkDebugPhase == P4_ScalarSA) {
    LLVM_DEBUG(PhaseScalarSA->print(dbgs()));
    return;
  }

  PhaseParser->run();

  for (auto &Reg : make_range(hir_begin(), hir_end())) {
    cleanupRefLowerBounds(cast<HLRegion>(Reg));
  }

  if (HIRFrameworkDebugPhase == P5_Parsing) {
    return;
  }

  // Initialize symbase start value.
  MaxSymbase = PhaseScalarSA->getMaxScalarSymbase();
  LLVM_DEBUG(dbgs() << "Initialized max symbase to " << MaxSymbase << " \n");

  PhaseSA.run();

  if (HIRFrameworkDebugPhase == P6_SA) {
    LLVM_DEBUG(PhaseSA.print(dbgs()));
    return;
  }

  HLNodeUtils::removeEmptyNodesRange(hir_begin(), hir_end());

  processDeferredZtts();

  HNU->initTopSortNum();

  estimateMaxTripCounts();

  updateNumLoopExits();

#ifndef NDEBUG
  verify();
#endif
#endif // INTEL_FEATURE_SHARED_SW_ADVANCED
}

HIRFramework::HIRFramework(Function &F, DominatorTree &DT,
                           PostDominatorTree &PDT, LoopInfo &LI, AAResults &AA,
                           HIRRegionIdentification &RI, HIRSCCFormation &SCCF,
                           OptReportVerbosity::Level VerbosityLevel,
                           HIRAnalysisProvider AnalysisProvider)
    : Func(F), DT(DT), PDT(PDT), LI(LI), AA(AA), RI(RI), SCCF(SCCF),
      AnalysisProvider(AnalysisProvider), MaxSymbase(0) {
  HNU.reset(new HLNodeUtils(*this));
  HNU->reset(F);

  ORBuilder.setup(F.getContext(), VerbosityLevel);

  PhaseCreation.reset(new HIRCreation(DT, PDT, LI, RI, *HNU));
  PhaseCleanup.reset(new HIRCleanup(LI, *PhaseCreation, *HNU));
  PhaseLoopFormation.reset(
      new HIRLoopFormation(DT, LI, RI, *PhaseCreation, *PhaseCleanup, *HNU));
  PhaseScalarSA.reset(new HIRScalarSymbaseAssignment(
      LI, RI.getScopedSE(), RI, SCCF, *PhaseLoopFormation, *HNU));
  PhaseParser.reset(new HIRParser(DT, LI, RI, *this, *PhaseCreation,
                                  *PhaseLoopFormation, *PhaseScalarSA, *HNU));

  runImpl();
}

HIRFramework::HIRFramework(HIRFramework &&Arg)
    : Func(Arg.Func), DT(Arg.DT), PDT(Arg.PDT), LI(Arg.LI), AA(Arg.AA),
      RI(Arg.RI), SCCF(Arg.SCCF), ORBuilder(std::move(Arg.ORBuilder)),
      HNU(std::move(Arg.HNU)),
      AnalysisProvider(std::move(Arg.AnalysisProvider)),
      Regions(std::move(Arg.Regions)),
      PhaseCreation(std::move(Arg.PhaseCreation)),
      PhaseCleanup(std::move(Arg.PhaseCleanup)),
      PhaseLoopFormation(std::move(Arg.PhaseLoopFormation)),
      PhaseScalarSA(std::move(Arg.PhaseScalarSA)),
      PhaseParser(std::move(Arg.PhaseParser)), MaxSymbase(Arg.MaxSymbase) {
  // Have to update HIRFramework reference to the new location.
  HNU->HIRF = std::ref(*this);
  PhaseParser->HIRF = std::ref(*this);
}

HIRFramework::~HIRFramework() {}

struct HIRFramework::MaxTripCountEstimator final : public HLNodeVisitorBase {
  const HIRFramework *HIRF;

  MaxTripCountEstimator(const HIRFramework *HIRF) : HIRF(HIRF) {}

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  void visit(HLLoop *Lp);
  void visit(HLDDNode *Node);

  void visit(RegDDRef *Ref, HLDDNode *Node);
  void visit(CanonExpr *CE, uint64_t NumElements, HLDDNode *Node);
};

void HIRFramework::MaxTripCountEstimator::visit(HLLoop *Lp) {

  if (Lp->isUnknown()) {
    return;
  }

  // This can set trip count estimate for triangular loops.
  // DO i1 = 0, 10
  // DO i2 = 0, i1 - 1  <MAX_TC_EST = 10>
  int64_t MaxVal;

  auto UpperCE = Lp->getUpperDDRef()->getSingleCanonExpr();

  if (!UpperCE->isIntConstant() &&
      HLNodeUtils::getMaxValue(UpperCE, Lp, MaxVal)) {
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

// In C99, there is a concept of 'flexible array members' where the last array
// field of a structure is used as a variable length array. Some codebases use
// an array size of 1 as a hacky way of achieving this prior to C99. Refer to
// "The 'struct hack'" section in the link below-
//
// https://riptutorial.com/c/example/10850/flexible-array-members
//
// We want to detect such usage in this function.
//
// Returns true if \p DimNum of \p Ref possibly represents a flexible array
// member.
bool isPossibleFlexibleArrayMember(RegDDRef *Ref, unsigned DimNum,
                                   uint64_t NumElements) {

  // Restrict to lowest array dimension access with a size of 1.
  if ((DimNum != 1) || (NumElements != 1)) {
    return false;
  }

  // The Ref is guaranteed to have at least 2 dimensions because of the for
  // loop's control condition in the caller.
  // TODO: Add a check if/when trying to make this function generic.
  auto Offsets = Ref->getTrailingStructOffsets(2);

  // No struct access.
  if (Offsets.empty()) {
    return false;
  }

  // Not applicable to fortran.
  if (Ref->getHLDDNode()->getHLNodeUtils().getFunction().isFortran()) {
    return false;
  }

  // Check if the array being accessed is the last field of the structure.
  auto *HigherDimTy = Ref->getDimensionElementType(2);

  unsigned FieldNum = Offsets.back();

  Offsets = Offsets.drop_back();

  auto *StructTy =
      cast<StructType>(DDRefUtils::getOffsetType(HigherDimTy, Offsets));
  return (StructTy->getNumElements() == (FieldNum + 1));
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
    auto NumElements = Ref->getNumDimensionElements(I);
    // Disregard dimensions which can represent 'flexible array members' as we
    // don't really know the dimension info.
    // TODO: Should the framework drop dimension info for these altogether? Some
    // loopopt utilities may choke on misleading info.
    if (NumElements != 0 &&
        !isPossibleFlexibleArrayMember(Ref, I, NumElements)) {
      visit(Ref->getDimensionIndex(I), NumElements, Node);
    }
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

  auto NumElements = HIRParser::getPossibleMaxPointerDimensionSize(BaseVal);

  if (NumElements) {
    visit(HighestCE, NumElements, Node);
  }
}

static bool isInRange(int64_t Val, int64_t LowerBound, int64_t UpperBound) {
  return (Val >= LowerBound) && (Val <= UpperBound);
}

// Returns true if \p ParentLp has a negative IV relationship with any of its
// parent loops. For ex-
//
// DO i1 = 0, 10, 1
//   DO i2 = 0, -1 * i1 + 10, 1
static bool hasNegativeIVRelationship(const HLLoop *ParentLp) {

  // If the immediate parent has no IVs or blobs it cannot have a negative
  // relationship with parent loop IVs.
  if (!ParentLp->isUnknown()) {
    auto *Upper = ParentLp->getUpperCanonExpr();
    if ((Upper->getDefinedAtLevel() == 0) && !Upper->hasIV()) {
      return false;
    }
  }

  // Return true if any parent loop in the loopnest has a negative IV term in
  // the upper like (-1 * i1).
  for (auto Lp = ParentLp; Lp != nullptr; Lp = Lp->getParentLoop()) {
    if (Lp->isUnknown()) {
      continue;
    }

    auto *Upper = Lp->getUpperCanonExpr();

    for (auto IVTerm : make_range(Upper->iv_begin(), Upper->iv_end())) {
      if (IVTerm.Coeff < 0) {
        return true;
      }
    }
  }

  return false;
}

void HIRFramework::MaxTripCountEstimator::visit(CanonExpr *CE,
                                                uint64_t NumElements,
                                                HLDDNode *Node) {

  // We cannot estimate the iteration space of the IV with a varying blob.
  if (CE->isNonLinear() || !CE->hasIV()) {
    return;
  }

  auto Denom = CE->getDenominator();

  // The analysis is carried out based on the assumption that the range of the
  // subscript is [0, NumElements].

  auto *ParentLp = Node->getParentLoop();
  for (auto Lp = ParentLp; Lp != nullptr; Lp = Lp->getParentLoop()) {
    unsigned Index;
    int64_t Coeff, BlobVal = 1;
    int64_t NonIVVal = 0, MinNonIVVal = 0, MaxNonIVVal = 0;
    uint64_t MaxTC = 0;

    if (!Lp->isUnknown() && Lp->getUpperCanonExpr()->isIntConstant()) {
      continue;
    }

    unsigned Level = Lp->getNestingLevel();
    CE->getIVCoeff(Level, &Index, &Coeff);

    if (!Coeff) {
      continue;
    }

    bool PositiveIVCoeff = (Coeff > 0);

    if (Index != InvalidBlobIndex) {
      if (!HLNodeUtils::isKnownPositiveOrNegative(Index, Node, BlobVal)) {
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

    // We compute the min/max value of the rest of the CE and use it like this-
    //
    // - If we have max value and there is a negative relationship between the
    // IVs, we take the average. This prevents us from using a very aggressive
    // bound for j using A[i + j] in cases like this-
    //
    // for (i=0; i < 10; i++)
    //   for (j=10-i; j < 10; j++)
    //     A[i + j] =
    //
    // - Else if we have max value, we use it because it gives a tigher bound on
    // current IV.
    // - Else if we have min value, we use it.
    // - Otherwise, we use zero.
    //
    // We ignore non-sensical values of MinNonIVVal/MaxNonIVVal. These are the
    // result of overly-conservative blob/upper bound values.

    bool HasMinNonIVVal =
        HLNodeUtils::getExactMinValue(CE, Node, MinNonIVVal) &&
        isInRange(MinNonIVVal, 0, NumElements);
    bool HasMaxNonIVVal =
        HLNodeUtils::getExactMaxValue(CE, Node, MaxNonIVVal) &&
        isInRange(MaxNonIVVal, 0, NumElements);

    if (HasMaxNonIVVal) {
      if (hasNegativeIVRelationship(ParentLp)) {
        int64_t OtherVal = HasMinNonIVVal ? MinNonIVVal : 0;
        NonIVVal = (MaxNonIVVal + OtherVal) / 2;
      } else {
        NonIVVal = MaxNonIVVal;
      }

    } else if (HasMinNonIVVal) {
      NonIVVal = MinNonIVVal;

    } else {
      // This gets us the most conservative estimate.
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

void HIRFramework::estimateMaxTripCounts() {
  MaxTripCountEstimator MTCE(this);
  getHLNodeUtils().visitAll(MTCE);
}

void HIRFramework::print(raw_ostream &OS) const { print(true, OS); }

void HIRFramework::print(bool FrameworkDetails, raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE
  formatted_raw_ostream FOS(OS);
  auto RegBegin = RI.begin();
  unsigned Offset = 0;
  bool PrintFrameworkDetails =
      PhaseParser->isReady() && (HIRFrameworkDetails || FrameworkDetails);

  for (auto I = hir_begin(), E = hir_end(); I != E; ++I, ++Offset) {
    assert(isa<HLRegion>(I) && "Top level node is not a region!");
    const HLRegion *Region = cast<HLRegion>(I);
    if ((HIRPrintModified && !Region->shouldGenCode()) ||
        (!HIRPrintOnly.empty() &&
         (std::find(HIRPrintOnly.begin(), HIRPrintOnly.end(),
                    Region->getNumber())) == HIRPrintOnly.end())) {
      continue;
    }

    // Print SCCs in hir-parser output and in detailed mode.
    if (PrintFrameworkDetails) {
      SCCF.print(FOS, RegBegin + Offset);
    }

    FOS << "\n";
    Region->print(FOS, 0, PrintFrameworkDetails, HIRPrinterDetails);
  }
  FOS << "\n";
#endif // !INTEL_PRODUCT_RELEASE
}

void HIRFramework::verify() const {
  if (HIRVerify) {
    HIRVerifier::verifyAll(*this);
    LLVM_DEBUG(dbgs() << "Verification of HIR done"
                      << "\n");
  }
}

unsigned HIRFramework::getMaxScalarSymbase() const {
  return PhaseScalarSA->getMaxScalarSymbase();
}

DDRefUtils &HIRFramework::getDDRefUtils() const {
  return PhaseParser->getDDRefUtils();
}

CanonExprUtils &HIRFramework::getCanonExprUtils() const {
  return PhaseParser->getCanonExprUtils();
}

BlobUtils &HIRFramework::getBlobUtils() const {
  return PhaseParser->getBlobUtils();
}

bool HIRFramework::isLiveinCopy(const HLInst *HInst) {
  return PhaseParser->isLiveinCopy(HInst);
}

bool HIRFramework::isLiveoutCopy(const HLInst *HInst) {
  return PhaseParser->isLiveoutCopy(HInst);
}
