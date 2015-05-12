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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/LegacyPassManager.h"

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"

using namespace llvm;

#define DEBUG_TYPE "VecDriver"


STATISTIC(NumVectorizations, "Counts number of vectorizations");


namespace intel {

  // Temporary Implementation To Test AVR Generation.
  static bool buildVectorizerAVR(Function& F, Module& M) 
  {
    legacy::FunctionPassManager fpm(&M); 
    
    AVRGenerate* AVRList = new AVRGenerate();
    fpm.add(AVRList);
    fpm.run(F);
    
    AVRList->print();
    
    return true;
  }
} // namespace intel

namespace {
  // VecDriver - Do function/loop vectorization.
  struct VecDriver : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    VecDriver() : FunctionPass(ID) {}

    LoopInfo *LI;
    bool runOnFunction(Function &F) override {
      DEBUG(errs() << "VecDriver: ");
      DEBUG(errs().write_escaped(F.getName()) << '\n');

      intel::buildVectorizerAVR(F, *(F.getParent()));

      LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
      return false;
    }

    // We don't modify the program for now, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.setPreservesAll();
    }
  };
}

char VecDriver::ID = 0;
static RegisterPass<VecDriver> Y("vec-driver", "Vectorization Driver Pass");
