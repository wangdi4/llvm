//===-- VPlanAllZeroBypass.h ------------------------------------*- C++ -*-===//
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

class VPlanAllZeroBypass {

public:
  // BypassPairTy indicates a begin/end set of basic blocks for which to form
  // an all-zero bypass around. This data structure reflects the blocks forming
  // the region before the bypass insertion.
  using BypassPairTy = std::pair<VPBasicBlock *, VPBasicBlock *>;
  using AllZeroBypassRegionsTy = SmallVector<BypassPairTy, 8>;

  // Keeps track of regions formed so as to not introduce unnecessary ones.
  // This maps reflects the regions after the bypass transformation.
  using RegionsInsertedTy = std::map<VPValue *, AllZeroBypassRegionsTy>;

private:
  // VPlan for bypass insertion.
  VPlan &Plan;

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
                       SmallPtrSetImpl<VPBasicBlock *> &RegionBlocks);

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
                             VPLoopInfo *VPLI);

  /// Assert if the structure of the bypass is not well formed.
  void verifyBypassRegion(BypassPairTy &Region);

public:
  VPlanAllZeroBypass(VPlan &Plan) : Plan(Plan) {};

  /// Collect regions of blocks that are safe/profitable for all-zero bypass
  /// insertion.
  void collectAllZeroBypassRegions(
      AllZeroBypassRegionsTy &AllZeroBypassRegions);

  /// Iterate over blocks previously collected and insert an all-zero bypass
  /// around those blocks. This is also done for loops with divergent loop
  /// entry.
  void insertAllZeroBypasses(AllZeroBypassRegionsTy &AllZeroBypassRegion);
};
} // end namespace vpo
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_ALLZEROBYPASS_H
