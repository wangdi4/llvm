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

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_INSTRUCTION_DATA_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_INSTRUCTION_DATA_H

namespace llvm { // LLVM Namespace

// Forward declarations
class Instruction;

namespace vpo {  // VPO Namespace

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
  enum { VPInstructionDataIRSC, VPInstructionDataHIRSC };

  unsigned getSCID() const { return SubclassID; }
};

// Class to hold LLVM-IR-specific information of a VPInstruction.
class VPInstructionDataIR : public VPInstructionData {
private:
  Instruction *Inst;

public:
  VPInstructionDataIR(Instruction *Inst)
      : VPInstructionData(VPInstructionDataIRSC), Inst(Inst) {}

  Instruction *getInstruction() { return Inst; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPInstructionData *V) {
    return V->getSCID() == VPInstructionDataIRSC;
  }
};

} // End VPO Namespace
} // End LLVM Namespace
#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_INSTRUCTION_DATA_H
