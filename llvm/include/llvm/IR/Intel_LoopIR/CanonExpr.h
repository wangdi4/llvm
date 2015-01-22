//===- CanonExpr.h - Closed form in high level IR ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the closed form representation in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_CANONEXPR_H
#define LLVM_IR_INTEL_LOOPIR_CANONEXPR_H

#include "llvm/Support/Compiler.h"
#include "llvm/ADT/SmallVector.h"
#include <stdint.h>
#include <utility>
#include <set>
#include <vector>
#include <iterator>

namespace llvm {

class Type;

namespace loopopt {

/// \brief Canonical form in high level IR
///
/// This class represents the closed form as a linear equation in terms of
/// induction variables and blobs. It is essentially an array of coefficients
/// of induction variables and blobs. A blob is usually a non-inductive, 
/// loop invariant variable in the equestion but is allowed to vary under 
/// some cases where a more generic representation is required. Blob exprs
/// are represented using SCEVs and mapped to blob indexes.
///
/// This class disallows creating objects on stack.
/// Objects are created/destroyed using CanonExprUtils friend class.
class CanonExpr {
public:
  typedef std::pair<bool, int64_t> BlobOrConstToValTy;
  typedef std::pair<int, int64_t> BlobIndexToCoeffTy;
  typedef SmallVector<BlobOrConstToValTy, 4> IVTy;
  typedef SmallVector<BlobIndexToCoeffTy, 2> BlobTy;

  /// Iterators to iterate over induction variables
  typedef IVTy::iterator iv_iterator;
  typedef IVTy::const_iterator const_iv_iterator;
  typedef IVTy::reverse_iterator reverse_iv_iterator;
  typedef IVTy::const_reverse_iterator const_reverse_iv_iterator; 

  /// Iterators to iterate over blobs
  typedef BlobTy::iterator blob_iterator;
  typedef BlobTy::const_iterator const_blob_iterator;
  typedef BlobTy::reverse_iterator reverse_blob_iterator;
  typedef BlobTy::const_reverse_iterator const_reverse_blob_iterator; 

private:
  /// \brief Make class uncopyable.
  CanonExpr(const CanonExpr &) LLVM_DELETED_FUNCTION;
  void operator=(const CanonExpr &) LLVM_DELETED_FUNCTION;

  CanonExpr(Type* Typ, bool Gen, int Level, int64_t Cons, int64_t Denom);
  ~CanonExpr() { }

  friend class CanonExprUtils;

  /// \brief Destroys the object.
  void destroy();
  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();
  /// Keeps track of objects of this class.
  static std::set< CanonExpr* >Objs;

  Type* Ty;
  bool Generable;
  int DefinedAtLevel;
  IVTy IVCoeffs;
  BlobTy BlobCoeffs;
  int64_t Const;
  int64_t Denominator;

public:

  CanonExpr* clone() const;
  void dump() const;
  void print() const;

  /// \brief Returns the LLVM type of this canon expr.
  const Type* getLLVMType() const { return Ty; }
  void setLLVMType(Type* Typ) { Ty = Typ; }

  /// \brief Returns true if we can generate code out of the closed form.
  /// This flag might move to DDRef
  bool isGenerable() const { return Generable; }
  void setGenerable(bool Gen = true) { Generable = Gen; }

  /// \brief Returns the innermost level at which some blob present
  /// in this canon expr is defined. The canon expr in linear in all 
  //  the inner loop levels w.r.t this level.
  int getDefinedAtLevel() const { return DefinedAtLevel; }
  void setDefinedAtLevel(int Lvl) { DefinedAtLevel = Lvl; }

  /// \brief Returns true if this is linear at all levels.
  bool isProperLinear() const { return (Generable && (DefinedAtLevel == 0)); }
  /// \brief Returns true if some blob in the canon expr is defined in
  /// the current loop level.
  bool isNonLinear() const { return (Generable && (DefinedAtLevel == -1)); }
  /// \brief Returns true if this is not non-linear.
  bool isLinearAtLevel() const { return !isNonLinear(); }

  /// \brief Returns the constant additive of the canon expr.
  int64_t getConstant() const { return Const; }
  void setConstant(int64_t Val) { Const = Val; }

  /// \brief Returns the denominator of the canon expr.
  int64_t getDenominator() const { return Denominator; }
  void setDenominator(int64_t Val) { Denominator = Val; }

  /// \brief Returns true if this contains any IV.
  bool hasIV() const;
  /// \brief Returns true if this contains any blobs.
  bool hasBlob() const { return !BlobCoeffs.empty(); }

  /// \brief Returns the IV coeffieicent at a particular loop level.
  int64_t getIVCoeff(int Lvl, bool* isBlobCoeff) const;
  /// \brief Sets the IV coeffieicent at a particular loop level.
  void setIVCoeff(int Lvl, int64_t Val, bool isBlobCoeff);
  
  /// \brief Adds to the existing IV coefficient at a particular loop level. 
  void addIV(int Lvl, int64_t Coeff, bool IsBlobCoeff);
  /// \brief Removes IV at a particular loop level.
  void removeIV(int Lvl);

  /// \brief Returns the blob coeffieicent.
  int64_t getBlobCoeff(int BlobIndex) const;
  /// \brief Sets the blob coeffieicent.
  void setBlobCoeff(int BlobIndex, int64_t BlobCoeff);

  /// \brief Adds to the existing blob coefficient.
  void addBlob(int BlobIndex, int64_t BlobCoeff);
  /// \brief Removes a blob.
  void removeBlob(int BlobIndex);

  /// \brief Replaces an old blob with a new one.
  void replaceBlob(int OldBlobIndex, int NewBlobIndex);

  /// \brief Shifts the canon expr by a constant offset.
  void shiftAtLevel(int Lvl, int64_t Val);
  
  /// \brief Adds the passed in canon expr to this one.
  void add(CanonExpr* CE);
  /// \brief Subtracts the passed in canon expr from this one.
  void subtract(CanonExpr* CE);

  /// \brief Multiplies this canon expr by a contant.
  CanonExpr* multiplyByConstant(int64_t Const);
  /// \brief Multiplies this canon expr by a blob.
  CanonExpr* multiplyByBlob(int BlobIndex);

  /// IV iterator methods
  iv_iterator               iv_begin()        { return IVCoeffs.begin(); }
  const_iv_iterator         iv_begin() const  { return IVCoeffs.begin(); }
  iv_iterator               iv_end()          { return IVCoeffs.end(); }
  const_iv_iterator         iv_end() const    { return IVCoeffs.end(); }

  reverse_iv_iterator       iv_rbegin()       { return IVCoeffs.rbegin(); }
  const_reverse_iv_iterator iv_rbegin() const { return IVCoeffs.rbegin(); }
  reverse_iv_iterator       iv_rend()         { return IVCoeffs.rend(); }
  const_reverse_iv_iterator iv_rend() const   { return IVCoeffs.rend(); }


  /// blob iterator methods
  blob_iterator               blob_begin()        { return BlobCoeffs.begin(); }
  const_blob_iterator         blob_begin()  const { return BlobCoeffs.begin(); }
  blob_iterator               blob_end()          { return BlobCoeffs.end(); }
  const_blob_iterator         blob_end()    const { return BlobCoeffs.end(); }

  reverse_blob_iterator       blob_rbegin()       { return BlobCoeffs.rbegin();}
  const_reverse_blob_iterator blob_rbegin() const { return BlobCoeffs.rbegin();}
  reverse_blob_iterator       blob_rend()         { return BlobCoeffs.rend(); }
  const_reverse_blob_iterator blob_rend()   const { return BlobCoeffs.rend(); }

};

} // End loopopt namespace

} // End llvm namespace

#endif

