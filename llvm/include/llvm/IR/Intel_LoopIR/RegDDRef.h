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
#include "llvm/Support/Casting.h"
#include <vector>
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
  typedef std::vector<CanonExpr *> CanonExprsTy;
  typedef std::vector<BlobDDRef *> BlobDDRefsTy;
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
    /// One for each dimension, corresponds to CanonExprs
    StrideTy Strides;
    bool InBounds;

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
  ~RegDDRef() { delete GepInfo; }

  /// \brief Copy constructor used by cloning.
  RegDDRef(const RegDDRef &RegDDRefObj);

  friend class DDRefUtils;

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

public:
  /// \brief Returns HLDDNode this DDRef is attached to.
  HLDDNode *getHLDDNode() const override { return Node; };

  /// TODO implementation
  Value *getLLVMValue() const override { return nullptr; }

  /// \brief Returns true if the DDRef has GEP Info.
  bool hasGEPInfo() const { return (GepInfo != nullptr); }

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

  /// \brief Returns the number of dimensions of the DDRef.
  unsigned getNumDimensions() { return CanonExprs.size(); }

  /// \brief Returns the only canon expr of this DDRef.
  CanonExpr *getSingleCanonExpr() {
    assert(getNumDimensions() == 1);
    return *(canon_begin());
  }
  const CanonExpr *getSingleCanonExpr() const {
    return const_cast<RegDDRef *>(this)->getSingleCanonExpr();
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
  const_blob_iterator blob_cbegin() const { return BlobDDRefs.cbegin(); }
  const_blob_iterator blob_cend() const { return BlobDDRefs.cend(); }

  const_reverse_blob_iterator blob_crbegin() const {
    return BlobDDRefs.crbegin();
  }
  const_reverse_blob_iterator blob_crend() const { return BlobDDRefs.crend(); }

  /// Stride iterator methods
  stride_iterator stride_begin() {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().begin();
  }
  const_stride_iterator stride_begin() const {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().begin();
  }
  stride_iterator stride_end() {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().end();
  }
  const_stride_iterator stride_end() const {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().end();
  }

  reverse_stride_iterator stride_rbegin() {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().rbegin();
  }
  const_reverse_stride_iterator stride_rbegin() const {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().rbegin();
  }
  reverse_stride_iterator stride_rend() {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().rend();
  }
  const_reverse_stride_iterator stride_rend() const {
    assert(hasGEPInfo() && "DDRef does not have stride!");
    return getStrides().rend();
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

  /// \brief Adds a dimension to the DDRef. Stride can be null for a scalar.
  void addDimension(CanonExpr *Canon, CanonExpr *Stride);

  /// \brief Removes a dimension from the DDRef. DimensionNum's range is
  /// [1, getNumDimensions()] with 1 representing the lowest dimension.
  void removeDimension(unsigned DimensionNum);

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
