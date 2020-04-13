//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/FoldingSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/ScalarEvolution.h"

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
  enum { VPBlobSC, VPIndVarSC, VPCanonExprSC };

  VPOperandHIR(const unsigned char ID) : SubclassID(ID) {}

public:
  virtual ~VPOperandHIR() {}
  VPOperandHIR() = delete;
  VPOperandHIR(const VPOperandHIR &) = delete;
  VPOperandHIR &operator=(const VPOperandHIR &) = delete;

  /// Return true if this VPBlob is structurally equal to \p U.
  virtual bool isStructurallyEqual(const VPOperandHIR *U) const = 0;

  /// Return true if this operand is equal to CE.
  virtual bool isEqual(const loopopt::CanonExpr *CE) const { return false; }

  /// Method to support FoldingSet's hashing.
  virtual void Profile(FoldingSetNodeID &ID) const = 0;

  virtual void print(raw_ostream &OS) const = 0;
  virtual void printDetail(raw_ostream &OS) const {
    formatted_raw_ostream FOS(OS);
    FOS << " %vp" << (unsigned short)(unsigned long long)this << " = ";
    print(OS);
    FOS << "\n";
  }

  unsigned char getSubclassID() const { return SubclassID; }
};

/// Class used to hold underlying HIR information for either unitary blobs or
/// arbitrary blobs that are invariant at the loop level being vectorized.
class VPBlob final : public VPOperandHIR {
private:
  // Holds either DDRef for unitary DDRef operand or the RegDDRef of which
  // the blob indicated by BlobIndex is part of.
  const loopopt::DDRef *OperandBlob;

  // Set to InvalidBlobIndex for unitary blobs. Otherwise, stores the blob
  // index of the invariant blob in DDR.
  unsigned BlobIndex;

public:
  /// Construct a VPBlob for the DDRef \p Operand or the invariant blob
  /// specified by index \p BlobIndex in \p Operand.
  VPBlob(const loopopt::DDRef *Operand,
         unsigned BlobIndex = loopopt::InvalidBlobIndex)
      : VPOperandHIR(VPBlobSC), OperandBlob(Operand), BlobIndex(BlobIndex) {
    assert(!Operand->isMetadata() && "Unexpected metadata!");
    assert((BlobIndex == loopopt::InvalidBlobIndex ||
            isa<loopopt::RegDDRef>(Operand)) &&
           "Expected a RegDDRef for valid blob index");
  }

  /// Return true if the blob is unitary.
  bool isUnitaryBlob() const { return BlobIndex == loopopt::InvalidBlobIndex; }

  /// Return true if this VPBlob is structurally equal to \p U.
  bool isStructurallyEqual(const VPOperandHIR *U) const override {
    const auto *UBlob = dyn_cast<VPBlob>(U);
    if (!UBlob)
      return false;

    // For unitary blobs equality is determined using symbase.
    if (UBlob->isUnitaryBlob())
      return isUnitaryBlob() &&
             getBlob()->getSymbase() == UBlob->getBlob()->getSymbase();

    // For non-unitary blobs, equality is determined by BlobIndex.
    if (BlobIndex == UBlob->getBlobIndex()) {
      assert(UBlob->getBlob()->getBlobUtils().getBlob(UBlob->getBlobIndex()) ==
                 getBlob()->getBlobUtils().getBlob(getBlobIndex()) &&
             "Expected blobs to be equal");
      return true;
    }

    return false;
  }

  // Return the DDRef of this VPBlob.
  const loopopt::DDRef *getBlob() const {
    assert(OperandBlob && "Unexpected null OperandBlob");
    return OperandBlob;
  }

  unsigned getBlobIndex() const { return BlobIndex; }

  /// Method to support FoldingSet's hashing.
  void Profile(FoldingSetNodeID &ID) const override {
    // For unitary blobs, symbase is the key. Otherwise, the SCEVExpr
    // corresponding to BlobIndex is the key.
    if (isUnitaryBlob()) {
      ID.AddPointer(nullptr);
      ID.AddInteger(getBlob()->getSymbase());
    } else {
      ID.AddPointer(getBlob()->getBlobUtils().getBlob(BlobIndex));
      ID.AddInteger(loopopt::InvalidSymbase /*Symbase*/);
    }

    ID.AddInteger(0 /*IVLevel*/);
  }

  void print(raw_ostream &OS) const override {
    formatted_raw_ostream FOS(OS);
    if (isUnitaryBlob())
      getBlob()->print(FOS);
    else
      OS << "%vp" << (unsigned short)(unsigned long long)this;
  }

  void printDetail(raw_ostream &OS) const override {
    formatted_raw_ostream FOS(OS);
    FOS << " %vp" << (unsigned short)(unsigned long long)this << " = ";
    if (isUnitaryBlob())
      print(OS);
    else {
      getBlob()->getBlobUtils().getBlob(getBlobIndex())->print(OS);
    }
    FOS << "\n";
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
    ID.AddPointer(nullptr);
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(IVLevel);
  }

  void print(raw_ostream &OS) const override { OS << "%i" << IVLevel; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPOperandHIR *U) {
    return U->getSubclassID() == VPIndVarSC;
  }
};

/// Class that holds underlying HIR information for invariant canon expressions.
class VPCanonExpr final : public VPOperandHIR {
private:
  // Hold invariant canon expression of the invariant operand.
  const loopopt::CanonExpr *OperandCE;

  // Hold the DDRef that this canon expression is part of. This is used
  // at code generation time to make the generated widened ref consistent
  // with appropriate defined at level information.
  const loopopt::RegDDRef *DDR;

public:
  /// Construct a VPCanonExpr for the given CanonExpr. DDR points to the DDRef
  /// containing this canon expression.
  VPCanonExpr(const loopopt::CanonExpr *CE, const loopopt::RegDDRef *DDR)
      : VPOperandHIR(VPCanonExprSC), OperandCE(CE), DDR(DDR) {}

  /// Return true if this VPCanonExpr is structurally equal to \p U.
  /// Structural comparision for VPCanonExprs checks if the underlying canon \
  /// expressions are equal.
  bool isStructurallyEqual(const VPOperandHIR *U) const override {
    const auto *UnitCE = dyn_cast<VPCanonExpr>(U);
    if (!UnitCE)
      return false;

    return isEqual(UnitCE->getCanonExpr());
  }

  bool isEqual(const loopopt::CanonExpr *CE) const override {
    return CE->getCanonExprUtils().areEqual(CE, getCanonExpr());
  }

  const loopopt::CanonExpr *getCanonExpr() const {
    assert(OperandCE && "Unexpected null canon expression in VPCanonExpr");
    return OperandCE;
  }
  const loopopt::RegDDRef *getDDR() const {
    assert(DDR && "Unexpected null DDREF in VPCanonExpr");
    return DDR;
  }

  /// Method to support FoldingSet's hashing.
  void Profile(FoldingSetNodeID &ID) const override {
    ID.AddPointer(getCanonExpr());
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(0 /*IVLevel*/);
  }

  void print(raw_ostream &OS) const override {
    OS << "%vp" << (unsigned short)(unsigned long long)this;
  }

  void printDetail(raw_ostream &OS) const override {
    formatted_raw_ostream FOS(OS);
    FOS << " %vp" << (unsigned short)(unsigned long long)this << " = ";
    getCanonExpr()->print(FOS);
    FOS << "\n";
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPOperandHIR *U) {
    return U->getSubclassID() == VPCanonExprSC;
  }
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANINSTRUCTION_DATA_HIR_H
