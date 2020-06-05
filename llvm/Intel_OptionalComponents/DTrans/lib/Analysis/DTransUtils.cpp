//===------------ Intel_DTransUtils.cpp - Utilities for DTrans ------------===//
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
using namespace dtrans;

#define DEBUG_TYPE "dtransanalysis"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
cl::opt<bool> dtrans::DTransPrintAnalyzedTypes("dtrans-print-types",
                                               cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

bool dtrans::dtransIsCompositeType(Type *Ty) {
  if (isa<StructType>(Ty) || isa<ArrayType>(Ty) || isa<VectorType>(Ty))
    return true;
  return false;
}

bool dtrans::dtransCompositeIndexValid(Type *Ty, unsigned Idx) {
  assert(dtransIsCompositeType(Ty) && "Expected Composite Type");
  if (auto *STy = dyn_cast<StructType>(Ty))
    return Idx < STy->getNumElements();
  // Sequential types can be indexed by any integer.
  return true;
}

Type *dtrans::dtransCompositeGetTypeAtIndex(Type *Ty, unsigned Idx) {
  assert(dtransIsCompositeType(Ty) && "Expected Composite Type");
  if (auto *STy = dyn_cast<StructType>(Ty)) {
    assert(dtransCompositeIndexValid(Ty, Idx) && "Invalid structure index!");
    return STy->getElementType(Idx);
  }
  return GetElementPtrInst::getTypeAtIndex(Ty, Idx);
}

bool dtrans::isSystemObjectType(llvm::StructType *Ty) {
  if (!Ty->hasName())
    return false;

  return StringSwitch<bool>(Ty->getName())
      .Case("struct._IO_FILE", true)
      .Case("struct._IO_marker", true)
      .Default(false);
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

Function *dtrans::getCalledFunction(const CallBase &Call) {
  Value *CalledValue = Call.getCalledOperand()->stripPointerCasts();
  if (auto *CalledF = dyn_cast<Function>(CalledValue))
    return CalledF;

  if (auto *GA = dyn_cast<GlobalAlias>(CalledValue))
    if (!GA->isInterposable())
      if (auto *AliasF =
              dyn_cast<Function>(GA->getAliasee()->stripPointerCasts()))
        return AliasF;

  return nullptr;
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
      return (uint64_t(1) << Shift) % Size == 0;
    return false;
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
  if (!SrcTy || !DestTy)
    return false;
  if (!DestTy->isPointerTy() || !SrcTy->isPointerTy())
    return false;
  llvm::Type *SrcPointeeTy = SrcTy->getPointerElementType();
  llvm::Type *DestPointeeTy = DestTy->getPointerElementType();
  // This will handle vector types, in addition to structs and arrays,
  // but I don't think we'd get here with a vector type (unless we end
  // up wanting to track vector types).
  if (dtransIsCompositeType(SrcPointeeTy)) {
    // This avoids problems with opaque types.
    if (!dtransCompositeIndexValid(SrcPointeeTy, 0u))
      return false;
    auto *ElementZeroTy = dtransCompositeGetTypeAtIndex(SrcPointeeTy, 0u);
    // If the element zero type matches the destination pointee type,
    // this is an element zero access.
    if (DestPointeeTy == ElementZeroTy) {
      if (AccessedTy)
        *AccessedTy = SrcTy;
      return true;
    }
    // Handle multiple levels of indirection with i8* destinations.
    // If the element zero type is a pointer type and the destination pointee
    // type is a corresponding i8* at any level of indirection, this is an
    // element zero access.
    if (ElementZeroTy->isPointerTy()) {
      auto *TempZeroTy = ElementZeroTy;
      auto *TempDestTy = DestPointeeTy;
      while (TempZeroTy->isPointerTy() && TempDestTy->isPointerTy()) {
        TempZeroTy = TempZeroTy->getPointerElementType();
        TempDestTy = TempDestTy->getPointerElementType();
      }
      if (TempDestTy == llvm::Type::getInt8Ty(SrcTy->getContext())) {
        if (AccessedTy)
          *AccessedTy = SrcTy;
        return true;
      }
    }
    // If element zero is an aggregate type, this cast might be accessing
    // element zero of the nested type.
    if (ElementZeroTy->isAggregateType())
      return isElementZeroAccess(ElementZeroTy->getPointerTo(), DestTy,
                                 AccessedTy);
    // If element zero is a pointer to an aggregate type this cast might
    // be creating a pointer to element zero of the type pointed to.
    // For instance, if we have the following types:
    //
    //   %A = type { %B*, ... }
    //   %B = type { %C, ... }
    //   %C = type { ... }
    //
    // The following IR would get the address of A->C.
    //
    //   %ppc = bitcast %A* %pa to %C**
    //
    if (ElementZeroTy->isPointerTy() &&
        ElementZeroTy->getPointerElementType()->isAggregateType()) {
      // In this case, tracking the accessed type is tricky because
      // the check is off by a level of indirection. If it's a match
      // we need to record it as element zero of SrcTy.
      bool Match = isElementZeroAccess(ElementZeroTy, DestPointeeTy);
      if (Match && AccessedTy)
        *AccessedTy = SrcTy;
      return Match;
    }
    // Otherwise, it must be a bad cast. The caller should handle that.
    return false;
  }
  return false;
}

bool dtrans::isElementZeroI8Ptr(llvm::Type *Ty, llvm::Type **AccessedTy) {
  if (!dtransIsCompositeType(Ty))
    return false;
  if (!dtransCompositeIndexValid(Ty, 0))
    return false;
  Type *ElementZeroTy = dtransCompositeGetTypeAtIndex(Ty, 0);
  // If element zero is a composite type, look at its first element.
  if (ElementZeroTy->isAggregateType())
    return isElementZeroI8Ptr(ElementZeroTy, AccessedTy);
  if (ElementZeroTy == llvm::Type::getInt8PtrTy(Ty->getContext())) {
    if (AccessedTy)
      *AccessedTy = Ty->getPointerTo();
    return true;
  }
  return false;
}

bool dtrans::isVTableAccess(llvm::Type *SrcTy, llvm::Type *DestTy) {
  // We're looking for a bitcast from a pointer to a class type (StructTy) to
  // the vtable element for that type. The destination type can have many
  // forms but it will always be a ptr-to-ptr-to-ptr-to-fn where the first
  // parameter of the function type (the 'this' argument) is the source type
  // or a pointer to a base class at element zero of the class pointed to by
  // SrcTy.

  // First we check to see if DestTy is a ptr-to-ptr-to-ptr-to-fn
  if (!DestTy->isPointerTy())
    return false;
  auto *DestTy2 = DestTy->getPointerElementType();
  if (!DestTy2->isPointerTy())
    return false;
  auto *DestTy3 = DestTy2->getPointerElementType();
  if (!DestTy3->isPointerTy())
    return false;
  auto *DestFnTy = dyn_cast<FunctionType>(DestTy3->getPointerElementType());
  if (!DestFnTy)
    return false;

  // Next check that the first parameter (the 'this' parameter) of DestFnTy
  // is compatible with SrcTy. The isElementZeroAccess check below tests to
  // see if the type pointed to by ThisParamTy is a base class of the type
  // pointed to by SrcTy. If the destination type is has zero parameters and
  // is variadic, we don't need to check the parameter type. If it has zero
  // parameters and is not variadic, it isn't a match.
  if (DestFnTy->getNumParams() == 0) {
    if (!DestFnTy->isVarArg())
      return false;
  } else {
    auto *ThisParamTy = DestFnTy->getParamType(0);
    if ((ThisParamTy != SrcTy) &&
        !dtrans::isElementZeroAccess(SrcTy, ThisParamTy))
      return false;
  }

  // Finally, look for a vtable pointer at element zero of the type pointed
  // to by SrcTy. The vtable is represented as i32 (...)** and an extra
  // level of indirection is added because the field points to it.
  // Get the expected vtable function type, i32 (...), then add three levels
  // of indirection to that type and see if it tests position as an
  // element zero access of SrcTy.
  auto VTablePtrTy =
      FunctionType::get(Type::getInt32Ty(SrcTy->getContext()), true)
          ->getPointerTo()
          ->getPointerTo()
          ->getPointerTo();
  if (!dtrans::isElementZeroAccess(SrcTy, VTablePtrTy))
    return false;

  return true;
}

// This function is used to recognize IR patterns like this:
//
//   %struct.test.a = type { i32, i32, %struct.test.b* }
//   %struct.test.b = type { %struct.test.c, i32, i32 }
//   %struct.test.c = type { i32, i32, i32, i32 }
//   ...
//   %tmp1 = getelementptr %struct.test.a, %struct.test.a* %p, i64 0, i32 2
//   %tmp2 = bitcast %struct.test.b** %tmp1 to %struct.test.c**
//   %tmp3 = load %struct.test.c*, %struct.test.c** %tmp2
//
// In this case %tmp1 is a pointer to a pointer to %struct.b and in %tmp2
// it is being bitcast as a pointer to a pointer to %struct.c, which is
// the first element of %struct.b.
bool dtrans::isPtrToPtrToElementZeroAccess(llvm::Type *SrcTy,
                                           llvm::Type *DestTy) {
  if (!DestTy->isPointerTy() || !SrcTy->isPointerTy())
    return false;
  llvm::Type *SrcPointeeTy = SrcTy->getPointerElementType();
  llvm::Type *DestPointeeTy = DestTy->getPointerElementType();
  if (!SrcPointeeTy->isPointerTy() || !DestPointeeTy->isPointerTy())
    return false;
  return isElementZeroAccess(SrcPointeeTy, DestPointeeTy);
}

Type *dtrans::unwrapType(Type *Ty) {
  Type *BaseTy = Ty;
  while (BaseTy->isPointerTy() || BaseTy->isArrayTy() || BaseTy->isVectorTy())
    if (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    else if (BaseTy->isArrayTy())
      BaseTy = cast<ArrayType>(BaseTy)->getElementType();
    else if (BaseTy->isVectorTy())
      BaseTy = cast<VectorType>(BaseTy)->getElementType();
  return BaseTy;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *dtrans::getSafetyDataName(const SafetyData &SafetyInfo) {
  assert(countPopulation(SafetyInfo) == 1 &&
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
  if (SafetyInfo & dtrans::FieldAddressTaken)
    return "Field address taken";
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
  if (SafetyInfo & dtrans::DopeVector)
    return "Dope vector";
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
      dtrans::UnsafePointerStore | dtrans::FieldAddressTaken |
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
      dtrans::UnsafePointerStoreConditional | dtrans::DopeVector |
      dtrans::UnhandledUse;
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
           dtrans::HasCppHandling ^ dtrans::HasZeroSizedArray ^
           dtrans::BadCastingPending ^ dtrans::BadCastingConditional ^
           dtrans::UnsafePointerStorePending ^
           dtrans::UnsafePointerStoreConditional ^ dtrans::DopeVector ^
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
  OS << "\n";
}

void dtrans::FieldInfo::print(raw_ostream &OS,
                              dtrans::Transform IgnoredInTransform) const {
  OS << "Field LLVM Type: " << *getLLVMType() << "\n";
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
  OS << "    Readers: ";
  dtrans::printCollectionSorted(OS, readers().begin(), readers().end(), ", ",
                                [](const Function *F) { return F->getName(); });
  OS << "\n";
  OS << "    Writers: ";
  dtrans::printCollectionSorted(OS, writers().begin(), writers().end(), ", ",
                                [](const Function *F) { return F->getName(); });
  OS << "\n";
  OS << "    RWState: "
     << (isRWBottom() ? "bottom" : (isRWComputed() ? "computed" : "top"))
     << "\n";
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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

// To represent call graph in C++ one stores outermost type,
// in whose methods there was reference to this structure.
//
// Lattice of properties is
//  {bottom = <nullptr, false>, <Type*, false>, top = <nullptr, true>}
// Final result represents the structure type, whose methods can be used to
// reach all uses of the given type ThisTy.
void dtrans::StructInfo::CallSubGraph::insertFunction(Function *F,
                                                      StructType *ThisTy) {
  // If we could not approximate CallGraph by methods of some class,
  // no need to analyze further.
  if (isTop())
    return;

  // If reference to ThisTy is encountered in global scope or
  // inside function, which does not look like class method,
  // then mark CallGraph approximation as 'top' or 'failed' approximation.
  if (!F || F->arg_size() < 1) {
    setTop();
    return;
  }
  // Candidate for 'this' pointer;
  auto *Ty = F->arg_begin()->getType();
  if (!isa<PointerType>(Ty)) {
    setTop();
    return;
  }
  auto *StTy = dyn_cast<StructType>(Ty->getPointerElementType());
  if (!StTy) {
    setTop();
    return;
  }

  // Ty >= ThisTy
  //
  // Check if ThisTy can be reachable from Ty by recursion to
  // structure's elements and following pointers.
  std::function<bool(Type *, StructType *, int)> findSubType =
      [&findSubType](Type *Ty, StructType *ThisTy, int Depth) -> bool {
    Depth--;
    if (Depth <= 0)
      return false;
    switch (Ty->getTypeID()) {
    default:
      return false;
    case Type::StructTyID: {
      auto *STy = cast<StructType>(Ty);
      if (STy == ThisTy)
        return true;
      for (auto *FTy : STy->elements())
        if (findSubType(FTy, ThisTy, Depth))
          return true;
      return false;
    }
    case Type::ArrayTyID:
      if (findSubType(Ty->getArrayElementType(), ThisTy, Depth))
        return true;
      return false;
    case Type::PointerTyID:
      if (findSubType(Ty->getPointerElementType(), ThisTy, Depth))
        return true;
      return false;
    }
    llvm_unreachable("Non-exhaustive switch statement");
  };

  // !(StTy >= ThisTy)
  // If ThisTy is not reachable from 'this' argument,
  // then mark as 'top'
  if (!findSubType(StTy, ThisTy, 5)) {
    setTop();
    return;
  }

  // Compute `join`.
  // If cannot find least common approximation to old and new approximation,
  // then mark as 'top'.
  if (isBottom()) {
    State.setPointer(StTy);
  } else if (findSubType(StTy, State.getPointer(), 5)) {
    State.setPointer(StTy);
  } else if (!findSubType(State.getPointer(), StTy, 5)) {
    setTop();
    return;
  }
  // else do nothing
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

  for (auto *T : ElemTypes) {
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

// Returns StringRef with the name of the transformation
StringRef dtrans::getStringForTransform(dtrans::Transform Trans) {
  if (Trans == 0 || Trans & ~dtrans::DT_Legal)
    return "";

  switch (Trans) {
  case dtrans::DT_FieldSingleValue:
    return "fsv";
  case dtrans::DT_FieldSingleAllocFunction:
    return "fsaf";
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

// Check if the last field in the struct type \p Ty is zero-sized array or the
// type is zero-sized array itself.
bool dtrans::hasZeroSizedArrayAsLastField(llvm::Type *Ty) {
  // Return true if it is an array with zero elements.
  if (auto *ArrayTy = dyn_cast<llvm::ArrayType>(Ty)) {
    if (ArrayTy->getArrayNumElements() == 0)
      return true;
  }

  if (auto *StructTy = dyn_cast<llvm::StructType>(Ty)) {
    // Return false if it is an empty structure.
    if (StructTy->getNumElements() == 0)
      return false;
    // Return true if it is a structure which last field is a zero-sized array.
    if (auto *FinalFieldTy =
            StructTy->getElementType(StructTy->getNumElements() - 1))
      if (auto *ArrayTy = dyn_cast<llvm::ArrayType>(FinalFieldTy))
        if (ArrayTy->getArrayNumElements() == 0)
          return true;
  }

  // If it is a pointer type - check the pointer element type.
  if (auto *PointerTy = dyn_cast<llvm::PointerType>(Ty))
    return hasZeroSizedArrayAsLastField(PointerTy->getPointerElementType());

  return false;
}

// Check that function only throws an exception.
bool dtrans::isDummyFuncWithUnreachable(const CallBase *Call,
                                        const TargetLibraryInfo &TLI) {
  auto *F = dtrans::getCalledFunction(*Call);
  if (!F)
    return false;
  if (F->size() != 1)
    return false;
  auto &BB = F->getEntryBlock();
  if (!isa<UnreachableInst>(BB.getTerminator()))
    return false;

  // In dummy function we expect to see only those instructions which throw
  // bad_alloc exception.
  bool CallExAllocFound = false, StoreFound = false, BitCastFound = false;
  bool GEPFound = true, CallExThrowFound = false;
  for (auto &I : BB) {
    if (isa<BitCastInst>(&I))
      BitCastFound = true;
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
  return CallExAllocFound && StoreFound && BitCastFound && GEPFound &&
         CallExThrowFound;
}

bool dtrans::isDummyFuncWithThisAndIntArgs(const CallBase *Call,
                                           const TargetLibraryInfo &TLI) {
  if (!isDummyFuncWithUnreachable(Call, TLI))
    return false;

  if (Call->arg_size() != 2)
    return false;

  Type *ZeroArgType = Call->getArgOperand(0)->getType();
  Type *FirstArgType = Call->getArgOperand(1)->getType();
  return (ZeroArgType->isPointerTy() &&
          ZeroArgType->getPointerElementType()->isStructTy() &&
          FirstArgType->isIntegerTy());
}

bool dtrans::isDummyFuncWithThisAndPtrArgs(const CallBase *Call,
                                           const TargetLibraryInfo &TLI) {
  if (!isDummyFuncWithUnreachable(Call, TLI))
    return false;
  if (Call->arg_size() != 2)
    return false;
  Type *ZeroArgType = Call->getArgOperand(0)->getType();
  Type *FirstArgType = Call->getArgOperand(1)->getType();
  return (ZeroArgType->isPointerTy() &&
          ZeroArgType->getPointerElementType()->isStructTy() &&
          FirstArgType->isPointerTy());
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

// If the loaded operand comes from a BitCast that points to a structure then
// then there is a chance that is loading the Zero element. For example,
// consider the following types:
//
//   %"class.outer" = type { %"class.inner" }
//   %"class.inner" = type { %class.TestClass*, %class.TestClass*}
//   %class.TestClass = type { i64, i64, i64}
//
// Then consider the following BitCast and Load instructions:
//
//   %1 = bitcast %"class.outer"* %0 to i64*
//   %2 = load i64, i64* %1
//
// Assuming that %0 is a memory allocated space (GEP, argument, etc.), then
// the instructions %1 and %2 mean that there is a load for the zero element
// in %class.inner (%class.TestClass*). This function will return the pointer
// type that is being loaded. Also, it will store the structure that
// encapsulates the loaded pointer in the parameter Pointee.
llvm::Type* dtrans::getTypeForZeroElementLoaded(LoadInst *Load,
                                                llvm::Type **Pointee) {

  // If the input type is a Structure, then return the casted form, else check
  // if it is a Pointer type. If so, then check if the pointer's element is a
  // a structure and return it. Else, return nullptr.
  auto GetStructFromPtr = [](Type *PtrTy) -> StructType* {
    if (!PtrTy)
      return nullptr;

    if (PtrTy->isStructTy())
      return cast<StructType>(PtrTy);

    PointerType *Ptr = dyn_cast<PointerType>(PtrTy);
    if (!Ptr)
      return nullptr;

    StructType *Str = dyn_cast<StructType>(Ptr->getPointerElementType());
    return Str;
  };

  if (!Load)
    return nullptr;

  BitCastInst *BCSrc = dyn_cast<BitCastInst>(Load->getPointerOperand());
  if (!BCSrc)
    return nullptr;

  PointerType *OperandTy =
      dyn_cast<PointerType>(Load->getPointerOperandType());
  // Make sure that we are loading from a pointer to an integer type
  if (!OperandTy || !OperandTy->getPointerElementType()->isIntegerTy())
    return nullptr;

  // Check the source
  Value *Src = BCSrc->getOperand(0);
  Type *SourceType = nullptr;

  // NOTE: This function takes care for GEPs and Arguments, but there
  // is the possibility to express the 0 entry of an Aggregate type as a
  // BitCast and a Load.
  if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Src)) {
    SourceType = GEP->getSourceElementType();
    if (!GEP->hasAllConstantIndices())
      return nullptr;

    StructType *CurrStruct = GetStructFromPtr(SourceType);
    if (!CurrStruct)
      return nullptr;

    unsigned NumIdx = GEP->getNumIndices();

    // Byte flattened GEPs might need another analysis
    if (NumIdx < 2)
      return nullptr;

    // If there is a GEP, first we need to traverse through the indices
    // to know which structures are being collected. From there then
    // we collect the 0 elements. This handles the following case:
    //
    //   %"class.outer" = type { %"class.inner1" }
    //   %"class.inner1" = type { %"class.inner2", %"classinner2"}
    //   %"class.inner2" = type { %class.TestClass*, %class.TestClass*}
    //   %class.TestClass = type { i64, i64, i64}
    //
    //   %1 = getelementptr inbounds %"class.outer", %"class.outer"* %0,
    //            i64 0, i32 0, i32 1
    //   %2 = bitcast %"class.inner2"* %1 to i64*
    //   %3 = load i64, i64* %2
    //
    // In the above example, %1 is a GEP that points to %"class.outer", entry 1
    // from %"class.inner1". But the BitCast and Load represents that the entry
    // 0 from %"class.inner2" will be loaded. We need first to traverse the GEP
    // to make sure which structures it is pointing to and then collect the 0
    // element.
    for (unsigned I = 2; I <= NumIdx; I++) {
      ConstantInt *IdxConst = cast<ConstantInt>(GEP->getOperand(I));
      unsigned Idx = IdxConst->getZExtValue();
      CurrStruct = dyn_cast<StructType>(CurrStruct->getElementType(Idx));
      // If the last type collected is not a structure then don't continue.
      // It might be that the GEP is not loading a structure or that the GEP
      // has all the indices needed to collect the type. This case is not
      // handled here.
      if (!CurrStruct)
        return nullptr;
    }

    // Else, there is more to catch, check the zero element
    SourceType = cast<Type>(CurrStruct);
  }
  else if (isa<Argument>(Src))
    SourceType = Src->getType();
  else
    return nullptr;

  StructType *CurrTy = GetStructFromPtr(SourceType);
  if (!CurrTy)
    return nullptr;

  while (CurrTy) {
    Type *Element = CurrTy->getElementType(0);
    if (Element->isStructTy()) {
      CurrTy = cast<StructType>(Element);
    }
    else if (GetStructFromPtr(Element)) {
      *Pointee = cast<Type>(CurrTy);
      return Element;
    } else {
      CurrTy = nullptr;
      break;
    }
  }

  return nullptr;
}

// Helper function to identify if the BitCast instruction will be used for
// loading the 0 element in a structure.
bool dtrans::isBitCastLoadingZeroElement(BitCastInst* BC) {
  if (!BC)
    return false;

  for (User *U : BC->users()) {
    if (auto *Load = dyn_cast<LoadInst>(U)) {
      // Check if the load represents the zero element of a structure.
      Type *Pointee = nullptr;
      Type *LoadedType = dtrans::getTypeForZeroElementLoaded(Load, &Pointee);
      if (LoadedType && Pointee)
        // Store the structure that is being accessed at 0
        return true;
    }
  }
  return false;
}