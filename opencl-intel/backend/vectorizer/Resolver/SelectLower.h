#ifndef __LOWSCALAR_H_
#define __LOWSCALAR_H_
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Module.h"

#include <vector>

using namespace llvm;

namespace intel {
/// @brief Lower vector selects to scalar selects
/// @Author Nadav Rotem
class SelectLower : public FunctionPass {
    public:
    static char ID; // Pass identification, replacement for typeid
    SelectLower() : FunctionPass(ID) {}

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {  }
};

}
#endif //define __LOWSCALAR_H_
