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

namespace llvm {

class TargetLibraryInfo;
class Function;
class Instruction;
class Type;
class StructType;
class CallInst;
class Value;
class Constant;

namespace dtrans {

//
// Enum to indicate the "single value" status of a field:
//   None: No write to field seen
//   Single: Only a single value for the field.  The value is obtained with
//     getSingleValue()
//   Multiple: Potentially or actually multiple values for the field
//
enum SingleValueKind { SVK_None, SVK_Single, SVK_Multiple };

class FieldInfo {
public:
  FieldInfo(llvm::Type *Ty)
      : LLVMType(Ty), Read(false), Written(false), AddressTaken(false),
        SVKind(SVK_None), SingleValue(nullptr) {}

  llvm::Type *getLLVMType() const { return LLVMType; }

  bool isRead() const { return Read; }
  bool isWritten() const { return Written; }
  bool isAddressTaken() const { return AddressTaken; }
  bool isNoValue() const { return SVKind == SVK_None; }
  bool isSingleValue() const { return SVKind == SVK_Single; }
  bool isMultipleValue() const { return SVKind == SVK_Multiple; }
  llvm::Constant *getSingleValue() {
    return SVKind == SVK_Single ? SingleValue : nullptr;
  }
  void setRead(bool b) { Read = b; }
  void setWritten(bool b) { Written = b; }
  void setAddressTaken() { AddressTaken = true; }
  void setSingleValue(llvm::Constant *C) {
    SVKind = SVK_Single;
    SingleValue = C;
  }
  void setMultipleValue() {
    SVKind = SVK_Multiple;
    SingleValue = nullptr;
  }
  //
  // Update the "single value" of the field, given that a constant value C
  // for the field has just been seen.
  //
  void processNewSingleValue(llvm::Constant *C);

private:
  llvm::Type *LLVMType;
  bool Read;
  bool Written;
  bool AddressTaken;
  SingleValueKind SVKind;
  llvm::Constant *SingleValue;
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

/// An object describing the DTrans-related characteristics of an LLVM type.
class TypeInfo {
public:
  /// Definitions to support type inquiry through isa, cast, and dyn_cast
  enum TypeInfoKind { NonAggregateInfo, PtrInfo, StructInfo, ArrayInfo };
  TypeInfoKind getTypeInfoKind() const { return TIK; }

protected:
  // This class should only be instantiated through its subclasses.
  TypeInfo(TypeInfoKind Kind, llvm::Type *Ty)
      : LLVMTy(Ty), SafetyInfo(0), TIK(Kind) {}

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

private:
  llvm::Type *LLVMTy;
  SafetyData SafetyInfo;

  // ID to support type inquiry through isa, cast, and dyn_cast
  TypeInfoKind TIK;
};

//
// Safety conditions for field single value analysis
//
const SafetyData SDFieldSingleValue =
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
enum AllocKind { AK_NotAlloc, AK_Malloc, AK_Calloc, AK_Realloc, AK_UserAlloc };

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
