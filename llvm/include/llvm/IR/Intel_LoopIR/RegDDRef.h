//===-------- RegDDRef.h - Data dependency node in HIR ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the RegDDRef node in high level IR.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_REGDDREF_H
#define LLVM_IR_INTEL_LOOPIR_REGDDREF_H

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/Support/Casting.h"

#include <vector>

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"

namespace llvm {

namespace loopopt {

class CanonExpr;

/// \brief Regular DDRef representing Values
///
/// Objects of this class represent temps and load/stores. Information to
/// regenerate GEP instruction associated with load/stores is maintained here.
///
class RegDDRef : public DDRef {
public:
  /// loads/stores can be mapped as multi-dimensional subscripts with each subscript
  /// having its own canonical form.
  typedef std::vector<CanonExpr*> CanonExprsTy;
  typedef std::vector<BlobDDRef*> BlobDDRefsTy;
  typedef CanonExprsTy SubscriptTy;
  typedef CanonExprsTy StrideTy;

private:

  /// \brief Contains extra information required to regenerate GEP instruction
  /// at code generation.
  struct GEPInfo {
    CanonExpr* BaseCE;
    StrideTy Strides;
    bool inbounds;
  };

  CanonExprsTy CanonExprs;
  BlobDDRefsTy BlobDDRefs;
  GEPInfo* GepInfo;
  HLNode* Node;
  void setGEP(CanonExpr* Base, StrideTy Stride, bool Inbounds) {
      //TODO destructord
        GepInfo = new GEPInfo;
        GepInfo->BaseCE = Base;
        GepInfo->Strides = Stride;
        GepInfo->inbounds = Inbounds;
  }

protected:
   RegDDRef(int SB, HLNode* HNode);
  ~RegDDRef() { }

  /// \brief Copy constructor used by cloning.
  RegDDRef(const RegDDRef &RegDDRefObj);

  friend class DDRefUtils;

  /// \brief Sets the HLNode of this RegDDRef
  void setHLNode(HLNode* HNode) override { 
    assert (!isa<HLRegion>(HNode) && "Cannot attach DDRef to a region!");
    Node = HNode; 
  }

public:

  /// \brief Returns HLNode this DDRef is attached to.
  HLNode* getHLNode() const override { return Node; };

  /// TODO implementation
  Value* getLLVMValue() const override { return nullptr; }

  /// \brief Returns the canonical exprs associated with this DDRef
  CanonExprsTy& getCanonExprs()             { return CanonExprs; }
  const CanonExprsTy& getCanonExprs() const { return CanonExprs; }

  /// \brief Returns the blob DDRefs associated with this DDRef
  BlobDDRefsTy& getBlobDDRefs()             { return BlobDDRefs; }
  const BlobDDRefsTy& getBlobDDRefs() const { return BlobDDRefs; }

  /// \brief Returns the stride associated with each dimension
  StrideTy* getStrides() { return &(GepInfo->Strides); }
  const StrideTy* getStrides() const;

  /// \brief Returns the canonical form of the subscript base
  CanonExpr* getBaseCE() {return GepInfo->BaseCE; }
  const CanonExpr* getBaseCE() const;

  /// \brief Returns true if the inbounds attribute is set for this access
  bool isInBounds() {return GepInfo->inbounds;}

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef* Ref) {
    return Ref->getDDRefID() == DDRef::RegDDRefVal;
  }

  /// clone() - Create a copy of 'this' RegDDRef that is identical in all
  /// ways except the following:
  ///   * The HLNode needs to be explicitly set
  RegDDRef* clone() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
