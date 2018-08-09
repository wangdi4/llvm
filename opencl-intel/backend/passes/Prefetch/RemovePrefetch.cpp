// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#define DEBUG_TYPE "DelSWPrefetch"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"

#include "OclTune.h"

#include <sstream>
#include <string>

using namespace llvm;
using namespace intel;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// The RemovePrefetch class is a pass that removes prefetch builtin calls if
// the environment DISMPF is defined. With this pass it's possible to disable
// prefetch calls inserted in the source code.

  class RemovePrefetch : public ModulePass {

    public:
      static char ID; // Pass identification, replacement for typeid
      RemovePrefetch() : ModulePass(ID) {}

      ~RemovePrefetch() {} ;

      virtual bool runOnModule(Module &M);

    private:
      // prefix of the prefetch builtin name
      static const std::string m_prefetchBuiltinPrefix;

  }; // RemovePrefetch class

  const std::string RemovePrefetch::m_prefetchBuiltinPrefix = "_Z8prefetchPU3AS1K";


  bool RemovePrefetch::runOnModule(Module &M) {

    bool removePF = getenv("DISMPF") != nullptr;

    // do not execute this pass unless the user asked to remove manual
    // prefetches or the user is collecting stats for this pass
    if (removePF && !intel::Statistic::isEnabled() &&
        !Statistic::isCurrentStatType(DEBUG_TYPE))
      return false;

    std::vector<Instruction *> removedInst;

    Statistic::ActiveStatsT kernelStats;
    OCLSTAT_DEFINE(SWPrefetches,
        "Number of SW prefetches detected in the code",kernelStats);

    // Find all prefetch builtin function calls and keep them
    for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
      Function *F = &*FI;
      for (Function::iterator BI = F->begin(), BE = F->end(); BI != BE; ++BI) {
        BasicBlock *BB = &*BI;
        for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE;
             ++II) {
          Instruction *I = &*II;
          CallInst* pCallInst;
          if ((pCallInst = dyn_cast<CallInst>(I)) != nullptr &&
              pCallInst->getCalledFunction()) {
            StringRef Name = pCallInst->getCalledFunction()->getName();
            if (Name.size() >= m_prefetchBuiltinPrefix.size() &&
                Name.startswith(m_prefetchBuiltinPrefix)) {
              SWPrefetches++;
              if (removePF)
                removedInst.push_back(I);
            }
          }
        }
      }
      intel::Statistic::pushFunctionStats (kernelStats, *F, DEBUG_TYPE);
    }

    // remove all prefetch builtin calls
    for (unsigned i = 0; i < removedInst.size(); i++)
      removedInst[i]->eraseFromParent();

    return removedInst.size() > 0;
  }


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

extern "C" {
ModulePass * createRemovePrefetchPass() {
  return new Intel::OpenCL::DeviceBackend::RemovePrefetch();
}
}

/// Support for dynamic loading of modules under Linux
char Intel::OpenCL::DeviceBackend::RemovePrefetch::ID = 0;
static RegisterPass<Intel::OpenCL::DeviceBackend::RemovePrefetch>
    CLIRemovePrefetch("remove-pf", "Remove prefetch builtin calls from a module's code.");

