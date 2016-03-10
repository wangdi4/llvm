//===----- RegDDRef.h - Regular data dependency node in HIR -----*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

class HLDDNode;

/// \brief Regular DDRef representing Values
///
/// Objects of this class represent temps and load/stores. Information to
/// regenerate GEP instruction associated with load/stores is maintained here.
///
class RegDDRef final : public DDRef {
public:
  /// loads/stores can be mapped as multi-dimensional subscripts with each
  /// subscript having its own canonical form.
  typedef SmallVector<CanonExpr *, 3> CanonExprsTy;
  typedef SmallVector<BlobDDRef *, 2> BlobDDRefsTy;
  typedef CanonExprsTy SubscriptTy;

  /// Iterators to iterate over canon exprs
  typedef CanonExprsTy::iterator canon_iterator;
  typedef CanonExprsTy::const_iterator const_canon_iterator;
  typedef CanonExprsTy::reverse_iterator reverse_canon_iterator;
  typedef CanonExprsTy::const_reverse_iterator const_reverse_canon_iterator;

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
  RegDDRef(unsigned SB);

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

  /// \brief Returns non-const iterator version of CBlobI.
  blob_iterator getNonConstBlobIterator(const_blob_iterator CBlobI);

  /// \brief Returns true if the Position is within the dimension range.
  bool isDimensionValid(unsigned Pos) const {
    return (Pos > 0 && Pos <= getNumDimensions());
  }

  /// \brief Implements getBase*Type() functionality.
  Type *getBaseTypeImpl(bool IsSrc) const;

  /// \brief Used by updateBlobDDRefs() to remove BlobDDRefs which are not
  /// needed anymore. The required blobs are passed in through BlobIndices. The
  /// function removes those blobs from BlobIndices whose BlobDDRef is already
  /// attached to RegDDRef.
  void removeStaleBlobDDRefs(SmallVectorImpl<unsigned> &BlobIndices);

  /// \brief Called by the verifier to check that the temp blobs contained in
  /// the DDRef correspond to blob DDRefs attached to the DDRef.
  void checkBlobDDRefsConsistency() const;

  /// \brief Implements get*Type() functionality.
  Type *getTypeImpl(bool IsSrc) const;

  /// \brief Returns maximum blob level amongst the blobs in the vector. If a
  /// non-linear blob is found, -1 is returned.
  int findMaxBlobLevel(const SmallVectorImpl<unsigned> &BlobIndices) const;

  /// \brief Updates def level of CE based on the level of the blobs present in
  /// CE. DDRef is assumed to have the passed in NestingLevel.
  void updateCEDefLevel(CanonExpr *CE, unsigned NestingLevel);

public:
  /// \brief Returns HLDDNode this DDRef is attached to.
  HLDDNode *getHLDDNode() const override { return Node; };

  /// \brief Prints RegDDRef.
  virtual void print(formatted_raw_ostream &OS,
                     bool Detailed = false) const override;

  /// \brief Returns true if the DDRef has GEP Info.
  bool hasGEPInfo() const { return (GepInfo != nullptr); }

  /// \brief Returns the src element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose src base type is [7 x
  /// [101 x float]]*, we will return float.
  /// TODO: extend to handle struct types.
  Type *getSrcType() const override;
  /// \brief Returns the dest element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose dest base type is [7 x
  /// [101 x int32]]*, we will return int32.
  /// TODO: extend to handle struct types.
  Type *getDestType() const override;

  /// \brief Returns the src type of the base CanonExpr for GEP DDRefs, returns
  /// null for non-GEP DDRefs.
  Type *getBaseSrcType() const;
  /// \brief Sets the src type of base CE of GEP DDRefs.
  void setBaseSrcType(Type *SrcTy);

  /// \brief Returns the dest type of the base CanonExpr for GEP DDRefs, returns
  /// null for non-GEP DDRefs.
  Type *getBaseDestType() const;
  /// \brief Sets the dest type of base CE of GEP DDRefs.
  void setBaseDestType(Type *DestTy);

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

  /// \brief Returns true if this RegDDRef is a constant integer.
  /// Val parameter is the value associated inside the CanonExpr
  /// of this RegDDRef
  bool isIntConstant(int64_t *Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isIntConstant(Val);
  }

  /// \brief Returns true if this RegDDRef represents an FP constant.
  /// Put the underlying LLVM Value in Val
  bool isFPConstant(ConstantFP **Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isFPConstant(Val);
  }

  /// \brief Returns true if this RegDDRef represents a vector of constants.
  /// Put the underlying LLVM Value in Val
  bool isConstantVector(Constant **Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isConstantVector(Val);
  }

  /// \brief Returns true if this RegDDRef represents a metadata.
  /// If true, metadata is returned in Val.
  bool isMetadata(MetadataAsValue **Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isMetadata(Val);
  }

  /// \brief Returns true if this RegDDRef represents null pointer.
  bool isNull() const {
    return isTerminalRef() && getSingleCanonExpr()->isNull();
  }

  /// \brief Returns true if this scalar RegDDRef's canonical expr is any kind
  /// of constant. Please note that this is different than the DDRef itself
  /// being a constant which is represented by setting the symbase to
  /// CONSTANT_SYMBASE. Lval DDRefs can have constant canonical expr but cannot
  /// have CONSTANT_SYMBASE.
  bool isConstant() const {
    return (isIntConstant() || isFPConstant() || isConstantVector() ||
            isNull() || isMetadata());
  }

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
    addDimension(CE);
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
  bool isTerminalRef() const;

  /// \brief Returns true if the DDRef is structurally invariant at \p Level.
  /// Note!: It does not check data-dependences, so there may be cases where
  /// the  DDRef is structurally invariant, but not actually invariant. For
  /// example, in the loop below, A[5] is structurally invariant, but not
  /// actually invariant because of the data-dependence:
  /// for (i=0; i<10; i++) { A[i] = A[5] + i;}
  bool isStructurallyInvariantAtLevel(unsigned Level) const;

  /// \brief Adds a dimension to the DDRef. Stride can be null for a scalar.
  void addDimension(CanonExpr *Canon, CanonExpr *Stride);

  /// \brief Returns true if the DDRef is a memory reference
  bool isMemRef() const;

  /// \brief Returns true if the DDRef represents a self-blob like (1 * %t). In
  /// addition DDRef's symbase should be the same as %t's symbase. This is so
  /// because for some livein copies %t1 = %t2, lval %t1 is parsed as 1 * %t2.
  /// But since %t1 has a different symbase than %t2 we still need to add a blob
  /// DDRef for %t2 to the DDRef.
  bool isSelfBlob() const override;

  /// \brief Returns true if this DDRef contains undefined canon expressions.
  bool containsUndef() const override;

  /// \brief Adds a dimension to the DDRef.
  void addDimension(CanonExpr *IndexCE);

  /// \brief Returns the stride in number of bytes for specified dimension.
  /// This is computed on the fly. DimensionNum must be within
  /// [1, getNumDimensions()].
  uint64_t getDimensionStride(unsigned DimensionNum) const;

  /// \brief Returns the canon expr (dimension) of this DDRef at specified
  /// position. DimensionNum must be within [1, getNumDimensions()].
  CanonExpr *getDimensionIndex(unsigned DimensionNum) {
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");
    return CanonExprs[DimensionNum - 1];
  }
  const CanonExpr *getDimensionIndex(unsigned DimensionNum) const {
    return const_cast<RegDDRef *>(this)->getDimensionIndex(DimensionNum);
  }

  /// \brief Returns the stride of this DDRef at specified loop level.
  /// Returns null if DDRef might not be a regular strided access
  /// (linear access with invariant stride at Level).
  CanonExpr *getStrideAtLevel(unsigned Level) const;

  /// \brief Removes a dimension from the DDRef. DimensionNum's range is
  /// [1, getNumDimensions()] with 1 representing the lowest dimension.
  void removeDimension(unsigned DimensionNum);

  /// \brief Returns the index of the blob represented by this self-blob DDRef.
  unsigned getSelfBlobIndex() const {
    assert(isSelfBlob() && "DDRef is not a self blob!");
    return getSingleCanonExpr()->getSingleBlobIndex();
  }

  /// \brief Adds a blob DDRef to this DDRef.
  void addBlobDDRef(BlobDDRef *BlobRef);

  /// \brief Creates a blob DDRef with passed in Index and Level and adds it to
  /// this DDRef. Level of -1 means non-linear blob.
  void addBlobDDRef(unsigned Index, int Level = -1);

  /// \brief Removes and returns blob DDRef corresponding to CBlobI iterator.
  BlobDDRef *removeBlobDDRef(const_blob_iterator CBlobI);

  /// \brief Removes all blob DDRefs attached to this DDRef.
  void removeAllBlobDDRefs();

  /// \brief Collects all the unique temp blobs present in the DDRef by visiting
  /// all the contained canon exprs.
  void collectTempBlobIndices(SmallVectorImpl<unsigned> &Indices) const;

  /// \brief Updates BlobDDRefs for this DDRef by going through the blobs in the
  /// associated canon exprs and populates NewBlobs with BlobDDRefs which have
  /// been added by the utility and whose defined at level needs to be updated.
  /// The utility will also remove BlobDDRefs associated with blobs which aren't
  /// present in the canon exprs anymore. It also sets the correct symbase for
  /// constant and self-blob DDRefs.
  ///
  /// NOTE: It is the responsibility of the user to call this utility after
  /// making changes to the DDRef and update defined at levels for the new
  /// blobs.
  void updateBlobDDRefs(SmallVectorImpl<BlobDDRef *> &NewBlobs,
                        bool AssumeLvalIfDetached = false);

  /// \brief Method to update CE def levels, if necessary. This should be called
  /// by transformations after they make any change to DDRef which affect the
  /// internal CE.
  /// for example:
  /// for(i=0; i<60; i++) {
  ///    a = A[i];
  ///    for(j=0; j<40; j++) {
  ///      b = A[j];
  ///      for(k=0; k<6; k++) {
  ///        A[k] = i + k*b;
  ///        A[2*k] = a + k*b;
  ///        A[3*k] = b;
  ///      }
  ///    }
  ///  }
  ///
  /// In this example all the rvals are marked as linear def @level 2. However,
  /// after complete unrolling of k-loop, in the first unrolled iteration when k
  /// is zero, the CE will be updated as follows-
  /// a) i + k*b (linear def@2) -> i (linear)
  /// b) a + k*b (linear def@2) -> a (linear def@1)
  /// c) b (linear def@2)       -> b (non-linear)
  ///
  /// It updates CE def level for attached blob DDRefs to non-linear as well, if
  /// applicable.
  ///
  /// NestingLevelIfDetached indicates the nesting level of this DDRef and is
  /// only meaningful for DDRefs not yet attached to HIR.
  ///
  /// NOTE: This utility cannot handle cases where blob definitions have been
  /// moved around or where non-linear blobs can be turned into linear blobs
  /// (during sinking, for example) because we do not track blob definitions.
  /// They require customized handling.
  void updateDefLevel(unsigned NestingLevelIfDetached = (MaxLoopNestLevel + 1));

  /// \brief Makes a modified ref internally consistent by updating blob DDRefs
  /// and containing CanonExprs' def level. The passed in AuxRefs should contain
  /// all the new blobs discovered in the DDRef or the function would assert.
  /// The blob DDRefs attached to these auxiliarry DDRefs are assumed to be in
  /// an updated state. If such DDRefs are not available the alternative is to
  /// call updateBlobDDRefs(), update the level of the new blobs manually and
  /// then call updateDefLevel().
  ///
  /// NestingLevelIfDetached indicates the nesting level of this DDRef and is
  /// only meaningful for DDRefs not yet attached to HIR.
  ///
  /// NOTE: This utility cannot handle cases where blob definitions have been
  /// moved around or where non-linear blobs can be turned into linear blobs
  /// (during sinking, for example) because we do not track blob definitions.
  /// These cases require customized handling. They can be partially handled
  /// using updateBlobDDRefs() and updateDefLevel() by manually updating blob
  /// levels.
  void
  makeConsistent(const SmallVectorImpl<const RegDDRef *> *AuxRefs = nullptr,
                 unsigned NestingLevelIfDetached = (MaxLoopNestLevel + 1));

  /// \brief Returns true if the blob is present in this DDRef and returns its
  /// defined at level via DefLevel. DefLevel is expected to be non-null. -1 is
  /// returned for non-linear blobs. The blob is searched in the blob DDRefs
  /// attached to this DDRef. This function can be used to update defined at
  /// levels for blobs which were copied from this DDRef to another DDRef.
  bool findBlobLevel(unsigned BlobIndex, int *DefLevel) const;

  /// \brief Verifies RegDDRef integrity.
  virtual void verify() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
