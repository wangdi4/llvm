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
#include <stdint.h>

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

#include <set>

namespace llvm {

class Type;
class APInt;

namespace loopopt {

class CanonExpr;

/// \brief Defines utilities for CanonExpr class
///
/// It contains a bunch of static member functions which manipulate CanonExprs.
/// It does not store any state.
///
/// RelaxedMode : This flag is used in a number of methods across this file.
/// The flag indicates if the types of canon expr can be relaxed for
/// computation.
/// For example, constant canon exprs, CE1 = 0 (srcTy = i32, dstTy = i64) and
/// CE2 = 1 (srcTy = i64, dstTy = i64), can be added in relaxed mode. Clients
/// can check canon expr with mergeable call.
class CanonExprUtils : public HIRUtils {
private:
  /// \brief Do not allow instantiation.
  CanonExprUtils() = delete;

  friend class DDRefUtils; // accesses createSelfBlobCanonExpr()
  friend class HIRParser;  // accesses destroyAll()

  /// \brief Destroys all CanonExprs. Called during HIR cleanup.
  static void destroyAll();

  /// \brief Calculates the lcm of two positive inputs.
  static int64_t lcm(int64_t A, int64_t B);

  /// \brief Creates a non-linear self blob canon expr from the passed in Value.
  /// The new blob is associated with symbase. New temp blobs from values are
  /// only created by framework.
  static CanonExpr *createSelfBlobCanonExpr(Value *Temp, unsigned Symbase);

  /// \brief Returns true if constant canon expr type can be updated to match
  /// the source type. For any other types or non-mergeable cases, it returns
  /// false.
  static bool canMergeConstants(const CanonExpr *CE1, const CanonExpr *CE2,
                                bool RelaxedMode);

  /// \brief Updates the constant canon expr source type in order to add them.
  static void updateConstantTypes(CanonExpr *CE1, CanonExpr **CE2,
                                  bool RelaxedMode, bool *CreatedAuxCE);

  /// \brief Implements add()/cloneAndAdd() functionality.
  /// This routine will return nullptr, if the canon exprs are not
  /// mergeable.
  static CanonExpr *addImpl(CanonExpr *CE1, const CanonExpr *CE2,
                            bool CreateNewCE, bool RelaxedMode);

public:
  /// \brief Returns a new CanonExpr with identical src and dest types. All
  /// canon exprs are created linear by default.
  static CanonExpr *createCanonExpr(Type *Ty, unsigned Level = 0,
                                    int64_t Const = 0, int64_t Denom = 1,
                                    bool IsSignedDiv = false);

  /// \brief Returns a new CanonExpr with zero or sign extension. All canon
  /// exprs are created linear by default.
  /// Note: Overloading createCanonExpr() causes ambiguous calls for constant
  /// arguments.
  static CanonExpr *createExtCanonExpr(Type *SrcType, Type *DestType,
                                       bool IsSExt, unsigned Level = 0,
                                       int64_t Const = 0, int64_t Denom = 1,
                                       bool IsSignedDiv = false);

  /// \brief Returns a new linear CanonExpr created from APInt Value. 
  static CanonExpr *createCanonExpr(Type *Ty, const APInt &APVal);

  /// \brief Returns a self-blob canon expr. Level is the defined at level for
  /// the blob.
  static CanonExpr *createSelfBlobCanonExpr(unsigned Index,
                                            unsigned Level = NonLinearLevel);

  /// \brief Destroys the passed in CanonExpr.
  static void destroy(CanonExpr *CE);

  /// \brief Calculates the gcd of two positive inputs.
  static int64_t gcd(int64_t A, int64_t B);

  /// \brief Returns the size of the type in bits.
  /// NOTE: This function asserts that the incoming type is sized.
  static uint64_t getTypeSizeInBits(Type *Ty);

  /// \brief Returns true if the type of both Canon Expr matches.
  static bool isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                          bool RelaxedMode = false);

  /// \brief Returns true if CE1 and CE2 can be merged (added/subtracted etc).
  static bool mergeable(const CanonExpr *CE1, const CanonExpr *CE2,
                        bool RelaxedMode = false);

  /// \brief Returns true if passed in canon cxprs are equal to each other.
  /// Ignores dest types of CE1 and CE2 if IgnoreDestType is set.
  static bool areEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                       bool RelaxedMode = false);

  /// \brief Modifies and returns CE1 to reflect sum of CE1 and CE2.
  /// CE1 = CE1 + CE2
  /// This routine can return nullptr, if the canon exprs are not
  /// mergeable.
  static CanonExpr *add(CanonExpr *CE1, const CanonExpr *CE2,
                        bool RelaxedMode = false);

  /// \brief Returns a canon expr which represents the sum of CE1 and CE2.
  /// Result = CE1 + CE2
  /// This routine can return nullptr, if the canon exprs are not
  /// mergeable.
  static CanonExpr *cloneAndAdd(const CanonExpr *CE1, const CanonExpr *CE2,
                                bool RelaxedMode = false);

  /// \brief Modifies and returns CE1 to reflect difference of CE1 and CE2.
  /// CE1 = CE1 - CE2
  /// This routine can return nullptr, if the canon exprs are not
  /// mergeable.
  static CanonExpr *subtract(CanonExpr *CE1, const CanonExpr *CE2,
                             bool RelaxedMode = false);

  /// \brief Returns a canon expr which represents the difference of CE1 and
  /// CE2. Result = CE1 - CE2
  /// This routine can return nullptr, if the canon exprs are not
  /// mergeable.
  static CanonExpr *cloneAndSubtract(const CanonExpr *CE1, const CanonExpr *CE2,
                                     bool RelaxedMode = false);

  /// \brief Returns a canon expr which represents the negation of CE.
  /// Result = -CE
  static CanonExpr *cloneAndNegate(const CanonExpr *CE);

  /// \brief Returns true if DefLevel is a valid DefinedAtLevel for any
  /// CanonExpr.
  static bool isValidDefLevel(unsigned DefLevel) {
    return (DefLevel <= NonLinearLevel);
  }

  /// \brief Returns true if DefLevel is a valid DefinedAtLevel for a linear
  /// CanonExpr.
  static bool isValidLinearDefLevel(unsigned DefLevel) {
    return (DefLevel <= MaxLoopNestLevel);
  }

  /// \brief Returns true if this CE should be considered non-linear given
  /// DefLevel and NestingLevel. DefLevel is the definition level of a blob
  /// contained in the CE. NestingLevel is the level where the CE is attached to
  /// HIR.
  static bool hasNonLinearSemantics(unsigned DefLevel, unsigned NestingLevel);

  /// \brief Replaces IV in *CE1* at a particular loop *Level* by a CanonExpr
  /// *CE2*.
  /// Please be aware that this method can handle only CE2 with unit
  /// denominator because b*(x/d) != (b*x)/d.
  static CanonExpr *replaceIVByCanonExpr(CanonExpr *CE1, unsigned Level,
                                         const CanonExpr *CE2,
                                         bool RelaxedMode = false);
};

} // End namespace loopopt

} // End namespace llvm

#endif
