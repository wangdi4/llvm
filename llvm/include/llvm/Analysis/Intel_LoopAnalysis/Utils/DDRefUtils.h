//===-------- DDRefUtils.h - Utilities for DDRef class ---*- C++ -*--------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for DDRef class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H

#include <map>

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/BlobDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Support/Compiler.h"

namespace llvm {

class ConstantAggregateZero;
class ConstantDataVector;
class ConstantVector;
class Function;
class Module;
class LLVMContext;
class DataLayout;

namespace loopopt {

class HIRParser;
class HIRSymbaseAssignment;

/// Defines utilities for DDRef class and manages their creation/destruction.
/// It contains a bunch of member functions which manipulate DDRefs.
class DDRefUtils {
  /// Keeps track of DDRef objects.
  std::set<DDRef *> Objs;

  CanonExprUtils CEU;

  DDRefUtils(HIRParser &HIRP) : CEU(HIRP) {}

  /// Make class uncopyable.
  DDRefUtils(const DDRefUtils &) = delete;
  void operator=(const DDRefUtils &) = delete;

  // Requires access to Objs.
  friend class DDRef;
  friend class HIRParser;
  friend class HLNodeUtils;
  // Sets itself.
  friend class HIRSymbaseAssignment;

  HIRParser &getHIRParser() { return getCanonExprUtils().getHIRParser(); }
  const HIRParser &getHIRParser() const {
    return getCanonExprUtils().getHIRParser();
  }

  /// Destroys all DDRefs.
  ~DDRefUtils();

  /// Creates a non-linear self blob RegDDRef from the passed in Value.
  /// Temp blobs from values are only created by framework.
  RegDDRef *createSelfBlobRef(Value *Temp);

  /// Return true if RegDDRef1 equals RegDDRef2.
  /// This routine compares the symbase, type and each of the canon exprs inside
  /// the references.
  static bool areEqualImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                           bool RelaxedMode, bool IgnoreAddressOf = false);

  /// Returns true if BlobDDRef1 equals BlobDDRef2.
  static bool areEqualImpl(const BlobDDRef *Ref1, const BlobDDRef *Ref2);

  /// Implements getConst*Distance() functionality.
  static bool getConstDistanceImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                                   unsigned LoopLevel, int64_t *Distance,
                                   bool RelaxedMode);

  /// Implements createMemRef()/createAddressOfRef().
  RegDDRef *createGEPRef(unsigned BasePtrBlobIndex, unsigned Level, unsigned SB,
                         bool IsMemRef, bool IsInBounds);

public:
  // Returns reference to CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() { return CEU; }
  const CanonExprUtils &getCanonExprUtils() const { return CEU; }

  // Returns reference to BlobUtils object.
  BlobUtils &getBlobUtils() { return getCanonExprUtils().getBlobUtils(); }
  const BlobUtils &getBlobUtils() const {
    return getCanonExprUtils().getBlobUtils();
  }

  /// Returns Function object.
  Function &getFunction() const;

  /// Returns Module object.
  Module &getModule() const;

  /// Returns LLVMContext object.
  LLVMContext &getContext() const;

  /// Returns DataLayout object.
  const DataLayout &getDataLayout() const;

  /// Returns a new RegDDRef.
  RegDDRef *createRegDDRef(unsigned SB);

  /// Creates a new DDRef with single canon expr CE.
  RegDDRef *createScalarRegDDRef(unsigned SB, CanonExpr *CE);

  /// Create a memref using the \p BasePtrBlobIndex as the blob index of the
  /// base pointer. \p Level is the defined at level of the base pointer. If no
  /// symbase is supplied by the caller, a new one is assigned to the ref. No
  /// dimensions are added to the ref. Caller is responsible for doing that
  /// using RegDDRef::addDimension().
  RegDDRef *createMemRef(unsigned BasePtrBlobIndex, unsigned Level = 0,
                         unsigned SB = InvalidSymbase, bool IsInBounds = true);

  /// Create an addressOf ref using the \p BasePtrBlobIndex as the blob index of
  /// the base pointer. \p Level is the defined at level of the base pointer. If
  /// no symbase is supplied by the caller, a new one is assigned to the ref. No
  /// dimensions are added to the ref. Caller is responsible for doing that
  /// using RegDDRef::addDimension().
  RegDDRef *createAddressOfRef(unsigned BasePtrBlobIndex, unsigned Level = 0,
                               unsigned SB = InvalidSymbase,
                               bool IsInBounds = true);

  /// Returns a new constant RegDDRef from a int value.
  /// This routine will automatically create a single canon expr from the val
  /// and attach it to the new RegDDRef.
  RegDDRef *createConstDDRef(Type *Ty, int64_t Val);

  /// Returns a new constant RegDDRef from a value representing some form of
  /// constant. This routine will automatically create a single canon expr from
  /// metadata and attach it to the new RegDDRef.
  RegDDRef *createConstDDRef(Value *Val);

  /// Creates a ref representing '0' for \p Ty wich may be int, ptr, fp etc.
  RegDDRef *createNullDDRef(Type *Ty);

  /// Returns a RegDDRef representing an undef value with type \p Type.
  RegDDRef *createUndefDDRef(Type *Type);

  /// Returns a new BlobDDRef representing blob with Index. Level is the defined
  /// at level for the blob.
  BlobDDRef *createBlobDDRef(unsigned Index, unsigned Level = NonLinearLevel);

  /// Returns a new RegDDRef representing blob with Index. Level is the defined
  /// at level for the blob.
  RegDDRef *createSelfBlobRef(unsigned Index, unsigned Level = NonLinearLevel);

  /// Destroys the passed in DDRef.
  void destroy(DDRef *Ref);

  /// Returns a brand new symbase.
  unsigned getNewSymbase();

  /// Returns true if the two DDRefs, Ref1 and Ref2, are equal.
  /// RelaxedMode is passed to CanonExprUtils::areEqual().
  static bool areEqual(const DDRef *Ref1, const DDRef *Ref2,
                       bool RelaxedMode = false);

  /// Returns true if GEPRef1 and GEPRef2 are equal if we ignore the AddressOf
  /// flag. In addition to identical refs, this will also return true for &A[i]
  /// and A[i]. RelaxedMode is passed to CanonExprUtils::areEqual().
  static bool areEqualWithoutAddressOf(const RegDDRef *GEPRef1,
                                       const RegDDRef *GEPRef2,
                                       bool RelaxedMode = false) {
    assert(GEPRef1->hasGEPInfo() && "GEPRef1 is not a GEP ref!");
    assert(GEPRef2->hasGEPInfo() && "GEPRef2 is not a GEP ref!");
    return areEqualImpl(GEPRef1, GEPRef2, RelaxedMode, true);
  }

  /// Prints metadata nodes attached to RegDDRef.
  void printMDNodes(formatted_raw_ostream &OS,
                    const RegDDRef::MDNodesTy &MDNodes) const;

  /// Returns true if it is able to compute a constant distance between \p Ref1
  /// and \p Ref2.
  ///
  /// Context: This utility is called by optVLS, which tries to find neighboring
  /// vector loads/stores (the refs are not yet vectorized, but this is called
  /// at the point when we are considering to vectorize a certain loop).
  /// Normally it will be called for two memrefs that are strided and have the
  /// same stride (a[2*i], a[2*i+1]) or two memrefs that are indexed (indirect)
  /// and have the same index vector (a[b[i]], a[b[i]+1]). When each of these
  /// two refs is vectorized, we will need to generate a gather instruction for
  /// each. Instead, we want to examine whether we can load the neighboring
  /// elements of these two (vectorized) refs together with regular loads
  /// (followed by shuffles). The distance will tell us if we can fit two
  /// neighbors in the same vector register.
  ///
  /// \param [out] Distance holds the constant distance in bytes if it isn't
  /// null.
  /// The Distance can result from a difference in any of the subscripts --
  /// not only the innermost, and even in multiple subscripts. For example,
  /// the distance between a[2*i][j] and a[2*i+1][j+1] when 'a' is int a[8][8]
  /// is 36 bytes, which allows fitting both elements in one vector register.
  /// The caller will consider this and decide if it is more efficient to
  /// do that than to generate two separate gathers. A difference between
  /// struct accesses such as a[i].I and a[i].F where 'a' is an array of
  /// struct S {int I; float F;} will also be supported.
  ///
  /// NOTE: This is strictly a structural check in the sense that the utility is
  /// context insensitive. It doesn't perform HIR based checks and so will
  /// return a valid distance for non-linear CEs. Caller is responsible for
  /// doing extra analysis. For example, it will return a valid distance between
  /// A[i1+%t] and A[i1+%t+1] even if %t is non-linear and has a different value
  /// for the refs.
  static bool getConstByteDistance(const RegDDRef *Ref1, const RegDDRef *Ref2,
                                   int64_t *Distance = nullptr,
                                   bool RelaxedMode = false);

  /// Returns true if there is constant distance in number of iterations at \p
  /// LoopLevel between \p Ref1 and \p Ref2.
  /// Populates this distance in \p Distance is it isn't null.
  ///
  /// This is different that getConstDistanceInBytes() above in that the
  /// distance should be an exact multiple of iterations of loop. For example,
  /// it returns false for A[2*i1] and A[2*i1+1] as they do not overlap w.r.t
  /// i1.
  ///
  /// NOTE: This is strictly a structural check in the sense that the utility is
  /// context insensitive. It doesn't perform HIR based checks and so will
  /// return a valid distance for non-linear CEs. Caller is responsible for
  /// doing extra analysis. For example, it will return a valid distance between
  /// A[i1+%t] and A[i1+%t+1] even if %t is non-linear and has a different value
  /// for the refs.
  static bool getConstIterationDistance(const RegDDRef *Ref1,
                                        const RegDDRef *Ref2,
                                        unsigned LoopLevel,
                                        int64_t *Distance = nullptr,
                                        bool RelaxedMode = false);

  /// Returns the type obtained by applying element offsets from \p Offsets to
  /// \p Ty. This is a no-op for non-struct types.
  static Type *getOffsetType(Type *Ty, ArrayRef<unsigned> Offsets);

  /// Given a type and field offset numbers, calculates the total byte offset.
  static int64_t getOffsetDistance(Type *Ty, const DataLayout &DL,
                                   ArrayRef<unsigned> Offsets);

  /// Given two sets of offsets returns negative, positive or zero value based
  /// on whether \p Offset1 has lower, higher or equal total byte offset than \p
  /// Offset2. This is useful for ordering DDRefs.
  static int compareOffsets(ArrayRef<unsigned> Offsets1,
                            ArrayRef<unsigned> Offsets2);

  /// Returns negative, positive or zero value based on whether \p Ref1 has
  /// lower, higher or equal total byte offset than \p Ref2 at \p DimensionNum.
  /// This is useful for ordering DDRefs.
  static int compareOffsets(const RegDDRef *Ref1, const RegDDRef *Ref2,
                            unsigned DimensionNum);

  /// Compares struct offsets in \p Ref1 and \p Ref2 and returns true if they
  /// are equal. For example, it will returns true for references like A[0].1
  /// and A[i1].1. Both refs should have same base and number of dimensions.
  static bool haveEqualOffsets(const RegDDRef *Ref1, const RegDDRef *Ref2);

  /// Compares BaseCE and number of dimension.
  /// Preliminary check for index comparison.
  static bool haveEqualBaseAndShape(const RegDDRef *Ref1, const RegDDRef *Ref2,
                                    bool RelaxedMode);

  // Sorting comparator operator for two Mem-RegDDRef, placing LVals before
  // RVals.
  static bool compareMemRef(const RegDDRef *Ref1, const RegDDRef *Ref2);

  // Sorting comparator operator for two Mem-RegDDRef, ignoring LVal and RVal
  // attribute.
  static bool compareMemRefAddress(const RegDDRef *Ref1, const RegDDRef *Ref2);

  /// Check if replaceIVByCanonExpr(.) can actually succeed without doing it for
  /// real.
  ///
  /// Return: bool
  /// - true: if replacIVByCanonExpr() succeeds on each loop-level IV in Ref
  /// -false: otherwise
  ///
  /// See Also: CanonExprUtils::canReplaceIVByCanonExpr()
  static bool canReplaceIVByCanonExpr(const RegDDRef *Ref, unsigned LoopLevel,
                                      const CanonExpr *CE,
                                      bool RelaxedMode = true);

  /// Replace any IV in the Ref with a given CanonExpr*.
  /// (e.g. A[i]->A[CE], A[i+2]->A[CE+2] )
  ///
  /// Note: The function asserts if the replacement fails as the Ref may be in
  /// an inconsistent state. Caller should call canReplaceIVByCanonExpr() first
  /// to make sure this is safe to do.
  ///
  /// See Also: CanonExprUtils::replaceIVByCanonExpr().
  static void replaceIVByCanonExpr(RegDDRef *Ref, unsigned LoopLevel,
                                   const CanonExpr *CE, bool IsNSW,
                                   bool RelaxedMode = true);

  /// Transform input single-dimension gep \p Refs to the multi-dimensional \p
  /// OutRefs.
  ///
  /// for (long i = 0; i < x1; i++)
  ///   for (long j = 0; j < x2; j++)
  ///     for (long k = 0; k < x3; k++) {
  ///
  ///       A[m*n*i + n*j + k] = 1.0;
  ///        -->
  ///       A[i][j][k] = 1.0 // with dimension strides: (m*n), (n), (1).
  ///
  ///     }
  ///
  /// Note: the caller have to check the correctness of such mapping-
  /// For all i,j,k: (0 <= i < x1) && (0 <= j <  x2) && (0 <= k < x3):
  ///                (0 <= k <  n) && (0 <= j < m*n) && (0 <= i)
  static bool delinearizeRefs(ArrayRef<const loopopt::RegDDRef *> GepRefs,
                              SmallVectorImpl<loopopt::RegDDRef *> &OutRefs,
                              SmallVectorImpl<BlobTy> *SizesPtr = nullptr);
};

} // End namespace loopopt

} // End namespace llvm

#endif
