//===-------- DDRefUtils.h - Utilities for DDRef class ---*- C++ -*--------===//
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
// This file defines the utilities for DDRef class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFUTILS_H

#include <map>

#include "llvm/Support/Compiler.h"

#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"

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
private:
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

  /// Destroys all DDRefs. Called during HIR cleanup.
  void destroyAll();

  /// Creates a non-linear self blob RegDDRef from the passed in Value.
  /// Temp blobs from values are only created by framework.
  RegDDRef *createSelfBlobRef(Value *Temp);

  /// Return true if RegDDRef1 equals RegDDRef2.
  /// This routine compares the symbase, type and each of the canon exprs inside
  /// the references.
  static bool areEqualImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                           bool RelaxedMode);

  /// Returns true if BlobDDRef1 equals BlobDDRef2.
  static bool areEqualImpl(const BlobDDRef *Ref1, const BlobDDRef *Ref2);

  /// Implements getConst*Distance() functionality.
  static bool getConstDistanceImpl(const RegDDRef *Ref1, const RegDDRef *Ref2,
                                   unsigned LoopLevel, int64_t *Distance);

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

  /// Returns a new constant RegDDRef from a int value.
  /// This routine will automatically create a single canon expr from the val
  /// and attach it to the new RegDDRef.
  RegDDRef *createConstDDRef(Type *Ty, int64_t Val);

  /// Returns a new constant RegDDRef from a value representing some form of constant.
  /// This routine will automatically create a single canon expr from metadata
  /// and attach it to the new RegDDRef.
  RegDDRef *createConstDDRef(Value *Val);

  /// Returns a new BlobDDRef representing blob with Index. Level is the defined
  /// at level for the blob.
  BlobDDRef *createBlobDDRef(unsigned Index, unsigned Level = NonLinearLevel);

  /// Returns a new RegDDRef representing blob with Index. Level is the defined
  /// at level for the blob.
  RegDDRef *createSelfBlobRef(unsigned Index, unsigned Level = NonLinearLevel);

  /// Destroys the passed in DDRef.
  void destroy(DDRef *Ref);

  /// Returns a generic rval symbase.
  unsigned getGenericRvalSymbase();

  /// Returns a brand new symbase.
  unsigned getNewSymbase();

  /// Returns true if the two DDRefs, Ref1 and Ref2, are equal.
  /// RelaxedMode is passed to CanonExprUtils::areEqual().
  static bool areEqual(const DDRef *Ref1, const DDRef *Ref2,
                       bool RelaxedMode = false);

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
  /// \param [out] Distance holds the constant distance in bytes if obtained.
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
                                   int64_t *Distance);

  /// Returns a constant distance in number of iterations at \p LoopLevel
  /// between \p Ref1 and \p Ref2.
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
                                        unsigned LoopLevel, int64_t *Distance);

  /// Returns the type obtained by applying element offsets from \p Offsets to
  /// \p Ty. This is a no-op for non-struct types.
  static Type *getOffsetType(Type *Ty,
                             const SmallVectorImpl<unsigned> &Offsets);

  /// Given a type and field offset numbers, calculates the total byte offset.
  static int64_t getOffsetDistance(Type *Ty, const DataLayout &DL,
                                   const SmallVectorImpl<unsigned> &Offsets);

  /// Given two sets of offsets returns negative, positive or zero value based
  /// on whether \p Offset1 has lower, higher or equal total byte offset than \p
  /// Offset2. This is useful for ordering DDRefs.
  static int compareOffsets(const SmallVectorImpl<unsigned> &Offsets1,
                            const SmallVectorImpl<unsigned> &Offsets2);

  /// Returns negative, positive or zero value based on whether \p Ref1 has
  /// lower, higher or equal total byte offset than \p Ref2 at \p DimensionNum.
  /// This is useful for ordering DDRefs.
  static int compareOffsets(const RegDDRef *Ref1, const RegDDRef *Ref2,
                            unsigned DimensionNum);

  // Sorting comparator operator for two Mem-RegDDRef.
  static bool compareMemRef(const RegDDRef *Ref1, const RegDDRef *Ref2);

  /// Check if replaceIVByCanonExpr(.) can actually succeed without doing it for
  /// real.
  ///
  /// Return: bool
  /// - true: if replacIVByCanonExpr() succeeds on each loop-level IV in Ref
  /// -false: otherwise
  static bool canReplaceIVByCanonExpr(const RegDDRef *Ref, unsigned LoopLevel,
                                      const CanonExpr *CE,
                                      bool RelaxedMode = true);

  /// Replace any IV in the Ref with a given CanonExpr*.
  ///(e.g. A[i]->A[CE], A[i+2]->A[CE+2] )
  ///
  /// Note: The function asserts if the replacement fails as the Ref may be in
  /// an inconsistent state. Caller should call canReplaceIVByCanonExpr() first
  /// to make sure this is safe to do.
  static void replaceIVByCanonExpr(RegDDRef *Ref, unsigned LoopLevel,
                                   const CanonExpr *CE,
                                   bool RelaxedMode = true);
};

} // End namespace loopopt

} // End namespace llvm

#endif
