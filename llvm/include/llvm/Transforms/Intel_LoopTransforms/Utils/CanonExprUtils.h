//===------ CanonExprUtils.h - Utilities for CanonExpr class --*- C++ -*---===//
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
// This file defines the utilities for CanonExpr class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_CANONEXPRUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_CANONEXPRUTILS_H

#include "llvm/Support/Compiler.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

#include <set>

namespace llvm {

class Type;
class APInt;
class ConstantFP;
class Function;
class Module;
class LLVMContext;
class DataLayout;

namespace loopopt {

class HIRParser;

/// Defines utilities for CanonExpr class and manages their
/// creation/destruction.
///
/// It contains a bunch of member functions which manipulate CanonExprs.
///
/// RelaxedMode : This flag is used in a number of methods across this file.
/// The flag indicates if the types of canon expr can be relaxed for
/// computation.
/// For example, constant canon exprs, CE1 = 0 (srcTy = i32, dstTy = i64) and
/// CE2 = 1 (srcTy = i64, dstTy = i64), can be added in relaxed mode. Clients
/// can check canon expr with mergeable call.
class CanonExprUtils {
private:
  /// Keeps track of CanonExpr objects.
  std::set<CanonExpr *> Objs;

  BlobUtils BU;

  CanonExprUtils(HIRParser &HIRP) : BU(HIRP) {}

  /// Make class uncopyable.
  CanonExprUtils(const CanonExprUtils &) = delete;
  void operator=(const CanonExprUtils &) = delete;

  // Requires access to Objs.
  friend class CanonExpr;
  friend class DDRefUtils; // accesses createSelfBlobCanonExpr()
  friend class HIRParser;  // accesses destroyAll()

  HIRParser &getHIRParser() { return getBlobUtils().getHIRParser(); }
  const HIRParser &getHIRParser() const {
    return getBlobUtils().getHIRParser();
  }

  /// Destroys all CanonExprs. Called during HIR cleanup.
  void destroyAll();

  /// Calculates the lcm of two positive inputs. Returns zero on overflow.
  static int64_t lcm(int64_t A, int64_t B);

  /// Creates a non-linear self blob canon expr from the passed in \p Val.
  /// The new blob is associated with symbase. New temp blobs from values are
  /// only created by framework.
  CanonExpr *createSelfBlobCanonExpr(Value *Val, unsigned Symbase);

  /// Returns a standalone blob canon expr representing a constant like
  /// ConstantVector, Metadata or Undef.
  CanonExpr *createConstStandAloneBlobCanonExpr(Value *Val);

  /// Returns a standalone blob canon expr. Level is the defined at level for
  /// the blob.
  CanonExpr *createStandAloneBlobCanonExpr(unsigned Index, unsigned Level);

  /// Returns true if constant canon expr type can be updated to match the
  /// source type. For any other types or non-mergeable cases, it returns false.
  static bool canMergeConstants(const CanonExpr *CE1, const CanonExpr *CE2,
                                bool RelaxedMode);

  /// Updates the src type of \p CE1 if necessary to perform addition with \p
  /// CE2.
  static void updateSrcType(CanonExpr *CE1, const CanonExpr *CE2,
                            bool RelaxedMode);

  /// Implements add() functionality.
  /// This routine asserts on canAddOrSubtract(CE1, CE2) so the caller is
  /// responsible for making sure they can be added.
  static void addImpl(CanonExpr *CE1, const CanonExpr *CE2, bool RelaxedMode);

public:
  // Returns reference to BlobUtils object.
  BlobUtils &getBlobUtils() { return BU; }
  const BlobUtils &getBlobUtils() const { return BU; }

  /// Returns Function object.
  Function &getFunction() const;

  /// Returns Module object.
  Module &getModule() const;

  /// Returns LLVMContext object.
  LLVMContext &getContext() const;

  /// Returns DataLayout object.
  const DataLayout &getDataLayout() const;

  /// Returns a new CanonExpr with identical src and dest types. All canon exprs
  /// are created linear by default.
  CanonExpr *createCanonExpr(Type *Ty, unsigned Level = 0, int64_t Const = 0,
                             int64_t Denom = 1, bool IsSignedDiv = false);

  /// Returns a new CanonExpr with zero or sign extension. All canon exprs are
  /// created linear by default.
  /// Note: Overloading createCanonExpr() causes ambiguous calls for constant
  /// arguments.
  CanonExpr *createExtCanonExpr(Type *SrcType, Type *DestType, bool IsSExt,
                                unsigned Level = 0, int64_t Const = 0,
                                int64_t Denom = 1, bool IsSignedDiv = false);

  /// Returns a new linear CanonExpr created from APInt Value.
  CanonExpr *createCanonExpr(Type *Ty, APInt APVal);

  /// Returns a self-blob canon expr. Level is the defined at level for the
  /// blob.
  CanonExpr *createSelfBlobCanonExpr(unsigned Index,
                                     unsigned Level = NonLinearLevel) {
    return createStandAloneBlobCanonExpr(Index, Level);
  }

  /// Destroys the passed in CanonExpr.
  void destroy(CanonExpr *CE);

  /// Calculates the gcd of two positive inputs.
  static int64_t gcd(int64_t A, int64_t B);

  /// Returns the size of the type in bits.
  /// NOTE: This function asserts that the incoming type is sized.
  uint64_t getTypeSizeInBits(Type *Ty) const;

  /// Returns true if the type of both Canon Expr matches.
  static bool isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                          bool RelaxedMode = false);

  /// Returns true if CE1 and CE2 can be merged (added/subtracted etc).
  static bool mergeable(const CanonExpr *CE1, const CanonExpr *CE2,
                        bool RelaxedMode = false);

  /// Returns true if passed in canon cxprs are equal to each other.
  /// Ignores dest types of CE1 and CE2 if IgnoreDestType is set.
  static bool areEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                       bool RelaxedMode = false);

  /// Returns true or false depending upon whether CE1 and CE2 can be added or
  /// subtracted.
  static bool canAddOrSubtract(const CanonExpr *CE1, const CanonExpr *CE2,
                               bool RelaxedMode = false);

  /// Modifies and returns CE1 to reflect sum of CE1 and CE2.
  /// CE1 = CE1 + CE2
  /// This routine returns false if the canon exprs are not mergeable.
  static bool add(CanonExpr *CE1, const CanonExpr *CE2,
                  bool RelaxedMode = false);

  /// Returns a canon expr which represents the sum of CE1 and CE2.
  /// Result = CE1 + CE2
  /// This routine can return nullptr, if the canon exprs are not mergeable.
  static CanonExpr *cloneAndAdd(const CanonExpr *CE1, const CanonExpr *CE2,
                                bool RelaxedMode = false);

  /// Modifies and returns CE1 to reflect difference of CE1 and CE2.
  /// CE1 = CE1 - CE2
  /// This routine returns false if the canon exprs are not mergeable.
  static bool subtract(CanonExpr *CE1, const CanonExpr *CE2,
                       bool RelaxedMode = false);

  /// Returns a canon expr which represents the difference of CE1 and CE2.
  /// Result = CE1 - CE2
  /// This routine can return nullptr, if the canon exprs are not mergeable.
  static CanonExpr *cloneAndSubtract(const CanonExpr *CE1, const CanonExpr *CE2,
                                     bool RelaxedMode = false);

  /// Returns a canon expr which represents the negation of CE.
  /// Result = -CE
  static CanonExpr *cloneAndNegate(const CanonExpr *CE);

  /// Returns true if the Loop level is in a valid range from
  /// [1, MaxLoopNestLevel].
  static bool isValidLoopLevel(unsigned Level) {
    return (Level > 0 && Level <= MaxLoopNestLevel);
  }

  /// Returns true if DefLevel is a valid DefinedAtLevel for any CanonExpr.
  static bool isValidDefLevel(unsigned DefLevel) {
    return (DefLevel <= NonLinearLevel);
  }

  /// Returns true if DefLevel is a valid DefinedAtLevel for a linear CanonExpr.
  static bool isValidLinearDefLevel(unsigned DefLevel) {
    return (DefLevel <= MaxLoopNestLevel);
  }

  /// Returns true if this CE should be considered non-linear given DefLevel and
  /// NestingLevel. DefLevel is the definition level of a blob contained in the
  /// CE. NestingLevel is the level where the CE is attached to HIR.
  static bool hasNonLinearSemantics(unsigned DefLevel, unsigned NestingLevel);

  // Returns true if IV in \p CE1 at the loop \p Level by the \p CE2.
  static bool canReplaceIVByCanonExpr(const CanonExpr *CE1, unsigned Level,
                                      const CanonExpr *CE2,
                                      bool RelaxedMode = false);

  /// Replaces IV in \p CE1 at the loop \p Level by the \p CE2.
  static bool replaceIVByCanonExpr(CanonExpr *CE1, unsigned Level,
                                   const CanonExpr *CE2,
                                   bool RelaxedMode = false);

  /// Returns true if CE1 - CE2 is a constant and returns the diff in \p
  /// Distance.
  ///
  /// NOTE: This is strictly a structural check in the sense that the utility is
  /// context insensitive. It doesn't perform HIR based checks and so will
  /// return a valid distance for non-linear CEs. Caller is responsible for
  /// doing extra analysis. For example, it will return a valid distance between
  /// (i1+%t) and (i1+%t+1) even if %t is non-linear and has a different value
  /// for the two CEs.
  static bool getConstDistance(const CanonExpr *CE1, const CanonExpr *CE2,
                               int64_t *Distance);

  /// Returns true if CE1 - CE2 results in a constant difference w.r.t IV at \p
  /// LoopLevel which is the distance in terms of number of loop iterations.
  /// For example-
  /// Returns true with distance of 1 if-
  /// CE1 = 2*i1 + 2
  /// CE2 = 2*i1
  ///
  /// Returns false if-
  /// CE1 = 2*i1 + 1
  /// CE2 = 2*i1
  ///
  /// NOTE: This is strictly a structural check in the sense that the utility is
  /// context insensitive. It doesn't perform HIR based checks and so will
  /// return a valid distance for non-linear CEs. Caller is responsible for
  /// doing extra analysis. For example, it will return a valid distance between
  /// (i1+%t) and (i1+%t+1) even if %t is non-linear and has a different value
  /// for the two CEs.
  /// TODO: Fix for vector type CEs.
  static bool getConstIterationDistance(const CanonExpr *CE1,
                                        const CanonExpr *CE2,
                                        unsigned LoopLevel, int64_t *Distance);

  // Sorting comparator for two canon expressions. Returns true if \p CE1 is
  // less then \p CE2.
  static bool compare(const CanonExpr *CE1, const CanonExpr *CE2);
};

} // End namespace loopopt

} // End namespace llvm

#endif
