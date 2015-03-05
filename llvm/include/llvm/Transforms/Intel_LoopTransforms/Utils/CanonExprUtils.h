//===------ CanonExprUtils.h - Utilities for CanonExpr class ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

namespace llvm {

class Type;

namespace loopopt {

/// \brief Defines utilities for CanonExpr class
///
/// It contains a bunch of static member functions which manipulate CanonExprs.
/// It does not store any state.
///
class CanonExprUtils {
private:
  /// \brief Do not allow instantiation.
  CanonExprUtils() LLVM_DELETED_FUNCTION;

public:
  /// \brief Returns a new CanonExpr.
  static CanonExpr *createCanonExpr(Type *Typ, bool Gen = true, int Level = 0,
                                    int64_t Const = 0, int64_t Denom = 1);

  /// \brief Destroys the passed in CanonExpr.
  static void destroy(CanonExpr *CE);
  /// \brief Destroys all CanonExprs. Should only be called after code gen.
  static void destroyAll();

  /// \brief Returns true if passed in canon cxprs are equal to each other.
  static bool areEqual(CanonExpr *CE1, CanonExpr *CE2);

  /// \brief Returns a canon expr which represents the sum of these canon exprs.
  /// Adds CE2 to CE1 if CreateNewCE is false.
  static CanonExpr *addCanonExprs(CanonExpr *CE1, CanonExpr *CE2,
                                  bool CreateNewCE = false);

  /// \brief Returns a canon expr which represents the difference of these canon
  /// exprs. Subtracts CE2 from CE1 if CreateNewCE is false.
  static CanonExpr *subtractCanonExprs(CanonExpr *CE1, CanonExpr *CE2,
                                       bool CreateNewCE = false);

  /// \brief Returns a blob which represents (LHS + RHS). Its index is returned
  /// via NewBlobIndex argument.
  static BlobTy createAddBlob(BlobTy LHS, BlobTy RHS, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS - RHS). Its index is returned
  /// via NewBlobIndex argument.
  static BlobTy createSubBlob(BlobTy LHS, BlobTy RHS, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS * RHS). Its index is returned
  /// via NewBlobIndex argument.
  static BlobTy createMulBlob(BlobTy LHS, BlobTy RHS, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS / RHS). Its index is returned
  /// via NewBlobIndex argument.
  static BlobTy createUDivBlob(BlobTy LHS, BlobTy RHS, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (trunc Blob to Ty). Its index is
  /// returned via NewBlobIndex argument.
  static BlobTy createTruncateBlob(BlobTy Blob, Type *Ty,
                                   unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (zext Blob to Ty). Its index is
  /// returned via NewBlobIndex argument.
  static BlobTy createZeroExtendBlob(BlobTy Blob, Type *Ty,
                                     unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (sext Blob to Ty). Its index is
  /// returned via NewBlobIndex argument.
  static BlobTy createSignExtendBlob(BlobTy Blob, Type *Ty,
                                     unsigned *NewBlobIndex);
};

} // End namespace loopopt

} // End namespace llvm

#endif
