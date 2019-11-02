//===----- HIRFramework.cpp - public interface for HIR framework ----------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"

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
      AM.getResult<ScalarEvolutionAnalysis>(F), AM.getResult<AAManager>(F),
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
          }));
}

INITIALIZE_PASS_BEGIN(HIRFrameworkWrapperPass, "hir-framework", "HIR Framework",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
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
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
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
      getAnalysis<ScalarEvolutionWrapperPass>().getSE(),
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
          })));
  return false;
}

void HIRFramework::runImpl() {
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
  HNU->initTopSortNum();

  estimateMaxTripCounts();

#ifndef NDEBUG
  verify();
#endif
}

HIRFramework::HIRFramework(Function &F, DominatorTree &DT,
                           PostDominatorTree &PDT, LoopInfo &LI,
                           ScalarEvolution &SE, AAResults &AA,
                           HIRRegionIdentification &RI, HIRSCCFormation &SCCF,
                           OptReportVerbosity::Level VerbosityLevel,
                           HIRAnalysisProvider AnalysisProvider)
    : Func(F), DT(DT), PDT(PDT), LI(LI), SE(SE), AA(AA), RI(RI), SCCF(SCCF),
      AnalysisProvider(AnalysisProvider), MaxSymbase(0) {
  HNU.reset(new HLNodeUtils(*this));
  HNU->reset(F);

  LORBuilder.setup(F.getContext(), VerbosityLevel);

  PhaseCreation.reset(new HIRCreation(DT, PDT, LI, RI, *HNU));
  PhaseCleanup.reset(new HIRCleanup(LI, *PhaseCreation, *HNU));
  PhaseLoopFormation.reset(
      new HIRLoopFormation(LI, SE, RI, *PhaseCreation, *PhaseCleanup, *HNU));
  PhaseScalarSA.reset(new HIRScalarSymbaseAssignment(
      LI, SE, RI, SCCF, *PhaseLoopFormation, *HNU));
  PhaseParser.reset(new HIRParser(DT, LI, SE, RI, *this, *PhaseCreation,
                                  *PhaseLoopFormation, *PhaseScalarSA, *HNU));

  runImpl();
}

HIRFramework::HIRFramework(HIRFramework &&Arg)
    : Func(Arg.Func), DT(Arg.DT), PDT(Arg.PDT), LI(Arg.LI), SE(Arg.SE),
      AA(Arg.AA), RI(Arg.RI), SCCF(Arg.SCCF),
      LORBuilder(std::move(Arg.LORBuilder)), HNU(std::move(Arg.HNU)),
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
  void visit(CanonExpr *CE, unsigned NumElements, HLDDNode *Node);
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
    if (auto NumElements = Ref->getNumDimensionElements(I)) {
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

  auto NumElements = HIRF->PhaseParser->getPointerDimensionSize(BaseVal);

  if (NumElements) {
    visit(HighestCE, NumElements, Node);
  }
}

bool isInRange(int64_t Val, int64_t LowerBound, int64_t UpperBound) {
  return (Val >= LowerBound) && (Val <= UpperBound);
}

void HIRFramework::MaxTripCountEstimator::visit(CanonExpr *CE,
                                                unsigned NumElements,
                                                HLDDNode *Node) {

  // We cannot estimate the iteration space of the IV with a varying blob.
  if (CE->isNonLinear() || !CE->hasIV()) {
    return;
  }

  auto Denom = CE->getDenominator();

  // The analysis is carried out based on the assumption that the range of the
  // subscript is [0, NumElements].

  for (auto Lp = Node->getParentLoop(); Lp != nullptr;
       Lp = Lp->getParentLoop()) {
    unsigned Index;
    int64_t Coeff, BlobVal = 1, NonIVVal = 0;
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

    // The max value of the rest of the CE gives a better estimate on max
    // trip count so we check max value first. For example, A[i + j] will
    // give a tighter bound on j for max value of i. Similarly, for A[i - j]
    // max value of i gives max estimate for j.
    // Ignore non-sensical values of NonIVVal. These are the result of
    // overly-conservative blob/upper bound values.
    if ((!HLNodeUtils::getExactMaxValue(CE, Node, NonIVVal) ||
         !isInRange(NonIVVal, 0, NumElements)) &&
        (!HLNodeUtils::getExactMinValue(CE, Node, NonIVVal) ||
         !isInRange(NonIVVal, 0, NumElements))) {
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
