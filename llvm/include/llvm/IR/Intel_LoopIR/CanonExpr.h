//===- CanonExpr.h - Closed form in high level IR ---------------*- C++ -*-===//
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
// This file defines the closed form representation in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_CANONEXPR_H
#define LLVM_IR_INTEL_LOOPIR_CANONEXPR_H

#include "llvm/ADT/SmallVector.h"

#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormattedStream.h"

#include <stdint.h>
#include <utility>
#include <set>
#include <vector>
#include <iterator>

namespace llvm {

class Type;
class SCEV;

namespace loopopt {

/// \brief The maximum loopnest level allowed in HIR.
const unsigned MaxLoopNestLevel = 9;
/// \brief Value to represent non-linear level.
const unsigned NonLinearLevel = MaxLoopNestLevel + 1;

/// \brief Canonical form in high level IR
///
/// This class represents the closed form as a linear equation in terms of
/// induction variables and blobs. It is essentially an array of coefficients
/// of induction variables and blobs. A blob is usually a non-inductive,
/// loop invariant variable but is allowed to vary under some cases where a
/// more generic representation is required. Blob exprs are represented using
/// SCEVs and mapped to blob indexes.
/// The denominator is always stored as a positive value. If a client sets a
/// negative denominator value, the numerator is negated instead.
///
/// CanonExpr representation-
/// (C1 * B1 * i1 + C2 * B2 * i2 + ... + BC1 * b1 + BC2 * b2 + ... + C0) / D
///
/// Where:
/// - i1, i2 etc are induction variables of loop at level 1, 2 etc.
/// - C1, C2 etc are constant coefficients of i1, i2 etc.
/// - B1, B2 etc are blob coefficients of i1, i2 etc. A zero blob coefficient
///   implies a constant only coefficient.
/// - b1, b2 etc are blobs.
/// - BC1, BC2 etc are constant coefficients of b1, b2 etc.
/// - C0 is the constant additive.
/// - D is the denominator.
///
/// This class disallows creating objects on stack.
/// Objects are created/destroyed using CanonExprUtils friend class.
class CanonExpr {
private:
  struct BlobIndexToCoeff {
    /// Valid index range is [1, UINT_MAX]. If this is associated with an IV,
    /// 0
    /// implies a constant only coefficient.
    unsigned Index;
    int64_t Coeff;
    BlobIndexToCoeff(unsigned Indx, int64_t Coef);
    ~BlobIndexToCoeff();
  };

  // Used to keep the blob vector sorted by index.
  struct BlobIndexCompareLess {
    bool operator()(const CanonExpr::BlobIndexToCoeff &B1,
                    const CanonExpr::BlobIndexToCoeff &B2) {
      return B1.Index < B2.Index;
    }
  };

  // Used to keep the blob vector sorted by index.
  struct BlobIndexCompareEqual {
    bool operator()(const CanonExpr::BlobIndexToCoeff &B1,
                    const CanonExpr::BlobIndexToCoeff &B2) {
      return B1.Index == B2.Index;
    }
  };

public:
  /// Each element represents blob index and coefficient associated with an IV
  /// at a particular loop level.
  typedef SmallVector<BlobIndexToCoeff, 4> IVCoeffsTy;
  /// Kept sorted by blob index
  typedef SmallVector<BlobIndexToCoeff, 2> BlobCoeffsTy;

  /// Iterators to iterate over induction variables
  typedef IVCoeffsTy::iterator iv_iterator;
  typedef IVCoeffsTy::const_iterator const_iv_iterator;
  typedef IVCoeffsTy::reverse_iterator reverse_iv_iterator;
  typedef IVCoeffsTy::const_reverse_iterator const_reverse_iv_iterator;

  /// Iterators to iterate over blobs
  typedef BlobCoeffsTy::iterator blob_iterator;
  typedef BlobCoeffsTy::const_iterator const_blob_iterator;
  typedef BlobCoeffsTy::reverse_iterator reverse_blob_iterator;
  typedef BlobCoeffsTy::const_reverse_iterator const_reverse_blob_iterator;

private:
  /// \brief Copy constructor; only used for cloning.
  CanonExpr(const CanonExpr &);
  /// \brief Make class unassignable.
  void operator=(const CanonExpr &) = delete;

  /// \brief Destroys all objects of this class. Should only be called after
  /// code gen.
  static void destroyAll();
  // Keeps track of objects of this class.
  static std::set<CanonExpr *> Objs;

  // SrcTy and DestTy hide one level of casting applied on top of the
  // canonical form.
  // If they are different, either both are integer types(regular canon exprs)
  // or pointer types(base canon exprs of GEP DDRefs).
  // Both types are identical in the absence of casting.
  Type *SrcTy;
  Type *DestTy;
  // Capture whether we are hiding signed or zero extension.
  bool IsSExt;
  unsigned DefinedAtLevel;
  IVCoeffsTy IVCoeffs;
  BlobCoeffsTy BlobCoeffs;
  int64_t Const;
  int64_t Denominator;
  // Capture whether we are representing signed or unsigned division.
  bool IsSignedDiv;

  // The flag is set if canon expr contains UndefValue.
  bool ContainsUndef;

protected:
  CanonExpr(Type *SrcType, Type *DestType, bool IsSExt, unsigned DefLevel,
            int64_t ConstVal, int64_t Denom, bool IsSignedDiv);
  virtual ~CanonExpr(){};

  friend class CanonExprUtils;
  friend class HIRParser;

  /// \brief Destroys the object.
  void destroy();

  /// \brief Implements hasIV()/numIV() and
  /// hasBlobIVCoeffs()/numBlobIVCoeffs() functionality.
  unsigned numIVImpl(bool CheckIVPresence, bool CheckBlobCoeffs) const;

  /// \brief Resizes IVCoeffs to max loopnest level if the passed in level
  /// goes beyond the current size. This will avoid future reallocs.
  void resizeIVCoeffsToMax(unsigned Lvl);

  /// \brief Sets blob/const coefficient of an IV at a particular loop level.
  /// Overwrite flags indicate what is to be overwritten.
  void setIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff,
                     bool OverwriteIndex, bool OverwriteCoeff);

  /// \brief Adds blob/const coefficient of an IV at a particular loop level.
  void addIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff);

  /// \brief Sets a blob coefficient. Depending upon the overwrite flag the
  /// existing coefficient is either overwritten or added to.
  void addBlobInternal(unsigned BlobIndex, int64_t BlobCoeff, bool overwrite);

  /// \brief Helper to calculate gcd for simplify(). Handles negative integers
  /// as well.
  int64_t simplifyGCDHelper(int64_t CurrentGCD, int64_t Num);

  /// \brief Implements multiplyByConstant() functionality. Simplify flag
  /// indicates whether simplification can be performed.
  void multiplyByConstantImpl(int64_t Val, bool Simplify);

  /// \brief Implements is*Ext() and isTrunc() functionality.
  bool isExtImpl(bool IsSigned, bool IsTrunc) const;

  /// \brief Implements collect*BlobIndices() functionality.
  void collectBlobIndicesImpl(SmallVectorImpl<unsigned> &Indices,
                              bool MakeUnique, bool NeedTempBlobs) const;

  /// \brief Marks this canon expr as containing undefined value.
  void setContainsUndef() { this->ContainsUndef = true; }

  /// \brief Returns true if the canon expr represents a constant.
  bool isConstInternal() const {
    return (!containsUndef() && !hasIV() && !hasBlob() &&
            (getDenominator() == 1));
  }

  /// \brief Return the mathematical coefficient to be used in cases
  /// where mathematical addition is performed. The Coeff value in those
  /// cases is multiplied by denominator.
  int64_t getMathCoeff(int64_t Coeff, bool IsMathAdd) {
    return IsMathAdd ? (getDenominator() * Coeff) : Coeff;
  }

public:
  CanonExpr *clone() const;

  /// \brief Dumps CanonExpr.
  void dump(bool Detailed) const;
  /// \brief Dumps CanonExpr in a simple format.
  void dump() const;
  /// \brief Prints CanonExpr.
  void print(formatted_raw_ostream &OS, bool Detailed = false) const;

  /// \brief Returns the src type of this canon expr.
  Type *getSrcType() const { return SrcTy; }
  void setSrcType(Type *SrcType) { SrcTy = SrcType; }

  /// \brief Returns the dest type of this canon expr.
  Type *getDestType() const { return DestTy; }
  void setDestType(Type *DestType) { DestTy = DestType; }

  /// \brief Returns true if the canon expr is hiding a signed extension.
  bool isSExt() const;

  /// \brief Returns true if the canon expr is hiding a zero extension.
  bool isZExt() const;

  /// \brief Returns true if the canon expr is hiding a trunc.
  bool isTrunc() const;

  /// \brief Returns true if the canon expr is hiding a pointer to pointer
  /// bitcast.
  bool isPtrToPtrCast() const;

  /// \brief Sets the extension type (signed or unsigned) for canon expr. This
  /// can be a no-op depending upon src and dest types.
  void setExtType(bool SExt) { IsSExt = SExt; }

  /// \brief Returns the innermost level at which some blob present
  /// in this canon expr is defined. The canon expr in linear in all
  /// the inner loop levels w.r.t this level.
  unsigned getDefinedAtLevel() const {
    assert(isLinearAtLevel() &&
           "DefinedAtLevel is meaningless for non-linear types!");
    return DefinedAtLevel;
  }
  /// \brief Sets non-negative defined at level.
  void setDefinedAtLevel(unsigned DefLvl) {
    assert((DefLvl <= MaxLoopNestLevel) && "DefLvl exceeds max level!");
    DefinedAtLevel = DefLvl;
  }

  /// \brief Returns true if some blob in the canon expr is defined in
  /// the current loop level.
  bool isNonLinear() const { return (DefinedAtLevel == NonLinearLevel); }

  /// \brief Mark this canon expr as non-linear.
  void setNonLinear() { DefinedAtLevel = NonLinearLevel; }

  /// \brief Returns true if this is linear at all levels.
  bool isProperLinear() const { return (DefinedAtLevel == 0); }

  /// \brief Returns true if this is linear at some levels (greater than
  /// DefinedAtLevel) in the current loopnest.
  bool isLinearAtLevel() const { return !isNonLinear(); }

  /// \brief Returns true if the canon expr is linear at level and does not
  /// have IV at given level.
  bool isInvariantAtLevel(unsigned Level) const {
    return (isLinearAtLevel() && (DefinedAtLevel < Level) && !hasIV(Level));
  }

  /// IV iterator methods
  iv_iterator iv_begin() { return IVCoeffs.begin(); }
  const_iv_iterator iv_begin() const { return IVCoeffs.begin(); }
  iv_iterator iv_end() { return IVCoeffs.end(); }
  const_iv_iterator iv_end() const { return IVCoeffs.end(); }
  reverse_iv_iterator iv_rbegin() { return IVCoeffs.rbegin(); }
  const_reverse_iv_iterator iv_rbegin() const { return IVCoeffs.rbegin(); }
  reverse_iv_iterator iv_rend() { return IVCoeffs.rend(); }
  const_reverse_iv_iterator iv_rend() const { return IVCoeffs.rend(); }

  /// Blob iterator methods
  blob_iterator blob_begin() { return BlobCoeffs.begin(); }
  const_blob_iterator blob_begin() const { return BlobCoeffs.begin(); }
  blob_iterator blob_end() { return BlobCoeffs.end(); }
  const_blob_iterator blob_end() const { return BlobCoeffs.end(); }
  reverse_blob_iterator blob_rbegin() { return BlobCoeffs.rbegin(); }
  const_reverse_blob_iterator blob_rbegin() const {
    return BlobCoeffs.rbegin();
  }
  reverse_blob_iterator blob_rend() { return BlobCoeffs.rend(); }
  const_reverse_blob_iterator blob_rend() const { return BlobCoeffs.rend(); }

  /// \brief Returns true if canon expr represents any kind of constant.
  bool isConstant() const {
    return (isIntConstant() || isFPConstant() || isNull());
  }

  /// \brief Returns true if canon expr is a constant integer. Integer value
  /// is returned in Val.
  bool isIntConstant(int64_t *Val = nullptr) const;

  /// \brief Returns true if canon expr represents a floating point constant.
  bool isFPConstant() const;

  /// \brief Returns true if canon expr represents null pointer value.
  bool isNull() const;

  /// \brief Returns true if this canon expr looks soemthing like (1 * %t).
  /// This is a broader check than isSelfBlob() because it allows the blob to
  /// be a FP constant or even metadata.
  bool isStandAloneBlob() const {
    return (!hasIV() && !getConstant() && (getDenominator() == 1) &&
            (numBlobs() == 1) && (getSingleBlobCoeff() == 1));
  }

  /// \brief Returns true if this canon expr looks something like (1 * %t)
  /// i.e. a single blob with a coefficient of 1. Please note that there is an
  /// additional symbase matching requirement for DDRef to be considered a
  /// self-blob. Please refer to description of isSelfBlob() in DDRef.h.
  bool isSelfBlob() const;

  /// \brief return true if the CanonExpr is zero
  bool isZero() const {
    int64_t Val;
    if (isIntConstant(&Val) && Val == 0) {
      return true;
    }
    return false;
  }

  /// \brief return true if the CanonExpr is one
  bool isOne() const {
    int64_t Val;
    if (isIntConstant(&Val) && Val == 1) {
      return true;
    }
    return false;
  }

  /// \brief Returns true if this expression contains undefined terms.
  bool containsUndef() const { return ContainsUndef; }
  /// \brief Indicates that the canon expr does not contain undefined terms
  /// anymore.
  /// NOTE: Canon expr can internally maintain this flag by inspecting all the
  /// blobs but not sure if it is worth implementing as it will unnecessarily
  /// increase compile time.
  void unsetContainsUndef() { ContainsUndef = false; }

  /// \brief Returns the constant additive of the canon expr.
  int64_t getConstant() const { return Const; }
  /// \brief Sets the constant additive of the canon expr.
  void setConstant(int64_t Val) { Const = Val; }

  /// \brief Adds a constant value (Val) to the existing constant additive
  /// of the canon expr. If IsMathAdd is set to true (default is false), it
  /// performs mathematical addition by considering denominator in addition.
  void addConstant(int64_t Val, bool IsMathAdd = false) {
    Const += getMathCoeff(Val, IsMathAdd);
  }

  /// \brief Returns the denominator of the canon expr.
  int64_t getDenominator() const { return Denominator; }
  /// \brief Sets canon expr's denominator. Negates it for negative
  /// denominators.
  /// If Simplifiy is set, we call simplify() on the canon expr after setting
  /// the denominator.
  void setDenominator(int64_t Val, bool Simplify = false);

  /// \brief Multiplies the constant value (Val) with the existing denominator
  /// of the canon expr. The new denominator equals (Old denominator * Val).
  void divide(int64_t Val, bool Simplify = false);

  /// \brief Returns true if the division in the canon expr is a signed
  /// division.
  bool isSignedDiv() const { return IsSignedDiv; }

  /// \brief Sets the division type which can be either signed or unsigned.
  /// This
  /// is a no-op for unit denominator.
  void setDivisionType(bool SignedDiv) { IsSignedDiv = SignedDiv; }

  /// \brief Returns true if this contains any IV.
  bool hasIV() const;
  /// \brief Returns true if this contains IV at the given Level.
  bool hasIV(unsigned Level) const;
  /// \brief Returns the number of non-zero IVs in the canon expr.
  unsigned numIVs() const;

  /// \brief Returns true if this contains any Blob IV Coeffs.
  /// Examples: -M*i, N*j
  bool hasBlobIVCoeffs() const;
  /// \brief Returns the number of blobs IV Coeffs.
  unsigned numBlobIVCoeffs() const;
  /// \brief Returns true if this contains any blobs.
  bool hasBlob() const { return !BlobCoeffs.empty(); }
  /// \brief Returns the number of blobs in the canon expr.
  unsigned numBlobs() const { return BlobCoeffs.size(); }

  /// \brief Returns the level of IV associated with this iterator.
  unsigned getLevel(const_iv_iterator ConstIVIter) const;

  /// \brief Returns blob index and coefficient associated with an IV at a
  /// particular loop level. Lvl's range is [1, MaxLoopNestLevel].
  void getIVCoeff(unsigned Lvl, unsigned *Index, int64_t *Coeff) const;
  /// \brief Iterator version of getIVCoeff().
  void getIVCoeff(const_iv_iterator ConstIVIter, unsigned *Index,
                  int64_t *Coeff) const;

  /// \brief Sets the blob index and coefficient associated with an IV at a
  /// particular loop level. Lvl's range is [1, MaxLoopNestLevel].
  void setIVCoeff(unsigned Lvl, unsigned Index, int64_t Coeff);
  /// \brief Iterator version of setIVCoeff().
  void setIVCoeff(iv_iterator IVI, unsigned Index, int64_t Coeff);

  /// \brief Returns the blob coefficient associated with an IV at a
  /// particular loop level. Lvl's range is [1, MaxLoopNestLevel]. Returns
  /// invalid value if there is no blob coeff.
  unsigned getIVBlobCoeff(unsigned Lvl) const;
  /// \brief Iterator version of getIVBlobCoeff().
  unsigned getIVBlobCoeff(const_iv_iterator ConstIVIter) const;

  /// \brief Sets the blob coefficient associated with an IV at a particular
  /// loop level. Lvl's range is [1, MaxLoopNestLevel].
  void setIVBlobCoeff(unsigned Lvl, unsigned Index);
  /// \brief Iterator version of setIVBlobCoeff().
  void setIVBlobCoeff(iv_iterator IVI, unsigned Index);

  /// \brief Returns true if IV has a blob coefficient.
  bool hasIVBlobCoeff(unsigned Lvl) const;
  /// \brief Iterator version of hasIVBlobCoeff().
  bool hasIVBlobCoeff(const_iv_iterator ConstIVIter) const;

  /// \brief Returns the constant coefficient associated with an IV at a
  /// particular loop level. Lvl's range is [1, MaxLoopNestLevel].
  int64_t getIVConstCoeff(unsigned Lvl) const;
  /// \brief Iterator version of getIVConstCoeff().
  int64_t getIVConstCoeff(const_iv_iterator ConstIVIter) const;

  /// \brief Sets the constant coefficient associated with an IV at a
  /// particular loop level. Lvl's range is [1, MaxLoopNestLevel].
  void setIVConstCoeff(unsigned Lvl, int64_t Coeff);
  /// \brief Iterator version of setIVConstCoeff().
  void setIVConstCoeff(iv_iterator IVI, int64_t Coeff);

  /// \brief Returns true if IV has a constant coefficient.
  bool hasIVConstCoeff(unsigned Lvl) const;
  /// \brief Iterator version of hasIVBlobCoeff().
  bool hasIVConstCoeff(const_iv_iterator ConstIVIter) const;

  /// \brief Adds to the existing blob/constant IV coefficients at a
  /// particular loop level. The new IV coefficient looks something like (C1 *
  /// b1 + C2 * b2). Index can be set to zero if only a constant needs to be
  /// added. For example if the canon expr looks like (2 * n) * i1 before
  /// change, it will be modified to (3 + 2 * n) * i1 after a call to addIV(1,
  /// 0, 3). If IsMathAdd is set to true (default is false), it
  /// performs mathematical addition by considering denominator in addition.
  void addIV(unsigned Lvl, unsigned Index, int64_t Coeff,
             bool IsMathAdd = false);
  /// \brief Iterator version of addIV().
  void addIV(iv_iterator IVI, unsigned Index, int64_t Coeff,
             bool IsMathAdd = false);

  /// \brief Removes IV at a particular loop level.
  void removeIV(unsigned Lvl);
  /// \brief Iterator version of removeIV().
  void removeIV(iv_iterator IVI);

  /// \brief Multiplies IV at a particular loop level by a constant.
  void multiplyIVByConstant(unsigned Level, int64_t Val);
  /// \brief Iterator version of multiplyIVByConstant().
  void multiplyIVByConstant(iv_iterator IVI, int64_t Val);

  /// \brief Replaces IV at a particular loop level by a constant.
  void replaceIVByConstant(unsigned Lvl, int64_t Val);
  /// \brief Iterator version of replaceIVByConstant().
  void replaceIVByConstant(iv_iterator IVI, int64_t Val);

  /// \brief Returns the index associated with this blob iterator.
  unsigned getBlobIndex(const_blob_iterator CBlobI) const;

  /// \brief Returns the blob coefficient.
  int64_t getBlobCoeff(unsigned Index) const;
  /// \brief Iterator version of getBlobCoeff().
  int64_t getBlobCoeff(const_blob_iterator CBlobI) const;

  /// \brief Returns the blob index of the only blob.
  unsigned getSingleBlobIndex() const {
    assert((numBlobs() == 1) && "Canon expr does not contain single blob!");
    return BlobCoeffs[0].Index;
  }
  /// \brief Returns the blob coeff of the only blob.
  int64_t getSingleBlobCoeff() const {
    assert((numBlobs() == 1) && "Canon expr does not contain single blob!");
    return BlobCoeffs[0].Coeff;
  }

  /// \brief Sets the blob coefficient.
  void setBlobCoeff(unsigned Index, int64_t Coeff);
  /// \brief Iterator version of setBlobCoeff().
  void setBlobCoeff(blob_iterator BlobI, int64_t Coeff);

  /// \brief Adds to the existing blob coefficient. If IsMathAdd is set to true
  /// (default is false), it performs mathematical addition by considering
  /// denominator in addition.
  void addBlob(unsigned Index, int64_t Coeff, bool IsMathAdd = false);
  /// \brief Iterator version of addBlob().
  void addBlob(blob_iterator BlobI, int64_t Coeff, bool IsMathAdd = false);

  /// \brief Removes a blob. It does not touch IV blob coefficients.
  void removeBlob(unsigned Index);
  /// \brief Iterator version of removeBlob().
  void removeBlob(blob_iterator BlobI);

  /// \brief Replaces an old blob with a new one (including blob IV coeffs).
  void replaceBlob(unsigned OldIndex, unsigned NewIndex);

  /// \brief Clears everything from the CanonExpr except Type. Denominator is
  /// set to 1.
  void clear();

  /// \brief Clears all the IV coefficients from the CanonExpr.
  void clearIVs();

  /// \brief Clears all the blobs (excluding blob IV coeffs) from the
  /// CanonExpr.
  void clearBlobs() { BlobCoeffs.clear(); }

  /// \brief Shifts the canon expr by a constant offset at a particular loop
  /// level.
  void shift(unsigned Lvl, int64_t Val);
  /// \brief Iterator version of shift.
  void shift(iv_iterator IVI, int64_t Val);

  /// \brief Multiplies this canon expr by a blob.
  void multiplyByBlob(unsigned Index);

  /// \brief Populates Indices with all the blobs contained in the CanonExpr
  /// (including blob IV coeffs). The blobs are sorted and uniqued if
  /// MakeUnique is true.
  void collectBlobIndices(SmallVectorImpl<unsigned> &Indices,
                          bool MakeUnique = true) const;

  /// \brief Populates Indices with all the temp blobs contained in the
  /// CanonExpr (including blob IV coeffs). The blobs are sorted and uniqued
  /// if MakeUnique is true.
  void collectTempBlobIndices(SmallVectorImpl<unsigned> &Indices,
                              bool MakeUnique = true) const;

  /// \brief Simplifies canon expr by dividing numerator and denominator by gcd.
  void simplify();

  /// \brief Multiplies the canon expr by Val.
  void multiplyByConstant(int64_t Val);

  /// \brief Negates canon expr.
  void negate();

  /// \brief Verifies canon expression
  void verify() const;
};

} // End loopopt namespace

} // End llvm namespace

#endif
