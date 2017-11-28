//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   It defines the class hierarchy that is used to hold underlying IR information
//   for VPInstructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INSTRUCTION_DATA_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INSTRUCTION_DATA_H

namespace llvm {

// Forward declarations
namespace loopopt {
class HLDDNode;
}

namespace vpo {

// Base class
class VPInstructionData {
private:
  const unsigned char SubclassID; ///< Subclass identifier (for isa/dyn_cast).

protected:
  VPInstructionData(unsigned char SC) : SubclassID(SC) {}
  virtual ~VPInstructionData() {}

public:
  /// An enumeration for keeping track of the concrete subclass of
  /// VPInstructionData that are actually instantiated. Values of this
  /// enumeration are kept in the SubclassID field of the VPInstructionData
  /// objects. They are used for concrete type identification.
  enum { VPInstructionDataHIRSC };

  unsigned getSCID() const { return SubclassID; }
};


using namespace loopopt;

// Class to hold HIR-specific information of a VPInstruction.
class VPInstructionDataHIR : public VPInstructionData {
private:
  HLDDNode *DDNode;

public:
  VPInstructionDataHIR(HLDDNode *DDNode)
      : VPInstructionData(VPInstructionDataHIRSC), DDNode(DDNode) {}

  HLDDNode *getInstruction() { return DDNode; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstructionData *V) {
    return V->getSCID() == VPInstructionDataHIRSC;
  }
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INSTRUCTION_DATA_H
