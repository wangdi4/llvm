/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __SIMPLIFY_GEP_H_
#define __SIMPLIFY_GEP_H_
#include "WIAnalysis.h"
#include "Logger.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3425
#include "llvm/Target/TargetData.h"
#else
#include "llvm/IR/DataLayout.h"
#endif

using namespace llvm;

namespace intel {

  /// @brief Simplify GEP class used to convert GEP instructions with
  ///  more than one index that are used as base pointer to load/store
  ///  instruction into two GEP instructions where the later is with
  ///  single index and set it as the base pointer to the load/store.
  class SimplifyGEP : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    SimplifyGEP();

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
    /// @brief Fix PhiNodes with GEP entries
    /// @param F Function to transform
    /// @return True if changed
    virtual bool FixPhiNodeGEP(Function &F);

    /// @brief Fix GEP with multi indices
    /// @param F Function to transform
    /// @return True if changed
    virtual bool FixMultiIndicesGEP(Function &F);

    /// @brief Check if given GEP instruction is simplifiable
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction can be simplified, False otherwise
    bool SimplifiableGep(GetElementPtrInst *pGEP);

    /// @brief Check if given GEP instruction is uniform after being simplified
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction can be simplified to uniform GEP, False otherwise
    bool IsUniformSimplifiableGep(GetElementPtrInst *pGEP);

    /// @brief Check if given GEP instruction has i32 indices
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction can be simplified to GEP with i32 index, False otherwise
    bool IsIndexTypeSimplifiableGep(GetElementPtrInst *pGEP);

    /// @brief Check if given PhiNode instruction is simplifiable
    /// @param pPhiNode PhiNode instruction
    /// @return negative number if PhiNode not is not simplifiable, otherwise
    //    0 or 1 according to the PhiNode incoming entry that contains the iterator instruction.
    int SimplifiablePhiNode(PHINode *pPhiNode);

  private:
    /// @brief pointer to work-item analysis performed for this function
    WIAnalysis *m_depAnalysis;
    /// @brief This holds DataLayout of processed module
#if LLVM_VERSION == 3425
    TargetData *m_pDL;
#else
    DataLayout *m_pDL;
#endif
  };
} // namespace


#endif //define __SIMPLIFY_GEP_H_
