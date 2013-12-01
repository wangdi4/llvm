#ifndef __OPENCL_ALIAS_ANALYSIS_H__
#define __OPENCL_ALIAS_ANALYSIS_H__

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/SmallSet.h"

namespace llvm { 
  struct OpenCLAliasAnalysis : public ModulePass, public AliasAnalysis {
    
    static char ID; 
 
    OpenCLAliasAnalysis() : ModulePass(ID) {
      initializeOpenCLAliasAnalysisPass(*PassRegistry::getPassRegistry());
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<AliasAnalysis>();
      AU.setPreservesAll();
    }

    virtual bool runOnModule(Module &M);     

    // Returns true if O was an actual parameter for a NoAlias formal parameter and false o/w
    bool isNoAliasArgument(const Value *O);

    virtual AliasResult alias(const Location &LocA, const Location &LocB);

    virtual void *getAdjustedAnalysisPointer(const void *ID) {
      if (ID == &AliasAnalysis::ID)
        return (AliasAnalysis*)this;
      return this;
    }

  private:
    // keeps pointers to actual parameter for a NoAlias formal parameter that appeared at the wrapper's inlined function
    llvm::SmallSet<Value *, 16>  noAliasArgs;
  };
}
#endif

