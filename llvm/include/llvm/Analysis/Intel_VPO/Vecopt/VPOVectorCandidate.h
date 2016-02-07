//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOVecoptCandidate.h -- Declares loop vectorization candidate object.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_VECOPT_VECTOR_CANDIDATE_H
#define LLVM_ANALYSIS_VPO_VECOPT_VECTOR_CANDIDATE_H

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"

namespace llvm { // LLVM Namespace

class HLNode;
class LoopInfo;

namespace vpo { // VPO Vectorizer Namespace

class WRNVecLoopNode;

/// \brief Candidate loop for vectorization.
///
///
class VectorCandidate {

public:

  typedef SmallPtrSet<const BasicBlock *, 32> VecCandBBlockSetTy;
  typedef VecCandBBlockSetTy::const_iterator const_bb_iterator;

  // This constructor moved to public to make it accessible from VecNodeVisitor
  /// LLVM IR Constructor
  VectorCandidate(WRNVecLoopNode *WRN);

protected:

  /// HIR Constructor
  VectorCandidate(HLNode* HLN);

  ~VectorCandidate() {}

  friend class IdentifyVectorCandidates;

private:

  //  All VectorCandidate objects should be built from an underlying
  //  Work Region Node.
  /// Work Region Node
  WRNVecLoopNode *WRNode;

  /// HIR Node (if available)
  HLNode *HIRNode;

  /// Candidate vector loop llvm IR entry basic block
  BasicBlock *EntryBBlock;

  /// Candidate vector loop llvm IR exit basic block
  BasicBlock *ExitBBlock;

  /// Candidate vector loop llvm ir basic block set
  VecCandBBlockSetTy BBlockSet;

  /// Candidate vector loop is incoming from HIR.
  bool IsHIRCandidate;

  /// \brief Destroys all objects of this class. Should be called only after
  /// AVRs have been generated.
  static void destroyAll();

  /// \brief Sets the entry(first) bblock of the vector candidate loop
  void setEntryBBlock(BasicBlock *EntryBB) { EntryBBlock = EntryBB; }

  /// \brief Sets the exit(last) bblock of the vector candidate loop
  void setExitBBlock(BasicBlock *ExitBB) { ExitBBlock = ExitBB; }

  /// \brief Sets the set of basic blocks which form this candidate loop
  void setCandBBlockSet(VecCandBBlockSetTy BBset) { BBlockSet = BBset; }


public:

  /// \brief Dumps Vector Candidate 
  void dump() const;

  /// \brief Prints the vector candidate;
  void print() const;

  /// \brief Returns the entry(first) bblock of this region.
  BasicBlock *getEntryBBlock() const { return EntryBBlock; }

  /// \brief Returns the exit(last) bblock of this region.
  BasicBlock *getExitBBlock() const { return ExitBBlock; }

  /// \brief Returns the set of basic blocks which form this candidate
  /// loop.
  const VecCandBBlockSetTy &getBBlockSet() const { return BBlockSet; }

  ///\brief Returns the Loop Info for the vector candidate.
  const LoopInfo *getLoopInfo() const { return WRNode->getLoopInfo(); }

  /// \brief Returns the predecessor bblock of this region.
  BasicBlock *getPredBBlock() const;

  /// \brief Returns the successor bblock of this region.
  BasicBlock *getSuccBBlock() const;

  /// \brief Returns the WRN node from which this candidate was built.
  WRNVecLoopNode *getWrnNode() { return WRNode; }

  // Iterator Methods
  const_bb_iterator bb_begin() const { return BBlockSet.begin(); }
  const_bb_iterator bb_end() const { return BBlockSet.end(); }

};

} // END VPO Vectorizer Namespace
} // END LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_VECOPT_VECTOR_CANDIDATE_H
