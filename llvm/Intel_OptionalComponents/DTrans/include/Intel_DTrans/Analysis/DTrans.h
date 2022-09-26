//===--------------- DTrans.h - Class definition -*- C++ -*----------------===//
//
// Copyright (C) 2017-2022 Intel Corporation. All rights reserved.
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

#if !INTEL_FEATURE_SW_DTRANS
#error DTrans.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANS_H
#define INTEL_DTRANS_ANALYSIS_DTRANS_H

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/MemoryBuiltinsExtras.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Debug.h"
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

// This class is used to represent a data type as either a llvm::Type or a
// dtrans::DtransType to allow the TypeInfo class objects to work with either
// the LocalPointerAnalyzer (by using llvm::Type objects) or the
// DTransSafetyAnalyzer (by using dtrans::DtransType objects). This is to enable
// the sharing of the existing TypeInfo classes while developing the DTrans
// support for opaque pointers. When the compiler is transitioned to fully using
// opaque pointers, and the LocalPointerAnalyzer is removed, this class can be
// removed.
class AbstractType {
public:
  AbstractType(llvm::Type *Ty) : Ty(Ty) {}
  AbstractType(dtransOP::DTransType *Ty) : Ty(Ty) {}

  // Get the corresponding type in the llvm::Type class hierarchy.
  Type *getLLVMType() const {
    return Ty.is<llvm::Type *>()
               ? Ty.get<llvm::Type *>()
               : Ty.get<dtransOP::DTransType *>()->getLLVMType();
  }

  bool isDTransType() const { return Ty.is<dtransOP::DTransType *>(); }

  dtransOP::DTransType *getDTransType() const {
    assert(isDTransType() && "Only valid when using DTransTypes");
    return Ty.get<dtransOP::DTransType *>();
  }

private:
  PointerUnion<llvm::Type *, dtransOP::DTransType *> Ty;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static inline raw_ostream &operator<<(raw_ostream &OS, const AbstractType &AT) {
  if (AT.isDTransType())
    OS << *AT.getDTransType();
  else
    OS << *AT.getLLVMType();
  return OS;
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//Type used for DTrans transformation bitmask
typedef uint32_t Transform;

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
  FieldInfo(AbstractType Ty)
      : Ty(Ty), Read(false), Written(false), UnusedValue(true),
        ComplexUse(false), AddressTaken(false), MismatchedElementAccess(false),
        NonGEPAccess(false), SVKind(SVK_Complete), SVIAKind(SVK_Incomplete),
        SAFKind(SAFK_Top), SingleAllocFunction(nullptr), RWState(RWK_Top),
        Frequency(0) {

    // If the current field is an array of integers then we can collect
    // constant entries for it.
    llvm::Type *ArrField = Ty.getLLVMType();

    // There are cases where the array is hidden behind another structure (e.g.
    // arrays created with boost libraries). The field type will be a
    // structure, with one field that is an array.
    if (auto *StrTy = dyn_cast<llvm::StructType>(Ty.getLLVMType()))
      if (StrTy && StrTy->getNumElements() == 1)
        ArrField = StrTy->getElementType(0);

    if (auto *ArrTy = dyn_cast<llvm::ArrayType>(ArrField))
      canAddConstantEntriesForArray = ArrTy->getElementType()->isIntegerTy();
    else
      canAddConstantEntriesForArray = false;
  }

  // Disallow copy
  FieldInfo(const FieldInfo&) = delete;
  FieldInfo &operator=(const FieldInfo &) = delete;

  // Move operators use default implementation
  FieldInfo(FieldInfo &&) = default;
  FieldInfo &operator=(FieldInfo &&) = default;

  llvm::Type *getLLVMType() const { return Ty.getLLVMType(); }
  dtransOP::DTransType *getDTransType() const { return Ty.getDTransType(); }
  bool isDTransType() const { return Ty.isDTransType(); }

  bool isRead() const { return Read; }
  bool isWritten() const { return Written; }
  bool isValueUnused() const { return UnusedValue && isRead(); }
  bool hasComplexUse() const { return ComplexUse; }
  bool isAddressTaken() const { return AddressTaken; }
  bool isMismatchedElementAccess() const { return MismatchedElementAccess; }
  bool hasNonGEPAccess() const { return NonGEPAccess; }
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
  llvm::Constant *getSingleValue() const {
    return isSingleValue() ? *ConstantValues.begin() : nullptr;
  }
  llvm::Constant *getSingleIAValue() const {
    return isSingleIAValue() ? *ConstantIAValues.begin() : nullptr;
  }
  llvm::Function *getSingleAllocFunction() const {
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
  void setNonGEPAccess() { NonGEPAccess = true; }
  void setSingleAllocFunction(llvm::Function *F) {
    assert((SAFKind == SAFK_Top) && "Expecting lattice at top");
    SAFKind = SAFK_Single;
    SingleAllocFunction = F;
  }
  void setIncompleteValueSet() { SVKind = SVK_Incomplete; }
  void setIAIncompleteValueSet() { SVIAKind = SVK_Incomplete; }
  void setBottomAllocFunction() {
    SAFKind = SAFK_Bottom;
    SingleAllocFunction = nullptr;
  }
  void setFrequency(uint64_t Freq) { Frequency = Freq; }
  uint64_t getFrequency() const { return Frequency; }

  // Returns a set of possible constant values.
  const llvm::SetVector<llvm::Constant *> &values() const
      { return ConstantValues; }

  // Returns a set of possible indirect array constant values.
  const llvm::SetVector<llvm::Constant *> &iavalues() const
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
  typedef llvm::SmallPtrSet<Function*, 2> const &FunctionSetConstRef;

  void addReader(Function *F) { Readers.insert(F); }
  void addWriter(Function *F) { Writers.insert(F); }
  FunctionSetConstRef readers() const { return Readers; }
  FunctionSetConstRef writers() const { return Writers; }

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

  // This enum is used for storing which information we have related to the
  // padded fields:
  // PFK_Clean : We know that there is no safety violation that could
  //             compromise the field and it is never accessed
  // PFK_Dirty : A safety violation happens, the field is accessed and/or
  //             we don't have enough information to mark it clean
  // PFK_NoPadding : The field is not used for padding.
  enum PF_Kind { PFK_Clean, PFK_Dirty, PFK_NoPadding };

  // Set the padded field as clean.
  // NOTE: Once the field is dirty it can't be clean.
  void setPaddedField() {
    if (PaddedField != PFK_Dirty)
      PaddedField = PFK_Clean;
  }
  // Set if the padded field is dirty
  void invalidatePaddedField() { PaddedField = PFK_Dirty; }
  void clearPaddedField() { PaddedField = PFK_NoPadding; }
  bool isPaddedField() const { return PaddedField != PFK_NoPadding; }
  bool isCleanPaddedField() const { return PaddedField == PFK_Clean; }

  // Return true if the current field is an array with constant entries
  bool isArrayWithConstantEntries() const {
    if (ArrayConstEntries.empty())
      return false;

    // If there is a nullptr in ArrayConstEntries, then we will treat
    // the information as invalid
    for (auto &Entry : ArrayConstEntries) {
      (void)Entry;
      assert((Entry.first && isa<ConstantInt>(Entry.first) &&
              Entry.second && isa<ConstantInt>(Entry.second)) &&
             "Non-constant integer information found in a field "
             "reserved for constant data");
    }

    return true;
  }

  // Return the information if the current field is an array with
  // constant entries
  const SetVector< std::pair<Constant*, Constant*> >
      &getArrayWithConstantEntries() const { return ArrayConstEntries; }

  // Insert a new entry in ArrayConstEntries assuming that the current field
  // is an array with constant entries.
  void addConstantEntryIntoTheArray(Constant *Index, Constant* ConstVal);

  // Helper function that generates the information related to arrays with
  // constant entries for the DTrans immutable analysis.
  //
  // NOTE: This function is intended for typed pointers. It will be removed
  // once we fully move from typed pointer to opaque pointers.
  void generateArraysWithConstInmmutableData();

  // Insert a new entry in ArrayWithConstEntriesMap.
  //
  // NOTE: This function replaces addConstantEntryIntoTheArray in the opaque
  // pointers case. The function addConstantEntryIntoTheArray will be removed
  // once we fully move from typed pointers to opaque pointers.
  void addNewArrayConstantEntry(Constant *Index, Constant* ConstVal);

  // Return the information if the current field is an array with
  // constant entries
  //
  // NOTE: This is used for the opaque pointers case. The function
  // getArrayWithConstantEntries will be removed once we move from typed
  // pointers to opaque pointers.
  const DenseMap<Constant*, Constant*>
      &getArrayConstantEntries() const { return ArrayWithConstEntriesMap; }

  // Return true if the current field is an array with constant entries
  //
  // NOTE: This function will replace isArrayWithConstantEntries in the
  // opaque pointers case. The function isArrayWithConstantEntries will
  // be removed once we fully move from typed pointers to opaque pointers.
  bool isFieldAnArrayWithConstEntries();

  // Disable all the data related to arrays with constant entries.
  void disableArraysWithConstantEntries() {
    canAddConstantEntriesForArray = false;
    ArrayWithConstEntriesMap.clear();
  }

  // Return true if the entries of ArrayWithConstEntriesMap can be updated.
  // Else return false.
  bool canUpdateArrayWithConstantEntries() {
    return canAddConstantEntriesForArray;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { print(dbgs()); }

  // An optional Transform bitmask, IgnoredInTransform, can be passed in to
  // report extra information about field information that may be ignored for
  // some transformation types.
  void print(raw_ostream &OS, dtrans::Transform IgnoredInTransform = 0) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  AbstractType Ty;
  bool Read;
  bool Written;
  bool UnusedValue;
  bool ComplexUse;
  bool AddressTaken;

  // Tracks whether this field triggered the mismatched element access safety
  // check on the structure.
  bool MismatchedElementAccess;

  // Indicates that a Load/Store instruction accessing the field was not based
  // on a GEP instruction to obtain the address. This can occur when a pointer
  // to the structure is used directly to access the zeroth element of the
  // structure.
  // For example:
  //   %struct.list_elem = type { %struct.arc*, %struct.list_elem* }
  //   %1510 = bitcast i8* %1506 to %struct.arc**
  //   store %struct.arc* %1505, %struct.arc** %1510
  //
  // If %1506 was resolved as being %struct.list_elem*, the store is writing
  // a field within the structure.
  //
  // The transformations that delete or reorder fields within a structure work
  // by rewriting the GEP instructions that obtain the address of the fields,
  // and currently do not support modifying an access to the field that does
  // not involve a GEP.
  bool NonGEPAccess;

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

  // Identify if the field is used for ABI padding. For example:
  // %class.A.base = type <{ %"class.boost::array", [2 x i8],
  //                         %"class.std::vector", i32 }>
  // %class.A = type <{ %"class.boost::array", [2 x i8],
  //                         %"class.std::vector", i32, [4 x i8] }>
  //
  // The structure type %class.A.base is the same as %class.A, except for
  // the last entry of structure %class.A. This entry at the end ([4 x i8])
  // is used for the application binary interface (ABI) and it will
  // be identified as the PaddedField.
  PF_Kind PaddedField = PFK_NoPadding;

  // Set vector that stores the index and the value that are constant if
  // the current field is an array with constant entries. The first
  // entry in the pair is the index in the array, the second entry is
  // the constant value.
  SetVector< std::pair<Constant*, Constant*> > ArrayConstEntries;

  // DenseMap that stores the index and the value that are constant if
  // the current field is an array with constant entries. The first
  // entry into the pair is the index in the array, the second entry is
  // the constant value.
  //
  // NOTE: This will replace the set ArrayConstEntries when we fully move
  // to opaque pointers and the typed pointers code is removed. Also,
  // the typed pointers case will use this map to transfer the information
  // of arrays with constant entries to the DTrans immutable analysis.
  DenseMap<Constant*, Constant*> ArrayWithConstEntriesMap;

  // True if it is enabled to insert new entries into ArrayWithConstEntriesMap,
  // else false. Also, false means that the current field may be an array but
  // the entries aren't constant, or the actual field is not even an array.
  bool canAddConstantEntriesForArray;
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
const SafetyData BadCasting = 0x0000'0000'0000'0001;

/// The size arguments passed to an allocation call could not be proven to
/// be a multiple of the size of the type being allocated.
const SafetyData BadAllocSizeArg = 0x0000'0000'0000'0002;

/// A pointer to an aggregate type is manipulated to compute an address that
/// is not the address of a field within the type.
const SafetyData BadPtrManipulation = 0x0000'0000'0000'0004;

/// An i8* value that may alias to multiple types is passed to a GetElementPtr
/// instruction.
const SafetyData AmbiguousGEP = 0x0000'0000'0000'0008;

/// A volatile memory operation was found operating on the type on one of its
/// elements.
const SafetyData VolatileData = 0x0000'0000'0000'0010;

/// A load or store operation was used with a pointer to an element within an
/// aggregate type, but the type of value loaded or stored did not match the
/// element type.
const SafetyData MismatchedElementAccess = 0x0000'0000'0000'0020;

/// A load or store instruction was found which loads or stores an entire
/// instance of the type.
const SafetyData WholeStructureReference = 0x0000'0000'0000'0040;

/// A store was seen using a value operand that aliases to a type of interest
/// with a pointer operand that was not known to alias to a pointer to a
/// pointer to that type.
const SafetyData UnsafePointerStore = 0x0000'0000'0000'0080;

/// The addresses of one or more fields within the type were written to memory.
const SafetyData FieldAddressTakenMemory = 0x0000'0000'0000'0100;

/// A global variable was found which is a pointer to the type.
const SafetyData GlobalPtr = 0x0000'0000'0000'0200;

/// A global variable was found which is an instance of the type.
const SafetyData GlobalInstance = 0x0000'0000'0000'0400;

/// A global variable was found which is an instance of the type and has a
/// non-zero initializer.
const SafetyData HasInitializerList = 0x0000'0000'0000'0800;

/// A PHI node or select was found with incompatible incoming values.
const SafetyData UnsafePtrMerge = 0x0000'0000'0000'1000;

/// A structure is modified via a memory function intrinsic (memcpy, memmove,
/// or memset), with a size that differs from the native structure size.
const SafetyData BadMemFuncSize = 0x0000'0000'0000'2000;

/// A proper subset of fields in a structure is modified via a memory function
/// intrinsic (memcpy, memmove, or memset).
const SafetyData MemFuncPartialWrite = 0x0000'0000'0000'4000;

/// A structure is modified via a memory function intrinsic (memcpy or memmove)
/// with conflicting or unknown types for the source and destination parameters.
const SafetyData BadMemFuncManipulation = 0x0000'0000'0000'8000;

/// A pointer is passed to an intrinsic or library function that can alias
/// incompatible types.
const SafetyData AmbiguousPointerTarget = 0x0000'0000'0001'0000;

/// The address of an aggregate object escaped through a function call or
/// a return statement.
const SafetyData AddressTaken = 0x0000'0000'0002'0000;

/// The structure was declared with no fields.
const SafetyData NoFieldsInStruct = 0x0000'0000'0004'0000;

/// The structure is contained as a non-pointer member of another structure.
const SafetyData NestedStruct = 0x0000'0000'0008'0000;

/// The structure contains another structure as a non-pointer member.
const SafetyData ContainsNestedStruct = 0x0000'0000'0010'0000;

/// The structure was identified as a system object type.
const SafetyData SystemObject = 0x0000'0000'0020'0000;

/// A local variable was found which is a pointer to the type.
const SafetyData LocalPtr = 0x00000'0000'0040'0000;

/// A local variable was found which is an instance of the type.
const SafetyData LocalInstance = 0x0000'0000'0080'0000;

/// A function was called with an i8* argument where the aliases of the
/// value passed to the function do not match the uses of the argument
/// within the function..
const SafetyData MismatchedArgUse = 0x0000'0000'0100'0000;

/// A global variable was found which is an array of the type.
const SafetyData GlobalArray = 0x0000'0000'0200'0000;

/// An element in the structure looks like a vtable.
const SafetyData HasVTable = 0x0000'0000'0400'0000;

/// An element in the structure points to a function.
const SafetyData HasFnPtr = 0x0000'0000'0800'0000;

/// A type has C++ processing:
///   allocation/deallocation with new/delete;
///   invoke instruction returns or takes structure/
///     pointer to structure.
const SafetyData HasCppHandling = 0x0000'0000'1000'0000;

/// The structure contains zero-sized array as the last field.
const SafetyData HasZeroSizedArray = 0x0000'0000'2000'0000;

// For use with BadCastingAnalyzer

/// A potential bad casting issue that will be either eliminated,
/// converted to bad casting conditional, or converted to bad casting
/// at the end of analysis by the bad casting analyzer.
const SafetyData BadCastingPending = 0x0000'0000'4000'0000;

/// Indicates that bad casting will occur only if specific conditions
/// are not fulfilled.  These conditions are noted by the bad casting
/// analyzer, and involve certain functions' arguments being nullptr on
/// entry to those functions.
const SafetyData BadCastingConditional = 0x0000'0000'8000'0000;

/// A potential unsafe pointer store issue that will be either eliminated,
/// converted to unsafe pointer store conditional, or converted to unsafe
/// pointer store at the end of analysis by the bad casting analyzer.
const SafetyData UnsafePointerStorePending = 0x0000'0001'0000'0000;

/// Indicates that an unsafe pointer store will occur only if specific
/// conditions are not fulfilled.  These conditions are noted by the bad
/// casting analyzer, and involve certain functions' arguments being nullptr
/// on entry to those functions.
const SafetyData UnsafePointerStoreConditional = 0x0000'0002'0000'0000;

/// A potential mismatched element access issue that will be either eliminated,
/// converted to mismatched element access conditional, or converted to
/// mismatched element access at the end of analysis by the bad casting
/// analyzer.
const SafetyData MismatchedElementAccessPending = 0x0000'0004'0000'0000;

/// Indicates that an mismatched element access will occur only if specific
/// conditions are not fulfilled.  These conditions are noted by the bad
/// casting analyzer, and involve certain functions' arguments being nullptr
/// on entry to those functions.
const SafetyData MismatchedElementAccessConditional = 0x0000'0008'0000'0000;

// End for use with BadCastingAnalyzer

/// The structure was identified as a dope vector type.
const SafetyData DopeVector = 0x0000'0010'0000'0000;

/// The following safety violations are for related types. These types are
/// structures that have two types in the IR, where one type represents the
/// base form and the other type has the same fields with an extra field at
/// the end used for padding.

/// This safety data is used for special bad casting cases that won't affect
/// related types.
const SafetyData BadCastingForRelatedTypes = 0x0000'0020'0000'0000;

/// This safety data is used to check if a bad pointer manipulation won't
/// affect the related types.
const SafetyData BadPtrManipulationForRelatedTypes = 0x0000'0040'0000'0000;

/// This safety data is used for a special mismatched element access to
/// the zero field of a structure but won't affect the related types.
const SafetyData MismatchedElementAccessRelatedTypes = 0x0000'0080'0000'0000;

/// This safety data is used for special unsafe pointer store to the zero
/// field of a structure but won't affect related types.
const SafetyData UnsafePointerStoreRelatedTypes = 0x0000'0100'0000'0000;

/// This safety data is used when a memory handling function (e.g. memcpy)
/// modifies part of the nested structures, but it won't fully cover the
/// field zero in the outer most structure.
const SafetyData MemFuncNestedStructsPartialWrite = 0x0000'0200'0000'0000;

/// This safety data is used when the memory allocation size is constant, but
/// is not a direct multiple of the element size. e.g. ElemSize * 4 + 128
const SafetyData ComplexAllocSize = 0x0000'0400'0000'0000;

/// This safety data is used when the address of a field is passed as an
/// argument to a callsite.
const SafetyData FieldAddressTakenCall = 0x0000'0800'0000'0000;

/// This safety data is used when the address of a field is returned by a
/// function.
const SafetyData FieldAddressTakenReturn = 0x0000'1000'0000'0000;

/// This safety data is used when a structure may have an extra field at the
/// end that could be used for ABI padding. There will be a base structure too
/// that doesn't have the extra field and uses the same name with '.base' at
/// the end. For example:
///
///   %struct.test.a = type <{ i32, i32, [4 x i8] }>
///   %struct.test.a.base = type <{ i32, i32 }>
///
/// The structure %struct.test.a is set as StructCouldHaveABIPadding since the
/// last field is used for padding, and the structure %struct.test.a.base is
/// the base structure.
const SafetyData StructCouldHaveABIPadding = 0x0000'2000'0000'0000;

/// This safety data is set when a structure is used as base structure for ABI
/// padding. The structure will have '.base' at the end the of name, and there
/// will be another structure related to it that have an extra field for
/// padding. For example:
///
///   %struct.test.a = type <{ i32, i32, [4 x i8] }>
///   %struct.test.a.base = type <{ i32, i32 }>
///
/// The structure %struct.test.a.base is set as StructCouldBeBaseABIPadding
/// since it is the base structure of %struct.test.a, which is the padded
/// structure.
const SafetyData StructCouldBeBaseABIPadding = 0x0000'4000'0000'0000;

/// This safety data is set when a possible BadMemFuncManipulation can happen
/// between types that have ABI padding, but it won't affect the padded field.
/// For an example of ABI padding, see StructCouldBeBaseABIPadding above.
const SafetyData BadMemFuncManipulationForRelatedTypes = 0x0000'8000'0000'0000;

/// This safety data is set when the type of a PHI node is identified for ABI
/// padding, but the instruction and its incoming values won't affect the
/// padded field. For an example of ABI padding, see
/// StructCouldBeBaseABIPadding above.
const SafetyData UnsafePtrMergeRelatedTypes = 0x0001'0000'0000'0000;

/// Multiple structures are modified via a single memory function intrinsic
// (memcpy, memmove, or memset).
const SafetyData MultiStructMemFunc = 0x0002'0000'0000'0000;

/// This is a catch-all flag that will be used to mark any usage pattern
/// that we don't specifically recognize. The use might actually be safe
/// or unsafe, but we will conservatively assume it is unsafe.
const SafetyData UnhandledUse = 0x8000'0000'0000'0000;

/// This condition encapsulates all forms of field address taken
/// (FieldAddressTakenMemory, FieldAddressTakenCall and
/// FieldAddressTakenReturn).
static const SafetyData AnyFieldAddressTaken =
    FieldAddressTakenMemory | FieldAddressTakenCall | FieldAddressTakenReturn;

// Enum that could be used to identify which variation of safety data will be
// preferred when we have multiple forms of the same safety violation. List
// could be expanded as we add more cases.
//
// NOTE: Not all safety violations will have all the forms listed in the enum.
// For example, the only variation for BadPtrManipulation is
// BadPtrManipulationForRelatedTypes. This means that when SafetyDataKind is
// used, it needs to be guarded with the proper checks for each case. This
// could be extended as a helper class as needed.
enum SafetyDataKind {
  Original,    // Used for identifying the original case (e.g. BadCasting)
  Pending,     // Used for "pending" (e.g. BadCastingPending)
  Conditional, // Used for "conditional" (e.g. BadCastingConditional)
  RelatedTypes // Used for "related types" (e.g. BadCastingForRelatedTypes)
};

// TODO: Create a safety mask for the conditions that are common to all
//       DTrans optimizations.
//
// Safety conditions for field reordering and deletion.
//

// TODO: Delete fields need to be updated to enable the cases with ABI padding
const SafetyData SDDeleteField =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | AnyFieldAddressTaken | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget | UnsafePtrMerge |
    AddressTaken | NoFieldsInStruct | SystemObject | MismatchedArgUse |
    HasVTable | HasFnPtr | HasZeroSizedArray | HasFnPtr |
    BadCastingConditional | UnsafePointerStoreConditional |
    MismatchedElementAccessConditional | DopeVector |
    BadCastingForRelatedTypes | BadPtrManipulationForRelatedTypes |
    MismatchedElementAccessRelatedTypes | UnsafePointerStoreRelatedTypes |
    MemFuncNestedStructsPartialWrite | ComplexAllocSize |
    StructCouldHaveABIPadding | StructCouldBeBaseABIPadding |
    BadMemFuncManipulationForRelatedTypes | UnsafePtrMergeRelatedTypes |
    MultiStructMemFunc;

const SafetyData SDReuseField =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | AnyFieldAddressTaken | GlobalInstance |
    HasInitializerList | UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
    MismatchedArgUse | LocalInstance | HasCppHandling | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    UnhandledUse | DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding | StructCouldBeBaseABIPadding |
    BadMemFuncManipulationForRelatedTypes;

const SafetyData SDReuseFieldPtrOfPtr =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | AnyFieldAddressTaken | HasInitializerList |
    UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
    MismatchedArgUse | LocalInstance | HasCppHandling | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    UnhandledUse | DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding | StructCouldBeBaseABIPadding |
    BadMemFuncManipulationForRelatedTypes;

const SafetyData SDReuseFieldPtr =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | FieldAddressTakenMemory | FieldAddressTakenReturn |
    HasInitializerList | UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
    MismatchedArgUse | LocalInstance | HasCppHandling | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    UnhandledUse | DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding | StructCouldBeBaseABIPadding |
    BadMemFuncManipulationForRelatedTypes;

const SafetyData SDReorderFields =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | AnyFieldAddressTaken | GlobalInstance |
    HasInitializerList | UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
    MismatchedArgUse | LocalInstance | HasCppHandling | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    UnhandledUse | DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding |
    StructCouldBeBaseABIPadding | BadMemFuncManipulationForRelatedTypes |
    UnsafePtrMergeRelatedTypes | MultiStructMemFunc;

const SafetyData SDReorderFieldsDependent =
    BadPtrManipulation | GlobalInstance | HasInitializerList |
    MemFuncPartialWrite | NoFieldsInStruct | LocalInstance |
    BadCastingConditional | UnsafePointerStoreConditional |
    UnhandledUse | WholeStructureReference | VolatileData | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget | DopeVector |
    BadPtrManipulationForRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding |
    StructCouldBeBaseABIPadding | BadMemFuncManipulationForRelatedTypes;

//
// Safety conditions for field single value analysis
//
const SafetyData SDFieldSingleValueNoFieldAddressTaken =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
    MismatchedElementAccess | UnsafePointerStore | AmbiguousPointerTarget |
    UnsafePtrMerge | AddressTaken | MismatchedArgUse |
    BadCastingForRelatedTypes | BadPtrManipulationForRelatedTypes |
    MismatchedElementAccessRelatedTypes | UnsafePointerStoreRelatedTypes |
    UnsafePtrMergeRelatedTypes | UnhandledUse;

const SafetyData SDFieldSingleValue =
    SDFieldSingleValueNoFieldAddressTaken | AnyFieldAddressTaken;

const SafetyData SDSingleAllocFunctionNoFieldAddressTaken =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
    MismatchedElementAccess | UnsafePointerStore | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget | UnsafePtrMerge |
    AddressTaken | MismatchedArgUse | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    BadMemFuncManipulationForRelatedTypes | UnsafePtrMergeRelatedTypes |
    UnhandledUse;

const SafetyData SDSingleAllocFunction =
    SDSingleAllocFunctionNoFieldAddressTaken | AnyFieldAddressTaken;

const SafetyData SDElimROFieldAccess =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
    MismatchedElementAccess | UnsafePointerStore | AnyFieldAddressTaken |
    BadMemFuncSize | BadMemFuncManipulation | AmbiguousPointerTarget |
    HasInitializerList | UnsafePtrMerge | AddressTaken | MismatchedArgUse |
    BadCastingConditional | UnsafePointerStoreConditional |
    MismatchedElementAccessConditional | UnhandledUse |
    DopeVector | BadCastingForRelatedTypes | BadPtrManipulationForRelatedTypes |
    MismatchedElementAccessRelatedTypes | UnsafePointerStoreRelatedTypes |
    MemFuncNestedStructsPartialWrite | BadMemFuncManipulationForRelatedTypes |
    UnsafePtrMergeRelatedTypes;

//
// Safety conditions for a structure to be considered for the AOS-to-SOA
// transformation.
const SafetyData SDAOSToSOA =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | AnyFieldAddressTaken | GlobalInstance |
    HasInitializerList | UnsafePtrMerge | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
    LocalInstance | MismatchedArgUse | GlobalArray | HasVTable | HasFnPtr |
    HasCppHandling | HasZeroSizedArray | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding |
    StructCouldBeBaseABIPadding | BadMemFuncManipulationForRelatedTypes |
    UnsafePtrMergeRelatedTypes | MultiStructMemFunc;

//
// Safety conditions for a structure type that contains a pointer to a
// structure that is being considered for the AOS-to-SOA transformation.
//
const SafetyData SDAOSToSOADependent =
    BadCasting | BadPtrManipulation | AmbiguousGEP | VolatileData |
    MismatchedElementAccess | WholeStructureReference | UnsafePointerStore |
    UnsafePtrMerge | AmbiguousPointerTarget | AddressTaken | NoFieldsInStruct |
    NestedStruct | ContainsNestedStruct | SystemObject | MismatchedArgUse |
    GlobalArray | HasVTable | HasCppHandling | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | ComplexAllocSize |
    StructCouldHaveABIPadding | StructCouldBeBaseABIPadding |
    UnsafePtrMergeRelatedTypes;

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
    HasZeroSizedArray | BadCastingConditional | UnsafePointerStoreConditional |
    MismatchedElementAccessConditional | DopeVector |
    BadCastingForRelatedTypes | BadPtrManipulationForRelatedTypes |
    MismatchedElementAccessRelatedTypes | UnsafePointerStoreRelatedTypes |
    MemFuncNestedStructsPartialWrite | ComplexAllocSize |
    StructCouldHaveABIPadding | StructCouldBeBaseABIPadding |
    BadMemFuncManipulationForRelatedTypes | UnsafePtrMergeRelatedTypes;

const SafetyData SDDynClone =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | AnyFieldAddressTaken | GlobalInstance |
    HasInitializerList | UnsafePtrMerge | BadMemFuncSize | MemFuncPartialWrite |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | NestedStruct | ContainsNestedStruct | SystemObject |
    LocalInstance | MismatchedArgUse | GlobalArray | HasVTable | HasFnPtr |
    HasZeroSizedArray | BadCastingConditional | UnsafePointerStoreConditional |
    MismatchedElementAccessConditional | UnhandledUse | DopeVector |
    BadCastingForRelatedTypes | BadPtrManipulationForRelatedTypes |
    MismatchedElementAccessRelatedTypes | UnsafePointerStoreRelatedTypes |
    MemFuncNestedStructsPartialWrite | ComplexAllocSize |
    BadMemFuncManipulationForRelatedTypes | UnsafePtrMergeRelatedTypes;

const SafetyData SDSOAToAOS =
    BadCasting | BadPtrManipulation | VolatileData | MismatchedElementAccess |
    WholeStructureReference | UnsafePointerStore | AnyFieldAddressTaken |
    GlobalInstance | HasInitializerList | UnsafePtrMerge | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | SystemObject | LocalInstance | MismatchedArgUse |
    GlobalArray | HasFnPtr | HasZeroSizedArray | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    UnhandledUse | DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding |
    StructCouldBeBaseABIPadding | BadMemFuncManipulationForRelatedTypes |
    UnsafePtrMergeRelatedTypes;

const SafetyData SDMemInitTrimDown =
    BadCasting | BadPtrManipulation | VolatileData | MismatchedElementAccess |
    WholeStructureReference | UnsafePointerStore | AnyFieldAddressTaken |
    GlobalInstance | HasInitializerList | UnsafePtrMerge | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget | AddressTaken |
    NoFieldsInStruct | SystemObject | LocalInstance | MismatchedArgUse |
    GlobalArray | HasFnPtr | HasZeroSizedArray | BadCastingConditional |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    UnhandledUse | DopeVector | BadCastingForRelatedTypes |
    BadPtrManipulationForRelatedTypes | MismatchedElementAccessRelatedTypes |
    UnsafePointerStoreRelatedTypes | MemFuncNestedStructsPartialWrite |
    ComplexAllocSize | StructCouldHaveABIPadding |
    StructCouldBeBaseABIPadding | BadMemFuncManipulationForRelatedTypes |
    UnsafePtrMergeRelatedTypes;

// Safety conditions for arrays with constant entries
// NOTE: FieldAddressTakenReturn is conservative. We can extend the analysis
// to check if the fields we care about will never be returned by a function.
const SafetyData SDArraysWithConstantEntries =
    BadCasting | BadAllocSizeArg | BadPtrManipulation | AmbiguousGEP |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | FieldAddressTakenMemory | HasInitializerList |
    GlobalArray | GlobalInstance | UnsafePtrMerge | BadMemFuncSize |
    MemFuncPartialWrite | BadMemFuncManipulation | AmbiguousPointerTarget |
    AddressTaken | NoFieldsInStruct | SystemObject | MismatchedArgUse |
    BadCastingPending | BadCastingConditional | UnsafePointerStorePending |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    FieldAddressTakenReturn | UnhandledUse;

// Safety conditions for arrays with constant entries in the opaque pointers
// case. The difference is that AmbiguousGEP won't be considered since the
// structures with related types will have uses without dominant aggregate
// types. Also, MemFuncPartialWrite is not an issue since a memfunc can
// be used to copy part of the structure.
// NOTE: This safety data should replace SDArraysWithConstantEntries
// when the code for the types pointed pointers is removed.
const SafetyData SDArraysWithConstantEntriesOpq =
    BadCasting | BadAllocSizeArg | BadPtrManipulation |
    VolatileData | MismatchedElementAccess | WholeStructureReference |
    UnsafePointerStore | FieldAddressTakenMemory | HasInitializerList |
    GlobalArray | GlobalInstance | UnsafePtrMerge | BadMemFuncSize |
    BadMemFuncManipulation | AmbiguousPointerTarget |
    AddressTaken | NoFieldsInStruct | SystemObject | MismatchedArgUse |
    BadCastingPending | BadCastingConditional | UnsafePointerStorePending |
    UnsafePointerStoreConditional | MismatchedElementAccessConditional |
    FieldAddressTakenReturn | UnhandledUse;

//
// TODO: Update the list each time we add a new safety conditions check for a
// new transformation pass.
//
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
const Transform DT_ArraysWithConstantEntries = 0x1000;
const Transform DT_ReuseField = 0x2000;
const Transform DT_ReuseFieldPtr = 0x4000;
const Transform DT_ReuseFieldPtrOfPtr = 0x8000;
const Transform DT_Last = 0x10000;
const Transform DT_Legal = 0xffff;

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
  TypeInfo(TypeInfoKind Kind, AbstractType Ty)
      : Ty(Ty), SafetyInfo(0), TIK(Kind), CRTypeKind(CRT_Unknown) {}

public:
  llvm::Type *getLLVMType() const { return Ty.getLLVMType(); }
  dtransOP::DTransType *getDTransType() const { return Ty.getDTransType(); }
  bool isDTransType() const { return Ty.isDTransType(); }

  bool testSafetyData(SafetyData Conditions) const {
    // If any unhandled uses have been seen, assume all conditions are set.
    if (SafetyInfo & dtrans::UnhandledUse)
      return true;
    return (SafetyInfo & Conditions);
  }

  void setSafetyData(SafetyData Conditions);

  void resetSafetyData(SafetyData Conditions) { SafetyInfo &= ~Conditions; }

  void clearSafetyData() { SafetyInfo = 0; }

  void mergeSafetyDataWithRelatedType();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printSafetyData(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  CRuleTypeKind getCRuleTypeKind() const { return CRTypeKind; }
  void setCRuleTypeKind(CRuleTypeKind K) { CRTypeKind = K; }

  // Returns true if the type is a zero-sized array or it is a structure with a
  // zero-sized array.
  bool hasZeroSizedArrayAsLastField() {
    return (SafetyInfo & dtrans::HasZeroSizedArray);
  }

private:
  AbstractType Ty;
  SafetyData SafetyInfo;

  // ID to support type inquiry through isa, cast, and dyn_cast
  TypeInfoKind TIK;
  // Indicates whether the Type has a C language rule compatible Type
  CRuleTypeKind CRTypeKind;
};

class NonAggregateTypeInfo : public TypeInfo {
public:
  NonAggregateTypeInfo(AbstractType Ty)
      : TypeInfo(TypeInfo::NonAggregateInfo, Ty) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::NonAggregateInfo;
  }
};

class PointerInfo : public TypeInfo {
public:
  PointerInfo(AbstractType Ty) : TypeInfo(TypeInfo::PtrInfo, Ty) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::PtrInfo;
  }
};

class StructInfo : public TypeInfo {
public:
  StructInfo(AbstractType Ty, ArrayRef<AbstractType> FieldTypes)
      : TypeInfo(TypeInfo::StructInfo, Ty), TotalFrequency(0), IsIgnoredFor(0),
        SubGraph() {
    for (AbstractType FieldTy : FieldTypes)
      Fields.push_back(FieldInfo(FieldTy));
  }

  size_t getNumFields() const { return Fields.size(); }
  SmallVectorImpl <FieldInfo> &getFields() { return Fields; }
  const SmallVectorImpl <FieldInfo> &getFields() const { return Fields; }
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
  dtrans::Transform getIgnoredFor() const { return IsIgnoredFor; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { print(dbgs()); }

  // Print the structure information.
  // An optional annotation function can be passed in to report extra
  // information about the structure, such as transformations that will ignore
  // the safety mask.
  void print(raw_ostream &OS,
             std::function<void(raw_ostream &OS, const StructInfo *)>
                 *Annotator = nullptr) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // To represent call graph in C++ one stores outermost type,
  // in whose methods there was reference to this structure.
  // Lattice: {bottom = <nullptr, false>, <Type*, false>, top = <nullptr, true>}
  // Type should be some struct from which given type is reachable.
  // GlobalVariable (Instance): Treat conservatively by marking it as 'top'.
  class CallSubGraph {
    PointerIntPair<StructType *, 1, bool> State;
  public:
    // State is zero-initialized to 'bottom'.
    CallSubGraph() = default;
    bool isBottom() const {
      return !State.getPointer() && !State.getInt();
    }
    bool isTop() const {
      return State.getInt();
    }
    void setTop() {
      State.setPointer(nullptr);
      State.setInt(true);
    }
    StructType *getEnclosingType() const {
      assert(!isBottom() && !isTop() && "Invalid access to CallSubGraph");
      return State.getPointer();
    }
    void setEnclosingType(StructType *STy) {
      return State.setPointer(STy);
    }
  };

  void setCallGraphEnclosingType(StructType *STy) {
    SubGraph.setEnclosingType(STy);
  }

  void setCallGraphTop() {
    SubGraph.setTop();
  }

  const CallSubGraph &getCallSubGraph() const { return SubGraph; }

  // Return the related type stored in the StructInfo
  dtrans::StructInfo* getRelatedType() const { return RelatedType; }

  void setRelatedType(dtrans::StructInfo *RelType) {
    assert (!RelatedType && "Only one related type could be set");
    RelatedType = RelType;
  }

  // Remove the related type and restore the safety conditions.
  void unsetRelatedType();

  // Return true if the last field in the structure is used for padding.
  bool hasPaddedField();

  // Update 'FieldNum' to indicate it has been assigned result of
  // calling 'Callee'.
  void updateNewSingleAllocFunc(unsigned FieldNum, Function &Callee);

  // Indicate 'FieldNum' is BottomAllocFunc
  void updateSingleAllocFuncToBottom(unsigned FieldNum);

  // Return true if the current structure is a base structure for ABI padding
  bool isABIPaddingBaseStructure() { return RTForm == RT_BASE; }

  // Return true if the current structure is a padded structure for ABI padding
  bool isABIPaddingPaddedStructure() { return RTForm == RT_PADDED; }

  // Return true if the current structure is base or padded structure for
  // ABI padding
  bool isUsedForABIPadding() {
    return RelatedType &&
           (isABIPaddingBaseStructure() || isABIPaddingPaddedStructure());
  }

  // Set the current structure as base structure for ABI padding
  void setAsABIPaddingBaseStructure();

  // Set the current structure as padded structure for ABI padding
  void setAsABIPaddingPaddedStructure();

private:
  SmallVector<FieldInfo, 16> Fields;
  // Total Frequency of all fields in struct.
  uint64_t TotalFrequency;
  dtrans::Transform IsIgnoredFor;
  CallSubGraph SubGraph;

  // The related type is used to map a base type to a padded type.
  // For example, consider the following classes:
  //
  // class A {
  //   int a;
  //   int b;
  //   int c;
  // }
  //
  // class B : public A {
  //   int d;
  // }
  //
  // Also consider that if there is an instantiation of A and B, then we will
  // see something like this in the IR:
  //
  // %class.A = type <{i32, i32, i32, [ 4 x i8 ]}>
  // %class.A.base = type <{i32, i32, i32}>
  // %class.B = type {%class.A.base, i32}
  //
  // The IR creates two forms of class A. The ".base" form is used for the
  // structure hierarchy, and the padded form (non ".base") is used across
  // the whole IR (bitcasting, GEPs, etc.). The padding is added by the CFE
  // and is used by the application binary interface (ABI). When we find
  // this relationship then we set the padded type as the RelatedType for
  // the ".base" class, and vice-versa. From the example above:
  //
  //   RelatedType for %class.A = %class.A.base
  //   RelatedType for %class.A.base = %class.A
  dtrans::StructInfo *RelatedType = nullptr;

  // Enum for handling which related type form is the current class. This
  // is used to identify if the current structure is base or padded structure
  // when handling ABI padding.
  enum RelatedTypeForm {
    RT_TOP,                 // Structure is not set for ABI padding
    RT_BASE,                // Base structure
    RT_PADDED,              // Padded structure
    RT_BOTTOM               // Can't be padded or base structure
  };

  // Used for tracking which type of ABI padding is the current structure.
  RelatedTypeForm RTForm = RT_TOP;
};

class ArrayInfo : public TypeInfo {
public:
  ArrayInfo(AbstractType Ty, dtrans::TypeInfo *DTransElemTy, size_t Size)
      : TypeInfo(TypeInfo::ArrayInfo, Ty), DTransElemTy(DTransElemTy),
        NumElements(Size) {}

  TypeInfo *getElementDTransInfo() const { return DTransElemTy; }
  llvm::Type *getElementLLVMType() const { return DTransElemTy->getLLVMType(); }
  dtransOP::DTransType *getElementDTransType() const {
    return DTransElemTy->getDTransType();
  }
  size_t getNumElements() const { return NumElements; }

  /// Method to support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const TypeInfo *TI) {
    return TI->getTypeInfoKind() == TypeInfo::ArrayInfo;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const { print(dbgs()); }
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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
  typedef SmallVector<AbstractType, 2> TypeAliasSet;
  typedef SmallVectorImpl<AbstractType> &TypeAliasSetRef;

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

  void addElemType(AbstractType Ty) {
    ElemTypes.push_back(Ty);
  }

  struct element_llvm_types_iterator
      : public iterator_adaptor_base<element_llvm_types_iterator,
                                     TypeAliasSet::iterator,
                                     std::forward_iterator_tag, AbstractType> {
    explicit element_llvm_types_iterator(TypeAliasSet::iterator X)
        : iterator_adaptor_base(X) {}

    llvm::Type *operator*() const { return I->getLLVMType(); }
    llvm::Type *operator->() const { return operator*(); }
  };

  iterator_range<element_llvm_types_iterator> element_llvm_types() {
    return make_range(element_llvm_types_iterator(ElemTypes.begin()),
                      element_llvm_types_iterator(ElemTypes.end()));
  }

  struct element_dtrans_types_iterator
    : public iterator_adaptor_base<element_dtrans_types_iterator,
    TypeAliasSet::iterator,
    std::forward_iterator_tag, AbstractType> {
    explicit element_dtrans_types_iterator(TypeAliasSet::iterator X)
      : iterator_adaptor_base(X) {}

    dtransOP::DTransType *operator*() const { return I->getDTransType(); }
    dtransOP::DTransType *operator->() const { return operator*(); }
  };

  iterator_range<element_dtrans_types_iterator> element_dtrans_types() {
    return make_range(element_dtrans_types_iterator(ElemTypes.begin()),
      element_dtrans_types_iterator(ElemTypes.end()));
  }

  TypeAliasSet::iterator begin() { return ElemTypes.begin(); }
  TypeAliasSet::iterator end() { return ElemTypes.end(); }

  size_t getNumTypes() const { return ElemTypes.size(); }

  llvm::Type *getElemLLVMType(size_t Idx) const {
    assert(Idx < ElemTypes.size() && "Index out of range");
    return ElemTypes[Idx].getLLVMType();
  }

  dtransOP::DTransType *getElemDTransType(size_t Idx) const {
    assert(Idx < ElemTypes.size() && "Index out of range");
    return ElemTypes[Idx].getDTransType();
  }

  // Change the type at index \p Idx to type \p Ty. This
  // function should only be used for updating a type based
  // on the type remapping done when processing a function.
  void setElemType(size_t Idx, AbstractType Ty) {
    assert(Idx < ElemTypes.size() && "Index out of range");
    assert(!(ElemTypes[Idx].isDTransType() ^ Ty.isDTransType()) &&
      "Cannot switch between DTransTypes and llvm::Types");

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

  void addElemType(AbstractType Ty) {
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

typedef SmallVector<CallInfo *, 2> CallInfoVec;

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

// Container class for managing the mapping of Instructions to CallInfo objects.
class CallInfoManager {
public:
  CallInfoManager() {}
  ~CallInfoManager() { reset(); }

  // This class owns pointers to CallInfo objects that will be destroyed when
  // the destructor is run. Copying is not permitted, because that would lead
  // to multiple instances holding the same pointer.
  CallInfoManager(const CallInfoManager &) = delete;
  CallInfoManager &operator=(const CallInfoManager &) = delete;

  CallInfoManager(CallInfoManager &&) = default;
  CallInfoManager &operator=(CallInfoManager &&) = default;

  // Retrieve the unique CallInfo object for the instruction, if information
  // exists. Otherwise, return nullptr.
  dtrans::CallInfo *getCallInfo(const Instruction *I) const;

  // Retrieve the CallInfoVec object for the instruction, if information exists.
  // Otherwise, return nullptr.
  dtrans::CallInfoVec *getCallInfoVec(const Instruction *I);
  const dtrans::CallInfoVec *getCallInfoVec(const Instruction *I) const;

  // Create an entry in the CallInfoMap about a memory allocation call.
  dtrans::AllocCallInfo *createAllocCallInfo(Instruction *I,
                                             dtrans::AllocKind AK);

  // Create an entry in the CallInfoMap about a memory freeing call
  dtrans::FreeCallInfo *createFreeCallInfo(Instruction *I, dtrans::FreeKind FK);

  // Create an entry in the CallInfoMap about a memset call.
  dtrans::MemfuncCallInfo *
  createMemfuncCallInfo(Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
                        dtrans::MemfuncRegion &MR);

  // Create an entry in the CallInfoMap about a memcpy/memmove call.
  dtrans::MemfuncCallInfo *
  createMemfuncCallInfo(Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
                        dtrans::MemfuncRegion &MR1, dtrans::MemfuncRegion &MR2);

  // Destroy the CallInfo stored about the specific instruction.
  void deleteCallInfo(Instruction *I);

  // Destroy the CallInfoVec stored about the specific instruction.
  void deleteCallInfoVec(Instruction *I);

  // Update the instruction associated with the CallInfo object. This
  // is necessary when a function is cloned during the DTrans optimizations to
  // transfer the information to the newly created instruction of the cloned
  // routine.
  void replaceCallInfoInstruction(dtrans::CallInfo *Info, Instruction *NewI);

  // Clear all the entries from the CallInfoMap.
  void reset();

  using CallInfoMapType = DenseMap<llvm::Instruction *, dtrans::CallInfoVec>;

  // Adaptor for directly iterating over the dtrans::CallInfo pointers.
  struct call_info_iterator
      : public iterator_adaptor_base<
            call_info_iterator, CallInfoMapType::iterator,
            std::forward_iterator_tag, CallInfoMapType::value_type> {
    explicit call_info_iterator(CallInfoMapType::iterator X)
        : iterator_adaptor_base(X) {}

    dtrans::CallInfoVec &operator*() const { return I->second; }
    dtrans::CallInfoVec &operator->() const { return operator*(); }
  };

  iterator_range<call_info_iterator> call_info_entries() {
    return make_range(call_info_iterator(CallInfoMap.begin()),
                      call_info_iterator(CallInfoMap.end()));
  }

private: // methods
  void addCallInfo(llvm::Instruction *I, dtrans::CallInfo *Info);
  void destructCallInfo(dtrans::CallInfo *Info);

private: // data
  // A mapping from function calls that special information is collected for
  // (malloc, free, memset, etc) to the information stored about those calls.
  CallInfoMapType CallInfoMap;
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

// Returns true if Ty is either StructType or SequentialType.
bool dtransIsCompositeType(llvm::Type *Ty);

// Return true if "Idx" is valid index for Ty.
bool dtransCompositeIndexValid(llvm::Type *Ty, unsigned Idx);

// Returns type at "Idx" in "Ty".
llvm::Type *dtransCompositeGetTypeAtIndex(llvm::Type *Ty, unsigned Idx);

// Return the type loaded if the Load instruction is loading the 0 element
// in a structure
llvm::Type* getTypeForZeroElementLoaded(LoadInst *Load,
                                        llvm::Type **Pointee);

// Return true if the BitCast instruction is used for loading the 0 element
// in a structure
bool isBitCastLoadingZeroElement(BitCastInst *BC);

// Return true if the input type Type1 is the same as Type2 except for
// the last element (or vice versa). This last element is used by the ABI.
bool isPaddedStruct(llvm::Type *Type1, llvm::Type *Type2);

// Given a type, find the related type from the input Module M.
llvm::Type* collectRelatedType(llvm::Type *InTy, Module &M);

StringRef getTypeBaseName(StringRef TyName);
llvm::StructType *getContainedStructTy(llvm::Type *Ty);
void collectAllStructTypes(Module &M, SetVector<llvm::StructType *> &SeenTypes);
} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANS_H
