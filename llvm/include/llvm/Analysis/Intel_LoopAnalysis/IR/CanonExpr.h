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
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormattedStream.h"

#include <iterator>
#include <set>
#include <stdint.h>
#include <utility>
#include <vector>

namespace llvm {

class Type;
class SCEV;
class MetadataAsValue;
class ConstantFP;
class Constant;
class ConstantData;

namespace loopopt {

class CanonExprUtils;
class BlobUtils;

/// The maximum loopnest level allowed in HIR.
const unsigned MaxLoopNestLevel = 9;
/// Value to represent non-linear level.
const unsigned NonLinearLevel = MaxLoopNestLevel + 1;

/// Canonical form in high level IR
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
    /// Valid index range is [1, UINT_MAX].
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
  /// Copy constructor; only used for cloning.
  CanonExpr(const CanonExpr &);
  /// Make class unassignable.
  void operator=(const CanonExpr &) = delete;

  /// Reference to parent utils object. This is needed to access util functions.
  CanonExprUtils &CEU;

  // SrcTy and DestTy hide one level of casting applied on top of the canonical
  // form.
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

  DebugLoc DbgLoc;

protected:
  CanonExpr(CanonExprUtils &CEU, Type *SrcType, Type *DestType, bool IsSExt,
            unsigned DefLevel, int64_t ConstVal, int64_t Denom,
            bool IsSignedDiv);
  virtual ~CanonExpr() {}

  friend class CanonExprUtils;

  /// Implements hasIV()/numIV() and hasBlobIVCoeffs()/numBlobIVCoeffs()
  /// functionality.
  unsigned numIVImpl(bool CheckIVPresence, bool CheckBlobCoeffs) const;

  /// Resizes IVCoeffs to max loopnest level if the passed in level goes beyond
  /// the current size. This will avoid future reallocs.
  void resizeIVCoeffsToMax(unsigned Lvl);

  /// Sets blob/const coefficient of an IV at a particular loop level.
  /// Overwrite flags indicate what is to be overwritten.
  void setIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff,
                     bool OverwriteIndex, bool OverwriteCoeff);

  /// Adds blob/const coefficient of an IV at a particular loop level.
  void addIVInternal(unsigned Lvl, unsigned Index, int64_t Coeff);

  /// Sets a blob coefficient. Depending upon the overwrite flag the existing
  /// coefficient is either overwritten or added to.
  void addBlobInternal(unsigned BlobIndex, int64_t BlobCoeff, bool overwrite);

  /// Helper to calculate gcd for simplify(). Handles negative integers as well.
  static int64_t simplifyGCDHelper(int64_t CurrentGCD, int64_t Num);

  /// Returns true if it's legal to multiply numerator by an arbitrary value.
  bool canMultiplyNumeratorByUnknown() const;

  /// Returns true if it's legal to multiply numerator by a constant.
  bool canMultiplyNumeratorByConstant(int64_t Val) const;

  /// Multiplies CanonExpr numerator by /p Val. /p Simplify flag indicates
  /// whether simplification can be performed.
  void multiplyNumeratorByConstant(int64_t Val, bool Simplify);

  /// Multiplies numerator of canon expr by a blob.
  void multiplyNumeratorByBlob(unsigned Index);

  /// Implements is*Ext() and isTrunc() functionality.
  bool isExtImpl(bool IsSigned, bool IsTrunc) const;

  /// Implements collect*BlobIndices() functionality.
  void collectBlobIndicesImpl(SmallVectorImpl<unsigned> &Indices,
                              bool MakeUnique, bool NeedTempBlobs) const;

  /// Returns true if the canon expr represents a constant.
  bool isConstInternal() const {
    return (!hasIV() && !hasBlob() && (getDenominator() == 1));
  }

  /// Returns true if canon expr is a constant integer. Integer value is
  /// returned in \pVal. If \pHandleSplat is true, handle a constant Int value
  /// cast to a vector type.
  bool isIntConstantImpl(int64_t *Val, bool HandleSplat) const;

  /// Returns true if canon expr represents a floating point constant.
  /// If yes, returns the underlying LLVM Value in \pVal. If \pHandleSplat is
  /// true, handle a constant FP value cast to a vector type.
  bool isFPConstantImpl(ConstantFP **Val, bool HandleSplat) const;

  /// Returns true if canon expr is a constant integer splat. The constant
  /// integer splat value is returned in \pVal.
  bool isIntConstantSplat(int64_t *Val = nullptr) const;

  /// Returns true if canon expr represents a floating point constant splat. If
  /// yes, returns the underlying LLVM splat Value in \pVal.
  bool isFPConstantSplat(ConstantFP **Val = nullptr) const;

  /// Returns true if canon expr is a vector of constants.
  /// If yes, returns the underlying LLVM Value in \pVal
  bool isConstantVectorImpl(Constant **Val = nullptr) const;

  /// Return the mathematical coefficient to be used in cases where mathematical
  /// addition is performed. The Coeff value in those cases is multiplied by
  /// denominator.
  /// Example: Original CE = i/2 and we want to add a blob 'b1' to it.
  /// If IsMath = true, Result = (i+2*b1)/2 .
  /// If IsMath = false, Result = (i+b1)/2 .
  int64_t getMathCoeff(int64_t Coeff, bool IsMathAdd) {
    return IsMathAdd ? (getDenominator() * Coeff) : Coeff;
  }

  /// Returns true if canon expr represents null pointer value.
  bool isNullImpl() const;

  /// Evaluates the canon expression if it represents constant value and stores
  /// it as C0.
  void simplifyConstantDenom();
  void simplifyConstantCast();

  /// Replaces temp blob with \p TempIndex by new blob with \p Operand index or
  /// constant, depending on the \p IsConstant template argument.
  /// Returns true if it is replaced.
  template <bool IsConstant, typename T>
  bool replaceTempBlobImpl(unsigned TempIndex, T Operand);

public:
  /// Returns parent CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() const { return CEU; }

  /// Returns parent BlobUtils object.
  BlobUtils &getBlobUtils() const;

  CanonExpr *clone() const;

  /// Dumps CanonExpr.
  void dump(bool Detailed) const;
  /// Dumps CanonExpr in a simple format.
  void dump() const;
  /// Prints CanonExpr.
  void print(formatted_raw_ostream &OS, bool Detailed = false) const;

  /// Returns the src type of this canon expr.
  Type *getSrcType() const { return SrcTy; }
  void setSrcType(Type *SrcType) { SrcTy = SrcType; }

  /// Returns the dest type of this canon expr.
  Type *getDestType() const { return DestTy; }
  void setDestType(Type *DestType) { DestTy = DestType; }

  /// Sets both src and dest types to \p Ty.
  void setSrcAndDestType(Type *Ty) { SrcTy = DestTy = Ty; }

  /// Returns true if the canon expr is hiding a signed extension.
  bool isSExt() const;

  /// Returns true if the canon expr is hiding a zero extension.
  bool isZExt() const;

  /// Returns true if the canon expr is hiding a trunc.
  bool isTrunc() const;

  /// Returns true if the canon expr is hiding a pointer to pointer bitcast.
  bool isPtrToPtrCast() const;

  /// Sets the extension type (signed or unsigned) for canon expr. This can be a
  /// no-op depending upon src and dest types.
  void setExtType(bool SExt) { IsSExt = SExt; }

  /// Returns the innermost level at which some blob present in this canon expr
  /// is defined. The canon expr in linear in all the inner loop levels w.r.t
  /// this level. 
  /// It returns NonLinearLevel for non-linear canon exprs.
  unsigned getDefinedAtLevel() const {
    return DefinedAtLevel;
  }
  /// Sets non-negative defined at level.
  void setDefinedAtLevel(unsigned DefLvl) {
    assert((DefLvl <= MaxLoopNestLevel) && "DefLvl exceeds max level!");
    DefinedAtLevel = DefLvl;
  }

  /// Returns true if some blob in the canon expr is defined in the current loop
  /// level.
  bool isNonLinear() const { return (DefinedAtLevel == NonLinearLevel); }

  /// Mark this canon expr as non-linear.
  void setNonLinear() { DefinedAtLevel = NonLinearLevel; }

  /// Returns true if this is linear at all levels.
  bool isProperLinear() const { return (DefinedAtLevel == 0); }

  /// Returns true if this is linear at some levels (greater than
  /// DefinedAtLevel) in the current loopnest.
  bool isLinearAtLevel() const { return !isNonLinear(); }

  /// Returns true if the canon expr is linear at level and does not have IV at
  /// given level.
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

  /// Returns true if canon expr represents any kind of constant.
  bool isConstant() const {
    return (isIntConstant() || isConstantData() || isNull() || isMetadata() ||
            isConstantVector() || isNullVector());
  }

  /// Returns true if canon expr is a constant integer. Integer value is
  /// returned in \p Val.
  bool isIntConstant(int64_t *Val = nullptr) const;

  /// Returns true if canon expr represents a floating point constant.
  /// If yes, returns the underlying LLVM Value in \pVal.
  bool isFPConstant(ConstantFP **Val = nullptr) const;

  /// Returns true if canon expr is a vector of constant Ints.
  /// If yes, returns the underlying LLVM Value in \pVal.
  bool isIntVectorConstant(Constant **Val = nullptr) const;

  /// Returns true if canon expr is a vector of constant FP values.
  /// If yes, returns the underlying LLVM Value in \pVal.
  bool isFPVectorConstant(Constant **Val = nullptr) const;

  /// Returns true if canon expr represents constant data.
  /// If yes, returns the underlying LLVM Value in \pVal.
  bool isConstantData(ConstantData **Val = nullptr) const;

  /// Returns true if canon expr is a vector of constants.
  /// If yes, returns the underlying LLVM Value in \pVal.
  bool isConstantVector(Constant **Val = nullptr) const {
    return (isIntVectorConstant(Val) || isFPVectorConstant(Val));
  }

  /// Returns true if canon expr represents a metadata.
  /// If true, metadata is returned in Val.
  bool isMetadata(MetadataAsValue **Val = nullptr) const;

  /// Returns true if canon expr represents null pointer value.
  bool isNull() const;

  /// Returns true if canon expr represents a vector of null pointer values.
  bool isNullVector() const;

  /// Returns true if this canon expr is a standalone IV (it looks something
  /// like (1 * i3)).
  /// If \p AllowConversion is true, conversions are allowed to be part of a
  /// standalone IV. Otherwise, an IV with a conversion is not considered a
  /// standalone IV.
  bool isStandAloneIV(bool AllowConversion = true) const;

  /// Returns the level of the first IV with coeff different from 0.
  /// It returns 0 if no IV is found with coeff different from 0.
  unsigned getFirstIVLevel() const;

  /// Returns true if this canon expr looks something like (1 * %t).
  /// This is a broader check than isSelfBlob() because it allows the blob to
  /// be a FP constant or even metadata.
  /// If \p AllowConversion is true, conversions are allowed to be part of a
  /// standalone blob. Otherwise, a blob with a conversion is not considered a
  /// standalone blob.
  bool isStandAloneBlob(bool AllowConversion = true) const {
    return ((AllowConversion || (getSrcType() == getDestType())) &&
            !getConstant() && (getDenominator() == 1) && (numBlobs() == 1) &&
            (getSingleBlobCoeff() == 1) && !hasIV());
  }

  // Returns true if the CanonExpr is a unitary blob. A unitary blob is a single
  // (non-nested) standalone blob.
  bool isUnitaryBlob() const;

  /// Returns true if CanonExpr can be converted into a stand alone blob.
  bool canConvertToStandAloneBlob() const;

  /// Merges all the blobs and the constant/denominator into a single compound
  /// blob. If the src/dest types are different the cast is merged into the blob
  /// too. Return value indicates whether conversion was performed.
  bool convertToStandAloneBlob();

  // Converts CE to standalone blob and applies appropriate cast on top. Return
  // value indicates whether conversion was performed. Cast operation is ignored
  // if old and new types are same.
  bool castStandAloneBlob(Type *Ty, bool IsSExt);

  /// Converts CE to a standalone blob and sign extends it to Ty. Return value
  /// indicates whether conversion was performed.
  bool convertSExtStandAloneBlob(Type *Ty);

  /// Converts CE to a standalone blob and zero extends it to Ty. Return value
  /// indicates whether conversion was performed.
  bool convertZExtStandAloneBlob(Type *Ty);

  /// Converts CE to a standalone blob and truncates it to Ty. Return value
  /// indicates whether conversion was performed.
  bool convertTruncStandAloneBlob(Type *Ty);

  /// Returns true if this canon expr looks something like (1 * %t) i.e. a
  /// single blob with a coefficient of 1. Please note that there is an
  /// additional symbase matching requirement for DDRef to be considered a
  /// self-blob. Please refer to description of isSelfBlob() in DDRef.h.
  bool isSelfBlob() const;

  bool isStandAloneUndefBlob() const;

  /// return true if the CanonExpr is zero
  bool isZero() const {
    int64_t Val;
    if (isIntConstant(&Val) && Val == 0) {
      return true;
    }
    return false;
  }

  /// return true if the CanonExpr is one
  bool isOne() const {
    int64_t Val;
    if (isIntConstant(&Val) && Val == 1) {
      return true;
    }
    return false;
  }

  /// Returns true if this expression contains undefined terms.
  bool containsUndef() const;

  /// Returns the constant additive of the canon expr.
  int64_t getConstant() const { return Const; }
  /// Sets the constant additive of the canon expr.
  void setConstant(int64_t Val) { Const = Val; }

  /// Adds a constant value (Val) to the existing constant additive of the canon
  /// expr. If IsMathAdd is set to true (default is false), it performs
  /// mathematical addition by considering denominator in addition.
  void addConstant(int64_t Val, bool IsMathAdd) {
    Const += getMathCoeff(Val, IsMathAdd);
  }

  /// Returns the denominator of the canon expr.
  int64_t getDenominator() const { return Denominator; }

  /// Sets canon expr's denominator. Negates it for negative denominators.
  void setDenominator(int64_t Val);

  /// Multiplies the constant value (Val) with the existing denominatora of the
  /// canon expr. The new denominator equals (Old denominator * Val).
  void divide(int64_t Val) { setDenominator(Denominator * Val); }

  /// Returns true if the division in the canon expr is a signed division.
  bool isSignedDiv() const { return IsSignedDiv; }

  /// Returns true if the division in the canon expr is an unsigned division.
  bool isUnsignedDiv() const { return !isSignedDiv(); }

  /// Sets the division type which can be either signed or unsigned.
  /// This is a no-op for unit denominator.
  void setDivisionType(bool SignedDiv) { IsSignedDiv = SignedDiv; }

  /// Returns true if this contains any IV.
  bool hasIV() const;
  /// Returns true if this contains IV at the given Level.
  bool hasIV(unsigned Level) const;
  /// Returns the number of non-zero IVs in the canon expr.
  unsigned numIVs() const;

  /// Returns true if this contains any Blob IV Coeffs.
  /// Examples: -M*i, N*j
  bool hasIVBlobCoeffs() const;
  /// Returns the number of blobs IV Coeffs.
  unsigned numIVBlobCoeffs() const;
  /// Returns true if this contains any blobs.
  bool hasBlob() const { return !BlobCoeffs.empty(); }
  /// Returns the number of blobs in the canon expr.
  unsigned numBlobs() const { return BlobCoeffs.size(); }

  /// Returns the level of IV associated with this iterator.
  unsigned getLevel(const_iv_iterator ConstIVIter) const;

  /// Returns blob index and coefficient associated with an IV at a particular
  /// loop level. Lvl's range is [1, MaxLoopNestLevel].
  void getIVCoeff(unsigned Lvl, unsigned *Index, int64_t *Coeff) const;
  /// Iterator version of getIVCoeff().
  void getIVCoeff(const_iv_iterator ConstIVIter, unsigned *Index,
                  int64_t *Coeff) const;

  /// Sets the blob index and coefficient associated with an IV at a particular
  /// loop level. Lvl's range is [1, MaxLoopNestLevel].
  void setIVCoeff(unsigned Lvl, unsigned Index, int64_t Coeff);
  /// Iterator version of setIVCoeff().
  void setIVCoeff(iv_iterator IVI, unsigned Index, int64_t Coeff);

  /// Returns the blob coefficient associated with an IV at a particular loop
  /// level. Lvl's range is [1, MaxLoopNestLevel]. Returns invalid value if
  /// there is no blob coeff.
  unsigned getIVBlobCoeff(unsigned Lvl) const;
  /// Iterator version of getIVBlobCoeff().
  unsigned getIVBlobCoeff(const_iv_iterator ConstIVIter) const;

  /// Sets the blob coefficient associated with an IV at a particular loop
  /// level. Lvl's range is [1, MaxLoopNestLevel].
  void setIVBlobCoeff(unsigned Lvl, unsigned Index);
  /// Iterator version of setIVBlobCoeff().
  void setIVBlobCoeff(iv_iterator IVI, unsigned Index);

  /// Returns true if IV has a blob coefficient.
  bool hasIVBlobCoeff(unsigned Lvl) const;
  /// Iterator version of hasIVBlobCoeff().
  bool hasIVBlobCoeff(const_iv_iterator ConstIVIter) const;

  /// Returns the constant coefficient associated with an IV at a particular
  /// loop level. Lvl's range is [1, MaxLoopNestLevel].
  int64_t getIVConstCoeff(unsigned Lvl) const;
  /// Iterator version of getIVConstCoeff().
  int64_t getIVConstCoeff(const_iv_iterator ConstIVIter) const;

  /// Sets the constant coefficient associated with an IV at a particular loop
  /// level. Lvl's range is [1, MaxLoopNestLevel].
  void setIVConstCoeff(unsigned Lvl, int64_t Coeff);
  /// Iterator version of setIVConstCoeff().
  void setIVConstCoeff(iv_iterator IVI, int64_t Coeff);

  /// Returns true if IV has a constant coefficient.
  bool hasIVConstCoeff(unsigned Lvl) const;
  /// Iterator version of hasIVBlobCoeff().
  bool hasIVConstCoeff(const_iv_iterator ConstIVIter) const;

  /// Adds to the existing blob/constant IV coefficients at a particular loop
  /// level. The new IV coefficient looks something like (C1 * b1 + C2 * b2).
  /// Index can be set to zero if only a constant needs to be added. For example
  /// if the canon expr looks like (2 * n) * i1 before change, it will be
  /// modified to (3 + 2 * n) * i1 after a call to addIV(1, 0, 3). If IsMathAdd
  /// is set to true (default is false), it performs mathematical addition by
  /// considering denominator in addition.
  void addIV(unsigned Lvl, unsigned Index, int64_t Coeff,
             bool IsMathAdd = false);
  /// Iterator version of addIV().
  void addIV(iv_iterator IVI, unsigned Index, int64_t Coeff,
             bool IsMathAdd = false);

  /// Removes IV at a particular loop level.
  void removeIV(unsigned Lvl);
  /// Iterator version of removeIV().
  void removeIV(iv_iterator IVI);

  /// Multiplies IV at a particular loop level by a constant.
  void multiplyIVByConstant(unsigned Level, int64_t Val);
  /// Iterator version of multiplyIVByConstant().
  void multiplyIVByConstant(iv_iterator IVI, int64_t Val);

  /// Replaces IV at a particular loop level by a constant.
  void replaceIVByConstant(unsigned Lvl, int64_t Val);
  /// Iterator version of replaceIVByConstant().
  void replaceIVByConstant(iv_iterator IVI, int64_t Val);

  /// Returns the index associated with this blob iterator.
  unsigned getBlobIndex(const_blob_iterator CBlobI) const;

  /// Returns the blob coefficient.
  int64_t getBlobCoeff(unsigned Index) const;
  /// Iterator version of getBlobCoeff().
  int64_t getBlobCoeff(const_blob_iterator CBlobI) const;

  /// Returns the blob index of the only blob.
  unsigned getSingleBlobIndex() const {
    assert((numBlobs() == 1) && "Canon expr does not contain single blob!");
    return BlobCoeffs[0].Index;
  }
  /// Returns the blob coeff of the only blob.
  int64_t getSingleBlobCoeff() const {
    assert((numBlobs() == 1) && "Canon expr does not contain single blob!");
    assert(BlobCoeffs[0].Coeff != 0 && "Single Blob Coeff should not be zero");
    return BlobCoeffs[0].Coeff;
  }

  /// Replaces existing single blob index with \p NewIndex.
  void replaceSingleBlobIndex(unsigned NewIndex) {
    assert((numBlobs() == 1) && "Canon expr does not contain single blob!");
    BlobCoeffs[0].Index = NewIndex;
  }

  /// Sets the blob coefficient.
  void setBlobCoeff(unsigned Index, int64_t Coeff);
  /// Iterator version of setBlobCoeff().
  void setBlobCoeff(blob_iterator BlobI, int64_t Coeff);

  /// Adds to the existing blob coefficient. If IsMathAdd is set to true
  /// (default is false), it performs mathematical addition by considering
  /// denominator in addition.
  void addBlob(unsigned Index, int64_t Coeff, bool IsMathAdd = false);
  /// Iterator version of addBlob().
  void addBlob(blob_iterator BlobI, int64_t Coeff, bool IsMathAdd = false);

  /// Removes a blob. It does not touch IV blob coefficients.
  void removeBlob(unsigned Index);
  /// Iterator version of removeBlob().
  void removeBlob(blob_iterator BlobI);

  /// Replaces an old blob with a new one (including blob IV coeffs).
  void replaceBlob(unsigned OldIndex, unsigned NewIndex);

  /// Replaces temp blob with \p OldTempIndex by new temp blob with
  /// \p NewTempIndex, if it exists in CE. Returns true if it is replaced.
  bool replaceTempBlob(unsigned TempIndex, unsigned NewTempIndex);

  /// Replaces the blob with \p OldTempIndex by the \p Constant value.
  bool replaceTempBlobByConstant(unsigned TempIndex, int64_t Constant);

  /// Clears everything from the CanonExpr except Type. Denominator is set to 1.
  void clear();

  /// Clears all the IV coefficients from the CanonExpr.
  void clearIVs();

  /// Clears all the blobs (excluding blob IV coeffs) from the CanonExpr.
  void clearBlobs() { BlobCoeffs.clear(); }

  /// Shifts the canon expr by a constant offset at a particular loop level.
  /// For example, if CE = 2 * i1, call to shift(1, 1) will result in
  /// CE = 2 * (i1 + 1) => 2 * i1 + 2.
  void shift(unsigned Lvl, int64_t Val);
  /// Iterator version of shift.
  void shift(iv_iterator IVI, int64_t Val);

  /// Populates Indices with all the blobs contained in the CanonExpr
  /// (including blob IV coeffs). The blobs are sorted and uniqued if
  /// \p MakeUnique is true.
  void collectBlobIndices(SmallVectorImpl<unsigned> &Indices,
                          bool MakeUnique = true) const;

  /// Populates Indices with all the temp blobs contained in the CanonExpr
  /// (including blob IV coeffs). The blobs are sorted and uniqued if MakeUnique
  /// is true.
  void collectTempBlobIndices(SmallVectorImpl<unsigned> &Indices,
                              bool MakeUnique = true) const;

  /// Simplifies canon expr by dividing numerator and denominator by gcd.
  void simplify(bool SimplifyCast = false);

  /// Multiplies the canon expr by Val. Returns false if the result expression
  /// can not be represented as a single CanonExpr.
  bool multiplyByConstant(int64_t Val);

  /// Multiplies this canon expr by a blob. Returns false if the result can not
  /// be represented as a single CanonExpr.
  bool multiplyByBlob(unsigned Index);

  /// Negates canon expr.
  void negate() { multiplyNumeratorByConstant(-1, true); }

  /// Verifies that all IVs contained in CE are valid, asserts otherwise.
  bool verifyIVs(unsigned NestingLevel) const;

  /// Verifies canon expression
  void verify(unsigned NestingLevel) const;

  /// Verifies that the incoming nesting level is valid for this CE, asserts
  /// otherwise.
  bool verifyNestingLevel(unsigned NestingLevel) const;

  void setDebugLoc(const DebugLoc &DbgLoc) { this->DbgLoc = DbgLoc; }
  const DebugLoc &getDebugLoc() const { return DbgLoc; }
};

} // End loopopt namespace

} // End llvm namespace

namespace std {

// default_delete<CanonExpr> is a helper for destruction CanonExpr objects to
// support std::unique_ptr<CanonExpr>.
template <> struct default_delete<llvm::loopopt::CanonExpr> {
  void operator()(llvm::loopopt::CanonExpr *CE) const;
};
}

#endif
