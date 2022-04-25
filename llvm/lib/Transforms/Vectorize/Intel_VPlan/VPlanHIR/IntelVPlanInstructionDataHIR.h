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
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

namespace llvm {

// Forward declarations
namespace loopopt {
class HLNode;
}

namespace vpo {

class VPInstruction;
class VPExternalDef;
class VPExternalUse;
class VPValue;
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
  enum { VPBlobSC, VPIndVarSC, VPCanonExprSC, VPIfCondSC };

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
    OS << " %vp" << (unsigned short)(unsigned long long)this << " = {";
    print(OS);
    OS << "}\n";
  }

  virtual StringRef getName() const { return ""; }
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

  StringRef getName() const override {
    if (!OperandBlob->isSelfBlob())
      return "";

    loopopt::BlobTy BlobScev =
        OperandBlob->getBlobUtils().getBlob(OperandBlob->getSelfBlobIndex());
    assert(BlobScev && OperandBlob->getBlobUtils().isTempBlob(BlobScev) &&
           "Unexpected blob scev");
    auto *UBlobScev = cast<SCEVUnknown>(BlobScev);
    return UBlobScev->getValue()->getName();
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
      FOS << "%vp" << (unsigned short)(unsigned long long)this;
  }

  void printDetail(raw_ostream &OS) const override {
    OS << " %vp" << (unsigned short)(unsigned long long)this << " = {";
    if (isUnitaryBlob())
      print(OS);
    else {
      getBlob()->getBlobUtils().getBlob(getBlobIndex())->print(OS);
    }
    OS << "}\n";
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
    OS << " %vp" << (unsigned short)(unsigned long long)this << " = {";
    getCanonExpr()->print(OS);
    OS << "}\n";
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPOperandHIR *U) {
    return U->getSubclassID() == VPCanonExprSC;
  }
};

class VPIfCond final : public VPOperandHIR {
private:
  const loopopt::HLIf *If;
public:
  VPIfCond(const loopopt::HLIf *If) : VPOperandHIR(VPIfCondSC), If(If) {
    assert(If && "Unexpected nullptr.");
  }
  bool isStructurallyEqual(const VPOperandHIR *U) const override {
    auto *Other = dyn_cast<VPIfCond>(U);
    return Other && Other->If == If;
  }
  void Profile(FoldingSetNodeID &ID) const override {
    ID.AddPointer(If);
    ID.AddInteger(0 /*Symbase*/);
    ID.AddInteger(0 /*IVLevel*/);
  }
  void print(raw_ostream &OS) const override {
    OS << "%vp" << (unsigned short)(unsigned long long)this;
  }

  void printDetail(raw_ostream &OS) const override {
    formatted_raw_ostream FOS(OS);
    FOS << " %vp" << (unsigned short)(unsigned long long)this << " = {";
    If->printHeader(FOS, 0);
    FOS << "}\n";
  }

  const loopopt::HLIf *getIf() const { return If; }

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPOperandHIR *U) {
    return U->getSubclassID() == VPIfCondSC;
  }
};

/// Hold all the HIR-specific data and interfaces for a VPInstruction.
class HIRSpecificsData {
  // VPInstruction needs access to the ctor.
  friend class VPInstruction;
  // This class only holds the data and hides it from everybody. The real
  // interfaces are in the thin wrapper HIRSpecifics because that's where we
  // have access to the owning VPInstruction without allocating needless space
  // for it.
  friend class HIRSpecifics;

  // Hold the underlying HIR information related to the LHS operand of this
  // VPInstruction.
  std::unique_ptr<VPOperandHIR> LHSHIROperand;

  // Union used to save needed information based on instruction opcode.
  // 1) For a load/store instruction, save the symbase of the corresponding
  //    scalar memref. Vector memref generated during vector CG is assigned
  //    the same symbase.
  // 2) For convert instructions, save whether the convert represents a
  //    convert of a loop IV that needs to be folded into the containing canon
  //    expression.
  union {
    unsigned Symbase = loopopt::InvalidSymbase;
    bool FoldIVConvert;
  };

  /// Pointer to access the underlying HIR data attached to this
  /// VPInstruction, if any, depending on its sub-type:
  ///   * Master VPInstruction: ExtraData points to a VPInstDataHIR holding
  ///     the actual HIR data.
  ///   * Decomposed VPInstruction: ExtraData points to master VPInstruction
  ///     holding the actual HIR data.
  ///   * VLSLoad/VLSStore: ExtraData contains fake symbases.
  ///   * Other VPInstruction: ExtraData is null. We use a void pointer to
  ///     represent this case.
  using FakeSymbases = SetVector<unsigned, SmallVector<unsigned, 4>>;
  PointerUnion<MasterVPInstData *, VPInstruction *, FakeSymbases *, void *>
      ExtraData = (int *)nullptr;

  HIRSpecificsData(const VPInstruction &Inst);

public:
  ~HIRSpecificsData() {
    if (ExtraData.is<MasterVPInstData *>())
      delete ExtraData.get<MasterVPInstData *>();
    if (ExtraData.is<FakeSymbases *>())
      delete ExtraData.get<FakeSymbases *>();
  }
};

/// We'd like to have access to the owning VPInstruction from the HIRSpecifics,
/// but the former isn't standard-layout so we can't use offsetof macro. Emulate
/// the same through a lightweight wrapper class with value semantics.
class HIRSpecifics {
  friend class VPInstruction;

  // Store as non-const reference, to be able to use the same class for both
  // const/non-const VPInstruction. Only the non-const methods of this class are
  // allowed to modify the HIRData inside the Inst though.
  VPInstruction &Inst;

private:
  HIRSpecifics(const VPInstruction &Inst);

  // VPInstruction is incomplete at this point, yet we have lots of one-liners
  // that would benefit from accessing it. Enable that by providing this helper.
  const HIRSpecificsData &HIRData() const;
  HIRSpecificsData &HIRData();

  /// Return true if the underlying HIR data is valid. If it's a decomposed
  /// VPInstruction, the HIR of the attached master VPInstruction is checked.
  bool isValid() const {
    if (isMaster() || isDecomposed())
      return getVPInstData()->isValid();

    return false;
  }

  /// Invalidate underlying HIR deta. If decomposed VPInstruction, the HIR of
  /// its master VPInstruction is invalidated.
  void invalidate() {
    if (isMaster() || isDecomposed())
      getVPInstData()->setInvalid();
  }

public:
  // Return the VPInstruction data of this VPInstruction if it's a master or
  // decomposed. Return nullptr otherwise.
  MasterVPInstData *getVPInstData();
  const MasterVPInstData *getVPInstData() const {
    return const_cast<HIRSpecifics *>(this)->getVPInstData();
  }

  /// Return true if this is a master VPInstruction.
  bool isMaster() const {
    return HIRData().ExtraData.is<MasterVPInstData *>();
  }

  /// Return true if this is a decomposed VPInstruction.
  bool isDecomposed() const {
    return HIRData().ExtraData.is<VPInstruction *>();
  }

  // Return true if MasterData contains actual HIR data.
  bool isSet() const {
    return !HIRData().ExtraData.is<void *>();
  }

  /// Return the underlying HIR attached to this master VPInstruction. Return
  /// nullptr if the VPInstruction doesn't have underlying HIR.
  loopopt::HLNode *getUnderlyingNode() {
    MasterVPInstData *MastData = getVPInstData();
    if (!MastData)
      return nullptr;
    return MastData->getNode();
  }
  loopopt::HLNode *getUnderlyingNode() const {
    return const_cast<HIRSpecifics *>(this)->getUnderlyingNode();
  }

  /// Attach \p UnderlyingNode to this VPInstruction and turn it into a master
  /// VPInstruction.
  void setUnderlyingNode(loopopt::HLNode *UnderlyingNode) {
    assert(!isSet() && "MasterData is already set!");
    HIRData().ExtraData = new MasterVPInstData(UnderlyingNode);
  }

  /// Attach \p Def to this VPInstruction as its VPOperandHIR.
  void setOperandDDR(const loopopt::DDRef *Def) {
    assert(!HIRData().LHSHIROperand && "LHSHIROperand is already set!");
    HIRData().LHSHIROperand.reset(new VPBlob(Def));
  }

  /// Attach \p IVLevel to this VPInstruction as its VPOperandHIR.
  void setOperandIV(unsigned IVLevel) {
    assert(!HIRData().LHSHIROperand && "LHSHIROperand is already set!");
    HIRData().LHSHIROperand.reset(new VPIndVar(IVLevel));
  }

  /// Return the VPOperandHIR with the underlying HIR information of the LHS
  /// operand.
  VPOperandHIR *getOperandHIR() const { return HIRData().LHSHIROperand.get(); }

  /// Return the master VPInstruction attached to a decomposed VPInstruction.
  VPInstruction *getMaster() {
    assert(isDecomposed() && "Only decomposed VPInstructions have a pointer "
                             "to a master VPInstruction!");
    return HIRData().ExtraData.get<VPInstruction *>();
  }
  VPInstruction *getMaster() const {
    return const_cast<HIRSpecifics *>(this)->getMaster();
  }

  /// Attach \p MasterVPI as master VPInstruction of a decomposed
  /// VPInstruction.
  void setMaster(VPInstruction *MasterVPI) {
    assert(MasterVPI && "Master VPInstruction cannot be set to null!");
    assert(!isMaster() &&
           "A master VPInstruction can't point to a master VPInstruction!");
    assert(!isSet() && "Master VPInstruction is already set!");
    HIRData().ExtraData = MasterVPI;
  }

  /// Mark the underlying HIR data as valid.
  void setValid() {
    assert(isMaster() && "Only a master VPInstruction must set HIR!");
    getVPInstData()->setValid();
  }

  /// Print HIR-specific flags. It's mainly for debugging purposes.
  void printHIRFlags(raw_ostream &OS) const {
    OS << "IsMaster=" << isMaster() << " IsDecomp=" << isDecomposed()
       << " IsNew=" << !isSet() << " HasValidHIR= " << isValid() << "\n";
  }

  void setSymbase(unsigned SB);
  unsigned getSymbase() const;

  void setFoldIVConvert(bool Fold);
  bool getFoldIVConvert() const;

  /// This method uniquifies symbases, i.e. if it's primary symbase it won't be
  /// added to the list of fakes ones. Fake symbases don't contain duplicates
  /// either.
  void addFakeSymbase(unsigned Symbase);
  ArrayRef<unsigned> fakeSymbases() const;

  void cloneFrom(const HIRSpecifics HIR);
};

// Wrapper for underlying HIR data of VPExternalDef and VPExternalUse.
class HIROperandSpecifics {
  PointerUnion<const VPExternalUse *, const VPExternalDef *> ExtObj;

public:
  HIROperandSpecifics(const VPExternalUse *ExtO) : ExtObj(ExtO) {}
  HIROperandSpecifics(const VPExternalDef *ExtO) : ExtObj(ExtO) {}

  unsigned getSymbase() const {
    const VPOperandHIR *Operand = getOperand();
    if (Operand && isa<VPBlob>(Operand))
      return cast<VPBlob>(Operand)->getBlob()->getSymbase();
    return loopopt::InvalidSymbase;
  }

private:
  const VPOperandHIR *getOperand() const;
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANINSTRUCTION_DATA_HIR_H
