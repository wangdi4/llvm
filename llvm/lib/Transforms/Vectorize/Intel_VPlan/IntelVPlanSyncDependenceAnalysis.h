#if INTEL_COLLAB
//===------------ IntelVPlanSyncDependenceAnalysis.h -*- C++ -*------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// This file defines the SyncDependenceAnalysis class, which computes for
// every divergent branch the set of phi nodes that the branch will make
// divergent.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_SYNC_DEPENDENCE_ANALYSIS_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_SYNC_DEPENDENCE_ANALYSIS_H

#include "IntelVPBasicBlock.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallPtrSet.h"

namespace llvm {
namespace vpo {

class VPLoopInfo;
using ConstBlockSet = SmallPtrSet<const VPBasicBlock *, 4>;

/// Relates points of divergent control to join points in reducible CFGs.
///
/// This analysis relates points of divergent control to points of converging
/// divergent control. The analysis requires all loops to be reducible.
class SyncDependenceAnalysis {
  void visitSuccessor(const VPBasicBlock &SuccBlock, const VPLoop *TermLoop,
                      const VPBasicBlock *DefBlock);

public:
  bool inRegion(const VPBasicBlock &BB) const;

  ~SyncDependenceAnalysis();
  SyncDependenceAnalysis(const VPBasicBlock *RegionEntry,
                         const VPDominatorTree &DT,
                         const VPPostDominatorTree &PDT, const VPLoopInfo &LI);

  /// Computes divergent join points and loop exits caused by branch divergence
  /// in \p Term.
  ///
  /// The set of blocks which are reachable by disjoint paths from \p Term.
  /// The set also contains loop exits if there two disjoint paths:
  /// one from \p Term to the loop exit and another from \p Term to the loop
  /// header. Those exit blocks are added to the returned set.
  /// If L is the parent loop of \p Term and an exit of L is in the returned
  /// set then L is a divergent loop.
  const ConstBlockSet &joinBlocks(const VPBasicBlock &TermBlock);

  /// Computes divergent join points and loop exits (in the surrounding loop)
  /// caused by the divergent loop exits of\p Loop.
  ///
  /// The set of blocks which are reachable by disjoint paths from the
  /// loop exits of \p Loop.
  /// This treats the loop as a single node in \p Loop's parent loop.
  /// The returned set has the same properties as for join_blocks(TermInst&).
  const ConstBlockSet &joinBlocks(const VPLoop &Loop);

private:
  static ConstBlockSet EmptyBlockSet;

  ReversePostOrderTraversal<const VPBasicBlock *> RegRPOT;
  const VPDominatorTree &DT;
  const VPPostDominatorTree &PDT;
  const VPLoopInfo &LI;

  std::map<const VPLoop *, std::unique_ptr<ConstBlockSet>> CachedLoopExitJoins;
  std::map<const VPBasicBlock *, std::unique_ptr<ConstBlockSet>>
      CachedBranchJoins;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_SYNC_DEPENDENCE_ANALYSIS_H
#endif //INTEL_COLLAB
