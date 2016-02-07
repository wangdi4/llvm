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
//   VPOVecCandIdenitfy.h -- Declares Identify Vector Candidate Class
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_VECOPT_VECTOR_CANDIDATE_IDENTIFY_H
#define LLVM_ANALYSIS_VPO_VECOPT_VECTOR_CANDIDATE_IDENTIFY_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVectorCandidate.h"

namespace llvm { // LLVM Namespace
namespace vpo { // VPO Vectorizer Namespace

class WRegionInfo;

/// \brief TODO.
class IdentifyVectorCandidates : public FunctionPass {

public:

  /// Pass Identification
  static char ID;

  IdentifyVectorCandidates();

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void verifyAnalysis() const override;
  void print();

  typedef SmallVector<VectorCandidate *, 16> VecCandidatesTy;

  typedef VecCandidatesTy::iterator iterator;
  typedef VecCandidatesTy::const_iterator const_iterator;
  typedef VecCandidatesTy::reverse_iterator reverse_iterator;
  typedef VecCandidatesTy::const_reverse_iterator const_reverse_iterator;

private:
 
  /// Vector data structure of Vectorization Candidates
  VecCandidatesTy VecCandidates;

  /// \brief Identifies candidate loops for vectorization
  void identifyVectorCandidates();

  /// \brief Identifies explicit SIMD loops for vectorization
  void identifyExplicitCandidates();

  /// \brief Identifies candidate loops for auto-vectorization
  void identifyAutoCandidates();

  /// True when auto-vectorization is enabled
  bool AutoVectorizationEnabled = false;

  /// Work Region Info Pass.
  WRegionInfo *WR;

  /// LoopInfo Pass.
  LoopInfo *LI;

public:

  /// Iterator Methods
  iterator begin() { return VecCandidates.begin(); }
  const_iterator begin() const { return VecCandidates.begin(); }
  iterator end() { return VecCandidates.end(); }
  const_iterator end() const { return VecCandidates.end(); }

  reverse_iterator rbegin() { return VecCandidates.rbegin(); }
  const_reverse_iterator rbegin() const { return VecCandidates.rbegin(); }
  reverse_iterator rend() { return VecCandidates.rend(); }
  const_reverse_iterator rend() const { return VecCandidates.rend(); }

  VecCandidatesTy getVectorizationCandidates() {return VecCandidates;}

  unsigned getNumCandidates() { return VecCandidates.size(); }

  /// \brief Release memory for identified vector candidates.
  void releaseMemory();

};

} // END VPO Vectorizer Namespace
} // END LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_VECOPT_VECTOR_CANDIDATE_IDENTIFY_H
