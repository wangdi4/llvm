//===-- VPlanDriver.cpp -----------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#include "LoopVectorizationCodeGen.h"
#include "LoopVectorizationPlanner.h"
#include "VPlanPredicator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Vectorize.h"

#define DEBUG_TYPE "VPlanDriver"

using namespace llvm;
using namespace llvm::vpo; //Needed for WRegionInfo


//static cl::opt<bool>
//    DisableVPODirectiveCleanup("disable-vpo-directive-cleanup", cl::init(false),
//                               cl::Hidden,
//                               cl::desc("Disable VPO directive cleanup"));

static cl::opt<bool> DisableCodeGen(
    "disable-vplan-codegen", cl::init(false), cl::Hidden,
    cl::desc(
        "Disable VPO codegen, when true, the pass stops at VPlan creation"));

static cl::opt<unsigned>
    VPlanDefaultVF("vplan-default-vf", cl::init(4),
                   cl::desc("Default VPlan vectorization factor"));

static cl::opt<bool> VPlanStressTest(
    "vplan-build-stress-test", cl::init(false),
    cl::desc("Construct VPlan for every loop (stress testing)"));

static cl::opt<bool> VPlanStressOnlyInnermost(
    "vplan-build-stress-only-innermost", cl::init(false),
    cl::desc("When stress testing is enable, construct VPlan only for "
             "innermost loops"));

static cl::opt<bool>
    VPlanForceBuild("vplan-force-build", cl::init(false),
                    cl::desc("Construct VPlan even if loop is not supported "
                             "(only for development)"));
static cl::opt<bool>
    DisableVPlanPredicator("disable-vplan-predicator", cl::init(false),
                           cl::Hidden, cl::desc("Disable VPlan predicator."));

static cl::opt<unsigned> VPlanVectCand(
    "vplan-build-vect-candidates", cl::init(0),
    cl::desc("Construct VPlan for vectorization candidates (CG stress testing)"));

STATISTIC(CandLoopsVectorized, "Number of candidate loops vectorized");


namespace {
// TODO: Not sure where to place this functions. There are utils but they are
// BB-based not VPBB-based
/// Check whether the edge (\p SrcBB, \p DestBB) is a backedge according to LI.
/// I.e., check if it exists a loop that contains SrcBB and where DestBB is the
/// loop header.
static bool isExpectedBackedge(LoopInfo *LI, BasicBlock *SrcBB,
                               BasicBlock *DestBB) {
  for (Loop *Loop = LI->getLoopFor(SrcBB); Loop; Loop = Loop->getParentLoop()) {
    if (Loop->getHeader() == DestBB)
      return true;
  }

  return false;
}

/// Check if the CFG of \p MF is irreducible.
static bool isIrreducibleCFG(Loop *Lp, LoopInfo *LI) {
  LoopBlocksDFS DFS(Lp);
  DFS.perform(LI);
  SmallPtrSet<const BasicBlock *, 32> VisitedBB;

  for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO())) {
    VisitedBB.insert(BB);
    for (BasicBlock *SuccBB : BB->getTerminator()->successors()) {
      // SuccBB hasn't been visited yet
      if (!VisitedBB.count(SuccBB))
        continue;
      // We already visited SuccBB, thus BB->SuccBB must be a backedge.
      // Check that the head matches what we have in the loop information.
      // Otherwise, we have an irreducible graph.
      if (!isExpectedBackedge(LI, BB, SuccBB))
        return true;
    }
  }

  return false;
}

//class VPODirectiveCleanup : public FunctionPass {
//public:
//  static char ID; // Pass identification, replacement for typeid
//
//  VPODirectiveCleanup() : FunctionPass(ID) {
//    initializeVPODirectiveCleanupPass(*PassRegistry::getPassRegistry());
//  }
//  bool runOnFunction(Function &F) override;
//  //  void getAnalysisUsage(AnalysisUsage &AU) const override;
//};

class VPlanDriverBase : public FunctionPass {

protected:
  // TODO: We are not using LoopInfo for HIR
  LoopInfo *LI;
  ScalarEvolution *SE;
  WRegionInfo *WR;

  /// Handle to Target Information 
  TargetTransformInfo *TTI;
  DominatorTree *DT;
  TargetLibraryInfo *TLI;

public:
  VPlanDriverBase(char &ID) : FunctionPass(ID){};
  bool runOnFunction(Function &F) override;

  virtual void processLoop(Loop *LoopNode, Function &F,
                           WRNVecLoopNode *WRLoop = 0) = 0;
  /// Get a handle to the engine that explores and evaluates the 
  /// vectorization opportunities in a Region.
  //virtual VPOScenarioEvaluationBase &getScenariosEngine(AVRWrn *AWrn, 
  //                                                      Function &F) = 0;

  /// Call the destructor of the ScenariosEngine for this region. 
  //virtual void resetScenariosEngineForRegion() = 0;
};

class VPlanDriver : public VPlanDriverBase {

private:

  void processLoop(Loop *Lp, Function &F, WRNVecLoopNode *WRLoop = 0);

public:
  static char ID; // Pass identification, replacement for typeid

  VPlanDriver() : VPlanDriverBase(ID) {
    initializeVPlanDriverPass(*PassRegistry::getPassRegistry());
    //ScenariosEngine = nullptr;
  }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  //VPOScenarioEvaluationBase &getScenariosEngine(AVRWrn *AvrWrn,
  //                                              Function &F) override {
  //  ScenariosEngine =
  //      new VPOScenarioEvaluation(AvrWrn, *TTI, *TLI, F.getContext(), *DefUse);
  //  return *ScenariosEngine;
  //}

  //void resetScenariosEngineForRegion() override {
  //  if (ScenariosEngine) {
  //    delete ScenariosEngine;
  //    ScenariosEngine = nullptr;
  //  }
  //}

};

//class VPODriverHIR : public VPODriverBase {
//public:
//  static char ID; // Pass identification, replacement for typeid
//
//  VPODriverHIR() : VPODriverBase(ID) {
//    initializeVPODriverHIRPass(*PassRegistry::getPassRegistry());
//    ScenariosEngine = nullptr;
//  }
//  bool runOnFunction(Function &F) override {
//    AV = &getAnalysis<AVRGenerateHIR>();
//    DDA = &getAnalysis<HIRDDAnalysis>();
//    VLS = &getAnalysis<HIRVectVLSAnalysis>();
//    DefUse = &getAnalysis<AvrDefUseHIR>();
//    return VPODriverBase::runOnFunction(F);
//  }
//  void getAnalysisUsage(AnalysisUsage &AU) const override;
//
//  /// \brief Overrides FunctionPass's printer pass to return one which prints
//  /// HIR instead of LLVM IR.
//  FunctionPass *createPrinterPass(raw_ostream &OS,
//                                  const std::string &Banner) const override {
//    return createHIRPrinterPass(OS, Banner);
//  }
//
//  VPOScenarioEvaluationBase &getScenariosEngine(AVRWrn *AvrWrn,
//                                                Function &F) override {
//    ScenariosEngine = new VPOScenarioEvaluationHIR(AvrWrn, DDA, VLS, *DefUse,
//                                                   *TTI, *TLI, F.getContext());
//    return *ScenariosEngine;
//  }
//
//  void resetScenariosEngineForRegion() override {
//    if (ScenariosEngine) {
//      delete ScenariosEngine;
//      ScenariosEngine = nullptr;
//    }
//  }
//
//private:
//  HIRDDAnalysis *DDA;
//  HIRVectVLSAnalysis *VLS;
//  AvrDefUseHIR *DefUse;
//  VPOScenarioEvaluationHIR *ScenariosEngine;
//};

} // anonymous namespace

//INITIALIZE_PASS_BEGIN(VPODirectiveCleanup, "VPODirectiveCleanup",
//                      "VPO Directive Cleanup", false, false)
//INITIALIZE_PASS_END(VPODirectiveCleanup, "VPODirectiveCleanup",
//                    "VPO Directive Cleanup", false, false)
//
//char VPODirectiveCleanup::ID = 0;

//INITIALIZE_PASS_BEGIN(VPODriverHIR, "VPODriverHIR",
//                      "VPO Vectorization Driver HIR", false, false)
//INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
//INITIALIZE_PASS_DEPENDENCY(HIRParser)
//INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
//INITIALIZE_PASS_DEPENDENCY(HIRVectVLSAnalysis)
//INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
//INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysis)
//INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(AvrDefUseHIR)
//INITIALIZE_PASS_END(VPODriverHIR, "VPODriverHIR",
//                    "VPO Vectorization Driver HIR", false, false)
//
//char VPODriverHIR::ID = 0;
//
//FunctionPass *llvm::createVPODirectiveCleanupPass() {
//  return new VPODirectiveCleanup();
//}
//FunctionPass *llvm::createVPODriverHIRPass() { return new VPODriverHIR(); }

static void collectAllLoops(Loop &L, SmallVectorImpl<Loop *> &V) {
  V.push_back(&L);
  for (Loop *InnerL : L)
    collectAllLoops(*InnerL, V);
}

static bool isVPlanCandidate(Loop *L) {
  MDNode *LoopID = L->getLoopID();

  if (LoopID) {
    for (unsigned i = 1, ie = LoopID->getNumOperands(); i < ie; ++i) {
      MDNode *MD = dyn_cast<MDNode>(LoopID->getOperand(i));
      if (MD) {
        const MDString *S = dyn_cast<MDString>(MD->getOperand(0));
        if (S && S->getString().startswith("vplan.vect.candidate")) {
          return true;
        }
      }
    }
  }

  return false;
}

bool VPlanDriverBase::runOnFunction(Function &Fn) {

  //  if (skipFunction(Fn))
  //    return false;
  //
  //  bool ret_val = false;
  //
  //  DEBUG(dbgs() << "VPODriver: ");
  //  DEBUG(dbgs().write_escaped(Fn.getName()) << '\n');
  //
  LI =  &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE =  &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);
  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  DT =  &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto *AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(Fn);

  // Auxiliary function that check only loop specific constraints. Generic loop
  // nest constraints are in 'isSupported' function
  std::function<bool(Loop *)> isSupportedRec = [&](Loop *Lp) -> bool {

    if (!Lp->getUniqueExitBlock()) {
      DEBUG(dbgs() << "VD: loop form "
                   << "(" << Lp->getName()
                   << ") is not supported: multiple exit blocks.\n");
      return false;
    }
    for (Loop *SubLoop : Lp->getSubLoops()) {
      if (!isSupportedRec(SubLoop))
        return false;
    }

    return true;
  };

  // Return true if this loop is supported in VPlan
  std::function<bool(Loop *)> isSupported = [&](Loop *Lp) -> bool {

    // Check for loop specific constraints
    if (!isSupportedRec(Lp)) {
      DEBUG(dbgs() << "VD: loop nest "
                   << "(" << Lp->getName() << ") is not supported.\n");
      return false;
    }

    // Check generic loop nest constraints
    if (isIrreducibleCFG(Lp, LI)) {
      DEBUG(dbgs() << "VD: loop nest "
                   << "(" << Lp->getName()
                   << ") is not supported: irreducible CFG.\n");
      return false;
    }

    for (BasicBlock *BB : Lp->blocks()) {
      // We don't support switch statements inside loops.
      if (!isa<BranchInst>(BB->getTerminator())) {
        DEBUG(dbgs() << "VD: loop nest contains a switch statement.\n");
        return false;
      }
    }
    
    DEBUG(dbgs() << "VD: loop nest "
          << "(" << Lp->getName() << ") is supported.\n");

    return true;
  };

  if (VPlanVectCand) {
    SmallVector<Loop *, 8> Worklist;
    for (Loop *L : *LI)
      collectAllLoops(*L, Worklist);

    for (auto Lp : Worklist) {
      if (CandLoopsVectorized < VPlanVectCand &&
          isVPlanCandidate(Lp)) {
        processLoop(Lp, Fn);
        CandLoopsVectorized++;
      }
    }
  } else if (!VPlanStressTest) {
    WRContainerImpl *WRGraph = WR->getWRGraph();
    DEBUG(dbgs() << "WD: WRGraph #nodes= " << WRGraph->size() << "\n");
    //for (auto I = WRGraph->begin(), E = WRGraph->end(); I != E; ++I) {
    //  DEBUG((*I)->dump());
    //}

    for (auto WRNode : make_range(WRGraph->begin(), WRGraph->end())) {

      if (WRNVecLoopNode *WRLoop = dyn_cast<WRNVecLoopNode>(WRNode)) {
        Loop *Lp = WRLoop->getLoop();
        simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
        formLCSSARecursively(*Lp, *DT, LI, SE);

        assert((VPlanForceBuild || isSupported(Lp)) &&
               "Loop is not supported by VPlan");

        DEBUG(dbgs() << "VD: Starting VPlan gen for \n");
        DEBUG(WRNode->dump());

        processLoop(Lp, Fn, WRLoop);
      }
    }
  } else {
    DEBUG(dbgs() << "VD: VPlan stress test mode\n");

    // Iterate on TopLevelLoops
    SmallVector<Loop *, 2> WorkList(LI->begin(), LI->end());
    while (!WorkList.empty()) {
      Loop *Lp = WorkList.pop_back_val();

      // Add subloops to worklist
      for (Loop *SubLp: Lp->getSubLoops())
        WorkList.push_back(SubLp);

      // Process only innermost loops if VPlanStressOnlyInnermost is enabled
      if (!VPlanStressOnlyInnermost || Lp->getSubLoops().empty()) {
        simplifyLoop(Lp, DT, LI, SE, AC, false /* PreserveLCSSA */);
        formLCSSARecursively(*Lp, *DT, LI, SE);
        if (VPlanForceBuild || isSupported(Lp))
          processLoop(Lp, Fn);
      }
    }
  }

  return false;
}

INITIALIZE_PASS_BEGIN(VPlanDriver, "VPlanDriver", "VPlan Vectorization Driver",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
//INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(AvrDefUse)
INITIALIZE_PASS_END(VPlanDriver, "VPlanDriver", "VPlan Vectorization Driver",
                    false, false)

char VPlanDriver::ID = 0;

Pass *llvm::createVPlanDriverPass() { return new VPlanDriver(); }

void VPlanDriver::getAnalysisUsage(AnalysisUsage &AU) const {

  // TODO: We do not preserve loopinfo as we remove loops, create new
  // loops. Same holds for Scalar Evolution which needs to be computed
  // for newly created loops. For now only mark AVRGenerate as
  // preserved.

  AU.addRequired<WRegionInfo>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
}

bool VPlanDriver::runOnFunction(Function &F) {
#ifdef INTEL_CUSTOMIZATION
  if (skipFunction(F))
    return false;
#endif

  bool ret_val = false;

  // TODO: get LI only for stress testing
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  WR = &getAnalysis<WRegionInfo>();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  WR->buildWRGraph(WRegionCollection::LLVMIR);

  ret_val = VPlanDriverBase::runOnFunction(F);

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  VPOUtils::stripDirectives(F);

  return ret_val;
}

void VPlanDriver::processLoop(Loop *Lp, Function &F, WRNVecLoopNode *WRLoop) {
  PredicatedScalarEvolution PSE(*SE, *Lp);
  VPOVectorizationLegality LVL(Lp, PSE, TLI, TTI, &F, LI, DT);

  // The function canVectorize() collects information about induction
  // and reduction variables. It also verifies that the loop vectorization
  // is fully supported.
  if (!LVL.canVectorize()) {
    DEBUG(dbgs() << "VD: Not vectorizing: Cannot prove legality.\n");
    
    // Only bail out if we are generating code, we want to continue if
    // we are only stress testing VPlan builds below.
    if (!DisableCodeGen)
      return;
  }

  // Get vectorization factor
  unsigned VF = VPlanDefaultVF;
  if (WRLoop) {
    unsigned Simdlen = WRLoop->getSimdlen();
    assert(Simdlen <= 64 && "Wrong Simdlen value");
    VF = Simdlen ? Simdlen : VPlanDefaultVF;
  }

  LoopVectorizationPlanner *LVP =
      new LoopVectorizationPlanner(WRLoop, Lp, LI, SE, TLI, TTI, DT, LVL);

  LVP->buildInitialVPlans(VF /*MinVF*/, VF /*MaxVF*/);
  // Predicator changes BEGIN
  if (!DisableVPlanPredicator) {
    IntelVPlan *Plan = LVP->getVPlanForVF(VF);
    VPlanPredicator VPP(Plan);
    VPP.predicate();
  }
  // Predicator changes END

  LVP->setBestPlan(VF, 1);

  DEBUG(VPlan *Plan = LVP->getVPlanForVF(VF);
        VPlanPrinter PlanPrinter(dbgs(), *Plan);
        std::string TitleString;
        raw_string_ostream RSO(TitleString);
        RSO << "VD: Initial VPlan for VF=" << VF;
        PlanPrinter.dump(RSO.str()));

  if (!DisableCodeGen) {
    if (VPlanVectCand)
      errs() << "VD: VPlan Generating code in function: " << F.getName() << "\n";

    VPOCodeGen VCodeGen(Lp, PSE, LI, DT, TLI, TTI, VF, 1, &LVL);
    LVP->executeBestPlan(VCodeGen);
  }

  // Destroy LVP
  delete LVP;

  return;
}

#if 0
void VPODirectiveCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
}
#endif
//bool VPODirectiveCleanup::runOnFunction(Function &F) {
//
//  // Skip if disabled
//  if (DisableVPODirectiveCleanup) {
//    return false;
//  }
//
//  // Remove calls to directive intrinsics since the LLVM back end does not know
//  // how to translate them.
//  if (!VPOUtils::stripDirectives(F)) {
//    // If nothing happens, simply return.
//    return false;
//  }
//
//  // Set up a function pass manager so that we can run some cleanup transforms
//  // on the LLVM IR after code gen.
//  Module *M = F.getParent();
//  legacy::FunctionPassManager FPM(M);
//
//  // It is possible that stripDirectives call
//  // eliminates all instructions in a basic block except for the branch
//  // instruction. Use CFG simplify to eliminate them.
//  FPM.add(createCFGSimplificationPass());
//  FPM.run(F);
//
//  return true;
//}

//void VPODriverHIR::getAnalysisUsage(AnalysisUsage &AU) const {
//  // HIR path does not work without setPreservesAll
//  AU.setPreservesAll();
//  AU.addRequired<LoopInfoWrapperPass>();
//  AU.addRequired<AVRGenerateHIR>();
//  AU.addRequired<HIRVectVLSAnalysis>();
//  AU.addRequired<ScalarEvolutionWrapperPass>();
//  AU.addRequired<TargetTransformInfoWrapperPass>();
//  AU.addRequired<TargetLibraryInfoWrapperPass>();
//  AU.addRequired<AvrDefUseHIR>();
//
//  AU.addRequiredTransitive<HIRParser>();
//  AU.addRequiredTransitive<HIRLocalityAnalysis>();
//  AU.addRequiredTransitive<HIRDDAnalysis>();
//  AU.addRequiredTransitive<HIRSafeReductionAnalysis>();
//}
