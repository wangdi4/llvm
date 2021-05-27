//===------- Intel_DopeVectorAnalysis.h -----------------------------------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
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
extern Optional<uint64_t> getConstGEPIndex(const GEPOperator &GEP,
                                           unsigned int OpNum);

// Helper routine to get the argument index corresponding to \p Val within the
// call \p CI. If the operand is not passed to the function, or is in more than
// one position, returns an empty object.
extern Optional<unsigned int> getArgumentPosition(const CallBase &CI,
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
// Similar to the above function, but to be used when we don't need to
// know the array rank and element type.
extern bool isDopeVectorType(const Type *Ty, const DataLayout &DL);

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
//
// If AllowMultipleFieldAddresses is true then it means that multiple pointers
// can access the field.
class DopeVectorFieldUse {
public:
  using LoadInstSet = SmallPtrSet<LoadInst *, 8>;
  using LoadInstSetIter = LoadInstSet::const_iterator;

  // Normally, we expect at most 1 store instruction
  using StoreInstSet = SmallPtrSet<StoreInst *, 1>;
  using StoreInstSetIter = StoreInstSet::const_iterator;

  DopeVectorFieldUse(bool AllowMultipleFieldAddresses = false)
      : IsBottom(false), IsRead(false), IsWritten(false),
        ConstantValue(nullptr),
        AllowMultipleFieldAddresses(AllowMultipleFieldAddresses) {}

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

  void addFieldAddr(Value *V) {
    // If AllowMultipleFieldAddresses is disabled then only one
    // value should access the current field.
    if (!AllowMultipleFieldAddresses &&
        !FieldAddr.empty() && FieldAddr[0] != V)
        IsBottom = true;
    FieldAddr.insert(V);
  }

  // Check if the field address has been set.
  bool hasFieldAddr() const { return !FieldAddr.empty(); }

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

  // Collect the load and store instructions that access the field address
  // through a subscript.
  void analyzeSubscriptsUses();

  // Insert a subscript instruction that will be processed by
  // analyzeSubscriptsUses
  void collectSubscriptInformation(SubscriptInst *I,
                                   DopeVectorFieldType FieldType,
                                   unsigned long DVRank);

  // Return the constant value collected
  ConstantInt *getConstantValue() { return ConstantValue; }

  // Analyze the store instructions collected to identify the possible constant
  // value for the current field
  void identifyConstantValue();

  // Indicate that multiple field addresses are allowed.
  void setAllowMultipleFieldAddresses();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
  void print(raw_ostream &OS, const Twine &Header) const;
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  bool IsBottom;
  bool IsRead;
  bool IsWritten;

  // SetVector that contains the addresses for the field.
  SetVector<Value *> FieldAddr;

  // Set of locations the field is written to. Used to check what
  // value(s) is stored.
  StoreInstSet Stores;

  // Set of locations the field is loaded. This will be used for examining the
  // usage for profitability heuristics and safety checks.
  LoadInstSet Loads;

  // Store the subscripts that access the extent, stride or lower bound
  SubscriptInstSet Subscripts;

  // Constant value collected for the current field
  ConstantInt *ConstantValue;

  // False if the DopeVectorFieldUse is limited to only one field address.
  // In the case of local dope vectors, we may have only one instruction
  // accessing the field address. On the other hand, for nested dope vectors
  // there is a chance for multiple pointers could access the current field.
  // The default value is false.
  bool AllowMultipleFieldAddresses;

  // Return true if the input value V is a load instruction, or a store
  // instruction where the pointer operand is Pointer.
  bool analyzeLoadOrStoreInstruction(Value *V, Value *Pointer);
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

  // Get all the store instructions for the extent field for the specified
  // dimension.
  iterator_range<DopeVectorFieldUse::StoreInstSetIter>
  getExtentStores(uint32_t Dim) const {
    assert(ExtentAddr.size() > Dim && "Invalid dimension");
    if (ExtentAddr[Dim].hasFieldAddr())
      return ExtentAddr[Dim].stores();

    // Return an empty set iterator range, if the field wasn't collected.
    return iterator_range<DopeVectorFieldUse::StoreInstSetIter>(
        DopeVectorFieldUse::StoreInstSet());
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
  // If DopeVectorIndex is larger than 0 then it means that the dope vector
  // information starts at that index in the GEP. For example:
  //
  //    %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE",
  //          %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 6, i64 0, i32 1
  //
  // In the example above, %"ARR_MOD$.btT_TESTTYPE" is a structure where
  // field 1 is a dope vector. The index 2 in the GEP %92 represents the access
  // to the dope vector, while the rest of the fields represents the access to
  // the fields of the dope vector.
  //
  //   * DopeVectorIndex == 0  -  Source type of the GEP is a dope vector
  //   * DopeVectorIndex == 1  -  Not allowed, operand 1 in the GEP is reserved
  //                              for the array index and should be 0
  //                              (return DopeVectorFieldType::DV_Invalid)
  //   * DopeVectorIndex >= 2  -  Source type of the GEP is a structure where
  //                              field DopeVectorIndex is a dope vector
  //
  // The default value is 0.
  static DopeVectorFieldType
  identifyDopeVectorField(const GEPOperator &GEP, uint64_t DopeVectorIndex = 0);

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

// Helper class to handle all the information related to one dope vector.
// The constructor and private fields are similar to the DopeVectorAnalyzer
// class. A dope vector is a LLVM structure whith the following form:
//
//    {LLVMType*, i64, i64, i64, i64, i64, CONSTANT INT x [i64, i64, i64] }
//
// Each field represents the following:
//
//  0. Array pointer address
//  1. Element size
//  2. Co-Dimensions
//  3. Flags
//  4. Dimension (Rank)
//  5. Reserved
//  6. Per-Dimension array: {Extent, Stride, LowerBound}
//
// If AllowMultipleFieldAddresses is true the it will be allowed to store
// multiple pointers for each dope vector field.
class DopeVectorInfo {

public:

  // Enum for tracking the results of the dope vector info analysis
  enum AnalysisResult {
    AR_Top = 0,                    // Initial value
    AR_Invalid,                    // Invalid dope vector info
    AR_ReadIllegality,             // A field was read when it shouldn't have
                                   //   been
    AR_WriteIllegality,            // A field was written when it shouldn't
                                   //   been
    AR_NoAllocSite,                // Alloc site wasn't collected
    AR_AllocStoreIllegality,       // Couldn't prove that the store will always
                                   //   happen with the allocation
    AR_Pass                        // Analysis pass
  };

  // Constructor for DopeVectorInfo. Technically is the same constructor
  // as DopeVectorAnalyzer, but the classes have different purposes.
  DopeVectorInfo (Value *DVObject, Type *DVType,
                  bool AllowMultipleFieldAddresses = false);

  ~DopeVectorInfo() {
    ExtentAddr.clear();
    StrideAddr.clear();
    LowerBoundAddr.clear();
  }

  // Return the object that represents the dope vector
  Value *getDVObject() { return DVObject; }

  // Check if all fields in the dope vector are valid (not set to bottom) and
  // if the allocation site was found.
  void validateDopeVector();

  // Given a dope vector field type, return the dope vector field. If the field
  // type is DV_ExtentBase, DV_StrideBase or DV_LowerBoundBase then the array
  // entry is needed.
  DopeVectorFieldUse *getDopeVectorField(
      DopeVectorFieldType DVFieldType, uint64_t ArrayEntry = UINT_MAX);

  // Return the dope vector rank
  unsigned long getRank() { return Rank; }

  // Set the allocation site
  void setAllocSite(CallBase *Call) {
    // There should be only one alloc site
    if (AllocSiteSet || !Call)
      AllocSite = nullptr;
    else
      AllocSite = Call;

    AllocSiteSet = true;
  }

  // Return the allocation site
  CallBase* getAllocSite() { return AllocSite; }

  // Return true if the allocation site was set
  bool isAllocSiteSet() { return AllocSiteSet; }

  // Get the type that represents the current dope vector
  StructType *getLLVMStructType() { return LLVMDVType; }

  // Return the analysis result
  AnalysisResult getAnalysisResult() { return AnalysisRes; }

  // Invalidate dope vector info
  void invalidateDopeVectorInfo() {
    if (AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Top ||
        AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Pass)
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Invalid;
  }

  // Set to true if the process of constant propagation was applied
  // to the current dope vector
  void setConstantsPropagated() {
    if (!ConstantsPropagated &&
        AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Pass)
      ConstantsPropagated = true;
  }

  // Return true if the constants were propagated for the current
  // dope vector
  bool getConstantsPropagated() { return ConstantsPropagated; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print the analysis results for debug purposes
  void print(uint64_t Indent);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

protected:
  // Value object that represents a dope vector.
  Value *DVObject;

  // Rank for the source array.
  unsigned long Rank;

  // Information about all field accesses for the dope vector.
  DopeVectorFieldUse PtrAddr;
  DopeVectorFieldUse ElementSizeAddr;
  DopeVectorFieldUse CodimAddr;
  DopeVectorFieldUse FlagsAddr;
  DopeVectorFieldUse DimensionsAddr;
  SmallVector<DopeVectorFieldUse, 4> ExtentAddr;
  SmallVector<DopeVectorFieldUse, 4> StrideAddr;
  SmallVector<DopeVectorFieldUse, 4> LowerBoundAddr;

  // Store the instruction that will allocate the dope vector, this will be
  // used for the purpose of analysis
  CallBase *AllocSite;

  // LLVM structure type of the current dope vector
  StructType *LLVMDVType;

  // True if the allocation site was found
  bool AllocSiteSet;

  // Result from the analysis process
  AnalysisResult AnalysisRes;

  // True if the constant propagation was already applied to the current
  // dope vector
  bool ConstantsPropagated;
};

// Helper class to handle the nested dope vectors. Nested dope vectors are dope
// vectors that are fields of a structure, and a pointer to the structure is
// used as the array pointer (field 0) of another dope vector. For example:
//
//   %test.nested_dv = type {double*, i64, i64, i64, i64, i64,
//                           3 x [i64, i64, i64] }
//
//   %test.struct = type { %test.nested_dv }
//
//   %test.dope_vector = type {%test.struct*, i64, i64, i64, i64, i64,
//                             1 x [i64, i64, i64] }
//
// In the above case, the array pointer (field 0) for %test.dope_vector is a
// pointer to the structure %test.struct. The field 0 for this structure is
// another dope vector (%test.nested_dv). This means that %test.nested_dv is a
// nested dope vector where the field number (FieldNum) is 0.
class NestedDopeVectorInfo : public DopeVectorInfo {
public:
  NestedDopeVectorInfo(Value *DVObject, Type *DVType, uint64_t FieldNum,
      bool AllowMultipleFieldAddresses = false) : DopeVectorInfo(DVObject,
      DVType, AllowMultipleFieldAddresses), FieldNum(FieldNum) { }
  uint64_t getFieldNum() { return FieldNum; }

  // Analyze the fields of the nested dope vector
  void analyzeNestedDopeVector();

private:
  uint64_t FieldNum;
};

// Helper class to handle a dope vector that is a global variable. A global
// dope vector is primarily composed of two parts:
//
//   1) GlobalDVInfo: The information related to the dope vector fields
//   2) NestedDopeVectors: Set that stores the information of each
//                         nested dope vector
class GlobalDopeVector {

public:

  // Enum for tracking the result of the analysis process:
  //
  //   * AR_Top:                     Initial value
  //   * AR_GlobalDVAnalysisFailed:  Analysis of the global dope vector failed
  //   * AR_IncompleteNestedDVData:  Collecting the nested dope vectors failed
  //   * AR_BadNestedDV:             At least one nested dope vector didn't
  //                                   pass analysis
  //   * AR_Pass:                    Global dope vector and nested dope vectors
  //                                   passed the analysis
  enum AnalysisResult {
    AR_Top = 0,
    AR_GlobalDVAnalysisFailed,
    AR_IncompleteNestedDVData,
    AR_BadNestedDV,
    AR_Pass
  };

  GlobalDopeVector(GlobalVariable *Glob, Type *DVType,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) :
      GlobalDVInfo(new DopeVectorInfo(Glob, DVType)), Glob(Glob),
      GetTLI(GetTLI), NestedDVDataCollected(false),
      AnalysisRes(GlobalDopeVector::AnalysisResult::AR_Top) { }

  ~GlobalDopeVector() {
    delete GlobalDVInfo;
    Glob = nullptr;
    for (auto NestedDV : NestedDopeVectors)
      delete NestedDV;

    NestedDopeVectors.clear();
  }

  // Return the dope vector information related to the global
  DopeVectorInfo *getGlobalDopeVectorInfo() {
    return GlobalDVInfo;
  }

  // Given an entry, return the nested dope vector in the pointer address if
  // it exists, else return nullptr.
  NestedDopeVectorInfo *getNestedDopeVector(uint64_t Entry) {
    for (auto *NestedDV : NestedDopeVectors)
      if (NestedDV->getFieldNum() == Entry)
        return NestedDV;
    return nullptr;
  }

  // Return all nested dope vectors
  auto getAllNestedDopeVectors() const { return NestedDopeVectors; }

  // Return true if the current global contains nested dope vectors
  bool hasNestedDopeVectors() { return !NestedDopeVectors.empty(); }

  // Return the number of nested dope vectors
  uint64_t getNumNestedDopeVector() { return NestedDopeVectors.size(); }

  // Collect and validate the global variable and all nested dope vectors
  void collectAndValidate(const DataLayout &DL);

  AnalysisResult getAnalysisResult() { return AnalysisRes; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print the information for debug purposes.
  void print();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
private:
  // Store the dope vector information related to the global variable
  DopeVectorInfo *GlobalDVInfo;

  // Store the data related to the nested dope vectors
  SetVector<NestedDopeVectorInfo*> NestedDopeVectors;

  // Store the global
  GlobalVariable *Glob;

  // Target library analysis
  std::function<const TargetLibraryInfo &(Function &F)> &GetTLI;

  // True if the nested dope vectors were collected correctly. Default
  // value is false.
  bool NestedDVDataCollected;

  // Result from validating the data in the global dope vector. Default
  // value is AR_Top
  AnalysisResult AnalysisRes;

  // Traverse through the users of the subscript instruction to identify
  // the nested dope vectors and analyze the use
  bool collectNestedDopeVectorFromSubscript(SubscriptInst *SI,
      const DataLayout &DL);

  // Given a GEP operator, check which dope vector field is being accessed,
  // collect the data and analyze it.
  bool collectAndAnalyzeGlobalDopeVectorField(GEPOperator *GEP);

  // Return true if the input BitCast operator is used for allocation
  bool collectAndAnalyzeAllocSite(BitCastOperator *BC);

  // Collect the nested dope vectors for the global variable
  void collectAndAnalyzeNestedDopeVectors(const DataLayout &DL);

  // Validate that all the data was collected correctly
  void validateGlobalDopeVector();
};

} // end namespace dvanalysis

} // end namespace llvm

#endif // LLVM_ANALYSIS_INTELDOPEVECTORANALYSIS_H
