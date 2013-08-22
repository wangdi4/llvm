#define DEBUG_TYPE "custom-inline"

#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/Function.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Transforms/IPO/InlinerPass.h"

using namespace llvm;

namespace intel {

  // This pass is used for special inlining rules during debugging.
  // At the moment, this inlines only struct-return function (functions in which
  // the first parameter is marked as 'sret'). This is a workaround for the
  // mic and cpu failure outlined at CSSD100016837
  class DebugFunctionInliner : public Inliner {
  public:
    DebugFunctionInliner() : Inliner(ID) {
      initializeDebugFunctionInlinerPass(*PassRegistry::getPassRegistry());
    }

    static char ID; // Pass identification, replacement for typeid

    InlineCost getInlineCost(CallSite CS) {
      return isSretFunction(CS.getCalledFunction()) ?
        InlineCost::getAlways() : InlineCost::getNever();
    }

    bool isSretFunction(Function* F) {
      // Just check the attributes for parameter 1. If there's no such
      // parameter this will just return false.
      return F->getParamAttributes(1).hasAttribute(Attributes::StructRet);
    }

    virtual const char *getPassName() const { return "debug-inliner"; }
  };

  char DebugFunctionInliner::ID = 0;
  OCL_INITIALIZE_PASS_BEGIN(DebugFunctionInliner, "debug-inliner",
      "Use debug-mode rules for inlining", false, false)
  OCL_INITIALIZE_AG_DEPENDENCY(CallGraph)
  OCL_INITIALIZE_PASS_END(DebugFunctionInliner, "debug-inliner",
      "Use debug-mode rules for inlining", false, false)
}

extern "C" {
  Pass *createDebugFunctionInliningPass() {
    return new intel::DebugFunctionInliner();
  }
}
