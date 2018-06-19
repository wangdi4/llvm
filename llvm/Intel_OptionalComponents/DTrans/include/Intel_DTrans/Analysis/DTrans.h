//===--------------- DTrans.h - Class definition -*- C++ -*----------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// General definitions required by DTrans.
///
// ===--------------------------------------------------------------------=== //

#if !INTEL_INCLUDE_DTRANS
#error DTrans.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANS_H
#define INTEL_DTRANS_ANALYSIS_DTRANS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"

namespace llvm {

class TargetLibraryInfo;
class Function;
class Instruction;
class Type;
class StructType;
class PointerType;
class CallInst;
class Value;
class Constant;
class raw_ostream;

namespace dtrans {

//
// Enum to indicate the "single value" status of a field:
//   None: No write to field seen
//   Single: Only a single value for the field.  The value is obtained with
//     getSingleValue()
//   Multiple: Potentially or actually multiple values for the field
//
enum SingleValueKind { SVK_None, SVK_Single, SVK_Multiple };

//
// Enum to indicate the "single value" status of a field:
//   Top: No write to field seen
//   Single: The field is assigned either nullptr or the return value of
//     calling a specific function which has been determined to be malloc-like
//     by the DtransAllocAnalyzer.
//   Bottom: Anything else, including an assignment by something other
//     than a nullptr or assignments from return values of multiple
//     functions.
//
enum SingleAllocFunctionKind { SAFK_Top, SAFK_Single, SAFK_Bottom };

class FieldInfo {
public:
  FieldInfo(llvm::Type *Ty)
      : LLVMType(Ty), Read(false), Written(false), ComplexUse(false),
        AddressTaken(false), SVKind(SVK_None), SingleValue(nullptr),
        SAFKind(SAFK_Top), SingleAllocFunction(nullptr) {}

  llvm::Type *getLLVMType() const { return LLVMType; }

  bool isRead() const { return Read; }
  bool isWritten() const { return Written; }
  bool hasComplexUse() const { return ComplexUse; }
  bool isAddressTaken() const { return AddressTaken; }
  bool isNoValue() const { return SVKind == SVK_None; }
  bool isTopAllocFunction() const { return SAFKind == SAFK_Top; }
  bool isSingleValue() const { return SVKind == SVK_Single; }
  bool isSingleAllocFunction() const { return SAFKind == SAFK_Single; }
  bool isMultipleValue() const { return SVKind == SVK_Multiple; }
  bool isBottomAllocFunction() const { return SAFKind == SAFK_Bottom; }
  llvm::Constant *getSingleValue() {
    return SVKind == SVK_Single ? SingleValue : nullptr;
  }
  llvm::Function *getSingleAllocFunction() {
    return SAFKind == SAFK_Single ? SingleAllocFunction : nullptr;
  }
  void setRead(bool b) { Read = b; }
  void setWritten(bool b) { Written = b; }
  void setComplexUse(bool b) { ComplexUse = b; }
  void setAddressTaken() { AddressTaken = true; }
  void setSingleValue(llvm::Constant *C) {
    SVKind = SVK_Single;
    SingleValue = C;
  }
  void setSingleAllocFunction(llvm::Function *F) {
    assert((SAFKind == SAFK_Top) && "Expecting lattice at top");
    SAFKind = SAFK_Single;
    SingleAllocFunction = F;
  }
  void setMultipleValue() {
    SVKind = SVK_Multiple;
    SingleValue = nullptr;
  }
  void setBottomAllocFunction() {
    SAFKind = SAFK_Bottom;
    SingleAllocFunction = nullptr;
  }
  //
  // Update the "single value" of the field, given that a constant value C
  // for the field has just been seen. Return true if the value is updated.
  //
  bool processNewSingleValue(llvm::Constant *C);
  //
  // Update the single alloc function for the field, given that we have just
  // seen an assignment to it from the return value of a call to F. Return
  // true if the value is updated.
  //
  bool processNewSingleAllocFunction(llvm::Function *F);

private:
  llvm::Type *LLVMType;
  bool Read;
  bool Written;
  bool ComplexUse;
  bool AddressTaken;
  SingleValueKind SVKind;
  llvm::Constant *SingleValue;
  SingleAllocFunctionKind SAFKind;
  llvm::Function *SingleAllocFunction;
};

/// DTrans optimization safety conditions for a structure type.
typedef uint64_t SafetyData;

/// No conditions were observed that could prevent legal optimization of the
/// type.
const SafetyData NoIssues = 0;

/// A cast was seen that may make this type a bad candidate for optimization.
/// This flag covers multiple casting problems, including casting of a
/// pointers from one type to another and casting of pointers to fields
/// within a structure to other types.
const SafetyData BadCasting = 0x0000000000000001;

/// The size arguments passed to an allocation call could not be proven to
/// be a multiple of the size of the type being allocated.
const SafetyData BadAllocSizeArg = 0x0000000000000002;

/// A pointer to an aggregate type is manipulated to compute an address that
/// is not the address of a field within the type.
const SafetyData BadPtrManipulation = 0x0000000000000004;

/// An i8* value that may alias to multiple types is passed to a GetElementPtr
/// instruction.
const SafetyData AmbiguousGEP = 0x0000000000000008;

/// A volatile memory operation was found operating on the type on one of its
/// elements.
const SafetyData VolatileData = 0x0000000000000010;

/// A load or store operation was used with a pointer to an element within an
/// aggregate type, but the type of value loaded or stored did not match the
/// element type.
const SafetyData MismatchedElementAccess = 0x0000000000000020;

/// A load or store instruction was found which loads or stores an entire
/// instance of the type.
const SafetyData WholeStructureReference = 0x0000000000000040;

/// A store was seen using a value operand that aliases to a type of interest
/// with a pointer operand that was not known to alias to a pointer to a
/// pointer to that type.
const SafetyData UnsafePointerStore = 0x0000000000000080;

/// The addresses of one or more fields within the type were written to memory,
/// passed as an argument to a function call, or returned from a function.
const SafetyData FieldAddressTaken = 0x0000000000000100;

/// A global variable was found which is a pointer to the type.
const SafetyData GlobalPtr = 0x0000000000000200;

/// A global variable was found which is an instance of the type.
const SafetyData GlobalInstance = 0x0000000000000400;

/// A global variable was found which is an instance of the type and has a
/// non-zero initializer.
const SafetyData HasInitializerList = 0x0000000000000800;

/// A PHI node or select was found with incompatible incoming values.
const SafetyData UnsafePtrMerge = 0x0000000000001000;

/// A structure is modified via a memory function intrinsic (memcpy, memmove,
/// or memset), with a size that differs from the native structure size.
const SafetyData BadMemFuncSize = 0x0000000000002000;

/// A proper subset of fields in a structure is modified via a memory function
/// intrinsic (memcpy, memmove, or memset).
const SafetyData MemFuncPartialWrite = 0x0000000000004000;

/// A structure is modified via a memory function intrinsic (memcpy or memmove)
/// with conflicting or unknown types for the source and destination parameters.
const SafetyData BadMemFuncManipulation = 0x0000000000008000;

/// A pointer is passed to an intrinsic or library function that can alias
/// incompatible types.
const SafetyData AmbiguousPointerTarget = 0x0000000000010000;

/// The address of an aggregate object escaped through a function call or
/// a return statement.
const SafetyData AddressTaken = 0x0000000000020000;

/// The structure was declared with no fields.
const SafetyData NoFieldsInStruct = 0x0000000000040000;

/// The structure is contained as a non-pointer member of another structure.
const SafetyData NestedStruct = 0x0000000000080000;

/// The structure contains another structure as a non-pointer member.
const SafetyData ContainsNestedStruct = 0x0000000000100000;

/// The structure was identified as a system object type.
const SafetyData SystemObject = 0x0000000000200000;

/// A local variable was found which is a pointer to the type.
const SafetyData LocalPtr = 0x00000000000400000;

/// A local variable was found which is an instance of the type.
const SafetyData LocalInstance = 0x0000000000000800000;

/// This is a catch-all flag that will be used to mark any usage pattern
/// that we don't specifically recognize. The use might actually be safe
/// or unsafe, but we will conservatively assume it is unsafe.
const SafetyData UnhandledUse = 0x8000000000000000;

/// A three value enum that indicates whether for a particular Type of
/// interest if a there is another distinct Type with which it is compatible
/// by C language rules.
///   CRT_Unknown: We don't know if there is such a type. If we need to
///     know, we will do analysis to determine if there is.
///   CRT_False: We know that there is no such compatible type.
///   CRT_True: We know that there is such a compatible type.
enum CRuleTypeKind { CRT_Unknown, CRT_False, CRT_True };

/// An object describing the DTrans-related characteristics of an LLVM type.
class TypeInfo {
public:
  /// Definitions to support type inquiry through isa, cast, and dyn_cast
  enum TypeInfoKind { NonAggregateInfo, PtrInfo, StructInfo, ArrayInfo };
  TypeInfoKind getTypeInfoKind() const { return TIK; }

protected:
  // This class should only be instantiated through its subclasses.
  TypeInfo(TypeInfoKind Kind, llvm::Type *Ty)
      : LLVMTy(Ty), SafetyInfo(0), TIK(Kind), CRTypeKind(CRT_Unknown) {}

public:
  llvm::Type *getLLVMType() const { return LLVMTy; }

  bool testSafetyData(SafetyData Conditions) const {
    // If any unhandled uses have been seen, assume all conditions are set.
    if (SafetyInfo & dtrans::UnhandledUse)
      return true;
    return (SafetyInfo & Conditions);
  }
  void setSafetyData(SafetyData Conditions);
  void resetSafetyData(SafetyData Conditions) { SafetyInfo &= ~Conditions; }
  void clearSafetyData() { SafetyInfo = 0; }

  void printSafetyData();

  CRuleTypeKind getCRuleTypeKind() { return CRTypeKind; }
  void setCRuleTypeKind(CRuleTypeKind K) { CRTypeKind = K; }

private:
  llvm::Type *LLVMTy;
  SafetyData SafetyInfo;

  // ID to support type inquiry through isa, cast, and dyn_cast
  TypeInfoKind TIK;
  // Indicates whether the Type has a C language rule compatible Type
  CRuleTypeKind CRTypeKind;
};

//
// Safety conditions for field single value analysis
//
const SafetyData SDFieldSingleValue =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
    MismatchedElementAccess | UnsafePointerStore | FieldAddressTaken |
    BadMemFuncSize | BadMemFuncManipulation | AmbiguousPointerTarget |
    UnsafePtrMerge | AddressTaken | UnhandledUse;

const SafetyData SDSingleAllocFunction =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
    MismatchedElementAccess | UnsafePointerStore | FieldAddressTaken |
    BadMemFuncSize | BadMemFuncManipulation | AmbiguousPointerTarget |
    UnsafePtrMerge | AddressTaken | UnhandledUse;

class NonAggregateTypeInfo : public TypeInfo {
public:
  NonAggregateTypeInfo(llvm::Type *Ty)
      : TypeInfo(TypeInfo::NonAggregateInfo, Ty) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::NonAggregateInfo;
  }
};

class PointerInfo : public TypeInfo {
public:
  PointerInfo(llvm::Type *Ty) : TypeInfo(TypeInfo::PtrInfo, Ty) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::PtrInfo;
  }
};

class StructInfo : public TypeInfo {
public:
  StructInfo(llvm::Type *Ty, ArrayRef<llvm::Type *> FieldTypes)
      : TypeInfo(TypeInfo::StructInfo, Ty) {
    for (llvm::Type *FieldTy : FieldTypes)
      Fields.push_back(FieldInfo(FieldTy));
  }

  size_t getNumFields() const { return Fields.size(); }
  SmallVectorImpl<FieldInfo> &getFields() { return Fields; }
  FieldInfo &getField(size_t N) { return Fields[N]; }

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::StructInfo;
  }

private:
  SmallVector<FieldInfo, 16> Fields;
};

class ArrayInfo : public TypeInfo {
public:
  ArrayInfo(llvm::Type *Ty, dtrans::TypeInfo *DTransElemTy, size_t Size)
      : TypeInfo(TypeInfo::ArrayInfo, Ty), DTransElemTy(DTransElemTy),
        NumElements(Size) {}

  TypeInfo *getElementDTransInfo() const { return DTransElemTy; }
  llvm::Type *getElementLLVMType() const { return DTransElemTy->getLLVMType(); }
  size_t getNumElements() const { return NumElements; }

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::ArrayInfo;
  }

private:
  TypeInfo *DTransElemTy;
  size_t NumElements;
};

/// Kind of allocation associated with a Function.
/// The malloc, calloc, and realloc allocation kinds each correspond to a call
/// to the standard library function of the same name.  C++ new operators are
/// not currently supported.
enum AllocKind {
  AK_NotAlloc,
  AK_Malloc,
  AK_Calloc,
  AK_Realloc,
  AK_UserMalloc,
  AK_UserMalloc0
};

/// Kind of free function call.
/// - FK_Free represents a direct call to the standard library function 'free'
/// - FK_UserFree represents a call to a user-wrapper function of 'free''
enum FreeKind { FK_NotFree, FK_Free, FK_UserFree };

/// Get a printable string for the AllocKind
StringRef AllocKindName(AllocKind Kind);

/// Get a printable string for the FreeKind
StringRef FreeKindName(FreeKind Kind);

/// Get a printable string for the CRuleTypeKind
StringRef CRuleTypeKindName(CRuleTypeKind Kind);

// This structure is used to describe the affected portion of an aggregate type
// passed as an argument of the memfunc call. This will be used to communicate
// information collected during the analysis to the transforms about how
// a memfunc call is impacting a structure.
struct MemfuncRegion {
  MemfuncRegion() : IsCompleteAggregate(true), FirstField(0), LastField(0) {}

  // If this is 'false', the FirstField and LastField members must be set
  // to indicate an inclusive set of fields within the structure that are
  // affected. If this is 'true', the FieldField and LastField member values
  // are undefined.
  bool IsCompleteAggregate;

  // If the region is a description of a partial structure modification, these
  // members specify the first and last fields touched.
  unsigned int FirstField;
  unsigned int LastField;
};

// This class is used to hold information that has been
// extracted from the LocalPointerInfo to contain a
// list of aggregate types being used by one of the tracked
// call instructions. This is kept outside of the CallInfo
// class itself to allow for cases where type information needs
// to be tracked for more than a single function argument.
class PointerTypeInfo {
public:
  typedef SmallVector<llvm::Type *, 2> PointerTypeAliasSet;
  typedef SmallVectorImpl<llvm::Type *> &PointerTypeAliasSetRef;

  PointerTypeInfo() : AliasesToAggregatePointer(false), Analyzed(false) {}

  // Returns 'true' if the type (at some level of indirection)
  // was known to be a pointer to an aggregate type.
  bool getAliasesToAggregatePointer() const {
    return AliasesToAggregatePointer;
  }

  void setAliasesToAggregatePointer(bool Val) {
    AliasesToAggregatePointer = Val;
  }

  void setAnalyzed(bool Val) { Analyzed = Val; }

  bool getAnalyzed() const { return Analyzed; }

  void addType(llvm::Type *Ty) {
    assert(isa<llvm::PointerType>(Ty) &&
           "PointerTypeInfo::addType: Expecting pointer type");
    Types.push_back(Ty);
  }
  PointerTypeAliasSetRef getTypes() { return Types; }

  size_t getNumTypes() { return Types.size(); }

  llvm::Type *getType(size_t Idx) const {
    assert(Idx < Types.size() && "Index out of range");
    return Types[Idx];
  }

  // Change the type at index \p Idx to type \p Ty. This
  // function should only be used for updating a type based
  // on the type remapping done when processing a function.
  void setType(size_t Idx, llvm::Type *Ty) {
    assert(Idx < Types.size() && "Index out of range");
    Types[Idx] = Ty;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // When true, indicates that the base type for one or more of the pointer
  // types collected for the pointer was an aggregate type.
  bool AliasesToAggregatePointer;

  // When true, indicates the LocalPointerAnalysis was performed to collect type
  // information for the pointer.
  bool Analyzed;

  // List of pointer to aggregate types resolved by the local pointer analysis
  // for this item.
  PointerTypeAliasSet Types;
};

// Base class for storing collected information about specific
// call instructions.
class CallInfo {
public:
  /// Kind of function or intrinsic call.
  enum CallInfoKind { CIK_Alloc, CIK_Free, CIK_Memfunc };

  CallInfoKind getCallInfoKind() const { return CIK; }

  Instruction *getInstruction() const { return I; }
  void setInstruction(Instruction *NewI) { I = NewI; }

  bool getAliasesToAggregatePointer() const {
    return PTI.getAliasesToAggregatePointer();
  }

  void setAliasesToAggregatePointer(bool Val) {
    PTI.setAliasesToAggregatePointer(Val);
  }

  void setAnalyzed(bool Val) { PTI.setAnalyzed(Val); }

  bool getAnalyzed() const { return PTI.getAnalyzed(); }

  void addType(llvm::Type *Ty) { PTI.addType(Ty); }

  PointerTypeInfo &getPointerTypeInfoRef() { return PTI; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

protected:
  CallInfo(Instruction *I, CallInfoKind Kind) : I(I), CIK(Kind) {}

  // Instruction the info corresponds to.
  Instruction *I;

  // The type list from the local pointer analysis.
  PointerTypeInfo PTI;

private:
  // ID to support type inquiry through isa, cast, and dyn_cast
  CallInfoKind CIK;
};

/// The AllocCallInfo class tracks a memory allocation site that dynamically
/// allocates a type of interest.
class AllocCallInfo : public CallInfo {
public:
  AllocCallInfo(Instruction *I, AllocKind AK)
      : CallInfo(I, CallInfo::CIK_Alloc), AK(AK) {}

  AllocCallInfo(const AllocCallInfo &) = delete;
  AllocCallInfo &operator=(const AllocCallInfo &) = delete;

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const CallInfo *CI) {
    return CI->getCallInfoKind() == CallInfo::CIK_Alloc;
  }

  AllocKind getAllocKind() const { return AK; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  AllocKind AK;
};

/// The FreeCallInfo class tracks the TypeInfo for a call to 'free' that
/// releases a type of interest.
class FreeCallInfo : public CallInfo {
public:
  explicit FreeCallInfo(Instruction *I, FreeKind FK)
      : CallInfo(I, CallInfoKind::CIK_Free), FK(FK) {}

  FreeCallInfo(const FreeCallInfo &) = delete;
  FreeCallInfo &operator=(const FreeCallInfo &) = delete;

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const CallInfo *CI) {
    return CI->getCallInfoKind() == CallInfo::CIK_Free;
  }

  FreeKind getFreeKind() const { return FK; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  FreeKind FK;
};

/// The MemfuncCallInfo class tracks a call to a memfunc that impacts a
/// type that DTrans may need to transform. The memfunc analysis supports
/// identifying when a complete aggregate is affected, or in the case of
/// a structure, when a subset of fields is affected.
class MemfuncCallInfo : public CallInfo {
public:
  /// Kind of memfunc intrinsic call that was analyzed.
  enum MemfuncKind { MK_Memset, MK_Memcpy, MK_Memmove };

  // Constructor to hold info about calls that only use a single memory
  // region, such as memset.
  MemfuncCallInfo(Instruction *I, MemfuncKind MK, MemfuncRegion &MR)
      : CallInfo(I, CallInfoKind::CIK_Memfunc), MK(MK) {

    assert(MK == MK_Memset &&
           "MemfuncCallInfo: Single range form expects memset");
    Regions.push_back(MR);
  }

  // Constructor to hold info about calls that have destination and source
  // regions, such as memcpy or memmove call.
  // The first region parameter is the destination, the second region parameter
  // is the source region.
  MemfuncCallInfo(Instruction *I, MemfuncKind MK, MemfuncRegion &MRDest,
                  MemfuncRegion &MRSrc)
      : CallInfo(I, CallInfoKind::CIK_Memfunc), MK(MK) {
    assert((MK == MK_Memcpy || MK_Memmove) &&
           "MemfuncCallInfo: Dual range form expects memcpy or memmove");

    Regions.push_back(MRDest);
    Regions.push_back(MRSrc);
  }

  static StringRef MemfuncKindName(MemfuncKind MK) {
    switch (MK) {
    case MK_Memset:
      return "memset";
    case MK_Memcpy:
      return "memcpy";
    case MK_Memmove:
      return "memmove";
    }

    llvm_unreachable("MemfuncKindName: Missing case in switch");
  }

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const CallInfo *CI) {
    return CI->getCallInfoKind() == CallInfo::CIK_Memfunc;
  }

  /// Returns the number of region objects for this call.
  unsigned int getNumRegions() const {
    switch (MK) {
    case MK_Memset:
      return 1;
    case MK_Memcpy:
    case MK_Memmove:
      return 2;
    }

    llvm_unreachable("MemfuncCall::getNumRegions missing case");
  }

  bool getIsCompleteAggregate(unsigned int RN) const {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    return Regions[RN].IsCompleteAggregate;
  }

  unsigned int getFirstField(unsigned int RN) const {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    assert(!getIsCompleteAggregate(RN) &&
           "Field tracking only value when not a complete aggregate");
    return Regions[RN].FirstField;
  }

  unsigned int getLastField(unsigned int RN) const {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    assert(!getIsCompleteAggregate(RN) &&
           "Field tracking only value when not a complete aggregate");
    return Regions[RN].LastField;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  MemfuncKind MK;
  SmallVector<MemfuncRegion, 2> Regions;
};

/// Determine whether the specified Function is an allocation function, and
/// if so what kind of allocation function it is and the size of the allocation.
AllocKind getAllocFnKind(Function *F, const TargetLibraryInfo &TLI);

/// Get the size and count arguments for the allocation call. AllocCountiVal is
/// used for calloc allocations.  For all other allocation kinds it will be set
/// to nullptr.
void getAllocSizeArgs(AllocKind Kind, CallInst *CI, Value *&AllocSizeVal,
                      Value *&AllocCountVal);

/// Determine whether or not the specified Function is the free library
/// function.
bool isFreeFn(Function *F, const TargetLibraryInfo &TLI);

/// This helper function checks if \p Val is a constant integer equal to
/// \p Size. Allows for \p Val to be nullptr, and will return false in
/// this case.
bool isValueEqualToSize(Value *Val, uint64_t Size);

/// This helper function checks \p Val to see if it is either (a) a constant
/// whose value is a multiple of \p Size, or (b) an integer multiplication
/// operator where either operand is a constant multiple of \p Size.
bool isValueMultipleOfSize(Value *Val, uint64_t Size);

/// Examine the specified types to determine if a bitcast from \p SrcTy to
/// \p DestTy could be used to access the first element of SrcTy. The
/// \p AccessedTy argument if non-null returns the type (possibly a nested
/// type) whose element zero is accessed, if any.
bool isElementZeroAccess(llvm::Type *SrcTy, llvm::Type *DestTy,
                         llvm::Type **AccessedTy = nullptr);

/// Check whether the specified type is the type of a known system object.
bool isSystemObjectType(llvm::StructType *Ty);

/// Get the maximum number of fields in a structure that are allowed before
/// we are unwilling to attempts dtrans optimizations.
unsigned getMaxFieldsInStruct();

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANS_H
