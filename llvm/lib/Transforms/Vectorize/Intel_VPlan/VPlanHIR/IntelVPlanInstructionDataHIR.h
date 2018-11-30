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

/// This class holds the underlying HIR information for unitary blobs and
/// induction variables. For unitary blobs, including metadata (for now), a
/// DDRef is held. For induction variables, the IV level is held.
///
/// DESIGN PRINCIPLE: A class hierarchy would make more sense here from a OOD
/// point of view. However, using a single class allows us to save a pointer in
/// VPExternalDef and use an UnitaryBlobOrIV object directly. This trade-off
/// should make sense as long as no more kinds are needed.
class UnitaryBlobOrIV {
  friend SymbaseIVLevelMapInfo;

private:
  // Hold DDRef for uninaty DDRef operand.
  loopopt::DDRef *UnitaryBlob;

  // Hold IV level for IV operand.
  unsigned IVLevel;

  void verifyState() const {
    assert((!UnitaryBlob ^ !IVLevel) && "Invalid state!");
  }

public:
  // Create an invalid UnitaryBlobOrIV (nullptr UnitaryBlob, level-0 IV)  when
  // no underlying HIR is passed. This implicit construction helps reduce
  // INTEL_CUSTOMIZATION in VPExternalDef constructors, which will make open
  // sourcing easier.
  UnitaryBlobOrIV() : UnitaryBlob(nullptr), IVLevel(0) {}

  /// Construct a UnitaryBlobOrIV for a unitary DDRef \p Unitary.
  UnitaryBlobOrIV(loopopt::DDRef *Unitary) : UnitaryBlob(Unitary), IVLevel(0) {
    assert(Unitary->isUnitaryBlob() &&
           "Expected BlobDDRef or unitary blob RegDDRef!");
  }

  /// Construct an UnitaryBlobOrIV for an induction variable given its \p
  /// IVLevel.
  UnitaryBlobOrIV(unsigned IVLevel) : UnitaryBlob(nullptr), IVLevel(IVLevel) {}

  ~UnitaryBlobOrIV() {}

  /// Return true if this UnitaryBlobOrIV is structurally equal to \p U.
  /// Structural comparison comprises:
  ///   a) Blobs: symbase comparison.
  ///   b) IVs: IVLevel comparison.
  ///   c) Metadata: Underlying MetadataAsValue comparison.
  bool isStructurallyEqual(const UnitaryBlobOrIV &U) const {
    if (isNonMDBlob() && U.isNonMDBlob())
      return getBlob()->getSymbase() == U.getBlob()->getSymbase();

    if (isIV() && U.isIV())
      return getIVLevel() == U.getIVLevel();

    if (isMDBlob() && U.isMDBlob()) {
      // All metadata as value has the same symbase in HIR so we compare that
      // the underlying LLVM-IR metadata is the same.
      const MetadataAsValue *MDLeft = getMetadata();
      const MetadataAsValue *MDRight = U.getMetadata();
      return MDLeft->getMetadata() == MDRight->getMetadata();
    }

    return false;
  }

  /// Return true if this UnitaryBlobOrIV is either a non-metadata or a metadata
  /// unitary blob.
  bool isBlob() const {
    verifyState();
    return UnitaryBlob != nullptr;
  }

  /// Return true if this UnitaryBlobOrIV is a non-metadata unitary blob.
  bool isNonMDBlob() const { return isBlob() && !UnitaryBlob->isMetadata(); }

  /// Return true if this UnitaryBlobOrIV is a metadata unitary blob.
  bool isMDBlob() const { return isBlob() && UnitaryBlob->isMetadata(); }

  /// Return true if this UnitaryBlobOrIV is an induction variable.
  bool isIV() const {
    verifyState();
    return IVLevel;
  }

  // Return the DDRef of this underlying unitary blob.
  const loopopt::DDRef *getBlob() const { return UnitaryBlob; }

  /// Return the IV level of this underlying IV.
  unsigned getIVLevel() const { return IVLevel; }

  const MetadataAsValue *getMetadata() const {
    assert(isMDBlob() && "Expected underlying metadata!");
    MetadataAsValue *MD = nullptr;
    UnitaryBlob->isMetadata(&MD);
    assert(MD && "Metadata not found!");
    return MD;
  }

  void print(raw_ostream &OS) const {
    if (isBlob()) {
      formatted_raw_ostream FOS(OS);
      UnitaryBlob->print(FOS);
    } else if (isIV())
      OS << "%i" << IVLevel;
    else
      llvm_unreachable("Unexpected UnitaryBlobOrIV kind!");
  }
};

/// MapInfo class necessary to use UnitaryBlobOrIV in DenseSet/DenseMap. This
/// implementation compares DDRefs based on their symbase. This means that two
/// different DDRef object with the same symbase will be considered equal (and
/// stored only once in the DenseSet/DenseMap.
struct SymbaseIVLevelMapInfo {
  static inline UnitaryBlobOrIV getEmptyKey() {
    // Empty key will be an IV with level UINT_MAX;
    return UnitaryBlobOrIV((unsigned)UINT_MAX);
  }

  static inline UnitaryBlobOrIV getTombstoneKey() {
    // Tombstone key will be an IV with level UINT_MAX -1;
    return UnitaryBlobOrIV((unsigned)UINT_MAX - 1);
  }

  // Following same approach as in DenseMapInfo<std::pair<T, U>> with
  // T = char, for enum, and unsigned for (symbase/IV level).
  static unsigned getHashValue(const UnitaryBlobOrIV &HIROperand) {
    unsigned Symbase =
        HIROperand.isBlob() ? HIROperand.getBlob()->getSymbase() : 0;
    unsigned IVLevel = HIROperand.getIVLevel();

    return DenseMapInfo<std::pair<unsigned, unsigned>>::getHashValue(
        std::make_pair(Symbase, IVLevel));
  }

  static bool isEqual(const UnitaryBlobOrIV &LHS, const UnitaryBlobOrIV &RHS) {
    return LHS.isStructurallyEqual(RHS);
  }
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANINSTRUCTION_DATA_HIR_H
