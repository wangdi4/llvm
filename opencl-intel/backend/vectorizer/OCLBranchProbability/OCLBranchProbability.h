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

#ifndef __OCL_BRANCH_PROB_H__
#define __OCL_BRANCH_PROB_H__

#include "WIAnalysis.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"

namespace intel {

#define NormalWeight   16
#define TakenWeight    64
#define NonTakenWeight  1

  using namespace llvm;

  /// @brief Branch probability that takes into account uniform and consecutive information.
  /// The pass utilizes WIAnalysis and gives high priority for one of the successors in the following cases:
  /// CONSECUTIVE == UNIFORM - the non-taken successor gets high priority
  /// CONSECUTIVE != UNIFORM - the taken successor gets high priority
  /// CONSECUTIVE ? UNIFORM and one of the successor's terminator is a return instruction
  ///                        - the other successor gets high priority
  class OCLBranchProbability : public FunctionPass {

    public:

      /// Pass identification, replacement for typeid
      static char ID;
      OCLBranchProbability() : FunctionPass(ID) {}

      /// @brief Provides name of pass
      virtual llvm::StringRef getPassName() const {
        return "OCLBranchProbability";
      }

      /// @brief    LLVM Function pass entry
      /// @param F  Function to transform
      /// @returns  true if changed    F.dump();

      virtual bool runOnFunction(Function &F);

    private:

      /// @brief  LLVM Interface
      /// @param AU Analysis
      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesAll();
        AU.addRequired<WIAnalysis>();
        AU.addRequired<BranchProbabilityInfoWrapperPass>();
      }

    public:

      /// The following are wrapper functions for BranchProbabilityInfo

      // Get an edge's probability, relative to other out-edges of the Src
      BranchProbability getEdgeProbability(const BasicBlock *Src, unsigned IndexInSuccessors) const;
      // Get the probability of going from Src to Dst
      BranchProbability getEdgeProbability(const BasicBlock *Src, const BasicBlock *Dst) const;

      // Test if an edge is hot relative to other out-edges of the Src
      // Check whether this edge out of the source block is 'hot'. We define hot as having a relative probability >= 80%.
      bool isEdgeHot(const BasicBlock *Src, const BasicBlock *Dst) const;
      // Retrieve the hot successor of a block if one exists
      const BasicBlock * getHotSucc(const BasicBlock *BB) const;

    private:

      BranchProbabilityInfo * m_BPI;
      WIAnalysis * m_WIA;
  };

} // namespace intel

#endif
