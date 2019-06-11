//===--------------------- IntelVPlanClone.h - Cloning functions for VPlan -==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCLONE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCLONE_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {

/// VPCloneUtils provides list of functions to clone HCFG.
/// NOTE: It DOES NOT update cloned instructions or create VPLoops. To do that
/// caller must use VPValueMapper.
class VPCloneUtils {
public:
  using Value2ValueMapTy = DenseMap<VPValue *, VPValue *>;
  using Block2BlockMapTy = DenseMap<VPBlockBase *, VPBlockBase *>;

  /// Generic function that accepts any \p Block and clones it.
  static VPBlockBase *cloneBlockBase(VPBlockBase *Block, std::string Prefix,
                                     Block2BlockMapTy &BlockMap,
                                     Value2ValueMapTy &ValueMap);

  /// Clone given VPBasicBlock \p Block.
  static VPBasicBlock *cloneBasicBlock(VPBasicBlock *Block, std::string Prefix,
                                       Value2ValueMapTy &ValueMap);

  /// Clone given VPLoopRegion \p LR.
  static VPLoopRegion *cloneLoopRegion(VPLoopRegion *LR, std::string Prefix,
                                       Block2BlockMapTy &BlockMap,
                                       Value2ValueMapTy &ValueMap);

  /// Clone given VPRegionBlocks \p Region.
  static VPRegionBlock *cloneRegion(VPRegionBlock *Region, std::string Prefix,
                                    Block2BlockMapTy &BlockMap,
                                    Value2ValueMapTy &ValueMap);
};

/// VPValueMapper is responsible to remap instructions within
/// VPlan/HCFG/VPInstruction.
/// This functionality is intentionally separated from cloning, because
/// attaching cloned HCFG is caller's responsibility, while it's impossible
/// to update VPLoopInfo/HCFG wihtout it.
class VPValueMapper {
private:
  VPCloneUtils::Block2BlockMapTy &Block2BlockMap;
  VPCloneUtils::Value2ValueMapTy &Value2ValueMap;

  bool AssertForNonCloned;

  void remapHCFG(VPBlockBase *Block);

  /// Recursively visit all VPRegionBlocks and VPBasicBlocks and create
  /// new VPLoop if BB has been cloned and in \p OriginalPlan same BB belongs
  /// to some VPLoop.
  void remapVPLoop(VPlan *Plan, const VPlan *OriginalPlan);

  /// As long as our VPBlockBase is not derived from VPValue, we cannot have
  /// single function to remap VPValue and VPBlockBase, like
  /// ValueMapper::remapValue.
  /// Thus templatization of remapValue and additional argument for a Map are
  /// necessary now.
  template <typename MapTy>
  typename MapTy::mapped_type remapValue(MapTy &Map,
                                         typename MapTy::key_type Value) {
    auto It = Map.find(Value);
    if (It != Map.end()) {
      return It->second;
    }
    if (std::is_same<typename MapTy::value_type, VPValue *>::value) {
      assert(!AssertForNonCloned &&
             "Either VPBB or VPInstruction was not cloned. Remapping is not "
             "possible.");
      return Value;
    }
    return Map[Value] = Value;
  }

  void updateLoopRegionsAfterCloning(VPlan *Plan);

  void cloneVPLoop(VPlan *Plan, const VPlan *OriginalPlan);
  bool entireLoopWasCloned(const VPLoop *Loop);

public:
  explicit VPValueMapper(VPCloneUtils::Block2BlockMapTy &BlockMap,
                         VPCloneUtils::Value2ValueMapTy &ValueMap,
                         const bool AssertForNonCloned = false)
      : Block2BlockMap(BlockMap), Value2ValueMap(ValueMap),
        AssertForNonCloned(AssertForNonCloned) {}

  /// Remap cloned VPBB and VPInstructions within \p Plan according to provided
  /// \p BlockMap and \p ValueMap.
  /// Has to pass \p OriginalPlan in order to update VPLoopInfo in \p Plan.
  ///
  /// Alternative solutions:
  ///  - maintain own version of the VPLoopInfo during cloning and merging it
  ///    with \p Plan's VPLoopInfo here doesn't work due to absence of such
  ///    functionality within LoopInfoBase.
  ///  - use VPLoopInfo->analyze() doesn't work because of issue with
  ///    GraphTraits<VPBlockBase *>, which doesn't visit Entry of VPRegionBlock,
  ///    which is essential to build dominator tree.
  void remapPlan(VPlan *Plan, const VPlan *OriginalPlan) {
    remapHCFG(Plan->getEntry());
    remapVPLoop(Plan, OriginalPlan);
  }

  void remapInstruction(VPInstruction *Inst);
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCLONE_H
