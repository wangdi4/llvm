//===-- IntelLoopVectorizationPlanner.cpp ---------------------------------===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
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
#include "HIR/IntelVPlanHCFGBuilderHIR.h"
#include "IntelVPlanAllZeroBypass.h"
#include "IntelVPlanCFGMerger.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanClone.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanEvaluator.h"
#include "IntelVPlanHCFGBuilder.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanLCSSA.h"
#include "IntelVPlanLoopCFU.h"
#include "IntelVPlanLoopExitCanonicalization.h"
#include "IntelVPlanPatternMatch.h"
#include "IntelVPlanPredicator.h"
#include "IntelVPlanTransformEarlyExitLoop.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVConflictTransformation.h"
#include "Intel_VPlan/IntelVPTransformLibraryCalls.h"
#include "LLVM/CodeGenLLVM.h"
#include "LLVM/LegalityLLVM.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/CommandLine.h"
#include <sstream>

#define DEBUG_TYPE "LoopVectorizationPlanner"

using namespace llvm::PatternMatch;

extern llvm::cl::opt<bool> VPlanConstrStressTest;

static cl::opt<unsigned> VecThreshold(
    "vec-threshold",
    cl::desc("sets a threshold for the vectorization on the probability"
             "of profitable execution of the vectorized loop in parallel."),
    cl::init(100));

static cl::opt<unsigned> MaskedMainThreshold(
    "vplan-masked-main-cost-threshold",
    cl::desc("Minimum value of masked main loop gain, in percent of scalar "
             "remainder cost. The bigger value the bigger gain required for "
             "masked main loop to be choosen."),
    cl::init(54));

static cl::opt<unsigned> VPlanForceVF(
    "vplan-force-vf", cl::init(0),
    cl::desc("Force VPlan to use given VF, for experimental purposes only."));

static cl::opt<unsigned> VPlanTargetVF(
    "vplan-target-vf", cl::init(0),
    cl::desc("When simdlen is not set force VPlan to use given VF"));

static cl::opt<unsigned> VPlanSearchLpPtrEqForceVF(
    "vplan-search-lp-ptr-eq-force-vf", cl::init(4),
    cl::desc("Force VPlan to use given VF to vectorize all search loops that "
             "match the SearchLoopPtrEq idiom."));

static cl::opt<bool>
    DisableVPlanPredicator("disable-vplan-predicator", cl::init(false),
                           cl::Hidden, cl::desc("Disable VPlan predicator."));

static cl::opt<bool> EnableAllZeroBypassNonLoops(
    "vplan-enable-all-zero-bypass-non-loops", cl::init(true), cl::Hidden,
    cl::desc("Enable all-zero bypass insertion for non-loops."));

static cl::opt<bool> EnableAllZeroBypassLoops(
    "vplan-enable-all-zero-bypass-loops", cl::init(true), cl::Hidden,
    cl::desc("Enable all-zero bypass insertion for loops."));

static cl::opt<bool, true> LoopMassagingEnabledOpt(
    "vplan-enable-loop-massaging", cl::location(LoopMassagingEnabled),
    cl::Hidden,
    cl::desc("Enable loop massaging in VPlan (Multiple to Singular Exit)"));

static cl::opt<bool>
    EnableAllLiveOuts("vplan-enable-all-liveouts", cl::init(false),
                           cl::Hidden, cl::desc("Enable all liveouts, including private."));

static cl::opt<unsigned> VPlanForceUF("vplan-force-uf", cl::init(0),
                                      cl::desc("Force VPlan to use given UF"));

static cl::opt<unsigned> VPlanMaximumUF(
    "vplan-maximum-uf", cl::init(1), cl::Hidden,
    cl::desc("Set the maximum UF to be considered for unrolling"));

static cl::opt<bool> EnableGeneralPeelingCostModel(
    "vplan-enable-general-peeling-cost-model", cl::init(true),
    cl::desc("Use more advanced general cost model instead of a simple one for "
             "peeling decisions"));

static cl::opt<bool> PrintVecScenario(
    "vplan-print-vec-scenario", cl::init(false), cl::Hidden,
    cl::desc("Print selected single loop vectorization scenario"));

static cl::opt<unsigned> FavorAlignedToleranceNonDefaultTCEst(
  "vplan-favor-aligned-tolerance-non-default-tc", cl::init(2), cl::Hidden,
  cl::desc("Favor aligned tolerance percent value for non-default TC estimation"));

static cl::opt<unsigned> FavorAlignedMultiplierDefaultTCEst(
  "vplan-favor-aligned-multiplier-default-tc", cl::init(2), cl::Hidden,
  cl::desc("Favour aligned tolerance multiplier for default TC estimation"));

static cl::opt<bool, true> EnableIntDivRemBlendWithSafeValueOpt(
    "vplan-enable-int-divrem-blend-with-safe-value", cl::Hidden,
    cl::location(EnableIntDivRemBlendWithSafeValue),
        cl::desc("Enable blend with safe value for integer div/rem."));

static cl::opt<bool> CalcMinProfitableDynPeelTC(
    "vplan-calc-min-profitable-dyn-peel-tc", cl::Hidden, cl::init(true),
    cl::desc(
        "Whether to calculate and emit a minimum trip count check when dynamic "
        "peeling. At runtime, if the real TC is less than the value computed "
        "for that particular loop, peeling will be skipped."));

static cl::opt<bool> OptimizeProfitablePeelCheck(
    "vplan-optimize-profitable-peel-check", cl::Hidden, cl::init(false),
    cl::desc("Whether to optimize peeling and peel checks using the calculated "
             "minimum profitable peel TC. When enabled, if the profitable peel "
             "TC is greater than the estimated max TC, we don't emit a peel "
             "loop; if the profitable peel TC is lesser than the estimated min "
             "TC, we omit the peel TC check."));

static cl::opt<uint64_t> SkipDynamicPeelTC(
    "vplan-skip-dynamic-peel-tc", cl::init(0), cl::Hidden,
    cl::desc("When dynamic peeling, VPlan emits a trip count check to see if "
             "peeling should be skipped. This flag controls the maximum trip "
             "count value (overriding any computed value.)"));

static cl::opt<uint64_t> MaxProfitableDynPeelMultiplier(
    "vplan-max-profitable-dyn-peel-multiplier", cl::init(150), cl::Hidden,
    cl::desc(
        "When dynamic peeling, VPlan emits a trip count check to see if "
        "peeling should be skipped. This switch acts as a maximum threshold "
        "for the profitable trip count calculation. If the computed value is "
        "more than this value * VF * UF, the value is truncated."));

static cl::opt<bool> EnableDivergentBranchLoops(
    "vplan-enable-divergent-branches", cl::init(true), cl::Hidden,
    cl::desc("Enable vectorization of loops containing divergent branches."));

static cl::opt<bool> EnableMaskedRemTCRuntimeCheck(
    "vplan-enable-masked-rem-tc-runtime-check", cl::Hidden, cl::init(true),
    cl::desc("Prefer a scalar remainder loop if vector remainder is not "
             "profitable for the runtime TC value."));

static cl::opt<uint64_t>
    MaskedRemTCThreshold("vplan-masked-rem-tc-threshold", cl::Hidden,
                         cl::init(5),
                         cl::desc("Minimum TC value to consider switching to "
                                  "scalar remainder profitable."));

static LoopVPlanDumpControl LCSSADumpControl("lcssa", "LCSSA transformation");
static LoopVPlanDumpControl LoopCFUDumpControl("loop-cfu",
                                               "LoopCFU transformation");
static LoopVPlanDumpControl
    PredicatorDumpControl("predicator", "predicator");
static LoopVPlanDumpControl
    AllZeroBypassDumpControl("all-zero-bypass", "all zero bypass insertion");

static LoopVPlanDumpControl LoopMassagingDumpControl("loop-massaging",
                                                     "loop massaging");

static LoopVPlanDumpControl
    VPEntityInstructionsDumpControl("vpentity-instrs",
                                    "insertion of VPEntities instructions");

static LoopVPlanDumpControl
    ClearReductionWrapFlagsDumpControl("clear-redn-wrap-flags",
                                       "clearing wrap flags for reductions");

static LoopVPlanDumpControl
    InitialTransformsDumpControl("initial-transforms",
                                 "initial VPlan transforms");
static LoopVPlanDumpControl
    BlendWithSafeValueDumpControl("blend-with-safe-value",
                                 "blend integer div/rem with safe value");

static LoopVPlanDumpControl
    EarlyPeepholeDumpControl("early-peephole",
                             "Peephole transformation before predicator");

// The following two options are used together to temporarily disable
// vectorization for loops that have both properties.
static cl::opt<unsigned> NumGathersThreshold(
    "vplan-num-gathers-threshold", cl::init(20), cl::Hidden,
    cl::desc("Threshold of gathers used for disabling vectorization."));

static cl::opt<unsigned> NumVConflictThreshold(
  "vplan-num-vconflict-threshold", cl::init(3), cl::Hidden,
  cl::desc("Threshold of vconflict idioms used for disabling vectorization."));

static cl::opt<bool> EnableMaskedMainLoop(
    "vplan-enable-masked-main-loop", cl::init(true), cl::Hidden,
    cl::desc("Enable masked mode for the main loop for short TCs"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::list<unsigned> VPlanCostModelPrintAnalysisForVF(
    "vplan-cost-model-print-analysis-for-vf", cl::Hidden, cl::CommaSeparated,
    cl::ZeroOrMore,
    cl::desc("Print detailed VPlan Cost Model Analysis report for the given "
             "VF. For testing/debug purposes only."));

static cl::opt<bool, true> PrintAfterCallVecDecisionsOpt(
    "vplan-print-after-call-vec-decisions", cl::Hidden,
    cl::location(PrintAfterCallVecDecisions),
    cl::desc("Print VPlan after analyzing calls for vectorization decisions."));

static cl::opt<bool, true> PrintSVAResultsOpt(
    "vplan-print-scalvec-results", cl::Hidden, cl::location(PrintSVAResults),
    cl::desc("Print VPlan with results of ScalVec analysis."));

// Flag to enable SOA-analysis.
// TODO: Ideally, this should be in the IntelVPlanDriver file. In the future,
// consider moving it there.
static cl::opt<bool, true>
    EnableSOAAnalysisOpt("vplan-enable-soa", cl::Hidden,
                         cl::location(EnableSOAAnalysis),
                         cl::desc("Enable VPlan SOAAnalysis."));
// Flag to enable SOA-analysis for HIR.
// TODO: Ideally, this should be in the IntelVPlanDriver file. In the future,
// consider moving it there.
static cl::opt<bool, true>
    EnableSOAAnalysisHIROpt("vplan-enable-soa-hir", cl::Hidden,
                            cl::location(EnableSOAAnalysisHIR),
                            cl::desc("Enable VPlan SOAAnalysis for HIR."));

static cl::opt<bool>
    PrintAfterEvaluator("vplan-print-after-evaluator", cl::init(false),
                        cl::Hidden, cl::desc("Print VPlan after evaluator."));

static cl::opt<std::string> VecScenarioStr(
    "vplan-vec-scenario", cl::init(""), cl::Hidden,
    cl::desc(
        "Set vectorization scenario defining peel/main/remainder kinds and VF. "
        "UF is always set to 1.\n"
        "Format: peel-kindVF;main-kindVF;rem-kindVF[rem-kindVF]. kind=n,s,v,m, "
        "VF=number. E.g. n0;v4;v2s1 means no peel, main vector loop with VF=4, "
        "vector remainder with VF=2, scalar remainder"));

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
            "Debug option to force vector factor of a particular "
            "loop that we are going to vectorize. This applies to the main "
            "part of vector loop only and not to peel/remainder. This option "
            "is *NOT* thread-safe and might not have "
            "production quality! Loops order is also implementation-defined "
            "and might not match the order of the loops in the input. Refer to "
            "-debug-only=LoopVectorizationPlanner to see the mapping. It is "
            "expected to be deterministic between multiple runs of the same "
            "compiler invocation though. Comma separated list of pairs isn't "
            "supported at this moment - use the option multiple times to force "
            "vector factors for multiple loops."));

static cl::list<ForceVFTy, bool /* Use internal storage */, VPlanLoopVFParser>
    ForceLoopUF(
        "vplan-force-loop-uf", cl::Hidden, cl::ZeroOrMore,
        cl::value_desc("LoopID:UF"),
        cl::desc(
            "Debug option to force unroll factor of a particular "
            "loop that we are going to vectorize. This applies to the main "
            "part of vector loop only and not to peel/remainder. This option "
            "is *NOT* thread-safe and might not have "
            "production quality! Loops order is also implementation-defined "
            "and might not match the order of the loops in the input. Refer to "
            "-debug-only=LoopVectorizationPlanner to see the mapping. It is "
            "expected to be deterministic between multiple runs of the same "
            "compiler invocation though. Comma separated list of pairs isn't "
            "supported at this moment - use the option multiple times to force "
            "unroll factors for multiple loops."));

namespace {
// Descriptor of vectorization range.
struct VPlanVecRange {
  int Start = 0;
  int End = 0;
  // True for an exclusion range. I.e. all loops except [Start:End] interval.
  bool Inverse = false;
  bool isInRange(int Num) const {
    if (Inverse)
      return Num < Start || Num > End;
    else
      return Num >= Start && Num <= End;
  }

  VPlanVecRange(int S, int E, bool I) : Start(S), End(E), Inverse(I) {
    if (Start > End)
      std::swap(Start, End);
  }
  VPlanVecRange(const VPlanVecRange &R) = default;
  VPlanVecRange() = default;
};

struct VPlanVecRangeParser : public cl::parser<VPlanVecRange> {
  VPlanVecRangeParser(cl::Option &O) : cl::parser<VPlanVecRange>(O) {}

  bool parse(cl::Option &O, StringRef ArgName, StringRef Arg,
             VPlanVecRange &Result) {
    bool Inverse = Arg.consume_front("~");

    std::pair<StringRef, StringRef> StartEndPair = Arg.split(':');
    int Start = 0;
    if (StartEndPair.first.getAsInteger(10, Start))
      return O.error("Cannot parse Start for vplan range!");

    int End = Start;
    if (!StartEndPair.second.empty())
      if (StartEndPair.second.getAsInteger(10, End))
        return O.error("Cannot parse End for vplan range!");

    Result = {Start, End, Inverse};
    return false;
  }
};
} // namespace

static cl::list<VPlanVecRange, bool /* Use internal storage */,
                VPlanVecRangeParser>
    VecRange(
        "vplan-vec-range", cl::Hidden, cl::ZeroOrMore,
        cl::value_desc("Start:End, Inverse"),
        cl::desc(
            "Debug option to enable vectorization for select loops only. The "
            "loops are selected by their internal number. This number is shown "
            "in opt report when -vplan-report-loop-number is on. Syntax: a "
            "loop range can be defined as a single number or a pair "
            "'Start:End'. "
            "In both cases the optional '~' prefix means reverting the range. "
            "E.g. '2:3' means 'try vectorizing loop number 2 and 3, skipping "
            "other loops'. '~2:3' means 'all loops except 2 and 3'. '20' means "
            "'try vectorizing the loop number 20', '~20' means 'all loops "
            "except the loop number 20'. Comma separated list of "
            "vectorization ranges isn't supported at this moment - use the "
            "option multiple times to define several vectorization ramges."
            "In case when several ranges are defined the loop will be tried"
            "to vectorize if it is in any range. This option is *NOT *thread "
            "safe and might not have the production quality."));

using namespace llvm;
using namespace llvm::vpo;

namespace llvm {
namespace vpo {
bool PrintSVAResults = false;
bool PrintAfterCallVecDecisions = false;
bool LoopMassagingEnabled = true;
bool EnableSOAAnalysis = true;
bool EnableSOAAnalysisHIR = true;
bool VPlanEnablePeeling = false;
bool VPlanEnableGeneralPeeling = true;
bool EnableIntDivRemBlendWithSafeValue = true;

int LoopVectorizationPlanner::VPlanOrderNumber = 0;
} // namespace vpo
} // namespace llvm

/// Calculate the minimum trip count value at which peel does not give a
/// benefit. The goal is to solve for a scalar trip count such that:
///
///    [total-cost-if-peel-is-skipped] < [total-cost-if-peel-is-executed]
///
/// Breaking the two expressions into their constituent costs, we obtain an
/// inequality in terms of the vector trip-count, i.e:
///
///    [unaligned-main-cost] * [unaligned-vector-tc] + [unaligned-overhead]
///       <
///    [peel-overhead] + [aligned-main-cost] * [aligned-vector-tc] +
///    [aligned-overhead]
///
/// where [unaligned-main-cost] = cost of one vector iteration without peeling,
///       [aligned-main-cost]   = cost of one vector iteration with peeling,
///       [unaligned-overhead]  = cost of alignment checks + remainder w/o peel,
///       [aligned-overhead]    = cost of alignment checks + remainder,
///       [peel-overhead]       = total estimated cost of the peel loop.
///
/// By noting that [*-vector-tc] can be expressed as a function of the scalar
/// trip count and peel count, i.e:
///
///    [aligned-vector-tc]   = ([scalar-tc] - [peel-tc]) / (VF * UF)
///    [unaligned-vector-tc] = [scalar-tc] / (VF * UF)  (since [peel-tc] == 0)
///
/// And then solving for [scalar-tc], we obtain the following inequality:
///
///                 VF * UF * [overhead-diff] - [aligned-main-cost] * [peel-tc]
///  [scalar-tc] <  ----------------------------------------------------------
///                        [unaligned-main-cost] - [aligned-main-cost]
///
/// where [overhead-diff] = [peel-overhead] + [aligned-overhead]
///                       - [unaligned-overhead].
///
/// By estimating [peel-tc], and noting that the other quantities are known
/// constants, we can calculate a value for the right-hand side of the
/// inequality and obtain our threshold value.
///
static uint64_t calcMinProfitablePeelTC(VPInstructionCost PeelOverhead,
                                        VPInstructionCost AlignedMainCost,
                                        VPInstructionCost UnalignedMainCost,
                                        VPInstructionCost AlignedOverhead,
                                        VPInstructionCost UnalignedOverhead,
                                        unsigned MaxPeelTC, unsigned VF,
                                        unsigned UF) {
  const auto VFUF = VF * UF;

  // This quantity represents the cost of alignment without taking into account
  // the effect of the trip-count. Since a higher trip count always favors
  // peeling, if this quantity isn't positive, it means that *any* scalar trip
  // count for this loop will result in a gain when aligned.
  const float OverheadDiff = PeelOverhead.getFloatValue() +
                             AlignedOverhead.getFloatValue() -
                             UnalignedOverhead.getFloatValue();
  if (OverheadDiff <= 0) {
    DEBUG_WITH_TYPE(
        "LoopVectorizationPlanner_peel_tc",
        dbgs() << "Unaligned overhead is higher than peel + aligned overhead ("
               << UnalignedOverhead << " > " << PeelOverhead << " + "
               << AlignedOverhead << "): peeling is always profitable.\n");
    return 0;
  }

  // This quantity represents the cost of alignment, minus the expected gain
  // from skipping an iteration of the main loop. If this value isn't positive,
  // the result can't be positive either (since the denominator must be
  // positive); same as above, peeling must be profitable regardless of TC.
  const float Numerator = (float)VFUF * OverheadDiff -
                          AlignedMainCost.getFloatValue() * (float)MaxPeelTC;
  if (Numerator <= 0) {
    DEBUG_WITH_TYPE(
        "LoopVectorizationPlanner_peel_tc",
        dbgs() << "Numerator of computed peel threshold is not positive: "
                  "peeling is always profitable.");
    return 0;
  }

  // This quantity represents the expected gain from aligning the main loop.
  // This should never be negative.
  const float Denominator =
      UnalignedMainCost.getFloatValue() - AlignedMainCost.getFloatValue();
  assert(Denominator > 0 && "Unaligned loop costs less than aligned loop?");
  if (Denominator <= 0)
    return 0;

  // If the numerator and denominator are both positive, it should be impossible
  // for their quotient (the computed threshold) to be negative.
  assert(Numerator / Denominator > 0 && "Computed TC is somehow negative?");
  if (Numerator / Denominator <= 0)
    return 0;

  const auto ProfitableTC = (uint64_t)(Numerator / Denominator);
  DEBUG_WITH_TYPE("LoopVectorizationPlanner_peel_tc",
                  dbgs() << "(VF = " << VF << ", UF = " << UF
                         << ") min profitable peel tc = " << ProfitableTC
                         << "\n");

  // If our profitable TC is less than or equal to VF*UF, emit no check, as in
  // this case we would skip the peel loop anyways.
  if (ProfitableTC <= (uint64_t)VFUF) {
    DEBUG_WITH_TYPE(
        "LoopVectorizationPlanner_peel_tc",
        dbgs() << "Min profitable peel tc is less than VF*UF, skipping\n");
    return 0;
  }
  // Arbitrarily truncate values to some maximum factor of VF*UF, to avoid
  // values becoming too large.
  const auto MaxProfitableTC = MaxProfitableDynPeelMultiplier * (uint64_t)VFUF;
  if (ProfitableTC > MaxProfitableTC) {
    DEBUG_WITH_TYPE("LoopVectorizationPlanner_peel_tc",
                    dbgs() << "Truncating min profitable peel tc to "
                           << MaxProfitableTC << "\n");
    return MaxProfitableTC;
  }

  return ProfitableTC;
}

unsigned getForcedVF(const WRNVecLoopNode *WRLp) {
  auto ForcedLoopVFIter = llvm::find_if(ForceLoopVF, [](const ForceVFTy &Pair) {
    return Pair.first == LoopVectorizationPlanner::getVPlanOrderNumber();
  });
  if (ForcedLoopVFIter != ForceLoopVF.end())
    return ForcedLoopVFIter->second;

  if (VPlanForceVF)
    return VPlanForceVF;
  return WRLp && WRLp->getSimdlen() ? WRLp->getSimdlen() : VPlanTargetVF;
}

static unsigned getSafelen(const WRNVecLoopNode *WRLp) {
  return WRLp && WRLp->getSafelen() ? WRLp->getSafelen() : UINT_MAX;
}

void LoopVectorizationPlanner::setDefaultVectorFactors() {
  unsigned ForcedVF = getForcedVF(WRLp);
  if (ForcedVF && !isPowerOf2_64(ForcedVF)) {
    VFs.push_back(0);
    bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
            AuxRemarkID::VFNotPowerOf2);
    return;
  }

  unsigned Safelen = getSafelen(WRLp);

  LLVM_DEBUG(dbgs() << "LVP: ForcedVF: " << ForcedVF << "\n");
  LLVM_DEBUG(dbgs() << "LVP: Safelen: " << Safelen << "\n");

  // Early return from vectorizer if forced VF or safelen is 1
  if (ForcedVF == 1 || Safelen == 1) {
    VFs.push_back(0);
    bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
            AuxRemarkID::ForcedVFIs1);
    return;
  }

  if (ForcedVF) {
    if (ForcedVF > Safelen) {
      // We are bailing out of vectorization if ForcedVF > safelen
      VFs.push_back(0);
      bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
              AuxRemarkID::ForcedVFExceedsSafeLen);
      return;
    }
    VFs.push_back(ForcedVF);
    LLVM_DEBUG(dbgs() << "LVP: MinVF: " << ForcedVF << " MaxVF: " << ForcedVF
                      << "\n");

  } else if (VPlanConstrStressTest) {
    // If we are only stress testing VPlan construction, force VPlan
    // construction for just VF 1. This avoids any divide by zero errors in the
    // min/max VF computation.
    VFs.push_back(1);
  } else if (VectorlengthMD != nullptr) {
    extractVFsFromMetadata(Safelen);
    if (VFs.size() > 0 && VFs[0] == 0)
      bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
              AuxRemarkID::PragmaVectorLength0);
  } else {
    unsigned MinWidthInBits, MaxWidthInBits, MinVF, MaxVF;
    std::tie(MinWidthInBits, MaxWidthInBits) = getTypesWidthRangeInBits();
    const unsigned MinVectorWidth = TTI->getMinVectorRegisterBitWidth();
    const unsigned MaxVectorWidth =
        TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector)
            .getFixedValue();
    MaxVF = MaxVectorWidth / MinWidthInBits;
    MinVF = std::max(MinVectorWidth / MaxWidthInBits, 1u);

    // FIXME: Why is this?
    MaxVF = std::min(MaxVF, 32u);
    MinVF = std::min(MinVF, 32u);

    LLVM_DEBUG(dbgs() << "LVP: Orig MinVF: " << MinVF
                      << " Orig MaxVF: " << MaxVF << "\n");

    // Maximum allowed VF specified by user is Safelen
    Safelen = (unsigned)llvm::bit_floor(Safelen);
    MaxVF = std::min(MaxVF, Safelen);
    // We won't be able to fill the entire register, but it
    // still might be profitable.
    MinVF = std::min(MinVF, Safelen);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!VecScenarioStr.empty()) {
      VecScenario.fromString(VecScenarioStr);
      SmallSet<unsigned, 4> UsedVFs;
      VecScenario.getUsedVFs(UsedVFs);
      for (auto VF : UsedVFs) {
        if (VF > 1 && VF < MinVF)
          MinVF = VF;
        if (VF > 1 && VF > MaxVF)
          MaxVF = VF;
      }
    }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    LLVM_DEBUG(dbgs() << "LVP: MinVF: " << MinVF << " MaxVF: " << MaxVF
                      << "\n");
    if (MinVF > MaxVF) {
      VFs.push_back(0);
      std::stringstream SS;
      SS << INTERNAL("Calculated minimum vectorization factor ") << MinVF
         << INTERNAL(" exceeds calculated maximum vectorization factor ")
         << MaxVF << ".";
      bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
              SS.str());
      return;
    }
    for (unsigned VF = MinVF; VF <= MaxVF; VF *= 2)
      VFs.push_back(VF);
  }
  // TODO: add information about specified vector lengths in opt-report
  DEBUG_WITH_TYPE("LoopVectorizationPlanner_vec_lengths",
                  dbgs() << "LVP: Specified vectorlengths: ");
  DEBUG_WITH_TYPE("LoopVectorizationPlanner_vec_lengths",
                  for (unsigned VF
                       : getVectorFactors()) dbgs()
                      << VF << " ";
                  dbgs() << "\n";);
  return;
}

unsigned LoopVectorizationPlanner::buildInitialVPlans(
    Module *M, std::string VPlanName, AssumptionCache &AC,
    VPAnalysesFactoryBase &VPAF, ScalarEvolution *SE, bool IsLegalToVec) {

  ++VPlanOrderNumber;
  clearBailoutRemark();

  // Bail out if the loop is not in any selected vectorization range.
  if (!VecRange.empty() && !llvm::any_of(VecRange, [](const VPlanVecRange &R) {
        return R.isInRange(VPlanOrderNumber);
      })) {
    std::stringstream SS;
    SS << OptReportAuxDiag::getMsg(AuxRemarkID::OutOfVPlanVecRange) << " (#"
       << VPlanOrderNumber << ")";
    bailoutWithDebug(OptReportVerbosity::Medium,
                     OptRemarkID::VecFailUserExcluded, SS.str(),
                     AuxRemarkID::Loop);
    DEBUG_WITH_TYPE("LoopVectorizationPlanner_vec_range",
                    for (const auto &R
                         : VecRange) dbgs()
                        << "vec range: " << R.Start << ":" << R.End << " "
                        << R.Inverse << "\n";);
    return 0;
  }

  // Concatenate VPlan order number into VPlanName which allows to align
  // VPlans in all different VPlan internal dumps.
  VPlanName += ".#" + std::to_string(VPlanOrderNumber);

  setDefaultVectorFactors();
  if (VFs[0] == 0) {
    assert(BR.BailoutRemark &&
           "setDefaultVectorFactors did not set bailout data!");
    return 0;
  }

  Externals = std::make_unique<VPExternalValues>(M);
  UnlinkedVPInsts = std::make_unique<VPUnlinkedInstructions>();

  // TODO: revisit when we build multiple VPlans.
  std::shared_ptr<VPlanVector> Plan =
      buildInitialVPlan(*Externals, *UnlinkedVPInsts, VPlanName, AC, SE);
  if (!Plan) {
    LLVM_DEBUG(dbgs() << "LVP: VPlan was not created.\n");
    assert(BR.BailoutRemark && "buildInitialVPlan did not set bailout data!");
    return 0;
  }

  if (!IsLegalToVec) {
    bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            INTERNAL("Loop previously found illegal to vectorize."));
    return 0;
  }

  VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
  if (VFs.size() == 1) {
    // If we are dealing with a single valid VF either due to a
    // forced vf or due to vector register size and representative
    // type, bail out if this VF exceeds known loop trip count.
    if (!EnableMaskedMainLoop && !MainLoop->getTripCountInfo().IsEstimated &&
        MainLoop->getTripCountInfo().TripCount < VFs[0]) {
      bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
              AuxRemarkID::VFExceedsTC);
      return 0;
    }
  }

  raw_ostream *OS = nullptr;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  OS = is_contained(VPlanCostModelPrintAnalysisForVF, 1) ? &outs() : nullptr;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  std::tie(ScalarIterationCost, std::ignore) =
      createCostModel(Plan.get(), 1)
          ->getCost(false /* ForPeel */, nullptr /* PeelingVariant */, OS);

  // Reset the results of SVA that are set as a side effect of CM invocation.
  // The results are dummy (specific to VF = 1) and may not be automatically
  // reused in general.
  Plan->invalidateAnalyses(VPAnalysisID::SVA);

  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  auto Error = LE->getImportingError();
  if (Error != VPLoopEntityList::ImportError::None) {
    bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            std::string(LE->getImportErrorStr(Error)));
    return 0;
  }
  LE->analyzeImplicitLastPrivates();

  // Check legality of VPlan before proceeding with other transforms/analyses.
  if (!canProcessVPlan(*Plan.get())) {
    LLVM_DEBUG(dbgs() << "LVP: VPlan is not legal to process, bailing out.\n");
    assert(BR.BailoutRemark && "canProcessVPlan did not set bailout data!");
    return 0;
  }

  // If we have non-unit strided compress/expand idioms we are going to call
  // @llvm.x86.avx512.mask.compress.xxx/@llvm.x86.avx512.mask.expand.xxx
  // intrinsics. LLVM codegen only supports 128/256/512 total bit width for
  // them. So we need to make sure that VF*sizeof(element) value is valid.
  SmallSet<int, 5> CELoadStoreWidths;
  for (VPCompressExpandIdiom *CEIdiom : LE->vpceidioms()) {
    if (CEIdiom->getTotalStride() == 1)
      continue;
    for (auto LoadStore :
         concat<VPLoadStoreInst *const>(CEIdiom->loads(), CEIdiom->stores()))
      CELoadStoreWidths.insert(
          LoadStore->getValueType()->getPrimitiveSizeInBits());
  }
  VFs.erase(std::remove_if(
                  VFs.begin(), VFs.end(),
                  [&CELoadStoreWidths](unsigned VF) {
                    return llvm::any_of(CELoadStoreWidths, [VF](int Width) {
                      int TotalWidth = VF * Width;
                      return TotalWidth != 128 && TotalWidth != 256 &&
                             TotalWidth != 512;
                    });
                  }),
              VFs.end());
  if (VFs.empty()) {
    bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            INTERNAL("The loop contains a compress/expand idiom that "
                     "cannot be vectorized with any suitable vectorization "
                     "factor."));
    return 0;
  }

  // TODO: Implement computeScalarVPlanCost()

  // Run initial set of vectorization specific transforms on plain VPlan CFG
  // obtained from CFGBuilder. Includes the following set of transforms -
  // 1. emitVPEntityInstrs
  // 2. emitVecSpecifics
  runInitialVecSpecificTransforms(Plan.get());

  // Bail out if we still have any fence instructions at this time.  We will
  // have removed expected fences associated with in-scan reductions by now,
  // so any remaining fences are bad news.
  if (FencesInInput && fencesFound(Plan.get())) {
    bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailFence,
            WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                          : AuxRemarkID::Loop);
    return 0;
  }

  createLiveInOutLists(*Plan.get());

  // CFG canonicalization transform (merge loop exits).
  doLoopMassaging(Plan.get());

  // Exchange input and scan phases of exclusive scan loops.
  exchangeExclusiveScanLoopInputScanPhases(Plan.get());

  printAndVerifyAfterInitialTransforms(Plan.get());

  // Transformation to massage early-exit loops and explicitly track execution
  // mask and early-exit lane.
  if (Plan->isEarlyExitLoop()) {
    VPTransformEarlyExitLoop VPEETransform(*Plan.get());
    VPEETransform.transform();
  }

  // Populate initial VPlan analyses (e.g. VPlanValueTracking) to ensure
  // they are available when computing DA.
  VPAF.populateVPlanAnalyses(*Plan.get());
  assert(Plan->getVPVT() && "No VPVT?");

  auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
  Plan->setVPlanDA(std::move(VPDA));
  Plan->computeDT();
  Plan->computePDT();
  Plan->computeDA();

  // For VConflict idiom, we should bail-out for some VFs because vconflict
  // intrinsic might not be available.
  unsigned MaxVecRegSize =
      TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector)
          .getFixedValue();

  unsigned NumVConflictIdioms = 0;
  unsigned NumGathers = 0;
  auto *DA = Plan->getVPlanDA();

  for (auto &VPInst : vpinstructions(Plan.get())) {

    // Honor user request to disable vectorization of loops containing
    // divergent branches.  The common use case is for the sampling pass
    // of SPGO (sample-based profile-guided optimization).
    if (auto *Branch = dyn_cast<VPBranchInst>(&VPInst))
      if (!EnableDivergentBranchLoops && DA->isDivergent(*Branch)) {
        bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
                AuxRemarkID::DivergentBranchDisabled);
        return 0;
      }

    // TODO: We're losing out on being able to vectorize vconflict idioms at
    // VF=16 for i32 types because the conflict index is always represented
    // as i64. Look into preserving the original VPValue for the index as
    // live-in to VPGeneralMemOptConflict.
    if (auto *LoadStore = dyn_cast<VPLoadStoreInst>(&VPInst)) {
      if (LoadStore->getOpcode() == Instruction::Load &&
          !Plan->getVPlanDA()->isUnitStridePtr(LoadStore->getPointerOperand(),
                                               LoadStore->getValueType())) {
        ++NumGathers;
      }
    }
    if (auto *VPConflict = dyn_cast<VPGeneralMemOptConflict>(&VPInst)) {
      // Filter out all VFs if tree conflict region does not meet certain
      // criteria.
      VPInstruction *InsnInVConflictRegion =
          isSupportedVConflictRegion(VPConflict);
      if (!InsnInVConflictRegion) {
        VFs.clear();
        break;
      }

      unsigned VConflictIndexSizeInBits = VPConflict->getConflictIndex()
                                              ->getType()
                                              ->getPrimitiveSizeInBits()
                                              .getFixedValue();

      unsigned MaxVF = MaxVecRegSize / VConflictIndexSizeInBits;

      auto VFNotValidForConflict = [MaxVF, VConflictIndexSizeInBits,
                                    this](unsigned VF) {
        // VF is greater than max supported VF.
        if (VF > MaxVF)
          return true;

        // No intrinsics to perform conflict for 64-bit vector register.
        if (VF == 2 && VConflictIndexSizeInBits == 32)
          return true;

        // Target does not have AVX512VL feature in which case only 512-bit
        // vector register conflict is supported.
        if (!TTI->hasVLX() && VF * VConflictIndexSizeInBits != 512)
          return true;

        return false;
      };

      VFs.erase(std::remove_if(VFs.begin(), VFs.end(),
                               [VFNotValidForConflict](unsigned VF) {
                                 return VFNotValidForConflict(VF);
                               }),
                VFs.end());

      // Filter out any VPlans where permute intrinsics for TreeConflict
      // aren't available or are not currently supported.
      VPValue *RednUpdateOp =
          getReductionUpdateOp(VPConflict, InsnInVConflictRegion);
      assert(RednUpdateOp && "Could not find reduction update op");
      if (!Plan->getVPlanDA()->isUniform(*RednUpdateOp)) {
        ++NumVConflictIdioms;
        unsigned PermuteSize =
            RednUpdateOp->getType()->getPrimitiveSizeInBits();
        MaxVF = MaxVecRegSize / PermuteSize;
        // Case (PermuteSize < 32): We don't yet support type sizes less than
        // 32-bits.
        if (PermuteSize < 32) {
          VFs.clear();
          break;
        }

        // Case where the generated loop for tree conflict lowering would cause
        // allowed maximum loop nesting level to be exceeded. In order to
        // prevent this, the conflict instruction's VPLoop depth when added to
        // the nesting level of the original loop being vectorized should be
        // atmost equal to maximum loop nesting level.
        auto *VPLI = Plan->getVPLoopInfo();
        VPLoop *VPLp = VPLI->getLoopFor(VPConflict->getParent());
        assert(VPLp && "Conflict instruction outside VPLoop");

        if (Plan->exceedsMaxLoopNestingLevel(VPLp->getLoopDepth())) {
          VFs.clear();
          break;
        }

        // Case (VF < 4): No direct intrinsic mapping and pumping is required.
        VFs.erase(std::remove_if(VFs.begin(), VFs.end(),
                                 [MaxVF](unsigned VF) {
                                   return VF > MaxVF || VF < 4;
                                 }),
                  VFs.end());
      }

      continue;
    }

    // We have known precision issues when we have a reciprocal sqrt being
    // used with imf-accuracy-bits set to 14. To workaround these precision
    // issues, we avoid remainder vectorization when we see a call to sqrt
    // intrinsic for double type whose only uses are as the second operand
    // of a fdiv.
    if (auto *VPCall = dyn_cast<VPCallInstruction>(&VPInst)) {
      if (!VPCall->getType()->isDoubleTy())
        continue;

      Function *ScalarF = VPCall->getCalledFunction();
      if (!ScalarF)
        continue;

      if (ScalarF->getIntrinsicID() != Intrinsic::sqrt)
        continue;

      if (!VPCall->getOrigCallAttrs().hasFnAttr("imf-accuracy-bits-sqrt"))
        continue;

      StringRef AccuracyBitsStr = VPCall->getOrigCallAttrs()
                                      .getFnAttr("imf-accuracy-bits-sqrt")
                                      .getValueAsString();
      double AccuracyBitsValue = 0;
      if (AccuracyBitsStr.empty() ||
          AccuracyBitsStr.getAsDouble(AccuracyBitsValue) ||
          AccuracyBitsValue != 14)
        continue;

      if (all_of(VPCall->users(), [&](const VPUser *U) {
            auto *UserInst = dyn_cast<VPInstruction>(U);
            if (!UserInst)
              return false;
            if (UserInst->getOpcode() != Instruction::FDiv)
              return false;
            if (UserInst->getOperand(1) != VPCall)
              return false;
            return true;
          })) {
        LLVM_DEBUG(
            dbgs() << "Disabling remainder vectorization for FP precision\n");
        disableVecRemainder();
      }
    }
  }

  // TODO: workaround fix for CMPLRLLVM-38438, which is a 4% perf regression
  // for namd. Further investigation is required, filed CMPLRLLVM-40940.
  if (NumGathers >= NumGathersThreshold &&
      NumVConflictIdioms >= NumVConflictThreshold)
    VFs.clear();

  // When we force VF to have a special value, VFs vector has only one value.
  // Therefore, we have to check if we removed the only value that was in VFs.
  if (VFs.empty()) {
    bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            INTERNAL("No vectorization factor was found that can "
                     "satisfy all VConflict idioms in the loop."));
    return 0;
  }

  assert(!VFs.empty() && "The vector with VFs should have at least one value.");

  // TODO: Insert initial run of SVA here for any new users before CM & CG.

  // Need to create multiple VPlans with different VFs to hit the non-masked
  // remainder vectorization when simdlen(n) is specified. Those VPlans will be
  // used only during remainder vectorization, so main loop will be vectorized
  // with one VF specified in simdlen(n)
  bool CanRemainderBeVectorized =
      !isVecRemainderDisabled() &&
      (isVecRemainderEnforced() || EnableNonMaskedVectorizedRemainder);
  unsigned VFUF = VPlanForceUF ? VPlanForceUF * VFs[0] : VFs[0];

  if (WRLp && VFs.size() == 1 && WRLp->getSimdlen() &&
      CanRemainderBeVectorized &&
      (MainLoop->getTripCountInfo().IsEstimated ||
       (isDynAlignEnabled() &&
        MainLoop->getTripCountInfo().TripCount != VFUF) ||
       MainLoop->getTripCountInfo().TripCount % VFUF != 0)) {
    for (unsigned TmpVF = 1; TmpVF <= VFs[0]; TmpVF = TmpVF * 2) {
      VPlans[TmpVF] = {Plan, nullptr};
    }
  } else {
    for (unsigned TmpVF : VFs)
      VPlans[TmpVF] = {Plan, nullptr};
  }

  // Always capture scalar VPlan to handle cases where vectorization
  // is not possible with VF > 1 (such as when forced VF greater than TC).
  VPlans[1] = VPlans[VFs[0] /*MinVF*/];
  assert(VPlans[1].MainPlan != nullptr && "expected non-null VPlan");

  return 1;
}

void LoopVectorizationPlanner::createLiveInOutLists(VPlanVector &Plan) {
  VPLiveInOutCreator LICreator(Plan);
  LICreator.createInOutValues(TheLoop);
}

void LoopVectorizationPlanner::selectBestPeelingVariants() {
  // Note that dynamic alignment for peeling is strictly opt-in by design.
  // This is a deliberate difference between xmain and mainline compilers,
  // driven by negative experience in icc.  It's difficult to ensure
  // consistent gains, and it complicates cost modeling for the main and
  // remainder loops.
  bool EnableDP = isDynAlignEnabled();

  std::map<VPlanNonMasked *, VPlanPeelingAnalysis> VPPACache;

  for (auto &Pair : VPlans) {
    auto VF = Pair.first;
    auto *Plan = cast<VPlanNonMasked>(Pair.second.MainPlan.get());

    if (VF == 1)
      continue;

    auto Found = VPPACache.find(Plan);
    if (Found == VPPACache.end()) {
      VPlanPeelingAnalysis VPPA(*(Plan->getVPSE()), *(Plan->getVPVT()), *DL);
      VPPA.collectMemrefs(*Plan);
      Found = VPPACache.emplace(Plan, std::move(VPPA)).first;
    }

    VPlanPeelingAnalysis &VPPA = Found->second;

    std::unique_ptr<VPlanPeelingVariant> BestPeeling;
    if (IsVecAlign.has_value()) {
      if (IsVecAlign.value())
        BestPeeling = std::make_unique<VPlanNoPeelingAligned>(
            VPlanNoPeelingAligned::LoopObject);
      else
        BestPeeling = std::make_unique<VPlanNoPeelingUnaligned>(
            VPlanNoPeelingUnaligned::LoopObject);
    } else if (!VPlanEnableGeneralPeeling && !EnableDP) {
      BestPeeling =
          std::make_unique<VPlanNoPeeling>(VPlanNoPeeling::LoopObject);
    } else if (EnableGeneralPeelingCostModel) {
      std::unique_ptr<VPlanCostModelInterface> CMPtr =
        createCostModel(Plan, VF);
      VPlanPeelingCostModelGeneral PeelingCM(CMPtr.get());
      BestPeeling = VPPA.selectBestPeelingVariant(VF, PeelingCM, EnableDP);
    } else {
      VPlanPeelingCostModelSimple PeelingCM(*DL);
      BestPeeling = VPPA.selectBestPeelingVariant(VF, PeelingCM, EnableDP);
    }

    Plan->setPreferredPeeling(VF, std::move(BestPeeling));
  }
}

LoopVectorizationPlanner::PlannerType
LoopVectorizationPlanner::getPlannerType() const {
  if (TTI->isAdvancedOptEnabled(
        TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelSSE42))
    return PlannerType::Full;
  else
    return PlannerType::Base;
}

std::unique_ptr<VPlanCostModelInterface>
LoopVectorizationPlanner::createCostModel(const VPlanVector *Plan,
                                          unsigned VF, unsigned UF) const {
  // Do not run VLSA for VF = 1
  VPlanVLSAnalysis *VLSACM = VF > 1 ? VLSA : nullptr;
  const loopopt::DDGraph *DDG = getDDGraph();

  switch (getPlannerType()) {
    case PlannerType::Full:
      return VPlanCostModelFull::makeUniquePtr(Plan, VF, UF, TTI,
                                               TLI, DL, VLSACM, DDG);
    case PlannerType::LightWeight:
      return VPlanCostModelLite::makeUniquePtr(Plan, VF, UF, TTI,
                                               TLI, DL, VLSACM, DDG);
    case PlannerType::Base:
      return VPlanCostModelBase::makeUniquePtr(Plan, VF, UF, TTI,
                                               TLI, DL, VLSACM, DDG);
  }
  llvm_unreachable("Uncovered Planner type in the switch-case above.");
}

std::unique_ptr<VPlanCostModelInterface>
LoopVectorizationPlanner::createNoSLPCostModel(const VPlanVector *Plan,
                                               unsigned VF,
                                               unsigned UF) const {
  // Do not run VLSA for VF = 1
  VPlanVLSAnalysis *VLSACM = VF > 1 ? VLSA : nullptr;
  const loopopt::DDGraph *DDG = getDDGraph();

  switch (getPlannerType()) {
    case PlannerType::Full:
      return VPlanCostModelFullNoSLP::makeUniquePtr(Plan, VF, UF, TTI,
                                                    TLI, DL, VLSACM, DDG);
    case PlannerType::LightWeight:
      return VPlanCostModelLiteNoSLP::makeUniquePtr(Plan, VF, UF, TTI,
                                                    TLI, DL, VLSACM, DDG);
    case PlannerType::Base:
      return VPlanCostModelBaseNoSLP::makeUniquePtr(Plan, VF, UF, TTI,
                                                    TLI, DL, VLSACM, DDG);
  }
  llvm_unreachable("Uncovered Planner type in the switch-case above.");
}

unsigned LoopVectorizationPlanner::getLoopUnrollFactor(bool *Forced) {
  if (VPlanForceUF == 0) {

    if (UnrollCount) {
      if (Forced)
        *Forced = true;
      return *UnrollCount;
    }

    // Capture any UF forced through ForceLoopUF option.
    auto ForcedLoopUFIter =
        llvm::find_if(ForceLoopUF, [](const ForceVFTy &Pair) {
          return Pair.first == LoopVectorizationPlanner::getVPlanOrderNumber();
        });
    if (ForcedLoopUFIter != ForceLoopUF.end()) {
      if (Forced)
        *Forced = true;
      return ForcedLoopUFIter->second;
    }

    if (Forced)
      *Forced = false;
    return 1;
  }

  if (Forced)
    *Forced = true;
  return VPlanForceUF;
}

void LoopVectorizationPlanner::extractVFsFromMetadata(unsigned Safelen) {
  SmallVector<unsigned, 5> TmpVFs;
  for (unsigned I = 1; I < (VectorlengthMD->getNumOperands()); I++) {
    ConstantInt *IntMD =
        mdconst::extract<ConstantInt>(VectorlengthMD->getOperand(I));
    if (IntMD->getZExtValue() <= Safelen)
      TmpVFs.push_back(IntMD->getZExtValue());
  }

  llvm::sort(TmpVFs);
  auto NewEnd = std::unique(TmpVFs.begin(), TmpVFs.end());
  TmpVFs.erase(NewEnd, TmpVFs.end());

  unsigned I = 0;
  if (TmpVFs.size() > 1)
    if (TmpVFs[0] <= 1)
      I = 1;
  if (TmpVFs.size() > 2)
    if (TmpVFs[1] <= 1)
      I = 2;
  for (; I < TmpVFs.size(); I++) {
    // TODO: replace assert with call to the ErrorHandler when ErrorHandler is
    // implemented.
    assert((isPowerOf2_64(TmpVFs[I]) || TmpVFs[I] == 0) &&
           "Value specified in llvm.loop.vector.vectorlength metadata must be "
           "power of 2!");
    VFs.push_back(TmpVFs[I]);
  }
}

bool LoopVectorizationPlanner::hasVFOneInMetadata() const {
  if (!VectorlengthMD)
    return false;
  for (unsigned I = 1; I < (VectorlengthMD->getNumOperands()); I++) {
    ConstantInt *IntMD =
        mdconst::extract<ConstantInt>(VectorlengthMD->getOperand(I));
    if (IntMD->getZExtValue() == 1)
      return true;
  }
  return false;
}

ArrayRef<unsigned> LoopVectorizationPlanner::getVectorFactors() { return VFs; }

void LoopVectorizationPlanner::selectSimplestVecScenario(unsigned VF,
                                                         unsigned UF,
                                                         bool MainIsMasked,
                                                         bool AddRemainder) {
  VecScenario.resetPeel();
  VecScenario.resetRemainders();
  if (!MainIsMasked) {
    if (AddRemainder)
      VecScenario.addScalarRemainder();
    VecScenario.setVectorMain(VF, UF);
  } else {
    assert(UF == 1 && "unrolling not supported for masked mode of main loop");
    VecScenario.setMaskedMain(VF);
  }
}

std::optional<bool> LoopVectorizationPlanner::readVecRemainderEnabled() {
  if (findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.vecremainder")) {
    DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                     dbgs() << "Vector Remainder was set by the user's "
                               "#pragma vecremainder\n");
    return true;
  }
  if (findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.novecremainder")) {
    DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                     dbgs() << "Scalar Remainder was set by the user's #pragma "
                                "novecremainder\n");
    return false;
  }
  return std::nullopt;
}

std::optional<bool> LoopVectorizationPlanner::readVecAlignEnabled() {
  if (findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.aligned")) {
    DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                    dbgs() << "Vector Aligned was set by the user's "
                              "#pragma vector aligned\n");
    return true;
  }
  if (findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.unaligned")) {
    DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                    dbgs() << "Vector Unaligned was set by the user's #pragma "
                              "vector unaligned\n");
    return false;
  }
  return std::nullopt;
}

bool LoopVectorizationPlanner::readDynAlignEnabled() {
  if (findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.dynamic_align")) {
    DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                     dbgs() << "Dynamic Align was set by the user's "
                               "#pragma vector dynamic_align\n");
    return true;
  }
  if (findOptionMDForLoop(TheLoop, "llvm.loop.intel.vector.nodynamic_align")) {
    DEBUG_WITH_TYPE("VPlan_pragma_metadata",
                     dbgs() << "No dynamic Align was set by the user's "
                               "#pragma vector nodynamic_align\n");
    return false;
  }
  return VPlanEnableGeneralPeeling && VPlanEnablePeeling;
}

struct PeelDecision {
  bool ShouldPeel;
  bool PeelIsDynamic;
  std::string Scenario;
  std::string Formula;

  PeelDecision(const VPlanPeelEvaluator &PeelEvaluator,
               VPInstructionCost AlignedGain, VPInstructionCost UnalignedGain,
               VPInstructionCost ScalarCost, uint64_t TripCount,
               bool PeelIsDynamic, bool IsLoopTripCountEstimated)
      : PeelIsDynamic(PeelIsDynamic) {
    LLVM_DEBUG(dbgs() << "Using cost model to enable peeling. ");

    if (PeelEvaluator.getPeelLoopKind() ==
        VPlanPeelEvaluator::PeelLoopKind::None) {
      Scenario = "No peeling variants selected.";
      ShouldPeel = false;
    } else if (!IsLoopTripCountEstimated && !PeelIsDynamic) {
      // When trip count of main loop is known, rely on cost model completely.
      // Same for when the trip count of peel loops is known, which is not the
      // case for dynamic peeling. Cost model in such cases must assume worst
      // case of VF - 1 iterations. Let further tests below drive the decision
      // on whether or not to peel.
      Scenario = "Trip count is known; peel is static.";
      ShouldPeel = UnalignedGain < AlignedGain;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      llvm::raw_string_ostream{Formula}
          << "UnalignedGain < AlignedGain: " << UnalignedGain << " < "
          << AlignedGain;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    } else if (TripCount != DefaultTripCount) {
      // Otherwise favor aligned with some tolerance of scalar iterations.
      Scenario = "Trip count != default trip count";
      ShouldPeel =
          UnalignedGain <
          AlignedGain + ScalarCost *
                            FavorAlignedToleranceNonDefaultTCEst.getValue() /
                            100;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      llvm::raw_string_ostream{Formula}
          << "UnalignedGain < "
          << "(AlignedGain + (ScalarCost * "
          << "FavorAlignedToleranceNonDefaultTCEst / 100)): " << UnalignedGain
          << " < (" << AlignedGain << " + (" << ScalarCost << " * "
          << FavorAlignedToleranceNonDefaultTCEst << " / 100))";
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    } else if (UnalignedGain > 0 && AlignedGain > 0) {
      Scenario = "Trip count == default trip count; aligned/unaligned gains "
                 "are both positive.";
      ShouldPeel = UnalignedGain <
                   FavorAlignedMultiplierDefaultTCEst.getValue() * AlignedGain;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      llvm::raw_string_ostream{Formula}
          << "UnalignedGain < FavorAlignedMultiplierDefaultTCEst "
          << "* AlignedGain: " << UnalignedGain << " < "
          << FavorAlignedMultiplierDefaultTCEst << " * " << AlignedGain;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    } else {
      // We're here when one (or both) is <= 0. Perhaps, not peeling is correct.
      Scenario = "Trip count == default trip count; one of aligned/unaligned "
                 "gains is negative.";
      ShouldPeel = false;
    }
    LLVM_DEBUG(dbgs() << Scenario << "\n");
    if (!Formula.empty())
      LLVM_DEBUG(dbgs() << "ShouldPeel = " << (ShouldPeel ? "true" : "false")
                        << " [" << Formula << "]\n");
  }
};

/// Evaluate cost model for available VPlans and find the best one.
/// \Returns pair: VF which corresponds to the best VPlan (could be VF = 1) and
/// the corresponding VPlan.
std::pair<unsigned, VPlanVector *> LoopVectorizationPlanner::selectBestPlan() {
  selectBestPeelingVariants();

  VPlanVector *ScalarPlan = getVPlanForVF(1);
  assert(ScalarPlan && "There is no scalar VPlan!");
  LLVM_DEBUG(dbgs() << "Selecting VF for VPlan " <<
             ScalarPlan->getName() << '\n');

  // FIXME: Without peel and remainder vectorization, it's ok to get trip count
  // from the original loop. Has to be revisited after enabling of
  // peel/remainder vectorization.

  // Even if TripCount is more than 2^32 we can safely assume that it's equal
  // to 2^32, otherwise all cost modeling logic below will have a problem with
  // overflow. But using the forced max for decision whether a remainder loop
  // is needed is incorrect - this should be based on the actual trip count.
  // TODO - look into handling overflow in a better way.
  VPLoop *OuterMostVPLoop = ScalarPlan->getMainLoop(true);
  uint64_t OrigTripCount = OuterMostVPLoop->getTripCountInfo().TripCount;
  uint64_t TripCount =
      std::min(OrigTripCount, (uint64_t)std::numeric_limits<unsigned>::max());
  bool IsUserForcedUF = false;
  unsigned ForcedUF = getLoopUnrollFactor(&IsUserForcedUF);

  // TODO - search loop representation needs to be made explicit before we
  // can support unrolling such loops. Force unroll factor to 1 for search
  // loops.
  RegDDRef *PeelArrayRef = nullptr;
  auto SearchIdiom = VPlanIdioms::isSearchLoop(ScalarPlan, true, PeelArrayRef);
  if (VPlanIdioms::isAnySearchLoop(SearchIdiom)) {
    ForcedUF = 1;
    IsUserForcedUF = true;
  }

  // TODO - unrolling needs to support outer loops. Force unroll factor to 1
  // until this support is in place. TreeConflict instruction generates a loop.
  // We also need to suppress unroll when conflict instructions are seen.
  if (!OuterMostVPLoop->getSubLoops().empty() ||
      ScalarPlan->getTreeConflictUsed()) {
    ForcedUF = 1;
    IsUserForcedUF = true;
  }

  const auto TripCountInfo = OuterMostVPLoop->getTripCountInfo();
  const bool IsTripCountEstimated = TripCountInfo.IsEstimated;
  unsigned ForcedVF = getForcedVF(WRLp);

  // Get the unrolling preferences to determine the maximum UF to attempt,
  // and identify whether unrolling is enabled.
  // TODO - refine the choice of the default maximum UF; the current value
  // is ad-hoc.
  unsigned MaximumUF = VPlanMaximumUF;
  if (!IsUserForcedUF && !VPlanMaximumUF.getNumOccurrences()) {
    TargetTransformInfo::VectorUnrollingPreferences UP;
    TTI->getVectorUnrollingPreferences(UP);
    MaximumUF *= UP.MaximumUFScale;
  }

  // If we have a known trip count loop and ForcedVF * ForcedUF exceeds the
  // trip count, instead of bailing out of vectorization altogether give
  // precedence to ForcedVF.
  if (!IsTripCountEstimated && ForcedVF && ForcedUF > 1) {
    uint64_t VFUF = ForcedVF * ForcedUF;

    // We can also consider forcing UF to 1 here. The following will attempt
    // to unroll as much as possible assuming that this would still be helpful.
    if (VFUF > OrigTripCount) {
      ForcedUF = OrigTripCount / ForcedVF;
      if (ForcedUF == 0)
        ForcedUF = 1;
    }

    IsUserForcedUF = true;
  }

  // Reset the current selection to scalar loop only.
  VecScenario.resetPeel();
  VecScenario.resetMain();
  VecScenario.resetRemainders();

  // Populate UFs with the unroll factors to be considered. UF entries
  // must be non-decreasing and unique.
  SmallVector<unsigned, 8> UFs;
  if (IsUserForcedUF)
    UFs.push_back(ForcedUF);
  else {
    // Here ForcedUF may come from a hint, which we consider
    // as the minimum UF to try. We consider ForcedUF and
    // all powers-of-two that aren't less than MaxUF.
    unsigned MaxUF = std::max(ForcedUF, MaximumUF);
    UFs.push_back(ForcedUF);
    for (unsigned i = NextPowerOf2(ForcedUF); i > 0 && i <= MaxUF; i *= 2)
      UFs.push_back(i);
  }

  if (ForcedVF > 0) {
    // Prune UFs which exceed the trip count for the forced VF.
    UFs.erase(std::remove_if(UFs.begin(), UFs.end(),
                             [TripCount, ForcedVF](unsigned UF) {
                               auto UFVF = (uint64_t)ForcedVF * (uint64_t)UF;
                               return UFVF > TripCount && UF != 1;
                             }),
              UFs.end());
    if (UFs.empty()) {
      std::stringstream SS;
      SS << "Bailing out to scalar VPlan because ForcedVF(" << ForcedVF
         << ") * U > TripCount(" << TripCount << ") for all unroll factors U";
      bailoutWithDebug(OptReportVerbosity::Medium,
                       OptRemarkID::VecFailGenericBailout, SS.str(),
                       AuxRemarkID::ForcedVFExceedsUnrolledTC);
      // The scenario was reset just before the check.
      return std::make_pair(VecScenario.getMainVF(), getBestVPlan());
    }

    // FIXME: this code should be revisited later to select best UF
    // even with forced VF.
    // FIXME: We may want to use BestUF in the Unroller.
    LLVM_DEBUG(dbgs() << "There is only VPlan with VF=" << ForcedVF
                      << ", selecting it.\n");
    if (ForcedVF == 1)
      bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
              AuxRemarkID::UserForcedVF1);
  }

  // In light weight and advanced modes select VF basing on Search Loop idioms
  // unless VF is forced elsehow.
  //
  // TODO: The VF selection code and Evaluators have to be taught to deal with
  // UnknownCost in ScalarIterationCost/ScalarCost. Once it is done we should
  // be able to remove explicit checks for ForcedVF/SearchLoopPreferredVF from
  // VF selection loop and manage available VFs through VFs[] alone. Meanwhile
  // SearchLoopPreferredVF has to be visible to VF selection loop.
  unsigned SearchLoopPreferredVF = 0;
  if (ForcedVF == 0 && getPlannerType() != PlannerType::Base) {
    switch (SearchIdiom) {
    case VPlanIdioms::Unsafe:
      SearchLoopPreferredVF = 1;
      break;
    case VPlanIdioms::SearchLoopStrEq:
      SearchLoopPreferredVF = 32;
      break;
    case VPlanIdioms::SearchLoopPtrEq: {
      // Use higher VF=16 for targets that have 2K or higher DSB size. Check
      // CMPLRLLVM-38144 for more details.
      if (TTI->has2KDSB())
        VPlanSearchLpPtrEqForceVF = 16;

      SearchLoopPreferredVF = VPlanSearchLpPtrEqForceVF;
      break;
    }
    case VPlanIdioms::SearchLoopValueCmp:
      // Intentional fall-through.
    default:
      break;
    }

    // Check the VPlan availability and remove VFs that are not preferred.
    if (SearchLoopPreferredVF > 0 && hasVPlanForVF(SearchLoopPreferredVF)) {
      VFs.erase(std::remove_if(VFs.begin(), VFs.end(),
                               [SearchLoopPreferredVF](unsigned VF) {
                                 return VF != SearchLoopPreferredVF;
                               }),
                VFs.end());
      LLVM_DEBUG(dbgs() << "Search Loop Preferred VF="
                 << SearchLoopPreferredVF << "\n");
      // Early return from the routine for VF = 1 as the code below does not
      // expect VF = 1 for ForcedVF or SearchLoopPreferredVF.
      // TODO: We may want to make the same return for all other values of
      // SearchLoopPreferredVF. That requires updating VecScenario structure
      // so getBestVF()/getBestVPlan() utilites can work properly.
      if (SearchLoopPreferredVF == 1) {
        bailoutWithDebug(OptReportVerbosity::Medium,
                         OptRemarkID::VecFailBadlyFormedMultiExitLoop,
                         INTERNAL("Selecting VPlan with search loop preferred "
                                  "VF=1"),
                         WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                       : AuxRemarkID::Loop);
        return std::make_pair(VecScenario.getMainVF(), getBestVPlan());
      }

      // Prune UFs which would exceed the trip count for the preferred VF.
      UFs.erase(std::remove_if(UFs.begin(), UFs.end(),
                               [SearchLoopPreferredVF, TripCount](unsigned UF) {
                                 return (uint64_t)SearchLoopPreferredVF *
                                            (uint64_t)UF >
                                        TripCount;
                               }),
                UFs.end());
      if (UFs.empty()) {
        std::stringstream SS;
        SS << INTERNAL("Bailing out to scalar VPlan because ")
           << INTERNAL("SearchLoopPreferredVF(") << SearchLoopPreferredVF
           << INTERNAL(") * U > TripCount(") << TripCount
           << INTERNAL(") for all unroll factors U\n");
        bailoutWithDebug(OptReportVerbosity::High,
                         OptRemarkID::VecFailGenericBailout, SS.str(),
                         INTERNAL("Trip count too low for search loop."));
        // The scenario was reset just before the check.
        return std::make_pair(VecScenario.getMainVF(), getBestVPlan());
      }
    }
    else
      SearchLoopPreferredVF = 0;
  }

  raw_ostream *OS = nullptr;
  VPInstructionCost ScalarCost = ScalarIterationCost * TripCount;

  if (ForcedVF > 0 || SearchLoopPreferredVF > 0) {
    assert((VFs.size() == 1 &&
            (VFs[0] == ForcedVF || VFs[0] == SearchLoopPreferredVF) &&
            hasVPlanForVF(VFs[0])) &&
           "Expected only one forced/preferred VF and non-null VPlan");
  }

  VPInstructionCost BestCost = ScalarCost;
  LLVM_DEBUG(dbgs() << "Cost of Scalar VPlan: " << ScalarCost << '\n');

  // When a loop is marked with pragma vector always, the user wants the loop
  // to be vectorized. In order to force the loop to be vectorized, we set
  // BestCost to MAX value for such a case. WRLp can be null when stress testing
  // vector code generation.
  bool ShouldIgnoreProfitability =
      WRLp && (WRLp->getHasVectorAlways() || WRLp->isOmpSIMDLoop());

  bool IsVectorAlways = ShouldIgnoreProfitability || VecThreshold == 0;

  if (IsVectorAlways) {
    BestCost = VPInstructionCost::getInvalid();
    LLVM_DEBUG(dbgs() << "'#pragma vector always'/ '#pragma omp simd' is used "
                         "for the given loop\n");
  }

  // FIXME: Currently limit this to VF = 16 for HIR path. Has to be fixed with
  // more accurate cost model. Still allow forced VFs to enter the loop below.
  if (getPlannerType() != PlannerType::Base)
    VFs.erase(
      std::remove_if(
        VFs.begin(), VFs.end(),
        [ForcedVF, SearchLoopPreferredVF](unsigned VF) {
          return VF > 16 && VF != ForcedVF && VF != SearchLoopPreferredVF;
        }),
      VFs.end());

  // Record the best cost summary for opt reporting. The associated flag
  // is required to preserve old behaviour where the report is not
  // emitted when no costs are valid.
  VPCostSummary BestCostSummary;
  bool BestCostSummarySet = false;

  // Add protection from MinVF evaluated to 1 when dealing with 'vector always'
  // and 'simd' loops. Otherwise, if VF=1 cost is lower than VF>1 costs, VF=1
  // will be selected. Do not remove VF=1 when it is forced.
  if (ShouldIgnoreProfitability && (ForcedVF != 1) && !hasVFOneInMetadata())
    VFs.erase(std::remove_if(VFs.begin(), VFs.end(),
                             [](unsigned VF) { return (VF == 1); }),
              VFs.end());

  //
  // The main loop where we choose VF and UF.
  // Please note that best peeling variant is expected to be selected up to
  // this moment.  Now we check whether the best peeling is profitable VS no
  // peeling.
  //
  // The rules for picking best VF follows:
  // 1. ForcedVF / SearchLoopPreferredVF and ForcedUF wins always even if
  //    the corresponding Plan is of Unknown/Invalid cost.
  // 2. IsVectorAlways = true gives preference to VF > 1 Plans.
  // 3. The least valid cost wins.
  // 4. Unknown cost wins over Invalid cost.
  // 5. The cases of tie:
  //      *) all costs are valid and equal.
  //      *) all costs are Unknown.
  //      *) all costs are Invalid.
  //    In case of tie VF = 1 wins unless IsVectorAlways is true.
  //    In case of tie and IsVectorAlways is true the first available VF
  //    is selected.
  //    The lowest (first available) UF is used in either case.
  for (unsigned VF : getVectorFactors()) {
    bool UseMasked = false;
    for (unsigned UF : UFs) {
      assert(hasVPlanForVF(VF) && "expected non-null VPlan");
      if (TripCount < (uint64_t)VF * (uint64_t)UF &&
          !(EnableMaskedMainLoop && UF == 1 && hasMaskedVPlanForVF(VF)))
        continue; // Consider masked low trip when UF == 1.

      UseMasked = TripCount < VF;
      VPlanVector *Plan =
          UseMasked ? getMaskedVPlanForVF(VF) : getVPlanForVF(VF);
      assert(Plan && "Unexpected null VPlan");

      LLVM_DEBUG(dbgs() << "Using " << (UseMasked ? "" : "un")
                        << "masked VPlan, VF=" << VF << " TC=" << TripCount
                        << "\n";);

      // Calculate cost for one iteration of the main loop.
      auto MainLoopCM = IsVectorAlways || ForcedVF > 1
                            ? createNoSLPCostModel(Plan, VF, UF)
                            : createCostModel(Plan, VF, UF);
      VPlanPeelingVariant *PeelingVariant = Plan->getPreferredPeeling(VF);

      if (UseMasked)
        assert(!PeelingVariant && "Expected no peeling for masked variant");

      // Peeling is not supported for non-normalized loops.
      VPLoop *L = Plan->getMainLoop(true);
      if (!L->hasNormalizedInduction())
        PeelingVariant = &VPlanNoPeeling::LoopObject;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      OS = is_contained(VPlanCostModelPrintAnalysisForVF, VF) ? &outs()
                                                              : nullptr;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

      VPInstructionCost MainLoopIterationCost, MainLoopOverhead;

      std::tie(MainLoopIterationCost, MainLoopOverhead) =
          MainLoopCM->getCost(false /*ForPeel */, PeelingVariant, OS);

      if (!MainLoopIterationCost.isValid() || !MainLoopOverhead.isValid()) {
        LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << ", UF = " << UF
                          << " is unknown. Skip it.\n");
        if (VF == ForcedVF || VF == SearchLoopPreferredVF) {
          // If the VF is forced and loop cost for it is unknown/invalid select
          // the simplest configuration: non-masked main loop + scalar
          // remainder.
          selectSimplestVecScenario(
              VF, UF, UseMasked,
              !UseMasked && (IsTripCountEstimated ||
                             (TripCount % VF) != 0) /* addRemainder*/);
        } else
          LLVM_DEBUG(dbgs()
                     << "Cost for VF = " << VF << ", UF = " << UF << " is "
                     << MainLoopIterationCost << ". Skip it.\n");
        continue;
      }
      assert(MainLoopIterationCost >= 0 && MainLoopOverhead >= 0 &&
             "Loop costs must be non-negative");
      LLVM_DEBUG(
        dbgs() << "Selected peeling: ";
        if (PeelingVariant) {
          if (isa<VPlanDynamicPeeling>(PeelingVariant))
            dbgs() << "Dynamic\n";
          else if (isa<VPlanNoPeeling>(PeelingVariant))
            dbgs() << "Disabled\n";
          else if (isa<VPlanNoPeelingAligned>(PeelingVariant))
            dbgs() << "Disabled(Aligned)\n";
          else if (isa<VPlanNoPeelingUnaligned>(PeelingVariant))
            dbgs() << "Disabled(Unaligned)\n";
          else
            dbgs() << "Static("
                   << cast<VPlanStaticPeeling>(PeelingVariant)->peelCount()
                   << ")\n";
        } else
            dbgs() << "None\n";
      );

      // Calculate the total cost of peel loop if there is one.
      VPlanPeelEvaluator PeelEvaluator(*this, ScalarIterationCost, TLI, TTI, DL,
                                       VLSA, VF, PeelingVariant);
      // Calculate the total cost of remainder loop if there is one.
      bool PeelIsDynamic =
          PeelingVariant ? isa<VPlanDynamicPeeling>(PeelingVariant) : false;
      VPlanRemainderEvaluator RemainderEvaluator(
          *this, ScalarIterationCost, TLI, TTI, DL, VLSA, OrigTripCount,
          IsTripCountEstimated, PeelEvaluator.getTripCount(), PeelIsDynamic, VF,
          UF);

      // Calculate main loop's trip count. Currently, the unroll factor is set
      // to 1 because VPlan's loop unroller is called after selecting the best
      // VF.
      decltype(TripCount) MainLoopTripCount =
          (TripCount - PeelEvaluator.getTripCount()) / (VF * UF);

      if (MainLoopTripCount == 0 && PeelEvaluator.getTripCount() == 0) {
        assert(UseMasked && "Expected masked VPlan for VF > TC");
        MainLoopTripCount = 1;
      }
      // The total vector cost is calculated by adding the total cost of peel,
      // main and remainder loops.
      VPInstructionCost VectorCost = PeelEvaluator.getLoopCost() +
                                     MainLoopIterationCost * MainLoopTripCount +
                                     MainLoopOverhead +
                                     RemainderEvaluator.getLoopCost();

      // Calculate cost of one iteration of the main loop without preferred
      // alignment.
      // This getCost() call leaves no traces in CM dumps enabled by
      // VPlanCostModelPrintAnalysisForVF for now. May want to reconsider in
      // future.
      VPInstructionCost MainLoopIterationCostWithoutPeel,
          MainLoopOverheadWithoutPeel;
      // Create new CM instead of reusing already created one. This is a
      // temporary workaround for the problem with SVA and CallVecDecision, they
      // are kept in VPlan but VPlan can be reused for another VF estimation by
      // peel/remainder evaluators.
      //
      auto MainLoopCMNoPeel = IsVectorAlways || ForcedVF > 1
                                  ? createNoSLPCostModel(Plan, VF, UF)
                                  : createCostModel(Plan, VF, UF);
      std::tie(MainLoopIterationCostWithoutPeel, MainLoopOverheadWithoutPeel) =
          MainLoopCMNoPeel->getCost(false /*ForPeel */, nullptr /*peel*/, OS);
      if (MainLoopIterationCostWithoutPeel.isUnknown() ||
          MainLoopOverheadWithoutPeel.isUnknown()) {
        LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << ", UF = " << UF
                          << " without peel is unknown. Skip it.\n");
        continue;
      }
      assert(MainLoopIterationCostWithoutPeel >= 0 &&
             MainLoopOverheadWithoutPeel >= 0 &&
             "Loop costs must be non-negative");

      // Calculate the total cost of remainder loop having no peeling, if there
      // is one.
      VPlanRemainderEvaluator RemainderEvaluatorWithoutPeel(
          *this, ScalarIterationCost, TLI, TTI, DL, VLSA, OrigTripCount,
          IsTripCountEstimated, 0 /*Peel trip count */,
          false /*no dynamic peeling*/, VF, UF);

      decltype(TripCount) MainLoopTripCountWithoutPeel = TripCount / (VF * UF);
      if (MainLoopTripCountWithoutPeel == 0) {
        assert(UseMasked && "Expected masked VPlan for VF > TC");
        MainLoopTripCountWithoutPeel = 1;
      }

      VPInstructionCost VectorCostWithoutPeel =
          MainLoopIterationCostWithoutPeel * MainLoopTripCountWithoutPeel +
          MainLoopOverheadWithoutPeel +
          RemainderEvaluatorWithoutPeel.getLoopCost();

      if (0 < VecThreshold && VecThreshold < 100) {
        LLVM_DEBUG(dbgs() << "Applying threshold " << VecThreshold << " for VF "
                          << VF << ", UF " << UF
                          << ". Original cost = " << VectorCost << '\n');
        VectorCost = (VectorCost * VecThreshold.getValue()) / 100;
        VectorCostWithoutPeel =
            (VectorCostWithoutPeel * VecThreshold.getValue()) / 100;
      }

      VPInstructionCost GainWithPeel = ScalarCost - VectorCost;
      VPInstructionCost GainWithoutPeel = ScalarCost - VectorCostWithoutPeel;

      PeelDecision PeelingDecision(PeelEvaluator, GainWithPeel, GainWithoutPeel,
                                   ScalarCost, TripCount, PeelIsDynamic,
                                   IsTripCountEstimated);

      VPInstructionCost VectorGain =
          PeelingDecision.ShouldPeel ? GainWithPeel : GainWithoutPeel;
      // Calculate speedup relatively to single scalar iteration.
      assert((ScalarIterationCost - VectorGain / TripCount) != 0 &&
             "Division by zero computing vector speedup");
      VPInstructionCost Speedup =
          ScalarIterationCost / (ScalarIterationCost - VectorGain / TripCount);
      VPInstructionCost VectorIterationCost =
          PeelingDecision.ShouldPeel ? MainLoopIterationCost
                                     : MainLoopIterationCostWithoutPeel;
      VectorIterationCost = VectorIterationCost / (VF * UF);
      VPInstructionCost VectorInitFini = PeelingDecision.ShouldPeel
                                             ? MainLoopOverhead
                                             : MainLoopOverheadWithoutPeel;

      const char CmpChar = ScalarCost < VectorCost    ? '<'
                           : ScalarCost == VectorCost ? '='
                                                      : '>';
      (void)CmpChar;
      LLVM_DEBUG(
          dbgs() << "Scalar Cost = " << TripCount << " x "
                 << ScalarIterationCost << " = " << ScalarCost << ' ' << CmpChar
                 << " VectorCost = " << PeelEvaluator.getLoopCost() << " + "
                 << MainLoopTripCount << " x " << MainLoopIterationCost << " + "
                 << MainLoopOverhead << " + "
                 << RemainderEvaluator.getLoopCost() << " = " << VectorCost
                 << '\n'
                 << "Peel loop cost = " << PeelEvaluator.getLoopCost() << " ("
                 << PeelEvaluator.getPeelLoopKindStr() << ")"
                 << "\n"
                 << "Main loop vector cost = "
                 << MainLoopTripCount * MainLoopIterationCost << " + "
                 << MainLoopOverhead << "\n"
                 << "Remainder loop cost = " << RemainderEvaluator.getLoopCost()
                 << " (" << RemainderEvaluator.getRemainderLoopKindStr() << ")"
                 << "\n"
                 // Print out costs without peel.
                 << " VectorCostWithoutPeel = " << MainLoopTripCountWithoutPeel
                 << " x " << MainLoopIterationCostWithoutPeel << " + "
                 << MainLoopOverheadWithoutPeel << " = "
                 << VectorCostWithoutPeel << "\n"
                 << "Main loop vector cost without peel = "
                 << MainLoopIterationCostWithoutPeel *
                            MainLoopTripCountWithoutPeel +
                        MainLoopOverheadWithoutPeel
                 << "\n"
                 << "Remainder loop cost without peel = "
                 << RemainderEvaluatorWithoutPeel.getLoopCost() << " ("
                 << RemainderEvaluatorWithoutPeel.getRemainderLoopKindStr()
                 << ")"
                 << "\n";);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (PrintAfterEvaluator) {
        dbgs() << "Evaluators for VF=" << VF << "\n";
        PeelEvaluator.dump();
        dbgs() << "The main loop is vectorized with vector factor " << VF
               << ". The vector cost is "
               << MainLoopTripCount * MainLoopIterationCost + MainLoopOverhead
               << "(" << MainLoopTripCount << " x " << MainLoopIterationCost
               << " + " << MainLoopOverhead << "). \n";
        RemainderEvaluator.dump();
      }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

      // If we've chosen to dynamically peel, but the trip count isn't known,
      // compute a minimum profitable peel TC so we can emit a run-time
      // profitability check.
      uint64_t MinProfitablePeelTC = 0;
      if (CalcMinProfitableDynPeelTC && PeelingDecision.ShouldPeel &&
          PeelingDecision.PeelIsDynamic && IsTripCountEstimated) {
        // TODO: should we still calculate the min profitable peeling TC when
        // trip count is known, and assert that it is less than the known TC?
        LLVM_DEBUG(
            dbgs() << "Dynamic peel chosen with unknown (estimated) trip "
                      "count; computing TC for profitable peel check.\n");
        MinProfitablePeelTC = calcMinProfitablePeelTC(
            /*PeelOverhead=*/PeelEvaluator.getLoopCost(),
            /*AlignedMainCost=*/MainLoopIterationCost,
            /*UnalignedMainCost=*/MainLoopIterationCostWithoutPeel,
            /*AlignedOverhead=*/MainLoopOverhead +
                RemainderEvaluator.getLoopCost(),
            /*UnalignedOverhead=*/MainLoopOverheadWithoutPeel +
                RemainderEvaluatorWithoutPeel.getLoopCost(),
            /*MaxPeelTC=*/PeelEvaluator.getTripCount(), VF, UF);

        // Using the threshold TC, further tune peeling decisions. In
        // particular, determine if we should omit the peel or the run-time
        // check.
        if (OptimizeProfitablePeelCheck && MinProfitablePeelTC != 0) {
          if (MinProfitablePeelTC > TripCountInfo.MaxTripCount) {
            LLVM_DEBUG(dbgs()
                       << "Calculated min profitable peel TC ("
                       << MinProfitablePeelTC
                       << ") is "
                          "greater than the expected max TC ("
                       << TripCountInfo.MaxTripCount
                       << ").\nPeel is likely unprofitable: disabling.\n");
            PeelingDecision.ShouldPeel = false;
          } else if (MinProfitablePeelTC < TripCountInfo.MinTripCount) {
            LLVM_DEBUG(
                dbgs()
                << "Calculated min profitable peel TC (" << MinProfitablePeelTC
                << ") is less than the expected min TC ("
                << TripCountInfo.MinTripCount
                << ").\nPeel is likely profitable: disabling TC check.\n");
            MinProfitablePeelTC = 0;
          }
        }
      }

      if (!PeelingDecision.ShouldPeel) {
        PeelEvaluator.disable();
        // Total cost is cost without peeling.
        VectorCost = VectorCostWithoutPeel;
        LLVM_DEBUG(dbgs() << "Peeling will not be performed.\n");
      } else {
        LLVM_DEBUG(dbgs() << "Peeling will be performed.\n");
      }

      if (UseMasked && MaskedMainThreshold != 0 && ForcedVF <= 1) {
        // Restrict gain for masked main loop by a threshold.
        VPInstructionCost MinReqGain =
            (ScalarCost * VPInstructionCost(MaskedMainThreshold)) / 100.;
        if (VectorGain < MinReqGain) {
          LLVM_DEBUG(dbgs() << "Masked main loop gain (" << VectorGain
                            << ") is less than threshold (" << ScalarCost << "*"
                            << MaskedMainThreshold << ")/100=" << MinReqGain
                            << ", skipping.\n");
          VectorCost = VPInstructionCost::getInvalid();
        }
      }

      // Current VF is invalid to vectorize with so skip it.
      if (VectorCost.isInvalid())
        continue;

      // The checks below implement the rules for selecting the best VF.
      if ((VectorCost.isUnknown() && BestCost.isInvalid()) ||
          (VectorCost.isValid() && !BestCost.isValid()) ||
          (VectorCost.isValid() && BestCost.isValid() &&
           VectorCost < BestCost) ||
          VF == ForcedVF || VF == SearchLoopPreferredVF) {
        BestCostSummary = VPCostSummary(
            ScalarIterationCost, VectorIterationCost, Speedup, VectorInitFini,
            VPPeelSummary{PeelingDecision.Scenario, PeelingDecision.Formula,
                          PeelingDecision.ShouldPeel,
                          PeelingDecision.PeelIsDynamic ? "dynamic" : "static",
                          GainWithPeel, GainWithoutPeel});
        BestCostSummarySet = true;
        BestCost = VectorCost;
        updateVecScenario(PeelEvaluator,
                          PeelingDecision.ShouldPeel
                              ? RemainderEvaluator
                              : RemainderEvaluatorWithoutPeel,
                          VF, UF, isa<VPlanMasked>(Plan));
        if (VecScenario.hasPeel()) {
          if (MinProfitablePeelTC != 0)
            LLVM_DEBUG(dbgs() << "Using min profitable peel TC: "
                              << MinProfitablePeelTC << "\n");
          VecScenario.setMinimumProfitablePeelTC(MinProfitablePeelTC);
        } else {
          VecScenario.setMinimumProfitablePeelTC(0);
        }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        if (PrintVecScenario) {
          dbgs() << "Updated scenario for VF: " << VF << ", UF: " << UF << "\n";
          VecScenario.dump();
        }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
      }
    }
    if (UseMasked) {
      // If we achieved the first VF greater than known TC we should not
      // continue, i.e. it does not make sense to try VF=8 for TC=3 if VF=4 can
      // be used.
      break;
    }
  }
  // Corner case: all available VPlans have Invalid cost.
  // With 'vector always' we have to vectorize with some VF, so select first
  // available VF.
  if (getBestVF() == 1 && IsVectorAlways) {
    bool IsMasked = !IsTripCountEstimated && TripCount < VFs[0] &&
                    EnableMaskedMainLoop && hasMaskedVPlanForVF(VFs[0]);

    selectSimplestVecScenario(VFs[0], 1, IsMasked,
                              !IsMasked &&
                                  (IsTripCountEstimated ||
                                   (TripCount % VFs[0]) != 0) /*addRemainder*/);
  }

  // Workaround for using DefaultTripCount: in some cases we can have situation
  // when static peeling and current value of DefaultTripCount can lead to
  // incorrect decision "the remainder is not needed". E.g. we have
  // DefaultTripCount = 303, with static peel count 3 we have tc = 300, and for
  // VF=4 or VF=2 we have no iterations for remainder.
  // The correct fix should be made in remainder evaluator, but that fix
  // requires the cost model tuning as with the current cost model we have
  // regressions.
  if (IsTripCountEstimated && !VecScenario.hasRemainder() &&
      !VecScenario.isMainMasked())
    VecScenario.addScalarRemainder();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (!VecScenarioStr.empty()) {
    VecScenario.fromString(VecScenarioStr);
    SmallSet<unsigned, 4> UsedVFs;
    VecScenario.getUsedVFs(UsedVFs);
    for (auto VF : UsedVFs) {
      VPlan *P = getVPlanForVF(VF);
      assert(P && "The VPlan for VF from string does not exist");
    }

    unsigned VF = VecScenario.getMainVF();
    VPlanVector *MPlan = getVPlanForVF(VF);
    if (VecScenario.hasPeel()) {
      VPLoop *L = *(MPlan->getVPLoopInfo())->begin();
      if (VecScenario.hasMaskedPeel() && !L->hasNormalizedInduction()) {
        // replace by scalar loop if there is no normalized induction
        VecScenario.setScalarPeel();
      }

      // If peel was forced, ensure we have a valid peel variant selected
      const auto *Peel = MPlan->getPreferredPeeling(VF);
      if (!Peel || dyn_cast<VPlanNoPeeling>(Peel) ||
          dyn_cast<VPlanNoPeelingAligned>(Peel) ||
          dyn_cast<VPlanNoPeelingUnaligned>(Peel))
        MPlan->setPreferredPeeling(VF, std::make_unique<VPlanStaticPeeling>(1));
    } else {
      // Peel was disabled: ensure peeling is unselected for all VFs.
      for (auto VF : UsedVFs) {
        MPlan->setPreferredPeeling(
            VF, std::make_unique<VPlanNoPeeling>(VPlanNoPeeling::LoopObject));
      }
    }
  }
  if (VecScenario.hasPeel() && SkipDynamicPeelTC != 0) {
    VecScenario.setMinimumProfitablePeelTC(SkipDynamicPeelTC);
  }
  if (PrintVecScenario || !VecScenarioStr.empty()) {
    VecScenario.dump();
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Delete all other VPlans.
  SmallSet<unsigned, 4> UsedVFs;
  VecScenario.getUsedVFs(UsedVFs);
  for (auto &It : VPlans) {
    if (!UsedVFs.count(It.first))
      VPlans.erase(It.first);
  }
  LLVM_DEBUG(dbgs() << "Selecting VPlan with VF=" << getBestVF() << '\n');

  if (getBestVF() == 1) {
    if (TripCount < ForcedVF) {
      bailoutWithDebug(
          OptReportVerbosity::Medium, OptRemarkID::VecFailLowTripCount,
          INTERNAL("The loop trip count is less than enforced vector factor."),
          WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                        : AuxRemarkID::Loop);
    } else {
      bailoutWithDebug(OptReportVerbosity::Medium,
                       OptRemarkID::VecFailNotProfitable,
                       INTERNAL("Loop is unprofitable to vectorize."),
                       WRLp && WRLp->isOmpSIMDLoop() ? AuxRemarkID::SimdLoop
                                                     : AuxRemarkID::Loop);
    }
  }
  if (BestCostSummarySet) {
    VPlanVector *BestVPlan = getBestVPlan();
    OptReportStatsTracker &OptRptStats =
        BestVPlan->getOptRptStatsForLoop(BestVPlan->getMainLoop(true));
    if (BestCostSummary.ScalarIterationCost.isValid()) {
      OptRptStats.CostModelRemarks.emplace_back(
          *Context, OptRemarkID::VectorizerScalarLoopCost,
          std::to_string(BestCostSummary.ScalarIterationCost.getFloatValue()));
    }
    if (BestCostSummary.VectorIterationCost.isValid()) {
      // Add remark: vector cost
      OptRptStats.CostModelRemarks.emplace_back(
          *Context, OptRemarkID::VectorizerVectorLoopCost,
          std::to_string(BestCostSummary.VectorIterationCost.getFloatValue()));
    }
    if (BestCostSummary.Speedup.isValid()) {
      // Add remark: estimated potential speedup
      OptRptStats.CostModelRemarks.emplace_back(
          *Context, OptRemarkID::VectorizerEstimatedSpeedup,
          std::to_string(BestCostSummary.Speedup.getFloatValue()));
    }
    if (BestCostSummary.LoopOverhead.isValid()) {
      // Add remark: vectorization support: normalized vectorization overhead
      OptRptStats.CostModelRemarks.emplace_back(
          *Context, OptRemarkID::NormalizedVecOverhead,
          std::to_string(BestCostSummary.LoopOverhead.getFloatValue()));
    }
    if (!IsTripCountEstimated) {
      // Add remark: using (estimated) scalar loop trip count
      OptRptStats.CostModelRemarks.emplace_back(
          *Context, OptRemarkID::VectorizerScalarTripCount,
          std::to_string(OrigTripCount));
    }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (EmitDebugOptRemarks) {
      // Add detailed peel decision info to debug opt report.
      const auto &PeelSummary = BestCostSummary.PeelSummary;
      DriverImpl::addDebugRemark(OptRptStats.CostModelRemarks, *Context,
                                 "peel scenario: ", PeelSummary.Scenario);
      DriverImpl::addDebugRemark(
          OptRptStats.CostModelRemarks, *Context, "peel was performed: ",
          PeelSummary.PeelingWasPerformed ? "yes" : "no");
      DriverImpl::addDebugRemark(
          OptRptStats.CostModelRemarks, *Context,
          "estimated gain from peeling: ", PeelSummary.GainWithPeel);
      DriverImpl::addDebugRemark(
          OptRptStats.CostModelRemarks, *Context,
          "estimated gain from *not* peeling: ", PeelSummary.GainWithoutPeel);
      DriverImpl::addDebugRemark(OptRptStats.CostModelRemarks, *Context,
                                 "peel kind: ", PeelSummary.PeelKind);
      if (!PeelSummary.Formula.empty())
        DriverImpl::addDebugRemark(OptRptStats.CostModelRemarks, *Context,
                                   "peel formula: ", PeelSummary.Formula);
    }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }

  return std::make_pair(getBestVF(), getBestVPlan());
}

void LoopVectorizationPlanner::updateVecScenario(
    VPlanPeelEvaluator const &PE, VPlanRemainderEvaluator const &RE,
    unsigned VF, unsigned UF, bool MainIsMasked) {
  using PeelKind = VPlanPeelEvaluator::PeelLoopKind;
  using RemKind = VPlanRemainderEvaluator::RemainderLoopKind;
  switch (PE.getPeelLoopKind()) {
  case PeelKind::None:
    VecScenario.resetPeel();
    break;
  case PeelKind::Scalar:
    VecScenario.setScalarPeel();
    break;
  case PeelKind::MaskedVector:
    // Note: for masked peel we use the same VF as for main loop.
    VecScenario.setMaskedPeel(VF);
    break;
  }
  VecScenario.resetRemainders();
  switch (RE.getRemainderLoopKind()) {
  case RemKind::None:
    break;
  case RemKind::Scalar:
    VecScenario.addScalarRemainder();
    break;
  case RemKind::VectorScalar:
    VecScenario.addUnmaskedRemainder(RE.getRemainderVF());
    VecScenario.addScalarRemainder();
    break;
  case RemKind::MaskedVector:
    // Note: for masked remainder we use the same VF as for main loop.
    VecScenario.addMaskedRemainder(VF);
    if (EnableMaskedRemTCRuntimeCheck && !isVecRemainderEnforced() &&
        RE.getMinProfitableMaskedRemTC() >= MaskedRemTCThreshold) {
      VecScenario.addScalarRemainder();
      VecScenario.setMinProfitableMaskedRemTC(RE.getMinProfitableMaskedRemTC());
    }
    break;
  }
  if (MainIsMasked) {
    assert(UF == 1 && "Unexpected unroll for masked mode");
    VecScenario.setMaskedMain(VF);
  } else {
    VecScenario.setVectorMain(VF, UF);
  }
}

void LoopVectorizationPlanner::predicate() {
  if (DisableVPlanPredicator)
    return;

  auto Predicate = [](VPlanVector &VPlan) {
    VPLoop *OuterLoop = VPlan.getMainLoop(true);
    // Search loops require multiple hacks. Skipping LCSSA/LoopCFU is one of
    // them.
    bool SearchLoopHack = !OuterLoop->getExitBlock();

    if (!SearchLoopHack)
      formLCSSA(VPlan, true /* SkipTopLoop */);
    VPLAN_DUMP(LCSSADumpControl, VPlan);

    if (!SearchLoopHack) {
      assert(!VPlan.getVPlanDA()->isDivergent(
                 *(OuterLoop)->getLoopLatch()->getCondBit()) &&
             "Outer loop doesn't have uniform backedge!");
      VPlanLoopCFU LoopCFU(VPlan);
      LoopCFU.run();
    }
    VPLAN_DUMP(LoopCFUDumpControl, VPlan);

    // Predication "has" to be done even for the search loop hack. Our
    // idiom-matching code and CG currently expect that. Note that predicator
    // has some hacks for search loop processing inside it as well.
    VPlanPredicator VPP(VPlan);
    VPP.predicate();
    VPLAN_DUMP(PredicatorDumpControl, VPlan);
  };

  transformAllVPlans(Predicate);
}

void LoopVectorizationPlanner::insertAllZeroBypasses(VPlanVector *Plan,
                                                     unsigned VF) {
  // Skip multi-exit loops at outer VPlan level. Inner loops will be
  // canonicalized to single exit in VPlan. TODO: this check is only
  // needed due to hacky search loop support. Change to assert in the
  // future.
  VPLoop *VPLp = *(Plan->getVPLoopInfo()->begin());
  if (!VPLp->getExitBlock())
    return;

  // Holds the pair of blocks representing the begin/end of an all-zero
  // bypass region. The block-predicate at the begin block is used to
  // generate the bypass. This is the final record of all-zero bypass
  // regions that will be inserted.
  VPlanAllZeroBypass::AllZeroBypassRegionsTy AllZeroBypassRegions;
  // Keep a map of the regions collected under a specific block-predicate
  // to avoid collecting/inserting unnecessary regions. This data structure
  // records the same regions as AllZeroBypassRegions, but is keyed by
  // block-predicate for quick look-up.
  VPlanAllZeroBypass::RegionsCollectedTy RegionsCollected;
  VPlanAllZeroBypass AZB(*Plan);
  if (EnableAllZeroBypassLoops)
    AZB.collectAllZeroBypassLoopRegions(AllZeroBypassRegions, RegionsCollected);
  if (EnableAllZeroBypassNonLoops &&
      TTI->isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelSSE42)) {

    AZB.collectAllZeroBypassNonLoopRegions(AllZeroBypassRegions,
                                           RegionsCollected,
                                           createCostModel(Plan, VF).get(), VF);
  }
  AZB.insertAllZeroBypasses(AllZeroBypassRegions);
  VPLAN_DUMP(AllZeroBypassDumpControl, Plan);
}

bool LoopVectorizationPlanner::unroll(VPlanVector &Plan) {
  unsigned UF = getBestUF();
  if (UF > 1) {
    VPlanLoopUnroller Unroller(Plan, UF);
    Unroller.run();
    if (TheLoop)
      TheLoop->setLoopAlreadyUnrolled();
    return true;
  }
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void SingleLoopVecScenario::fromString(StringRef S) {
  // The supported string format is:
  //  Spec:= <Peel>;<Main>;<Remainder>
  //  Peel:= <Loop>
  //  Main:= <Loop>
  //  Remainder:= <Loops>
  //  LoopKinds:= <Loop> {<Loops>}
  //  Loop := <LoopKind><VF>
  //  VF := a positive number
  //  LoopKind := n | s | v | m
  //    n := none, means no loop
  //    s := scalar
  //    v := non-masked vector
  //    m := masked vector
  // E.g."n0;v8;v4s1" means "no peel, unmasked vector main loop with VF=8,
  // unmasked vector remainder with VF=4 and scalar remainder".

  SmallVector<StringRef, 3> Parts;
  S.split(Parts, ';', 3, false);
  assert(Parts.size() == 3 && "expected three substrings");

  // Get one descriptor from a string R, setting E to the beginning of the next
  // substring. The routine performs parsing of the <Loop> term from the spec
  // above (<LoopKind><VF>).
  auto getDescr = [](StringRef &R, StringRef &E) -> AuxLoopDescr {
    AuxLoopDescr Res;
    assert(R.size() >= 2 && "unexpectedly short string");
    switch (R[0]) {
    case 'n':
      Res.Kind = LKNone;
      break;
    case 's':
      Res.Kind = LKScalar;
      break;
    case 'v':
      Res.Kind = LKVector;
      break;
    case 'm':
      Res.Kind = LKMasked;
      break;
    default:
      llvm_unreachable("unexpected character");
      break;
    }
    size_t C = 1;
    int VF = 0;
    assert(std::isdigit(R[1]) && "expected a number");
    while (C < R.size() && std::isdigit(R[C])) {
      VF *= 10;
      VF += R[C] - '0';
      C++;
    }
    Res.VF = VF;
    // C is incremented on the last iteration, so it points to beginning of the
    // next substring
    E = R.drop_front(C);
    return Res;
  };

  StringRef E;
  Peel = getDescr(Parts[0], E);
  Main = getDescr(Parts[1], E);

  resetRemainders();
  StringRef R = Parts[2];
  while (R.size()) {
    AuxLoopDescr D = getDescr(R, E);
    addRemainder(D);
    R = E;
  }
  std::sort(Remainders.begin(), Remainders.end(),
            [](const AuxLoopDescr &F, const AuxLoopDescr &S) {
              return F.Kind < S.Kind;
            });
}

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

void LoopVectorizationPlanner::setVPlanFlagsFromFunction(VPlan *Plan,
                                                         const Function *F) {
  Plan->setPrintingEnabled(llvm::isFunctionInPrintList(F->getName()));
  Plan->setIsVecFuncVariant(VFInfo::isVectorVariant(F->getName()));
}

std::shared_ptr<VPlanVector> LoopVectorizationPlanner::buildInitialVPlan(
    VPExternalValues &Ext, VPUnlinkedInstructions &UnlinkedVPInsts,
    std::string VPlanName, AssumptionCache &AC, ScalarEvolution *SE) {
  // Create new empty VPlan. At this stage we want to create only NonMasked
  // VPlans.
  std::shared_ptr<VPlanVector> SharedPlan =
      std::make_shared<VPlanNonMasked>(Ext, UnlinkedVPInsts);
  VPlanVector *Plan = SharedPlan.get();
  Plan->setName(VPlanName);

  const Function* F = TheLoop->getHeader()->getParent();
  setVPlanFlagsFromFunction(Plan, F);

  if (EnableSOAAnalysis)
    // Enable SOA-analysis.
    Plan->enableSOAAnalysis();

  // Set early-exit loop property.
  if (VPlanEnableEarlyExitLoops && TheLoop->getExitingBlock() != nullptr)
    Plan->setIsEarlyExitLoop(true);

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(TheLoop, LI, *DL, WRLp, Plan, Legal, AC, *DT, SE,
                               BFI);
  if (!HCFGBuilder.buildHierarchicalCFG())
    return nullptr;

  return SharedPlan;
}

void LoopVectorizationPlanner::runInitialVecSpecificTransforms(
    VPlanVector *Plan) {
  // 1. Convert VPLoopEntities into explicit VPInstructions.
  emitVPEntityInstrs(Plan);
  // 2. Wrap flags are in general invalid after vectorization, clear them.
  clearWrapFlagsForReductions(Plan);
  // 3. Emit explicit uniform vector loop IV.
  emitVecSpecifics(Plan);
}

void LoopVectorizationPlanner::emitVPEntityInstrs(VPlanVector *Plan) {
  VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  VPBuilder VPIRBuilder;
  LE->insertVPInstructions(VPIRBuilder);

  if (LE->hasReduction())
    reportReductions(Plan, MainLoop, LE);

  VPLAN_DUMP(VPEntityInstructionsDumpControl, Plan);
}

// Simplified type-printing utility.  We use this instead of Type::print()
// because the latter is disabled in Intel release compilers.
static void printReductionType(raw_ostream &OS, Type *Ty) {

  switch (Ty->getTypeID()) {

  case Type::FloatTyID:
    OS << "float";
    return;

  case Type::DoubleTyID:
    OS << "double";
    return;

  case Type::IntegerTyID:
    OS << "int" << cast<IntegerType>(Ty)->getBitWidth() << "_t";
    return;

  case Type::StructTyID: {
    StructType *STy = cast<StructType>(Ty);
    OS << "structure of " << STy->getNumElements() << " elements";
    return;
  }

  case Type::ArrayTyID: {
    ArrayType *ATy = cast<ArrayType>(Ty);
    OS << "array of ";
    printReductionType(OS, ATy->getElementType());
    OS << " (" << ATy->getNumElements() << " elements)";
    return;
  }

  default:
    OS << "unknown";
    return;
  }
}

void LoopVectorizationPlanner::reportReductions(VPlanVector *Plan,
                                                VPLoop *MainLoop,
                                                VPLoopEntityList *LE) {

  OptReportStatsTracker &OptRptStats = Plan->getOptRptStatsForLoop(MainLoop);

  if (WRLp && WRLp->isOmpSIMDLoop())
    // Add remark: Loop has SIMD reduction
    OptRptStats.ReductionInstRemarks.emplace_back(
        *Context, OptRemarkID::LoopHasSimdReduction);
  else
    // Add remark: Loop has reduction
    OptRptStats.ReductionInstRemarks.emplace_back(
        *Context, OptRemarkID::LoopHasReduction);

  // Generate specific remarks for each reduction in the loop.
  for (auto Red : LE->vpreductions()) {
    std::string S1, S2;
    raw_string_ostream SS1(S1), SS2(S2);

    if (isa<VPInscanReduction>(Red) || isa<VPUserDefinedScanReduction>(Red))
      SS1 << "inscan ";

    switch (Red->getRecurrenceKind()) {
    case RecurKind::Add:
    case RecurKind::FAdd:
      SS1 << "add";
      break;
    case RecurKind::Mul:
    case RecurKind::FMul:
      SS1 << "multiply";
      break;
    case RecurKind::Or:
      SS1 << "OR";
      break;
    case RecurKind::And:
      SS1 << "AND";
      break;
    case RecurKind::Xor:
      SS1 << "XOR";
      break;
    case RecurKind::SMin:
      SS1 << "signed minimum";
      break;
    case RecurKind::SMax:
      SS1 << "signed maximum";
      break;
    case RecurKind::UMin:
      SS1 << "unsigned minimum";
      break;
    case RecurKind::UMax:
      SS1 << "unsigned maximum";
      break;
    case RecurKind::FMin:
    case RecurKind::FMinimum:
      SS1 << "minimum";
      break;
    case RecurKind::FMax:
    case RecurKind::FMaximum:
      SS1 << "maximum";
      break;
    case RecurKind::FMulAdd:
      SS1 << "multiply-add";
      break;
    case RecurKind::Udr:
      SS1 << "user-defined";
      break;
    case RecurKind::IAnyOf:
    case RecurKind::FAnyOf:
      SS1 << "select-compare";
      break;
    default:
      break;
    }

    Type *Ty = Red->getRecurrenceType();

    if (Ty->isArrayTy())
      SS1 << " array or array-section";

    printReductionType(SS2, Ty);

    auto Exit = Red->getLoopExitInstr();
    if (Exit && Exit->getDebugLocation()) {
      SS2 << " [";
      Exit->getDebugLocation().print(SS2);
      SS2 << "]";
    }

    OptRptStats.ReductionInstRemarks.emplace_back(
        *Context, OptRemarkID::VectorizerReductionInfo, S1, S2);
  }
}

void LoopVectorizationPlanner::clearWrapFlagsForReductions(VPlanVector *Plan) {
  auto *VPLI = Plan->getVPLoopInfo();
  for (auto *OuterLp : *VPLI) {
    auto *OuterLpPreheader = cast<VPBasicBlock>(OuterLp->getLoopPreheader());
    for (VPInstruction &Inst : *OuterLpPreheader) {
      auto *RedInit = dyn_cast<VPReductionInit>(&Inst);
      if (!RedInit)
        continue;

      SmallVector<VPInstruction *, 8> Worklist;
      SmallPtrSet<VPInstruction *, 8> Visited;
      Worklist.push_back(RedInit);
      Visited.insert(RedInit);

      while (!Worklist.empty()) {
        VPInstruction *Cur = Worklist.pop_back_val();
        // Drop NSW/NUW flags for this instruction that participates in
        // reduction.
        if (Cur->hasNoSignedWrap() || Cur->hasNoUnsignedWrap()) {
          Cur->setHasNoSignedWrap(false);
          Cur->setHasNoUnsignedWrap(false);
        }

        for (auto *U : Cur->users()) {
          if (!isa<VPInstruction>(U))
            continue;

          auto *UInst = cast<VPInstruction>(U);
          if (OuterLp->contains(UInst->getParent()) &&
              Visited.insert(UInst).second)
            Worklist.push_back(UInst);
        }
      }
    }
  }

  VPLAN_DUMP(ClearReductionWrapFlagsDumpControl, Plan);
}

bool LoopVectorizationPlanner::fencesFound(VPlan *Plan) {
  for (auto &Inst : vpinstructions(*Plan))
    if (Inst.getOpcode() == Instruction::Fence)
      return true;

  return false;
}

void LoopVectorizationPlanner::exchangeExclusiveScanLoopInputScanPhases(
    VPlanVector *Plan) {
  if (!WRLp)
    return;

  // Nothing to do if this is not an exclusive scan loop. According to the spec,
  // scan reductions must be either all inclusive or all exclusive.
  // If we don't have exclusive reductions, this loop does not require
  // the transformation.
  bool IsExclusiveLoop = false;
  bool IsArrayTyReduction = false;
  for (ReductionItem *Item : WRLp->getRed().items()) {
    if (Item->getIsInscan() &&
        isa<ExclusiveItem>(
            WRegionUtils::getInclusiveExclusiveItemForReductionItem(WRLp,
                                                                    Item))) {
      IsExclusiveLoop = true;
      // Check if this exclusive scan reduction is array type.
      Value *NumElements = nullptr;
      std::tie(std::ignore /*Type*/, NumElements, std::ignore /*AddrSpace*/) =
          VPOParoptUtils::getItemInfo(Item);
      if (auto *CI = dyn_cast_or_null<ConstantInt>(NumElements))
        if (CI->getValue().ugt(1))
          IsArrayTyReduction = true;
      break;
    }
  }
  if (!IsExclusiveLoop)
    return;

  // Entities framework removed the scan region directives, however their
  // parent basic blocks remained. As the scan region is single entry/single
  // exit, the structure would look like this:
  //
  // ScanPhaseBB:
  //  ... ; scan phase
  //
  // ScanBeginBB: #preds : ScanPhaseBB
  //  br ScanBB
  //
  // ScanBB: # preds: ScanBeginBB
  //  ...
  //  float %vp3 = running-exclusive-reduction float %vp1 float %vp2 ...
  //  ...
  //  br ScanEndBB
  //
  // NOTE: For array type reduction the above BB would be single block loop over
  // array elements like below -
  //
  // ArrScanBB: # preds ScanBeginBB, ArrScanBB
  //  %idx = phi [ 0, %ScanBeginBB ], [ %idx.next, %ArrScanBB]
  //  ...
  //  float %vp3 = running-exclusive-reduction float %vp1 float %vp2 ...
  //  ...
  //  %idx.next = add %idx, 1
  //  %exit.cond = icmp eq %idx.next, ArrNumElems
  //  br i1 %exit.cond, ScanEndBB, ArrScanBB
  //
  // ScanEndBB: # preds: ScanBB
  //  br InputPhaseBB
  //
  // InputPhaseBB: # preds: ScanEndBB
  //  ... ; input phase

  // After the transformation the loop would like as follows:
  // InputPhaseBB:
  //  ... ; input phase
  //  br ScanBeginBB
  //
  // ScanBeginBB: #preds : InputPhaseBB
  //  br ScanBB
  //
  // ScanBB (or ArrScanBB): # preds: ScanBeginBB
  //  ...
  //  float %vp3 = running-exclusive-reduction float %vp1 float %vp2 ...
  //  br ScanEndBB
  //
  // ScanEndBB: # preds: ScanBB
  //  br ScanPhaseBB
  //
  // ScanPhaseBB: # preds: ScanEndBB
  //  ... ; scan phase

  // Locate the block containing running exclusive reduction instructions.
  // It is enough to find one such instruction as they all have been insterted
  // into the same block.
  VPInstruction *RunningRedInst = nullptr;
  for (auto &I : vpinstructions(Plan)) {
    if (isa<VPRunningExclusiveReduction>(&I) ||
        isa<VPRunningExclusiveUDS>(&I)) {
      RunningRedInst = &I;
      break;
    }
  }
  assert(RunningRedInst &&
         "Running reduction instructions must have been inserted!");

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *VPL = *VPLInfo->begin();

  // Split the latch before all of the induction increments.
  // Find the first increment for inductions in the Latch, traversing it in
  // reverse.
  VPBasicBlock *Latch = VPL->getLoopLatch();
  assert(Latch && "Latch is expected to exist.");
  VPBranchInst *Br = Latch->getTerminator();
  assert(Br && "Expect branch condition!");
  VPCmpInst *Cond = dyn_cast<VPCmpInst>(Br->getCondition());
  assert(Cond && (Cond->getNumUsers() == 1) &&
         "Loop latch condition is expected to have one user!");

  auto IsIndIncrement = [VPL](VPInstruction *PossibleIndInc) {
    if (!PossibleIndInc)
      return false;
    if (PossibleIndInc->getOpcode() == Instruction::Add &&
        (isa<VPInductionInitStep>(PossibleIndInc->getOperand(1)) ||
         isa<VPInductionInitStep>(PossibleIndInc->getOperand(0))) &&
        llvm::find_if(PossibleIndInc->users(), [VPL](auto &User) {
          return (isa<VPPHINode>(User) &&
                  cast<VPPHINode>(User)->getParent() == VPL->getHeader());
        }) != PossibleIndInc->users().end())
      return true;
    return false;
  };

  auto GetIndInc = [&IsIndIncrement](VPInstruction *VPInst) -> VPInstruction * {
    for (unsigned Idx = 0; Idx < VPInst->getNumOperands(); Idx++) {
      VPInstruction *Op = cast<VPInstruction>(VPInst->getOperand(Idx));
      if (IsIndIncrement(Op))
        return Op;
    }
    return nullptr;
  };
  VPInstruction *IVIndInc = GetIndInc(Cond);
  assert(IVIndInc && "Expect increment for loop IV!");
  // In case the increment is not in the Latch, move it there.
  if (IVIndInc->getParent() != Latch)
    IVIndInc->moveBefore(Cond);

  // Go starting from Cond in reverse looking for the first induction increment.
  // We do expect at least one induction increment for the main loop IV.
  VPBasicBlock::iterator FirstIndInc = --Cond->getIterator();
  for (auto It = FirstIndInc; It != Latch->begin(); It--) {
    VPInstruction *PossibleIndInc = &*It;
    if (IsIndIncrement(PossibleIndInc))
      FirstIndInc = It;
    else
      break;
  }

  VPBasicBlock *NewLatch =
      VPBlockUtils::splitBlock(Latch, FirstIndInc, VPLInfo);
  NewLatch->setName("new_latch");

  // Split the header after the last store to the Linear.IV. Otherwise,
  // this initializing store would move past the separating pragma. This would
  // lead to uninitialized memory loads for Linear.IV in the part of loop
  // preceeding the separating pragma.
  // Loop closed calculation and store for Linear.IV are first instruction in
  // the original header. Entity framework have inserted its instructions just
  // after the phis, so the split will be correct.

  // Obtain pointer for in-memory LINEAR.IV, traverse through inductions.
  VPValue *LinearIVMemPtr = nullptr;
  VPBasicBlock *Preheader = VPL->getLoopPreheader();
  for (const auto &I : *Preheader) {
    auto *VPIndInit = dyn_cast<VPInductionInit>(&I);
    if (!VPIndInit)
      continue;
    if (VPIndInit->getIsLinearIV()) {
      for (const auto *U : VPIndInit->users()) {
        // The only Store of InductionInit is the store to Linear.IV alloca.
        auto *LSI = dyn_cast<VPLoadStoreInst>(U);
        if (!LSI)
          continue;
        if (LSI->getOpcode() != Instruction::Store)
          continue;
        LinearIVMemPtr = LSI->getPointerOperand();
      }
    }
  }
  assert(LinearIVMemPtr && "LinearIV memory has to be found!");
  auto *Header = VPL->getHeader();
  // Inscan reduction requires memory guard regions to be present,
  // thus the regions were inserted in the Header right after phis.
  // After VPOCFGRestructuringPass pass they were outlined into separate BBs
  // and removed during VPlan construction, but BBs remained.
  // That means that we should look for Linear.IV store in the second next BB
  // after the Header.
  // HeaderBB:
  //   %vp1 = phi [ ], ... // header phis.
  //   br BB1
  // BB1:
  //   br BB2
  // BB2:
  //   ... ; actual header code.
  VPInstruction *LastLinearIVStore = nullptr;
  auto *OriginalHeader = Header->getSingleSuccessor()->getSingleSuccessor();

  // Skip empty blocks that we might get when doing O0 vectorization.
  // Running any of CFGSimplify passes which gets rid of extra empty blocks
  // before VPlanDriver is not possible as they are permanenty turned off inside
  // of a pass implementation with bool isRequired {return false;} and changing
  // it is not the right approach to do. This machanism came with new pass
  // manager. Please take a look into PassInstrumentation.h runBeforePass(..)
  // and StandardInstrumentations.cpp OptNoneInstrumentation::shouldRun())
  while (OriginalHeader->size() == 1) {
    OriginalHeader = OriginalHeader->getSingleSuccessor();
    assert(OriginalHeader && OriginalHeader->getSinglePredecessor() &&
           "Expected non-null block with single predecessor");
  }

  for (auto &I : reverse(*OriginalHeader)) {
    auto *LSI = dyn_cast<VPLoadStoreInst>(&I);
    if (!LSI)
      continue;
    if (LSI->getOpcode() != Instruction::Store)
      continue;
    const VPValue *PtrOp = LSI->getPointerOperand();
    if (PtrOp == LinearIVMemPtr) {
      LastLinearIVStore = &I;
      break;
    }
  }
  assert(LastLinearIVStore && "LinearIV must have a store in OriginalHeader!");

  // Incrementing LastLinearIVStore is safe as in the worst case scenario
  // it will be a terminator inst.
  VPBasicBlock *HeaderSucc = VPBlockUtils::splitBlock(
      OriginalHeader, ++LastLinearIVStore->getIterator(), VPLInfo);

  VPBasicBlock *ScanBB = RunningRedInst->getParent();
  VPBasicBlock *ScanBeginBB = nullptr, *ScanEndBB = nullptr;
  if (!IsArrayTyReduction) {
    ScanBeginBB = ScanBB->getSinglePredecessor();
    ScanEndBB = ScanBB->getSingleSuccessor();
  } else {
    // For array scan reductions, search through all successors and predecessors
    // to find ScanBeginBB and ScanEndBB.
    assert((ScanBB->getNumSuccessors() == 2 &&
            ScanBB->getNumPredecessors() == 2) &&
           "Expected single BB loop for array scan reduction.");
    ScanBeginBB = ScanBB->getPredecessor(0) == ScanBB
                      ? ScanBB->getPredecessor(1)
                      : ScanBB->getPredecessor(0);
    ScanEndBB = ScanBB->getSuccessor(0) == ScanBB ? ScanBB->getSuccessor(1)
                                                  : ScanBB->getSuccessor(0);
  }
  assert(ScanBeginBB && ScanEndBB && "Separating scan pragma was not found!");
  assert(ScanBeginBB->getSingleSuccessor() &&
         ScanEndBB->getSinglePredecessor() &&
         "Separating pragma directives must have one successor/predecessor!");
  assert(ScanBeginBB->getSingleSuccessor() ==
             ScanEndBB->getSinglePredecessor() &&
         "Separating pragma directives must be one basic block away!");

  assert(ScanEndBB->getSingleSuccessor() && "Expect only one successor!");
  assert(ScanBeginBB->getSinglePredecessor() && "Expect only one predecessor!");
  auto *BeginPred = ScanBeginBB->getSinglePredecessor();
  auto *EndSucc = ScanEndBB->getSingleSuccessor();

  OriginalHeader->replaceSuccessor(HeaderSucc, EndSucc);
  Latch->replaceSuccessor(NewLatch, ScanBeginBB);
  ScanEndBB->replaceSuccessor(EndSucc, HeaderSucc);
  BeginPred->replaceSuccessor(ScanBeginBB, NewLatch);

  // Lifetime start/end intrinsics might become reversed.
  // Move them to the header and the new latch. In theory, their scope might be
  // widened in doing so. However, this is more of a conservative approach as
  // it seems to be unlikely to have deeply nested Lifetime start/end intrinsics
  // in scan loops. We expect at least one pair for LINEAR.IV.
  SmallVector<VPCallInstruction *, 2> LifetimeStartVec;
  SmallVector<VPCallInstruction *, 2> LifetimeEndVec;
  for (auto &I : vpinstructions(Plan)) {
    VPInstruction *Inst = &I;
    if (!VPL->contains(Inst))
      continue;
    if (auto *VPCall = dyn_cast<VPCallInstruction>(Inst)) {
      if (VPCall->isIntrinsicFromList({Intrinsic::lifetime_start}))
        LifetimeStartVec.push_back(VPCall);
      if (VPCall->isIntrinsicFromList({Intrinsic::lifetime_end}))
        LifetimeEndVec.push_back(VPCall);
    }
  }
  for (auto *LifetimeStart : LifetimeStartVec)
    LifetimeStart->moveBefore(Header->getFirstNonPhi());
  for (auto *LifetimeEnd : LifetimeEndVec)
    LifetimeEnd->moveBefore(NewLatch->getFirstNonPhi());

  // TODO: CMPLRLLVM-9535 Update VPDomTree and VPPostDomTree instead of
  // recalculating it.
  Plan->computeDT();
  Plan->computePDT();
}

void LoopVectorizationPlanner::emitVecSpecifics(VPlanVector *Plan) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  assert(PreHeader && "Single pre-header is expected!");

  VPBuilder Builder;
  Builder.setInsertPoint(PreHeader);
  VPValue *OrigTC = nullptr;
  VPValue *VF = nullptr;
  Type *VectorLoopIVType = nullptr;
  VPInstruction *IVUpdate = nullptr;
  bool ExactUB = true;
  bool HasNormalizedInd = hasLoopNormalizedInduction(CandidateLoop, ExactUB);
  CandidateLoop->setHasNormalizedInductionFlag(HasNormalizedInd, ExactUB);
  if (!HasNormalizedInd) {
    // If loop does not have normalized induction then emit it.
    VectorLoopIVType = Legal->getWidestInductionType();
    if (!VectorLoopIVType) {
      // Ugly workaround for tests forcing VPlan build when we can't actually do
      // that. Shouldn't happen outside stress/forced pipeline.
      VectorLoopIVType = Type::getInt64Ty(*Plan->getLLVMContext());
    }
    auto *VPOne = Plan->getVPConstant(ConstantInt::get(VectorLoopIVType, 1));

    VF = Builder.create<VPInductionInitStep>("VF", VPOne, Instruction::Add);
  } else {
    VPCmpInst *Cond;
    std::tie(OrigTC, Cond) = CandidateLoop->getLoopUpperBound();
    IVUpdate = cast<VPInstruction>(Cond->getOperand(0) == OrigTC
                                       ? Cond->getOperand(1)
                                       : Cond->getOperand(0));
    VectorLoopIVType = IVUpdate->getType();
  }

  if (!OrigTC)
    OrigTC = Builder.create<VPOrigTripCountCalculation>(
        "orig.trip.count", TheLoop, CandidateLoop, VectorLoopIVType);
  auto VecTC =
      Builder.create<VPVectorTripCountCalculation>("vector.trip.count", OrigTC);
  emitVectorLoopIV(Plan, VecTC, VF, IVUpdate, ExactUB);
}

void LoopVectorizationPlanner::emitVectorLoopIV(VPlanVector *Plan,
                                                VPValue *TripCount, VPValue *VF,
                                                VPValue *IVUpdate, bool ExactUB) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  auto *Header = CandidateLoop->getHeader();
  auto *Latch = CandidateLoop->getLoopLatch();
  assert(PreHeader && "Single pre-header is expected!");
  assert(Latch && "Single loop latch is expected!");

  VPBuilder Builder;
  if (!IVUpdate) {
    Type *VectorLoopIVType = TripCount->getType();
    auto *VPZero =
        Plan->getVPConstant(ConstantInt::getNullValue(VectorLoopIVType));

    Builder.setInsertPoint(Header, Header->begin());
    auto *IV = Builder.createPhiInstruction(VectorLoopIVType, "vector.loop.iv");
    IV->addIncoming(VPZero, PreHeader);
    Builder.setInsertPoint(Latch);
    IVUpdate = Builder.createAdd(IV, VF, "vector.loop.iv.next");
    IV->addIncoming(IVUpdate, Latch);
  }

  // Set nowrap flags for the vector loop iv update instruction. This instruction
  // is known to be within the signed/unsigned range.
  cast<VPInstruction>(IVUpdate)->setHasNoUnsignedWrap(true);
  cast<VPInstruction>(IVUpdate)->setHasNoSignedWrap(true);

  Builder.setInsertPoint(Latch);
  auto *ExitCond = Builder.createCmpInst(
      Latch->getSuccessor(0) == Header
          ? (ExactUB ? CmpInst::ICMP_ULT : CmpInst::ICMP_ULE)
          : (ExactUB ? CmpInst::ICMP_UGE : CmpInst::ICMP_UGT),
      IVUpdate, TripCount, "vector.loop.exitcond");

  VPValue *OrigExitCond = Latch->getCondBit();
  if (Latch->getNumSuccessors() > 1)
    Latch->setCondBit(ExitCond);

  // If original exit condition had single use, remove it - we calculate exit
  // condition differently now.
  // FIXME: "_or_null" here is due to broken stess pipeline that must really
  // stop right after CFG is imported, before *any* transformation is tried on
  // it.
  if (auto *Inst = dyn_cast_or_null<VPInstruction>(OrigExitCond)) {
    ExitCond->setDebugLocation(Inst->getDebugLocation());
    if (Inst->getNumUsers() == 0)
      Latch->eraseInstruction(Inst);
  }
}

void LoopVectorizationPlanner::doLoopMassaging(VPlanVector *Plan) {
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  assert((VPLInfo->size() == 1) && "Expected only 1 top-level loop");
  VPLoop *TopLoop = *VPLInfo->begin();
  auto &VPDomTree = *Plan->getDT();
  (void)VPDomTree;

  LLVM_DEBUG(dbgs() << "Dominator Tree Before mergeLoopExits\n";
             VPDomTree.print(dbgs()));

  if (LoopMassagingEnabled) {
    // TODO: Bail-out loop massaging for uniform inner loops.
    for (auto *VPL : post_order(TopLoop)) {
      if (VPL == TopLoop) {
        // Loop massaging is needed for outer multi-exit loops, if explicit
        // early-exit loop vectorization is enabled.
        if (!(VPlanEnableEarlyExitLoops && VPL->getExitingBlock() == nullptr))
          continue;
      }
      singleExitWhileLoopCanonicalization(VPL);
      mergeLoopExits(VPL, true /*NeedsOuterLpEarlyExitHandling*/);
      // TODO: Verify loops here? It is done again after all initial transforms.
    }
    VPLAN_DUMP(LoopMassagingDumpControl, Plan);
    LLVM_DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
               VPDomTree.print(dbgs()));
  }
}

void LoopVectorizationPlanner::printAndVerifyAfterInitialTransforms(
    VPlan *Plan) {
  LLVM_DEBUG(dbgs() << *Plan);

  if (auto *VPlanVec = dyn_cast<VPlanVector>(Plan)) {

    LLVM_DEBUG(VPlanVec->printVectorVPlanData());
#ifndef NDEBUG
    // Run verifier after initial transforms in debug build.
    VPlanVerifier::verify(VPlanVec, TheLoop, VPlanVerifier::SkipLoopInfo);
#endif
    (void)VPlanVec;
  }

  VPLAN_DUMP(InitialTransformsDumpControl, Plan);
}

bool LoopVectorizationPlanner::canProcessVPlan(const VPlanVector &Plan) {
  VPLoop *VPLp = *(Plan.getVPLoopInfo()->begin());
  VPBasicBlock *Header = VPLp->getHeader();
  const VPLoopEntityList *LE = Plan.getLoopEntities(VPLp);
  if (!LE) {
    bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            INTERNAL("There are no loop entities (e.g., inductions or "
                     "reductions) for this loop."));
    return false;
  }
  // Check whether all reductions are supported.
  for (auto Red : LE->vpreductions()) {
    // Bailouts for user-defined reductions and scans.
    if ((Red->getRecurrenceKind() == RecurKind::Udr) ||
        isa<VPInscanReduction>(Red)) {
      // Check if UDR/Scan is registerized. This can happen due to
      // hoisting/invariant code motion done by pre-vectorizer passes.
      // Asserts in debug build to detect accidental registerization during
      // testing.
      if (!Red->getRecurrenceStartValue() ||
          LE->getMemoryDescriptor(Red) == nullptr || !Red->getIsMemOnly()) {
        bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                INTERNAL("A user-defined reduction or scan has been "
                         "registerized, and cannot be vectorized."));
        return false;
      }

      auto *UDS = dyn_cast<VPUserDefinedScanReduction>(Red);
      if (UDS && !UDS->getInitializer()) {
        bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
                AuxRemarkID::UDRWithoutInitializer);
        return false;
      }
    }

    // Bailouts for complex type reductions.
    if (Red->isComplex()) {
      auto *RecTy = cast<StructType>(Red->getRecurrenceType());
      assert(RecTy->isPossibleComplexFPType() &&
             "Complex type expected for reduction");
      auto RecTySize = Plan.getDataLayout()->getTypeAllocSize(RecTy);

      // Based on the unique element type, compute size of the equivalent packed
      // vector type that can represent the complex type. First element of
      // vector is the real component while second represents the imaginary
      // part. For example -
      // ComplexTy/RecTy = {double , double}
      // ElemTy = double
      // PackedVecTy = <2 x double>
      Type *ElemTy = RecTy->getElementType(0);
      auto *PackedVecTy = FixedVectorType::get(ElemTy, 2);
      auto PackedVecTySize =
          Plan.getDataLayout()->getTypeAllocSize(PackedVecTy);

      // For efficient initialization/finalization we try to pack complex type
      // into vector registers as mentioned above. Check if this is legal based
      // on type sizes.
      if (RecTySize != PackedVecTySize) {
        bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                INTERNAL("Complex struct type cannot be packed into "
                         "trivial vector type."));
        return false;
      }

      // Ensure that the complex struct type does not have any padding using the
      // type's alignment.
      unsigned OrigAlignV =
          Plan.getDataLayout()->getPrefTypeAlign(RecTy).value();
      if (RecTySize.getFixedValue() % OrigAlignV != 0) {
        bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                INTERNAL("Complex struct type size is not divisible by "
                         "its alignment; i.e., it has padding."));
        return false;
      }
    }

    // Check that reduction does not have more than one liveout instruction.
    // TODO: This scenario can potentially be handled in the future by emitting
    // a reduction-final for each liveout value.
    unsigned NumLiveOutInsts =
        llvm::count_if(Red->getLinkedVPValues(), [&VPLp](VPValue *V) {
          if (auto *I = dyn_cast<VPInstruction>(V))
            return VPLp->isLiveOut(I);
          return false;
        });
    // Consider the following input with non-simd loop.
    // DO i1 = 0, %N + -1, 1
    //   %add = %add21  +  i1; <Safe Reduction>
    //   ...
    //   %add21 = %add; <Safe Reduction>
    // END LOOP
    //
    // We will create the VPlan IR below. Here the %vp30972 is reduction
    // exit, but it's not real liveout. The real live out is %vp24048. That
    // discrepancy is resulted from HIR's incomplete temps matching. Both
    // statements are marked in one "safe reduction" chain but we don't
    // account this: even the chained statements are put into linked value list
    // of reduction, we don't look through that list during importing. Bail out
    // for now. TODO: implement the lookup and correct linking, see the test
    // in Intel_LoopTransforms/HIROptReport/loop-unswitch.ll
    //
    // i32 %vp16330 = phi  [ i32 %add21, BB21 ],  [ i32 %vp30972, BB22 ]
    // i32 %vp14746 = phi  [ i32 0, BB21 ],  [ i32 %vp23996, BB22 ]
    // i32 %vp24048 = add i32 %vp16330 i32 %vp14746
    // ...
    // i32 %vp30972 = hir-copy i32 %vp24048 , OriginPhiId: -1
    // i32 %vp23996 = add i32 %vp14746 i32 1
    // i1 %vp30894 = icmp slt i32 %vp23996 i32 %vp31834
    // br i1 %vp30894, BB22, BB23
    // 
    if (Red->getLoopExitInstr() && !VPLp->isLiveOut(Red->getLoopExitInstr()))
      NumLiveOutInsts++; // at the moment we make reduction exit as liveout.

    if (NumLiveOutInsts > 1) {
      bailout(OptReportVerbosity::Medium, OptRemarkID::VecFailGenericBailout,
              AuxRemarkID::MultipleLiveOutReduction);
      return false;
    }
  }

  // Check whether all header phis are recognized as entities.
  for (auto &Phi : Header->getVPPhis())
    if (!LE->getInduction(&Phi) && !LE->getReduction(&Phi) &&
        !LE->getPrivate(&Phi) && !LE->getCompressExpandIdiom(&Phi)) {
      // Non-entity phi. No other PHIs are expected in loop header since we are
      // working on plain CFG before any transforms.
      LLVM_DEBUG(dbgs() << "LVP: Unrecognized phi found.\n" << Phi << "\n");
      bailoutWithDebug(OptReportVerbosity::Medium,
                       OptRemarkID::VecFailUnknownRecurrence,
                       INTERNAL("Loop contains a recurrent computation that "
                                "could not be identified as an induction or "
                                "reduction."),
                       AuxRemarkID::Loop);
      return false;
    }
  if (!canProcessLoopBody(Plan, *VPLp)) {
    assert(BR.BailoutRemark && "canProcessLoopBody did not set bailout data!");
    return false;
  }

  // All safety checks passed.
  return true;
}

bool LoopVectorizationPlanner::canLowerVPlan(const VPlanVector &Plan,
                                             unsigned VF) {
  for (auto &VPI : vpinstructions(&Plan)) {
    // Check if an array private is marked as SOA profitable and if we
    // lack the needed codegen support.
    if (auto *AllocaPriv = dyn_cast<VPAllocatePrivate>(&VPI))
      if (AllocaPriv->isSOASafe() && AllocaPriv->isSOAProfitable() &&
          !isSOACodegenSupported() &&
          AllocaPriv->getAllocatedType()->isArrayTy()) {
        bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                INTERNAL("SOA transformation for privates is not yet "
                         "supported on this path."));
        LLVM_DEBUG(dbgs() << VPI << "\n");
        return false;
      }

    // Check if a library call that needs argument repacking transform has only
    // extractvalue users in VPlan. We can optimize this case to completely
    // eliminate redundant usage of aggregate types in the loop.
    if (auto *VPCall = dyn_cast<VPCallInstruction>(&VPI)) {
      auto *CalledFn = VPCall->getCalledFunction();
      if (!CalledFn)
        continue;

      // Check if call will be vectorized using vector library scenario.
      if (VPCall->getVectorizationScenario() !=
          VPCallInstruction::CallVecScenariosTy::LibraryFunc)
        continue;

      // Check if call needs argument repacking transform.
      if (!TLI->doesVectorFuncNeedArgRepacking(CalledFn->getName()))
        continue;

      // Check that all users of this call are extractvalue instructions.
      if (!all_of(VPCall->users(), [](VPUser *U) {
            return isa<VPInstruction>(U) &&
                   cast<VPInstruction>(U)->getOpcode() ==
                       Instruction::ExtractValue;
          })) {
        bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
                INTERNAL("Argument repacking call has non-optimizable "
                         "aggregate operations."));
        LLVM_DEBUG(dbgs() << VPI << "\n");
        return false;
      }
    }
  }

  // All checks passed.
  return true;
}

bool LoopVectorizationPlanner::isInvalidOMPConstructInSIMD(VPCallInstruction *VPCall) const {
  auto *CalledF = VPCall->getCalledFunction();
  auto *CI = VPCall->getUnderlyingCallInst();
  if (!CalledF || !CI)
    return false;

  // Check for lowered intrinsics
  // simd doesn't have a corresponding LibFunc_kmpc_*
  LibFunc TheLibFunc;
  if (TLI->getLibFunc(*CalledF, TheLibFunc) && TLI->isOMPLibFunc(TheLibFunc)) {
    switch (TheLibFunc) {
      case LibFunc_kmpc_atomic_compare_exchange:
      case LibFunc_kmpc_atomic_fixed4_add:
      case LibFunc_kmpc_atomic_float8_add:
      case LibFunc_kmpc_atomic_load:
      case LibFunc_kmpc_atomic_store:
        return false;
      default:
        LLVM_DEBUG(dbgs() << "LVP: unsupported nested OpenMP directive."
                          << *CI << "\n");
        return true;
    }
  }

  // Check for non-lowered intrinsics (llvm.directive.entry...)
  int DirID = vpo::VPOAnalysisUtils::getDirectiveID(CI);
  if (vpo::VPOAnalysisUtils::isBeginDirective(DirID) ||
      vpo::VPOAnalysisUtils::isStandAloneBeginDirective(DirID)) {
    switch (DirID) {
      case DIR_OMP_ATOMIC:
      case DIR_OMP_SIMD:
      case DIR_OMP_SCAN:
        return false;
      case DIR_OMP_ORDERED: {
        StringRef SimdClauseString =
            VPOAnalysisUtils::getClauseString(QUAL_OMP_ORDERED_SIMD);
        for (unsigned I = 0; I < CI->getNumOperandBundles(); ++I) {
          OperandBundleUse BU = CI->getOperandBundleAt(I);
          if (BU.getTagName() == SimdClauseString)
            return false;
        }
        LLVM_FALLTHROUGH;
      }
      default:
        LLVM_DEBUG(dbgs() << "LVP: unsupported nested OpenMP directive."
                          << *CI << "\n");
        return true;
    }
  }
  return false;
}

bool LoopVectorizationPlanner::canProcessLoopBody(const VPlanVector &Plan,
                                                  const VPLoop &Loop) const {
  // Check for live out values that are not inductions/reductions.
  // Their processing is not ready yet.
  if (EnableAllLiveOuts)
    return true;
  const VPLoopEntityList *LE = Plan.getLoopEntities(&Loop);
  assert(LE && "No loop entities for loop!");
  if (!LE) {
    bailout(OptReportVerbosity::High, OptRemarkID::VecFailGenericBailout,
            INTERNAL("There are no loop entities (e.g., inductions or "
                     "reductions) for this loop."));
    return false;
  }
  for (auto *BB : Loop.blocks())
    for (VPInstruction &Inst : *BB) {
        if (LE->getReduction(&Inst) || LE->getInduction(&Inst)) {
          // Entities code and CG need to be uplifted to handle vector type
          // inductions and reductions.
          if (isa<VectorType>(Inst.getType())) {
            bailoutWithDebug(OptReportVerbosity::Medium,
                             OptRemarkID::VecFailReducingVectorType,
                             INTERNAL("A reduction or induction of a vector "
                                      "type is not supported."),
                             AuxRemarkID::Loop);
            LLVM_DEBUG(dbgs() << Inst << "\n");
            return false;
          }
        } else if (Loop.isLiveOut(&Inst) && !LE->getPrivate(&Inst)) {
          // Some liveouts are left unrecognized due to unvectorizable use-def
          // chains.
          bailoutWithDebug(OptReportVerbosity::Medium,
                           OptRemarkID::VecFailUnknownLiveOut,
                           INTERNAL("Loop contains a live-out value that could "
                                    "not be identified as an induction or "
                                    "reduction."),
                           AuxRemarkID::Loop);
          return false;
        }
        if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst)) {
          if (isInvalidOMPConstructInSIMD(VPCall)) {
            bailout(OptReportVerbosity::Medium,
                    OptRemarkID::VecFailGenericBailout,
                    AuxRemarkID::IllegalOpenMPInSIMD);
            return false;
          }
        }
        // Detect presence of fences.  We expect to see them for in-scan
        // reductions, but other fences are forbidden.  If fences are
        // detected here, we will later scan to see if any remain after
        // reductions are processed.
        if (Inst.getOpcode() == Instruction::Fence)
          FencesInInput = true;
    }

  return true;
}

VPlanVector *LoopVectorizationPlanner::getBestVPlan() {
  unsigned VF = getBestVF();
  assert(VF != 0 && "best vplan is not selected");
  if (VecScenario.getMain().Kind == SingleLoopVecScenario::LKScalar) {
    // Check for consitency. Just in case.
    assert(VF == 1 && "expected VF=1 for scalar VPlan");
    return getVPlanForVF(VF);
  }
  // Get either masked or non-masked main VPlan, depending on
  // selection.
  return VecScenario.getMain().Kind == SingleLoopVecScenario::LKVector
           ? getVPlanForVF(VF)
           : getMaskedVPlanForVF(VF);
}

void LoopVectorizationPlanner::executeBestPlan(CodeGenLLVM &LB) {
  assert(getBestVF() > 1 && "Non-vectorized loop should be handled elsewhere!");
  ILV = &LB;

  // Perform the actual loop widening (vectorization).
  // 1. Create a new empty loop. Unlink the old loop and connect the new one.
  ILV->createEmptyLoop();

  // 2. Widen each instruction in the old loop to a new one in the new loop.
  VPCallbackILV CallbackILV;

  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  // Temporary, until CFG merge is implemented. Replace VPLiveInValue-s by
  // original incoming values.
  VPLiveInOutCreator LICreator(*Plan);
  LICreator.restoreLiveIns();

  // Run CallVecDecisions analysis for final VPlan which will be used by CG.
  VPlanCallVecDecisions CallVecDecisions(*Plan);
  std::string Label;
  CallVecDecisions.runForMergedCFG(TLI, TTI);
  Label = "CallVecDecisions analysis for merged CFG";
  VPLAN_DUMP(PrintAfterCallVecDecisions, Label, Plan);

  // Transform lib calls (like 'sincos') that need additional processing.
  VPTransformLibraryCalls VPTransLibCalls(*Plan, *TLI);
  VPTransLibCalls.transform();

  // Compute SVA results for final VPlan which will be used by CG.
  Plan->runSVA(getBestVF());
  VPLAN_DUMP(PrintSVAResults, "ScalVec analysis", Plan);

  VPTransformState State(getBestVF(), 1 /* UF */, LI, DT, ILV->getBuilder(),
                         ILV, CallbackILV, Plan->getVPLoopInfo());
  State.CFG.PrevBB = ILV->getLoopVectorPH();

  // Set ILV transform state
  ILV->setTransformState(&State);

  Plan->execute(&State);

  // 3. Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  ILV->finalizeLoop();
}

void LoopVectorizationPlanner::emitPeelRemainderVPLoops(unsigned VF, unsigned UF) {
  assert(getBestVF() > 1 && "Unexpected VF");
  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger CFGMerger(*Plan, VF, UF);

  // Run CFGMerger.
  CFGMerger.createMergedCFG(VecScenario, MergerVPlans, TopLoopDescrs, TheLoop);
}

void LoopVectorizationPlanner::createMergerVPlans(VPAnalysesFactoryBase &VPAF) {
  assert(MergerVPlans.empty() && "Non-empty list of VPlans");
  assert(getBestVF() > 1 && "Unexpected VF");

  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger::createPlans(*this, VecScenario, MergerVPlans, TheLoop,
                              *Plan, VPAF);
  fillLoopDescrs();
}

void LoopVectorizationPlanner::fillLoopDescrs() {
  for (const CfgMergerPlanDescr &PlanDescr : MergerVPlans) {
    VPlan *Plan = PlanDescr.getVPlan();
    if (isa<VPlanScalar>(Plan))
      continue;
    VPLoop *Lp = cast<VPlanVector>(Plan)->getMainLoop(true /* StrictCheck */);
    assert(Lp && "Expected non-null mainloop");
    TopLoopDescrs[Lp] = &PlanDescr;
  }
}
// Return true if the compare and branch sequence guarantees the loop trip count
// is meaningful, i.e. the loop is executed as a loop. For example, consider the
// following comination:
//  %c = cmp EQ i, N
//  br %c, label %loop_header, label %loop_exit
// Taking into account that the loop IV starts from 0, this condition will allow
// only one iteration and only when N is 0. In general, the loop should be
// optimized into a scalar sequence with the corresponding guard, e.g.
// Unoptimized version
//  loop_header:
//    %i = phi [0, %pre_header], [%add, %body]
//    loop_body
//    %c = cmp EQ %i, %N
//    br %c, label %loop_header, label %loop_exit
//  loop_exit:
//  ==>
// Optimized version (does not have any loop):
//  loop_header:
//    %c = cmp EQ 0, %N
//    br %c label %body, label %loop_exit
//  body:
//    loop_body
//  loop_exit:
//
// So having the condition and successors' order above we don't consider the
// loop as not having normalized induction.
//
// \p ExactUB is set to true if the loop trip count is equal to
// invariant operand of the compare and to false if tc is equal to that
// operand + 1.
static bool supportedCmpBranch(VPBasicBlock *Header, VPBasicBlock *Latch,
                               VPCmpInst *Cond, VPInstruction *AddI,
                               bool &ExactUB) {
  auto Pred = Cond->getPredicate();
  bool IsFirstOpIV = Cond->getOperand(0) == AddI;
  bool IsFirstSuccHeader = Latch->getSuccessor(0) == Header;

  ExactUB = true;
  // Starting induction value from 0, we have the following iteration counts
  // for different combinations of latch condition and successors
  // EQ i, N                 loop impossible
  //   br header, exit
  // EQ i, N                 N
  //   br exit, header
  // EQ N, i                 loop impossible
  //   br header, exit
  // EQ N, i                 N
  //   br exit, header
  if (Pred == CmpInst::ICMP_EQ && !IsFirstSuccHeader)
    return true;

  // NE i, N                 N
  //   br header, exit
  // NE i, N                 loop impossible
  //   br exit, header
  // NE N, i                 N
  //   br header, exit
  // NE N, i                 loop impossible
  //   br exit, header
  if (Pred == CmpInst::ICMP_NE && IsFirstSuccHeader)
    return true;

  // LT i, N                 N
  //   br header, exi
  // LT i, N                 loop impossible
  //   br exit, header
  // LT N, i                 loop impossible
  //   br header, exit
  // LT N, i                 N + 1
  //   br exit, header
  if (ICmpInst::isLT(Pred) && ((IsFirstOpIV && IsFirstSuccHeader) ||
                               (!IsFirstOpIV && !IsFirstSuccHeader))) {
    ExactUB = (IsFirstOpIV && IsFirstSuccHeader);
    return true;
  }

  // GE i, N                 loop impossible
  //   br header, exit
  // GE i, N                 N
  //   br exit, header
  // GE N, i                 N +1
  //   br header, exit
  // GE N,i                  loop impossible
  //   Br exit, header
  if (ICmpInst::isGE(Pred) && ((IsFirstOpIV && !IsFirstSuccHeader) ||
                               (!IsFirstOpIV && IsFirstSuccHeader))) {
    ExactUB = (IsFirstOpIV && !IsFirstSuccHeader);
    return true;
  }

  // GT i, N                 loop impossible
  //   br header, exit
  // GT i, N                 N + 1
  //   br exit, header
  // GT N, i                 N
  //   br header, exit
  // GT N, i                 loop impossible
  //  br exit, header
  if (ICmpInst::isGT(Pred) && ((!IsFirstOpIV && IsFirstSuccHeader) ||
                               (IsFirstOpIV && !IsFirstSuccHeader))) {
    ExactUB = (!IsFirstOpIV && IsFirstSuccHeader);
    return true;
  }

  // LE i, N                 N+1
  //  br header, exit
  // LE i, N                 loop impossible
  //  br exit, header
  // LE N, i                 loop impossible
  //  br header, exit
  // LE N, i                 N
  //  br exit, header
  if (ICmpInst::isLE(Pred) && ((!IsFirstOpIV && !IsFirstSuccHeader) ||
                               (IsFirstOpIV && IsFirstSuccHeader))) {
    ExactUB = (!IsFirstOpIV && !IsFirstSuccHeader);
    return true;
  }
  return false;
}

// The following code snippet illustrates what is detected by the
// function.
// ...
// %ind.step = induction-init-step{add} i64 1
// ...
// %ind.phi = phi i64 [0, %preheader], [%add, %loop_latch]
// ...
// %add = add i64 %ind.phi, %ind.step
// %cmp = icmp sle i64 %add, %loop.invariant
//
bool LoopVectorizationPlanner::hasLoopNormalizedInduction(const VPLoop *Loop,
                                                          bool &ExactUB) {
  ExactUB = true;
  VPBasicBlock *Latch = Loop->getLoopLatch();
  if (!Latch)
    return false;
  VPBranchInst *Br = Latch->getTerminator();
  if (!Br || Br->getCondition() == nullptr)
    return false;
  VPCmpInst *Cond = dyn_cast<VPCmpInst>(Br->getCondition());
  if (!Cond)
    return false;
  if (Cond->getNumUsers() != 1)
    return false;

  VPBasicBlock *Header = Loop->getHeader();
  VPInstruction *AddI = nullptr;
  auto getAddInstr = [&AddI, Cond, Loop](int NumOp) -> bool {
    // Check that the NumOp-th operand of Cond is an "add" instruction inside
    // the loop with one of operands equal to InductionInitStep(1) and another
    // operand of Cond is a loop invariant. The add instriuction is stored to
    // AddI for further checks.
    // In the example above it's the %add instruction.
    AddI = dyn_cast<VPInstruction>(Cond->getOperand(NumOp));
    if (AddI && AddI->getOpcode() == Instruction::Add && Loop->contains(AddI)) {
      VPValue *SecOp = Cond->getOperand(NumOp ^ 1);
      // Check upper bound.
      if (!Loop->isDefOutside(SecOp) && !isa<VPConstant>(SecOp))
        return false;
      // Check step.
      if (auto *StepInit = dyn_cast<VPInductionInitStep>(AddI->getOperand(0)))
        SecOp = StepInit->getOperand(0);
      else if (auto *StepInit =
                   dyn_cast<VPInductionInitStep>(AddI->getOperand(1)))
        SecOp = StepInit->getOperand(0);
      else
        return false;

      if (VPConstantInt *Step = dyn_cast<VPConstantInt>(SecOp))
        return Step->getValue() == 1;
    }
    return false;
  };
  if (getAddInstr(0) || getAddInstr(1)) {
    VPBasicBlock *Preheader = Loop->getLoopPreheader();
    // Check that increment is used only in condition and in phi.
    for (auto *U : AddI->users()) {
      if (U == Cond)
        continue;
      VPPHINode *PN = dyn_cast<VPPHINode>(U);
      if (!PN)
        return false;
      if (PN->getParent() != Header)
        return false;
      // Header phi, check start value for 0.
      VPValue *Init = PN->getIncomingValue(Preheader);
      if (auto IndInit = dyn_cast<VPInductionInit>(Init)) {
        Init = IndInit->getStartValueOperand();
      }
      if (isa<VPConstantInt>(Init) &&
          cast<VPConstantInt>(Init)->getValue() == 0)
        continue;
      // The starting value is not induction-init(0).
      return false;
    }
    // All checks succeeded so far. Return true if compare and branch
    // are in supported form.
    return supportedCmpBranch(Header, Latch, Cond, AddI, ExactUB);
  }
  return false;
}

// Transforms the integer masked div/rem instructions blending the divisor with
// a safe value (1). The transformation looks like below:
//   %p = block-predicate i1 ...
//   ...
//   %r = idiv i32 %a, %b
// ==>
//   %p = block-predicate i1 ...
//   ...
//   %safe_divisor = select i1 %p, i32 %b, 1
//   %r = idiv i32 %a, %safe_divisor
//
void LoopVectorizationPlanner::blendWithSafeValue() {

  if (!EnableIntDivRemBlendWithSafeValue)
    return;

  auto ProcessVPlan = [](VPlanVector &P) -> void {
    auto NeedProcessInst = [&P](const VPInstruction &Inst) {
      auto Opcode = Inst.getOpcode();
      if (Opcode != Instruction::SDiv && Opcode != Instruction::UDiv &&
          Opcode != Instruction::SRem && Opcode != Instruction::URem)
        return false;

      // Skipping non masked div/rem.
      if (!Inst.getParent()->getPredicate())
        return false;

      // And uniform too, they are processed in a special way in CG.
      if (P.getVPlanDA()->isUniform(Inst))
        return false;

      // When the divisor is unsafe we blend with safe value.
      return !isDivisorSpeculationSafeForDivRem(Inst.getOpcode(),
                                                Inst.getOperand(1));
    };

    SmallVector<VPInstruction *, 4> InstrToProcess(
        map_range(make_filter_range(vpinstructions(&P), NeedProcessInst),
                  [](VPInstruction &I) { return &I; }));

    VPBuilder Builder;
    for (auto *Inst : InstrToProcess) {
      Builder.setInsertPoint(Inst);
      // Get divisor and insert 1 in masked off lanes, to ensure we don't have
      // unwanted exception raised.
      auto *Op = Inst->getOperand(1);
      auto *VPOne = P.getVPConstant(ConstantInt::get(Op->getType(), 1));
      VPValue *SafeOp =
          Builder.createSelect(Inst->getParent()->getPredicate(), Op, VPOne);
      P.getVPlanDA()->updateDivergence(*SafeOp);
      Inst->replaceUsesOfWith(Op, SafeOp);
    }
    VPLAN_DUMP(BlendWithSafeValueDumpControl, P);
  };

  transformAllVPlans(ProcessVPlan);
}

void LoopVectorizationPlanner::disableNegOneStrideOptInMaskedModeVPlans() {

  auto ProcessVPlan = [](VPlanVector &Plan) -> void {
    if (!isa<VPlanMasked>(Plan))
      return;

    // Don't disable optimization if TC is unknown.
    VPLoop *L = Plan.getMainLoop(true /*StrictCheck*/);
    if (L->getTripCountInfo().IsEstimated)
      return;

    for (auto &Inst :
         make_filter_range(vpinstructions(Plan), [](const VPInstruction &Inst) {
           return isa<VPLoadStoreInst>(Inst);
         }))
      cast<VPLoadStoreInst>(Inst).disableNegOneOpt();
  };

  transformAllVPlans(ProcessVPlan);
}

void LoopVectorizationPlanner::runPeepholeBeforePredicator() {

  auto ProcessPlan = [](VPlan &Plan) -> void {
    // Bailout of optimization for multi-exit outer loops. They are mostly idiom
    // recognized as search loops.
    // TODO: This should be removed when search loops are represented explicitly
    // in VPlan.
    if (auto *VecPlan = dyn_cast<VPlanVector>(&Plan)) {
      if (VecPlan->getMainLoop(true)->getExitBlock() == nullptr)
        return;
    }

    SmallVector<VPInstruction *, 4> InstToRemove;

    auto ReplaceTruncZExtByAnd = [&InstToRemove,
                                  &Plan](VPInstruction &Inst) -> bool {
      // Perform the following transformation
      //   %t = trunc ty1 %op to ty2
      //   %r = zext %t to ty1
      // ==>
      //   %r = and ty1 %op, ty2 all_ones
      //
      VPInstruction *TruncI = nullptr, *Operand = nullptr;
      if (!match(&Inst, m_ZExt(m_Bind(TruncI))) ||
          !match(TruncI, m_Trunc(m_Bind(Operand))))
        return false;
      if (TruncI->getNumUsers() != 1)
        return false;
      if (Operand->getType() != Inst.getType())
        return false;
      // We don't check postdomination due to that is not needed. E.g. even we
      // have something like below we can safely replace ZExt by And, we will
      // just have masked And and all uses of ZExt are under the same mask.
      //
      // %t = trunc typeOf_v %v to someTy
      // if (%cond)
      //   %r = zext SomeTy %t to typeOf_v
      //
      VPBuilder Builder;
      Builder.setInsertPoint(&Inst);
      APInt V = APInt::getAllOnes(TruncI->getType()->getPrimitiveSizeInBits())
                    .zext(Operand->getType()->getPrimitiveSizeInBits());
      VPInstruction *AndI = Builder.createAnd(
          Operand, Plan.getVPConstant(ConstantInt::get(Operand->getType(), V)));
      AndI->setDebugLocation(Inst.getDebugLocation());
      Inst.replaceAllUsesWith(AndI);
      VPlanDivergenceAnalysis *DA =
          cast<VPlanDivergenceAnalysis>(Plan.getVPlanDA());
      // Copy shape from the zext instruction. Avoid calling updateDivergence()
      // because this will invalidate underlying IR and will overwrite any
      // results computed from improveStrideUsingIR in DA. In addition, it's
      // not necessary to do so and will take more compile time.
      DA->updateVectorShape(AndI, DA->getVectorShape(Inst));
      InstToRemove.push_back(&Inst); // Need first to remove the use
      InstToRemove.push_back(TruncI);
      return true;
    };

    for (auto &Inst : vpinstructions(&Plan)) {
      if (ReplaceTruncZExtByAnd(Inst))
        continue; // looks weird at the moment, leaving for further adding
    }
    for (auto *Inst : InstToRemove) {
      Inst->getParent()->eraseInstruction(Inst);
    }
    VPLAN_DUMP(EarlyPeepholeDumpControl, Plan);
  };

  transformAllVPlans(ProcessPlan);
}
