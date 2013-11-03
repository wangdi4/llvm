#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"

#include <sstream>
#include <string>

using namespace llvm;

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

  const std::string RemovePrefetch::m_prefetchBuiltinPrefix = "_Z8prefetchPKU3AS1";


  bool RemovePrefetch::runOnModule(Module &M) {
    // remove prefetch builtins only if requested by the user
    if (getenv("DISMPF") == NULL)
      return false;

    std::vector<Instruction *> removedInst;

    // Find all prefetch builtin function calls and keep them
    for (Module::iterator FI = M.begin(), FE = M.end(); FI != FE; ++FI) {
      Function *F = FI;
      for (Function::iterator BI = F->begin(), BE = F->end(); BI != BE; ++BI) {
        BasicBlock *BB = BI;
        for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE;
             ++II) {
          Instruction *I = II;
          CallInst* pCallInst;
          if ((pCallInst = dyn_cast<CallInst>(I)) != NULL &&
              pCallInst->getCalledFunction()) {
            StringRef Name = pCallInst->getCalledFunction()->getName();
            if (Name.size() >= m_prefetchBuiltinPrefix.size() &&
                Name.startswith(m_prefetchBuiltinPrefix)) {
//                strncmp (m_prefetchBuiltinPrefix.c_str(), Name.str().c_str(),
//                    m_prefetchBuiltinPrefix.size()) == 0) {
              removedInst.push_back(I);
            }
          }
        }
      }
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

