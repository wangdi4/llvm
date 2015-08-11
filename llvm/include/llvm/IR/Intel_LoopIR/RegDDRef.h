//===----- RegDDRef.h - Regular data dependency node in HIR -----*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the RegDDRef node in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_REGDDREF_H
#define LLVM_IR_INTEL_LOOPIR_REGDDREF_H

#include "llvm/Support/Casting.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"

namespace llvm {

namespace loopopt {

class CanonExpr;
class HLDDNode;

/// \brief Regular DDRef representing Values
///
/// Objects of this class represent temps and load/stores. Information to
/// regenerate GEP instruction associated with load/stores is maintained here.
///
class RegDDRef : public DDRef {
public:
  /// loads/stores can be mapped as multi-dimensional subscripts with each
  /// subscript
  /// having its own canonical form.
  typedef SmallVector<CanonExpr *, 3> CanonExprsTy;
  typedef SmallVector<BlobDDRef *, 2> BlobDDRefsTy;
  typedef CanonExprsTy SubscriptTy;
  typedef CanonExprsTy StrideTy;

  /// Iterators to iterate over canon exprs
  typedef CanonExprsTy::iterator canon_iterator;
  typedef CanonExprsTy::const_iterator const_canon_iterator;
  typedef CanonExprsTy::reverse_iterator reverse_canon_iterator;
  typedef CanonExprsTy::const_reverse_iterator const_reverse_canon_iterator;

  /// Iterators to iterate over stride exprs
  typedef canon_iterator stride_iterator;
  typedef const_canon_iterator const_stride_iterator;
  typedef reverse_canon_iterator reverse_stride_iterator;
  typedef const_reverse_canon_iterator const_reverse_stride_iterator;

  /// Iterators to iterate over blob ddrefs
  typedef BlobDDRefsTy::iterator blob_iterator;
  typedef BlobDDRefsTy::const_iterator const_blob_iterator;
  typedef BlobDDRefsTy::reverse_iterator reverse_blob_iterator;
  typedef BlobDDRefsTy::const_reverse_iterator const_reverse_blob_iterator;

private:
  /// \brief Contains extra information required to regenerate GEP instruction
  /// at code generation.
  struct GEPInfo {
    CanonExpr *BaseCE;
    // One for each dimension, corresponds to CanonExprs
    StrideTy Strides;
    bool InBounds;
    // This is set if this DDRef represents an address computation (GEP) instead
    // of a load or store.
    bool AddressOf;

    GEPInfo();
    ~GEPInfo();
  };

  /// Goes from lowest to highest dimension.
  /// Ex- A[CanonExpr3][CanonExpr2][CanonExpr1]
  CanonExprsTy CanonExprs;
  BlobDDRefsTy BlobDDRefs;
  GEPInfo *GepInfo;
  HLDDNode *Node;

protected:
  RegDDRef(int SB);

  /// Calling delete on a null pointer has no effect.
  virtual ~RegDDRef() override { delete GepInfo; }

  /// \brief Copy constructor used by cloning.
  RegDDRef(const RegDDRef &RegDDRefObj);

  friend class DDRefUtils;

  /// \brief Required to access setHLDDNode().
  friend class HLDDNode;

  /// \brief Sets the HLDDNode of this RegDDRef
  void setHLDDNode(HLDDNode *HNode) override { Node = HNode; }

  /// Non-const BlobDDRef iterator methods
  blob_iterator blob_begin() { return BlobDDRefs.begin(); }
  blob_iterator blob_end() { return BlobDDRefs.end(); }
  reverse_blob_iterator blob_rbegin() { return BlobDDRefs.rbegin(); }
  reverse_blob_iterator blob_rend() { return BlobDDRefs.rend(); }

  /// \brief Creates GEPInfo object for the DDRef.
  void createGEP() {
    assert(!GepInfo && "Attempt to overwrite GEP info!");
    GepInfo = new GEPInfo;
  }

  /// \brief Returns the stride associated with each dimension
  StrideTy &getStrides() { return GepInfo->Strides; }
  StrideTy &getStrides() const { return GepInfo->Strides; }

  /// \brief Returns non-const iterator version of CBlobI.
  blob_iterator getNonConstBlobIterator(const_blob_iterator CBlobI);

public:
  /// \brief Returns HLDDNode this DDRef is attached to.
  HLDDNode *getHLDDNode() const override { return Node; };

  /// \brief Prints RegDDRef.
  virtual void print(formatted_raw_ostream &OS,
                     bool Detailed = false) const override;

  /// TODO implementation
  /// Value *getLLVMValue() const override { return nullptr; }

  /// \brief Returns true if the DDRef has GEP Info.
  bool hasGEPInfo() const { return (GepInfo != nullptr); }

  /// \brief Returns the type of the base CanonExpr for GEP DDRefs, else returns
  /// null.
  Type *getBaseType() const;

  /// \brief Returns the canonical form of the subscript base.
  CanonExpr *getBaseCE() { return hasGEPInfo() ? GepInfo->BaseCE : nullptr; }
  const CanonExpr *getBaseCE() const {
    return const_cast<RegDDRef *>(this)->getBaseCE();
  }
  /// \brief Sets the canonical form of the subscript base.
  void setBaseCE(CanonExpr *BaseCE) {
    if (!hasGEPInfo()) {
      createGEP();
    }
    GepInfo->BaseCE = BaseCE;
  }

  /// \brief Returns true if the inbounds attribute is set for this access.
  bool isInBounds() const { return hasGEPInfo() ? GepInfo->InBounds : false; }
  /// Sets the inbounds attribute for this access.
  void setInBounds(bool IsInBounds) {
    if (!hasGEPInfo()) {
      createGEP();
    }
    GepInfo->InBounds = IsInBounds;
  }

  /// \brief Returns true if this ia an address computation.
  bool isAddressOf() const { return hasGEPInfo() ? GepInfo->AddressOf : false; }
  /// Marks this ref as an address computation.
  void setAddressOf(bool IsAddressOf) {
    if (!hasGEPInfo()) {
      createGEP();
    }
    GepInfo->AddressOf = IsAddressOf;
  }

  /// \brief Returns true if this RegDDRef is a constant
  /// Val parameter is the value associated inside the CanonExpr
  /// of this RegDDRef
  bool isIntConstant(int64_t *Val = nullptr) const;

  /// \brief Returns true if this RegDDRef is constant
  /// TODO: Add support for other types like float
  bool isConstant() const { return isIntConstant(nullptr); }

  /// \brief Returns the number of dimensions of the DDRef.
  unsigned getNumDimensions() const { return CanonExprs.size(); }

  /// \brief Returns the only canon expr of this DDRef.
  CanonExpr *getSingleCanonExpr() {
    assert(getNumDimensions() == 1);
    return *(canon_begin());
  }
  const CanonExpr *getSingleCanonExpr() const {
    return const_cast<RegDDRef *>(this)->getSingleCanonExpr();
  }

  /// \brief Returns true if this DDRef has only one canon expr.
  bool isSingleCanonExpr() const { return (getNumDimensions() == 1); }

  /// \brief Updates the only Canon Expr of this RegDDRef
  void setSingleCanonExpr(CanonExpr *CE) {
    assert((getNumDimensions() == 0) && " RegDDRef already has one or more "
                                        "CanonExprs");
    // TODO: Add replace dimension when available
    addDimension(CE, nullptr);
  }

  /// CanonExpr iterator methods
  canon_iterator canon_begin() { return CanonExprs.begin(); }
  const_canon_iterator canon_begin() const { return CanonExprs.begin(); }
  canon_iterator canon_end() { return CanonExprs.end(); }
  const_canon_iterator canon_end() const { return CanonExprs.end(); }

  reverse_canon_iterator canon_rbegin() { return CanonExprs.rbegin(); }
  const_reverse_canon_iterator canon_rbegin() const {
    return CanonExprs.rbegin();
  }
  reverse_canon_iterator canon_rend() { return CanonExprs.rend(); }
  const_reverse_canon_iterator canon_rend() const { return CanonExprs.rend(); }

  /// BlobDDRef iterator methods
  /// c-version allows use of "auto" keyword and doesn't conflict with protected
  /// non-const begin() / end().
  const_blob_iterator blob_cbegin() const { return BlobDDRefs.begin(); }
  const_blob_iterator blob_cend() const { return BlobDDRefs.end(); }

  const_reverse_blob_iterator blob_crbegin() const {
    return BlobDDRefs.rbegin();
  }
  const_reverse_blob_iterator blob_crend() const { return BlobDDRefs.rend(); }

  /// Stride iterator methods
  stride_iterator stride_begin() {
    if (hasGEPInfo()) {
      return getStrides().begin();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_end();
    }
  }
  const_stride_iterator stride_begin() const {
    if (hasGEPInfo()) {
      return getStrides().begin();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_end();
    }
  }
  stride_iterator stride_end() {
    if (hasGEPInfo()) {
      return getStrides().end();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_end();
    }
  }
  const_stride_iterator stride_end() const {
    if (hasGEPInfo()) {
      return getStrides().end();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_end();
    }
  }

  reverse_stride_iterator stride_rbegin() {
    if (hasGEPInfo()) {
      return getStrides().rbegin();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_rend();
    }
  }
  const_reverse_stride_iterator stride_rbegin() const {
    if (hasGEPInfo()) {
      return getStrides().rbegin();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_rend();
    }
  }
  reverse_stride_iterator stride_rend() {
    if (hasGEPInfo()) {
      return getStrides().rend();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_rend();
    }
  }
  const_reverse_stride_iterator stride_rend() const {
    if (hasGEPInfo()) {
      return getStrides().rend();
    } else {
      // Workaround to avoid forcing clients to check for GEPInfo before
      // accessing strides.
      return canon_rend();
    }
  }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef *Ref) {
    return Ref->getDDRefID() == DDRef::RegDDRefVal;
  }

  /// clone() - Create a copy of 'this' RegDDRef that is identical in all
  /// ways except the following:
  ///   * The HLDDNode needs to be explicitly set
  RegDDRef *clone() const override;

  /// \brief Returns true if this DDRef is a lval DDRef. This function
  /// assumes that the DDRef is connected to a HLDDNode.
  bool isLval() const;

  /// \brief Returns true if this DDRef is a rval DDRef. This function
  /// assumes that the DDRef is connected to a HLDDNode.
  bool isRval() const;

  /// \brief Returns true if this DDRef is a fake DDRef. This function
  /// assumes that the DDRef is connected to a HLDDNode.
  bool isFake() const;

  /// \brief This method checks if the DDRef is
  /// not a memory reference or a pointer reference
  /// Returns false for:
  ///      RegDDRef is Memory Reference - A[i]
  ///      RegDDRef is a Pointer Reference - *p
  /// Else returns true for cases like DDRef - 2*i and M+N.
  bool isScalarRef() const;

  /// \brief Adds a dimension to the DDRef. Stride can be null for a scalar.
  void addDimension(CanonExpr *Canon, CanonExpr *Stride);

  /// \brief Removes a dimension from the DDRef. DimensionNum's range is
  /// [1, getNumDimensions()] with 1 representing the lowest dimension.
  void removeDimension(unsigned DimensionNum);

  /// \brief Adds a blob DDRef to this DDRef.
  void addBlobDDRef(BlobDDRef *BlobRef);

  /// \brief Removes and returns blob DDRef corresponding to CBlobI iterator.
  BlobDDRef *removeBlobDDRef(const_blob_iterator CBlobI);

  /// \brief Updates BlobDDRefs for this DDRef by going through the blobs in
  /// the associated canon exprs. It will also remove BlobDDRefs associated
  /// with blobs which aren't present in the canon exprs anymore.
  /// It is the respoonsibility of the user to  call this function after
  /// making changes to the DDRef.
  void updateBlobDDRefs();
};

} // End namespace loopopt

} // End namespace llvm

#endif
