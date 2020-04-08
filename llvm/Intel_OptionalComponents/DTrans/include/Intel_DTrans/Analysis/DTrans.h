//===--------------- DTrans.h - Class definition -*- C++ -*----------------===//
//
// Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
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

#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

class TargetLibraryInfo;
class CallBase;
class Type;
class StructType;
class PointerType;
class Value;
class Constant;
class raw_ostream;
class DTransAnalysisInfo;

namespace dtrans {

//
// Enum to indicate the "single value" status of a field:
//   Complete: All values of the field are constant and known.
//   Incomplete: Potentially or actually unknown values for the field.
//
enum SingleValueKind { SVK_Complete, SVK_Incomplete };

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
      : LLVMType(Ty), Read(false), Written(false), UnusedValue(true),
        ComplexUse(false), AddressTaken(false), MismatchedElementAccess(false),
        SVKind(SVK_Complete), SVIAKind(SVK_Incomplete), SAFKind(SAFK_Top),
        SingleAllocFunction(nullptr), RWState(RWK_Top), Frequency(0) {}

  llvm::Type *getLLVMType() const { return LLVMType; }

  bool isRead() const { return Read; }
  bool isWritten() const { return Written; }
  bool isValueUnused() const { return UnusedValue && isRead(); }
  bool hasComplexUse() const { return ComplexUse; }
  bool isAddressTaken() const { return AddressTaken; }
  bool isMismatchedElementAccess() const { return MismatchedElementAccess; }
  bool isNoValue() const {
    return SVKind == SVK_Complete && ConstantValues.empty();
  }
  bool isNoIAValue() const {
    return SVIAKind == SVK_Complete && ConstantIAValues.empty();
  }
  bool isTopAllocFunction() const { return SAFKind == SAFK_Top; }
  bool isSingleValue() const {
    return SVKind == SVK_Complete && ConstantValues.size() == 1;
  }
  bool isSingleIAValue() const {
    return SVIAKind == SVK_Complete && ConstantIAValues.size() == 1;
  }
  bool isSingleAllocFunction() const { return SAFKind == SAFK_Single; }
  bool isMultipleValue() const {
    return SVKind == SVK_Incomplete || ConstantValues.size() > 1;
  }
  bool isMultipleIAValue() const {
    return SVIAKind == SVK_Incomplete || ConstantIAValues.size() > 1;
  }
  bool isBottomAllocFunction() const { return SAFKind == SAFK_Bottom; }
  llvm::Constant *getSingleValue() {
    return isSingleValue() ? *ConstantValues.begin() : nullptr;
  }
  llvm::Constant *getSingleIAValue() {
    return isSingleIAValue() ? *ConstantIAValues.begin() : nullptr;
  }
  llvm::Function *getSingleAllocFunction() {
    return SAFKind == SAFK_Single ? SingleAllocFunction : nullptr;
  }
  void setRead(Instruction &I) { Read = true; addReader(I.getFunction());  }
  void setWritten(Instruction &I) {
    Written = true;
    addWriter(I.getFunction());
  }
  void setValueUnused(bool b) {
    UnusedValue = b;
  }
  void setComplexUse(bool b) { ComplexUse = b; }
  void setAddressTaken() { AddressTaken = true; }
  void setMismatchedElementAccess() { MismatchedElementAccess = true; }
  void setSingleAllocFunction(llvm::Function *F) {
    assert((SAFKind == SAFK_Top) && "Expecting lattice at top");
    SAFKind = SAFK_Single;
    SingleAllocFunction = F;
  }
  void setMultipleValue() { SVKind = SVK_Incomplete; }
  void setIAMultipleValue() { SVIAKind = SVK_Incomplete; }
  void setBottomAllocFunction() {
    SAFKind = SAFK_Bottom;
    SingleAllocFunction = nullptr;
  }
  void setFrequency(uint64_t Freq) { Frequency = Freq; }
  uint64_t getFrequency() const { return Frequency; }

  // Returns a set of possible constant values.
  llvm::SetVector<llvm::Constant *> &values()
      { return ConstantValues; }

  // Returns a set of possible indirect array constant values.
  llvm::SetVector<llvm::Constant *> &iavalues()
      { return ConstantIAValues; }

  // Returns true if the set of possible values is complete.
  bool isValueSetComplete() const { return SVKind == SVK_Complete; }

  // Returns true if the set of possible values is complete.
  bool isIAValueSetComplete() const { return SVIAKind == SVK_Complete; }

  //
  // Update the "single value" of the field, given that a constant value C
  // for the field has just been seen. Return true if the value is updated.
  //
  bool processNewSingleValue(llvm::Constant *C);
  //
  // Update the "indirect array single value" of the field, given that a
  // constant value C for the field has just been seen. Return true if the
  // value is updated.
  //
  bool processNewSingleIAValue(llvm::Constant *C);
  //
  // Update the single alloc function for the field, given that we have just
  // seen an assignment to it from the return value of a call to F. Return
  // true if the value is updated.
  //
  bool processNewSingleAllocFunction(llvm::Function *F);

  // For tracking the set of functions that read/write the field.
  typedef llvm::SmallPtrSet<Function*, 2> FunctionSet;
  typedef llvm::SmallPtrSet<Function*, 2> &FunctionSetRef;

  void addReader(Function *F) { Readers.insert(F); }
  void addWriter(Function *F) { Writers.insert(F); }
  FunctionSetRef readers() { return Readers; }
  FunctionSetRef writers() { return Writers; }

  // Lattice regarding the information contained within the Readers/Writers
  // sets.
  // RWK_Top - Initial state.
  // RWK_Computed - After safety checks are performed, elements that are
  //   resolved as being safe to rely on the Readers/Writers fields will be
  //   marked as RWK_Computed.
  // RWK_Bottom - Elements that are determined to not be safe.
  //   This may also be used in the future to limit the size to the
  //   Readers/Writers sets by going conservative if the sets become to large.
  enum RW_Kind { RWK_Top, RWK_Computed, RWK_Bottom };
  void setRWComputed() {
    assert(!isRWBottom() && "State is already bottom.");
    RWState = RWK_Computed;
  }
  void setRWBottom() { RWState = RWK_Bottom; }

  bool isRWTop() const { return RWState == RWK_Top; }
  bool isRWComputed() const { return RWState == RWK_Computed; }
  bool isRWBottom() const { return RWState == RWK_Bottom; }

private:
  llvm::Type *LLVMType;
  bool Read;
  bool Written;
  bool UnusedValue;
  bool ComplexUse;
  bool AddressTaken;

  // Tracks whether this field triggered the mismatched element access safety
  // check on the structure.
  bool MismatchedElementAccess;

  SingleValueKind SVKind;
  llvm::SetVector<llvm::Constant *> ConstantValues;
  SingleValueKind SVIAKind;
  llvm::SetVector<llvm::Constant *> ConstantIAValues;
  SingleAllocFunctionKind SAFKind;
  llvm::Function *SingleAllocFunction;
  // For computing ModRef information for the field, these sets contain the
  // functions that are known to directly read/write the field.
  FunctionSet Readers;
  FunctionSet Writers;

  // Status for Readers/Writers information.
  RW_Kind RWState;

  // It represents relative field access frequency and is used in
  // heuristics to enable transformations. Load/Store is considered as
  // field access. AddressTaken of struct or field is not considered as
  // field access currently.
  // TODO: Frequency is not computed correctly for aggregate fields. Need
  // to compute more accurate Frequency for aggregate fields.
  uint64_t Frequency;
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

/// A function was called with an i8* argument where the aliases of the
/// value passed to the function do not match the uses of the argument
/// within the function..
const SafetyData MismatchedArgUse = 0x0000000000001000000;

/// A global variable was found which is an array of the type.
const SafetyData GlobalArray = 0x0000000000002000000;

/// An element in the structure looks like a vtable.
const SafetyData HasVTable = 0x0000000000004000000;

/// An element in the structure points to a function.
const SafetyData HasFnPtr = 0x0000000000008000000;

/// A type has C++ processing:
///   allocation/deallocation with new/delete;
///   invoke instruction returns or takes structure/
///     pointer to structure.
const SafetyData HasCppHandling = 0x0000000000010000000;

/// The structure contains zero-sized array as the last field.
const SafetyData HasZeroSizedArray = 0x0000000000020000000;

/// For use with BadCastingAnalyzer

// A potential bad casting issue that will be either eliminated,
// converted to bad casting conditional, or converted to bad casting
// at the end of analysis by the bad casting analyzer.
const SafetyData BadCastingPending = 0x0000000000040000000;

// Indicates that bad casting will occur only if specific conditions
// are not fulfilled.  These conditions are noted by the bad casting
// analyzer, and involve certain functions' arguments being nullptr on
// entry to those functions.
const SafetyData BadCastingConditional = 0x0000000000080000000;

// A potential unsafe pointer store issue that will be either eliminated,
// converted to unsafe pointer store conditional, or converted to unsafe
// pointer store at the end of analysis by the bad casting analyzer.
const SafetyData UnsafePointerStorePending = 0x0000000000100000000;

// Indicates that an unsafe pointer store  will occur only if specific
// conditions are not fulfilled.  These conditions are noted by the bad
// casting analyzer, and involve certain functions' arguments being nullptr
// on entry to those functions.
const SafetyData UnsafePointerStoreConditional = 0x0000000000200000000;

/// This is a catch-all flag that will be used to mark any usage pattern
/// that we don't specifically recognize. The use might actually be safe
/// or unsafe, but we will conservatively assume it is unsafe.
const SafetyData UnhandledUse = 0x8000000000000000;

// TODO: Create a safety mask for the conditions that are common to all
//       DTrans optimizations.
//
// Safety conditions for field reordering and deletion.
//
const SafetyData SDDeleteField =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | FieldAddressTaken | BadMemFuncSize |
        BadMemFuncManipulation | AmbiguousPointerTarget | UnsafePtrMerge |
        AddressTaken | NoFieldsInStruct | SystemObject | MismatchedArgUse |
        HasVTable | HasFnPtr | HasZeroSizedArray | HasFnPtr |
        BadCastingConditional | UnsafePointerStoreConditional;

const SafetyData SDReorderFields =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | FieldAddressTaken | GlobalInstance |
        HasInitializerList | UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
        BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
        NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
        MismatchedArgUse | LocalInstance | HasCppHandling |
        BadCastingConditional | UnsafePointerStoreConditional | UnhandledUse;

const SafetyData SDReorderFieldsDependent =
    BadPtrManipulation | GlobalInstance | HasInitializerList |
        MemFuncPartialWrite | NoFieldsInStruct | LocalInstance |
        BadCastingConditional | UnsafePointerStoreConditional | UnhandledUse |
        WholeStructureReference | VolatileData | BadMemFuncSize |
        BadMemFuncManipulation | AmbiguousPointerTarget;

//
// Safety conditions for field single value analysis
//
const SafetyData SDFieldSingleValueNoFieldAddressTaken =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
        MismatchedElementAccess | UnsafePointerStore | AmbiguousPointerTarget |
        UnsafePtrMerge | AddressTaken | MismatchedArgUse | UnhandledUse;

const SafetyData SDFieldSingleValue =
    SDFieldSingleValueNoFieldAddressTaken | FieldAddressTaken;

const SafetyData SDSingleAllocFunctionNoFieldAddressTaken =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
        MismatchedElementAccess | UnsafePointerStore | BadMemFuncSize |
        BadMemFuncManipulation | AmbiguousPointerTarget | UnsafePtrMerge |
        AddressTaken | MismatchedArgUse | UnhandledUse;

const SafetyData SDSingleAllocFunction =
    SDSingleAllocFunctionNoFieldAddressTaken | FieldAddressTaken;

const SafetyData SDElimROFieldAccess =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
        MismatchedElementAccess | UnsafePointerStore | FieldAddressTaken |
        BadMemFuncSize | BadMemFuncManipulation | AmbiguousPointerTarget |
        HasInitializerList | UnsafePtrMerge | AddressTaken | MismatchedArgUse |
        BadCastingConditional | UnsafePointerStoreConditional | UnhandledUse;

//
// Safety conditions for a structure to be considered for the AOS-to-SOA
// transformation.
const SafetyData SDAOSToSOA =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | FieldAddressTaken | GlobalInstance |
        HasInitializerList | UnsafePtrMerge | BadMemFuncSize |
        BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
        NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
        LocalInstance | MismatchedArgUse | GlobalArray | HasVTable | HasFnPtr |
        HasCppHandling | HasZeroSizedArray |
        BadCastingConditional | UnsafePointerStoreConditional;

//
// Safety conditions for a structure type that contains a pointer to a
// structure that is being considered for the AOS-to-SOA transformation.
//
const SafetyData SDAOSToSOADependent =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
        MismatchedElementAccess | WholeStructureReference | UnsafePointerStore |
        UnsafePtrMerge | AmbiguousPointerTarget | AddressTaken | NoFieldsInStruct |
        NestedStruct | ContainsNestedStruct | SystemObject | MismatchedArgUse |
        GlobalArray | HasVTable | HasCppHandling |
        BadCastingConditional | UnsafePointerStoreConditional;

//
// Safety conditions for a structure type that contains a pointer to a
// structure that is being considered for the AOS-to-SOA transformation
// when the peeling index is being converted to use 32-bits, causing
// the size of the dependent structure to change.
//
const SafetyData SDAOSToSOADependentIndex32 =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | UnsafePtrMerge | BadMemFuncSize |
        BadMemFuncManipulation | MemFuncPartialWrite | AmbiguousPointerTarget |
        AddressTaken | NoFieldsInStruct | NestedStruct | ContainsNestedStruct |
        SystemObject | MismatchedArgUse | GlobalArray | HasVTable | HasCppHandling |
        HasZeroSizedArray | BadCastingConditional | UnsafePointerStoreConditional;

const SafetyData SDDynClone =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | FieldAddressTaken | GlobalInstance |
        HasInitializerList | UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
        BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
        NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
        LocalInstance | MismatchedArgUse | GlobalArray | HasVTable | HasFnPtr |
        HasZeroSizedArray | BadCastingConditional | UnsafePointerStoreConditional |
        UnhandledUse;

const SafetyData SDSOAToAOS =
    BadCasting | BadPtrManipulation |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | FieldAddressTaken | GlobalInstance |
        HasInitializerList | UnsafePtrMerge | BadMemFuncSize |
        BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
        NoFieldsInStruct | SystemObject | LocalInstance | MismatchedArgUse |
        GlobalArray | HasFnPtr | HasZeroSizedArray |
        BadCastingConditional | UnsafePointerStoreConditional | UnhandledUse;

const SafetyData SDMemInitTrimDown =
    BadCasting | BadPtrManipulation |
        VolatileData | MismatchedElementAccess | WholeStructureReference |
        UnsafePointerStore | FieldAddressTaken | GlobalInstance |
        HasInitializerList | UnsafePtrMerge | BadMemFuncSize |
        BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
        NoFieldsInStruct | SystemObject | LocalInstance | MismatchedArgUse |
        GlobalArray | HasFnPtr | HasZeroSizedArray |
        BadCastingConditional | UnsafePointerStoreConditional | UnhandledUse;

//
// TODO: Update the list each time we add a new safety conditions check for a
// new transformation pass.
//
typedef uint32_t Transform;

const Transform DT_First = 0x0001;
const Transform DT_FieldSingleValue = 0x0001;
const Transform DT_FieldSingleAllocFunction = 0x0002;
const Transform DT_ReorderFields = 0x0004;
const Transform DT_ReorderFieldsDependent = 0x0008;
const Transform DT_DeleteField = 0x0010;
const Transform DT_AOSToSOA = 0x0020;
const Transform DT_AOSToSOADependent = 0x0040;
const Transform DT_AOSToSOADependentIndex32 = 0x0080;
const Transform DT_ElimROFieldAccess = 0x0100;
const Transform DT_DynClone = 0x0200;
const Transform DT_SOAToAOS = 0x0400;
const Transform DT_MemInitTrimDown = 0x0800;
const Transform DT_Last = 0x1000;
const Transform DT_Legal = 0x0fff;

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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printSafetyData();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  CRuleTypeKind getCRuleTypeKind() { return CRTypeKind; }
  void setCRuleTypeKind(CRuleTypeKind K) { CRTypeKind = K; }

  // Returns true if the type is a zero-sized array or it is a structure with a
  // zero-sized array.
  bool hasZeroSizedArrayAsLastField() {
    return (SafetyInfo & dtrans::HasZeroSizedArray);
  }

private:
  llvm::Type *LLVMTy;
  SafetyData SafetyInfo;

  // ID to support type inquiry through isa, cast, and dyn_cast
  TypeInfoKind TIK;
  // Indicates whether the Type has a C language rule compatible Type
  CRuleTypeKind CRTypeKind;
};

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
  StructInfo(llvm::Type *Ty, ArrayRef<llvm::Type *> FieldTypes, bool IgnoreFlag)
      : TypeInfo(TypeInfo::StructInfo, Ty), IsIgnoredFor(IgnoreFlag),
        SubGraph() {
    for (llvm::Type *FieldTy : FieldTypes)
      Fields.push_back(FieldInfo(FieldTy));
  }

  size_t getNumFields() const { return Fields.size(); }
  SmallVectorImpl <FieldInfo> &getFields() { return Fields; }
  FieldInfo &getField(size_t N) { return Fields[N]; }

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::StructInfo;
  }
  uint64_t getTotalFrequency() const { return TotalFrequency; }
  void setTotalFrequency(uint64_t TFreq) { TotalFrequency = TFreq; }

  /// Sets IsIgnoredFor field to true if the type was indeed ignored during FSV
  /// and/or FSAF safety checking.
  void setIgnoredFor(dtrans::Transform Flag) { IsIgnoredFor |= Flag; }
  /// Returns FSV and/or FSAF if the type was ignored in those optimizations.
  dtrans::Transform getIgnoredFor() { return IsIgnoredFor; }

  // To represent call graph in C++ one stores outermost type,
  // in whose methods there was reference to this structure.
  // Lattice: {bottom = <nullptr, false>, <Type*, false>, top = <nullptr, true>}
  // Type should be some struct from which given type is reachable.
  // insertFunction is a 'join' operation of this lattice.
  class CallSubGraph {
    PointerIntPair<StructType *, 1, bool> State;
    void setTop() {
      State.setPointer(nullptr);
      State.setInt(true);
    }
  public:
    // State is zero-initialized to 'bottom'.
    CallSubGraph() = default;
    bool isBottom() const {
      return !State.getPointer() && !State.getInt();
    }
    bool isTop() const {
      return State.getInt();
    }
    StructType *getEnclosingType() const {
      assert(!isBottom() && !isTop() && "Invalid access to CallSubGraph");
      return State.getPointer();
    }
    // If occurrence is not inside specific Function,
    // then mark it as 'top'.
    // It is a case of GlobalVariable.
    void insertFunction(Function *F, StructType *ThisTy);
  };

  void insertCallGraphNode(Function *F) {
    SubGraph.insertFunction(F, cast<StructType>(getLLVMType()));
  }
  const CallSubGraph &getCallSubGraph() const { return SubGraph; }

private:
  SmallVector<FieldInfo, 16> Fields;
  // Total Frequency of all fields in struct.
  uint64_t TotalFrequency;
  dtrans::Transform IsIgnoredFor;
  CallSubGraph SubGraph;
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


/// Get a printable string for the CRuleTypeKind
StringRef CRuleTypeKindName(CRuleTypeKind Kind);

// This structure is used to describe the affected portion of an aggregate type
// passed as an argument of the memfunc call. This will be used to communicate
// information collected during the analysis to the transforms about how
// a memfunc call is impacting a structure.
struct MemfuncRegion {
  MemfuncRegion()
      : IsCompleteAggregate(true), PrePadBytes(0), FirstField(0), LastField(0),
        PostPadBytes(0) {}

  // If this is 'false', the FirstField and LastField members must be set
  // to indicate an inclusive set of fields within the structure that are
  // affected. If this is 'true', the FieldField and LastField member values
  // are undefined.
  bool IsCompleteAggregate;

  // If the region is a description of a partial structure modification, these
  // members specify the first and last fields touched, and number of padding
  // bytes before/after the first/last field.
  unsigned int PrePadBytes;
  unsigned int FirstField;
  unsigned int LastField;
  unsigned int PostPadBytes;
};

// This class is used to hold information related to the object type(s) that are
// used by a CallInfo object. The information stored here has been extracted
// from the LocalPointerInfo to contain a list of aggregate types being used by
// one of the tracked call instructions. This is kept outside of the CallInfo
// class itself to allow for cases where type information needs to be tracked
// for more than a single function argument.
class CallInfoElementTypes {
public:
  typedef SmallVector<llvm::Type *, 2> TypeAliasSet;
  typedef SmallVectorImpl<llvm::Type *> &TypeAliasSetRef;

  CallInfoElementTypes() : AliasesToAggregateType(false), Analyzed(false) {}

  // Returns 'true' if the call was known to be a pointer (at some level of
  // indirection) to an aggregate type.
  bool getAliasesToAggregateType() const {
    return AliasesToAggregateType;
  }

  void setAliasesToAggregateType(bool Val) {
    AliasesToAggregateType = Val;
  }

  void setAnalyzed(bool Val) { Analyzed = Val; }

  bool getAnalyzed() const { return Analyzed; }

  void addElemType(llvm::Type *Ty) {
    ElemTypes.push_back(Ty);
  }

  TypeAliasSetRef getElemTypes() { return ElemTypes; }

  size_t getNumTypes() { return ElemTypes.size(); }

  llvm::Type *getElemType(size_t Idx) const {
    assert(Idx < ElemTypes.size() && "Index out of range");
    return ElemTypes[Idx];
  }

  // Change the type at index \p Idx to type \p Ty. This
  // function should only be used for updating a type based
  // on the type remapping done when processing a function.
  void setElemType(size_t Idx, llvm::Type *Ty) {
    assert(Idx < ElemTypes.size() && "Index out of range");
    ElemTypes[Idx] = Ty;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // When true, indicates that the base type for one or more of the pointer
  // types operated upon by the call was an aggregate type.
  bool AliasesToAggregateType;

  // When true, indicates the LocalPointerAnalysis was performed to collect type
  // information for the pointer.
  bool Analyzed;

  // List of element types, resolved by the local pointer analysis, that the
  // call instruction is operating upon. e.g. an allocation of %struct.foo
  // objects, or a memset of i64 objects or %struct.foo* pointers.
  TypeAliasSet ElemTypes;
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

  bool getAliasesToAggregateType() const {
    return ElementTypes.getAliasesToAggregateType();
  }

  void setAliasesToAggregateType(bool Val) {
    ElementTypes.setAliasesToAggregateType(Val);
  }
  void setAnalyzed(bool Val) { ElementTypes.setAnalyzed(Val); }

  bool getAnalyzed() const { return ElementTypes.getAnalyzed(); }

  void addElemType(llvm::Type *Ty) {
    ElementTypes.addElemType(Ty);
  }

  CallInfoElementTypes &getElementTypesRef() { return ElementTypes; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

protected:
  CallInfo(Instruction *I, CallInfoKind Kind) : I(I), CIK(Kind) {}

  // Instruction the info corresponds to.
  Instruction *I;

  // The type list from the local pointer analysis.
  CallInfoElementTypes ElementTypes;

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
    assert(((MK == MK_Memcpy) || (MK == MK_Memmove)) &&
        "MemfuncCallInfo: Dual range form expects memcpy or memmove");

    Regions.push_back(MRDest);
    Regions.push_back(MRSrc);
  }

  MemfuncKind getMemfuncCallInfoKind() const { return MK; }

  static StringRef MemfuncKindName(MemfuncKind MK) {
    switch (MK) {
    case MK_Memset:return "memset";
    case MK_Memcpy:return "memcpy";
    case MK_Memmove:return "memmove";
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
    case MK_Memset:return 1;
    case MK_Memcpy:
    case MK_Memmove:return 2;
    }

    llvm_unreachable("MemfuncCall::getNumRegions missing case");
  }

  bool getIsCompleteAggregate(unsigned int RN) const {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    return Regions[RN].IsCompleteAggregate;
  }

  unsigned int getPrePadBytes(unsigned int RN) {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    assert(!getIsCompleteAggregate(RN) &&
      "Field tracking only valid when not a complete aggregate");
    return Regions[RN].PrePadBytes;
  }

  unsigned int getFirstField(unsigned int RN) const {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    assert(!getIsCompleteAggregate(RN) &&
        "Field tracking only valid when not a complete aggregate");
    return Regions[RN].FirstField;
  }

  unsigned int getLastField(unsigned int RN) const {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    assert(!getIsCompleteAggregate(RN) &&
        "Field tracking only valid when not a complete aggregate");
    return Regions[RN].LastField;
  }

  unsigned int getPostPadBytes(unsigned int RN) {
    assert(RN <= getNumRegions() && "RegionNum for memfunc call out of range");
    assert(!getIsCompleteAggregate(RN) &&
      "Field tracking only valid when not a complete aggregate");
    return Regions[RN].PostPadBytes;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
  void print(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  MemfuncKind MK;
  SmallVector<MemfuncRegion, 2> Regions;
};

// Get the printable name for a SafetyData bit. The \p SafetyInfo value input to
// this function may only have a single non-zero bit set.
const char* getSafetyDataName(const SafetyData &SafetyInfo);


/// Checks if a \p Val is a constant integer and sets it to \p ConstValue.
bool isValueConstant(const Value *Val, uint64_t *ConstValue = nullptr);

/// This helper function checks if \p Val is a constant integer equal to
/// \p Size. Allows for \p Val to be nullptr, and will return false in
/// this case.
bool isValueEqualToSize(const Value *Val, uint64_t Size);

/// This helper function checks \p Val to see if it is either (a) a constant
/// whose value is a multiple of \p Size, (b) an integer multiplication
/// operator where either operand is a constant multiple of \p Size, or
/// (c) an integer value left-shifted by a value that results in a multiple
/// of the \p Size.
bool isValueMultipleOfSize(const Value *Val, uint64_t Size);

/// Examine the specified types to determine if a bitcast from \p SrcTy to
/// \p DestTy could be used to access the first element of SrcTy. The
/// \p AccessedTy argument if non-null returns the type (possibly a nested
/// type) whose element zero is accessed, if any.
bool isElementZeroAccess(llvm::Type *SrcTy, llvm::Type *DestTy,
                         llvm::Type **AccessedTy = nullptr);

/// Examine the specified type to determine if it is a composite type whose
/// first element (at any level of casting) has i8* type. The
/// \p AccessedTy argument if non-null returns the type (possibly a nested
/// type) whose element zero is i8*, if any.
bool isElementZeroI8Ptr(llvm::Type *Ty, llvm::Type **AccessedTy = nullptr);

/// Examine the specified types to determine if a bitcast from \p SrcTy to
/// \p DestTy could be used to convert a pointer-to-pointer to a source
/// type to a pointer-to-pointer to element zero of that type. This is
/// equivalent to isElementZeroAccess with an additional level of indirection.
bool isPtrToPtrToElementZeroAccess(llvm::Type *SrcTy, llvm::Type *DestTy);

/// Examine the specified types to determine if a bitcast from \p SrcTy to
/// \p DestTy could be used to access the vtable of a class pointed to by
/// SrcTy.
bool isVTableAccess(llvm::Type *SrcTy, llvm::Type *DestTy);

/// Remove pointer, vector, and array types to uncover the base type which
/// the contain.
Type *unwrapType(Type *Ty);

/// Check whether the specified type is the type of a known system object.
bool isSystemObjectType(llvm::StructType *Ty);

/// Get the maximum number of fields in a structure that are allowed before
/// we are unwilling to attempts dtrans optimizations.
unsigned getMaxFieldsInStruct();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Get the transformation printable name.
StringRef getStringForTransform(dtrans::Transform Trans);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Get the safety conditions for the transformation.
dtrans::SafetyData getConditionsForTransform(dtrans::Transform Trans,
                                             bool DTransOutOfBoundsOK);

StringRef getStructName(llvm::Type *Ty);

/// Check if the last field in the struct type \p Ty is zero-sized array or the
/// type is zero-size array itself.
bool hasZeroSizedArrayAsLastField(llvm::Type *Ty);

/// Check if the called function has only one basic block that ends with
/// 'unreachable' instruction.
bool isDummyFuncWithUnreachable(const CallBase *Call,
                                const TargetLibraryInfo &TLI);
/// Check if the called function has two arguments ('this' pointer and an
/// integer size) and is dummy.
bool isDummyFuncWithThisAndIntArgs(const CallBase *Call,
                                   const TargetLibraryInfo &TLI);
/// Check if the called function has two arguments ('this' pointer and a
/// pointer) and is dummy.
bool isDummyFuncWithThisAndPtrArgs(const CallBase *Call,
                                   const TargetLibraryInfo &TLI);

// Returns true if Ty is either StructType or SequentialType.
bool dtransIsCompositeType(llvm::Type *Ty);

// Return true if "Idx" is valid index for Ty.
bool dtransCompositeIndexValid(llvm::Type *Ty, unsigned Idx);

// Returns type at "Idx" in "Ty".
llvm::Type *dtransCompositeGetTypeAtIndex(llvm::Type *Ty, unsigned Idx);

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANS_H
