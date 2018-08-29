// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __SIMPLIFY_GEP_H_
#define __SIMPLIFY_GEP_H_
#include "WIAnalysis.h"
#include "Logger.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DataLayout.h"

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
    virtual llvm::StringRef getPassName() const {
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

    /// @brief Simplify GEP instruction with all indices except the last one are uniform
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction has been simplified, False otherwise
    bool SimplifyUniformGep(GetElementPtrInst *pGEP);

    /// @brief Simplify GEP instruction with i32 indices
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction has been simplified, False otherwise
    bool SimplifyIndexTypeGep(GetElementPtrInst *pGEP);

    /// @brief Check if given PhiNode instruction is simplifiable
    /// @param pPhiNode PhiNode instruction
    /// @return negative number if PhiNode not is not simplifiable, otherwise
    //    0 or 1 according to the PhiNode incoming entry that contains the iterator instruction.
    int SimplifiablePhiNode(PHINode *pPhiNode);


    /// @brief If a GEP has signel index which is a sum of uniform and random/strided values
    //         then reassotiate this sum and make two GEPs one of which is uniform.
    /// @param pGEP GEP instruction
    /// @return True if GEP instruction has been transformed, False otherwise
    bool ReassociateIndexSum(GetElementPtrInst *pGEP);

  private:
    /// @brief pointer to work-item analysis performed for this function
    WIAnalysis *m_depAnalysis;
    /// @brief This holds DataLayout of processed module
    const DataLayout *m_pDL;

    Statistic::ActiveStatsT m_kernelStats;
    Statistic Simplified_Multi_Indices_GEPs;
    Statistic Simplified_Phi_Node_GEPs;
  };
} // namespace


#endif //define __SIMPLIFY_GEP_H_
