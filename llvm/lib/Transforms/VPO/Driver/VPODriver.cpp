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
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"

#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/VPO/Vecopt/VecoptPasses.h"
#include "llvm/Transforms/VPO/VPODriver.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"

#define DEBUG_TYPE "VPODriver"

using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(VPODriver, "VPODriver", "VPO Vectorization Driver",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_END(VPODriver, "VPODriver", "VPO Vectorization Driver",
                    false, false)

char VPODriver::ID = 0;

FunctionPass *llvm::createVPODriverPass() {

  return new VPODriver();
}

VPODriver::VPODriver() : FunctionPass(ID) {

  initializeVPODriverPass(*PassRegistry::getPassRegistry());
}

void VPODriver::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AVRGenerate>();
  AU.addRequired<ScalarEvolution>();
}

bool VPODriver::runOnFunction(Function &F) {

  bool ret_val;

  DEBUG(errs() << "VPODriver: ");
  DEBUG(errs().write_escaped(F.getName()) << '\n');

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SC = &getAnalysis<ScalarEvolution>();
  AV = &getAnalysis<AVRGenerate>();

  // Print results of AvrGenerate Pass
  AV->dump();

  // Invoke AVR->LLVM_IR Code Generation.
  ret_val = AV->codeGen();

  return ret_val;
}

