//===-- VPlanAllZeroBypass.h ------------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VPlanAllZeroBypass class which contains the public
/// interfaces to insert all-zero bypasses around divergent block regions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_ALLZEROBYPASS_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_ALLZEROBYPASS_H

#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/IR/Dominators.h"

using namespace llvm::vpo;

namespace llvm {
namespace vpo {

class VPlanCostModelInterface;

class VPlanAllZeroBypass {

public:
  // BypassPairTy is used for both pre-insertion and post-insertion to indicate
  // the begin/end blocks of bypass regions. For region pre-insertion, it
  // corresponds to the first/last blocks that are contained on the edge where
  // the bypass is not taken. During insertion, a separate data structure is
  // used to mark the new begin/end blocks of the region now forming the bypass.
  // These blocks are used later at the end of region insertion to make sure
  // valid regions are formed. E.g., assert that we have single entry, single
  // exit regions formed.
  using BypassPairTy = std::pair<VPBasicBlock *, VPBasicBlock *>;
  using AllZeroBypassRegionsTy = SmallVector<BypassPairTy, 8>;

  // Keeps track of regions formed so as to not introduce unnecessary ones.
  // This maps reflects the regions after the bypass transformation. Use a
  // multimap because it could be possible that multiple regions are formed
  // using the same block-predicate.
  using RegionsCollectedTy =
      std::multimap<VPValue *, SetVector<VPBasicBlock *>>;

private:
  // VPlan for bypass insertion.
  VPlanVector &Plan;

  // VPlan builder used to generate VPInstructions for !all-zero checks.
  VPBuilder Builder;

  using LiveOutUsersTy = SmallVector<VPUser *, 4>;
  using LiveOutUsersMapTy = MapVector<VPValue *, LiveOutUsersTy>;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Dumps live-out information for the region specified by
  /// \p FirstBlockInBypassRegion and \p LastBlockInBypassRegion.
  void dumpBypassRegionLiveOuts(VPBasicBlock *FirstBlockInBypassRegion,
                                VPBasicBlock *LastBlockInBypassRegion,
                                LiveOutUsersMapTy &LiveOutMap);
#endif

  /// Computes the set of blocks that forms the all-zero bypass region.
  void getRegionBlocks(VPBasicBlock *FirstBlockInBypassRegion,
                       VPBasicBlock *LastBlockInBypassRegion,
                       SetVector<VPBasicBlock *> &RegionBlocks);

  /// Records a live-out value \p LiveOutVal and its corresponding user
  /// \p LiveOutUser that is outside of the region specified by the caller.
  void addLiveOutValue(VPValue *LiveOutVal, VPUser *LiveOutUser,
                       LiveOutUsersMapTy &LiveOutMap);

  /// Computes the VPValues that are live-out of the region specified by
  /// \p BlockPair and records their corresponding users outside of this region.
  void collectRegionLiveOuts(VPBasicBlock *FirstBlockInBypassRegion,
                             VPBasicBlock *LastBlockInBypassRegion,
                             LiveOutUsersMapTy &LiveOutMap);

  /// Creates a new non-blend phi for VPValues that are live-out of the all
  /// zero bypass region and replaces all users of the original value with
  /// this new phi. Incoming blocks for phi operands will include VPValues
  /// from \p LastBlockInBypassRegion and \p BypassBegin, where \p BypassBegin
  /// will have 0/false as the incoming value.
  void createLiveOutPhisAndReplaceUsers(VPBasicBlock *LastBlockInBypassRegion,
                                        VPBasicBlock *BypassBegin,
                                        VPBasicBlock *BypassEnd,
                                        LiveOutUsersMapTy &LiveOutMap);

  /// Inserts an all-zero bypass around the region specified by the first
  /// and last blocks of the region, \p FirstBlockInBypassRegion and
  /// \p LastBlockInBypassRegion, respectively.
  void insertBypassForRegion(VPBasicBlock *FirstBlockInBypassRegion,
                             VPBasicBlock *LastBlockInBypassRegion,
                             VPDominatorTree *DT,
                             VPPostDominatorTree *PDT,
                             VPLoopInfo *VPLI,
                             AllZeroBypassRegionsTy &AllZeroBypassRegionsFinal);

  /// Assert if the structure of the bypass is not well formed.
  void verifyBypassRegion(VPBasicBlock *FirstBlockInRegion,
                          SetVector<VPBasicBlock *> &RegionBlocks);

  /// Returns true if \p Block has already been included within a region
  /// under block-predicate \p Pred.
  bool regionFoundForBlock(VPBasicBlock *Block, VPValue *Pred,
                           RegionsCollectedTy &RegionsCollected);

  /// Returns true if \p MaybePred is an anded condition of \p BaseCond.
  bool isStricterOrEqualPred(const VPValue *MaybePred, const VPValue *BaseCond);

  /// Returns true if the incoming predicates of \p Blend are not under the
  /// influence of the same block-predicate as the region.
  bool blendTerminatesRegion(const VPBlendInst *Blend, VPValue *RegionPred);

  /// Returns true if the loop or non-loop region should end at \p Block.
  /// The region will not include \p Block.
  bool endRegionAtBlock(VPBasicBlock *Block, VPValue *CandidateBlockPred,
                        SetVector<VPBasicBlock *> &RegionBlocks);

public:
  VPlanAllZeroBypass(VPlanVector &Plan) : Plan(Plan){};

  /// Collect regions of blocks that are safe/profitable for all-zero bypass
  /// insertion for non-loops.
  void collectAllZeroBypassNonLoopRegions(
      AllZeroBypassRegionsTy &AllZeroBypassRegions,
      RegionsCollectedTy &RegionsCollected,
      VPlanCostModelInterface *CM = nullptr, Optional<unsigned> VF = None);

  /// Collect regions of blocks that are safe/profitable for all-zero bypass
  /// insertion for loops.
  void collectAllZeroBypassLoopRegions(
      AllZeroBypassRegionsTy &AllZeroBypassRegions,
      RegionsCollectedTy &RegionsCollected);

  /// Iterate over blocks previously collected and insert an all-zero bypass
  /// around those blocks. This is also done for loops with divergent loop
  /// entry.
  void insertAllZeroBypasses(AllZeroBypassRegionsTy &AllZeroBypassRegions);
};
} // end namespace vpo
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_ALLZEROBYPASS_H
