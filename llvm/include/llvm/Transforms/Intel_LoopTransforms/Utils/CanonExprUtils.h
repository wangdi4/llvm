//===------ CanonExprUtils.h - Utilities for CanonExpr class ----*- C++ -*-===//
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
// This file defines the utilities for CanonExpr class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_CANONEXPRUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_CANONEXPRUTILS_H

#include <stdint.h>
#include "llvm/Support/Compiler.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLUtils.h"

namespace llvm {

class Type;
class APInt;

namespace loopopt {

/// \brief Defines utilities for CanonExpr class
///
/// It contains a bunch of static member functions which manipulate CanonExprs.
/// It does not store any state.
///
class CanonExprUtils : public HLUtils {
private:
  /// \brief Do not allow instantiation.
  CanonExprUtils() = delete;

  friend class HIRParser;

  /// \brief Destroys all CanonExprs and BlobTable. Called during HIR cleanup.
  static void destroyAll();

  /// \brief Calculates the gcd of two positive inputs.
  static int64_t gcd(int64_t A, int64_t B);

  /// \brief Calculates the lcm of two positive inputs.
  static int64_t lcm(int64_t A, int64_t B);

  /// \brief Helper to calculate gcd for simplify(). Handles negative integers
  /// as well.
  static int64_t simplifyGCDHelper(int64_t CurrentGCD, int64_t Num);

  /// \brief Implements multiplyByConstant() functionality.
  static CanonExpr *multiplyByConstantImpl(CanonExpr *CE1, int64_t Val,
                                           bool CreateNewCE = false,
                                           bool Simplify = true);

public:
  /// \brief Returns a new CanonExpr. All canon exprs are created linear.
  static CanonExpr *createCanonExpr(Type *Typ, unsigned Level = 0,
                                    int64_t Const = 0, int64_t Denom = 1);

  /// \brief Returns a new CanonExpr created from APInt Value
  static CanonExpr *createCanonExpr(const APInt &APVal, int Level = 0);

  /// \brief Destroys the passed in CanonExpr.
  static void destroy(CanonExpr *CE);

  /// \brief Multiplies IV of CanonExpr by a constant based on level.
  static void multiplyIVByConstant(CanonExpr *CE, unsigned Level, int64_t Val);

  /// \brief Returns true if the type of both Canon Expr matches
  static bool isTypeEqual(const CanonExpr *CE1, const CanonExpr *CE2);

  /// \brief Returns true if passed in canon cxprs are equal to each other.
  static bool areEqual(const CanonExpr *CE1, const CanonExpr *CE2);

  /// \brief Returns a canon expr which represents the sum of these canon
  /// exprs. Result = CE1+CE2
  /// If CreateNewCE is true, results in a new canon expr.
  /// If CreateNewCE is false, it updates the input canon expr.
  static CanonExpr *add(CanonExpr *CE1, const CanonExpr *CE2,
                        bool CreateNewCE = false);

  /// \brief Multiplies constant by Canon Expr and returns result pointer
  /// Result = CE1*Const
  /// If CreateNewCE is true, results in a new canon expr.
  /// If CreateNewCE is false, it updates the input canon expr.
  static CanonExpr *multiplyByConstant(CanonExpr *CE1, int64_t Val,
                                       bool CreateNewCE = false);

  /// \brief Returns a canon expr which represents the negation of given
  /// canon expr. Result = -CE1
  /// If CreateNewCE is true, results in a new canon expr.
  /// If CreateNewCE is false, it updates the input canon expr.
  static CanonExpr *negate(CanonExpr *CE1, bool CreateNewCE = false);

  /// \brief Returns a canon expr which represents the difference of these
  /// canon exprs. Result = CE1 - CE2
  /// If CreateNewCE is true, results in a new canon expr.
  /// If CreateNewCE is false, it updates the input canon expr.
  static CanonExpr *subtract(CanonExpr *CE1, const CanonExpr *CE2,
                             bool CreateNewCE = false);

  /// \brief Simplifies canon expr by dividing numerator and denominator by
  /// common gcd.
  static void simplify(CanonExpr *CE);
};

} // End namespace loopopt

} // End namespace llvm

#endif
