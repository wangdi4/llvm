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

  /// Clone given VPBasicBlock \p Block inside the current VPlan or in a
  /// NewVPlan (if NewVPlan!=nullptr).
  static VPBasicBlock *cloneBasicBlock(VPBasicBlock *Block, const Twine &Prefix,
                                       Value2ValueMapTy &ValueMap,
                                       VPlan::iterator InsertBefore,
                                       VPlanDivergenceAnalysis *DA = nullptr,
                                       VPlan *NewVPlan = nullptr);

  /// Clone given blocks from Begin to End inside the current Plan if
  /// NewVPlan=nullptr. Otherwise, it clones the basic blocks from Begin to End
  /// and adds them to a new VPlan.
  static VPBasicBlock *cloneBlocksRange(VPBasicBlock *Begin, VPBasicBlock *End,
                                        Value2ValueMapTy &ValueMap,
                                        VPlanDivergenceAnalysis *DA = nullptr,
                                        const Twine &Prefix = "cloned.",
                                        VPlan *NewVPlan = nullptr);
};

/// VPValueMapper is responsible to remap instructions within
/// VPlan/HCFG/VPInstruction.
/// This functionality is intentionally separated from cloning, because
/// attaching cloned HCFG is caller's responsibility, while it's impossible
/// to update VPLoopInfo/HCFG wihtout it.
class VPValueMapper {
private:
  VPCloneUtils::Value2ValueMapTy &Value2ValueMap;

  bool AssertForNonCloned;

  VPValue *remapValue(VPCloneUtils::Value2ValueMapTy &Map, VPValue *Value) {
    auto It = Map.find(Value);
    if (It != Map.end()) {
      return It->second;
    }
    assert(!AssertForNonCloned &&
           "Either VPBB or VPInstruction was not cloned. Remapping is not "
           "possible.");

    if (dyn_cast<VPInstruction>(Value))
      Map[Value] = Value;

    return Value;
  }

public:
  explicit VPValueMapper(VPCloneUtils::Value2ValueMapTy &ValueMap,
                         const bool AssertForNonCloned = false)
      : Value2ValueMap(ValueMap), AssertForNonCloned(AssertForNonCloned) {}

  void remapInstruction(VPInstruction *Inst);

  // Updates the operands of cloned instructions and basic blocks by replacing
  // the original values with the cloned ones.
  void remapOperands(VPBasicBlock *OrigVPBB,
                     std::function<void(VPInstruction &)> UpdateFunc =
                         [](VPInstruction &VPInst) {});
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCLONE_H
