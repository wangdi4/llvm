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
#include "IntelVPlanPredicator.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVConflictTransformation.h"
#include "VPlanHIR/IntelVPlanHCFGBuilderHIR.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringExtras.h"
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
    InitialTransformsDumpControl("initial-transforms",
                                 "initial VPlan transforms");
static LoopVPlanDumpControl
    BlendWithSafeValueDumpControl("blend-with-safe-value",
                                 "blend integer div/rem with safe value");


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
static int VPlanOrderNumber = 0;

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
  return WRLp && WRLp->getSimdlen() ? WRLp->getSimdlen() : VPlanTargetVF;
}

#if INTEL_CUSTOMIZATION
static unsigned getSafelen(const WRNVecLoopNode *WRLp) {
  return WRLp && WRLp->getSafelen() ? WRLp->getSafelen() : UINT_MAX;
}
#endif // INTEL_CUSTOMIZATION

int LoopVectorizationPlanner::setDefaultVectorFactors() {
  unsigned ForcedVF = getForcedVF(WRLp);
  if (ForcedVF && !isPowerOf2_64(ForcedVF)) {
    LLVM_DEBUG(
        dbgs()
        << "LVP: The forced VF is not power of two, skipping the loop\n");
    VFs.push_back(0);
    return 0;
  }


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
  } else if (VectorlengthMD != nullptr) {
    extractVFsFromMetadata(Safelen);
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

unsigned LoopVectorizationPlanner::buildInitialVPlans(LLVMContext *Context,
                                                      const DataLayout *DL,
                                                      std::string VPlanName,
                                                      ScalarEvolution *SE,
                                                      bool IsLegalToVec) {
  ++VPlanOrderNumber;
  // Concatenate VPlan order number into VPlanName which allows to align
  // VPlans in all different VPlan internal dumps.
  VPlanName += ".#" + std::to_string(VPlanOrderNumber);

  setDefaultVectorFactors();
  if (VFs[0] == 0)
    return 0;

  Externals = std::make_unique<VPExternalValues>(Context, DL);
  UnlinkedVPInsts = std::make_unique<VPUnlinkedInstructions>();

  // TODO: revisit when we build multiple VPlans.
  std::shared_ptr<VPlanVector> Plan =
      buildInitialVPlan(*Externals, *UnlinkedVPInsts, VPlanName, SE);
  if (!Plan) {
    LLVM_DEBUG(dbgs() << "LVP: VPlan was not created.\n");
    return 0;
  }

  if (!IsLegalToVec) {
    LLVM_DEBUG(dbgs() << "LVP: VPlan is not legal to vectorize.\n");
    return 0;
  }

  raw_ostream *OS = nullptr;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  OS = is_contained(VPlanCostModelPrintAnalysisForVF, 1) ? &outs() : nullptr;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  std::tie(ScalarIterationCost, std::ignore) =
      createCostModel(Plan.get(), 1)->getCost(nullptr /* PeelingVariant */, OS);

  // Reset the results of SVA that are set as a side effect of CM invocation.
  // The results are dummy (specific to VF = 1) and may not be automatically
  // reused in general.
  Plan->invalidateAnalyses(VPAnalysisID::SVA);

  VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  if (LE->getImportingError() != VPLoopEntityList::ImportError::None) {
    LLVM_DEBUG(dbgs() << "LVP: Entities import error.\n");
    return 0;
  }
  LE->analyzeImplicitLastPrivates();

  // Check legality of VPlan before proceeding with other transforms/analyses.
  if (!canProcessVPlan(*Plan.get())) {
    LLVM_DEBUG(dbgs() << "LVP: VPlan is not legal to process, bailing out.\n");
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
    LLVM_DEBUG(dbgs() << "There is no suitable VF for compress/expand idiom "
                         "vectorization.\n");
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

  // For VConflict idiom, we should bail-out for some VFs because vconflict
  // intrinsic might not be available.
  unsigned MaxVecRegSize =
      TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector)
          .getFixedSize();

  // TODO: We're losing out on being able to vectorize vconflict idioms at
  // VF=16 for i32 types because the conflict index is always represented
  // as i64. Look into preserving the original VPValue for the index as
  // live-in to VPGeneralMemOptConflict.
  for (auto &VPInst : vpinstructions(Plan.get()))
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
                                              .getFixedSize();

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
    }

  // When we force VF to have a special value, VFs vector has only one value.
  // Therefore, we have to check if we removed the only value that was in VFs.
  if (VFs.empty()) {
    LLVM_DEBUG(dbgs() << "There is no VF found that all VConflict idioms in "
                         "loop can be optimized for.\n");
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
  bool EnableDP = isDynAlignEnabled();
  if (!VPlanEnableGeneralPeeling && !EnableDP)
    return;

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
                                          unsigned VF) const {
  // Do not run VLSA for VF = 1
  VPlanVLSAnalysis *VLSACM = VF > 1 ? VLSA : nullptr;

  switch (getPlannerType()) {
    case PlannerType::Full:
      return VPlanCostModelFull::makeUniquePtr(Plan, VF, TTI, TLI, DL, VLSACM);
    case PlannerType::LightWeight:
      return VPlanCostModelLite::makeUniquePtr(Plan, VF, TTI, TLI, DL, VLSACM);
    case PlannerType::Base:
      return VPlanCostModelBase::makeUniquePtr(Plan, VF, TTI, TLI, DL, VLSACM);
  }
  llvm_unreachable("Uncovered Planner type in the switch-case above.");
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

ArrayRef<unsigned> LoopVectorizationPlanner::getVectorFactors() { return VFs; }

void LoopVectorizationPlanner::selectSimplestVecScenario(unsigned VF,
                                                         unsigned UF) {
  VecScenario.resetPeel();
  VecScenario.resetRemainders();
  VecScenario.addScalarRemainder();
  VecScenario.setVectorMain(VF, UF);
}

Optional<bool> LoopVectorizationPlanner::readVecRemainderEnabled() {
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
  return None;
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

static bool makeGoUnalignedDecision(VPInstructionCost AlignedGain,
                                    VPInstructionCost UnalignedGain,
                                    VPInstructionCost ScalarCost,
                                    uint64_t TripCount,
                                    bool PeelIsDynamic,
                                    bool IsLoopTripCountEstimated) {
  LLVM_DEBUG(dbgs() << "Using cost model to enable peeling. ");
  if (!IsLoopTripCountEstimated && !PeelIsDynamic) {
    // When trip count of main loop is known, rely on cost model completely.
    // Same for when the trip count of peel loops is known, which is not the
    // case for dynamic peeling. Cost model in such cases must assume worst
    // case of VF - 1 iterations. Let further tests below drive the decision
    // on whether or not to peel.
    bool GoUnaligned = UnalignedGain > AlignedGain;
    LLVM_DEBUG(dbgs() << "Trip count is known. "
                      << "GoUnaligned = UnalignedGain > AlignedGain: "
                      << UnalignedGain << " > " << AlignedGain << " = "
                      << GoUnaligned << "\n");
    return GoUnaligned;
  }

  if (TripCount != DefaultTripCount) {
    // Otherwise favor aligned with some tolerance of scalar iterations.
    bool GoUnaligned = UnalignedGain >
      AlignedGain +
      ScalarCost * FavorAlignedToleranceNonDefaultTCEst.getValue() / 100;
    LLVM_DEBUG(dbgs() << "Trip count != default trip count. GoUnaligned = "
                      << "UnalignedGain > "
                      << "(AlignedGain + (ScalarCost * "
                      << "FavorAlignedToleranceNonDefaultTCEst / 100)): "
                      << UnalignedGain << " > (" << AlignedGain << " + ("
                      << ScalarCost << " * "
                      << FavorAlignedToleranceNonDefaultTCEst << " / 100))"
                      << " = " << GoUnaligned << "\n");
    return GoUnaligned;
  }
  if (UnalignedGain > 0 && AlignedGain > 0) {
    bool GoUnaligned =
      UnalignedGain >
      FavorAlignedMultiplierDefaultTCEst.getValue() * AlignedGain;
    LLVM_DEBUG(dbgs() << "Aligned and unaligned gains are positive. "
                      << "GoUnaligned = "
                      << "UnalignedGain > FavorAlignedMultiplierDefaultTCEst "
                      << "* AlignedGain: "
                      << UnalignedGain << " > "
                      << FavorAlignedMultiplierDefaultTCEst << " * "
                      << AlignedGain << " = " << GoUnaligned << "\n");
    return GoUnaligned;
  }

  // We're here when one (or both) is <= 0. Perhaps, going unaligned is correct.
  return false;
}

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
  unsigned BestUF = getLoopUnrollFactor();
  bool IsTripCountEstimated = OuterMostVPLoop->getTripCountInfo().IsEstimated;
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
    RegDDRef *PeelArrayRef = nullptr;

    switch (VPlanIdioms::isSearchLoop(ScalarPlan, true, PeelArrayRef)) {
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
        LLVM_DEBUG(dbgs() << "Selecting VPlan with VF=" <<
                   SearchLoopPreferredVF << '\n');
        return std::make_pair(VecScenario.getMainVF(), getBestVPlan());
      }

      if (SearchLoopPreferredVF * BestUF > TripCount) {
        LLVM_DEBUG(dbgs() << "Bailing out to scalar VPlan because "
                   << "SearchLoopPreferredVF(" << SearchLoopPreferredVF
                   << ") * BestUF(" << BestUF << ") > TripCount("
                   << TripCount << ")\n");
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

#if INTEL_CUSTOMIZATION
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
#endif // INTEL_CUSTOMIZATION

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

  DenseMap<unsigned, VPCostSummary> VFCosts;

  //
  // The main loop where we choose VF.
  // Please note that best peeling variant is expected to be selected up to
  // this moment.  Now we check whether the best peeling is profitable VS no
  // peeling.
  //
  // The rules for picking best VF follows:
  // 1. ForcedVF / SearchLoopPreferredVF wins always even the corresponding Plan
  //    is of Unknown/Invalid cost.
  // 2. IsVectorAlways = true gives preference to VF > 1 Plans.
  // 3. The least valid cost wins.
  // 4. Unknown cost wins over Invalid cost.
  // 5. The cases of tie:
  //      *) all costs are valid and equal.
  //      *) all costs are Unknown.
  //      *) all costs are Invalid.
  //    In case of tie VF = 1 wins unless IsVectorAlways is true.
  //    In case of tie and IsVectorAlways is true the first avaiable VF
  //    is selected.
  for (unsigned VF : getVectorFactors()) {
    assert(hasVPlanForVF(VF) && "expected non-null VPlan");
    if (TripCount < VF * BestUF)
      continue; // FIXME: Consider masked low trip later.

    VPlanVector *Plan = getVPlanForVF(VF);
    assert(Plan && "Unexpected null VPlan");

    // Calculate cost for one iteration of the main loop.
    auto MainLoopCM = createCostModel(Plan, VF);
    VPlanPeelingVariant *PeelingVariant = Plan->getPreferredPeeling(VF);

    // Peeling is not supported for non-normalized loops.
    VPLoop *L = Plan->getMainLoop(true);
    if (!L->hasNormalizedInduction())
      PeelingVariant = &VPlanStaticPeeling::NoPeelLoop;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    OS = is_contained(VPlanCostModelPrintAnalysisForVF, VF) ?
      &outs() : nullptr;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

    VPInstructionCost MainLoopIterationCost, MainLoopOverhead;

    std::tie(MainLoopIterationCost, MainLoopOverhead) =
        MainLoopCM->getCost(PeelingVariant, OS);

    if (!MainLoopIterationCost.isValid() || !MainLoopOverhead.isValid()) {
      LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << " is unknown. Skip it.\n");
      if (VF == ForcedVF || VF == SearchLoopPreferredVF) {
        // If the VF is forced and loop cost for it is unknown/invalid select
        // the simplest configuration: non-masked main loop + scalar remainder.
        selectSimplestVecScenario(VF, BestUF);
      }
      else
        LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << " is " <<
                   MainLoopIterationCost << ". Skip it.\n");
      continue;
    }
    LLVM_DEBUG(dbgs() << "Selected peeling: ";
      if (PeelingVariant) {
        if (isa<VPlanDynamicPeeling>(PeelingVariant))
          dbgs() << "Dynamic\n";
        else
          dbgs() << "Static("
                 << cast<VPlanStaticPeeling>(PeelingVariant)->peelCount()
                 << ")\n";
      } else {
        dbgs() << "None\n";
      }
    );

    // Calculate the total cost of peel loop if there is one.
    VPlanPeelEvaluator PeelEvaluator(*this, ScalarIterationCost, TLI, TTI, DL,
                                     VLSA, VF, PeelingVariant);
    // Calculate the total cost of remainder loop if there is one.
    bool PeelIsDynamic = PeelingVariant ?
        isa<VPlanDynamicPeeling>(PeelingVariant) : false;
    VPlanRemainderEvaluator RemainderEvaluator(
        *this, ScalarIterationCost, TLI, TTI, DL, VLSA, OrigTripCount,
        PeelEvaluator.getTripCount(), PeelIsDynamic, VF, BestUF);

    // Calculate main loop's trip count. Currently, the unroll factor is set to
    // 1 because VPlan's loop unroller is called after selecting the best VF.
    const decltype(TripCount) MainLoopTripCount =
        (TripCount - PeelEvaluator.getTripCount()) / (VF * BestUF);

    // The total vector cost is calculated by adding the total cost of peel,
    // main and remainder loops.
    VPInstructionCost VectorCost =
      PeelEvaluator.getLoopCost() +
      MainLoopIterationCost * MainLoopTripCount + MainLoopOverhead +
      RemainderEvaluator.getLoopCost();

    // Calculate cost of one iteration of the main loop without preferred
    // alignment.
    // This getCost() call leaves no traces in CM dumps enabled by
    // VPlanCostModelPrintAnalysisForVF for now. May want to reconsider in
    // future.
    VPInstructionCost MainLoopIterationCostWithoutPeel,
        MainLoopOverheadWithoutPeel;
    std::tie(MainLoopIterationCostWithoutPeel, MainLoopOverheadWithoutPeel) =
        MainLoopCM->getCost();
    if (MainLoopIterationCostWithoutPeel.isUnknown() ||
        MainLoopOverheadWithoutPeel.isUnknown()) {
      LLVM_DEBUG(dbgs() << "Cost for VF = " << VF <<
                 " without peel is unknown. Skip it.\n");
      continue;
    }

    // Calculate the total cost of remainder loop having no peeling, if there
    // is one.
    VPlanRemainderEvaluator RemainderEvaluatorWithoutPeel(
        *this, ScalarIterationCost, TLI, TTI, DL, VLSA, OrigTripCount,
        0 /*Peel trip count */, false /*no dynamic peeling*/, VF, BestUF);
    const decltype(TripCount) MainLoopTripCountWithoutPeel =
        TripCount / (VF * BestUF);
    VPInstructionCost VectorCostWithoutPeel =
        MainLoopIterationCostWithoutPeel * MainLoopTripCountWithoutPeel +
        MainLoopOverheadWithoutPeel +
        RemainderEvaluatorWithoutPeel.getLoopCost();

    if (0 < VecThreshold && VecThreshold < 100) {
      LLVM_DEBUG(dbgs() << "Applying threshold " << VecThreshold << " for VF "
                        << VF << ". Original cost = " << VectorCost << '\n');
      VectorCost = (VectorCost * VecThreshold.getValue()) / 100;
      VectorCostWithoutPeel =
        (VectorCostWithoutPeel * VecThreshold.getValue()) / 100;
    }

    VPInstructionCost GainWithPeel    = ScalarCost - VectorCost;
    VPInstructionCost GainWithoutPeel = ScalarCost - VectorCostWithoutPeel;

    bool GoUnaligned = PeelEvaluator.getPeelLoopKind() ==
                       VPlanPeelEvaluator::PeelLoopKind::None;
    if (!GoUnaligned)
      GoUnaligned = makeGoUnalignedDecision(
          GainWithPeel, GainWithoutPeel, ScalarCost, TripCount, PeelIsDynamic,
          IsTripCountEstimated);

    VPInstructionCost VectorGain = GoUnaligned ? GainWithoutPeel : GainWithPeel;
    // Calculate speedup relatively to single scalar iteration.
    VPInstructionCost Speedup =
        ScalarIterationCost / (ScalarIterationCost - VectorGain / TripCount);
    VPInstructionCost VectorIterationCost =
        GoUnaligned ? MainLoopIterationCostWithoutPeel : MainLoopIterationCost;
    VectorIterationCost = VectorIterationCost / (VF * BestUF);
    VPInstructionCost VectorInitFini =
        GoUnaligned ? MainLoopOverheadWithoutPeel : MainLoopOverhead;
    VFCosts.insert({VF, VPCostSummary(ScalarIterationCost, VectorIterationCost,
                                      Speedup, VectorInitFini)});

    const char CmpChar = ScalarCost < VectorCost    ? '<'
                         : ScalarCost == VectorCost ? '='
                                                    : '>';
    (void)CmpChar;
    LLVM_DEBUG(
        dbgs() << "Scalar Cost = " << TripCount << " x " << ScalarIterationCost
               << " = " << ScalarCost << ' ' << CmpChar
               << " VectorCost = " << PeelEvaluator.getLoopCost() << " + "
               << MainLoopTripCount << " x " << MainLoopIterationCost << " + "
               << MainLoopOverhead << " + " << RemainderEvaluator.getLoopCost()
               << " = " << VectorCost << '\n'
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
               << MainLoopOverheadWithoutPeel << " = " << VectorCostWithoutPeel
               << "\n"
               << "Main loop vector cost without peel = "
               << MainLoopIterationCostWithoutPeel *
                          MainLoopTripCountWithoutPeel +
                      MainLoopOverheadWithoutPeel
               << "\n"
               << "Remainder loop cost without peel = "
               << RemainderEvaluatorWithoutPeel.getLoopCost() << " ("
               << RemainderEvaluatorWithoutPeel.getRemainderLoopKindStr() << ")"
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

    if (GoUnaligned) {
      PeelEvaluator.disable();
      // Total cost is cost without peeling.
      VectorCost = VectorCostWithoutPeel;
      LLVM_DEBUG(dbgs() << "Peeling will not be performed.\n");
    } else {
      LLVM_DEBUG(dbgs() << "Peeling will be performed.\n");
    }

    // Current VF is invalid to vectorize with so skip it.
    if (VectorCost.isInvalid())
      continue;

    // The checks below implement the rules for selecting the best VF.
    if ((VectorCost.isUnknown() && BestCost.isInvalid()) ||
        (VectorCost.isValid() && !BestCost.isValid())    ||
        (VectorCost.isValid() && BestCost.isValid() &&
         VectorCost < BestCost)                          ||
        VF == ForcedVF || VF == SearchLoopPreferredVF) {
      BestCost = VectorCost;
      updateVecScenario(PeelEvaluator,
                        GoUnaligned ? RemainderEvaluatorWithoutPeel
                                    : RemainderEvaluator,
                        VF, BestUF);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (PrintVecScenario) {
        dbgs() << "Updated scenario for VF: " << VF << "\n";
        VecScenario.dump();
      }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    }
  }
#if INTEL_CUSTOMIZATION
  // Corner case: all available VPlans have Invalid cost.
  // With 'vector always' we have to vectorize with some VF, so select first
  // available VF.
  if (VecScenario.getMainVF() == 1 && IsVectorAlways)
    selectSimplestVecScenario(VFs[0], 1);

  // Workaround for using DefaultTripCount: in some cases we can have situation
  // when static peeling and current value of DefaultTripCount can lead to
  // incorrect decision "the remainder is not needed". E.g. we have
  // DefaultTripCount = 303, with static peel count 3 we have tc = 300, and for
  // VF=4 or VF=2 we have no iterations for remainder.
  // The correct fix should be made in remainder evaluator, but that fix
  // requires the cost model tuning as with the current cost model we have
  // regressions.
  if (IsTripCountEstimated && !VecScenario.hasRemainder())
    VecScenario.addScalarRemainder();

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
      if (VecScenario.hasMaskedPeel() && !L->hasNormalizedInduction()) {
        // replace by scalar loop if there is no normalized induction
        VecScenario.setScalarPeel();
      }
      auto *StaticPeelingVariant =
          dyn_cast_or_null<VPlanStaticPeeling>(MPlan->getPreferredPeeling(VF));
      if (!MPlan->getPreferredPeeling(VF) ||
          (StaticPeelingVariant && !StaticPeelingVariant->peelCount()))
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

  OptReportStatsTracker &OptRptStats =
      getVPlanForVF(getBestVF())->getOptRptStatsForLoop(OuterMostVPLoop);
  auto Record = VFCosts.find(getBestVF());
  if (Record != VFCosts.end()) {
    VPCostSummary &CostSummary = Record->second;
    if (CostSummary.ScalarIterationCost.isValid()) {
      // Add remark: scalar cost
      OptRptStats.CostModelRemarks.emplace_back(
          15476,
          std::to_string(CostSummary.ScalarIterationCost.getFloatValue()));
    }
    if (CostSummary.VectorIterationCost.isValid()) {
      // Add remark: vector cost
      OptRptStats.CostModelRemarks.emplace_back(
          15477,
          std::to_string(CostSummary.VectorIterationCost.getFloatValue()));
    }
    if (CostSummary.Speedup.isValid()) {
      // Add remark: estimated potential speedup
      OptRptStats.CostModelRemarks.emplace_back(
          15478, std::to_string(CostSummary.Speedup.getFloatValue()));
    }
    if (CostSummary.LoopOverhead.isValid()) {
      // Add remark: vectorization support: normalized vectorization overhead
      OptRptStats.CostModelRemarks.emplace_back(
          15309, std::to_string(CostSummary.LoopOverhead.getFloatValue()));
    }
  }

  return std::make_pair(getBestVF(), getBestVPlan());
}

void LoopVectorizationPlanner::updateVecScenario(
    VPlanPeelEvaluator const &PE, VPlanRemainderEvaluator const &RE,
    unsigned VF, unsigned UF) {
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
    VPLAN_DUMP(PredicatorDumpControl, VPlan);

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

    AZB.collectAllZeroBypassNonLoopRegions(AllZeroBypassRegions,
                                           RegionsCollected,
                                           createCostModel(Plan, VF).get(), VF);
  }
  AZB.insertAllZeroBypasses(AllZeroBypassRegions);
  VPLAN_DUMP(AllZeroBypassDumpControl, Plan);
}

bool LoopVectorizationPlanner::unroll(VPlanVector &Plan) {
  unsigned UF = getLoopUnrollFactor();
  if (UF > 1) {
    VPlanLoopUnroller Unroller(Plan, UF);
    Unroller.run();
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

  const Function* F = TheLoop->getHeader()->getParent();
  Plan->setPrintingEnabled(llvm::isFunctionInPrintList(F->getName()));

  if (EnableSOAAnalysis)
    // Enable SOA-analysis.
    Plan->enableSOAAnalysis();

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(TheLoop, LI, *DL, WRLp, Plan, Legal, SE, BFI);
  if (!HCFGBuilder.buildHierarchicalCFG())
    return nullptr;

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
  OptReportStatsTracker &OptRptStats = Plan->getOptRptStatsForLoop(MainLoop);
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
  VPBuilder VPIRBuilder;
  LE->insertVPInstructions(VPIRBuilder);

  if (LE->hasReduction()) {
    if (WRLp && WRLp->isOmpSIMDLoop())
      // Add remark Loop has SIMD reduction
      OptRptStats.ReductionInstRemarks.emplace_back(25588, "");
    else
      // Add remark Loop has reduction
      OptRptStats.ReductionInstRemarks.emplace_back(25587, "");
  }

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

  LLVM_DEBUG(dbgs() << *Plan);

  if (auto *VPlanVec = dyn_cast<VPlanVector>(Plan)) {

    LLVM_DEBUG(VPlanVec->printVectorVPlanData());

    LLVM_DEBUG(Verifier->verifyLoops(VPlanVec, *VPlanVec->getDT(),
                                     VPlanVec->getVPLoopInfo()));
    (void)VPlanVec;
    (void)Verifier;
  }

  VPLAN_DUMP(InitialTransformsDumpControl, Plan);
}

bool LoopVectorizationPlanner::canProcessVPlan(const VPlanVector &Plan) {
  VPLoop *VPLp = *(Plan.getVPLoopInfo()->begin());
  VPBasicBlock *Header = VPLp->getHeader();
  const VPLoopEntityList *LE = Plan.getLoopEntities(VPLp);
  if (!LE)
    return false;
  // Check whether all reductions are supported
  for (auto Red : LE->vpreductions()) {
    // Bailouts for user-defined reductions.
    if (Red->getRecurrenceKind() == RecurKind::Udr) {
      // Check if UDR is registerized. This can happen due to hoisting/invariant
      // code motion done by pre-vectorizer passes. TODO: This should be changed
      // to assert in future when Paropt is fixed to prohibit registerization of
      // UDRs.
      if (!Red->getRecurrenceStartValue() ||
          LE->getMemoryDescriptor(Red) == nullptr || !Red->getIsMemOnly()) {
        LLVM_DEBUG(dbgs() << "LVP: Registerized UDR found.\n");
        return false;
      }
    }
  }

  // Check whether all header phis are recognized as entities.
  for (auto &Phi : Header->getVPPhis())
    if (!LE->getInduction(&Phi) && !LE->getReduction(&Phi) &&
        !LE->getPrivate(&Phi) && !LE->getCompressExpandIdiom(&Phi)) {
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

bool LoopVectorizationPlanner::canLowerVPlan(const VPlanVector &Plan,
                                             unsigned VF) {
  for (auto &VPI : vpinstructions(&Plan)) {
    // Check if an array private is marked as SOA profitable and if we
    // lack the needed codegen support.
    if (auto *AllocaPriv = dyn_cast<VPAllocatePrivate>(&VPI))
      if (AllocaPriv->isSOASafe() && AllocaPriv->isSOAProfitable() &&
          !isSOACodegenSupported() &&
          AllocaPriv->getAllocatedType()->isArrayTy())
        return false;
  }

  // All checks passed.
  return true;
}

bool LoopVectorizationPlanner::canProcessLoopBody(const VPlanVector &Plan,
                                                  const VPLoop &Loop) const {
  // Check for live out values that are not inductions/reductions.
  // Their processing is not ready yet.
  if (EnableAllLiveOuts)
    return true;
  const VPLoopEntityList *LE = Plan.getLoopEntities(&Loop);
  if (!LE)
    return false;
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
      } else if (Loop.isLiveOut(&Inst) && !LE->getPrivate(&Inst)) {
        // Some liveouts are left unrecognized due to unvectorizable use-def
        // chains.
        LLVM_DEBUG(dbgs() << "LVP: Unrecognized liveout found.");
        return false;
      }
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
  CallVecDecisions.runForMergedCFG(TLI, TTI);
  Label = "CallVecDecisions analysis for merged CFG";
  VPLAN_DUMP(PrintAfterCallVecDecisions, Label, Plan);

  // Compute SVA results for final VPlan which will be used by CG.
  Plan->runSVA(getBestVF());
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
  assert(getBestVF() > 1 && "Unexpected VF");
  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger CFGMerger(*Plan, VF, UF);

  // Run CFGMerger.
  CFGMerger.createMergedCFG(VecScenario, MergerVPlans, TheLoop);
}

void LoopVectorizationPlanner::createMergerVPlans(VPAnalysesFactoryBase &VPAF) {
  assert(MergerVPlans.empty() && "Non-empty list of VPlans");
  assert(getBestVF() > 1 && "Unexpected VF");

  VPlanVector *Plan = getBestVPlan();
  assert(Plan && "No best VPlan found.");

  VPlanCFGMerger::createPlans(*this, VecScenario, MergerVPlans, TheLoop,
                              *Plan, VPAF);
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

  SmallPtrSet<VPlan *, 2> Visited;

  auto ProcessVPlan = [&Visited](VPlan &P) -> void {
    if (!Visited.insert(&P).second)
      return;

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

  for (auto &Pair : VPlans) {
    if (Pair.first == 1) // VF
      continue;

    ProcessVPlan(*Pair.second.MainPlan);
    if (Pair.second.MaskedModeLoop)
      ProcessVPlan(*Pair.second.MaskedModeLoop);
  }
}
