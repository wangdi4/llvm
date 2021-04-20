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
#include "IntelVPlanCFGMerger.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanClone.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanEvaluator.h"
#include "IntelVPlanHCFGBuilder.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanLCSSA.h"
#include "IntelVPlanLoopCFU.h"
#include "IntelVPlanLoopExitCanonicalization.h"
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

static cl::opt<unsigned> VecThreshold(
    "vec-threshold",
    cl::desc("sets a threshold for the vectorization on the probability"
             "of profitable execution of the vectorized loop in parallel."),
    cl::init(100));
#endif // INTEL_CUSTOMIZATION

static cl::opt<unsigned> VPlanForceVF("vplan-force-vf", cl::init(0),
                                      cl::desc("Force VPlan to use given VF"));

static cl::opt<bool>
    DisableVPlanPredicator("disable-vplan-predicator", cl::init(false),
                           cl::Hidden, cl::desc("Disable VPlan predicator."));
static cl::opt<bool>
    EnableCFGMerge("vplan-enable-cfg-merge", cl::init(true), cl::Hidden,
                   cl::desc("Enable CFG merge before VPlan code gen."));

static cl::opt<bool, true>
    EnableNewCFGMergeOpt("vplan-enable-new-cfg-merge", cl::Hidden,
                         cl::location(EnableNewCFGMerge),
                         cl::desc("Enable the new CFG merger."));

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

static cl::opt<bool> EnableGeneralPeelingCostModel(
    "vplan-enable-general-peeling-cost-model", cl::init(true),
    cl::desc("Use more advanced general cost model instead of a simple one for "
             "peeling decisions"));

static cl::opt<bool> PrintVecScenario(
    "vplan-print-vec-scenario", cl::init(false), cl::Hidden,
    cl::desc("Print selected single loop vectorization scenario"));

static LoopVPlanDumpControl LCSSADumpControl("lcssa", "LCSSA transformation");
static LoopVPlanDumpControl LoopCFUDumpControl("loop-cfu",
                                               "LoopCFU transformation");
static LoopVPlanDumpControl
    LinearizationDumpControl("linearization", "predication and linearization");
static LoopVPlanDumpControl
    AllZeroBypassDumpControl("all-zero-bypass", "all zero bypass insertion");
static LoopVPlanDumpControl
    LiveInOutListsDumpControl("live-inout-list", "live in/out lists creation");

static LoopVPlanDumpControl LoopMassagingDumpControl("loop-massaging",
                                                     "loop massaging");

static LoopVPlanDumpControl
    VPEntityInstructionsDumpControl("vpentity-instrs",
                                    "insertion of VPEntities instructions");
static LoopVPlanDumpControl
    InitialTransformsDumpControl("initial-transforms",
                                 "initial VPlan transforms");

static LoopVPlanDumpControl CfgMergeDumpControl("cfg-merge",
                                                "CFG merge before CG");

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
static int VPlanOrderNumber = 0;

using namespace llvm;
using namespace llvm::vpo;

namespace llvm {
namespace vpo {
bool PrintSVAResults = false;
bool PrintAfterCallVecDecisions = false;
bool LoopMassagingEnabled = true;
bool EnableSOAAnalysis = false;
bool EnableNewCFGMerge = false;
} // namespace vpo
} // namespace llvm

unsigned getForcedVF(const WRNVecLoopNode *WRLp) {
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

int LoopVectorizationPlanner::setDefaultVectorFactors(MDNode *MD) {
  unsigned ForcedVF = getForcedVF(WRLp);

#if INTEL_CUSTOMIZATION
  unsigned Safelen = getSafelen(WRLp);

  LLVM_DEBUG(dbgs() << "LVP: ForcedVF: " << ForcedVF << "\n");
  LLVM_DEBUG(dbgs() << "LVP: Safelen: " << Safelen << "\n");

  // Early return from vectorizer if forced VF or safelen is 1
  if (ForcedVF == 1 || Safelen == 1) {
    LLVM_DEBUG(dbgs() << "LVP: The forced VF or safelen specified by user is "
                         "1, VPlans need not be constructed.\n");
    VFs.push_back(0);
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
      VFs.push_back(0);
      return 0;
    }
#endif // INTEL_CUSTOMIZATION
    VFs.push_back(ForcedVF);
#if INTEL_CUSTOMIZATION
    LLVM_DEBUG(dbgs() << "LVP: MinVF: " << ForcedVF << " MaxVF: " << ForcedVF
                      << "\n");
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  } else if (VPlanConstrStressTest) {
    // If we are only stress testing VPlan construction, force VPlan
    // construction for just VF 1. This avoids any divide by zero errors in the
    // min/max VF computation.
    VFs.push_back(1);
#endif // INTEL_CUSTOMIZATION
  } else if (MD != nullptr) {
    extractVFsFromMetadata(MD, Safelen);
  } else {
    unsigned MinWidthInBits, MaxWidthInBits, MinVF, MaxVF;
    std::tie(MinWidthInBits, MaxWidthInBits) = getTypesWidthRangeInBits();
    const unsigned MinVectorWidth = TTI->getMinVectorRegisterBitWidth();
    const unsigned MaxVectorWidth =
        TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector)
            .getFixedSize();
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
#if INTEL_CUSTOMIZATION
    LLVM_DEBUG(dbgs() << "LVP: MinVF: " << MinVF << " MaxVF: " << MaxVF
                      << "\n");
#endif // INTEL_CUSTOMIZATION
    if (MinVF > MaxVF) {
      VFs.push_back(0);
      return 0;
    }
    for (unsigned VF = MinVF; VF <= MaxVF; VF *= 2)
      VFs.push_back(VF);
  }
#if INTEL_CUSTOMIZATION
  // TODO: add information about specified vector lengths in opt-report
  DEBUG_WITH_TYPE("LoopVectorizationPlanner_vec_lengths",
                  dbgs() << "LVP: Specified vectorlengths: ");
  DEBUG_WITH_TYPE("LoopVectorizationPlanner_vec_lengths",
                  for (unsigned VF
                       : getVectorFactors()) dbgs()
                      << VF << " ";
                  dbgs() << "\n";);
#endif // INTEL_CUSTOMIZATION
  return 1;
}

unsigned LoopVectorizationPlanner::buildInitialVPlans(MDNode *MD,
                                                      LLVMContext *Context,
                                                      const DataLayout *DL,
                                                      std::string VPlanName,
                                                      ScalarEvolution *SE) {
  ++VPlanOrderNumber;
  setDefaultVectorFactors(MD);
  if (VFs[0] == 0)
    return 0;

  Externals = std::make_unique<VPExternalValues>(Context, DL);
  UnlinkedVPInsts = std::make_unique<VPUnlinkedInstructions>();

  // TODO: revisit when we build multiple VPlans.
  std::shared_ptr<VPlanVector> Plan = buildInitialVPlan(*Externals, *UnlinkedVPInsts,
                                                        VPlanName, SE);

  VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  LE->analyzeImplicitLastPrivates();

  // Check legality of VPlan before proceeding with other transforms/analyses.
  if (!canProcessVPlan(*Plan.get())) {
    LLVM_DEBUG(dbgs() << "LVP: VPlan is not legal to process, bailing out.\n");
    return 0;
  }

  // TODO: Implement computeScalarVPlanCost()

  // Run initial set of vectorization specific transforms on plain VPlan CFG
  // obtained from CFGBuilder. Includes the following set of transforms -
  // 1. emitVPEntityInstrs
  // 2. emitVecSpecifics
  runInitialVecSpecificTransforms(Plan.get());

  createLiveInOutLists(*Plan.get());

  // CFG canonicalization transform (merge loop exits).
  doLoopMassaging(Plan.get());

  printAndVerifyAfterInitialTransforms(Plan.get());

  auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
  Plan->setVPlanDA(std::move(VPDA));
  Plan->computeDT();
  Plan->computePDT();
  Plan->computeDA();

  // TODO: Insert initial run of SVA here for any new users before CM & CG.
  for (unsigned TmpVF : VFs)
    VPlans[TmpVF] = {Plan, nullptr};

  // Always capture scalar VPlan to handle cases where vectorization
  // is not possible with VF > 1 (such as when forced VF greater than TC).
  VPlans[1] = VPlans[VFs[0] /*MinVF*/];

  return 1;
}

void LoopVectorizationPlanner::createLiveInOutLists(VPlanVector &Plan) {
  VPLiveInOutCreator LICreator(Plan);
  LICreator.createInOutValues(TheLoop);
  VPLAN_DUMP(LiveInOutListsDumpControl, Plan);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (LiveInOutListsDumpControl.dumpPlain())
    Plan.getExternals().dumpScalarInOuts(outs(), TheLoop);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

void LoopVectorizationPlanner::selectBestPeelingVariants() {
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
    if (EnableGeneralPeelingCostModel) {
      VPlanCostModel GeneralCM(Plan, VF, TTI, TLI, DL, VLSA);
      VPlanPeelingCostModelGeneral PeelingCM(GeneralCM);
      BestPeeling = VPPA.selectBestPeelingVariant(VF, PeelingCM);
    } else {
      VPlanPeelingCostModelSimple PeelingCM(*DL);
      BestPeeling = VPPA.selectBestPeelingVariant(VF, PeelingCM);
    }

    Plan->setPreferredPeeling(VF, std::move(BestPeeling));
  }
}

unsigned LoopVectorizationPlanner::getLoopUnrollFactor(bool *Forced) {
  if (VPlanForceUF == 0) {
    if (Forced)
      *Forced = false;
    return 1;
  }

  if (Forced)
    *Forced = true;
  return VPlanForceUF;
}

void LoopVectorizationPlanner::extractVFsFromMetadata(MDNode *MD,
                                                      unsigned Safelen) {
  SmallVector<unsigned, 5> TmpVFs;
  for (unsigned I = 1; I < (MD->getNumOperands()); I++) {
    ConstantInt *IntMD = mdconst::extract<ConstantInt>(MD->getOperand(I));
    if (IntMD->getZExtValue() <= Safelen)
      TmpVFs.push_back(IntMD->getZExtValue());
  }

  llvm::sort(TmpVFs);
  auto NewEnd = std::unique(TmpVFs.begin(), TmpVFs.end());
  TmpVFs.erase(NewEnd, TmpVFs.end());

  unsigned I = 0;
  if(TmpVFs.size() > 1)
    if (TmpVFs[0] <= 1)
      I = 1;
  if(TmpVFs.size() > 2)
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

ArrayRef<unsigned> LoopVectorizationPlanner::getVectorFactors() { return VFs; }

void LoopVectorizationPlanner::selectSimplestVecScenario(unsigned VF,
                                                         unsigned UF) {
  VecScenario.resetPeel();
  VecScenario.resetRemainders();
  VecScenario.addScalarRemainder();
  VecScenario.setVectorMain(VF, UF);
}

/// Evaluate cost model for available VPlans and find the best one.
/// \Returns pair: VF which corresponds to the best VPlan (could be VF = 1) and
/// the corresponding VPlan.
template <typename CostModelTy>
std::pair<unsigned, VPlanVector *> LoopVectorizationPlanner::selectBestPlan() {
  LLVM_DEBUG(dbgs() << "Selecting VF for VPlan #" << VPlanOrderNumber << '\n');
  VPlanVector *ScalarPlan = getVPlanForVF(1);
  assert(ScalarPlan && "There is no scalar VPlan!");
  // FIXME: Without peel and remainder vectorization, it's ok to get trip count
  // from the original loop. Has to be revisited after enabling of
  // peel/remainder vectorization.

  // Even if TripCount is more than 2^32 we can safely assume that it's equal
  // to 2^32, otherwise all logic below will have a problem with overflow.
  VPLoop *OuterMostVPLoop = ScalarPlan->getMainLoop(true);
  uint64_t TripCount = std::min(OuterMostVPLoop->getTripCountInfo().TripCount,
                                (uint64_t)std::numeric_limits<unsigned>::max());
  unsigned BestUF = getLoopUnrollFactor();
  unsigned ForcedVF = getForcedVF(WRLp);

  // Reset the current selection to scalar loop only.
  VecScenario.resetPeel();
  VecScenario.resetMain();
  VecScenario.resetRemainders();

  if (ForcedVF > 0) {
    if (ForcedVF * BestUF > TripCount) {
      LLVM_DEBUG(dbgs() << "Bailing out to scalar VPlan because ForcedVF("
                        << ForcedVF << ") * BestUF(" << BestUF
                        << ") > TripCount(" << TripCount << ")\n");
      // The scenario was reset just before the check.
      return std::make_pair(VecScenario.getMainVF(), getBestVPlan());
    }

    // FIXME: this code should be revisited later to select best UF
    // even with forced VF.
    // FIXME: We may want to use BestUF in the Unroller.
    LLVM_DEBUG(dbgs() << "There is only VPlan with VF=" << ForcedVF
                      << ", selecting it.\n");
  }

  CostModelTy ScalarCM(ScalarPlan, 1, TTI, TLI, DL);
  unsigned ScalarIterationCost = ScalarCM.getCost();
  ScalarIterationCost =
      ScalarIterationCost == CostModelTy::UnknownCost ? 0 : ScalarIterationCost;
  // FIXME: that multiplication should be the part of CostModel - see below.
  uint64_t ScalarCost = ScalarIterationCost * TripCount;

  if (ForcedVF > 0) {
    assert((VFs.size() == 1 && VFs[0] == ForcedVF && hasVPlanForVF(VFs[0])) &&
           "Expected only one forced VF and non-null VPlan");
  }

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
  //
  // The main loop where we choose VF.
  // Please note that best peeling variant is expected to be selected up to
  // this moment.  Now we check whether the best peeling is profitable VS no
  // peeling.
  for (unsigned VF : getVectorFactors()) {
    assert(hasVPlanForVF(VF) && "expected non-null VPlan");
    if (TripCount < VF * BestUF)
      continue; // FIXME: Consider masked low trip later.

    VPlanVector *Plan = getVPlanForVF(VF);
    assert(Plan && "Unexpected null VPlan");

    // Calculate cost for one iteration of the main loop.
    CostModelTy MainLoopCM(Plan, VF, TTI, TLI, DL, VLSA);
    const unsigned MainLoopIterationCost = MainLoopCM.getCost();
    if (MainLoopIterationCost == CostModelTy::UnknownCost) {
      LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << " is unknown. Skip it.\n");
      if (VF == ForcedVF) {
        // If the VF is forced and loop cost for it is unknown select the
        // simplest configuration: non-masked main loop + scalar remainder.
        selectSimplestVecScenario(ForcedVF, BestUF);
      }
      continue;
    }
    VPlanPeelingVariant *PeelingVariant = Plan->getPreferredPeeling(VF);
    // Calculate the total cost of peel loop if there is one.
    VPlanPeelEvaluator PeelEvaluator(*this, ScalarIterationCost, TLI, TTI, DL,
                                     VLSA, VF, PeelingVariant);

    // Calculate the total cost of remainder loop if there is one.
    VPlanRemainderEvaluator RemainderEvaluator(
        *this, ScalarIterationCost, TLI, TTI, DL, VLSA, TripCount,
        PeelEvaluator.getTripCount(), VF, BestUF);

    // Calculate main loop's trip count. Currently, the unroll factor is set to
    // 1 because VPlan's loop unroller is called after selecting the best VF.
    const decltype(TripCount) MainLoopTripCount =
        (TripCount - PeelEvaluator.getTripCount()) / (VF * BestUF);

    // The total vector cost is calculated by adding the total cost of peel,
    // main and remainder loops.
    uint64_t VectorCost = PeelEvaluator.getLoopCost() +
                          MainLoopIterationCost * MainLoopTripCount +
                          RemainderEvaluator.getLoopCost();

    if (0 < VecThreshold && VecThreshold < 100) {
      LLVM_DEBUG(dbgs() << "Applying threshold " << VecThreshold << " for VF "
                        << VF << ". Original cost = " << VectorCost << '\n');
      VectorCost = (VectorCost * VecThreshold) / 100;
    }
    const char CmpChar =
        ScalarCost < VectorCost ? '<' : ScalarCost == VectorCost ? '=' : '>';
    (void)CmpChar;
    LLVM_DEBUG(
        dbgs() << "Scalar Cost = " << TripCount << " x " << ScalarIterationCost
               << " = " << ScalarCost << ' ' << CmpChar
               << " VectorCost = " << PeelEvaluator.getLoopCost() << " + "
               << MainLoopTripCount << " x " << MainLoopIterationCost << " + "
               << RemainderEvaluator.getLoopCost() << " = " << VectorCost
               << '\n'
               << "Peel loop cost = " << PeelEvaluator.getLoopCost() << " ("
               << PeelEvaluator.getPeelLoopKindStr() << ")"
               << "\n"
               << "Main loop vector cost = "
               << MainLoopTripCount * MainLoopIterationCost << "\n"
               << "Remainder loop cost = " << RemainderEvaluator.getLoopCost()
               << " (" << RemainderEvaluator.getRemainderLoopKindStr() << ")"
               << "\n";);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (PrintAfterEvaluator) {
      dbgs() << "Evaluators for VF=" << VF << "\n";
      PeelEvaluator.dump();
      dbgs() << "The main loop is vectorized with vector factor " << VF
             << ". The vector cost is "
             << MainLoopTripCount * MainLoopIterationCost << "("
             << MainLoopTripCount << " x " << MainLoopIterationCost << "). \n";
      RemainderEvaluator.dump();
    }
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    if (VectorCost < BestCost || VF == ForcedVF) {
      BestCost = VectorCost;
      updateVecScenario(PeelEvaluator, RemainderEvaluator, VF, BestUF);
    }
  }
#if INTEL_CUSTOMIZATION
  // Corner case: all available VPlans have UMAX cost.
  // With 'vector always' we have to vectorize with some VF, so select first
  // available VF.
  if (VecScenario.getMainVF() == 1 && IsVectorAlways) {
    selectSimplestVecScenario(VFs[0], 1);
  }
#endif // INTEL_CUSTOMIZATION
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (!VecScenarioStr.empty()) {
    VecScenario.fromString(VecScenarioStr);
    SmallSet<unsigned, 4> UsedVFs;
    VecScenario.getUsedVFs(UsedVFs);
    for (auto VF : UsedVFs) {
      VPlan *P = getVPlanForVF(VF);
      assert(P && "The VPlan for VF from string does not exist");
    }
    // just in case, to not face assert on null VPlan
    unsigned VF = VecScenario.getMainVF();
    if (VecScenario.hasPeel()) {
      VPlanVector *MPlan = getVPlanForVF(VF);
      VPLoop *L = *(MPlan->getVPLoopInfo())->begin();
      if (VecScenario.hasMaskedPeel() && !hasLoopNormalizedInduction(L)) {
        // replace by scalar loop if there is no normalized induction
        VecScenario.setScalarPeel();
      }
      if (!MPlan->getPreferredPeeling(VF))
        MPlan->setPreferredPeeling(VF, std::make_unique<VPlanStaticPeeling>(1));
    }
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

  return std::make_pair(getBestVF(), getBestVPlan());
}

void LoopVectorizationPlanner::updateVecScenario(
    VPlanPeelEvaluator const &PE, VPlanRemainderEvaluator const &RE,
    unsigned VF, unsigned UF) {
  if (!isNewCFGMergeEnabled()) {
    selectSimplestVecScenario(VF, UF);
    return;
  }
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
    break;
  }
  // TODO: need update for cases when we select masked mode for main loop.
  VecScenario.setVectorMain(VF, UF);
}

template std::pair<unsigned, VPlanVector *>
LoopVectorizationPlanner::selectBestPlan<VPlanCostModel>(void);
#if INTEL_CUSTOMIZATION
template std::pair<unsigned, VPlanVector *>
LoopVectorizationPlanner::selectBestPlan<VPlanCostModelProprietary>(void);
#endif // INTEL_CUSTOMIZATION

void LoopVectorizationPlanner::predicate() {
  if (DisableVPlanPredicator)
    return;

  DenseSet<VPlanVector *> PredicatedVPlans;
  auto Predicate = [&PredicatedVPlans](VPlanVector *VPlan) {
    if (PredicatedVPlans.count(VPlan))
      return; // Already predicated.

    VPLoop *OuterLoop = VPlan->getMainLoop(true);
    // Search loops require multiple hacks. Skipping LCSSA/LoopCFU is one of
    // them.
    bool SearchLoopHack = !OuterLoop->getExitBlock();

    if (!SearchLoopHack)
      formLCSSA(*VPlan, true /* SkipTopLoop */);
    VPLAN_DUMP(LCSSADumpControl, VPlan);

    if (!SearchLoopHack) {
      assert(!VPlan->getVPlanDA()->isDivergent(
                 *(OuterLoop)->getLoopLatch()->getCondBit()) &&
             "Outer loop doesn't have uniform backedge!");
      VPlanLoopCFU LoopCFU(*VPlan);
      LoopCFU.run();
    }
    VPLAN_DUMP(LoopCFUDumpControl, VPlan);

    // Predication "has" to be done even for the search loop hack. Our
    // idiom-matching code and CG currently expect that. Note that predicator
    // has some hacks for search loop processing inside it as well.
    VPlanPredicator VPP(*VPlan);
    VPP.predicate();
    VPLAN_DUMP(LinearizationDumpControl, VPlan);

    PredicatedVPlans.insert(VPlan);
  };

  for (auto It : VPlans) {
    if (It.first == 1)
      continue; // Ignore Scalar VPlan;
    VPlanVector *MainPlan = It.second.MainPlan.get();
    Predicate(MainPlan);
    // Masked mode loop might not exist.
    if (VPlanVector *MaskedModeLoopPlan = It.second.MaskedModeLoop.get())
      Predicate(MaskedModeLoopPlan);
  }
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
    VPlanCostModelProprietary VectorCM(Plan, VF, TTI, TLI, DL, VLSA);
    AZB.collectAllZeroBypassNonLoopRegions(AllZeroBypassRegions,
                                           RegionsCollected,
                                           &VectorCM);
  }
  AZB.insertAllZeroBypasses(AllZeroBypassRegions);
  VPLAN_DUMP(AllZeroBypassDumpControl, Plan);
}

void LoopVectorizationPlanner::unroll(
    VPlanVector &Plan,
    VPlanLoopUnroller::VPInstUnrollPartTy *VPInstUnrollPart) {
  unsigned UF = getLoopUnrollFactor();
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
    VPlanVector *Plan = getVPlanForVF(VFRequested);
    assert(Plan && "Unexpected null VPlan");

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

std::shared_ptr<VPlanVector> LoopVectorizationPlanner::buildInitialVPlan(
    VPExternalValues &Ext,
    VPUnlinkedInstructions &UnlinkedVPInsts, std::string VPlanName,
    ScalarEvolution *SE) {
  // Create new empty VPlan. At this stage we want to create only NonMasked
  // VPlans.
  std::shared_ptr<VPlanVector> SharedPlan =
      std::make_shared<VPlanNonMasked>(Ext, UnlinkedVPInsts);
  VPlanVector *Plan = SharedPlan.get();
  Plan->setName(VPlanName);

  if (EnableSOAAnalysis)
    // Enable SOA-analysis.
    Plan->enableSOAAnalysis();

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(TheLoop, LI, *DL, WRLp, Plan, Legal, SE);
  HCFGBuilder.buildHierarchicalCFG();

  return SharedPlan;
}

void LoopVectorizationPlanner::runInitialVecSpecificTransforms(
    VPlanVector *Plan) {
  // 1. Convert VPLoopEntities into explicit VPInstructions.
  emitVPEntityInstrs(Plan);
  // 2. Emit explicit uniform vector loop IV.
  emitVecSpecifics(Plan);
}

void LoopVectorizationPlanner::emitVPEntityInstrs(VPlanVector *Plan) {
  VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  VPBuilder VPIRBuilder;
  LE->insertVPInstructions(VPIRBuilder);

  VPLAN_DUMP(VPEntityInstructionsDumpControl, Plan);
}

void LoopVectorizationPlanner::emitVecSpecifics(VPlanVector *Plan) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  assert(PreHeader && "Single pre-header is expected!");

  VPBuilder Builder;
  Builder.setInsertPoint(PreHeader);
  VPValue *OrigTC = nullptr;
  bool HasNormalizedInd = hasLoopNormalizedInduction(CandidateLoop);
  if (!HasNormalizedInd) {
    // If loop does not have normalized induction then emit it.
    Type *VectorLoopIVType = Legal->getWidestInductionType();
    if (!VectorLoopIVType) {
      // Ugly workaround for tests forcing VPlan build when we can't actually do
      // that. Shouldn't happen outside stress/forced pipeline.
      VectorLoopIVType = Type::getInt64Ty(*Plan->getLLVMContext());
    }
    auto *VPOne = Plan->getVPConstant(ConstantInt::get(VectorLoopIVType, 1));

    auto *VF =
        Builder.create<VPInductionInitStep>("VF", VPOne, Instruction::Add);

    OrigTC = Builder.create<VPOrigTripCountCalculation>(
        "orig.trip.count", TheLoop, CandidateLoop, VectorLoopIVType);
    auto *TC = Builder.create<VPVectorTripCountCalculation>("vector.trip.count",
                                                            OrigTC);

    emitVectorLoopIV(Plan, TC, VF);
  } else {
    // Having normalized induction we just replace the upper bound.
    VPInstruction *Cond = nullptr;
    std::tie(OrigTC, Cond) = CandidateLoop->getLoopUpperBound();
    auto *VTC = Builder.create<VPVectorTripCountCalculation>(
        "vector.trip.count", OrigTC);
    // TODO: propagate attributes from OrigTC, if possible.
    if (auto *IOrigTC = dyn_cast<VPInstruction>(OrigTC))
      VTC->setDebugLocation(IOrigTC->getDebugLocation());
    Cond->replaceUsesOfWith(OrigTC, VTC);
  }
}

void LoopVectorizationPlanner::emitVectorLoopIV(VPlanVector *Plan,
                                                VPValue *TripCount,
                                                VPValue *VF) {
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();

  auto *PreHeader = CandidateLoop->getLoopPreheader();
  auto *Header = CandidateLoop->getHeader();
  auto *Latch = CandidateLoop->getLoopLatch();
  assert(PreHeader && "Single pre-header is expected!");
  assert(Latch && "Single loop latch is expected!");

  Type *VectorLoopIVType = TripCount->getType();
  auto *VPZero =
      Plan->getVPConstant(ConstantInt::getNullValue(VectorLoopIVType));

  VPBuilder Builder;
  Builder.setInsertPoint(Header, Header->begin());
  auto *IV = Builder.createPhiInstruction(VectorLoopIVType, "vector.loop.iv");
  IV->addIncoming(VPZero, PreHeader);
  Builder.setInsertPoint(Latch);
  auto *IVUpdate = Builder.createAdd(IV, VF, "vector.loop.iv.next");
  IV->addIncoming(IVUpdate, Latch);
  auto *ExitCond = Builder.createCmpInst(
      Latch->getSuccessor(0) == Header ? CmpInst::ICMP_ULT : CmpInst::ICMP_UGE,
      IVUpdate, TripCount, "vector.loop.exitcond");

  VPValue *OrigExitCond = Latch->getCondBit();
  if (Latch->getNumSuccessors() > 1)
    Latch->setCondBit(ExitCond);

  // If original exit condition had single use, remove it - we calculate exit
  // condition differently now.
  // FIXME: "_or_null" here is due to broken stess pipeline that must really
  // stop right after CFG is imported, before *any* transformation is tried on
  // it.
  if (auto *Inst = dyn_cast_or_null<VPInstruction>(OrigExitCond))
    if (Inst->getNumUsers() == 0)
      Latch->eraseInstruction(Inst);
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
        // TODO: Uncomment after search loops are supported without hacks.
        // assert(VPL->getLoopLatch() == VPL->getExitingBlock() &&
        //        "Top level loop is expected to be in canonical form!");
        continue;
      }
      singleExitWhileLoopCanonicalization(VPL);
      mergeLoopExits(VPL);
      // TODO: Verify loops here? It is done again after all initial transforms.
    }
    VPLAN_DUMP(LoopMassagingDumpControl, Plan);
    LLVM_DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
               VPDomTree.print(dbgs()));
  }
}

void LoopVectorizationPlanner::printAndVerifyAfterInitialTransforms(
    VPlan *Plan) {
  // Run verifier after initial transforms in debug build.
  std::unique_ptr<VPlanVerifier> Verifier(new VPlanVerifier(TheLoop, *DL));

  LLVM_DEBUG(Plan->setName("Planner: After initial VPlan transforms\n");
             dbgs() << *Plan);

  if (auto *VPlanVec = dyn_cast<VPlanVector>(Plan)) {

    LLVM_DEBUG(VPlanVec->printVectorVPlanData());

    LLVM_DEBUG(Verifier->verifyLoops(VPlanVec, *VPlanVec->getDT(),
                                     VPlanVec->getVPLoopInfo()));
    (void)VPlanVec;
    (void)Verifier;
  }

  VPLAN_DUMP(InitialTransformsDumpControl, Plan);
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
      if (PrivItem->getIsNonPod())
        LVL.addLoopPrivate(PrivVal, PrivItem->getConstructor(),
                           PrivItem->getDestructor(), PrivItem->getCopyAssign(),
                           true /* IsLast */);
      else
        LVL.addLoopPrivate(PrivVal, true /* IsLast */,
                           PrivItem->getIsConditional());
    }
    PrivateClause &PrivateClause = WRLp->getPriv();
    for (PrivateItem *PrivItem : PrivateClause.items()) {
#if INTEL_CUSTOMIZATION
      auto PrivVal = PrivItem->getOrig<Kind>();
#else
      auto PrivVal = PrivItem->getOrig();
#endif
      if (PrivItem->getIsNonPod())
        LVL.addLoopPrivate(PrivVal, PrivItem->getConstructor(),
                           PrivItem->getDestructor(),
                           nullptr /* no CopyAssign for PrivateItem */);
      else
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
  if (WRLp) {
    LVL.collectPreLoopDescrAliases();
    LVL.collectPostExitLoopDescrAliases();
  }
}

bool LoopVectorizationPlanner::canProcessVPlan(const VPlanVector &Plan) {
  VPLoop *VPLp = *(Plan.getVPLoopInfo()->begin());
  VPBasicBlock *Header = VPLp->getHeader();
  const VPLoopEntityList *LE = Plan.getLoopEntities(VPLp);
  // Check whether all header phis are recognized as entities.
  for (auto &Phi : Header->getVPPhis())
    if (!LE->getInduction(&Phi) && !LE->getReduction(&Phi) &&
        !LE->getPrivate(&Phi)) {
      // Non-entity phi. No other PHIs are expected in loop header since we are
      // working on plain CFG before any transforms.
      LLVM_DEBUG(dbgs() << "LVP: Unrecognized phi found.\n" << Phi << "\n");
      return false;
    }
  if (!canProcessLoopBody(Plan, *VPLp))
    return false;

  // All safety checks passed.
  return true;
}

bool LoopVectorizationPlanner::canProcessLoopBody(const VPlanVector &Plan,
                                                  const VPLoop &Loop) const {
  // Check for live out values that are not inductions/reductions.
  // Their processing is not ready yet.
  if (EnableAllLiveOuts)
    return true;
  const VPLoopEntityList *LE = Plan.getLoopEntities(&Loop);
  for (auto *BB : Loop.blocks())
    for (VPInstruction &Inst : *BB) {
      if (LE->getReduction(&Inst) || LE->getInduction(&Inst)) {
        // Entities code and CG need to be uplifted to handle vector type
        // inductions and reductions.
        if (isa<VectorType>(Inst.getType())) {
          LLVM_DEBUG(dbgs() << "LVP: Vector type reduction/induction currently"
                            << " not supported.\n"
                            << Inst << "\n");
          return false;
        }
      } else if (Loop.isLiveOut(&Inst)) {
        // Liveout that is not an induction or reduction is not supported
        // currently.
        LLVM_DEBUG(dbgs() << "LVP: Unsupported liveout found.\n"
                          << Inst << "\n");
        return false;
      }
    }

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

VPlanVector *LoopVectorizationPlanner::getBestVPlan() {
  unsigned VF = getBestVF();
  assert(VF != 0 && "best vplan is not selected");
  if (VecScenario.getMain().Kind == SingleLoopVecScenario::LKScalar) {
    // Check for consitency. Just in case.
    assert(VF == 1 && "expected VF=1 for scalar VPlan");
    return getVPlanForVF(VF);
  }
  if (isNewCFGMergeEnabled())
    // Get either masked or non-masked main VPlan, depending on
    // selection.
    return VecScenario.getMain().Kind == SingleLoopVecScenario::LKVector
               ? getVPlanForVF(VF)
               : getMaskedVPlanForVF(VF);
  return getVPlanForVF(VF);
}

void LoopVectorizationPlanner::executeBestPlan(VPOCodeGen &LB) {
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
  if ((EnableCFGMerge && EmitPushPopVF) || EnableNewCFGMerge) {
    CallVecDecisions.runForMergedCFG(TLI, TTI);
    Label = "CallVecDecisions analysis for merged CFG";
  } else {
    CallVecDecisions.runForVF(getBestVF(), TLI, TTI);
    Label = "CallVecDecisions analysis for VF=" + std::to_string(getBestVF());
  }
  VPLAN_DUMP(PrintAfterCallVecDecisions, Label, Plan);

  // Compute SVA results for final VPlan which will be used by CG.
  Plan->runSVA();
  VPLAN_DUMP(PrintSVAResults, "ScalVec analysis", Plan);

  VPTransformState State(getBestVF(), 1 /* UF */, LI, DT, ILV->getBuilder(),
                         ILV, CallbackILV, Plan->getVPLoopInfo());
  State.CFG.PrevBB = ILV->getLoopVectorPH();

#if INTEL_CUSTOMIZATION
  // Set ILV transform state
  ILV->setTransformState(&State);
#endif // INTEL_CUSTOMIZATION

  Plan->execute(&State);

  // 3. Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  ILV->finalizeLoop();
}

void LoopVectorizationPlanner::emitPeelRemainderVPLoops(unsigned VF, unsigned UF) {
  if (!EnableCFGMerge && !EnableNewCFGMerge)
    return;
  assert(getBestVF() > 1 && "Unexpected VF");
  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger CFGMerger(*Plan, VF, UF);

  // Run CFGMerger.
  if (!EnableNewCFGMerge)
    CFGMerger.createSimpleVectorRemainderChain(TheLoop);
  else
    CFGMerger.createMergedCFG(VecScenario, MergerVPlans);

  VPLAN_DUMP(CfgMergeDumpControl, Plan);
}

void LoopVectorizationPlanner::createMergerVPlans(VPAnalysesFactory &VPAF) {
  assert(MergerVPlans.empty() && "Non-empty list of VPlans");
  if (EnableNewCFGMerge) {
    assert(getBestVF() > 1 && "Unexpected VF");

    VPlanVector *Plan = getBestVPlan();
    assert(Plan && "No best VPlan found.");

    VPlanCFGMerger::createPlans(*this, VecScenario, MergerVPlans, TheLoop,
                                *Plan, VPAF);
  }
}

// Return true if the compare and branch sequence guarantees the loop trip count
// to match the invariant operand of the compare.
static bool supportedCmpBranch(VPBasicBlock *Header, VPBasicBlock *Latch,
                               VPBasicBlock *LoopExit, VPCmpInst *Cond,
                               VPInstruction *AddI) {
  auto Pred = Cond->getPredicate();
  auto *CondOp0 = Cond->getOperand(0);
  auto *CondOp1 = Cond->getOperand(1);
  auto *LatchSucc0 = Latch->getSuccessor(0);
  auto *LatchSucc1 = Latch->getSuccessor(1);

  if (Pred == CmpInst::ICMP_EQ && LatchSucc0 == LoopExit &&
      LatchSucc1 == Header)
    return true;

  if (Pred == CmpInst::ICMP_NE && LatchSucc0 == Header &&
      LatchSucc1 == LoopExit)
    return true;

  if (ICmpInst::isLT(Pred) && CondOp0 == AddI && LatchSucc0 == Header &&
      LatchSucc1 == LoopExit)
    return true;

  if (ICmpInst::isGE(Pred) && CondOp0 == AddI && LatchSucc0 == LoopExit &&
      LatchSucc1 == Header)
    return true;

  if (ICmpInst::isGT(Pred) && CondOp1 == AddI && LatchSucc0 == Header &&
      LatchSucc1 == LoopExit)
    return true;

  if (ICmpInst::isLE(Pred) && CondOp1 == AddI && LatchSucc0 == LoopExit &&
      LatchSucc1 == Header)
    return true;

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
bool LoopVectorizationPlanner::hasLoopNormalizedInduction(const VPLoop *Loop) {
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
  VPBasicBlock *LoopExit = Loop->getExitBlock();
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
        if (auto *VPLiveIn = dyn_cast<VPLiveInValue>(Init))
          Init = IndInit->getParent()
                     ->getParent()
                     ->getExternals()
                     .getOriginalIncomingValue(VPLiveIn->getMergeId());
      }
      if (isa<VPConstantInt>(Init) &&
          cast<VPConstantInt>(Init)->getValue() == 0)
        continue;
      // The starting value is not induction-init(0).
      return false;
    }
    // All checks succeeded so far. Return true if compare and branch
    // are in supported form.
    return supportedCmpBranch(Header, Latch, LoopExit, Cond, AddI);
  }
  return false;
}
