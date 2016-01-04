//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/VPO/Vecopt/VecoptPasses.h"
#include "llvm/Transforms/VPO/VPODriver.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"

#include "llvm/Transforms/VPO/Vecopt/VPOAvrLLVMCodeGen.h"
#include "llvm/Transforms/VPO/Vecopt/VPOAvrHIRCodeGen.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"

#define DEBUG_TYPE "VPODriver"

using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(VPODriver, "VPODriver", "VPO Vectorization Driver", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_END(VPODriver, "VPODriver", "VPO Vectorization Driver", false,
                    false)

char VPODriver::ID = 0;

FunctionPass *llvm::createVPODriverPass() { return new VPODriver(); }

VPODriver::VPODriver() : FunctionPass(ID) {

  initializeVPODriverPass(*PassRegistry::getPassRegistry());
}

void VPODriver::getAnalysisUsage(AnalysisUsage &AU) const {

  // TODO: We do not preserve loopinfo as we remove loops, create new
  // loops. Same holds for Scalar Evolution which needs to be computed
  // for newly created loops. For now only mark AVRGenerate as
  // preserved.
  
  // TBD: Unable to get HIR path working without setPreservesAll
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AVRGenerate>();
  AU.addRequired<ScalarEvolutionWrapperPass>();

  AU.addRequiredTransitive<HIRParser>();
  AU.addRequiredTransitive<HIRLocalityAnalysis>();
  AU.addRequiredTransitive<DDAnalysis>();

#if 0
  AU.addPreserved<AVRGenerate>();
  AU.addPreserved<HIRParser>();
  AU.addPreserved<HIRLocalityAnalysis>();
  AU.addPreserved<DDAnalysis>();
#endif
}

bool VPODriver::runOnFunction(Function &F) {

  bool ret_val = false;

  // Set up a function pass manager so that we can run some cleanup transforms
  // on the LLVM IR after code gen.
  Module *M = F.getParent();
  legacy::FunctionPassManager FPM(M);

  DEBUG(errs() << "VPODriver: ");
  DEBUG(errs().write_escaped(F.getName()) << '\n');

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SC = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  AV = &getAnalysis<AVRGenerate>();

  for (auto I = AV->begin(), E = AV->end(); I != E; ++I) {
    AVR *avr = I;
    AVRWrn *AvrWrn;

    AvrWrn = dyn_cast<AVRWrn>(avr);

    if (!AvrWrn) {
      continue;
    }

    if (AvrWrn->getWrnNode()->getIsFromHIR() == false) {
      AVRCodeGen *SP;

      SP = new AVRCodeGen(avr, SC, LI, &F);
      ret_val = ret_val | SP->vectorize();
      delete SP;
    }
    else {
      AVRCodeGenHIR *SP;

      SP = new AVRCodeGenHIR(avr);
      ret_val = ret_val | SP->vectorize();
      delete SP;
    }
  }

  // Print results of AvrGenerate Pass
  DEBUG(AV->dump());

  // Remove calls to directive intrinsics since the LLVM back end does not know
  // how to translate them.
  VPOUtils::stripDirectives(F);

  // It is possible that stripDirectives eliminates all instructions in a basic
  // block except for the branch instruction. Use CFG simplify to eliminate
  // them.

  // Calling CFG simplification pass causes assertions about uses remaining to
  // deleted value handles. Commenting out for now.
  //FPM.add(createCFGSimplificationPass());
  //FPM.run(F);

  return ret_val;
}
