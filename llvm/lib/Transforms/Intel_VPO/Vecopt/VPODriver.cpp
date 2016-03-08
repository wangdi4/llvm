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

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VecoptPasses.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPODriver.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"

#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrLLVMCodeGen.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrHIRCodeGen.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"


#define DEBUG_TYPE "VPODriver"

using namespace llvm;
using namespace llvm::vpo;

namespace {
class VPODriver : public VPODriverBase {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODriver() : VPODriverBase(ID) {
    initializeVPODriverPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

class VPODriverHIR : public VPODriverBase {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODriverHIR() : VPODriverBase(ID) {
    initializeVPODriverHIRPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override {
    AV = &getAnalysis<AVRGenerateHIR>();
    return VPODriverBase::runOnFunction(F);
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override {
    return createHIRPrinterPass(OS, Banner);
  }
};

} // anonymous namespace

INITIALIZE_PASS_BEGIN(VPODriver, "VPODriver", "VPO Vectorization Driver",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_END(VPODriver, "VPODriver", "VPO Vectorization Driver",
                    false, false)

char VPODriver::ID = 0;

INITIALIZE_PASS_BEGIN(VPODriverHIR, "VPODriverHIR",
                      "VPO Vectorization Driver HIR", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_END(VPODriverHIR, "VPODriverHIR",
                    "VPO Vectorization Driver HIR", false, false)

char VPODriverHIR::ID = 0;

FunctionPass *llvm::createVPODriverPass() { return new VPODriver(); }
FunctionPass *llvm::createVPODriverHIRPass() { return new VPODriverHIR(); }

bool VPODriverBase::runOnFunction(Function &F) {

  bool ret_val = false;

  DEBUG(errs() << "VPODriver: ");
  DEBUG(errs().write_escaped(F.getName()) << '\n');

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SC = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  for (auto I = AV->begin(), E = AV->end(); I != E; ++I) {
    AVR *Avr = &*I;
    AVRWrn *AvrWrn;

    if (!(AvrWrn = dyn_cast<AVRWrn>(Avr))) {
      continue;
    }

    if (AvrWrn->getWrnNode()->getIsFromHIR() == false) {
      AVRCodeGen AvrCGNode(Avr, SC, LI, &F);

      ret_val = ret_val | AvrCGNode.vectorize();
    }
    else {
      AVRCodeGenHIR AvrCGNode(Avr);

      ret_val = ret_val | AvrCGNode.vectorize();
    }
  }

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  VPOUtils::stripDirectives(F);

  return ret_val;
}

void VPODriver::getAnalysisUsage(AnalysisUsage &AU) const {

  // TODO: We do not preserve loopinfo as we remove loops, create new
  // loops. Same holds for Scalar Evolution which needs to be computed
  // for newly created loops. For now only mark AVRGenerate as
  // preserved.
  
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AVRGenerate>();
  AU.addRequired<ScalarEvolutionWrapperPass>();

  AU.addPreserved<AVRGenerate>();
}

bool VPODriver::runOnFunction(Function &F) {

  AV = &getAnalysis<AVRGenerate>();
  bool ret_val = VPODriverBase::runOnFunction(F);

  // Set up a function pass manager so that we can run some cleanup transforms
  // on the LLVM IR after code gen.
  Module *M = F.getParent();
  legacy::FunctionPassManager FPM(M);

  // It is possible that stripDirectives call in parent runOnFunction()
  // eliminates all instructions in a basic block except for the branch
  // instruction. Use CFG simplify to eliminate them.
  FPM.add(createCFGSimplificationPass());
  FPM.run(F);

  return ret_val;
}

void VPODriverHIR::getAnalysisUsage(AnalysisUsage &AU) const {
  // HIR path does not work without setPreservesAll
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AVRGenerateHIR>();
  AU.addRequired<ScalarEvolutionWrapperPass>();

  AU.addRequiredTransitive<HIRParser>();
  AU.addRequiredTransitive<HIRLocalityAnalysis>();
  AU.addRequiredTransitive<DDAnalysis>();
}

