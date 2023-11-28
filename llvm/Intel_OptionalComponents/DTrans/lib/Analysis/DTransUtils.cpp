//===------------ Intel_DTransUtils.cpp - Utilities for DTrans ------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of utilities that are used by DTrans for both
/// analysis and optimization.
///
// ===--------------------------------------------------------------------=== //

#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace llvm::PatternMatch;
using namespace dtrans;

#define DEBUG_TYPE "dtransanalysis"

// Debug type for verbose field single alloc function analysis output.
#define SAFETY_FSAF "dtrans-safetyanalyzer-fsaf"

// When set, force the opaque pointer passes instead of the legacy DTrans passes
// to run if the IR is using typed pointers.
static cl::opt<bool> DTransForceOpaquePointerPasses(
    "dtransop-allow-typed-pointers", cl::init(false), cl::Hidden,
    cl::desc("Use the DTrans opaque pointers passes even when the IR is using "
             "typed pointers"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
cl::opt<bool> dtrans::DTransPrintAnalyzedTypes("dtrans-print-types",
                                               cl::ReallyHidden);

/// Prints information that is saved during analysis about specific function
/// calls (malloc, free, memset, etc) that may be useful to the transformations.
cl::opt<bool> dtrans::DTransPrintAnalyzedCalls("dtrans-print-callinfo",
                                               cl::ReallyHidden);

// Prints information that was stored into the DTransImmutableInfo result used
// by HIR to get likely values that may be stored within structure fields.
cl::opt<bool>
    dtrans::DTransPrintImmutableAnalyzedTypes("dtrans-print-immutable-types",
                                              cl::ReallyHidden);

// Enables identification of structure fields that are loaded but never used in
// a way that affects the program. (e.g. a field may be loaded, and not used or
// only used to compute some other value that is not used.)
static cl::opt<bool> DTransIdentifyUnusedValues("dtrans-identify-unused-values",
                                                cl::init(true),
                                                cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Use the C language compatibility rule to determine if two aggregate types
// are compatible. This is used by the analysis of AddressTaken safety checks.
// If the actual argument of a call is a pointer to an aggregate with type T,
// and there is no type U distinct from T which is compatible with T, then
// we know that the types of the formal and actual arguments must be identical.
// So, in this case, we will not need to report an AddressTaken safety check
// for a potential mismatch between formal and actual arguments.
//
cl::opt<bool> dtrans::DTransUseCRuleCompat("dtrans-usecrulecompat",
                                           cl::init(false), cl::ReallyHidden);

// Enable merging padded structures with base structures. For example,
// consider that there is a class A which will be a base class for other
// derived classes and there is an instantiation of A. Then we might see
// the following structure types in the IR:
//
// %class.A.base = type <{ %"class.boost::array", [2 x i8],
//                         %"class.std::vector", i32 }>
// %class.A = type <{ %"class.boost::array", [2 x i8],
//                         %"class.std::vector", i32, [4 x i8] }>
//
// This option enables treating both types as the same in DTrans. Any
// safety information we find in one type will be added to the other type.
static cl::opt<bool> DTransMergePaddedStructs("dtrans-merge-padded-structs",
                                              cl::init(true),
                                              cl::ReallyHidden);

StringRef dtrans::CRuleTypeKindName(CRuleTypeKind Kind) {
  switch (Kind) {
  case CRT_Unknown:
    return "Unknown";
  case CRT_False:
    return "False";
  case CRT_True:
    return "True";
  }
  llvm_unreachable("Unexpected continuation past CRuleTypeKind switch.");
}

bool dtrans::isValueConstant(const Value *Val, uint64_t *ConstValue) {
  if (!Val)
    return false;

  if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
    if (ConstValue)
      *ConstValue = ConstVal->getLimitedValue();
    return true;
  }

  return false;
}

/// This helper function checks if \p Val is a constant integer equal to
/// \p Size
bool dtrans::isValueEqualToSize(const Value *Val, uint64_t Size) {
  if (!Val)
    return false;

  uint64_t ConstSize;
  if (isValueConstant(Val, &ConstSize)) {
    return ConstSize == Size;
  }

  return false;
}

// This helper function checks a value to see if it is either (a) a constant
// whose value is a multiple of the specified size, or (b) an integer
// multiplication operator where either operand is a constant multiple of the
// specified size.
bool dtrans::isValueMultipleOfSize(const Value *Val, uint64_t Size) {
  if (!Val)
    return false;

  // If the size is zero, always return false.
  //
  // In practice, this can happen with zero-sized arrays, which could be handled
  // differently. For instance, if an allocated pointer is cast as a
  // [0 x <type>]* array, we could possibly handle this case by checking that
  // the allocation size is a multiple of the size of <type> but the
  // data layout will report that the allocation size of [0 x <type>] is
  // zero, so we'd need special handling where we call isValueMultipleOfSize
  // and in DTransOptBase::findMultipleOfSizeInst.
  //
  // The effect of returning false here is that the caller will assume that
  // the allocation is not in a form that we can optimize, so this keeps us
  // out of trouble.
  if (Size == 0)
    return false;

  // Is it a constant?
  uint64_t ConstSize;
  if (isValueConstant(Val, &ConstSize)) {
    return ((ConstSize % Size) == 0);
  }
  // Is it a mul?
  Value *LHS;
  Value *RHS;
  if (PatternMatch::match(Val,
                          PatternMatch::m_Mul(PatternMatch::m_Value(LHS),
                                              PatternMatch::m_Value(RHS)))) {
    return (isValueMultipleOfSize(LHS, Size) ||
            isValueMultipleOfSize(RHS, Size));
  } else if (PatternMatch::match(
                 Val, PatternMatch::m_Shl(PatternMatch::m_Value(LHS),
                                          PatternMatch::m_Value(RHS)))) {
    uint64_t Shift = 0;
    if (isValueConstant(RHS, &Shift))
      if (Shift < 64 && !(cast<ConstantInt>(RHS)->isNegative()))
        return (uint64_t(1) << Shift) % Size == 0;
    return false;
  }
  // Handle sext and zext
  if (isa<SExtInst>(Val) || isa<ZExtInst>(Val))
    return isValueMultipleOfSize(cast<Instruction>(Val)->getOperand(0), Size);
  // Otherwise, it's not what we needed.
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *dtrans::getSafetyDataName(const SafetyData &SafetyInfo) {
  assert(llvm::popcount(SafetyInfo) == 1 &&
         "More than one safety type detected\n");

  if (SafetyInfo & dtrans::BadCasting)
    return "Bad casting";
  if (SafetyInfo & dtrans::BadAllocSizeArg)
    return "Bad alloc size";
  if (SafetyInfo & dtrans::BadPtrManipulation)
    return "Bad pointer manipulation";
  if (SafetyInfo & dtrans::AmbiguousGEP)
    return "Ambiguous GEP";
  if (SafetyInfo & dtrans::VolatileData)
    return "Volatile data";
  if (SafetyInfo & dtrans::MismatchedElementAccess)
    return "Mismatched element access";
  if (SafetyInfo & dtrans::WholeStructureReference)
    return "Whole structure reference";
  if (SafetyInfo & dtrans::UnsafePointerStore)
    return "Unsafe pointer store";
  if (SafetyInfo & dtrans::FieldAddressTakenMemory)
    return "Field address taken memory";
  if (SafetyInfo & dtrans::GlobalPtr)
    return "Global pointer";
  if (SafetyInfo & dtrans::GlobalInstance)
    return "Global instance";
  if (SafetyInfo & dtrans::HasInitializerList)
    return "Has initializer list";
  if (SafetyInfo & dtrans::BadMemFuncSize)
    return "Bad memfunc size";
  if (SafetyInfo & dtrans::MemFuncPartialWrite)
    return "Memfunc partial write";
  if (SafetyInfo & dtrans::BadMemFuncManipulation)
    return "Bad memfunc manipulation";
  if (SafetyInfo & dtrans::AmbiguousPointerTarget)
    return "Ambiguous pointer target";
  if (SafetyInfo & dtrans::UnsafePtrMerge)
    return "Unsafe pointer merge";
  if (SafetyInfo & dtrans::AddressTaken)
    return "Address taken";
  if (SafetyInfo & NoFieldsInStruct)
    return "No fields in structure";
  if (SafetyInfo & dtrans::NestedStruct)
    return "Nested structure";
  if (SafetyInfo & dtrans::ContainsNestedStruct)
    return "Contains nested structure";
  if (SafetyInfo & dtrans::SystemObject)
    return "System object";
  if (SafetyInfo & dtrans::LocalPtr)
    return "Local pointer";
  if (SafetyInfo & dtrans::LocalInstance)
    return "Local instance";
  if (SafetyInfo & dtrans::MismatchedArgUse)
    return "Mismatched argument use";
  if (SafetyInfo & dtrans::GlobalArray)
    return "Global array";
  if (SafetyInfo & dtrans::HasVTable)
    return "Has vtable";
  if (SafetyInfo & dtrans::HasFnPtr)
    return "Has function ptr";
  if (SafetyInfo & dtrans::HasCppHandling)
    return "Has C++ handling";
  if (SafetyInfo & dtrans::HasZeroSizedArray)
    return "Has zero-sized array";
  if (SafetyInfo & dtrans::BadCastingPending)
    return "Bad casting (pending)";
  if (SafetyInfo & dtrans::BadCastingConditional)
    return "Bad casting (conditional)";
  if (SafetyInfo & dtrans::UnsafePointerStorePending)
    return "Unsafe pointer store (pending)";
  if (SafetyInfo & dtrans::UnsafePointerStoreConditional)
    return "Unsafe pointer store (conditional)";
  if (SafetyInfo & dtrans::MismatchedElementAccessPending)
    return "Mismatched element access (pending)";
  if (SafetyInfo & dtrans::MismatchedElementAccessConditional)
    return "Mismatched element access (conditional)";
  if (SafetyInfo & dtrans::DopeVector)
    return "Dope vector";
  if (SafetyInfo & dtrans::BadCastingForRelatedTypes)
    return "Bad casting (related types)";
  if (SafetyInfo & dtrans::BadPtrManipulationForRelatedTypes)
    return "Bad pointer manipulation (related types)";
  if (SafetyInfo & dtrans::MismatchedElementAccessRelatedTypes)
    return "Mismatched element access (related types)";
  if (SafetyInfo & dtrans::UnsafePointerStoreRelatedTypes)
    return "Unsafe pointer store (related types)";
  if (SafetyInfo & dtrans::MemFuncNestedStructsPartialWrite)
    return "Memfunc partial write (nested structure)";
  if (SafetyInfo & dtrans::ComplexAllocSize)
    return "Complex alloc size";
  if (SafetyInfo & dtrans::FieldAddressTakenCall)
    return "Field address taken call";
  if (SafetyInfo & dtrans::FieldAddressTakenReturn)
    return "Field address taken return";
  if (SafetyInfo & dtrans::StructCouldHaveABIPadding)
    return "Structure may have ABI padding";
  if (SafetyInfo & dtrans::StructCouldBeBaseABIPadding)
    return "Structure could be base for ABI padding";
  if (SafetyInfo & dtrans::BadMemFuncManipulationForRelatedTypes)
    return "Bad memfunc manipulation (related types)";
  if (SafetyInfo & dtrans::UnsafePtrMergeRelatedTypes)
    return "Unsafe pointer merge (related types)";
  if (SafetyInfo & dtrans::MultiStructMemFunc)
    return "Multi-struct memfunc";
  if (SafetyInfo & dtrans::UnhandledUse)
    return "Unhandled use";

  llvm_unreachable("Unknown SafetyData type");
}

static void printSafetyInfo(const SafetyData &SafetyInfo,
                            llvm::raw_ostream &ostr) {
  if (SafetyInfo == 0) {
    ostr << "No issues found\n";
    return;
  }
  // TODO: As safety checks are implemented, add them here.
  const SafetyData ImplementedMask =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP | dtrans::VolatileData |
      dtrans::MismatchedElementAccess | dtrans::WholeStructureReference |
      dtrans::UnsafePointerStore | dtrans::FieldAddressTakenMemory |
      dtrans::GlobalPtr | dtrans::GlobalInstance | dtrans::HasInitializerList |
      dtrans::BadMemFuncSize | dtrans::MemFuncPartialWrite |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::UnsafePtrMerge | dtrans::AddressTaken | dtrans::NoFieldsInStruct |
      dtrans::NestedStruct | dtrans::ContainsNestedStruct |
      dtrans::SystemObject | dtrans::LocalPtr | dtrans::LocalInstance |
      dtrans::MismatchedArgUse | dtrans::GlobalArray | dtrans::HasVTable |
      dtrans::HasFnPtr | dtrans::HasCppHandling | dtrans::HasZeroSizedArray |
      dtrans::BadCastingPending | dtrans::BadCastingConditional |
      dtrans::UnsafePointerStorePending |
      dtrans::UnsafePointerStoreConditional |
      dtrans::MismatchedElementAccessPending |
      dtrans::MismatchedElementAccessConditional |
      dtrans::DopeVector | dtrans::BadCastingForRelatedTypes |
      dtrans::BadPtrManipulationForRelatedTypes |
      dtrans::MismatchedElementAccessRelatedTypes |
      dtrans::UnsafePointerStoreRelatedTypes |
      dtrans::MemFuncNestedStructsPartialWrite | dtrans::ComplexAllocSize |
      dtrans::FieldAddressTakenCall | dtrans::FieldAddressTakenReturn |
      dtrans::StructCouldHaveABIPadding |
      dtrans::StructCouldBeBaseABIPadding |
      dtrans::BadMemFuncManipulationForRelatedTypes |
      dtrans::UnsafePtrMergeRelatedTypes |
      dtrans::MultiStructMemFunc |
      dtrans::UnhandledUse;
  // This assert is intended to catch non-unique safety condition values.
  // It needs to be kept synchronized with the statement above.
  static_assert(
      ImplementedMask ==
          (dtrans::BadCasting ^ dtrans::BadAllocSizeArg ^
           dtrans::BadPtrManipulation ^ dtrans::AmbiguousGEP ^
           dtrans::VolatileData ^ dtrans::MismatchedElementAccess ^
           dtrans::WholeStructureReference ^ dtrans::UnsafePointerStore ^
           dtrans::FieldAddressTakenMemory ^ dtrans::GlobalPtr ^
           dtrans::GlobalInstance ^ dtrans::HasInitializerList ^
           dtrans::BadMemFuncSize ^ dtrans::MemFuncPartialWrite ^
           dtrans::BadMemFuncManipulation ^ dtrans::AmbiguousPointerTarget ^
           dtrans::UnsafePtrMerge ^ dtrans::AddressTaken ^
           dtrans::NoFieldsInStruct ^ dtrans::NestedStruct ^
           dtrans::ContainsNestedStruct ^ dtrans::SystemObject ^
           dtrans::LocalPtr ^ dtrans::LocalInstance ^ dtrans::MismatchedArgUse ^
           dtrans::GlobalArray ^ dtrans::HasVTable ^ dtrans::HasFnPtr ^
           dtrans::HasCppHandling ^ dtrans::HasZeroSizedArray ^
           dtrans::BadCastingPending ^ dtrans::BadCastingConditional ^
           dtrans::UnsafePointerStorePending ^
           dtrans::UnsafePointerStoreConditional ^
           dtrans::MismatchedElementAccessPending ^
           dtrans::MismatchedElementAccessConditional ^
           dtrans::DopeVector ^ dtrans::BadCastingForRelatedTypes ^
           dtrans::BadPtrManipulationForRelatedTypes ^
           dtrans::MismatchedElementAccessRelatedTypes ^
           dtrans::UnsafePointerStoreRelatedTypes ^
           dtrans::MemFuncNestedStructsPartialWrite ^ dtrans::ComplexAllocSize ^
           dtrans::FieldAddressTakenCall ^ dtrans::FieldAddressTakenReturn ^
           dtrans::StructCouldHaveABIPadding ^
           dtrans::StructCouldBeBaseABIPadding ^
           dtrans::BadMemFuncManipulationForRelatedTypes ^
           dtrans::UnsafePtrMergeRelatedTypes ^
           dtrans::MultiStructMemFunc ^
           dtrans::UnhandledUse),
      "Duplicate value used in dtrans safety conditions");

  // Go through the issues in the order of LSB to MSB, and print the names of
  // the SafetyData bits that are set.
  SafetyData TmpInfo = SafetyInfo;
  SafetyData Bit = 1;
  bool First = true;
  while (TmpInfo) {
    if (SafetyInfo & Bit) {
      if (!First)
        ostr << " | ";
      ostr << getSafetyDataName(Bit);
      First = false;
    }
    Bit <<= 1;
    TmpInfo >>= 1;
  }

  // TODO: Make this unnecessary.
  if (SafetyInfo & ~ImplementedMask) {
    ostr << " + other issues that need format support ("
         << (SafetyInfo & ~ImplementedMask) << ")";
    ostr << "\nImplementedMask = " << ImplementedMask;
  }

  ostr << "\n";
}

void dtrans::TypeInfo::printSafetyData(raw_ostream &OS) const {
  OS << "  Safety data: ";
  printSafetyInfo(SafetyInfo, OS);
}

void dtrans::StructInfo::print(
    raw_ostream &OS,
    std::function<void(raw_ostream &OS, const StructInfo *)> *Annotator) const {
  llvm::StructType *S = cast<llvm::StructType>(getLLVMType());
  OS << "DTRANS_StructInfo:\n";
  OS << "  LLVMType: " << *S << "\n";
  if (S->hasName())
    OS << "  Name: " << S->getName() << "\n";
  if (getCRuleTypeKind() != dtrans::CRT_Unknown) {
    OS << "  CRuleTypeKind: ";
    OS << dtrans::CRuleTypeKindName(getCRuleTypeKind()) << "\n";
  }
  if (Annotator)
    (*Annotator)(OS, this);

  if (auto *RelatedInfo =
          dyn_cast_or_null<dtrans::StructInfo>(getRelatedType())) {
    llvm::StructType *RelatedType =
        cast<llvm::StructType>(RelatedInfo->getLLVMType());
    if (S->getName().endswith(".base"))
      OS << "  Related padded structure: ";
    else
      OS << "  Related base structure: ";

    OS << RelatedType->getName() << "\n";
  }

  OS << "  Number of fields: " << getNumFields() << "\n";
  unsigned Number = 0;
  for (auto &Field : getFields()) {
    OS << format_decimal(Number++, 3) << ")";
    Field.print(OS, getIgnoredFor());
  }
  OS << "  Total Frequency: " << getTotalFrequency() << "\n";
  auto &CG = getCallSubGraph();
  OS << "  Call graph: "
     << (CG.isBottom() ? "bottom\n" : (CG.isTop() ? "top\n" : ""));
  if (!CG.isBottom() && !CG.isTop()) {
    OS << "enclosing type: " << CG.getEnclosingType()->getName() << "\n";
  }
  printSafetyData(OS);
  OS << "  End LLVMType: " << *S << "\n";
  OS << "\n";
}

void dtrans::ArrayInfo::print(raw_ostream &OS) const {
  OS << "DTRANS_ArrayInfo:\n";
  OS << "  LLVMType: " << *getLLVMType() << "\n";
  if (getCRuleTypeKind() != dtrans::CRT_Unknown) {
    OS << "  CRuleTypeKind: ";
    OS << dtrans::CRuleTypeKindName(getCRuleTypeKind()) << "\n";
  }
  OS << "  Number of elements: " << getNumElements() << "\n";
  OS << "  Element LLVM Type: " << *getElementLLVMType() << "\n";
  if (DTransElemTy->isDTransType())
    OS << "  Element DTrans Type: " << *DTransElemTy->getDTransType() << "\n";
  printSafetyData(OS);
  OS << "  End LLVMType: " << *getLLVMType() << "\n";
  OS << "\n";
}

void dtrans::FieldInfo::print(raw_ostream &OS,
                              dtrans::Transform IgnoredInTransform) const {
  OS << "Field LLVM Type: " << *getLLVMType() << "\n";
  if (isDTransType())
    OS << "    DTrans Type: " << *getDTransType() << "\n";
  OS << "    Field info:";

  if (isRead())
    OS << " Read";

  if (isWritten())
    OS << " Written";

  if (isValueUnused())
    OS << " UnusedValue";

  if (hasComplexUse())
    OS << " ComplexUse";

  if (isAddressTaken())
    OS << " AddressTaken";

  if (isMismatchedElementAccess())
    OS << " MismatchedElementAccess";

  if (hasNonGEPAccess())
    OS << " NonGEPAccess";

  if (isPaddedField())
    OS << (isCleanPaddedField() ? "" : " Dirty") << " PaddedField";

  OS << "\n";
  OS << "    Frequency: " << getFrequency();
  OS << "\n";

  if (isNoValue())
    OS << "    No Value";
  else if (isSingleValue()) {
    OS << "    Single Value: ";
    getSingleValue()->printAsOperand(OS);
  } else if (isMultipleValue()) {
    OS << "    Multiple Value: [ ";
    dtrans::printCollectionSorted(OS, values().begin(), values().end(), ", ",
                                  [](llvm::Constant *C) {
                                    std::string OutputVal;
                                    raw_string_ostream OutputStream(OutputVal);
                                    C->printAsOperand(OutputStream, false);
                                    OutputStream.flush();
                                    return OutputVal;
                                  });
    OS << " ] <" << (isValueSetComplete() ? "complete" : "incomplete") << ">";
  }
  if (IgnoredInTransform & dtrans::DT_FieldSingleValue)
    OS << " (ignored)";
  OS << "\n";

  if (isNoIAValue())
    OS << "    No IA Value";
  else if (isSingleIAValue()) {
    OS << "    Single IA Value: ";
    getSingleValue()->printAsOperand(OS);
  } else {
    assert(isMultipleIAValue() && "Expecting multiple value");
    OS << "    Multiple IA Value: [ ";
    dtrans::printCollectionSorted(OS, iavalues().begin(), iavalues().end(),
                                  ", ", [](llvm::Constant *C) {
                                    std::string OutputVal;
                                    raw_string_ostream OutputStream(OutputVal);
                                    C->printAsOperand(OutputStream, false);
                                    OutputStream.flush();
                                    return OutputVal;
                                  });
    OS << " ] <" << (isIAValueSetComplete() ? "complete" : "incomplete") << ">";
  }
  OS << "\n";

  if (isArrayWithConstantEntries()) {
    OS << "    Array with constant entries: [ ";
    dtrans::printCollectionSorted(OS, getArrayWithConstantEntries().begin(),
                                  getArrayWithConstantEntries().end(), ", ",
                                  [](std::pair<Constant *, Constant*>
                                      Pair) {
                                    std::string OutputVal;
                                    raw_string_ostream OutputStream(OutputVal);
                                    OutputStream << "Index: ";
                                    Pair.first->printAsOperand(OutputStream,
                                                               false);
                                    OutputStream << "  Constant: ";
                                    Pair.second->printAsOperand(OutputStream,
                                                                false);
                                    OutputStream.flush();
                                    return OutputVal;
                                  });
    OS << " ] \n";
  }

  if (isTopAllocFunction())
    OS << "    Top Alloc Function";
  else if (isSingleAllocFunction()) {
    OS << "    Single Alloc Function: ";
    getSingleAllocFunction()->printAsOperand(OS);
  } else if (isBottomAllocFunction())
    OS << "    Bottom Alloc Function";
  if (IgnoredInTransform & dtrans::DT_FieldSingleAllocFunction)
    OS << " (ignored)";
  OS << "\n";
  OS << "    Readers:" << (readers().empty() ? "" : " ");
  dtrans::printCollectionSorted(OS, readers().begin(), readers().end(), ", ",
                                [](const Function *F) { return F->getName(); });
  OS << "\n";
  OS << "    Writers:" << (writers().empty() ? "" : " ");
  dtrans::printCollectionSorted(OS, writers().begin(), writers().end(), ", ",
                                [](const Function *F) { return F->getName(); });
  OS << "\n";
  OS << "    RWState: "
     << (isRWBottom() ? "bottom" : (isRWComputed() ? "computed" : "top"))
     << "\n";
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void dtrans::TypeInfo::mergeSafetyDataWithRelatedType() {
  dtrans::StructInfo *CurrStrInfo = dyn_cast<dtrans::StructInfo>(this);
  if (!CurrStrInfo)
    return;

  dtrans::StructInfo *RelatedInfo = CurrStrInfo->getRelatedType();
  if (RelatedInfo)
    RelatedInfo->setSafetyData(SafetyInfo);
}

void dtrans::TypeInfo::setSafetyData(SafetyData Conditions) {
  SafetyInfo |= Conditions;

  LLVM_DEBUG(dbgs() << "dtrans-safety-detail: " << *getLLVMType() << " :: ");
  LLVM_DEBUG(printSafetyInfo(Conditions, dbgs()));
}

bool dtrans::FieldInfo::processNewSingleValue(llvm::Constant *C) {
  if (!C)
    return false;
  return ConstantValues.insert(C);
}

bool dtrans::FieldInfo::processNewSingleIAValue(llvm::Constant *C) {
  if (!C)
    return false;
  return ConstantIAValues.insert(C);
}

bool dtrans::FieldInfo::processNewSingleAllocFunction(llvm::Function *F) {
  if (isTopAllocFunction()) {
    if (F == nullptr)
      setBottomAllocFunction();
    else
      setSingleAllocFunction(F);
    return true;
  }
  if (isSingleAllocFunction() && getSingleAllocFunction() != F) {
    setBottomAllocFunction();
    return true;
  }
  return false;
}

// Insert a new entry in ArrayConstEntries assuming that the current field
// is an array with constant entries. Index is the entry in the array that
// is constant and ConstVal is the constant value for that index.
void dtrans::FieldInfo::addConstantEntryIntoTheArray(Constant *Index,
                                                     Constant* ConstVal) {

  assert ((Index && ConstVal && isa<ConstantInt>(Index) &&
           isa<ConstantInt>(ConstVal)) && "Inserting non-constant information "
                                          "into a space reserved for constants "
                                          "integers");
  assert (!(cast<ConstantInt>(Index)->isNegative()) &&
          "Accessing array with negative index");

  ArrayConstEntries.insert({Index, ConstVal});
}

// Insert a new entry in ArrayConstEntries assuming that the current field
// is an array with constant entries. Index is the entry in the array that
// is constant and ConstVal is the constant value for that index. If Index
// is in the map then replace the value with nullptr. Also, nullptr for
// ConstVal is allowed. It means that there is no constant for the index
// accessed.
void dtrans::FieldInfo::addNewArrayConstantEntry(Constant *Index,
                                                 Constant* ConstVal) {
  if (!canAddConstantEntriesForArray)
    return;

  // Index is needed and must be a constant integer
  if (!Index) {
    disableArraysWithConstantEntries();
    return;
  }

  // If canAddConstantEntriesForArray is true then it means that the current
  // field is an array of integers
  llvm::ArrayType *CurrArr = dyn_cast<llvm::ArrayType>(getLLVMType());
  if (!CurrArr) {
    llvm::StructType *SpecialArr = dyn_cast<llvm::StructType>(getLLVMType());
    if (!SpecialArr || SpecialArr->getNumElements() != 1) {
      disableArraysWithConstantEntries();
      return;
    }

    CurrArr = dyn_cast<llvm::ArrayType>(SpecialArr->getElementType(0));
    if (!CurrArr) {
      disableArraysWithConstantEntries();
      return;
    }
  }

  auto *ConstIntIndex = dyn_cast<ConstantInt>(Index);
  auto *ConstIntVal = dyn_cast_or_null<ConstantInt>(ConstVal);
  Constant *ConstValMap = nullptr;
  if (!ConstIntIndex) {
    disableArraysWithConstantEntries();
    return;
  }

  // A negative index means an out of bounds access, disable array
  // with constant
  if (ConstIntIndex->isNegative()) {
    disableArraysWithConstantEntries();
    return;
  }

  // The index must be within the bounds, else disable the arrays with constant
  if (ConstIntIndex->getZExtValue() >= CurrArr->getNumElements()) {
    disableArraysWithConstantEntries();
    return;
  }

  // If there is a value then check if the types match
  if (ConstIntVal) {
    llvm::Type *ElemType = CurrArr->getElementType();
    if (ElemType != ConstIntVal->getType()) {
      disableArraysWithConstantEntries();
      return;
    }
    ConstValMap = ConstIntVal;
  }

  // If the entry is not in the map then add it. Else, set it to nullptr if
  // the value is not the same. If ConstVal is nullptr and not in the map,
  // then it means that we are adding a non-constant integer for the input
  // index.
  auto It = ArrayWithConstEntriesMap.find(Index);
  if (It == ArrayWithConstEntriesMap.end())
    ArrayWithConstEntriesMap.insert({Index, ConstValMap});
  else if (It->second != ConstValMap)
    It->second = nullptr;
}

// Return true if the current field is an array with constant entries. Else
// return false.
bool dtrans::FieldInfo::isFieldAnArrayWithConstEntries() {
  if (!canAddConstantEntriesForArray)
    return false;

  if (ArrayWithConstEntriesMap.empty())
    return false;

  // If all entries are nullptr then we don't have any information
  for (const auto &Pair : ArrayWithConstEntriesMap)
    if (Pair.second)
      return true;

  return false;
}

// This is a helper function used to break the relationship between
// a base and a padded structure.
void StructInfo::unsetRelatedType() {
  if (!RelatedType)
    return;

  // Clear the related type
  StructInfo *CurrRelated = RelatedType;
  RelatedType = nullptr;

  // Clear the padded field
  size_t NumFields = getNumFields();
  FieldInfo &LastField = getField(NumFields - 1);
  if (LastField.isPaddedField())
    LastField.clearPaddedField();

  RTForm = RT_BOTTOM;

  CurrRelated->unsetRelatedType();
}

// Return true if the last field in the structure is used for padding.
bool StructInfo::hasPaddedField() {
  if (!getRelatedType())
    return false;

  int64_t LastField = getNumFields() - 1;
  dtrans::FieldInfo &Field = getField(LastField);

  return Field.isPaddedField();
}

// Set current structure as a base structure for ABI padding (RT_BASE) if it
// isn't set (RT_TOP). If it is set and is not RT_BASE, it will set the
// structure as RT_BOTTOM.
void StructInfo::setAsABIPaddingBaseStructure() {
  if (RTForm == RT_BASE)
    return;

  // If not top then something happened that broke the relationship
  if (RTForm != RT_TOP) {
    RTForm = RT_BOTTOM;
    return;
  }

  if (!RelatedType)
    return;

  // Related type is already set, make sure that the current one is the base
  // by checking the field size
  size_t NumFields = getNumFields();
  size_t RelatedNumFields = RelatedType->getNumFields();
  if (RelatedNumFields - NumFields != 1) {
    RTForm = RT_BOTTOM;
    return;
  }

  RTForm = RT_BASE;
}

// Set current structure as a padded structure for ABI padding (RT_PADDED) if
// it isn't set (RT_TOP). If it is set and is not RT_PADDED, it will set the
// structure as RT_BOTTOM.
void StructInfo::setAsABIPaddingPaddedStructure() {
  if (RTForm == RT_PADDED)
    return;

  // If not top then something happened that broke the relationship
  if (RTForm != RT_TOP) {
    RTForm = RT_BOTTOM;
    return;
  }

  if (!RelatedType)
    return;

  // Related type is already set, make sure that the current one is the padded
  // by checking the field size
  size_t NumFields = getNumFields();
  size_t RelatedNumFields = RelatedType->getNumFields();
  if (NumFields - RelatedNumFields != 1) {
    RTForm = RT_BOTTOM;
    return;
  }

  RTForm = RT_PADDED;
}

void StructInfo::updateNewSingleAllocFunc(unsigned FieldNum,
                                          Function &Callee) {
  dtrans::FieldInfo &FI = getField(FieldNum);
  if (!FI.processNewSingleAllocFunction(&Callee))
    return;
  DEBUG_WITH_TYPE(SAFETY_FSAF, {
    dbgs() << "dtrans-fsaf: " << *(getLLVMType()) << " [" << FieldNum << "] ";
    if (FI.isSingleAllocFunction())
      Callee.printAsOperand(dbgs());
    else
      dbgs() << "<BOTTOM>";
    dbgs() << "\n";
  });
}

void StructInfo::updateSingleAllocFuncToBottom(unsigned FieldNum) {
  dtrans::FieldInfo &FI = getField(FieldNum);
  DEBUG_WITH_TYPE(SAFETY_FSAF, {
    if (!FI.isBottomAllocFunction())
      dbgs() << "dtrans-fsaf: " << *(getLLVMType()) << " [" << FieldNum << "] <BOTTOM>\n";
  });
  FI.setBottomAllocFunction();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void CallInfoElementTypes::dump() { print(dbgs()); }

void CallInfoElementTypes::print(raw_ostream &OS) {
  if (!getAnalyzed()) {
    OS << "    Type: Not analyzed\n";
    return;
  }

  if (!getAliasesToAggregateType()) {
    OS << "    Type: Non-aggregate\n";
    return;
  }

  // Put the type names in a vector so that we can output
  // it in sorted order to enable consistency for testing.
  std::vector<std::string> StrVec;

  for (auto &AT : ElemTypes) {
    std::string Name;
    raw_string_ostream(Name) << "    Type: " << AT;
    StrVec.push_back(Name);
  }

  std::sort(StrVec.begin(), StrVec.end());
  for (auto &S : StrVec)
    OS << S << "\n";
}

void CallInfo::dump() { print(dbgs()); }

/// Dispatcher to invoke the appropriate dump method based on the specific type
/// of call being tracked.
void CallInfo::print(raw_ostream &OS) {
  switch (getCallInfoKind()) {
  case CIK_Alloc:
    cast<AllocCallInfo>(this)->print(OS);
    break;
  case CIK_Free:
    cast<FreeCallInfo>(this)->print(OS);
    break;
  case CIK_Memfunc:
    cast<MemfuncCallInfo>(this)->print(OS);
    break;
  }
}

void AllocCallInfo::dump() { print(dbgs()); }

void AllocCallInfo::print(raw_ostream &OS) {
  OS << "AllocCallInfo:\n";
  OS << "  Kind: " << AllocKindName(AK) << "\n";
  OS << "  Aliased types:\n";
  ElementTypes.print(OS);
}

void FreeCallInfo::dump() { print(dbgs()); }

void FreeCallInfo::print(raw_ostream &OS) {
  OS << "FreeCallInfo:\n";
  OS << "  Kind: " << FreeKindName(FK) << "\n";
  OS << "  Aliased types:\n";
  ElementTypes.print(OS);
}

void MemfuncCallInfo::dump() { print(dbgs()); }

void MemfuncCallInfo::print(raw_ostream &OS) {
  OS << "MemfuncInfo:\n";
  OS << "    Kind: " << MemfuncKindName(MK) << "\n";

  unsigned int NumRegions = getNumRegions();
  for (unsigned int RN = 0; RN < NumRegions; ++RN) {
    bool IsComplete = getIsCompleteAggregate(RN);
    OS << "  Region " << RN << ":\n";
    OS << "    Complete: " << (IsComplete ? "true" : "false") << "\n";
    if (!IsComplete) {
      OS << "    PrePad:     " << getPrePadBytes(RN) << "\n";
      OS << "    FirstField: " << getFirstField(RN) << "\n";
      OS << "    LastField:  " << getLastField(RN) << "\n";
      OS << "    PostPad:    " << getPostPadBytes(RN) << "\n";
    }

    ElementTypes.print(OS);
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

dtrans::CallInfo *
CallInfoManager::getCallInfo(const llvm::Instruction *I) const {
  auto CInfoVec = getCallInfoVec(I);
  if (!CInfoVec)
    return nullptr;
  assert(CInfoVec->size() == 1 && "Expecting single element call info");
  return (*CInfoVec)[0];
}

const dtrans::CallInfoVec *
CallInfoManager::getCallInfoVec(const llvm::Instruction *I) const {
  auto Entry = CallInfoMap.find(I);
  if (Entry == CallInfoMap.end())
    return nullptr;
  return &Entry->second;
}

dtrans::CallInfoVec *
CallInfoManager::getCallInfoVec(const llvm::Instruction *I) {
  auto Entry = CallInfoMap.find(I);
  if (Entry == CallInfoMap.end())
    return nullptr;
  return &Entry->second;
}

void CallInfoManager::addCallInfo(Instruction *I, dtrans::CallInfo *CI) {
  CallInfoMap[I].push_back(CI);
}

dtrans::AllocCallInfo *
CallInfoManager::createAllocCallInfo(Instruction *I, dtrans::AllocKind AK) {
  dtrans::AllocCallInfo *Info = new dtrans::AllocCallInfo(I, AK);
  addCallInfo(I, Info);
  return Info;
}

dtrans::FreeCallInfo *CallInfoManager::createFreeCallInfo(Instruction *I,
                                                          dtrans::FreeKind FK) {
  dtrans::FreeCallInfo *Info = new dtrans::FreeCallInfo(I, FK);
  addCallInfo(I, Info);
  return Info;
}

dtrans::MemfuncCallInfo *
CallInfoManager::createMemfuncCallInfo(Instruction *I,
                                       dtrans::MemfuncCallInfo::MemfuncKind MK,
                                       const dtrans::MemfuncRegion &MR) {
  dtrans::MemfuncCallInfo *Info = new dtrans::MemfuncCallInfo(I, MK, MR);
  addCallInfo(I, Info);
  return Info;
}

dtrans::MemfuncCallInfo *CallInfoManager::createMemfuncCallInfo(
    Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
    const dtrans::MemfuncRegion &MR1, const dtrans::MemfuncRegion &MR2) {
  dtrans::MemfuncCallInfo *Info = new dtrans::MemfuncCallInfo(I, MK, MR1, MR2);
  addCallInfo(I, Info);
  return Info;
}

void CallInfoManager::deleteCallInfo(Instruction *I) {
  auto InfoVec = getCallInfoVec(I);
  if (!InfoVec)
    return;
  assert(InfoVec->size() == 1 &&  "Expecting single element CallInfoVec");
  auto CI = InfoVec->pop_back_val();
  delete CI;
  CallInfoMap.erase(I);
}

void CallInfoManager::deleteCallInfoVec(Instruction *I) {
  auto InfoVec = getCallInfoVec(I);
  if (!InfoVec)
    return;
  while (!InfoVec->empty()) {
    auto CI = InfoVec->pop_back_val();
    delete CI;
  }
  CallInfoMap.erase(I);
}

void CallInfoManager::replaceCallInfoInstruction(dtrans::CallInfo *Info,
                                                 Instruction *NewI) {
  Instruction *OldI = Info->getInstruction();
  auto CInfoVec = CallInfoMap[OldI];
  auto It = std::find(CInfoVec.begin(), CInfoVec.end(), Info);
  if (It == CInfoVec.end())
    return;
  CInfoVec.erase(It);
  if (CInfoVec.empty())
    CallInfoMap.erase(OldI);
  addCallInfo(NewI, Info);
  Info->setInstruction(NewI);
}

void CallInfoManager::reset() {
  for (const auto &Info : CallInfoMap)
    for (const auto &CallInfo : Info.second)
      destructCallInfo(CallInfo);
  CallInfoMap.clear();
}

// Helper to invoke the right destructor for destroying a CallInfo type object.
void CallInfoManager::destructCallInfo(dtrans::CallInfo *Info) {
  if (!Info)
    return;

  switch (Info->getCallInfoKind()) {
  case dtrans::CallInfo::CIK_Alloc:
    delete cast<dtrans::AllocCallInfo>(Info);
    break;
  case dtrans::CallInfo::CIK_Free:
    delete cast<dtrans::FreeCallInfo>(Info);
    break;
  case dtrans::CallInfo::CIK_Memfunc:
    delete cast<dtrans::MemfuncCallInfo>(Info);
    break;
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Returns StringRef with the name of the transformation
StringRef dtrans::getStringForTransform(dtrans::Transform Trans) {
  if (Trans == 0 || Trans & ~dtrans::DT_Legal)
    return "";

  switch (Trans) {
  case dtrans::DT_FieldSingleValue:
    return "fsv";
  case dtrans::DT_FieldSingleAllocFunction:
    return "fsaf";
  case dtrans::DT_ReuseField:
    return "reusefield";
  case dtrans::DT_ReuseFieldPtrOfPtr:
    return "reusefieldptrofptr";
  case dtrans::DT_ReuseFieldPtr:
    return "reusefieldptr";
  case dtrans::DT_DeleteField:
    return "deletefield";
  case dtrans::DT_ReorderFields:
    return "reorderfields";
  case dtrans::DT_AOSToSOA:
    return "aostosoa";
  case dtrans::DT_AOSToSOADependent:
    return "aostosoadependent";
  case dtrans::DT_AOSToSOADependentIndex32:
    return "aostosoadependentindex32";
  case dtrans::DT_ElimROFieldAccess:
    return "elimrofieldaccess";
  case dtrans::DT_DynClone:
    return "dynclone";
  case dtrans::DT_SOAToAOS:
    return "soatoaos";
  case dtrans::DT_MemInitTrimDown:
    return "meminittrimdown";
  case dtrans::DT_ArraysWithConstantEntries:
    return "arrayswithconstantentries";
  }
  llvm_unreachable("Unexpected continuation past dtrans::Transform switch.");
  return "";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Returns safety conditions for the transformation
dtrans::SafetyData dtrans::getConditionsForTransform(dtrans::Transform Trans,
                                                     bool DTransOutOfBoundsOK) {
  if (Trans == 0 || Trans & ~dtrans::DT_Legal)
    return dtrans::NoIssues;

  switch (Trans) {
  // In the cases of FSV and FSAF, if DTransOutOfBoundsOK is false we exclude
  // FieldAddressTaken from the general safety checks and check each field
  // individually.
  case dtrans::DT_FieldSingleValue:
    return DTransOutOfBoundsOK ? dtrans::SDFieldSingleValue
                               : dtrans::SDFieldSingleValueNoFieldAddressTaken;
  case dtrans::DT_FieldSingleAllocFunction:
    return DTransOutOfBoundsOK
               ? dtrans::SDSingleAllocFunction
               : dtrans::SDSingleAllocFunctionNoFieldAddressTaken;
  case dtrans::DT_ReuseField:
    return dtrans::SDReuseField;
  case dtrans::DT_ReuseFieldPtrOfPtr:
    return dtrans::SDReuseFieldPtrOfPtr;
  case dtrans::DT_ReuseFieldPtr:
    return dtrans::SDReuseFieldPtr;
  case dtrans::DT_DeleteField:
    return dtrans::SDDeleteField;
  case dtrans::DT_ReorderFields:
    return dtrans::SDReorderFields;
  case dtrans::DT_ReorderFieldsDependent:
    return dtrans::SDReorderFieldsDependent;
  case dtrans::DT_AOSToSOA:
    return dtrans::SDAOSToSOA;
  case dtrans::DT_AOSToSOADependent:
    return dtrans::SDAOSToSOADependent;
  case dtrans::DT_AOSToSOADependentIndex32:
    return dtrans::SDAOSToSOADependentIndex32;
  case dtrans::DT_ElimROFieldAccess:
    return dtrans::SDElimROFieldAccess;
  case dtrans::DT_DynClone:
    return dtrans::SDDynClone;
  case dtrans::DT_SOAToAOS:
    return dtrans::SDSOAToAOS;
  case dtrans::DT_MemInitTrimDown:
    return dtrans::SDMemInitTrimDown;
  case dtrans::DT_ArraysWithConstantEntries:
    return dtrans::SDArraysWithConstantEntries;
  }
  llvm_unreachable("Unexpected continuation past dtrans::Transform switch.");
  return dtrans::NoIssues;
}

// Helper method for getting a name to print for structures in debug traces.
StringRef dtrans::getStructName(llvm::Type *Ty) {
  auto *StructTy = dyn_cast<llvm::StructType>(Ty);
  assert(StructTy && "Expected structure type");
  return StructTy->hasName() ? StructTy->getStructName() : "<unnamed struct>";
}

// Check that function only throws an exception.
bool dtrans::isDummyFuncWithUnreachable(const CallBase *Call,
                                        const TargetLibraryInfo &TLI) {

  // Returns true if "PtrOp" is accessing memory allocated by "AllocI".
  // Ex:
  // %AllocI = alloca %"bad_alloc", align 8
  // %4 = getelementptr %"bad_alloc", %"bad_alloc"* %AllocI, i64 0, i32 0
  // %5 = getelementptr inbounds %"exception", %"exception"* %4, i64 0, i32 1
  // %6 = bitcast %__std_exception_data* %5 to i8*
  // %7 = getelementptr inbounds i8, i8* %6, i64 8
  // %PtrOp = bitcast i8* %7 to i64*
  // store i64 0, i64* %PtrOp
  std::function<bool(Value *, AllocaInst *)> IsPtrOpReferencesAllocaMem;
  IsPtrOpReferencesAllocaMem =
      [&IsPtrOpReferencesAllocaMem](Value *PtrOp, AllocaInst *AllocI) {
        if (PtrOp == AllocI)
          return true;
        Value *Op = PtrOp;
        if (auto BC = dyn_cast<BitCastInst>(Op))
          Op = BC->getOperand(0);
        if (auto GEPI = dyn_cast<GetElementPtrInst>(Op))
          return IsPtrOpReferencesAllocaMem(GEPI->getPointerOperand(), AllocI);
        return false;
      };

  // Returns true if "BB" just calls _CxxThrowException function (Windows EH).
  // It allows store instructions that save data to std::bad_alloc object.
  //
  // entry:
  // %3 = alloca %"bad_alloc", align 8
  // %g1 = getelementptr %"bad_alloc", %"bad_alloc"* %3, i64 0, i32 0
  // %4 = getelementptr %"exception", %"exception"* %g1, i64 0, i32 1
  // %5 = bitcast i8* %4 to i64*
  // store i64 0, i64* %5, align 8
  // %6 = getelementptr %"bad_alloc", %"bad_alloc"* %3, i64 0, i32 0, i32 1
  // store i8* some_const, i8** %6
  // %7 = getelementptr %"bad_alloc", %"bad_alloc"* %3, i64 0, i32 0, i32 0
  // store i32 some_const, i32 (...)*** %7
  // %8 = bitcast %"bad_alloc"* %3 to i8*
  // call void @_CxxThrowException(i8* nonnull %8, ...)
  // unreachable
  //
  auto DummyAllocBBWithCxxThrowException = [&](BasicBlock &BB) {
    auto CI = dyn_cast_or_null<CallInst>(
        BB.getTerminator()->getPrevNonDebugInstruction());
    if (!CI)
      return false;
    auto *Func = dtrans::getCalledFunction(*CI);
    if (!Func)
      return false;
    LibFunc LFunc;
    if (!TLI.getLibFunc(*Func, LFunc) || !TLI.has(LFunc) ||
        LFunc != LibFunc_msvc_std_CxxThrowException)
      return false;

    // Get std::bad_alloc object pointer from _CxxThrowException call.
    Value *EhArg = CI->getArgOperand(0);
    if (auto BC = dyn_cast<BitCastInst>(EhArg))
      EhArg = BC->getOperand(0);
    auto AI = dyn_cast<AllocaInst>(EhArg);
    if (!AI)
      return false;
    // Makes sure BB doesn't have any instruction that has side effects
    // except stores to std::bad_alloc object.
    for (auto &I : BB) {
      if (&I == CI)
        continue;
      if (auto SI = dyn_cast<StoreInst>(&I)) {
        if (!isa<Constant>(SI->getValueOperand()))
          return false;
        if (!IsPtrOpReferencesAllocaMem(SI->getPointerOperand(), AI))
          return false;
        continue;
      }
      if (I.mayReadOrWriteMemory())
        return false;
    }
    return true;
  };

  // Returns true if "BB" just calls _cxa_throw function (Linux EH).
  //
  // entry:
  //  %3 = tail call i8* @__cxa_allocate_exception(i64 8)
  //  (optional) %4 = bitcast i8* %3 to %"bad_alloc"*
  //  %5 = getelementptr %"bad_alloc", %"bad_alloc"* %4, i64 0, i32 0, i32 0
  //  store i32 some_const, i32 (...)*** %5, align 8, !tbaa !14619
  //  call void @__cxa_throw() #62
  //  unreachable
  //
  auto DummyAllocBBWithCxaThrow = [&](BasicBlock &BB) {
    // In dummy function we expect to see only those instructions which throw
    // bad_alloc exception.
    bool CallExAllocFound = false, StoreFound = false;
    bool GEPFound = true, CallExThrowFound = false;
    for (auto &I : BB) {
      if (isa<BitCastInst>(&I))
        continue;
      if (isa<GetElementPtrInst>(&I))
        GEPFound = true;
      auto *Call = dyn_cast<CallInst>(&I);
      if (Call) {
        // Skip debug intrinsics
        if (isa<DbgInfoIntrinsic>(&I))
          continue;
        auto *Func = dtrans::getCalledFunction(*Call);
        if (!Func)
          return false;

        LibFunc LFunc;
        if (!TLI.getLibFunc(*Func, LFunc) || !TLI.has(LFunc))
          return false;

        if (LFunc == LibFunc_cxa_allocate_exception)
          CallExAllocFound = true;
        else if (LFunc == LibFunc_cxa_throw)
          CallExThrowFound = true;
        else
          return false;
      }
      if (isa<StoreInst>(&I)) {
        if (!StoreFound && CallExAllocFound)
          StoreFound = true;
        else
          return false;
      }
    }
    return CallExAllocFound && StoreFound && GEPFound &&
           CallExThrowFound;
  };

  auto *F = dtrans::getCalledFunction(*Call);
  if (!F)
    return false;
  if (F->size() != 1)
    return false;
  auto &BB = F->getEntryBlock();
  if (!isa<UnreachableInst>(BB.getTerminator()))
    return false;
  // Makes sure arguments of "F" are not used.
  for (Argument &Arg : F->args())
    if (!Arg.use_empty())
      return false;

  if (!DummyAllocBBWithCxaThrow(BB) && !DummyAllocBBWithCxxThrowException(BB))
    return false;
  return true;
}

bool dtrans::hasPointerType(llvm::Type *Ty) {
  if (Ty->isPointerTy())
    return true;

  if (Ty->isArrayTy())
    return hasPointerType(cast<ArrayType>(Ty)->getElementType());
  if (Ty->isVectorTy())
    return hasPointerType(cast<VectorType>(Ty)->getElementType());

  if (auto *StTy = dyn_cast<StructType>(Ty)) {
    // Check inside of literal structs because those cannot be referenced by
    // name. However, there is no need to look inside non-literal structures
    // because those will be referenced by their name.
    if (StTy->isLiteral())
      for (auto *ElemTy : StTy->elements()) {
        bool HasPointer = hasPointerType(ElemTy);
        if (HasPointer)
          return true;
      }
  }

  if (auto *FnTy = dyn_cast<FunctionType>(Ty)) {
    // Check the return type and the parameter types for any possible
    // pointer because metadata descriptions on these will be used to help
    // recovery of opaque pointer types.
    Type *RetTy = FnTy->getReturnType();
    if (hasPointerType(RetTy))
      return true;

    unsigned NumParams = FnTy->getNumParams();
    for (unsigned Idx = 0; Idx < NumParams; ++Idx) {
      Type *ParmTy = FnTy->getParamType(Idx);
      if (hasPointerType(ParmTy))
        return true;
    }
  }

  return false;
}

// Helper function that checks if at least one field in the structure StTy
// is an opaque pointer type, or if it contains a reference to an opaque
// pointer.
bool dtrans::hasOpaquePointerFields(llvm::StructType *StTy) {
  assert(StTy && "Trying to access the information in a null type");

  // Return true if the input type is an opaque pointer
  std::function<bool(llvm::Type *, llvm::SetVector<Type *>&)>
      hasOpaquePointerType = [&hasOpaquePointerType](llvm::Type *Ty,
      llvm::SetVector<Type *> &Visited) -> bool {

    // If the type was visited then return false, there is nothing to check
    if (!Visited.insert(Ty))
      return false;

    if (Ty->isPointerTy())
      return true;

    // If the input type is an array or a vector, then check the element
    else if (Ty->isArrayTy())
      return hasOpaquePointerType(cast<ArrayType>(Ty)->getElementType(),
                                  Visited);

    else if (Ty->isVectorTy())
      return hasOpaquePointerType(cast<VectorType>(Ty)->getElementType(),
                                  Visited);

    // If the input type is a structure, then check the fields
    else if (auto *StTy = dyn_cast<StructType>(Ty))
      for (auto *ElemTy : StTy->elements())
        if(hasOpaquePointerType(ElemTy, Visited))
          return true;

    // No need to check for function pointers because they are pointers
    return false;
  };

  // No need to call the recursion when there are no elements
  if (StTy->isOpaque() || StTy->getNumElements() == 0)
    return false;

  SetVector<Type *> Visited;
  return hasOpaquePointerType(cast<Type>(StTy), Visited);
}

// Return 'true' if the value is only used as the destination pointer of memset
// calls.
bool dtrans::valueOnlyUsedForMemset(Value *V) {
  if (V->users().empty())
    return false;

  for (auto *U : V->users()) {
    if (auto *MC = dyn_cast<MemSetInst>(U))
      if (MC->getDest() == V)
        continue;
    return false;
  }

  return true;
}

bool dtrans::isLoadedValueUnused(Value *V, Value *LoadAddr) {
  std::function<bool(Value *, Value *, SmallPtrSetImpl<Value *> &)> IsUnused =
      [&IsUnused](Value *V, Value *LoadAddr,
                  SmallPtrSetImpl<Value *> &UsedValues) -> bool {
    for (auto U : V->users()) {
      // If we've seen this user before, assume its path is OK.
      if (!UsedValues.insert(U).second)
        continue;

      // If the user is a call or invoke, the value escapes.
      // If needed this can be extended for pure functions.
      if (isa<CallBase>(U))
        return false;

      // If the value is used by a terminator, it's used.
      if (cast<Instruction>(U)->isTerminator())
        return false;

      // If the user is a store, check the target address.
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        // If it is volatile or it doesn't match the load address, the value is
        // used.
        if (SI->isVolatile() || SI->getPointerOperand() != LoadAddr)
          return false;

        continue;
      }

      // If load is volatile, the value is used.
      if (auto *LI = dyn_cast<LoadInst>(U))
        if (LI->isVolatile())
          return false;

      // Follow the users of any other user
      if (!IsUnused(U, LoadAddr, UsedValues))
        return false;
    }

    // If the value has no users, this path is unused.
    return true;
  };

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!DTransIdentifyUnusedValues)
      return false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  SmallPtrSet<Value *, 4> UsedValues;
  return IsUnused(V, LoadAddr, UsedValues);
}

bool dtrans::isTypeTestRelatedIntrinsic(const Instruction *I) {
  const IntrinsicInst *II = dyn_cast_or_null<IntrinsicInst>(I);
  if (!II)
    return false;
  Intrinsic::ID IID = II->getIntrinsicID();
  return (IID == Intrinsic::assume || IID == Intrinsic::type_test);
}

// Trace back instruction sequence corresponding to the following code:
//     foo (..., int n, ...) {
//         struct s *s_ptr = malloc(c1 + c2 * n);
//     }
// Returns false if it cannot trace \p InVal back to constants and calculate
// the size.
bool dtrans::traceNonConstantValue(Value *InVal, uint64_t ElementSize,
                                   bool EndsInZeroSizedArray) {
  // Trace call sites to collect all constant actual parameters corresponding to
  // \p FormalArgNo.
  std::function<bool(Function *, Value *, unsigned,
                     SmallVectorImpl<ConstantInt *> &)>
      FindAllArgValues =
          [&FindAllArgValues](
              Function *F, Value *V, unsigned FormalArgNo,
              SmallVectorImpl<ConstantInt *> &ActualArgs) -> bool {
    for (Use &U : V->uses()) {
      Value *Inst = U.getUser();
      // In case of function cast operator do one more step.
      if (BitCastOperator *BC = dyn_cast<BitCastOperator>(Inst)) {
        if (!FindAllArgValues(F, BC, FormalArgNo, ActualArgs))
          return false;
        continue;
      }

      // Must be a direct call.
      auto *Call = dyn_cast<CallBase>(Inst);
      if (!Call || Call->isIndirectCall())
        return false;

      // A called function should be F.
      if (!Call->isCallee(&U))
        if (dtrans::getCalledFunction(*Call) != F)
          return false;

      ConstantInt *ArgC =
          dyn_cast_or_null<ConstantInt>(Call->getArgOperand(FormalArgNo));
      if (!ArgC)
        return false;

      ActualArgs.push_back(ArgC);
    }

    return true;
  };

  if (!InVal)
    return false;

  Value *AddOp, *ShlOp;
  ConstantInt *AddC, *ShlC = nullptr, *MulC = nullptr;

  // Match alloc size with the add with the constant operand.
  if (!match(InVal, m_OneUse(m_Add(m_ConstantInt(AddC), m_Value(AddOp)))) &&
      !match(InVal, m_OneUse(m_Add(m_Value(AddOp), m_ConstantInt(AddC)))))
    return false;

  // Second add operand with the shl or mul with the constant operand.
  if (!match(AddOp, m_Shl(m_Value(ShlOp), m_ConstantInt(ShlC))) &&
      !match(AddOp, m_Mul(m_Value(ShlOp), m_ConstantInt(MulC))) &&
      !match(AddOp, m_Mul(m_ConstantInt(MulC), m_Value(ShlOp))))
    return false;

  // A left shift by a negative value will be undefined behavior, so should
  // return that the size is unknown.
  if (ShlC && ShlC->isNegative())
    return false;

  // Second operand of the shl or mul expected to be function argument.
  Argument *FormalArg = dyn_cast<Argument>(ShlOp);
  if (!FormalArg)
    return false;

  // Now we need to look into each call site and find all constant values
  // for the corresponding argument. If not all actual arguments are constant,
  // return false.
  Function *Callee = FormalArg->getParent();
  unsigned FormalArgNo = FormalArg->getArgNo();

  SmallVector<ConstantInt *, 8> ActualArgs;
  if (!FindAllArgValues(Callee, Callee, FormalArgNo, ActualArgs))
    return false;

  // Now iterate through all constants to verify that the allocation size was
  // correct.
  bool Verified = true;
  for (auto *Const : ActualArgs) {
    uint64_t ArgConst = Const->getLimitedValue();
    uint64_t Res = ShlC ? (ArgConst << ShlC->getLimitedValue())
                        : (ArgConst * MulC->getLimitedValue());
    Res += AddC->getLimitedValue();

    // If the structure has zero-sized array in the last field. It means
    // that allocation size is allowed to be greater or equal to the structure
    // size.
    if (!EndsInZeroSizedArray)
      Verified &= ((Res % ElementSize) == 0);
    else
      Verified &= (Res > ElementSize);
  }

  return Verified;
}

// Helper to analyze a pointer-to-member usage to determine if only a
// specific subset of the structure fields of \p StructTy, starting from \p
// FieldNum and extending by \p AccessSize bytes of the structure are
// touched.
//
// Return 'true' if it can be resolved to precisely match one or more
// adjacent fields starting with the field number identified in the 'LPI'.
// If so, also updated the RegionDesc to set the starting index into
// 'FirstField' and the ending index of affected fields into 'LastField'.
// Otherwise, return 'false'.
static bool analyzeStructFieldAccess(const DataLayout &DL, StructType *StructTy,
                                     size_t FieldNum, uint64_t PrePadBytes,
                                     uint64_t AccessSize,
                                     dtrans::MemfuncRegion *RegionDesc) {
  uint64_t TypeSize = DL.getTypeAllocSize(StructTy);

  // If the size is larger than the base structure size, then the write
  // exceeds the bounds of a single structure, and it's an unsupported
  // use.
  if (AccessSize > TypeSize)
    return false;

  // Try to identify the range of fields being accessed based on the
  // layout of the structure.
  auto FieldTypes = StructTy->elements();
  auto *SL = DL.getStructLayout(StructTy);
  uint64_t FieldOffset = SL->getElementOffset(FieldNum);
  uint64_t AccessStart = FieldOffset - PrePadBytes;
  uint64_t AccessEnd = AccessStart + AccessSize - 1;

  // Check that the access stays within the memory region of the structure,
  // and is not just padding bytes between the fields.
  if (AccessEnd > TypeSize || AccessEnd < FieldOffset)
    return false;

  unsigned int LF = SL->getElementContainingOffset(AccessEnd);

  // Check if the last field was completely covered. If not, we do not
  // support it. It could be safe, but could complicate transforms that need
  // to work with nested structures.
  uint64_t LastFieldStart = SL->getElementOffset(LF);
  uint64_t LastFieldSize = DL.getTypeStoreSize(FieldTypes[LF]);
  if (AccessEnd < (LastFieldStart + LastFieldSize - 1))
    return false;

  uint64_t PostPadBytes = AccessEnd - (LastFieldStart + LastFieldSize - 1);
  RegionDesc->PrePadBytes = PrePadBytes;
  RegionDesc->FirstField = FieldNum;
  RegionDesc->LastField = LF;
  RegionDesc->PostPadBytes = PostPadBytes;
  if (!(FieldNum == 0 && LF == (StructTy->getNumElements() - 1)))
    RegionDesc->IsCompleteAggregate = false;
  else
    RegionDesc->IsCompleteAggregate = true;
  return true;
}

bool dtrans::compareStructName(const llvm::StructType *Ty1,
                               const llvm::StructType *Ty2) {
  if (Ty1->hasName() && Ty2->hasName())
    return Ty1->getName() < Ty2->getName();

  // Named structures before literal structures
  if (Ty1->hasName())
    return true;
  if (Ty2->hasName())
    return false;

  // Compare the printed forms of two literal structures
  std::string Lit1;
  raw_string_ostream OS1(Lit1);
  OS1 << *Ty1;
  OS1.flush();

  std::string Lit2;
  raw_string_ostream OS2(Lit2);
  OS2 << *Ty2;
  OS2.flush();
  return Lit1 < Lit2;
}
