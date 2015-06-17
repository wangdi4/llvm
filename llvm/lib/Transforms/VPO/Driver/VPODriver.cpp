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

namespace intel {

// Temporary Implementation To Test AVR Generation.
static bool buildVectorizerAVR(Function &F, Module &M) {
  legacy::FunctionPassManager fpm(&M);

  AVRGenerate *AVRList = new AVRGenerate();
  fpm.add(AVRList);
  fpm.run(F);

  AVRList->print();

  return true;
}
} // namespace intel

using namespace llvm;
using namespace llvm::vpo;

VPODriver::VPODriver() : FunctionPass(ID) {}

bool VPODriver::runOnFunction(Function &F) {
  DEBUG(errs() << "VPODriver: ");
  DEBUG(errs().write_escaped(F.getName()) << '\n');

  intel::buildVectorizerAVR(F, *(F.getParent()));

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  WR = &getAnalysis<WRegionInfo>();
  SC = &getAnalysis<ScalarEvolution>();

  return false;
}

char VPODriver::ID = 0;

INITIALIZE_PASS_BEGIN(VPODriver, "VPODriver", "VPO Vectorization Driver",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_END(VPODriver, "VPODriver", "VPO Vectorization Driver",
                    false, false)

FunctionPass *llvm::createVPODriverPass() {
  initializeVPODriverPass(*PassRegistry::getPassRegistry());
  return new VPODriver();
}

