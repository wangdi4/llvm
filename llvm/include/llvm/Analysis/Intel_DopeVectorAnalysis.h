//===------- Intel_DopeVectorAnalysis.h -----------------------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Dope Vector Analysis
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTELDOPEVECTORANALYSIS_H
#define LLVM_ANALYSIS_INTELDOPEVECTORANALYSIS_H

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace llvm {

namespace dvanalysis {

// Maximum rank for a Fortran array.
const uint32_t FortranMaxRank = 9;

// Argument positions for parameters to subscript intrinsic call.
const unsigned RankOpNum = 0;
const unsigned LBOpNum = 1;
const unsigned StrideOpNum = 2;
const unsigned PtrOpNum = 3;

// Type to store a Function and an argument number
using FuncArgPosPair = std::pair<Function *, unsigned int>;
using FuncArgPosPairSet = SmallSet<FuncArgPosPair, 8>;
using FuncArgPosPairSetIter = SmallSet<FuncArgPosPair, 8>::const_iterator;

// Type to store a collection of SubscriptInst pointers.
using SubscriptInstSet = SmallPtrSet<SubscriptInst *, 16>;
using SubscriptInstSetIter = SubscriptInstSet::const_iterator;

// Type to store a collection of SubscriptInst pointers.
using SubscriptInstSet = SmallPtrSet<SubscriptInst *, 16>;
using SubscriptInstSetIter = SubscriptInstSet::const_iterator;

// An uplevel variable is a structure type that holds values or pointers of
// variables in the parent routine of nested routines. This type is to describe
// an uplevel use of a specific dope vector.  It consists of a variable and the
// field number of the structure containing the dope vector.
using UplevelDVField = std::pair<Value *, uint64_t>;

// Helper routine for checking and getting a constant integer from a GEP
// operand. If the value is not a constant, returns an empty object.
extern Optional<uint64_t> getConstGEPIndex(const GetElementPtrInst &GEP,
                                           unsigned int OpNum);

// Helper routine to get the argument index corresponding to \p Val within the
// call \p CI. If the operand is not passed to the function, or is in more than
// one position, returns an empty object.
extern Optional<unsigned int> getArgumentPosition(const CallInst &CI,
                                                  const Value *Val);

// Check if the type matches the signature for a dope vector.
// Dope vector types look like:
// { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
// where:
//  - the pointer field will be a pointer to the type of the data stored in
//    the source array.
//  - the array dimension varies based on the Rank of the source array.
//  - the integer types in the structure are i64 when compiling with targets
//    that use 64-bit pointers, and i32 for targets using 32-bit pointers.
//
// In the future the FE will provide some metadata to avoid the need to
// pattern match this.
//
// When this function returns 'true', the '*ArrayRank' and '*ElementType'
// are filled in with the rank and element type of the dope vector.
extern bool isDopeVectorType(const Type *Ty, const DataLayout &DL,
                             uint32_t *ArrayRank,
                             Type **ElementType);

// Helper routine to check whether a variable type is a type for an
// uplevel variable.
extern bool isUplevelVarType(Type *Ty);

// Check for arguments of a subscript intrinsic call for the expected values.
// The intrinsic call is declared as:
//    declare <ty>* @llvm.intel.subscript...(i8 <rank>, <ty> <lb>,
//                                           <ty> <stride>, <ty>* <base>,
//                                           <ty> <index>)
//
// Return 'true' if call has the expected values for the Base, if
// CheckForTranspose is set, check that the Rank is equal to the supplied
// value. If the LowerBound and Stride parameters are supplied, also
// check those.
//
extern bool isValidUseOfSubscriptCall(const SubscriptInst &Subs,
                                      const Value &Base,
                                      uint32_t ArrayRank, uint32_t Rank,
                                      bool CheckForTranspose,
                                      Optional<uint64_t> LowerBound = None,
                                      Optional<uint64_t> Stride = None);

// This class is used to collect information about a single field address that
// points to one of the dope vector fields. This is used during dope vector
// analysis to track loads and stores of the field for safety.
class DopeVectorFieldUse {
public:
  using LoadInstSet = SmallPtrSet<LoadInst *, 8>;
  using LoadInstSetIter = LoadInstSet::const_iterator;

  // Normally, we expect at most 1 store instruction
       using StoreInstSet = SmallPtrSet<StoreInst *, 1>;
  using StoreInstSetIter = StoreInstSet::const_iterator;

  DopeVectorFieldUse()
      : IsBottom(false), IsRead(false), IsWritten(false), FieldAddr(nullptr) {}

  DopeVectorFieldUse(const DopeVectorFieldUse &) = delete;
  DopeVectorFieldUse(DopeVectorFieldUse &&) = default;
  DopeVectorFieldUse &operator=(const DopeVectorFieldUse &) = delete;
  DopeVectorFieldUse &operator=(DopeVectorFieldUse &&) = delete;

  bool getIsBottom() const { return IsBottom; }
  bool getIsRead() const { return IsRead; }
  bool getIsWritten() const { return IsWritten; }
  bool getIsSingleValue() const { return !getIsBottom() && Stores.size() == 1; }
  Value *getSingleValue() const {
    if (!getIsSingleValue())
      return nullptr;
    return (*Stores.begin())->getValueOperand();
  }

  void setFieldAddr(Value *V) {
    // If we already saw an object that holds a pointer to the field address,
    // then we go to bottom since we only expect a single Value object to hold
    // the address for the entire function being analyzed.
    if (FieldAddr)
      IsBottom = true;
    FieldAddr = V;
  }

  // Check if the field address has been set.
  bool hasFieldAddr() const { return FieldAddr != nullptr; }

  // Get the set of load instructions.
  iterator_range<LoadInstSetIter> loads() const {
    return iterator_range<LoadInstSetIter>(Loads);
  }

  // Get the set of store instructions.
  iterator_range<StoreInstSetIter> stores() const {
    return iterator_range<StoreInstSetIter>(Stores);
  }

  // Collect the load and store instructions that use the field address. Set the
  // field to Bottom if there are any unsupported uses.
  void analyzeUses();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
  void print(raw_ostream &OS, const Twine &Header) const;
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  bool IsBottom;
  bool IsRead;
  bool IsWritten;

  // Value object that contains the address for the field.
  Value *FieldAddr;

  // Set of locations the field is written to. Used to check what
  // value(s) is stored.
  StoreInstSet Stores;

  // Set of locations the field is loaded. This will be used for examining the
  // usage for profitability heuristics and safety checks.
  LoadInstSet Loads;
};

// This class is for analyzing the uses of all the fields that make up a dope
// vector.
//
// The layout of a dope vector consists of a fixed size block followed by a
// variable sized array: The fixed sized block is (24 or 48 bytes depending on
// the platform):
//   Type* pointer;   /* pointer to array */
//   long length;    /* size of one element of array */
//   long codim;     /* number of co-dimensions, if coarray */
//   long flag;      /* flags */
//   long dim;       /* number of dimensions in array */
//   long reserved; /* used by the backend's openmp support */
//
// The variable sized array (12 or 24 bytes per dimension depending on the
// platform) that is stored at the end is a structure for each dimension of the
// source array containing:
//   long extent;     /* highest index for dimension */
//   long stride;     /* inter element spacing, in bytes */
//   long lower_bound /* lowest index for dimension*/
//
// This class collects the loads/stores to the fields to enable analysis for
// what values are stored, or whether the DV object is read-only.
class DopeVectorAnalyzer {
public:
  // Enumeration fields related to dope vectors. The first 7 items in this
  // list correspond exactly to the field layout of the corresponding dope
  // vector fields, and correspond to GEP indices. Do not re-order these because
  // we directly map GEP index values to them.
  enum DopeVectorFieldType {
    DV_ArrayPtr = 0, // Pointer to array
    DV_ElementSize,  // size of one element of array
    DV_Codim,        // number of co-dimensions
    DV_Flags,        // flag bits
    DV_Dimensions,   // Number of dimensions
    DV_Reserved,
    DV_PerDimensionArray, // Array of structures {extent, stride, lower bound}
                          // for
                          //  each dimension

    // The following field types are indices used to represent the extent,
    // stride or lower bound components for the variable-sized block array
    DV_ExtentBase,
    DV_StrideBase,
    DV_LowerBoundBase,
    DV_Invalid // End of enumeration
  };

  // Each dimension in the dope vector is composed of a structure containing
  // the fields listed in this enumeration.
  enum DopeVectorRankFields { DVR_Extent, DVR_Stride, DVR_LowerBound };

  DopeVectorAnalyzer(Value *DVObject) : DVObject(DVObject) {
    assert(
        DVObject->getType()->isPointerTy() &&
        DVObject->getType()->getPointerElementType()->isStructTy() &&
        DVObject->getType()->getPointerElementType()->getStructNumElements() ==
            7 &&
        DVObject->getType()
            ->getPointerElementType()
            ->getContainedType(6)
            ->isArrayTy() &&
        "Invalid type for dope vector object");

    // The rank of the dope vector can be determined by the array length of
    // array that is the last field of the dope vector.
    Rank = DVObject->getType()
        ->getPointerElementType()
        ->getContainedType(6)
        ->getArrayNumElements();

    // Set as invalid, until analyzed.
    IsValid = false;
  }

  DopeVectorAnalyzer(const DopeVectorAnalyzer &) = delete;
  DopeVectorAnalyzer(DopeVectorAnalyzer &&) = default;
  DopeVectorAnalyzer operator=(const DopeVectorAnalyzer &) = delete;
  DopeVectorAnalyzer operator=(DopeVectorAnalyzer &&) = delete;

  // Accessor for dope vector object
  Value* getDVObject() { return DVObject; }

  // Accessor for array rank
  unsigned long getRank() { return Rank; }

  // Check whether the dope vector was able to be analyzed.
  bool getIsValid() const { return IsValid; }

  // The analysis can only set the state invalid, so only include a method that
  // sets 'IsValid' to false.
  void setInvalid();

  // Provide accessors for fields that the client of dope vector analyzer needs
  // to examine the uses of.
  //
  // Currently, the only field that need to be directly accessible is the array
  // pointer field.
  const DopeVectorFieldUse &getPtrAddrField() const { return PtrAddr; }

  // Helper functions for retrieving value stored to configure the
  // dope vector per-dimension info, if there is a single store to the field.
  //
  Value *getLowerBound(uint32_t Dim) {
    assert(LowerBoundAddr.size() > Dim && "Invalid dimension");
    if (LowerBoundAddr[Dim].hasFieldAddr())
      return LowerBoundAddr[Dim].getSingleValue();
    return nullptr;
  }

  Value *getStride(uint32_t Dim) {
    assert(StrideAddr.size() > Dim && "Invalid dimension");
    if (StrideAddr[Dim].hasFieldAddr())
      return StrideAddr[Dim].getSingleValue();
    return nullptr;
  }

  // Check whether information is available about the stride for the specified
  // dimension.
  bool hasStrideField(uint32_t Dim) const {
    if (StrideAddr.size() <= Dim)
      return false;
    return StrideAddr[Dim].hasFieldAddr();
  }

  // Get the stride field information for the specified dimension.
  const DopeVectorFieldUse &getStrideField(uint32_t Dim) const {
    assert(hasStrideField(Dim) && "Invalid request");
    return StrideAddr[Dim];
  }

  // Get all the store instructions for the stride field for the specified
  // dimension.
  iterator_range<DopeVectorFieldUse::StoreInstSetIter>
  getStrideStores(uint32_t Dim) const {
    assert(StrideAddr.size() > Dim && "Invalid dimension");
    if (StrideAddr[Dim].hasFieldAddr())
      return StrideAddr[Dim].stores();

    // Return an empty set iterator range, if the field wasn't collected.
    return iterator_range<DopeVectorFieldUse::StoreInstSetIter>(
        DopeVectorFieldUse::StoreInstSet());
  }

  Value *getExtent(uint32_t Dim) {
    assert(ExtentAddr.size() > Dim && "Invalid dimension");
    if (ExtentAddr[Dim].hasFieldAddr())
      return ExtentAddr[Dim].getSingleValue();
    return nullptr;
  }

  // Accessor for uplevel variable.
  UplevelDVField getUplevelVar() const { return Uplevel; }

  // Check if any field of the dope vector may be written.
  bool checkMayBeModified() const;

  // Populate \p ValueSet with all the objects that hold the value for the
  // specific dope vector field in \p Field. This set contains all the LoadInst
  // instructions that were identified as loading the value of the field, and
  // all the PHI node and SelectInst instructions the value gets moved to.
  // Returns 'false' if a PHI/Select gets a value that did not originate from a
  // load of the field. Otherwise, returns 'true'.
  bool getAllValuesHoldingFieldValue(const DopeVectorFieldUse &Field,
                                     SmallPtrSetImpl<Value *> &ValueSet) const;

  // Get the number of calls the dope vector is passed to
  uint64_t getNumberCalledFunctions() const { return FuncsWithDVParam.size(); }

  // Check if the uses of the pointer address field results in a load
  // instruction that may result in the address of the array pointer being used
  // for something other than a supported subscript call. Return 'true' if all
  // the uses are supported. If \p SubscriptCalls is not nullptr, this function
  // collects the set of subscript calls which use the address of the array
  // pointer into \p SubscriptCalls.
  bool checkArrayPointerUses(SubscriptInstSet *SubscriptCalls);

  // Accessor for the set of calls taking dope vector as parameter.
  iterator_range<FuncArgPosPairSetIter> funcsWithDVParam() const {
    return iterator_range<FuncArgPosPairSetIter>(FuncsWithDVParam);
  }

  // Walk the uses of the dope vector object to collect information about all
  // the field accesses to check for safety.
  //
  // If \p ForCreation is set, it means the analysis is for the construction of
  // the dope vector, and requires addresses for all fields to be identified.
  // When it is not set, it is allowed to only identify a subset of the Value
  // objects holding field addresses.
  void analyze(bool ForCreation);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Identify the field a getelementptr instruction corresponds to in the dope
  // vector object. Return DV_Invalid if it is not a valid dope vector field.
  static DopeVectorFieldType
  identifyDopeVectorField(const GetElementPtrInst &GEP);

  // For the per-dimension array, we expect to find a sequence of the following
  // form that gets the address of the per-dimensional fields: (This is the GEP
  // passed into this routine):
  //
  // %GEP = getelementptr
  //         {i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }]},
  //         {i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }]}* %2,
  //         i64 0, i32 6, i64 0
  //
  // This routine then traces the use of the GEP to the following pattern to get
  // the address of the a dope vector field {Extent, Stride, Lower Bound} of
  // the first element of the variable sized array.
  //
  // The structure is laid out as: {Extent, Stride, Lower Bound}
  //   %EXTENT = getelementptr {i64, i64, i64}, {i64, i64, i64}* %GEP,
  //               i64 0, i32 0
  //   %STRIDE = getelementptr {i64, i64, i64}, {i64, i64, i64}* %GEP,
  //               i64 0, i32 1
  //   %LB = getelementptr {i64, i64, i64}, {i64, i64, i64}* %GEP,
  //           i64 0, i32 2
  //
  enum FindResult { FR_Invalid, FR_Valid };
  std::pair<GetElementPtrInst *, FindResult>
  findPerDimensionArrayFieldGEP(GetElementPtrInst &GEP,
                                DopeVectorRankFields RankFieldType);
  //
  // Find the object that holds the address for the element of the variable
  // sized array of the dimension desired.
  //
  // The input to this function is the address of the field in the first array
  // element, as computed by findPerDimensionArrayFieldGEP(). This is then used
  // in an IR sequence as follows: (Note, These are being done via the subscript
  // intrinsic rather than GEPs and get lowered later.)
  //
  // For example, on a 2 dimensional array we would have:
  // Getting the lower bound address for each dimension
  // %134 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(
  //                     i8 0, i64 0, i32 24, i64* %LB, i32 1)
  // %131 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(
  //                     i8 0, i64 0, i32 24, i64* %LB, i32 0)
  //
  // Getting the extent address
  // %135 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(
  //                     (i8 0, i64 0, i32 24, i64* %EXTENT, i32 1)
  // %132 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(
  //                     i8 0, i64 0, i32 24, i64* %EXTENT, i32 0)
  //
  // Getting the stride address
  // %133 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(
  //                     i8 0, i64 0, i32 24, i64* %STRIDE, i32 1)
  // %130 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(
  //                     i8 0, i64 0, i32 24, i64* %STRIDE, i32 0)
  Value *findPerDimensionArrayFieldPtr(GetElementPtrInst &FieldGEP,
                                       unsigned Dimension);

  // Check that the subscript calls are using stride values from the dope
  // vector. This should always be true, until dope vector constant
  // propagation is implemented, in which case this transform needs to occur
  // first. Otherwise, this check will invalidate candidates that have
  // had constants substituted into the subscript calls.
  bool checkSubscriptStrideValues(const SubscriptInstSet &SubscriptCalls);

  // This checks the use of a dope vector in a function to verify the fields
  // are not modified and the address of the array does not escape. If
  // SubscriptCalls is not nullptr, add the subscript calls for the dope
  // vector to *SubscriptCalls.
  bool analyzeDopeVectorUseInFunction(const Function &F,
                                      SubscriptInstSet *SubscriptCalls =
                                          nullptr);
private:
  // Value object that represents a dope vector.
  Value *DVObject;

  // Rank for the source array.
  unsigned long Rank;

  // Indicates whether all the uses were successfully analyzed.
  bool IsValid;

  // Information about all field accesses for the dope vector.
  DopeVectorFieldUse PtrAddr;
  DopeVectorFieldUse ElementSizeAddr;
  DopeVectorFieldUse CodimAddr;
  DopeVectorFieldUse FlagsAddr;
  DopeVectorFieldUse DimensionsAddr;
  SmallVector<DopeVectorFieldUse, 4> ExtentAddr;
  SmallVector<DopeVectorFieldUse, 4> StrideAddr;
  SmallVector<DopeVectorFieldUse, 4> LowerBoundAddr;

  // Set of functions that take a dope vector parameter that need to be
  // checked to ensure there is no modification to the dope vector within
  // the function. Pair is: { Function, Argument position }
  FuncArgPosPairSet FuncsWithDVParam;

  // Uplevel variable corresponding to this dope vector. We only expect a single
  // uplevel variable to be created for the dope vector being analyzed, because
  // even if there are multiple routines contained within the routine that
  // created the dope vector, the same uplevel variable is passed to all of
  // them.
  UplevelDVField Uplevel;
};

} // end namespace dvanalysis

} // end namespace llvm

#endif // LLVM_ANALYSIS_INTELDOPEVECTORANALYSIS_H
