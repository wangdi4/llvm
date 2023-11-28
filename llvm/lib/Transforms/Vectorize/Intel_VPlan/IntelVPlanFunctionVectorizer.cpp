//===-- IntelVPlanFunctionVectorizer.cpp ----------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// Implements VPlanFunctionVectorizerPass/VPlanFunctionVectorizerLegacyPass.
//
// TODO: Assert for loop-simplified form (for inner loops).
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Vectorize/IntelVPlanFunctionVectorizer.h"

#include "IntelVPlanAllZeroBypass.h"
#include "IntelVPlanCFGBuilder.h"
#include "IntelVPlanLCSSA.h"
#include "IntelVPlanLoopCFU.h"
#include "IntelVPlanLoopExitCanonicalization.h"
#include "IntelVPlanPredicator.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/OptBisect.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Vectorize.h"

#define DEBUG_TYPE "vplan-function-vectorizer"

static FuncVecVPlanDumpControl
    CFGImportDumpControl("cfg-import",
                         "CFG import for VPlan Function vectorization");

static FuncVecVPlanDumpControl LoopExitsCanonicalizationDumpControl(
    "loop-exit-canon",
    "Lopo Exits Canonicalizaiton for VPlan Function vectorization");

static FuncVecVPlanDumpControl
DADumpControl("da", "DA analysis for VPlan Function vectorization");

static FuncVecVPlanDumpControl
    LCSSADumpControl("lcssa", "LCSSA for VPlan Function vectorization");

static FuncVecVPlanDumpControl
    LoopCFUDumpControl("loop-cfu", "LoopCFU for VPlan Function vectorization");

static FuncVecVPlanDumpControl
    PredicatorDumpControl("predicator",
                          "Predication for VPlan Function vectorization");

// FIXME: temporarily add switch to control all-zero bypass insertion
// due to failing vplan_preserveSSA_after_while_loop_canonicalization.ll.
// https://git-amr-2.devtools.intel.com/gerrit/#/c/248816/ will address
// this because it is the same issue.
static cl::opt<bool> EnableFuncVecAllZeroBypassNonLoops(
    "enable-vplan-func-vec-all-zero-bypass-non-loops",
    cl::desc("Enable all-zero bypass for VPlan Function vectorization for "
             "non-loops "),
    cl::init(false), cl::Hidden);

static cl::opt<bool> EnableFuncVecAllZeroBypassLoops(
    "enable-vplan-func-vec-all-zero-bypass-loops",
    cl::desc("Enable all-zero bypass for VPlan Function vectorization for "
             "loops "),
    cl::init(false), cl::Hidden);

static FuncVecVPlanDumpControl
    AllZeroBypassDumpControl("all-zero-bypass",
                             "all-zero bypass for VPlan Function vectorization",
                             true);

using namespace llvm;
using namespace llvm::vpo;
namespace {
class VPlanFunctionVectorizer {
  Function &F;
  AssumptionCache &AC;
  const DominatorTree &DT;

public:
  VPlanFunctionVectorizer(Function &F, AssumptionCache &AC,
                          const DominatorTree &DT)
      : F(F), AC(AC), DT(DT) {}

  bool run() {
    auto Externals = std::make_unique<VPExternalValues>(F.getParent());
    auto UnlinkedVPInsts = std::make_unique<VPUnlinkedInstructions>();
    auto Plan = std::make_unique<VPlanNonMasked>(*Externals, *UnlinkedVPInsts);

    VPlanFunctionCFGBuilder Builder(Plan.get(), F, AC, DT);
    Builder.buildCFG();
    Plan->setName(F.getName());

    VPLAN_DUMP(CFGImportDumpControl, *Plan);

    Plan->computeDT();
    Plan->computePDT();
    Plan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
    auto *VPLInfo = Plan->getVPLoopInfo();
    VPLInfo->analyze(*Plan->getDT());

    for (auto *TopLoop : *VPLInfo)
      for (auto *VPL : post_order(TopLoop)) {
        singleExitWhileLoopCanonicalization(VPL);
        mergeLoopExits(VPL, false /*NeedsOuterLpEarlyExitHandling*/);
      }

    VPLAN_DUMP(LoopExitsCanonicalizationDumpControl, *Plan);

    auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
    Plan->setVPlanDA(std::move(VPDA));
    Plan->getVPlanDA()->compute(Plan.get(), nullptr /*RegionLoop*/, VPLInfo,
                                nullptr /*VPVT*/, *Plan->getDT(),
                                *Plan->getPDT(), false /*Not in LCSSA form*/);

    VPLAN_DUMP(DADumpControl, *Plan);

    formLCSSA(*Plan);
    VPLAN_DUMP(LCSSADumpControl, *Plan);

    VPlanLoopCFU LoopCFU(*Plan);
    LoopCFU.run();
    VPLAN_DUMP(LoopCFUDumpControl, *Plan);

    VPlanPredicator Predicator(*Plan.get());
    Predicator.predicate();
    VPLAN_DUMP(PredicatorDumpControl, *Plan);

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
    if (EnableFuncVecAllZeroBypassLoops)
      AZB.collectAllZeroBypassLoopRegions(AllZeroBypassRegions,
                                          RegionsCollected);
    if (EnableFuncVecAllZeroBypassNonLoops)
      AZB.collectAllZeroBypassNonLoopRegions(AllZeroBypassRegions,
                                             RegionsCollected, false);
    AZB.insertAllZeroBypasses(AllZeroBypassRegions);
    VPLAN_DUMP(AllZeroBypassDumpControl, *Plan);

    return false;
  }
};
} // namespace
INITIALIZE_PASS_BEGIN(VPlanFunctionVectorizerLegacyPass, "vplan-func-vec",
                      "VPlan Function Vectorization Driver", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_END(VPlanFunctionVectorizerLegacyPass, "vplan-func-vec",
                    "VPlan Function Vectorization Driver", false, false)

Pass *llvm::createVPlanFunctionVectorizerPass() {
  return new VPlanFunctionVectorizerLegacyPass();
}

char VPlanFunctionVectorizerLegacyPass::ID = 0;

VPlanFunctionVectorizerLegacyPass::VPlanFunctionVectorizerLegacyPass()
    : FunctionPass(ID) {
  initializeVPlanFunctionVectorizerLegacyPassPass(
      *PassRegistry::getPassRegistry());
}

void VPlanFunctionVectorizerLegacyPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addPreserved<GlobalsAAWrapperPass>();
}

static std::string getDescription(const Function &F) {
  return "function (" + F.getName().str() + ")";
}

bool VPlanFunctionVectorizerLegacyPass::skipFunction(const Function &F) const {
  OptPassGate &Gate = F.getContext().getOptPassGate();
  if (Gate.isEnabled() &&
      !Gate.shouldRunPass(this->getPassName(), getDescription(F)))
    return true;

  if (F.hasOptNone() &&
      VPOAnalysisUtils::skipFunctionForOpenmp(const_cast<Function &>(F))) {
    LLVM_DEBUG(dbgs() << "Skipping pass '" << getPassName() << "' on function "
                      << F.getName() << "\n");
    return true;
  }
  return false;
}

bool VPlanFunctionVectorizerLegacyPass::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  auto &AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(Fn);
  auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  VPlanFunctionVectorizer FunctionVectorizer(Fn, AC, DT);
  return FunctionVectorizer.run();
}

PreservedAnalyses
VPlanFunctionVectorizerPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  const auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

  VPlanFunctionVectorizer FunctionVectorizer(F, AC, DT);
  if (!FunctionVectorizer.run())
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses::none();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}
