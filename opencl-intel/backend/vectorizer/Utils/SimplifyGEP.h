/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  SimplifyGEP.h

\*****************************************************************************/

#ifndef __SIMPLIFY_GEP_H_
#define __SIMPLIFY_GEP_H_
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"

#include "WIAnalysis.h"
#include "Logger.h"

using namespace llvm;

namespace intel {

  /// @brief Simplify GEP class used to convert GEP instructions with
  ///  more than one index that are used as base pointer to load/store
  ///  instruction into two GEP instructions where the later is with
  ///  single index and set it as the base pointer to the load/store.
  class SimplifyGEP : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    SimplifyGEP() : FunctionPass(ID) {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "SimplifyGEP";
    }

    /// @brief  LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      AU.addRequired<WIAnalysis>();
    }

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F);

  private:
    /// @brief Check if given GEP instruction is simplifiable
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction can be simplified, False otherwise
    bool SimplifiableGep(GetElementPtrInst *pGEP);

    /// @brief Check if given GEP instruction is unfirom after being simplified
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction can be simplified to uniform GEP, False otherwise
    bool IsUniformSimplifiableGep(GetElementPtrInst *pGEP);

    /// @brief Check if given GEP instruction has i32 indices
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction can be simplified to GEP with i32 index, False otherwise
    bool IsIndexTypeSimplifiableGep(GetElementPtrInst *pGEP);

  private:
    /// @brief pointer to work-item analysis performed for this function
    WIAnalysis *m_depAnalysis;
  };
} // namespace


#endif //define __SIMPLIFY_GEP_H_
