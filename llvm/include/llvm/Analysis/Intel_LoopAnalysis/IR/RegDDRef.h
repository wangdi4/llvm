//===----- RegDDRef.h - Regular data dependency node in HIR -----*- C++ -*-===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/BlobDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/DDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/IntegerRange.h"
#include "llvm/Analysis/MemoryLocation.h"

namespace llvm {

class MDNode;
struct AAMDNodes;

namespace loopopt {

class HLDDNode;

/// Regular DDRef representing Values
///
/// Objects of this class represent temps and load/stores. Information to
/// regenerate GEP instruction associated with load/stores is maintained here.
///
class RegDDRef final : public DDRef {
public:
  /// loads/stores can be mapped as multi-dimensional subscripts with each
  /// subscript having its own canonical form.
  typedef SmallVector<CanonExpr *, 3> CanonExprsTy;
  typedef SmallVector<const CanonExpr *, 3> ConstCanonExprsTy;
  typedef SmallVector<BlobDDRef *, 2> BlobDDRefsTy;
  typedef SmallVector<const BlobDDRef *, 2> ConstBlobDDRefsTy;
  typedef CanonExprsTy SubscriptTy;
  typedef std::pair<unsigned, MDNode *> MDPairTy;
  typedef SmallVector<MDPairTy, 6> MDNodesTy;

  /// Iterators to iterate over canon exprs
  typedef CanonExprsTy::iterator canon_iterator;
  typedef ConstCanonExprsTy::const_iterator const_canon_iterator;
  typedef CanonExprsTy::reverse_iterator reverse_canon_iterator;
  typedef ConstCanonExprsTy::const_reverse_iterator
      const_reverse_canon_iterator;

  /// Iterators to iterate over blob ddrefs
  typedef BlobDDRefsTy::iterator blob_iterator;
  typedef ConstBlobDDRefsTy::const_iterator const_blob_iterator;
  typedef BlobDDRefsTy::reverse_iterator reverse_blob_iterator;
  typedef ConstBlobDDRefsTy::const_reverse_iterator const_reverse_blob_iterator;

  /// Traverses all DDRefs in HLDDNode.
  /// RegDDRef is traversed before all BlobDDRefs associated with RegDDRef.
  ///
  /// Traversal corresponds to the following loop nest:
  /// for (RegIt: make_range(ddref_begin(), ddref_end())) {
  ///   // Processing of *RegIt
  ///   for (BlobIt:
  ///     make_range((*RegIt)->blob_begin(), (*RegIt)->blob_begin())) {
  ///      // Processing of *BlobIt
  ///   }
  /// }
  ///
  /// value and reference types are DDRef*.
  template <typename DDRefIteratorTy>
  class const_all_ddref_iterator {
  public:
    using iteration_category = std::bidirectional_iterator_tag;
    using value_type = const DDRef *;
    using difference_type = ptrdiff_t;
    using pointer = const DDRef *const *;
    using reference = const DDRef *;

    typedef RegDDRef::const_blob_iterator const_blob_iterator;

  public:
    explicit const_all_ddref_iterator(DDRefIteratorTy RegIt)
        : RegIt(RegIt), BlobIt(nullptr), IsRegDDRef(true) {}

    bool operator==(const const_all_ddref_iterator &It) const {
      return RegIt == It.RegIt && IsRegDDRef == It.IsRegDDRef &&
             BlobIt == It.BlobIt;
    }

    bool operator!=(const const_all_ddref_iterator &It) const {
      return !(operator==(It));
    }

    const_all_ddref_iterator &operator++() {
      // See descriptors in private section
      if (IsRegDDRef) {
        IsRegDDRef = false;
        BlobIt = (*RegIt)->blob_begin();
      } else {
        ++BlobIt;
      }
      if (BlobIt == (*RegIt)->blob_end()) {
        IsRegDDRef = true;
        BlobIt = nullptr;
        ++RegIt;
      }
      return *this;
    }

    const_all_ddref_iterator &operator--() {
      // See descriptors in private section
      if (IsRegDDRef) {
        IsRegDDRef = false;
        BlobIt = (*(--RegIt))->blob_end();
      }
      if (BlobIt == (*RegIt)->blob_begin()) {
        IsRegDDRef = true;
        BlobIt = nullptr;
      } else {
        --BlobIt;
      }
      return *this;
    }

    const_all_ddref_iterator operator++(int) {
      const_all_ddref_iterator retval = *this;
      ++(*this);
      return retval;
    }

    const_all_ddref_iterator operator--(int) {
      const_all_ddref_iterator retval = *this;
      --(*this);
      return retval;
    }

    reference operator*() const {
      if (IsRegDDRef) {
        return *RegIt;
      }
      return *BlobIt;
    }

  private:
    // This iterator is one-past-end when RegIt is one-past-end and IsRegDDRef
    // is true.
    //
    // This iterator can be dereferenced when RegIt can be dereferenced and
    // either IsRegDDRef is true or BlobIt can be dereferenced.
    DDRefIteratorTy RegIt;
    const_blob_iterator BlobIt;
    bool IsRegDDRef;
  };

  template <typename T>
  using addressof_iterator =
      mapped_iterator<T, decltype(&std::pointer_traits<T>::pointer_to)>;

  struct const_all_ddref_single_iterator
      : public const_all_ddref_iterator<addressof_iterator<const RegDDRef *>> {
    const_all_ddref_single_iterator(const RegDDRef *Ref)
        : const_all_ddref_iterator(addressof_iterator<const RegDDRef *>(
              Ref, std::pointer_traits<const RegDDRef *>::pointer_to)) {}
  };

private:
  typedef SmallVector<unsigned, 2> OffsetsTy;

  /// Contains extra information required to regenerate GEP instruction
  /// at code generation.
  struct GEPInfo {
    CanonExpr *BaseCE;
    // Base ptr element type is needed to be stored explicitly in the presence
    // of opaque ptrs. For example, when parsing this opaque ptr GEP, we will
    // store [20 x i32] as BasePtrElementTy- %gep = getelementptr [20 x i32],
    // ptr @p, i64 0, i64 1 This will let us recreate the GEP during CodeGen.
    Type *BasePtrElementTy;
    // If there is a bitcast on the GEP before its use (in load/store
    // instruction etc), we store the destination element type of the bitcast
    // here. The only exception is when the bitcast dest type is a vector of
    // pointers (like <4 x i32*>) instead of a pointer type. This is set on
    // AddressOf refs by vectorizer. In this case we store the vector type. If
    // no bitcast type is present, it is set to null. For example, we will store
    // i64 as the BitCastDestVecOrElemTy in the following case-
    //
    //   %gep = getelementptr i8, i8* %indvars.iv2526, i64 4
    //   %bc = bitcast i8* %gep to i64*
    //   store i64 %add, i64* %bc
    //
    // This field is necessary even in the presence of opaque ptrs to catch
    // mismatch between the type indexed in GEP and the load/store type.
    // For example, we parse the load as (double*)(%p)[0][10] with double stored
    // in BitCastDestVecOrElemTy for the following opaque ptr IR-
    //
    //   %gep = getelementptr [100 x i32], ptr %p, i64 0, i64 10
    //   %ld = double, ptr %gep
    Type *BitCastDestVecOrElemTy;
    bool InBounds;
    // This is set if this DDRef represents an address computation (GEP) instead
    // of a load or store.
    bool AddressOf;
    bool IsCollapsed; // Set if the DDRef has been collapsed through Loop
                      // Collapse Pass. Needed for DD test to bail out often.
    unsigned MaxVecLenAllowed; // Maximum Vector length allowed
    unsigned Alignment;
    // Extent is passed by Fortran FE to indicate known highest dimsize.
    unsigned HighestDimNumElements;
    // Set to true for fake pointer DD ref which access type is known.
    bool CanUsePointeeSize;
    // Fortran only. Set to true if one of the non-constant strides may be zero
    // as a result of PRODUCT or SUM functions.
    bool AnyVarDimStrideMayBeZero;

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

    CanonExprsTy LowerBounds;
    CanonExprsTy Strides;
    SmallVector<Type *, 3> DimTypes;
    // Dimension element types need to be stored explicitly in the presence of
    // opaque ptrs.
    SmallVector<Type *, 3> DimElementTypes;

    // Indicates whether stride is an exact multiple of element size.
    SmallVector<bool, 3> StrideIsExactMultiple;

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

    // For some refs, dummy gep inst is constructed to be used for alias
    // analysis and is cached here.
    GetElementPtrInst *DummyGepLoc;

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

  RegDDRef(DDRefUtils &DDRU, unsigned SB);

  /// Calling delete on a null pointer has no effect.
  virtual ~RegDDRef() override { delete GepInfo; }

  /// Copy constructor used by cloning.
  RegDDRef(const RegDDRef &RegDDRefObj);

  friend class DDRefUtils;

  // Required to access setHLDDNode().
  friend class HLDDNode;

  // Accesses MDNodes.
  friend class HIRParser;

  /// Sets the HLDDNode of this RegDDRef
  void setHLDDNode(HLDDNode *HNode) override { Node = HNode; }

  /// Creates GEPInfo object for the DDRef.
  void createGEP() {
    if (!hasGEPInfo()) {
      GepInfo = new GEPInfo;
    }
  }

  /// Returns contained GEPInfo. Asserts if it is not set.
  GEPInfo *getGEPInfo() const {
    assert(hasGEPInfo() && "GEPInfo not present!");
    return GepInfo;
  }

  /// Returns non-const iterator version of CBlobI.
  blob_iterator getNonConstBlobIterator(const_blob_iterator CBlobI);

  /// Returns true if the Position is within the dimension range.
  bool isDimensionValid(unsigned Pos) const {
    return (Pos > 0 && Pos <= getNumDimensions());
  }

  /// Used by updateBlobDDRefs() to remove BlobDDRefs which are not
  /// needed anymore. The required blobs are passed in through BlobIndices. The
  /// function removes those blobs from BlobIndices whose BlobDDRef is already
  /// attached to RegDDRef. It returns stale blobs in \p StaleBlobs.
  void removeStaleBlobDDRefs(SmallVectorImpl<unsigned> &BlobIndices,
                             SmallVectorImpl<BlobDDRef *> &StaleBlobs);

  /// Checks that the DefAtLevel of \p CE is consistent with the temp blobs
  /// contained in it. Returns the contained blobs in \p TempBlobIndices.
  void
  checkDefAtLevelConsistency(const CanonExpr *CE,
                             SmallVectorImpl<unsigned> &TempBlobIndices) const;

  /// Called by the verifier to check that the temp blobs contained in
  /// the DDRef correspond to blob DDRefs attached to the DDRef. Also checks
  /// that the DefAtLevel set in canon exprs is consistent with the contained
  /// temp blobs' def levels.
  void checkBlobAndDefAtLevelConsistency() const;

  /// Implements get*Type() functionality.
  Type *getTypeImpl(bool IsSrc) const;

  /// Updates def level of CE based on the level of the blobs present in
  /// CE. DDRef is assumed to have the passed in NestingLevel.
  void updateCEDefLevel(CanonExpr *CE, unsigned NestingLevel);

  /// Implements def level update for the RegDDRef.
  void updateDefLevelInternal(unsigned NewLevel);

  /// Implementes populateTempBlobIndices() and populateTempBlobSymbases().
  void populateTempBlobImpl(SmallVectorImpl<unsigned> &Blobs,
                            bool GetIndices) const;

  /// Clarifies the result type associated with \p DimensionNum.
  /// Pointer Ty* may be substituted to Ty* or to an array [ x Ty].
  /// Arrays may be substituted to the same arrays only.
  void setDimensionType(unsigned DimensionNum, Type *Ty) {
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");
    assert(hasGEPInfo() && "Call is only meaningful for GEP DDRefs!");

    GepInfo->DimTypes[DimensionNum - 1] = Ty;
  }

  void setHighestDimNumElements(unsigned NumElements) {
    GepInfo->HighestDimNumElements = NumElements;
  }

  static Type *getMorePreciseDimensionType(Type *T1, Type *T2);

  /// Adds a dimension to the DDRef with optional trailing offsets. The new
  /// dimension becomes the highest dimension of the ref. For example, if the
  /// ref looks like A[i1] before the call, it will look like A[0][i1] after
  /// adding a zero canon expr as an additional dimension.
  void addDimensionHighest(CanonExpr *IndexCE,
                           ArrayRef<unsigned> TrailingOffsets = {},
                           CanonExpr *LowerBoundCE = nullptr,
                           CanonExpr *StrideCE = nullptr, Type *DimTy = nullptr,
                           Type *DimElemTy = nullptr,
                           bool IsExactMultiple = true);

  /// Returns true if the GEP ref has a 'known' location (address range). An
  /// unattached or fake ref's location is unknown.
  bool hasKnownLocation() const;

  /// Returns true if a GEP representing the ref can be created for alias
  /// analyis.
  bool canCreateLocationGEP() const;

  /// Returns a GEP Inst which represents the ref, for alias analysis.
  /// Asserts that canCreateLocationGEP() is true.
  /// The GEP Inst is cached for reuse.
  GetElementPtrInst *getOrCreateLocationGEP() const;

  void printImpl(formatted_raw_ostream &OS, bool Detailed,
                 bool DimDetails) const;

public:
  /// Returns HLDDNode this DDRef is attached to.
  const HLDDNode *getHLDDNode() const override { return Node; };

  HLDDNode *getHLDDNode() override { return Node; };

  /// Prints RegDDRef.
  virtual void print(formatted_raw_ostream &OS,
                     bool Detailed = false) const override;

  /// Print together with list of attached BlobDDRefs.
  void printWithBlobDDRefs(formatted_raw_ostream &OS, unsigned Depth) const;

  /// Prints details of dimensions, matching with -hir-details-refs
  /// Argument Detailed has the same meaning of Detailed in print.
  void dumpDims(bool Detailed = false) const;

  /// Returns true if the DDRef has GEP Info.
  bool hasGEPInfo() const { return (GepInfo != nullptr); }

  /// Returns the src type of the base CanonExpr for GEP DDRefs, asserts for
  /// non-GEP DDRefs.
  Type *getBaseType() const { return getBaseCE()->getSrcType(); }

  /// Returns the element type of base ptr GEP DDRefs.
  /// For example, will return [10 x i32] for a base ptr type of [10 x i32]*.
  Type *getBasePtrElementType() const { return getGEPInfo()->BasePtrElementTy; }
  void setBasePtrElementType(Type *Ty) {
    getGEPInfo()->BasePtrElementTy = Ty;

    // Set dimension element type of highest dimension (if it exists) to be the
    // same as BasePtrElementType.
    unsigned NumDims = getNumDimensions();
    if (NumDims != 0) {
      GepInfo->DimElementTypes[NumDims - 1] = Ty;
    }
  }

  /// Returns the src element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose src base type is [7 x
  /// [101 x float]]*, we will return float.
  Type *getSrcType() const override { return getTypeImpl(true); }

  /// Returns the dest element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose dest base type is [7 x
  /// [101 x int32]]*, we will return int32.
  Type *getDestType() const override { return getTypeImpl(false); }

  // Returns destination type size.
  uint64_t getDestTypeSizeInBits() const {
    return getCanonExprUtils().getTypeSizeInBits(getDestType());
  }
  uint64_t getDestTypeSizeInBytes() const {
    return getCanonExprUtils().getTypeSizeInBytes(getDestType());
  }

  // Returns source type size. User is responsible for checking that the type is
  // sized.
  uint64_t getSrcTypeSizeInBytes() const {
    return getCanonExprUtils().getTypeSizeInBytes(getSrcType());
  }

  // Returns true if the ref is AddressOf ref whose element type is sized.
  bool isAddressOfSizedType() const {
    if (!isAddressOf()) {
      return false;
    }

    auto *DerefTy = getDereferencedType();
    return (DerefTy && DerefTy->isSized());
  }

  // Returns size of element type. This is only applicable for AddressOf refs.
  // For example it will return 1 byte for &(i8*)A[i1].
  uint64_t getDereferencedTypeSizeInBytes() const {
    assert(isAddressOfSizedType() && "Dereferenceable AddressOf ref expected!");

    auto *DerefTy = getDereferencedType();
    return getCanonExprUtils().getTypeSizeInBytes(DerefTy);
  }

  /// Returns a pointer val which can act as the location pointer for the GEP
  /// ref for alias analysis.
  /// Sets \p IsPrecise to true if the pointer is a precise location for ref.
  Value *getLocationPtr(bool &IsPrecise) const;

  /// MemoryLocation for Alias Analysis, only valid for memrefs.
  MemoryLocation getMemoryLocation() const;

  /// Returns address spaces for GEP DDRefs.
  /// Asserts for non-GEP DDRefs.
  unsigned getPointerAddressSpace() const {
    return getBaseType()->getPointerAddressSpace();
  }

  // Returns true if this is either isSelfAddressOf() or isSelfMemRef().
  bool isSelfGEPRef(bool IgnoreBitCast = false) const {
    return hasGEPInfo() && isSingleDimension() &&
           getSingleCanonExpr()->isZero() && getDimensionLower(1)->isZero() &&
           getTrailingStructOffsets(1).empty() &&
           (IgnoreBitCast || !getBitCastDestVecOrElemType());
  }

  /// Returns true if the reference represents a pointer value equal to the
  /// BaseCE: &((%b)[0]).
  bool isSelfAddressOf(bool IgnoreBitCast = false) const {
    return isAddressOf() && isSelfGEPRef(IgnoreBitCast);
  }

  /// Returns true if the reference represents a load/store of a pointer value
  /// equal to the BaseCE: ((%b)[0]).
  bool isSelfMemRef(bool IgnoreBitCast = false) const {
    return isMemRef() && isSelfGEPRef(IgnoreBitCast);
  }

  /// Returns the destination element type of the bitcast applied to GEP DDRefs.
  /// The only exception is vector AddressOf refs for which vector of pointers
  /// is stored as the type. It asserts for non-GEP DDRefs. For example-
  ///
  /// %arrayidx = getelementptr [10 x float], [10 x float]* %p, i64 0, i64 %k
  /// %190 = bitcast float* %arrayidx to i32*
  /// store i32 %189, i32* %190
  ///
  /// i32 is the BitCastDestVecOrElemTy.
  /// Element type is stored because opaque pointers will not contain this
  /// information.
  ///
  /// The DDRef looks like this in HIR-
  /// (i32*)(%p)[0][i1]
  ///
  Type *getBitCastDestVecOrElemType() const {
    return getGEPInfo()->BitCastDestVecOrElemTy;
  }

  /// Sets the dest vec or pointer element type of the bitcast of GEP DDRefs.
  void setBitCastDestVecOrElemType(Type *DestTy) {
    getGEPInfo()->BitCastDestVecOrElemTy = DestTy;
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
  Type *getDimensionType(unsigned DimensionNum) const {
    assert(hasGEPInfo() && "Call is only meaningful for GEP DDRefs!");
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

    return GepInfo->DimTypes[DimensionNum - 1];
  }

  /// Returns the element type of the dimension type associated with \p
  /// DimensionNum. For the example in description of getDimensionType() they
  /// are as follows-
  /// Dimension1 - %struct.S1
  /// Dimension2 - %struct.S2
  /// Dimension3 - [50 x %struct.S2]
  Type *getDimensionElementType(unsigned DimensionNum) const {
    assert(hasGEPInfo() && "Call is only meaningful for GEP DDRefs!");
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

    return GepInfo->DimElementTypes[DimensionNum - 1];
  }

  /// Returns true if the Ref accesses a structure.
  bool accessesStruct() const;

  /// Returns underlying LLVM value of the base. Only applicable to GEP refs.
  Value *getBaseValue() const;

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

  /// Returns true if the Ref access function argument.
  bool accessesFunctionArgument() const {
    auto BaseVal = getTempBaseValue();
    return (BaseVal && isa<Argument>(BaseVal));
  }

  /// Returns true if the Ref access function argument with noalias attribute.
  bool accessesNoAliasFunctionArgument() const {
    auto BaseVal = getTempBaseValue();
    if (!BaseVal) {
      return false;
    }
    auto *Arg = dyn_cast<Argument>(BaseVal);
    return (Arg && Arg->hasNoAliasAttr());
  }

  /// Returns true if Ref is an alloca access.
  bool accessesAlloca() const {
    auto BaseVal = getTempBaseValue();
    return (BaseVal && isa<AllocaInst>(BaseVal));
  }

  /// Returns the canonical form of the subscript base.
  CanonExpr *getBaseCE() { return getGEPInfo()->BaseCE; }
  const CanonExpr *getBaseCE() const {
    return const_cast<RegDDRef *>(this)->getBaseCE();
  }

  /// Returns the blob index of the base pointer.
  /// InvalidBlobIndex is returned if base pointer is undef or null.
  unsigned getBasePtrBlobIndex() const;

  /// Returns the symbase of the base pointer.
  /// ConstantSymbase is returned if base pointer is undef or null.
  unsigned getBasePtrSymbase() const;

  /// Returns the dereferenced type of the address of Ref. Returns null if the
  /// info is not available. For example, it will return i32 for a ref like
  /// &(p)[5] where p is i32*.
  Type *getDereferencedType() const;

  /// Sets the canonical form of the subscript base.
  void setBaseCE(CanonExpr *BaseCE) {
    createGEP();
    getGEPInfo()->BaseCE = BaseCE;
  }

  /// Removes all the blob ddrefs and clears the canonical form of this RegDDRef
  /// so it represents constant 0 or null. RegDDRef must be a terminal ref.
  void clear(bool AssumeLvalIfDetached = false);

  /// Returns true if the inbounds attribute is set for this access.
  bool isInBounds() const { return getGEPInfo()->InBounds; }

  /// Sets the inbounds attribute for this access.
  void setInBounds(bool IsInBounds) {
    createGEP();
    getGEPInfo()->InBounds = IsInBounds;
  }

  /// Returns true if this is an address computation.
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

  unsigned getAlignment() const { return getGEPInfo()->Alignment; }

  void setAlignment(unsigned Align) {
    createGEP();
    getGEPInfo()->Alignment = Align;
  }

  /// Returns true if fake mem ref has known access type.
  bool canUsePointeeSize() const {
    assert (isFake() && "Fake ref expected");
    if (!hasGEPInfo())
      return false;
    return getGEPInfo()->CanUsePointeeSize;
  }

  /// Sets CanUsePointeeSize for this ref
  void setCanUsePointeeSize(bool Val) {
    assert (isFake() && "Fake ref expected");
    getGEPInfo()->CanUsePointeeSize = Val;
  }

  /// \brief Returns true if this is a collapsed ref.
  bool isCollapsed(void) const { return getGEPInfo()->IsCollapsed; }

  /// Sets collapse flag for this ref.
  void setCollapsed(bool CollapseFlag) {
    createGEP();
    getGEPInfo()->IsCollapsed = CollapseFlag;
  }

  /// Get/Set Max Vector Length Allowed. The field is set by collapsing pass to
  /// indicate dependence distance for collapsed refs. Value 0 means there is no
  /// information.
  unsigned getMaxVecLenAllowed(void) const {
    return getGEPInfo()->MaxVecLenAllowed;
  }
  void setMaxVecLenAllowed(unsigned DepDistance) {
    getGEPInfo()->MaxVecLenAllowed = DepDistance;
  }

  // Get/Set DebugLoc for the Load/Store instruction
  const DebugLoc &getMemDebugLoc() const { return getGEPInfo()->MemDbgLoc; }
  void setMemDebugLoc(const DebugLoc &Loc) { getGEPInfo()->MemDbgLoc = Loc; }

  // Get/Set DebugLoc for the GEP instruction
  const DebugLoc &getGepDebugLoc() const { return getGEPInfo()->GepDbgLoc; }
  void setGepDebugLoc(const DebugLoc &Loc) { getGEPInfo()->GepDbgLoc = Loc; }

  // Wrapper method to return relevant debug location.
  const DebugLoc &getDebugLoc() const {
    if (isTerminalRef())
      return getSingleCanonExpr()->getDebugLoc();
    return isAddressOf() ? getGepDebugLoc() : getMemDebugLoc();
  }

  /// Extract and submit AA metadata
  void getAAMetadata(AAMDNodes &AANodes) const;
  void setAAMetadata(const AAMDNodes &AANodes);

  /// Returns the metadata of given kind attached to this ref, else
  /// returns null.
  MDNode *getMetadata(StringRef Kind) const;
  MDNode *getMetadata(unsigned KindID) const;

  /// Returns all metadata attached to this ref.
  void getAllMetadata(MDNodesTy &MDs) const;

  /// Returns all metadata attached to this ref other than DebugLoc.
  void getAllMetadataOtherThanDebugLoc(MDNodesTy &MDs) const;

  /// Sets the metadata of the specific kind. This updates/replaces
  /// metadata if already present, or removes it if Node is null.
  void setMetadata(StringRef Kind, MDNode *Node);
  void setMetadata(unsigned KindID, MDNode *Node);

  /// Returns true if this RegDDRef is a constant integer.
  /// Val parameter is the value associated inside the CanonExpr
  /// of this RegDDRef
  bool isIntConstant(int64_t *Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isIntConstant(Val);
  }

  /// Returns true if this RegDDRef is a constant integer splat.
  /// Val parameter is the value associated inside the CanonExpr
  /// of this RegDDRef
  bool isIntConstantSplat(int64_t *Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isIntConstantSplat(Val);
  }

  /// Returns true if this RegDDRef represents an FP constant.
  /// Put the underlying LLVM Value in Val
  bool isFPConstant(ConstantFP **Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isFPConstant(Val);
  }

  /// Returns true if this RegDDRef represents a vector of constant FP values.
  /// Put the underlying LLVM Value in Val.
  bool isFPVectorConstant(Constant **Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isFPVectorConstant(Val);
  }

  /// Returns true if this RegDDRef represents a vector of constants.
  /// Put the underlying LLVM Value in Val
  bool isConstantVector(Constant **Val = nullptr) const {
    return isTerminalRef() && getSingleCanonExpr()->isConstantVector(Val);
  }

  /// Returns true if this RegDDRef represents a metadata.
  /// If true, metadata is returned in Val.
  bool isMetadata(MetadataAsValue **Val = nullptr) const override {
    return isTerminalRef() && getSingleCanonExpr()->isMetadata(Val);
  }

  /// Returns true if this RegDDRef represents null pointer.
  bool isNull() const {
    return isTerminalRef() && getSingleCanonExpr()->isNull();
  }

  /// Returns true if this scalar RegDDRef's canonical expr is any kind
  /// of constant. Please note that this is different than the DDRef itself
  /// being a constant which is represented by setting the symbase to
  /// CONSTANT_SYMBASE. Lval DDRefs can have constant canonical expr but cannot
  /// have CONSTANT_SYMBASE.
  bool isConstant() const {
    return isTerminalRef() && getSingleCanonExpr()->isConstant();
  }

  /// Returns true if ref is able to safely propagate a constant from lval
  /// to future rval. Currently supports scalar/vector types.
  ///
  bool isFoldableConstant() const {
    return isTerminalRef() && getSingleCanonExpr()->isFoldableConstant();
  }

  /// Returns the number of dimensions of the DDRef.
  unsigned getNumDimensions() const { return CanonExprs.size(); }

  /// Returns the only canon expr of this DDRef.
  CanonExpr *getSingleCanonExpr() override {
    assert(getNumDimensions() == 1);
    return *(canon_begin());
  }

  const CanonExpr *getSingleCanonExpr() const override {
    return const_cast<RegDDRef *>(this)->getSingleCanonExpr();
  }

  /// Returns true if this DDRef has only one dimension.
  bool isSingleDimension() const { return (getNumDimensions() == 1); }

  /// Updates the only Canon Expr of this RegDDRef
  void setSingleCanonExpr(CanonExpr *CE) {
    assert((getNumDimensions() == 0) && " RegDDRef already has one or more "
                                        "CanonExprs");
    assert(CE && "CE is null!");
    CanonExprs.push_back(CE);
  }

  /// CanonExpr iterator methods
  canon_iterator canon_begin() { return CanonExprs.begin(); }
  const_canon_iterator canon_begin() const { return CanonExprs.begin(); }
  canon_iterator canon_end() { return CanonExprs.end(); }
  const_canon_iterator canon_end() const { return CanonExprs.end(); }
  iterator_range<canon_iterator> canons() {
    return make_range(canon_begin(), canon_end());
  }
  iterator_range<const_canon_iterator> canons() const {
    return make_range(canon_begin(), canon_end());
  }

  reverse_canon_iterator canon_rbegin() { return CanonExprs.rbegin(); }
  const_reverse_canon_iterator canon_rbegin() const {
    return CanonExprs.rbegin();
  }
  reverse_canon_iterator canon_rend() { return CanonExprs.rend(); }
  const_reverse_canon_iterator canon_rend() const { return CanonExprs.rend(); }

  /// BlobDDRef iterator methods
  blob_iterator blob_begin() { return BlobDDRefs.begin(); }
  blob_iterator blob_end() { return BlobDDRefs.end(); }

  const_blob_iterator blob_begin() const { return BlobDDRefs.begin(); }
  const_blob_iterator blob_end() const { return BlobDDRefs.end(); }

  auto blobs() { return make_range(blob_begin(), blob_end()); }
  auto blobs() const { return make_range(blob_begin(), blob_end()); }

  reverse_blob_iterator blob_rbegin() { return BlobDDRefs.rbegin(); }
  reverse_blob_iterator blob_rend() { return BlobDDRefs.rend(); }

  const_reverse_blob_iterator blob_crbegin() const {
    return BlobDDRefs.rbegin();
  }
  const_reverse_blob_iterator blob_crend() const { return BlobDDRefs.rend(); }

  /// Dimension index iterator methods
  //    Iterates through dimension 1 to getNumDimensions(), inclusively.
  //     ex)
  //        for (auto I : make_range(Ref->dim_num_begin(),
  //        Ref->dim_num_end()))
  //          CanonExpr *CE = Ref->getNumDimension(I);
  IntegerRangeIterator dim_num_begin() const { return IntegerRangeIterator(1); }
  IntegerRangeIterator dim_num_end() const {
    return IntegerRangeIterator(getNumDimensions() + 1);
  }

  const_all_ddref_single_iterator all_dd_begin() const {
    return const_all_ddref_single_iterator(this);
  }
  const_all_ddref_single_iterator all_dd_end() const {
    return const_all_ddref_single_iterator(std::next(this));
  }

  bool hasBlobDDRefs() const { return !BlobDDRefs.empty(); }
  unsigned numBlobDDRefs() const { return BlobDDRefs.size(); }

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef *Ref) {
    return Ref->getDDRefID() == DDRef::RegDDRefVal;
  }

  /// clone() - Create a copy of 'this' RegDDRef that is identical in all
  /// ways except the following:
  ///   * The HLDDNode needs to be explicitly set
  RegDDRef *clone() const override;

  /// Returns true if this DDRef is a lval DDRef. This function
  /// assumes that the DDRef is connected to a HLDDNode.
  bool isLval() const override;

  /// Returns true if this DDRef is a fake DDRef. This function
  /// assumes that the DDRef is connected to a HLDDNode.
  bool isFake() const;

  /// Returns true if Ref is a fake lval DDRef. This function assumes that the
  /// DDRef is connected to a HLDDNode.
  bool isFakeLval() const;

  /// Returns true if Ref is a fake rval DDRef. This function assumes that the
  /// DDRef is connected to a HLDDNode.
  bool isFakeRval() const;

  /// Returns true if Ref executed under a mask. This function assumes that the
  /// DDRef is connected to a HLDDNode.
  bool isMasked() const;

  /// This method checks if the DDRef is
  /// not a memory reference or a pointer reference
  /// Returns false for:
  ///      RegDDRef is Memory Reference - A[i]
  ///      RegDDRef is a Pointer Reference - *p
  /// Else returns true for cases like DDRef - 2*i and M+N.
  bool isTerminalRef() const override {
    if (!hasGEPInfo()) {
      assert(isSingleDimension() &&
             "Terminal ref has more than one dimension!");
      return true;
    }
    return false;
  }

  /// Returns true if the DDRef is structurally invariant at \p Level.
  /// If \p IgnoreInnerIVs is true, inner loop IVs are ignored.
  /// Note: It does not check data-dependences, so there may be cases where
  /// the  DDRef is structurally invariant, but not actually invariant. For
  /// example, in the loop below, A[5] is structurally invariant, but not
  /// actually invariant because of the data-dependence:
  /// for (i=0; i<10; i++) { A[i] = A[5] + i;}
  bool isStructurallyInvariantAtLevel(unsigned Level,
                                      bool IgnoreInnerIVs = false) const;

  /// Returns true if the ref is structurally invariant in the HLRegion.
  bool isStructurallyRegionInvariant() const;

  /// Returns true if the DDRef is a memory reference
  bool isMemRef() const { return hasGEPInfo() && !isAddressOf(); }

  /// Returns true if the RegDDRef represents a standalone IV like (1 *
  /// i3).
  /// If \p AllowConversion is true, conversions are allowed to be part of a
  /// standalone IV.
  /// Returns the level of the IV in \p Level.
  bool isStandAloneIV(bool AllowConversion = true,
                      unsigned *Level = nullptr) const {
    return isTerminalRef() &&
           getSingleCanonExpr()->isStandAloneIV(AllowConversion, Level);
  }

  /// Returns true if the DDRef represents a self-blob like (1 * %t). In
  /// addition DDRef's symbase should be the same as %t's symbase. This is so
  /// because for some livein copies %t1 = %t2, lval %t1 is parsed as 1 * %t2.
  /// But since %t1 has a different symbase than %t2 we still need to add a blob
  /// DDRef for %t2 to the DDRef.
  bool isSelfBlob() const override;

  /// Returns true if the DDRef cannot be decomposed further into simpler
  /// operations. Non-decomposable refs include LHS terminal refs and RHS
  /// unitary blobs.
  bool isNonDecomposable() const override;

  /// Returns true if this ref looks like 1 * undef.
  bool isStandAloneUndefBlob() const override;

  /// Returns true if the DDRef represents a blob like (1 * %t).
  /// This is a broader check than isSelfBlob() because DDRef's symbase is not
  /// taken into account. In addition, a standalone blob allows a FP constant or
  /// even metadata.
  /// If \p AllowConversion is true, conversions are allowed to be part of a
  /// standalone blob. Otherwise, a blob with a conversion is not considered a
  /// standalone blob.
  bool isStandAloneBlob(bool AllowConversion = true) const;

  /// Return true if the DDRef represents a constant 0.
  bool isZero() const {
    return isTerminalRef() && getSingleCanonExpr()->isZero();
  }

  /// Return true if the DDRef represents a constant 1.
  bool isOne() const {
    return isTerminalRef() && getSingleCanonExpr()->isOne();
  }

  /// Return true if the DDRef represents a constant -1.
  bool isMinusOne() const {
    return isTerminalRef() && getSingleCanonExpr()->isMinusOne();
  }

  /// Returns true if this DDRef contains undefined canon expressions.
  bool containsUndef() const override;

  /// Adds a dimension to the DDRef with optional trailing offsets. The new
  /// dimension becomes the lowest dimension of the ref. For example, if the
  /// ref looks like A[i1] before the call, it will look like A[i1][0] after
  /// adding a zero canon expr as an additional dimension.
  void addDimension(CanonExpr *IndexCE, ArrayRef<unsigned> TrailingOffsets = {},
                    CanonExpr *LowerBoundCE = nullptr,
                    CanonExpr *StrideCE = nullptr, Type *DimTy = nullptr,
                    Type *DimElemTy = nullptr, bool IsExactMultiple = true);

  /// Sets trailing offsets for \p DimensionNum.
  void setTrailingStructOffsets(unsigned DimensionNum,
                                ArrayRef<unsigned> Offsets);

  /// Returns trailing offsets for \p DimensionNum. The array would be empty if
  /// there are no offsets.
  ArrayRef<unsigned> getTrailingStructOffsets(unsigned DimensionNum) const;

  /// Removes trailing offsets for \p DimensionNum.
  void removeTrailingStructOffsets(unsigned DimensionNum) {
    // Sets an empty vector which is equivalent to no offsets.
    OffsetsTy Offsets;
    setTrailingStructOffsets(DimensionNum, Offsets);
  }

  /// Returns true if the Ref has trailing offsets for \p DimensionNum.
  bool hasTrailingStructOffsets(unsigned DimensionNum) const {
    return !getTrailingStructOffsets(DimensionNum).empty();
  }

  /// Returns true if \p DimensionNum has non-zero trailing offsets. For
  /// example, it will return true for A[i].1, A[i].0.1 and false for A[i].0 and
  /// A[i].0.0.
  bool hasNonZeroTrailingStructOffsets(unsigned DimensionNum) const;

  /// Returns true if the Ref has trailing offsets for any dimension.
  bool hasTrailingStructOffsets() const;

  /// Returns the number of elements for specified dimension. 0 is returned if
  /// it is unknown. DimensionNum must be within [1, getNumDimensions()].
  unsigned getNumDimensionElements(unsigned DimensionNum) const;

  /// Returns the stride in number of bytes for specified dimension if it is
  /// constant, else returns 0. DimensionNum must be within [1,
  /// getNumDimensions()].
  int64_t getDimensionConstStride(unsigned DimensionNum) const;

  /// Returns the stride in number of bytes associated with the \p DimensionNum.
  /// DimensionNum must be within [1, getNumDimensions()].
  CanonExpr *getDimensionStride(unsigned DimensionNum) {
    assert(!isTerminalRef() && "Stride info not applicable for scalar refs!");
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

    return GepInfo->Strides[DimensionNum - 1];
  }

  const CanonExpr *getDimensionStride(unsigned DimensionNum) const {
    return const_cast<RegDDRef *>(this)->getDimensionStride(DimensionNum);
  }

  /// Returns the lower bound of specified dimension if it is
  /// constant, else returns 0. DimensionNum must be within [1,
  /// getNumDimensions()].
  int64_t getDimensionConstLower(unsigned DimensionNum) const;

  /// Returns the lower bound associated with the \p DimensionNum. DimensionNum
  /// must be within [1, getNumDimensions()].
  CanonExpr *getDimensionLower(unsigned DimensionNum) {
    assert(!isTerminalRef() && "Stride info not applicable for scalar refs!");
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");

    return GepInfo->LowerBounds[DimensionNum - 1];
  }

  const CanonExpr *getDimensionLower(unsigned DimensionNum) const {
    return const_cast<RegDDRef *>(this)->getDimensionLower(DimensionNum);
  }

  /// Returns the size in number of bytes for specified dimension.
  /// 0 is returned for pointer dimension.
  /// DimensionNum must be within [1, getNumDimensions()].
  uint64_t getDimensionSize(unsigned DimensionNum) const {
    return getDimensionConstStride(DimensionNum) *
           getNumDimensionElements(DimensionNum);
  }

  /// Returns the canon expr (dimension) of this DDRef at specified
  /// position. DimensionNum must be within [1, getNumDimensions()].
  CanonExpr *getDimensionIndex(unsigned DimensionNum) {
    assert(isDimensionValid(DimensionNum) && " DimensionNum is invalid!");
    return CanonExprs[DimensionNum - 1];
  }

  const CanonExpr *getDimensionIndex(unsigned DimensionNum) const {
    return const_cast<RegDDRef *>(this)->getDimensionIndex(DimensionNum);
  }

  bool isStrideExactMultiple(unsigned DimensionNum) const {
    return GepInfo->StrideIsExactMultiple[DimensionNum - 1];
  }

  /// Returns true if the dimension associated with \p DimensionNum represent
  /// LLVM array.
  bool isDimensionLLVMArray(unsigned DimensionNum) const {
    return getDimensionType(DimensionNum)->isArrayTy();
  }

  /// Returns the stride in number of bytes of this DDRef at specified loop
  /// level. Returns null if DDRef might not be a regular strided access (linear
  /// access with invariant stride at Level).
  CanonExpr *getStrideAtLevel(unsigned Level) const;

  /// Populates constant stride in number of bytes of DDRef at \p Level in \p
  /// Stride, if is is not null. Returns false if the stride is not constant.
  bool getConstStrideAtLevel(unsigned Level, int64_t *Stride) const;

  /// Returns true if ref has unit stride at \p Level, such as A[i1] or
  /// &A[-1 * i1]. \p IsNegStride is set when stride is -1.
  bool isUnitStride(unsigned Level, bool &IsNegStride) const;

  /// Removes a dimension from the DDRef.
  /// DimensionIndex's range is [1, getNumDimensions()] with 1 representing the
  /// lowest dimension.
  void removeDimension(unsigned DimensionIndex);

  /// Replaces existing self blob index with \p NewIndex and corresponding SB.
  void replaceSelfBlobIndex(unsigned NewIndex);

  /// Replaces existing self blob index with \p NewIndex and Constant SB.
  void replaceSelfBlobByConstBlob(unsigned NewIndex);

  /// Converts a terminal lval ref into a self blob ref using its symbase.
  /// For example, if we have t1 = t2 + t3, where t1's canonical form is (1 * t2
  /// + 1 * t3), it will be converted to 1 * t1.
  /// The ref is asserted as an lval unless it is detached and
  /// AssumeLvalIfDetached is set to true.
  void makeSelfBlob(bool AssumeLvalIfDetached = false);

  /// Adds a blob DDRef to this DDRef.
  void addBlobDDRef(BlobDDRef *BlobRef);

  /// Creates a blob DDRef with passed in Index and Level and adds it to
  /// this DDRef.
  void addBlobDDRef(unsigned Index, unsigned Level = NonLinearLevel);

  /// Returns the blob DDRef with \p Index attached to this RegDDRef.
  /// It returns null if the blob DDRef is not found.
  BlobDDRef *getBlobDDRef(unsigned Index);
  const BlobDDRef *getBlobDDRef(unsigned Index) const;

  // Returns the single non-linear blob that is seen in this RegDDRef. Returns
  // nullptr if zero or multiple non-linear blob is found.
  const BlobDDRef *getSingleNonLinearBlobRef() const;

  /// Removes and returns blob DDRef corresponding to CBlobI iterator.
  BlobDDRef *removeBlobDDRef(const_blob_iterator CBlobI);

  /// Remove and return attached BlobDDRef based on Blob Index
  BlobDDRef *removeBlobDDRefWithIndex(unsigned Index);

  /// Replaces temp blob with \p OldIndex by new temp blob with \p NewIndex, if
  /// it exists in DDRef. Returns true if it is replaced.
  /// If the ref is a terminal lval ref and \p OldIndex corresponds to the
  /// symbase of the Ref, it is assumed as a use of the temp. This is relavant
  /// for instructions such as: t = i1 + 1, where 't' has a linear form.
  /// Note: The new temp is assumed to have the same def level as the old temp.
  /// For constants, see replaceSelfBlobByConstBlob()
  bool replaceTempBlob(unsigned OldIndex, unsigned NewIndex,
                       bool AssumeLvalIfDetached = false);

  /// Replaces temp blobs using pairs (OldIndex, NewIndex) in \p BlobMap.
  /// Returns true if any blob is replaced.
  bool replaceTempBlobs(
      const SmallVectorImpl<std::pair<unsigned, unsigned>> &BlobMap,
      bool AssumeLvalIfDetached = false);

  bool replaceTempBlobs(const DenseMap<unsigned, unsigned> &BlobMap,
                        bool AssumeLvalIfDetached = false);

  /// Replaces temp blob with int constant
  bool replaceTempBlobByConstant(unsigned OldIndex, int64_t Constant);

  /// Removes all blob DDRefs attached to this DDRef.
  void removeAllBlobDDRefs();

  /// Returns true if there is a use of temp blob with \p Index in the DDRef.
  /// IsSelfBlob is set to true if the DDRef is a self blob.
  /// If the ref is a terminal lval ref and \p Index corresponds to the symbase
  /// of the Ref, it is assumed as a use of the temp. This is relavant for
  /// instructions such as: t = i1 + 1, where 't' has a linear form.
  bool usesTempBlob(unsigned Index, bool *IsSelfBlob = nullptr,
                    bool AssumeLvalIfDetached = false) const;

  /// Returns true if there is a use of a temp blob with \p Symbase.
  /// Calls usesTempBlob (above).
  bool usesSymbase(unsigned Symbase) const;

  /// Collects all the unique temp blobs present in the DDRef by visiting
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

  /// Updates BlobDDRefs for this DDRef by going through the blobs in the
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

  /// Method to update CE def levels, if necessary. This should be called
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

  /// Makes a modified ref internally consistent by updating blob DDRefs
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
  void makeConsistent(ArrayRef<const RegDDRef *> AuxRefs = {},
                      unsigned NewLevel = NonLinearLevel);

  /// Returns true if the blob is present in this DDRef and returns its
  /// defined at level via DefLevel. DefLevel is expected to be non-null. The
  /// blob is searched in the blob DDRefs attached to this DDRef. This function
  /// can be used to update defined at levels for blobs which were copied from
  /// this DDRef to another DDRef.
  bool findTempBlobLevel(unsigned BlobIndex, unsigned *DefLevel) const;

  /// Returns maximum blob level amongst the blobs in the vector.
  /// NOTE: this function asserts if any of the temp blobs is not contained in
  /// the DDRef.
  unsigned
  findMaxTempBlobLevel(const SmallVectorImpl<unsigned> &TempBlobIndices) const;

  /// Returns maximum blob level of temp blobs contained in this blob.
  /// NOTE: This function asserts if any of the temp blobs is not contained in
  /// the DDRef.
  unsigned findMaxBlobLevel(unsigned BlobIndex) const;

  /// Returns true if ref has an IV at \p Level.
  bool hasIV(unsigned Level) const;

  /// Returns true if ref contains any IV.
  bool hasIV() const;

  /// Returns the defined at level of the ref.
  unsigned getDefinedAtLevel() const override;

  /// Replace any loop-level IV by a given constant integer.
  void replaceIVByConstant(unsigned LoopLevel, int64_t Val);

  /// A RegDDRef is linear if all of the following is true:
  /// - its baseCE (if available) is linear
  /// - any CE is linear
  bool isLinear(void) const { return !isNonLinear(); }

  /// A RegDDRef is nonlinear if any of the following is true:
  /// - its baseCE (if available) is nonlinear
  /// - any CE is nonlinear
  bool isNonLinear(void) const { return getDefinedAtLevel() == NonLinearLevel; }

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

  /// Demote nesting levels of all IVs in RegDDRef. Asserts if new IV level
  /// becomes invalid.
  /// E.g.
  /// i2 -> i1, i3 -> i2, ...
  ///
  /// See Also: CanonExpr::demoteIVs();
  void promoteIVs(unsigned StartLevel);
  void demoteIVs(unsigned StartLevel);

  /// Returns true if any of the dimension indices are vector type.
  /// Only applicable to GEP refs.
  bool hasAnyVectorIndices() const;

  /// Fortran only. Returns true if any of the dimension strides may be zero as
  /// a result of PRODUCT or SUM function. Only applicable to GEP refs.
  bool anyVarDimStrideMayBeZero() const;

  void setAnyVarDimStrideMayBeZero(bool Val) {
    assert(hasGEPInfo() && "Mem ref expected");
    getGEPInfo()->AnyVarDimStrideMayBeZero = Val;
  }

  /// Verifies RegDDRef integrity.
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
