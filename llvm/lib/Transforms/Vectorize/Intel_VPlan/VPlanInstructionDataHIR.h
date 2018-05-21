//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   It defines MasterVPInstData, which is used to hold the underlying HIR
//   information of a master VPInstruction and its decomposed VPInstructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INSTRUCTION_DATA_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INSTRUCTION_DATA_HIR_H

namespace llvm {

// Forward declarations
namespace loopopt {
class HLDDNode;
}
namespace vpo {

/// Class to hold the underlying HLDDNode of a master VPInstruction and its
/// validity. This information will also be accessible from the decomposed
/// VPInstructions of the master VPInstruction, if any.
class MasterVPInstData {
private:
  /// Contain the pointer to the underlying HLDDNode for a master VPInstruction
  /// and its validity flag. This flag states whether the underlying HLDDNode is
  /// still valid to be reused during codegen. This applies to the master
  /// VPInstruction holding a pointer to this objets and all the decomposed
  /// VPInstructions pointing to such master VPInstruction.
  PointerIntPair<loopopt::HLDDNode *, 1, bool> UnderlyingDDN;

public:
  MasterVPInstData(loopopt::HLDDNode *DDN) : UnderlyingDDN(DDN, false) {
    assert(DDN && "MasterVPInstData must hold a valid HLDDNode.");
  }

  /// Return the underlying HLDDNode.
  loopopt::HLDDNode *getNode() { return UnderlyingDDN.getPointer(); }

  /// Return true if the underlying HLDDNode is still valid.
  bool isValid() const { return UnderlyingDDN.getInt(); }

  /// Set valididity flag to true.
  void setValid() { UnderlyingDDN.setInt(true);}

  /// Set validity flag to false.
  void setInvalid() { UnderlyingDDN.setInt(false);}
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INSTRUCTION_DATA_HIR_H
