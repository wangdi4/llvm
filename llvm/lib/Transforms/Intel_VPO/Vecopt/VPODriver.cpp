//===-- VPODriver.cpp -----------------------------------------------------===//
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
// This file implements VPO vectorizer driver pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPODefUse.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOSIMDLaneEvolution.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOScenarioEvaluation.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPODriver.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VecoptPasses.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrHIRCodeGen.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrLLVMCodeGen.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRVectVLSAnalysis.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPODriver"

static cl::opt<bool>
    DisableVPODirectiveCleanup("disable-vpo-directive-cleanup", cl::init(false),
                               cl::Hidden,
                               cl::desc("Disable VPO directive cleanup"));

using namespace llvm;
using namespace llvm::vpo;

namespace {
class VPODirectiveCleanup : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODirectiveCleanup() : FunctionPass(ID) {
    initializeVPODirectiveCleanupPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  //  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

class VPODriver : public VPODriverBase {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODriver() : VPODriverBase(ID) {
    initializeVPODriverPass(*PassRegistry::getPassRegistry());
    ScenariosEngine = nullptr;
  }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  VPOScenarioEvaluationBase &getScenariosEngine(AVRWrn *AvrWrn,
                                                Function &F) override {
    ScenariosEngine =
        new VPOScenarioEvaluation(AvrWrn, *TTI, *TLI, F.getContext(), *DefUse);
    return *ScenariosEngine;
  }

  void resetScenariosEngineForRegion() override {
    if (ScenariosEngine) {
      delete ScenariosEngine;
      ScenariosEngine = nullptr;
    }
  }

private:
  VPOScenarioEvaluation *ScenariosEngine;
  AvrDefUse *DefUse;
};

class VPODriverHIR : public VPODriverBase {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODriverHIR() : VPODriverBase(ID) {
    initializeVPODriverHIRPass(*PassRegistry::getPassRegistry());
    ScenariosEngine = nullptr;
  }
  bool runOnFunction(Function &F) override {
    AV = &getAnalysis<AVRGenerateHIR>();
    DDA = &getAnalysis<HIRDDAnalysis>();
    VLS = &getAnalysis<HIRVectVLSAnalysis>();
    DefUse = &getAnalysis<AvrDefUseHIR>();
    return VPODriverBase::runOnFunction(F);
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;

#if !INTEL_PRODUCT_RELEASE
  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override {
    return createHIRPrinterPass(OS, Banner);
  }
#endif // !INTEL_PRODUCT_RELEASE

  VPOScenarioEvaluationBase &getScenariosEngine(AVRWrn *AvrWrn,
                                                Function &F) override {
    ScenariosEngine = new VPOScenarioEvaluationHIR(AvrWrn, DDA, VLS, *DefUse,
                                                   *TTI, *TLI, F.getContext());
    return *ScenariosEngine;
  }

  void resetScenariosEngineForRegion() override {
    if (ScenariosEngine) {
      delete ScenariosEngine;
      ScenariosEngine = nullptr;
    }
  }

private:
  HIRDDAnalysis *DDA;
  HIRVectVLSAnalysis *VLS;
  AvrDefUseHIR *DefUse;
  VPOScenarioEvaluationHIR *ScenariosEngine;
};

} // anonymous namespace

INITIALIZE_PASS_BEGIN(VPODirectiveCleanup, "VPODirectiveCleanup",
                      "VPO Directive Cleanup", false, false)
INITIALIZE_PASS_END(VPODirectiveCleanup, "VPODirectiveCleanup",
                    "VPO Directive Cleanup", false, false)

char VPODirectiveCleanup::ID = 0;

INITIALIZE_PASS_BEGIN(VPODriver, "VPODriver", "VPO Vectorization Driver", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AvrDefUse)
INITIALIZE_PASS_END(VPODriver, "VPODriver", "VPO Vectorization Driver", false,
                    false)

char VPODriver::ID = 0;

INITIALIZE_PASS_BEGIN(VPODriverHIR, "VPODriverHIR",
                      "VPO Vectorization Driver HIR", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRVectVLSAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysis)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AvrDefUseHIR)
INITIALIZE_PASS_END(VPODriverHIR, "VPODriverHIR",
                    "VPO Vectorization Driver HIR", false, false)

char VPODriverHIR::ID = 0;

FunctionPass *llvm::createVPODirectiveCleanupPass() {
  return new VPODirectiveCleanup();
}
FunctionPass *llvm::createVPODriverPass() { return new VPODriver(); }
FunctionPass *llvm::createVPODriverHIRPass() { return new VPODriverHIR(); }

bool VPODriverBase::runOnFunction(Function &Fn) {
  if (skipFunction(Fn))
    return false;

  bool ret_val = false;

  DEBUG(errs() << "VPODriver: ");
  DEBUG(errs().write_escaped(Fn.getName()) << '\n');

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SC = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Fn);
  TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

  // TODO - driver should not invoke VPODriver for non-vector target
  // Until then - bail out. This should be changed to an assert later.
  if (TTI->getRegisterBitWidth(true) == 0)
    return false;

  for (auto I = AV->begin(), E = AV->end(); I != E; ++I) {
    AVR *Avr = &*I;
    AVRWrn *AvrWrn;

    if (!(AvrWrn = dyn_cast<AVRWrn>(Avr))) {
      continue;
    }

    VPOScenarioEvaluationBase &ScenariosEngine = getScenariosEngine(AvrWrn, Fn);

    if (AvrWrn->getWrnNode()->getIsFromHIR() == false) {
      AVRCodeGen AvrCGNode(Avr, SC, LI, TLI, &Fn);
      assert(isa<VPOScenarioEvaluation>(ScenariosEngine));
      VPOScenarioEvaluation *LLVMIRScenariosEngine =
          cast<VPOScenarioEvaluation>(&ScenariosEngine);
      LLVMIRScenariosEngine->setCG(&AvrCGNode);

      // Decide if/how to vectorize in terms of profitability.
      VPOVecContextBase VC = ScenariosEngine.getBestCandidate(AvrWrn);

      // FORNOW: We pass CodeGen only the selected VF, since currently AvrWrn
      // contains only a single ALoop.
      int VF = VC.getVectFactor();
      DEBUG(errs() << "VPODriver: Scenarios engine selected VF " << VF << "\n");

      // Widen selected candidate
      ret_val = ret_val | AvrCGNode.vectorize(VF);
    } else {
      HIRSafeReductionAnalysis *SRA;
      SRA = &getAnalysis<HIRSafeReductionAnalysis>();
      AVRCodeGenHIR AvrCGNode(Avr, TLI, SRA, Fn);

      assert(isa<VPOScenarioEvaluationHIR>(ScenariosEngine));
      VPOScenarioEvaluationHIR *HIRScenariosEngine =
          cast<VPOScenarioEvaluationHIR>(&ScenariosEngine);
      HIRScenariosEngine->setCG(&AvrCGNode);

      // Decide if/how to vectorize in terms of profitability.
      VPOVecContextBase VC = ScenariosEngine.getBestCandidate(AvrWrn);

      // FORNOW: We pass CodeGen only the selected VF, since currently AvrWrn
      // contains only a single ALoop.
      int VF = VC.getVectFactor();
      DEBUG(errs() << "VPODriver: Scenarios engine selected VF " << VF << "\n");

      // Widen selected candidate
      ret_val = ret_val | AvrCGNode.vectorize(VF);
    }

    resetScenariosEngineForRegion();
  }

  return ret_val;
}

#if 0
void VPODirectiveCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
}
#endif

bool VPODirectiveCleanup::runOnFunction(Function &F) {

  // Skip if disabled
  if (DisableVPODirectiveCleanup) {
    return false;
  }

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  if (!VPOUtils::stripDirectives(F)) {
    // If nothing happens, simply return.
    return false;
  }

  // Set up a function pass manager so that we can run some cleanup transforms
  // on the LLVM IR after code gen.
  Module *M = F.getParent();
  legacy::FunctionPassManager FPM(M);

  // It is possible that stripDirectives call
  // eliminates all instructions in a basic block except for the branch
  // instruction. Use CFG simplify to eliminate them.
  FPM.add(createCFGSimplificationPass());
  FPM.run(F);

  return true;
}

void VPODriver::getAnalysisUsage(AnalysisUsage &AU) const {

  // TODO: We do not preserve loopinfo as we remove loops, create new
  // loops. Same holds for Scalar Evolution which needs to be computed
  // for newly created loops. For now only mark AVRGenerate as
  // preserved.

  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AVRGenerate>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<AvrDefUse>();

  AU.addPreserved<AVRGenerate>();
  AU.addPreserved<TargetTransformInfoWrapperPass>();
  AU.addPreserved<AvrDefUse>();
}

bool VPODriver::runOnFunction(Function &F) {

  AV = &getAnalysis<AVRGenerate>();
  DefUse = &getAnalysis<AvrDefUse>();
  bool ret_val = VPODriverBase::runOnFunction(F);

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  VPOUtils::stripDirectives(F);

  return ret_val;
}

void VPODriverHIR::getAnalysisUsage(AnalysisUsage &AU) const {
  // HIR path does not work without setPreservesAll
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AVRGenerateHIR>();
  AU.addRequired<HIRVectVLSAnalysis>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<AvrDefUseHIR>();

  AU.addRequiredTransitive<HIRParser>();
  AU.addRequiredTransitive<HIRLocalityAnalysis>();
  AU.addRequiredTransitive<HIRDDAnalysis>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysis>();
}
