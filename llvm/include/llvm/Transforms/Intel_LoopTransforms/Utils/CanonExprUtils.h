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

#include <set>

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

  /// Keeps track of objects of the CanonExpr class.
  static std::set<CanonExpr *> GlobalCanonExprs;

  /// \brief Destroys all CanonExprs and BlobTable. Called during HIR cleanup.
  static void destroyAll();

  /// \brief Calculates the lcm of two positive inputs.
  static int64_t lcm(int64_t A, int64_t B);

public:
  /// \brief Returns a new CanonExpr. All canon exprs are created linear.
  static CanonExpr *createCanonExpr(Type *Typ, unsigned Level = 0,
                                    int64_t Const = 0, int64_t Denom = 1);

  /// \brief Returns a new CanonExpr created from APInt Value
  static CanonExpr *createCanonExpr(const APInt &APVal, int Level = 0);

  /// \brief Destroys the passed in CanonExpr.
  static void destroy(CanonExpr *CE);

  /// \brief Calculates the gcd of two positive inputs.
  static int64_t gcd(int64_t A, int64_t B);

  /// \brief Returns the index of Blob in the blob table. Index range is [1,
  /// UINT_MAX]. Returns 0 if the blob is not present in the table.
  static unsigned findBlob(CanonExpr::BlobTy Blob);

  /// \brief Returns the index of Blob in the blob table. Blob is first
  /// inserted, if it isn't already present in the blob table. Index range is
  /// [1, UINT_MAX].
  static unsigned findOrInsertBlob(CanonExpr::BlobTy Blob);

  /// \brief Returns blob corresponding to BlobIndex.
  static CanonExpr::BlobTy getBlob(unsigned BlobIndex);

  /// \brief Prints blob.
  static void printBlob(raw_ostream &OS, CanonExpr::BlobTy Blob,
                        bool Detailed = false);

  /// \brief Prints scalar corresponding to symbase.
  static void printScalar(raw_ostream &OS, unsigned Symbase,
                          bool Detailed = false);

  /// \brief Checks if the blob is constant or not.
  /// If blob is constant, sets the return value in Val.
  static bool isConstantIntBlob(CanonExpr::BlobTy Blob, int64_t *Val);

  /// \brief Returns true if Blob is a temp.
  static bool isTempBlob(CanonExpr::BlobTy Blob);

  /// \brief Returns a new blob created from a constant value.
  static CanonExpr::BlobTy createBlob(int64_t Val, bool Insert = true,
                                      unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a blob which represents (LHS + RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy createAddBlob(CanonExpr::BlobTy LHS,
                                         CanonExpr::BlobTy RHS,
                                         bool Insert = true,
                                         unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a blob which represents (LHS - RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy createMinusBlob(CanonExpr::BlobTy LHS,
                                           CanonExpr::BlobTy RHS,
                                           bool Insert = true,
                                           unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (LHS * RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy createMulBlob(CanonExpr::BlobTy LHS,
                                         CanonExpr::BlobTy RHS,
                                         bool Insert = true,
                                         unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (LHS / RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy createUDivBlob(CanonExpr::BlobTy LHS,
                                          CanonExpr::BlobTy RHS,
                                          bool Insert = true,
                                          unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (trunc Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy createTruncateBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                              bool Insert = true,
                                              unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (zext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy
  createZeroExtendBlob(CanonExpr::BlobTy Blob, Type *Ty, bool Insert = true,
                       unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (sext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  static CanonExpr::BlobTy
  createSignExtendBlob(CanonExpr::BlobTy Blob, Type *Ty, bool Insert = true,
                       unsigned *NewBlobIndex = nullptr);

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

  /// \brief Returns a canon expr which represents the difference of these
  /// canon exprs. Result = CE1 - CE2
  /// If CreateNewCE is true, results in a new canon expr.
  /// If CreateNewCE is false, it updates the input canon expr.
  static CanonExpr *subtract(CanonExpr *CE1, const CanonExpr *CE2,
                             bool CreateNewCE = false);

  /// \brief Returns a canon expr which represents the negation of given
  /// canon expr. Result = -CE1
  /// If CreateNewCE is true, results in a new canon expr.
  /// If CreateNewCE is false, it updates the input canon expr.
  static CanonExpr *negate(CanonExpr *CE1, bool CreateNewCE = false);
};

} // End namespace loopopt

} // End namespace llvm

#endif
