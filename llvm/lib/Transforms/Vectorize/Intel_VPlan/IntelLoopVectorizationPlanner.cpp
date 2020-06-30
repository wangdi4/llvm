//===-- IntelLoopVectorizationPlanner.cpp ---------------------------------===//
//
//   Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements LoopVectorizationPlannerBase and
/// LoopVectorizationPlanner.
///
//===----------------------------------------------------------------------===//

#include "IntelLoopVectorizationPlanner.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelVPOCodeGen.h"
#include "IntelVPSOAAnalysis.h"
#include "IntelVPlanAllZeroBypass.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanClone.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanHCFGBuilder.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanLCSSA.h"
#include "IntelVPlanLoopCFU.h"
#include "IntelVPlanPredicator.h"
#include "IntelVPlanUtils.h"
#include "VPlanHIR/IntelVPlanHCFGBuilderHIR.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "LoopVectorizationPlanner"

#if INTEL_CUSTOMIZATION
extern llvm::cl::opt<bool> VPlanConstrStressTest;
extern llvm::cl::opt<bool> EnableVPValueCodegen;

static cl::opt<unsigned> VecThreshold(
    "vec-threshold",
    cl::desc("sets a threshold for the vectorization on the probability"
             "of profitable execution of the vectorized loop in parallel."),
    cl::init(100));

#else
cl::opt<unsigned> VPlanDefaultEstTrip("vplan-default-est-trip", cl::init(300),
                                      cl::desc("Default estimated trip count"));
#endif // INTEL_CUSTOMIZATION
static cl::opt<unsigned> VPlanForceVF("vplan-force-vf", cl::init(0),
                                      cl::desc("Force VPlan to use given VF"));

static cl::opt<bool>
    DisableVPlanPredicator("disable-vplan-predicator", cl::init(false),
                           cl::Hidden, cl::desc("Disable VPlan predicator."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::list<unsigned> VPlanCostModelPrintAnalysisForVF(
    "vplan-cost-model-print-analysis-for-vf", cl::Hidden, cl::CommaSeparated,
    cl::ZeroOrMore,
    cl::desc("Print detailed VPlan Cost Model Analysis report for the given "
             "VF. For testing/debug purposes only."));

static cl::opt<bool>
    PrintAfterLCSSA("vplan-print-after-lcssa", cl::init(false), cl::Hidden,
                    cl::desc("Print VPlan after LCSSA transformation."));

static cl::opt<bool>
    PrintAfterLoopCFU("vplan-print-after-loop-cfu", cl::init(false), cl::Hidden,
                      cl::desc("Print VPlan after LoopCFU transformation."));

static cl::opt<bool> PrintAfterLinearization(
    "vplan-print-after-linearization", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan after predication and linearization."));

static cl::opt<bool> DotAfterLinearization(
    "vplan-dot-after-linearization", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan digraph after predication and linearization."));

static cl::opt<bool> PrintAfterAllZeroBypass(
    "vplan-print-after-all-zero-bypass", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan after all zero bypass insertion."));

static cl::opt<bool> DotAfterAllZeroBypass(
    "vplan-dot-after-all-zero-bypass", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan digraph after all zero bypass insertion."));

static cl::opt<bool, true> PrintAfterCallVecDecisionsOpt(
    "vplan-print-after-call-vec-decisions", cl::Hidden,
    cl::location(PrintAfterCallVecDecisions),
    cl::desc("Print VPlan after analyzing calls for vectorization decisions."));

static cl::opt<bool, true> PrintSVAResultsOpt(
    "vplan-print-scalvec-results", cl::Hidden, cl::location(PrintSVAResults),
    cl::desc("Print VPlan with results of ScalVec analysis."));
#else
static constexpr bool PrintAfterLCSSA = false;
static constexpr bool PrintAfterLoopCFU = false;
static constexpr bool PrintAfterLinearization = false;
static constexpr bool DotAfterLinearization = false;
static constexpr bool PrintAfterAllZeroBypass = false;
static constexpr bool DotAfterAllZeroBypass = false;
static constexpr bool PrintAfterCallVecDecisionsOpt = false;
static constexpr bool PrintSVAResultsOpt = false;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

namespace {
using ForceVFTy = std::pair<int /* LoopID */, unsigned /* VF */>;

struct VPlanLoopVFParser : public cl::parser<ForceVFTy> {
  VPlanLoopVFParser(cl::Option &O)
      : cl::parser<ForceVFTy>(O) {}

  bool parse(cl::Option &O, StringRef ArgName, StringRef Arg,
             ForceVFTy &Result) {
    std::pair<StringRef, StringRef> LoopVFPair = Arg.split(':');
    int LoopId = -1;
    if (LoopVFPair.first.getAsInteger(10, LoopId))
      return O.error("Cannot parse LoopID!");

    assert(LoopId != -1 && "Couldn't get a valid LoopId.");

    unsigned VF = 0;
    if (LoopVFPair.second.getAsInteger(10, VF))
      return O.error("Cannot parse VF!");

    assert(VF != 0 && "Couldn't get a valid value for VF.");

    Result = std::make_pair(LoopId, VF);
    return false;
  }
};
}

static cl::list<ForceVFTy, bool /* Use internal storage */, VPlanLoopVFParser>
    ForceLoopVF(
        "vplan-force-loop-vf", cl::Hidden, cl::ZeroOrMore,
        cl::value_desc("LoopID:VF"),
        cl::desc(
            "Debug option to force vectorization scenario of a particular "
            "loop. This option is *NOT* thread-safe and might not have "
            "production quality! Loops order is also implementation-defined "
            "and might not match the order of the loops in the input. Refer to "
            "-debug-only=LoopVectorizationPlanner to see the mapping. It is "
            "expected to be deterministic between multiple runs of the same "
            "compiler invocation though. Comma separated list of pairs isn't "
            "supported at this moment - use the option multiple times to force "
            "scenarios for multiple loops."));
static int VPlanOrderNumber = 0;

using namespace llvm;
using namespace llvm::vpo;

namespace llvm {
namespace vpo {
bool PrintSVAResults = false;
bool PrintAfterCallVecDecisions = false;
} // namespace vpo
} // namespace llvm

static unsigned getForcedVF(const WRNVecLoopNode *WRLp) {
  auto ForcedLoopVFIter = llvm::find_if(ForceLoopVF, [](const ForceVFTy &Pair) {
    return Pair.first == VPlanOrderNumber;
  });
  if (ForcedLoopVFIter != ForceLoopVF.end())
    return ForcedLoopVFIter->second;

  if (VPlanForceVF)
    return VPlanForceVF;
  return WRLp && WRLp->getSimdlen() ? WRLp->getSimdlen() : 0;
}

#if INTEL_CUSTOMIZATION
static unsigned getSafelen(const WRNVecLoopNode *WRLp) {
  return WRLp && WRLp->getSafelen() ? WRLp->getSafelen() : UINT_MAX;
}
#endif // INTEL_CUSTOMIZATION

unsigned LoopVectorizationPlanner::buildInitialVPlans(LLVMContext *Context,
                                                      const DataLayout *DL) {
  ++VPlanOrderNumber;
  unsigned MinVF, MaxVF;
  unsigned ForcedVF = getForcedVF(WRLp);

#if INTEL_CUSTOMIZATION
  unsigned Safelen = getSafelen(WRLp);

  LLVM_DEBUG(dbgs() << "LVP: ForcedVF: " << ForcedVF << "\n");
  LLVM_DEBUG(dbgs() << "LVP: Safelen: " << Safelen << "\n");

  // Early return from vectorizer if forced VF or safelen is 1
  if (ForcedVF == 1 || Safelen == 1) {
    LLVM_DEBUG(dbgs() << "LVP: The forced VF or safelen specified by user is "
                         "1, VPlans need not be constructed.\n");
    return 0;
  }
#endif // INTEL_CUSTOMIZATION

  if (ForcedVF) {
#if INTEL_CUSTOMIZATION
    if (ForcedVF > Safelen) {
      // We are bailing out of vectorization if ForcedVF > safelen
      assert(WRLp && WRLp->isOmpSIMDLoop() &&
             "safelen is set on a non-OMP SIMD loop.");
      LLVM_DEBUG(dbgs() << "VPlan: The forced VF is greater than safelen set "
                           "via `#pragma omp simd`\n");
      return 0;
    }
#endif // INTEL_CUSTOMIZATION
    MinVF = ForcedVF;
    MaxVF = ForcedVF;
#if INTEL_CUSTOMIZATION
  } else if (VPlanConstrStressTest) {
    // If we are only stress testing VPlan construction, force VPlan
    // construction for just VF 1. This avoids any divide by zero errors in the
    // min/max VF computation.
    MinVF = 1;
    MaxVF = 1;
#endif // INTEL_CUSTOMIZATION
  } else {
    unsigned MinWidthInBits, MaxWidthInBits;
    std::tie(MinWidthInBits, MaxWidthInBits) = getTypesWidthRangeInBits();
    const unsigned MinVectorWidth = TTI->getMinVectorRegisterBitWidth();
    const unsigned MaxVectorWidth = TTI->getRegisterBitWidth(true /* Vector */);
    MaxVF = MaxVectorWidth / MinWidthInBits;
    MinVF = std::max(MinVectorWidth / MaxWidthInBits, 1u);

    // FIXME: Why is this?
    MaxVF = std::min(MaxVF, 32u);
    MinVF = std::min(MinVF, 32u);

    LLVM_DEBUG(dbgs() << "LVP: Orig MinVF: " << MinVF
                      << " Orig MaxVF: " << MaxVF << "\n");

    // Maximum allowed VF specified by user is Safelen
    Safelen = (unsigned)PowerOf2Floor(Safelen);
    MaxVF = std::min(MaxVF, Safelen);
    // We won't be able to fill the entire register, but it
    // still might be profitable.
    MinVF = std::min(MinVF, Safelen);
  }

#if INTEL_CUSTOMIZATION
  LLVM_DEBUG(dbgs() << "LVP: MinVF: " << MinVF << " MaxVF: " << MaxVF << "\n");
#endif // INTEL_CUSTOMIZATION

  unsigned StartRangeVF = MinVF;
  unsigned EndRangeVF = MaxVF + 1;

  Externals = std::make_unique<VPExternalValues>(Context, DL);

  unsigned i = 0;
  for (; StartRangeVF < EndRangeVF; ++i) {
    // TODO: revisit when we build multiple VPlans.
    std::shared_ptr<VPlan> Plan =
        buildInitialVPlan(StartRangeVF, EndRangeVF, *Externals);

    // Check legality of VPlan before proceeding with other transforms/analyses.
    if (!canProcessVPlan(*Plan.get())) {
      LLVM_DEBUG(
          dbgs() << "LVP: VPlan is not legal to process, bailing out.\n");
      return 0;
    }

    auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
    Plan->setVPlanDA(std::move(VPDA));
    auto *VPLInfo = Plan->getVPLoopInfo();
    VPLoop *CandidateLoop = *VPLInfo->begin();
    Plan->computeDT();
    Plan->computePDT();
    Plan->getVPlanDA()->compute(Plan.get(), CandidateLoop, VPLInfo,
                                *Plan->getDT(), *Plan->getPDT(),
                                false /*Not in LCSSA form*/);

    // Do SOA-analysis for loop-privates.
    VPSOAAnalysis VPSOAA(*Plan.get(), *CandidateLoop);
    SmallPtrSet<VPInstruction *, 32> SOAVars;
    VPSOAA.doSOAAnalysis(SOAVars);

    if (EnableSOAAnalysis)
      Plan->getVPlanDA()->recomputeShapes(SOAVars);

    // TODO: Insert initial run of SVA here for any new users before CM & CG.

    for (unsigned TmpVF = StartRangeVF; TmpVF < EndRangeVF; TmpVF *= 2)
      VPlans[TmpVF] = Plan;

    StartRangeVF = EndRangeVF;
    EndRangeVF = MaxVF + 1;
  }

  // Scalar VPlan is not necessary when VF was forced.
  // TODO: Need it later for optreport to print potential speedup.
  if (!ForcedVF)
    VPlans[1] = VPlans[MinVF];

  return i;
}

void LoopVectorizationPlanner::selectBestPeelingVariants() {
  std::map<VPlan *, VPlanPeelingAnalysis> VPPACache;
  VPlanPeelingCostModelSimple CM(*DL);

  for (auto &Pair : VPlans) {
    auto VF = Pair.first;
    VPlan &Plan = *Pair.second;

    if (VF == 1)
      continue;

    auto Found = VPPACache.find(&Plan);
    if (Found == VPPACache.end()) {
      VPlanPeelingAnalysis VPPA(CM, *Plan.getVPSE(), *Plan.getVPVT(), *DL);
      VPPA.collectMemrefs(Plan);
      std::tie(Found, std::ignore) = VPPACache.emplace(&Plan, std::move(VPPA));
    }

    Plan.setPreferredPeeling(VF, Found->second.selectBestPeelingVariant(VF));
  }
}

/// Evaluate cost model for available VPlans and find the best one.
/// \Returns VF which corresponds to the best VPlan (could be VF = 1).
template <typename CostModelTy>
unsigned LoopVectorizationPlanner::selectBestPlan() {
  LLVM_DEBUG(dbgs() << "Selecting VF for VPlan #" << VPlanOrderNumber << '\n');
  if (VPlans.size() == 1) {
    unsigned ForcedVF = getForcedVF(WRLp);
    assert(ForcedVF &&
           "Only one VPlan was constructed with non-forced vectorization.");
    BestVF = ForcedVF;
    // FIXME: this code should be revisited later to select best UF
    // even with forced VF.
    BestUF = 1;
    LLVM_DEBUG(dbgs() << "There is only VPlan with VF=" << BestVF
                      << ", selecting it.\n");

    return ForcedVF;
  }

  // FIXME: This value of MaxVF has to be aligned with value of MaxVF in
  // buildInitialVPlan.
  // TODO: Add options to set MinVF and MaxVF.
  const unsigned MaxVF = 32;
  VPlan *ScalarPlan = getVPlanForVF(1);
  assert(ScalarPlan && "There is no scalar VPlan!");
  // FIXME: Without peel and remainder vectorization, it's ok to get trip count
  // from the original loop. Has to be revisited after enabling of
  // peel/remainder vectorization.

  // Even if TripCount is more than 2^32 we can safely assume that it's equal
  // to 2^32, otherwise all logic below will have a problem with overflow.
  VPLoopInfo *VPLI = ScalarPlan->getVPLoopInfo();
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
         "Expected single outermost loop!");
  VPLoop *OuterMostVPLoop = *VPLI->begin();
  uint64_t TripCount = std::min(OuterMostVPLoop->getTripCountInfo().TripCount,
                                (uint64_t)std::numeric_limits<unsigned>::max());
#if INTEL_CUSTOMIZATION
  CostModelTy ScalarCM(ScalarPlan, 1, TTI, TLI, DL, VLSA);
#else
  CostModelTy ScalarCM(ScalarPlan, 1, TTI, TLI, DL);
#endif // INTEL_CUSTOMIZATION
  unsigned ScalarIterationCost = ScalarCM.getCost();
  ScalarIterationCost =
      ScalarIterationCost == CostModelTy::UnknownCost ? 0 : ScalarIterationCost;
  // FIXME: that multiplication should be the part of CostModel - see below.
  uint64_t ScalarCost = ScalarIterationCost * TripCount;

  BestVF = 1;
  // FIXME: Currently, unroll is not in VPlan, so set it to 1.
  BestUF = 1;
  uint64_t BestCost = ScalarCost;
  LLVM_DEBUG(dbgs() << "Cost of Scalar VPlan: " << ScalarCost << '\n');

#if INTEL_CUSTOMIZATION
  // When a loop is marked with pragma vector always, the user wants the loop
  // to be vectorized. In order to force the loop to be vectorized, we set
  // BestCost to MAX value for such a case. WRLp can be null when stress testing
  // vector code generation.
  bool ShouldIgnoreProfitability =
      WRLp && (WRLp->getHasVectorAlways() || WRLp->isOmpSIMDLoop());

  bool IsVectorAlways = ShouldIgnoreProfitability || VecThreshold == 0;

  if (IsVectorAlways) {
    BestCost = std::numeric_limits<uint64_t>::max();
    LLVM_DEBUG(dbgs() << "'#pragma vector always'/ '#pragma omp simd' is used "
                         "for the given loop\n");
  }
#endif // INTEL_CUSTOMIZATION

  // FIXME: Currently limit this to VF = 16. Has to be fixed with more accurate
  // cost model.
  for (unsigned VF = 2; VF <= MaxVF; VF *= 2) {
    if (!hasVPlanForVF(VF))
      continue;

    if (TripCount < VF)
      continue; // FIXME: Consider masked low trip later.

    VPlan *Plan = getVPlanForVF(VF);

    // FIXME: The remainder loop should be an explicit part of VPlan and the
    // cost model should just do the right thing calulating the cost of the
    // plan. However this is not the case yet so do some simple heuristic.
#if INTEL_CUSTOMIZATION
    CostModelTy VectorCM(Plan, VF, TTI, TLI, DL, VLSA);
#else
    CostModelTy VectorCM(Plan, VF, TTI, TLI, DL);
#endif // INTEL_CUSTOMIZATION
    const unsigned VectorIterationCost = VectorCM.getCost();
    if (VectorIterationCost == CostModelTy::UnknownCost) {
      LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << " is unknown. Skip it.\n");
      continue;
    }

    const decltype(TripCount) VectorTripCount = TripCount / VF;
    const decltype(TripCount) RemainderTripCount = TripCount % VF;
    // TODO: Take into account overhead for some instructions until explicit
    // representation of peel/remainder not ready.
    uint64_t VectorCost = VectorIterationCost * VectorTripCount +
                          ScalarIterationCost * RemainderTripCount;
    if (0 < VecThreshold && VecThreshold < 100) {
      LLVM_DEBUG(dbgs() << "Applying threshold " << VecThreshold << " for VF "
                        << VF << ". Original cost = " << VectorCost << '\n');
      VectorCost = (uint64_t)(VectorCost * (100.0 - VecThreshold)) / 100.0f;
    }
    const char CmpChar =
        ScalarCost < VectorCost ? '<' : ScalarCost == VectorCost ? '=' : '>';
    (void)CmpChar;
    LLVM_DEBUG(dbgs() << "Scalar Cost = " << TripCount << " x "
                      << ScalarIterationCost << " = " << ScalarCost << ' '
                      << CmpChar << " VectorCost = " << VectorTripCount << "[x"
                      << VF << "] x " << VectorIterationCost << " + "
                      << ScalarIterationCost << " x " << RemainderTripCount
                      << " = " << VectorCost << '\n';);
    if (VectorCost < BestCost) {
      BestCost = VectorCost;
      BestVF = VF;
    }
  }
#if INTEL_CUSTOMIZATION
  // Corner case: all available VPlans have UMAX cost.
  // With 'vector always' we have to vectorize with some VF, so select first
  // available VF.
  if (BestVF == 1 && IsVectorAlways)
    for (BestVF = 2; BestVF <= MaxVF; BestVF *= 2)
      if (hasVPlanForVF(BestVF))
        break;
#endif // INTEL_CUSTOMIZATION

  // Delete all other VPlans.
  for (auto &It : VPlans) {
    if (It.first != BestVF)
      VPlans.erase(It.first);
  }
  LLVM_DEBUG(dbgs() << "Selecting VPlan with VF=" << BestVF << '\n');
  return BestVF;
}

template unsigned
LoopVectorizationPlanner::selectBestPlan<VPlanCostModel>(void);
#if INTEL_CUSTOMIZATION
template unsigned
LoopVectorizationPlanner::selectBestPlan<VPlanCostModelProprietary>(void);
#endif // INTEL_CUSTOMIZATION

void LoopVectorizationPlanner::predicate() {
  if (DisableVPlanPredicator)
    return;

  DenseSet<VPlan *> PredicatedVPlans;
  for (auto It : VPlans) {
    if (It.first == 1)
      continue; // Ignore Scalar VPlan;
    VPlan *VPlan = It.second.get();
    if (PredicatedVPlans.count(VPlan))
      continue; // Already predicated.

    VPLoopInfo *VPLI = VPlan->getVPLoopInfo();
    assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
           "There should be single outer loop!");
    VPLoop *OuterLoop = *VPLI->begin();
    // Search loops require multiple hacks. Skipping LCSSA/LoopCFU is one of
    // them.
    bool SearchLoopHack = !OuterLoop->getExitBlock();

    if (!SearchLoopHack)
      formLCSSA(*VPlan, true /* SkipTopLoop */);
    VPLAN_DUMP(PrintAfterLCSSA, "LCSSA transformation", VPlan);

    if (!SearchLoopHack) {
      assert(!VPlan->getVPlanDA()->isDivergent(
                 *(OuterLoop)->getLoopLatch()->getCondBit()) &&
             "Outer loop doesn't have uniform backedge!");
      VPlanLoopCFU LoopCFU(*VPlan);
      LoopCFU.run();
    }
    VPLAN_DUMP(PrintAfterLoopCFU, "Loop CFU transformation", VPlan);

    // Predication "has" to be done even for the search loop hack. Our
    // idiom-matching code and CG currently expect that. Note that predicator
    // has some hacks for search loop processing inside it as well.
    VPlanPredicator VPP(*VPlan);
    VPP.predicate();

    VPLAN_DUMP(PrintAfterLinearization, "predication and linearization", VPlan);
    VPLAN_DOT(DotAfterLinearization, VPlan);

    PredicatedVPlans.insert(VPlan);
  }
}

void LoopVectorizationPlanner::insertAllZeroBypasses(VPlan *Plan) {
  // Skip multi-exit loops at outer VPlan level. Inner loops will be
  // canonicalized to single exit in VPlan. TODO: this check is only
  // needed due to hacky search loop support. Change to assert in the
  // future.
  VPLoop *VPLp = *(Plan->getVPLoopInfo()->begin());
  if (!VPLp->getExitBlock())
    return;

  // Holds the pair of blocks representing the begin/end of an all-zero
  // bypass region. The block-predicate at the begin block is used to
  // generate the bypass.
  VPlanAllZeroBypass::AllZeroBypassRegionsTy AllZeroBypassRegions;
  VPlanAllZeroBypass AZB(*Plan);
  AZB.collectAllZeroBypassRegions(AllZeroBypassRegions);
  AZB.insertAllZeroBypasses(AllZeroBypassRegions);

  VPLAN_DUMP(PrintAfterAllZeroBypass, "all zero bypass insertion", Plan);
  VPLAN_DOT(DotAfterAllZeroBypass, Plan);
}

void LoopVectorizationPlanner::unroll(
    VPlan &Plan, unsigned UF,
    VPlanLoopUnroller::VPInstUnrollPartTy *VPInstUnrollPart) {
  if (UF > 1) {
    VPlanLoopUnroller Unroller(Plan, UF);
    Unroller.run(VPInstUnrollPart);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
template <typename CostModelTy>
void LoopVectorizationPlanner::printCostModelAnalysisIfRequested(
  const std::string &Header) {
  for (unsigned VFRequested : VPlanCostModelPrintAnalysisForVF) {
    if (!hasVPlanForVF(VFRequested)) {
      errs() << "VPlan for VF = " << VFRequested << " was not constructed\n";
      continue;
    }
    VPlan *Plan = getVPlanForVF(VFRequested);

#if INTEL_CUSTOMIZATION
    CostModelTy CM(Plan, VFRequested, TTI, TLI, DL, VLSA);
#else
    CostModelTy CM(Plan, VFRequested, TTI, TLI, DL);
#endif // INTEL_CUSTOMIZATION

    // If different stages in VPlanDriver were proper passes under pass manager
    // control it would have been opt's output stream (via "-o" switch). As it
    // is not so, just pass stdout so that we would not be required to redirect
    // stderr to Filecheck.
    CM.print(outs(), Header);
  }
}

// Explicit instantiations.
template void
LoopVectorizationPlanner::printCostModelAnalysisIfRequested<VPlanCostModel>(
  const std::string &Header);
template void LoopVectorizationPlanner::printCostModelAnalysisIfRequested<
    VPlanCostModelProprietary>(
  const std::string &Header);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

// TODO: Current implementation is too aggressive and may lead to increase of
// compile time. Also similar changeset in community led to performance drops.
// See https://reviews.llvm.org/D44523 for a similar discussion about LV.
std::pair<unsigned, unsigned>
LoopVectorizationPlanner::getTypesWidthRangeInBits() const {
  unsigned MinWidth = static_cast<unsigned>(-1);
  unsigned MaxWidth = 0;

  for (const BasicBlock *BB : TheLoop->blocks()) {
    for (const Instruction &Inst : *BB) {
      if (auto Ty = Inst.getType()) {
        unsigned Width = Ty->getPrimitiveSizeInBits();

        // Ignore the conditions (i1) - they're unlikely to be widened.
        if (Width < 8)
          continue;

        MinWidth = std::min(MinWidth, Width);
        MaxWidth = std::max(MaxWidth, Width);
      }
    }
  }
  return {MinWidth, MaxWidth};
}

std::shared_ptr<VPlan> LoopVectorizationPlanner::buildInitialVPlan(
    unsigned StartRangeVF, unsigned &EndRangeVF, VPExternalValues &Ext) {
  // Create new empty VPlan
  std::shared_ptr<VPlan> SharedPlan = std::make_shared<VPlan>(Ext);
  VPlan *Plan = SharedPlan.get();

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(TheLoop, LI, *DL, WRLp, Plan, Legal);
  HCFGBuilder.buildHierarchicalCFG();

  return SharedPlan;
}

// Feed explicit data, saved in WRNVecLoopNode to the CodeGen.
#if INTEL_CUSTOMIZATION
template <class Legality> constexpr IRKind getIRKindByLegality() {
  return std::is_same<Legality, VPOVectorizationLegality>::value
             ? IRKind::LLVMIR
             : IRKind::HIR;
}

template <class VPOVectorizationLegality>
#endif
void LoopVectorizationPlanner::EnterExplicitData(
    WRNVecLoopNode *WRLp, VPOVectorizationLegality &LVL) {
#if INTEL_CUSTOMIZATION
  constexpr IRKind Kind = getIRKindByLegality<VPOVectorizationLegality>();
#endif
  // Collect any SIMD loop private information
  if (WRLp) {
    LastprivateClause &LastPrivateClause = WRLp->getLpriv();
    for (LastprivateItem *PrivItem : LastPrivateClause.items()) {
#if INTEL_CUSTOMIZATION
      auto PrivVal = PrivItem->getOrig<Kind>();
#else
      auto PrivVal = PrivItem->getOrig();
#endif
      LVL.addLoopPrivate(PrivVal, true, PrivItem->getIsConditional());
    }
    PrivateClause &PrivateClause = WRLp->getPriv();
    for (PrivateItem *PrivItem : PrivateClause.items()) {
#if INTEL_CUSTOMIZATION
      auto PrivVal = PrivItem->getOrig<Kind>();
#else
      auto PrivVal = PrivItem->getOrig();
#endif
      LVL.addLoopPrivate(PrivVal);
    }

    // Add information about loop linears to Legality
    LinearClause &LinearClause = WRLp->getLinear();
    for (LinearItem *LinItem : LinearClause.items()) {
#if INTEL_CUSTOMIZATION
      auto LinVal = LinItem->getOrig<Kind>();
      auto Step = LinItem->getStep<Kind>();
#else
      auto LinVal = LinItem->getOrig();
      auto Step = LinItem->getStep();
#endif
      LVL.addLinear(LinVal, Step);
    }

    ReductionClause &RedClause = WRLp->getRed();
    for (ReductionItem *RedItem : RedClause.items()) {
#if INTEL_CUSTOMIZATION
      auto V = RedItem->getOrig<Kind>();
#else
      auto V = RedItem->getOrig();
#endif
      ReductionItem::WRNReductionKind Type = RedItem->getType();
      switch (Type) {
      case ReductionItem::WRNReductionMin:
        LVL.addReductionMin(V, !RedItem->getIsUnsigned());
        break;
      case ReductionItem::WRNReductionMax:
        LVL.addReductionMax(V, !RedItem->getIsUnsigned());
        break;
      case ReductionItem::WRNReductionAdd:
      case ReductionItem::WRNReductionSub:
        LVL.addReductionAdd(V);
        break;
      case ReductionItem::WRNReductionMult:
        LVL.addReductionMult(V);
        break;
      case ReductionItem::WRNReductionBor:
        LVL.addReductionOr(V);
        break;
      case ReductionItem::WRNReductionBxor:
        LVL.addReductionXor(V);
        break;
      case ReductionItem::WRNReductionBand:
        LVL.addReductionAnd(V);
        break;
      default:
        break;
      }
    }
  }
}

bool LoopVectorizationPlanner::canProcessVPlan(const VPlan &Plan) {
  VPLoop *VPLp = *(Plan.getVPLoopInfo()->begin());
  VPBasicBlock *Header = VPLp->getHeader();
  const VPLoopEntityList *LE = Plan.getLoopEntities(VPLp);
  // Check whether all header phis are recognized as entities.
  for (auto &Phi : Header->getVPPhis())
    if (!LE->getInduction(&Phi) && !LE->getReduction(&Phi) &&
        !LE->getPrivate(&Phi)) {
      // Non-entity phi. Check whether it is explicit vector loop induction.
      if (llvm::any_of(Phi.operands(), [](const VPValue *V) {
            if (auto *I = dyn_cast<VPInstruction>(V))
              return I->getOpcode() == Instruction::Add &&
                     llvm::any_of(I->operands(), [](const VPValue *V) {
                       return isa<VPInductionInitStep>(V);
                     });
            return false;
          }))
        continue;
      LLVM_DEBUG(dbgs() << "LVP: Unrecognized phi found.\n" << Phi);
      return false;
    }

  // All safety checks passed.
  return true;
}

namespace llvm {
namespace vpo {
#if INTEL_CUSTOMIZATION
template void
LoopVectorizationPlanner::EnterExplicitData<HIRVectorizationLegality>(
    WRNVecLoopNode *WRLp, HIRVectorizationLegality &LVL);
template void
LoopVectorizationPlanner::EnterExplicitData<VPOVectorizationLegality>(
    WRNVecLoopNode *WRLp, VPOVectorizationLegality &LVL);
#endif
} // namespace vpo
} // namespace llvm

void LoopVectorizationPlanner::executeBestPlan(VPOCodeGen &LB) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  ILV = &LB;

  // Perform the actual loop widening (vectorization).
  // 1. Create a new empty loop. Unlink the old loop and connect the new one.
  ILV->createEmptyLoop();

  // 2. Widen each instruction in the old loop to a new one in the new loop.
  VPCallbackILV CallbackILV;

  // Run CallVecDecisions analysis for final VPlan which will be used by CG.
  VPlan *Plan = getVPlanForVF(BestVF);
  assert(Plan && "No VPlan found for BestVF.");
  VPlanCallVecDecisions CallVecDecisions(*Plan);
  CallVecDecisions.run(BestVF, TLI, TTI);
  std::string Label("CallVecDecisions analysis for VF=" +
                    std::to_string(BestVF));
  VPLAN_DUMP(PrintAfterCallVecDecisions, Label, Plan);

  // Compute SVA results for final VPlan which will be used by CG.
  Plan->runSVA(BestVF, TLI);
  VPLAN_DUMP(PrintSVAResults, "ScalVec analysis", Plan);

  VPTransformState State(BestVF, BestUF, LI, DT, ILV->getBuilder(), ILV,
                         CallbackILV, Plan->getVPLoopInfo());
  State.CFG.PrevBB = ILV->getLoopVectorPH();

#if INTEL_CUSTOMIZATION
  // Set ILV transform state
  ILV->setTransformState(&State);
#endif // INTEL_CUSTOMIZATION

  Plan->execute(&State);

  // 3. Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  ILV->finalizeLoop();
}
