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

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/DDRef.h"

namespace llvm {

class MDNode;
struct AAMDNodes;

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
  typedef std::pair<unsigned, MDNode *> MDPairTy;
  typedef SmallVector<MDPairTy, 6> MDNodesTy;

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
  typedef SmallVector<unsigned, 2> OffsetsTy;

  /// \brief Contains extra information required to regenerate GEP instruction
  /// at code generation.
  struct GEPInfo {
    CanonExpr *BaseCE;
    bool InBounds;
    // This is set if this DDRef represents an address computation (GEP) instead
    // of a load or store.
    bool AddressOf;
    bool Volatile;
    unsigned Alignment;

    // Stores trailing structure element offsets for each dimension of the ref.
    // Consider the following structure GEP as an example-
    //
    // %struct.S = type { i32 }
    // @arr = [100 x %struct.S]
    // %t = GEP @arr, 0, i, 0
    //
    // The dimensions of this ref are as follows-
    // [0][i]
    //
    // The trailing offsets are stored as follows-
    // [[], [0]]
    //
    // The higher dimension has no trailing offsets. Lower dimension has a
    // single trailing offset of 0.
    SmallVector<OffsetsTy, 3> DimensionOffsets;

    // TODO: Atomic attribute is missing. Should we even build regions with
    // atomic load/stores since optimizing multi-threaded code might be
    // dangerous? IRBuilder doesn't even seem to have members to create atomic
    // load/stores.

    // Stores metadata associated with load/stores in sorted order by KindID.
    // This is the same setup as for LLVM instructions. Refer to
    // getAllMetadata() in Instruction.h.
    MDNodesTy MDNodes;

    // Debug location of the GEP instruction.
    DebugLoc GepDbgLoc;

    // Debug location of the load/store instruction.
    DebugLoc MemDbgLoc;

    // Comparators to sort MDNodes.
    struct MDKindCompareLess;
    struct MDKindCompareEqual;

    GEPInfo();
    GEPInfo(const GEPInfo &);

    ~GEPInfo();
  };

  /// Goes from lowest to highest dimension.
  /// Ex- A[CanonExpr3][CanonExpr2][CanonExpr1]
  CanonExprsTy CanonExprs;
  BlobDDRefsTy BlobDDRefs;
  GEPInfo *GepInfo;
  HLDDNode *Node;

protected:
  RegDDRef(DDRefUtils &DDRU, unsigned SB);

  /// Calling delete on a null pointer has no effect.
  virtual ~RegDDRef() override { delete GepInfo; }

  /// \brief Copy constructor used by cloning.
  RegDDRef(const RegDDRef &RegDDRefObj);

  friend class DDRefUtils;

  // Required to access setHLDDNode().
  friend class HLDDNode;

  // Accesses MDNodes.
  friend class HIRParser;

  /// \brief Sets the HLDDNode of this RegDDRef
  void setHLDDNode(HLDDNode *HNode) override { Node = HNode; }

  /// Non-const BlobDDRef iterator methods
  blob_iterator blob_begin() { return BlobDDRefs.begin(); }
  blob_iterator blob_end() { return BlobDDRefs.end(); }
  reverse_blob_iterator blob_rbegin() { return BlobDDRefs.rbegin(); }
  reverse_blob_iterator blob_rend() { return BlobDDRefs.rend(); }

  /// \brief Creates GEPInfo object for the DDRef.
  void createGEP() {
    if (!hasGEPInfo()) {
      GepInfo = new GEPInfo;
    }
  }

  /// \brief Returns contained GEPInfo. Asserts if it is not set.
  GEPInfo *getGEPInfo() const {
    assert(hasGEPInfo() && "GEPInfo not present!");
    return GepInfo;
  }

  /// \brief Returns non-const iterator version of CBlobI.
  blob_iterator getNonConstBlobIterator(const_blob_iterator CBlobI);

  /// \brief Returns true if the Position is within the dimension range.
  bool isDimensionValid(unsigned Pos) const {
    return (Pos > 0 && Pos <= getNumDimensions());
  }

  /// \brief Implements getBase*Type() functionality.
  Type *getBaseTypeImpl(bool IsSrc) const {
    if (hasGEPInfo()) {
      return IsSrc ? getBaseCE()->getSrcType() : getBaseCE()->getDestType();
    }
    return nullptr;
  }

  /// \brief Used by updateBlobDDRefs() to remove BlobDDRefs which are not
  /// needed anymore. The required blobs are passed in through BlobIndices. The
  /// function removes those blobs from BlobIndices whose BlobDDRef is already
  /// attached to RegDDRef. It returns stale blobs in \p StaleBlobs.
  void removeStaleBlobDDRefs(SmallVectorImpl<unsigned> &BlobIndices,
                             SmallVectorImpl<BlobDDRef *> &StaleBlobs);

  /// \brief Called by the verifier to check that the temp blobs contained in
  /// the DDRef correspond to blob DDRefs attached to the DDRef.
  void checkBlobDDRefsConsistency() const;

  /// \brief Implements get*Type() functionality.
  Type *getTypeImpl(bool IsSrc) const;

  /// \brief Updates def level of CE based on the level of the blobs present in
  /// CE. DDRef is assumed to have the passed in NestingLevel.
  void updateCEDefLevel(CanonExpr *CE, unsigned NestingLevel);

  /// Implements def level update for the RegDDRef.
  void updateDefLevelInternal(unsigned NewLevel);

  /// Implementes populateTempBlobIndices() and populateTempBlobSymbases().
  void populateTempBlobImpl(SmallVectorImpl<unsigned> &Blobs,
                            bool GetIndices) const;

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
  Type *getSrcType() const override { return getTypeImpl(true); }
  /// \brief Returns the dest element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose dest base type is [7 x
  /// [101 x int32]]*, we will return int32.
  /// TODO: extend to handle struct types.
  Type *getDestType() const override { return getTypeImpl(false); }

  /// \brief Returns the src type of the base CanonExpr for GEP DDRefs, returns
  /// null for non-GEP DDRefs.
  Type *getBaseSrcType() const { return getBaseTypeImpl(true); }
  /// \brief Sets the src type of base CE of GEP DDRefs.
  void setBaseSrcType(Type *SrcTy) {
    assert(hasGEPInfo() && "Base CE accessed for non-GEP DDRef!");
    getBaseCE()->setSrcType(SrcTy);
  }

  /// Returns the dest type of the base CanonExpr for GEP DDRefs, returns null
  /// for non-GEP DDRefs. Base destination type represents a bitcast on the GEP
  /// like this-
  /// %arrayidx = getelementptr [10 x float], [10 x float]* %p, i64 0, i64 %k
  /// %190 = bitcast float* %arrayidx to i32*
  /// store i32 %189, i32* %190
  ///
  /// The DDRef looks like this in HIR-
  /// *(i32*)(%ex1)[0][i1]
  ///
  /// The base canon expr is stored like this-
  /// bitcast.[1001 x float]*.i32*(%ex1)
  /// The represented cast is imprecise because the actual casting occurs from
  /// float* to i32*. The stored information is enough to generate the correct
  /// code though. This setup needs to be rethought if the transformations want
  /// to access three different types involved here-
  /// [1001 x float]*, float* and i32*. We are currently not storing the
  /// intermediate float* type but it can be computed on the fly.
  ///
  /// TODO: Rethink the setup, if required.
  Type *getBaseDestType() const { return getBaseTypeImpl(false); }

  /// \brief Sets the dest type of base CE of GEP DDRefs.
  void setBaseDestType(Type *DestTy) {
    assert(hasGEPInfo() && "Base CE accessed for non-GEP DDRef!");
    getBaseCE()->setDestType(DestTy);
  }

  /// Returns the type associated with \p DimensionNum. For example, consider
  /// this case-
  /// %struct.S2 = type { float, [100 x %struct.S1] }
  /// %struct.S1 = type { i32, i32 }
  /// @obj2 = [50 x %struct.S2]
  ///
  /// %t = GEP @obj2, 0, i, 1, j, 1
  /// store to %t
  ///
  /// This reference looks like this in HIR-
  /// (@obj2)[0][i].1[j].1
  ///
  /// This reference has the following dimension types (from lower to higher)-
  /// Dimension1 - [100 x %struct.S1]
  /// Dimension2 - [50 x %struct.S2]
  /// Dimension3 - [50 x %struct.S2]*
  Type *getDimensionType(unsigned DimensionNum) const;

  /// Returns the element type of the dimension type associated with \p
  /// DimensionNum. For the example in description of getDimensionType() they
  /// are as follows-
  /// Dimension1 - %struct.S1
  /// Dimension2 - %struct.S2
  /// Dimension3 - [50 x %struct.S2]
  Type *getDimensionElementType(unsigned DimensionNum) const {
    auto DimTy = getDimensionType(DimensionNum);
    return DimTy->isPointerTy() ? DimTy->getPointerElementType()
                                : DimTy->getArrayElementType();
  }

  /// Returns true if the Ref accesses a structure.
  bool accessesStruct() const;

  /// Returns underlying LLVM value of the base if it is a temp, otherwise
  /// returns nullptr.
  Value *getTempBaseValue() const;

  /// Returns true if the Ref accesses a global variable.
  bool accessesGlobalVar() const {
    auto BaseVal = getTempBaseValue();

    if (!BaseVal) {
      return false;
    }

    return isa<GlobalVariable>(BaseVal);
  }

  /// Returns true if the Ref accesses an internal global variable.
  bool accessesInternalGlobalVar() const {
    auto GlobalVar = dyn_cast_or_null<GlobalVariable>(getTempBaseValue());
    return (GlobalVar && GlobalVar->hasInternalLinkage());
  }

  /// Returns true if the Ref accesses a constant array.
  bool accessesConstantArray() const {
    auto GlobalVar = dyn_cast_or_null<GlobalVariable>(getTempBaseValue());
    return (GlobalVar && GlobalVar->isConstant());
  }

  /// Returns true if Ref is an alloca access.
  bool accessesAlloca() const {
    auto BaseVal = getTempBaseValue();

    if (!BaseVal) {
      return false;
    }

    return isa<AllocaInst>(BaseVal);
  }

  /// \brief Returns the canonical form of the subscript base.
  CanonExpr *getBaseCE() { return getGEPInfo()->BaseCE; }
  const CanonExpr *getBaseCE() const {
    return const_cast<RegDDRef *>(this)->getBaseCE();
  }

  /// \brief Sets the canonical form of the subscript base.
  void setBaseCE(CanonExpr *BaseCE) {
    createGEP();
    getGEPInfo()->BaseCE = BaseCE;
  }

  /// \brief Returns true if the inbounds attribute is set for this access.
  bool isInBounds() const { return getGEPInfo()->InBounds; }

  /// Sets the inbounds attribute for this access.
  void setInBounds(bool IsInBounds) {
    createGEP();
    getGEPInfo()->InBounds = IsInBounds;
  }

  /// \brief Returns true if this is an address computation.
  bool isAddressOf() const {
    // getGEPInfo() asserts that RegDDRef has GEPInfo. Clients of isAddressOf
    // should not be forced to check for hasGEPInfo() before calling
    // isAddressOf().
    if (!hasGEPInfo()) {
      return false;
    }
    return getGEPInfo()->AddressOf;
  }

  /// Sets/resets this ref as an address computation.
  void setAddressOf(bool IsAddressOf) {
    createGEP();
    getGEPInfo()->AddressOf = IsAddressOf;
  }

  /// \brief Returns true if this is a volatile load/store.
  bool isVolatile() const { return getGEPInfo()->Volatile; }

  /// Sets/resets this ref as a volatile load/store.
  void setVolatile(bool IsVolatile) {
    createGEP();
    getGEPInfo()->Volatile = IsVolatile;
  }

  /// \brief Returns alignment info for this ref.
  unsigned getAlignment() const { return getGEPInfo()->Alignment; }

  /// Sets alignment for this ref.
  void setAlignment(unsigned Align) {
    createGEP();
    getGEPInfo()->Alignment = Align;
  }

  // Get/Set DebugLoc for the Load/Store instruction
  const DebugLoc &getMemDebugLoc() const { return getGEPInfo()->MemDbgLoc; }
  void setMemDebugLoc(const DebugLoc &Loc) { getGEPInfo()->MemDbgLoc = Loc; }

  // Get/Set DebugLoc for the GEP instruction
  const DebugLoc &getGepDebugLoc() const { return getGEPInfo()->GepDbgLoc; }
  void setGepDebugLoc(const DebugLoc &Loc) { getGEPInfo()->GepDbgLoc = Loc; }

  // Wrapper method to return relevant debug location.
  const DebugLoc &getDebugLoc() const {
    return isAddressOf() ? getGepDebugLoc() : getMemDebugLoc();
  }

  /// \brief Extract and submit AA metadata
  void getAAMetadata(AAMDNodes &AANodes) const;
  void setAAMetadata(AAMDNodes &AANodes);

  /// \brief Returns the metadata of given kind attached to this ref, else
  /// returns null.
  MDNode *getMetadata(StringRef Kind) const;
  MDNode *getMetadata(unsigned KindID) const;

  /// \brief Returns all metadata attached to this ref.
  void getAllMetadata(MDNodesTy &MDs) const;

  /// \brief Returns all metadata attached to this ref other than DebugLoc.
  void getAllMetadataOtherThanDebugLoc(MDNodesTy &MDs) const;

  /// \brief Sets the metadata of the specific kind. This updates/replaces
  /// metadata if already present, or removes it if Node is null.
  void setMetadata(StringRef Kind, MDNode *Node);
  void setMetadata(unsigned KindID, MDNode *Node);

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
    return isTerminalRef() && getSingleCanonExpr()->isConstant();
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

  bool hasBlobDDRefs() const { return !BlobDDRefs.empty(); }
  unsigned numBlobDDRefs() const { return BlobDDRefs.size(); }

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

  /// Returns true if Ref is a fake lval DDRef. This function assumes that the
  /// DDRef is connected to a HLDDNode.
  bool isFakeLval() const;

  /// Returns true if Ref is a fake rval DDRef. This function assumes that the
  /// DDRef is connected to a HLDDNode.
  bool isFakeRval() const;

  /// \brief This method checks if the DDRef is
  /// not a memory reference or a pointer reference
  /// Returns false for:
  ///      RegDDRef is Memory Reference - A[i]
  ///      RegDDRef is a Pointer Reference - *p
  /// Else returns true for cases like DDRef - 2*i and M+N.
  bool isTerminalRef() const {
    if (!hasGEPInfo()) {
      assert(isSingleCanonExpr() &&
             "Terminal ref has more than one dimension!");
      return true;
    }
    return false;
  }

  /// \brief Returns true if the DDRef is structurally invariant at \p Level.
  /// Note!: It does not check data-dependences, so there may be cases where
  /// the  DDRef is structurally invariant, but not actually invariant. For
  /// example, in the loop below, A[5] is structurally invariant, but not
  /// actually invariant because of the data-dependence:
  /// for (i=0; i<10; i++) { A[i] = A[5] + i;}
  bool isStructurallyInvariantAtLevel(unsigned Level) const;

  /// \brief Returns true if the DDRef is a memory reference
  bool isMemRef() const { return hasGEPInfo() && !isAddressOf(); }

  /// \brief Returns true if the RegDDRef represents a standalone IV like (1 *
  /// i3).
  /// If \p AllowConversion is true, conversions are allowed to be part of a
  /// standalone IV.
  /// Otherwise, an IV with a conversion is not considered a standalone IV.
  bool isStandAloneIV(bool AllowConversion = true) const;

  /// \brief Returns true if the DDRef represents a self-blob like (1 * %t). In
  /// addition DDRef's symbase should be the same as %t's symbase. This is so
  /// because for some livein copies %t1 = %t2, lval %t1 is parsed as 1 * %t2.
  /// But since %t1 has a different symbase than %t2 we still need to add a blob
  /// DDRef for %t2 to the DDRef.
  bool isSelfBlob() const override;

  // \brief Returns true if the DDRef is a unitary blob. A unitary blob is a
  // single (non-nested) standalone blob.
  bool isUnitaryBlob() const;

  /// Returns true if this ref looks like 1 * undef.
  bool isStandAloneUndefBlob() const override;

  /// \brief Returns true if the DDRef represents a blob like (1 * %t).
  /// This is a broader check than isSelfBlob() because DDRef's symbase is not
  /// taken into account. In addition, a standalone blob allows a FP constant or
  /// even metadata.
  /// If \p AllowConversion is true, conversions are allowed to be part of a
  /// standalone blob. Otherwise, a blob with a conversion is not considered a
  /// standalone blob.
  bool isStandAloneBlob(bool AllowConversion = true) const;

  /// \brief Returns true if this DDRef contains undefined canon expressions.
  bool containsUndef() const override;

  /// \brief Adds a dimension to the DDRef with optional trailing offsets.
  void
  addDimension(CanonExpr *IndexCE,
               const SmallVectorImpl<unsigned> *TrailingOffsets = nullptr) {
    assert(IndexCE && "IndexCE is null!");
    CanonExprs.push_back(IndexCE);

    if (TrailingOffsets) {
      setTrailingStructOffsets(getNumDimensions(), *TrailingOffsets);
    }
  }

  /// Sets trailing offsets for \p DimensionNum.
  void setTrailingStructOffsets(unsigned DimensionNum,
                                const SmallVectorImpl<unsigned> &Offsets);

  /// Returns trailing offsets for \p DimensionNum. Returns null if there are no
  /// offsets.
  const SmallVectorImpl<unsigned> *
  getTrailingStructOffsets(unsigned DimensionNum) const;

  /// Removes trailing offsets for \p DimensionNum.
  void removeTrailingStructOffsets(unsigned DimensionNum) {
    // Sets an empty vector which is equivalent to no offsets.
    OffsetsTy Offsets;
    setTrailingStructOffsets(DimensionNum, Offsets);
  }

  /// Returns true if the Ref has trailing offsets for \p DimensionNum.
  bool hasTrailingStructOffsets(unsigned DimensionNum) const {
    return (getTrailingStructOffsets(DimensionNum) != nullptr);
  }

  /// Returns true if the Ref has trailing offsets for any dimension.
  bool hasTrailingStructOffsets() const;

  /// \brief Returns the stride in number of bytes for specified dimension.
  /// This is computed on the fly. DimensionNum must be within
  /// [1, getNumDimensions()].
  uint64_t getDimensionStride(unsigned DimensionNum) const;

  /// Returns the size in number of bytes for specified dimension.
  /// This is computed on the fly. 0 is returned for pointer dimension.
  /// DimensionNum must be within [1, getNumDimensions()].
  uint64_t getDimensionSize(unsigned DimensionNum) const;

  /// Returns the number of elements for specified dimension. 0 is returned for
  /// pointer dimension. DimensionNum must be within [1, getNumDimensions()].
  uint64_t getNumDimensionElements(unsigned DimensionNum) const;

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

  /// Populates constant stride of DDRef at \p Level in \p Stride, if is is not
  /// null.
  /// Returns false if the stride is not constant.
  bool getConstStrideAtLevel(unsigned Level, int64_t *Stride) const;

  /// Not sure if removeDimension() operation even makes sense. Commenting it
  /// out for now.
  /// \brief Removes a dimension from the DDRef. DimensionNum's range is
  /// [1, getNumDimensions()] with 1 representing the lowest dimension.
  // void removeDimension(unsigned DimensionNum) {
  //  assert(isDimensionValid(DimensionNum) && "DimensionNum is out of range!");
  //  assert((getNumDimensions() > 1) && "Attempt to remove the only
  //  dimension!");
  //
  //  CanonExprs.erase(CanonExprs.begin() + (DimensionNum - 1));
  // }

  /// \brief Returns the index of the blob represented by this self-blob DDRef.
  unsigned getSelfBlobIndex() const {
    assert(isSelfBlob() && "DDRef is not a self blob!");
    return getSingleCanonExpr()->getSingleBlobIndex();
  }

  /// Replaces existing self blob index with \p NewIndex.
  void replaceSelfBlobIndex(unsigned NewIndex);

  /// Converts a terminal lval ref into a self blob ref using its symbase.
  /// For example, if we have t1 = t2 + t3, where t1's canonical form is (1 * t2
  /// + 1 * t3), it will be converted to 1 * t1.
  void makeSelfBlob();

  /// \brief Adds a blob DDRef to this DDRef.
  void addBlobDDRef(BlobDDRef *BlobRef);

  /// \brief Creates a blob DDRef with passed in Index and Level and adds it to
  /// this DDRef.
  void addBlobDDRef(unsigned Index, unsigned Level = NonLinearLevel);

  /// \brief Returns the blob DDRef with \p Index attached to this RegDDRef.
  /// It returns null if the blob DDRef is not found.
  BlobDDRef *getBlobDDRef(unsigned Index);
  const BlobDDRef *getBlobDDRef(unsigned Index) const;

  /// \brief Removes and returns blob DDRef corresponding to CBlobI iterator.
  BlobDDRef *removeBlobDDRef(const_blob_iterator CBlobI);

  /// Replaces temp blob with \p OldIndex by new temp blob with \p NewIndex, if
  /// it exists in DDRef. Returns true if it is replaced.
  bool replaceTempBlob(unsigned OldIndex, unsigned NewIndex);

  /// \brief Removes all blob DDRefs attached to this DDRef.
  void removeAllBlobDDRefs();

  /// Returns true if there is a use of temp blob with \p Index in the DDRef.
  /// IsSelfBlob is set to true if the DDRef is a self blob.
  bool usesTempBlob(unsigned Index, bool *IsSelfBlob = nullptr) const;

  /// \brief Collects all the unique temp blobs present in the DDRef by visiting
  /// all the contained canon exprs.
  void collectTempBlobIndices(SmallVectorImpl<unsigned> &Indices) const;

  /// In contrast to collectTempBlobIndices(), this function assumes the ref is
  /// in a consistent state. Therefore it populates the vector using blob ddrefs
  /// attached to the ref which is a more efficient approach.
  void populateTempBlobIndices(SmallVectorImpl<unsigned> &Indices) const {
    populateTempBlobImpl(Indices, true);
  }

  /// Same as populateTempBlobIndices() above except that it populates symbases
  /// instead of indices.
  void populateTempBlobSymbases(SmallVectorImpl<unsigned> &Symbases) const {
    populateTempBlobImpl(Symbases, false);
  }

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
  /// The NewLevel argument indicates the nesting level of this DDRef.
  /// The argument may be omitted, in this case the level of attachment in HIR
  /// will be used.
  ///
  /// NOTE: This utility cannot handle cases where blob definitions have been
  /// moved around or where non-linear blobs can be turned into linear blobs
  /// (during sinking, for example) because we do not track blob definitions.
  /// They require customized handling.
  void updateDefLevel(unsigned NewLevel = NonLinearLevel);

  /// \brief Makes a modified ref internally consistent by updating blob DDRefs
  /// and containing CanonExprs' def level. The passed in AuxRefs should contain
  /// all the new blobs discovered in the DDRef or the function would assert.
  /// The blob DDRefs attached to these auxiliarry DDRefs are assumed to be in
  /// an updated state. If such DDRefs are not available the alternative is to
  /// call updateBlobDDRefs(), update the level of the new blobs manually and
  /// then call updateDefLevel().
  ///
  /// The NewLevel argument indicates the nesting level of this DDRef.
  /// The argument may be omitted, in this case the level of attachment in HIR
  /// will be used.
  ///
  /// NOTE: This utility cannot handle cases where blob definitions have been
  /// moved around or where non-linear blobs can be turned into linear blobs
  /// (during sinking, for example) because we do not track blob definitions.
  /// These cases require customized handling. They can be partially handled
  /// using updateBlobDDRefs() and updateDefLevel() by manually updating blob
  /// levels.
  void
  makeConsistent(const SmallVectorImpl<const RegDDRef *> *AuxRefs = nullptr,
                 unsigned NewLevel = NonLinearLevel);

  /// \brief Returns true if the blob is present in this DDRef and returns its
  /// defined at level via DefLevel. DefLevel is expected to be non-null. The
  /// blob is searched in the blob DDRefs attached to this DDRef. This function
  /// can be used to update defined at levels for blobs which were copied from
  /// this DDRef to another DDRef.
  bool findTempBlobLevel(unsigned BlobIndex, unsigned *DefLevel) const;

  /// \brief Returns maximum blob level amongst the blobs in the vector.
  /// NOTE: this function asserts if any of the temp blobs is not contained in
  /// the DDRef.
  unsigned
  findMaxTempBlobLevel(const SmallVectorImpl<unsigned> &TempBlobIndices) const;

  /// \brief Returns maximum blob level of temp blobs contained in this blob.
  /// NOTE: This function asserts if any of the temp blobs is not contained in
  /// the DDRef.
  unsigned findMaxBlobLevel(unsigned BlobIndex) const;

  /// Returns true if ref has an IV at \p Level.
  bool hasIV(unsigned Level) const;

  /// Returns the defined at level of the ref.
  unsigned getDefinedAtLevel() const;

  /// \brief Replace any loop-level IV by a given constant integer.
  void replaceIVByConstant(unsigned LoopLevel, int64_t Val);

  /// \brief A RegDDRef is nonlinear if any of the following is true:
  /// - its baseCE (if available) is nonlinear
  /// - any CE is nonlinear
  bool isNonLinear(void) const;

  /// Shift all CE(s) in the RegDDRef* by a given Amount.
  /// E.g.
  /// -----------------------------------------------------
  /// |Orig Ref  |Shift Amount  | After Shift             |
  /// -----------------------------------------------------
  ///  A[i]      | 1            |  A[i+1]                 |
  ///  A[2i]     | 1            |  A[2(i+1)]  -> A[2i+2]  |
  ///  A[2i+1]   | -1           |  A[2(i-1)+1]-> A[2i-1]  |
  /// -----------------------------------------------------
  void shift(unsigned LoopLevel, int64_t Amount);

  /// \brief Verifies RegDDRef integrity.
  virtual void verify() const override;
};

} // End namespace loopopt

} // End namespace llvm

namespace std {

// default_delete<RegDDRef> is a helper for destruction RegDDRef objects to
// support std::unique_ptr<RegDDRef>.
template <> struct default_delete<llvm::loopopt::RegDDRef> {
  void operator()(llvm::loopopt::RegDDRef *Ref) const;
};
} // namespace std

#endif
