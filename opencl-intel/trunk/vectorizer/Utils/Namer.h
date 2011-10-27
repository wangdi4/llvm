#ifndef __BBNAMER_H_
#define __BBNAMER_H_
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
/// @brief A debug pass to give nice readable names to basic blocks
/// @Author Nadav Rotem
class Namer : public FunctionPass {
    public:
    static char ID; // Pass identification, replacement for typeid
    Namer() : FunctionPass(ID) {}

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F);

    /*! \name Debug Helpers
     * \{ */
    /// @brief DEBUG: give basic blocks letter names (A, B, C ...)
    /// @param BB BB to name
    void nameBB(BasicBlock *BB);
    /*! \} */

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {  }
};

}
#endif //define __BBNAMER_H_
