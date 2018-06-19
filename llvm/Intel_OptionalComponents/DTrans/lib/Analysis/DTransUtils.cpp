//===------------ Intel_DTransUtils.cpp - Utilities for DTrans ------------===//
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
/// This file provides a set of utilities that are used by DTrans for both
/// analysis and optimization.
///
// ===--------------------------------------------------------------------=== //

#include "Intel_DTrans/Analysis/DTrans.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace dtrans;

#define DEBUG_TYPE "dtransanalysis"

bool dtrans::isSystemObjectType(llvm::StructType *Ty) {
  if (!Ty->hasName())
    return false;

  return StringSwitch<bool>(Ty->getName())
      .Case("struct._IO_FILE", true)
      .Case("struct._IO_marker", true)
      .Default(false);
}

StringRef dtrans::AllocKindName(AllocKind Kind) {
  switch (Kind) {
  case AK_NotAlloc:
    return "NotAlloc";
  case AK_Malloc:
    return "Malloc";
  case AK_Calloc:
    return "Calloc";
  case AK_Realloc:
    return "Realloc";
  case AK_UserMalloc:
    return "UserMalloc";
  case AK_UserMalloc0:
    return "UserMalloc0";
  }
  llvm_unreachable("Unexpected continuation past AllocKind switch.");
}

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

StringRef dtrans::FreeKindName(FreeKind Kind) {
  switch (Kind) {
  case FK_NotFree:
    return "NotFree";
  case FK_Free:
    return "Free";
  case FK_UserFree:
    return "UserFree";
  }
  llvm_unreachable("Unexpected continuation past FreeKind switch.");
}

AllocKind dtrans::getAllocFnKind(Function *F, const TargetLibraryInfo &TLI) {
  // TODO: Make this implementation more comprehensive.
  if (!F)
    return AK_NotAlloc;
  LibFunc LF;
  if (!TLI.getLibFunc(*F, LF))
    return AK_NotAlloc;
  switch (LF) {
  default:
    return AK_NotAlloc;
  case LibFunc_malloc:
    return AK_Malloc;
  case LibFunc_calloc:
    return AK_Calloc;
  case LibFunc_realloc:
    return AK_Realloc;
  }
  llvm_unreachable("Unexpected continuation past LibFunc switch.");
}

void dtrans::getAllocSizeArgs(AllocKind Kind, CallInst *CI,
                              Value *&AllocSizeVal, Value *&AllocCountVal) {
  assert(Kind != AK_NotAlloc && Kind != AK_UserMalloc0 &&
         "Unexpected alloc kind passed to getAllocSizeArgs");

  if (Kind == AK_Malloc || Kind == AK_UserMalloc) {
    AllocSizeVal = CI->getArgOperand(0);
    AllocCountVal = nullptr;
    return;
  }

  if (Kind == AK_Calloc) {
    AllocCountVal = CI->getArgOperand(0);
    AllocSizeVal = CI->getArgOperand(1);
    return;
  }

  if (Kind == AK_Realloc) {
    AllocSizeVal = CI->getArgOperand(1);
    AllocCountVal = nullptr;
    return;
  }

  llvm_unreachable("Unexpected alloc kind passed to getAllocSizeArgs");
}

bool dtrans::isFreeFn(Function *F, const TargetLibraryInfo &TLI) {
  if (!F)
    return false;
  LibFunc LF;
  if (!TLI.getLibFunc(*F, LF))
    return false;
  return (LF == LibFunc_free);
}

/// This helper function checks if \p Val is a constant integer equal to
/// \p Size
bool dtrans::isValueEqualToSize(Value *Val, uint64_t Size) {
  if (!Val)
    return false;

  if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
    uint64_t ConstSize = ConstVal->getLimitedValue();
    return ConstSize == Size;
  }

  return false;
}

// This helper function checks a value to see if it is either (a) a constant
// whose value is a multiple of the specified size, or (b) an integer
// multiplication operator where either operand is a constant multiple of the
// specified size.
bool dtrans::isValueMultipleOfSize(Value *Val, uint64_t Size) {
  if (!Val)
    return false;

  // Is it a constant?
  if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
    uint64_t ConstSize = ConstVal->getLimitedValue();
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
  }
  // Handle sext and zext
  if (isa<SExtInst>(Val) || isa<ZExtInst>(Val))
    return isValueMultipleOfSize(cast<Instruction>(Val)->getOperand(0), Size);
  // Otherwise, it's not what we needed.
  return false;
}

// This function is called to determine if a bitcast to the specified
// destination type could be used to access element 0 of the source type.
// If the destination type is a pointer type whose element type is the same
// as the type of element zero of the aggregate pointed to by the source
// pointer type, then it would be a valid element zero access.
//
// For example, consider:
//
//   %struct.S1 = type { %struct.S2, i32 }
//   ...
//   %p = bitcast %struct.S1* to %struct.S2*
//
// Because element zero of %struct.S1 has %struct.S2 as a type, this is a
// safe cast that accesses that element. Notice that the pointer to the
// element has a different level of indirection than the declaration of the
// element within the structure. This is expected because the element is
// accessed through a pointer to that element.
//
// Also, consider this example of an unsafe cast.
//
//   %struct.S3 = type { %struct.S4*, i32 }
//   ...
//   %p = bitcast %struct.S3* to %struct.S4*
//
// In this case, element zero of the %struct.S3 type is a pointer, %struct.S4*
// so the correct cast to access that element would be:
//
//   %p = bitcast %struct.S3* to %struct.S4**
//
// Element zero can also be access by casting an i8* pointer that is known
// to point to a given structure to a pointer to element zero of that type.
// However, the caller must handle that case by obtaining the necessary
// type alias information and calling this function with the known alias as
// the SrcTy argument.
//
// If the \p AccessedTy argument is not null, this function will set it to
// nullptr if this is not an element zero access or a pointer to the type of
// the aggregate whose element zero is being accessed. This may be \p SrcTy or
// it may be a nested type if element zero of the source type is an aggregate
// type whose element zero is being accessed.
bool dtrans::isElementZeroAccess(llvm::Type *SrcTy, llvm::Type *DestTy,
                                 llvm::Type **AccessedTy) {
  if (AccessedTy)
    *AccessedTy = nullptr;
  if (!DestTy->isPointerTy() || !SrcTy->isPointerTy())
    return false;
  llvm::Type *SrcPointeeTy = SrcTy->getPointerElementType();
  llvm::Type *DestPointeeTy = DestTy->getPointerElementType();
  // This will handle vector types, in addition to structs and arrays,
  // but I don't think we'd get here with a vector type (unless we end
  // up wanting to track vector types).
  if (auto *CompTy = dyn_cast<CompositeType>(SrcPointeeTy)) {
    // This avoids problems with opaque types.
    if (!CompTy->indexValid(0u))
      return false;
    auto *ElementZeroTy = CompTy->getTypeAtIndex(0u);
    if (DestPointeeTy == ElementZeroTy) {
      if (AccessedTy)
        *AccessedTy = SrcTy;
      return true;
    }
    // If element zero is an aggregate type, this cast might be accessing
    // element zero of the nested type.
    if (ElementZeroTy->isAggregateType())
      return isElementZeroAccess(ElementZeroTy->getPointerTo(), DestTy,
                                 AccessedTy);
    // Otherwise, it must be a bad cast. The caller should handle that.
    return false;
  }
  return false;
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
      dtrans::UnsafePointerStore | dtrans::FieldAddressTaken |
      dtrans::GlobalPtr | dtrans::GlobalInstance | dtrans::HasInitializerList |
      dtrans::BadMemFuncSize | dtrans::MemFuncPartialWrite |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::UnsafePtrMerge | dtrans::AddressTaken | dtrans::NoFieldsInStruct |
      dtrans::NestedStruct | dtrans::ContainsNestedStruct |
      dtrans::SystemObject | dtrans::LocalPtr | dtrans::LocalInstance |
      dtrans::MismatchedArgUse | dtrans::GlobalArray | dtrans::HasVTable |
      dtrans::HasFnPtr | dtrans::UnhandledUse;
  // This assert is intended to catch non-unique safety condition values.
  // It needs to be kept synchronized with the statement above.
  static_assert(
      ImplementedMask ==
          (dtrans::BadCasting ^ dtrans::BadAllocSizeArg ^
           dtrans::BadPtrManipulation ^ dtrans::AmbiguousGEP ^
           dtrans::VolatileData ^ dtrans::MismatchedElementAccess ^
           dtrans::WholeStructureReference ^ dtrans::UnsafePointerStore ^
           dtrans::FieldAddressTaken ^ dtrans::GlobalPtr ^
           dtrans::GlobalInstance ^ dtrans::HasInitializerList ^
           dtrans::BadMemFuncSize ^ dtrans::MemFuncPartialWrite ^
           dtrans::BadMemFuncManipulation ^ dtrans::AmbiguousPointerTarget ^
           dtrans::UnsafePtrMerge ^ dtrans::AddressTaken ^
           dtrans::NoFieldsInStruct ^ dtrans::NestedStruct ^
           dtrans::ContainsNestedStruct ^ dtrans::SystemObject ^
           dtrans::LocalPtr ^ dtrans::LocalInstance ^ dtrans::MismatchedArgUse ^
           dtrans::GlobalArray ^ dtrans::HasVTable ^ dtrans::HasFnPtr ^
           dtrans::UnhandledUse),
      "Duplicate value used in dtrans safety conditions");
  std::vector<StringRef> SafetyIssues;
  if (SafetyInfo & dtrans::BadCasting)
    SafetyIssues.push_back("Bad casting");
  if (SafetyInfo & dtrans::BadAllocSizeArg)
    SafetyIssues.push_back("Bad alloc size");
  if (SafetyInfo & dtrans::BadPtrManipulation)
    SafetyIssues.push_back("Bad pointer manipulation");
  if (SafetyInfo & dtrans::AmbiguousGEP)
    SafetyIssues.push_back("Ambiguous GEP");
  if (SafetyInfo & dtrans::VolatileData)
    SafetyIssues.push_back("Volatile data");
  if (SafetyInfo & dtrans::MismatchedElementAccess)
    SafetyIssues.push_back("Mismatched element access");
  if (SafetyInfo & dtrans::WholeStructureReference)
    SafetyIssues.push_back("Whole structure reference");
  if (SafetyInfo & dtrans::UnsafePointerStore)
    SafetyIssues.push_back("Unsafe pointer store");
  if (SafetyInfo & dtrans::FieldAddressTaken)
    SafetyIssues.push_back("Field address taken");
  if (SafetyInfo & dtrans::GlobalPtr)
    SafetyIssues.push_back("Global pointer");
  if (SafetyInfo & dtrans::GlobalInstance)
    SafetyIssues.push_back("Global instance");
  if (SafetyInfo & dtrans::HasInitializerList)
    SafetyIssues.push_back("Has initializer list");
  if (SafetyInfo & dtrans::BadMemFuncSize)
    SafetyIssues.push_back("Bad memfunc size");
  if (SafetyInfo & dtrans::MemFuncPartialWrite)
    SafetyIssues.push_back("Memfunc partial write");
  if (SafetyInfo & dtrans::BadMemFuncManipulation)
    SafetyIssues.push_back("Bad memfunc manipulation");
  if (SafetyInfo & dtrans::AmbiguousPointerTarget)
    SafetyIssues.push_back("Ambiguous pointer target");
  if (SafetyInfo & dtrans::UnsafePtrMerge)
    SafetyIssues.push_back("Unsafe pointer merge");
  if (SafetyInfo & dtrans::AddressTaken)
    SafetyIssues.push_back("Address taken");
  if (SafetyInfo & NoFieldsInStruct)
    SafetyIssues.push_back("No fields in structure");
  if (SafetyInfo & dtrans::NestedStruct)
    SafetyIssues.push_back("Nested structure");
  if (SafetyInfo & dtrans::ContainsNestedStruct)
    SafetyIssues.push_back("Contains nested structure");
  if (SafetyInfo & dtrans::SystemObject)
    SafetyIssues.push_back("System object");
  if (SafetyInfo & dtrans::LocalPtr)
    SafetyIssues.push_back("Local pointer");
  if (SafetyInfo & dtrans::LocalInstance)
    SafetyIssues.push_back("Local instance");
  if (SafetyInfo & dtrans::MismatchedArgUse)
    SafetyIssues.push_back("Mismatched argument use");
  if (SafetyInfo & dtrans::GlobalArray)
    SafetyIssues.push_back("Global array");
  if (SafetyInfo & dtrans::HasVTable)
    SafetyIssues.push_back("Has vtable");
  if (SafetyInfo & dtrans::HasFnPtr)
    SafetyIssues.push_back("Has function ptr");
  if (SafetyInfo & dtrans::UnhandledUse)
    SafetyIssues.push_back("Unhandled use");
  // Print the safety issues found
  size_t NumIssues = SafetyIssues.size();
  for (size_t i = 0; i < NumIssues; ++i) {
    ostr << SafetyIssues[i];
    if (i != NumIssues - 1) {
      ostr << " | ";
    }
  }

  // TODO: Make this unnecessary.
  if (SafetyInfo & ~ImplementedMask) {
    ostr << " + other issues that need format support ("
         << (SafetyInfo & ~ImplementedMask) << ")";
    ostr << "\nImplementedMask = " << ImplementedMask;
  }

  ostr << "\n";
}

void dtrans::TypeInfo::printSafetyData() {
  outs() << "  Safety data: ";
  printSafetyInfo(SafetyInfo, outs());
}

void dtrans::TypeInfo::setSafetyData(SafetyData Conditions) {
  SafetyInfo |= Conditions;
  LLVM_DEBUG(dbgs() << "dtrans-safety-detail: " << *getLLVMType() << " :: ");
  LLVM_DEBUG(printSafetyInfo(Conditions, dbgs()));
}

bool dtrans::FieldInfo::processNewSingleValue(llvm::Constant *C) {
  if (isNoValue()) {
    setSingleValue(C);
    return true;
  }
  if (isSingleValue() && getSingleValue() != C) {
    setMultipleValue();
    return true;
  }
  return false;
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void PointerTypeInfo::dump() { print(dbgs()); }

void PointerTypeInfo::print(raw_ostream &OS) {
  if (!getAnalyzed()) {
    OS << "    Type: Not analyzed\n";
    return;
  }

  if (!getAliasesToAggregatePointer()) {
    OS << "    Type: Non-aggregate\n";
    return;
  }

  // Put the type names in a vector so that we can output
  // it in sorted order to enable consistency for testing.
  std::vector<std::string> StrVec;

  for (auto *T : Types) {
    std::string Name;
    raw_string_ostream(Name) << "    Type: " << *T;
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
  PTI.print(OS);
}

void FreeCallInfo::dump() { print(dbgs()); }

void FreeCallInfo::print(raw_ostream &OS) {
  OS << "FreeCallInfo:\n";
  OS << "  Kind: " << FreeKindName(FK) << "\n";
  OS << "  Aliased types:\n";
  PTI.print(OS);
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
      OS << "    FirstField: " << getFirstField(RN) << "\n";
      OS << "    LastField:  " << getLastField(RN) << "\n";
    }

    PTI.print(OS);
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
