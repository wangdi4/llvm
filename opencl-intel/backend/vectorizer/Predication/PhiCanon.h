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

#ifndef __PHICANON_H_
#define __PHICANON_H_
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Module.h"

#include <vector>

using namespace llvm;

namespace intel {

/// @brief
///  Phi canonicalizatiton. This pass converts each PHINode with three
///  or more entries into a two-based PHINode. It does so by
///  splitting two of the edges and creating an additional basic block.
///  This trashes the CFG. However, future passes can easily go over the
///  CFG and clean it.
class PhiCanon : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  PhiCanon();

  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "PhiCanon";
  }

  /// @brief LLVM Function pass entry
  /// @param F Function to transform
  /// @return True if changed
  virtual bool runOnFunction(Function &F);
  /// @brief Perform the modifications to the BasicBlock
  /// Create a new BasicBlock with incoming edges
  /// @param toFix BasicBlock to fix
  void fixBlock(BasicBlock* toFix);
  /// @brief Make new intermediate Basic Block so that it will become PHI block
  /// for 'prev0' and 'prev1' instead of old one
  /// @param toFix old block to be replaced as PHI block for 'prev0' and 'prev1'
  /// @param prev0 first block to be retargeted to the intermediate block
  /// @param prev1 second block to be retargeted to the intermediate block
  /// @return pointer to the created Basic Block
  BasicBlock* makeNewPhiBB(BasicBlock* toFix, BasicBlock* prev0, BasicBlock* prev1);
  /// @brief After creating a new intermediate BasicBlock,
  /// predecessors must jump to the new BB and not to the old one.
  /// @param to_fix The Pointer
  /// @param old_target The pointee
  /// @param new_target New target to point
  void fixBasicBlockSucessor(BasicBlock* to_fix,
                             BasicBlock* old_target,
                             BasicBlock* new_target);
  // Need Dominator Tree and PostDominator tree prior to Phi Canonization
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<PostDominatorTreeWrapperPass>();
  }

};

}
#endif //define __PHICANON_H_
