//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANINSTRUCTION_DATA_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANINSTRUCTION_DATA_HIR_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"

namespace llvm {

// Forward declarations
namespace loopopt {
class HLNode;
}

namespace vpo {

struct SymbaseIVLevelMapInfo;

/// Class to hold the underlying HLNode of a master VPInstruction and its
/// validity. This information will also be accessible from the decomposed
/// VPInstructions of the master VPInstruction, if any.
class MasterVPInstData {
private:
  /// Contain the pointer to the underlying HLNode for a master VPInstruction
  /// and its validity flag. This flag states whether the underlying HLNode is
  /// still valid to be reused during codegen. This applies to the master
  /// VPInstruction holding a pointer to this objets and all the decomposed
  /// VPInstructions pointing to such master VPInstruction.
  PointerIntPair<loopopt::HLNode *, 1, bool> UnderlyingNode;

public:
  MasterVPInstData(loopopt::HLNode *Node) : UnderlyingNode(Node, false) {
    assert(Node && "MasterVPInstData must hold a valid HLNode.");
  }

  /// Return the underlying HLNode.
  loopopt::HLNode *getNode() { return UnderlyingNode.getPointer(); }

  /// Return true if the underlying HLNode is still valid.
  bool isValid() const { return UnderlyingNode.getInt(); }

  /// Set valididity flag to true.
  void setValid() { UnderlyingNode.setInt(true);}

  /// Set validity flag to false.
  void setInvalid() { UnderlyingNode.setInt(false);}
};

/// Base class to hold the underlying HIR information for blobs and induction
/// variables.
class VPOperandHIR {
private:
  /// Subclass identifier (for isa/dyn_cast).
  const unsigned char SubclassID;

protected:
  /// Enumeration to keep track of the concrete sub-classes of VPOperandHIR.
  enum { VPBlobSC, VPIndVarSC };

  VPOperandHIR(const unsigned char ID) : SubclassID(ID) {}

public:
  virtual ~VPOperandHIR() {}
  VPOperandHIR() = delete;
  VPOperandHIR(const VPOperandHIR &) = delete;
  VPOperandHIR &operator=(const VPOperandHIR &) = delete;

  /// Return true if this VPBlob is structurally equal to \p U.
  virtual bool isStructurallyEqual(const VPOperandHIR *U) const = 0;

  /// Method to support FoldingSet's hashing.
  virtual void Profile(FoldingSetNodeID &ID) const = 0;

  virtual void print(raw_ostream &OS) const = 0;

  unsigned char getSubclassID() const { return SubclassID; }
};

/// Class that holds underlying HIR information for unitaty blobs.
class VPBlob final : public VPOperandHIR {
private:
  // Hold DDRef for uninaty DDRef operand.
  const loopopt::DDRef *OperandBlob;

public:
  /// Construct a VPBlob with the DDRef \p Operand.
  VPBlob(const loopopt::DDRef *Operand)
      : VPOperandHIR(VPBlobSC), OperandBlob(Operand) {
    assert(!Operand->isMetadata() && "Unexpected metadata!");
  }

  /// Return true if this VPBlob is structurally equal to \p U.
  /// Structural comparision for VPBlobs checks if the symbase is the
  /// same.
  bool isStructurallyEqual(const VPOperandHIR *U) const override {
    const auto *UnitBlob = dyn_cast<VPBlob>(U);
    if (!UnitBlob)
      return false;

    return getBlob()->getSymbase() == UnitBlob->getBlob()->getSymbase();
  }

  // Return the DDRef of this VPBlob.
  const loopopt::DDRef *getBlob() const { return OperandBlob; }

  /// Method to support FoldingSet's hashing.
  void Profile(FoldingSetNodeID &ID) const override {
    ID.AddInteger(OperandBlob->getSymbase());
    ID.AddInteger(0 /*IVLevel*/);
  }

  void print(raw_ostream &OS) const override {
    formatted_raw_ostream FOS(OS);
    OperandBlob->print(FOS);
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPOperandHIR *U) {
    return U->getSubclassID() == VPBlobSC;
  }
};

/// Class that holds underlying HIR information for induction variables.
class VPIndVar final : public VPOperandHIR {
private:
  // Hold IV level for IV operand.
  unsigned IVLevel;

public:
  /// Construct an VPIndVar for an induction variable given its \p
  /// IVLevel.
  VPIndVar(unsigned IVLevel) : VPOperandHIR(VPIndVarSC), IVLevel(IVLevel) {}

  /// Return true if this VPIndVar is structurally equal to \p U.
  /// Structural comparision for VPIndVars checks if the IV level is the
  /// same.
  bool isStructurallyEqual(const VPOperandHIR *U) const override {
    const auto *UnitIV = dyn_cast<VPIndVar>(U);
    if (!UnitIV)
      return false;

    return getIVLevel() == UnitIV->getIVLevel();
  }

  /// Return the IV level of this underlying IV.
  unsigned getIVLevel() const { return IVLevel; }

  /// Method to support FoldingSet's hashing.
  void Profile(FoldingSetNodeID &ID) const override {
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(IVLevel);
  }

  void print(raw_ostream &OS) const override { OS << "%i" << IVLevel; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPOperandHIR *U) {
    return U->getSubclassID() == VPIndVarSC;
  }
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANINSTRUCTION_DATA_HIR_H
