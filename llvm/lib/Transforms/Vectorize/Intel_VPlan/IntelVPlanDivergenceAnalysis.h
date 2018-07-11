//===------------------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPlanDivergenceAnalysis class. This class computes
/// the divergent/uniform properties of all VPValues within a VPlan region with
/// respect to the loop(s) considered for vectorization.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_DIVERGENCE_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_DIVERGENCE_ANALYSIS_H

#include "IntelVPlanLoopInfo.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

namespace llvm {

namespace vpo {

class VPValue;
class VPInstruction;
class VPBasicBlock;
class VPLoop;
class VPRegionBlock;
class VPBlockBase;

using VPDominatorTree = DomTreeBase<VPBlockBase>;
using VPPostDominatorTree = PostDomTreeBase<VPBlockBase>;

class VPlanDivergenceAnalysis {

public:
  ~VPlanDivergenceAnalysis();

  /// Computes divergence information for the candidate vector loop using
  /// current dominator information and loop info. TODO: VF to be added at
  /// a later time. This is a public interface that can be used in case
  /// DA needs to be recomputed using updated dominator tree information.
  /// Note that if the dominator trees changed, so will VPLI, which is why
  /// it is also passed in here.
  void compute(VPLoop *CandidateLoop, VPLoopInfo *VPLI,
               VPDominatorTree *RegionDT, VPPostDominatorTree *RegionPDT);

private:
  bool updateDivergenceInfo(const VPInstruction *I);

  /// Prints the divergence information for each VPValue in the Region.
  void print(raw_ostream &OS, VPLoop *VPLp) const;

  /// Set of divergent VPValues.
  DenseSet<const VPValue *> DivergentValues;

  /// Map of VPValues to stride.
  DenseMap<const VPValue *, unsigned> StrideMap;

  /// Map of VPValues to alignment.
  DenseMap<const VPValue *, int> AlignmentMap;

  /// The VPBasicBlocks that are marked as divergent as a result of Branch
  /// Dependence Analysis. This information is used to transfer divergent
  /// properties to PHI nodes.
  SmallDenseSet<const VPBasicBlock *, 4> DivergentBlocks;

  void markDivergent(const VPValue *V);
  bool isDivergent(const VPValue *V) const;
};
} // namespace vpo
} // namespace llvm
#endif
