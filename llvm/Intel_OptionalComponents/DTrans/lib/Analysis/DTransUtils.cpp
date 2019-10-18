//===------------ Intel_DTransUtils.cpp - Utilities for DTrans ------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PatternMatch.h"

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
  case AK_New:
    return "new/new[]";
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
  case FK_Delete:
    return "delete/delete[]";
  }
  llvm_unreachable("Unexpected continuation past FreeKind switch.");
}

AllocKind dtrans::getAllocFnKind(const CallBase *Call,
                                 const TargetLibraryInfo &TLI) {
  // Returns non-null, so C++ function.
  if (isNewLikeFn(Call, &TLI))
    return AK_New;
  if (isMallocLikeFn(Call, &TLI))
    // if C++ and could return null, then there should be more than one
    // argument.
    return Call->arg_size() == 1 ? AK_Malloc : AK_New;
  if (isCallocLikeFn(Call, &TLI))
    return AK_Calloc;
  if (isReallocLikeFn(Call, &TLI))
    return AK_Realloc;
  return AK_NotAlloc;
}

void dtrans::getAllocSizeArgs(AllocKind Kind, const CallBase *Call,
                              unsigned &AllocSizeInd, unsigned &AllocCountInd,
                              const TargetLibraryInfo &TLI) {
  assert(Kind != AK_NotAlloc && Kind != AK_UserMalloc0 &&
         "Unexpected alloc kind passed to getAllocSizeArgs");
  switch (Kind) {
  case AK_UserMalloc: {
    // User-defined malloc with two arguments comes from the operator new which
    // was re-defined by user in some class. In this case the first argument is
    // always 'this' pointer and the second argument is 'size' argument.
    // Indirect call means that devirtualization on this call site didn't
    // happen.
    if (Call->arg_size() == 2 || !dtrans::getCalledFunction(*Call)) {
      // Allow user-defined malloc with 'this' ptr argument.
      Type *ZeroArgType = Call->getArgOperand(0)->getType();
      Type *FirstArgType = Call->getArgOperand(1)->getType();
      if (ZeroArgType->isPointerTy() &&
          ZeroArgType->getPointerElementType()->isStructTy() &&
          FirstArgType->isIntegerTy()) {
        AllocSizeInd = 1;
        AllocCountInd = -1U;
        return;
      }
    }
    AllocSizeInd = 0;
    AllocCountInd = -1U;
    return;
  }
  case AK_New:
    AllocSizeInd = 0;
    AllocCountInd = -1U;
    return;
  case AK_Calloc:
  case AK_Malloc:
  case AK_Realloc: {
    /// All functions except calloc return -1 as a second argument.
    auto Inds = getAllocSizeArgumentIndices(Call, &TLI);
    if (Inds.second == -1U) {
      AllocSizeInd = Inds.first;
      AllocCountInd = -1U;
    } else {
      assert(Kind == AK_Calloc && "Only calloc has two size arguments");
      AllocCountInd = Inds.first;
      AllocSizeInd = Inds.second;
    }
    break;
  }
  default:
    llvm_unreachable("Unexpected alloc kind passed to getAllocSizeArgs");
  }
}

// Should be kept in sync with DTransInstVisitor::DTanalyzeAllocationCall.
void dtrans::collectSpecialAllocArgs(AllocKind Kind, const CallBase *Call,
                                     SmallPtrSet<const Value *, 3> &OutputSet,
                                     const TargetLibraryInfo &TLI) {

  unsigned AllocSizeInd = -1U;
  unsigned AllocCountInd = -1U;
  getAllocSizeArgs(Kind, Call, AllocSizeInd, AllocCountInd, TLI);
  if (AllocSizeInd < Call->arg_size())
    OutputSet.insert(Call->getArgOperand(AllocSizeInd));
  if (AllocCountInd < Call->arg_size())
    OutputSet.insert(Call->getArgOperand(AllocCountInd));

  if (Kind == AK_Realloc)
    OutputSet.insert(Call->getArgOperand(0));
}

bool dtrans::isFreeFn(const CallBase *Call, const TargetLibraryInfo &TLI) {
  return isFreeCall(Call, &TLI, false);
}

bool dtrans::isDeleteFn(const CallBase *Call, const TargetLibraryInfo &TLI) {
  return isDeleteCall(Call, &TLI, false);
}

void dtrans::getFreePtrArg(FreeKind Kind, const CallBase *Call,
                           unsigned &PtrArgInd, const TargetLibraryInfo &TLI) {
  assert(Kind != FK_NotFree && "Unexpected free kind passed to getFreePtrArg");

  if (!dtrans::getCalledFunction(*Call)) {
    assert(Kind == FK_UserFree);
    PtrArgInd = 1;
    return;
  }

  if ((Kind == FK_UserFree) && (Call->arg_size() == 2)) {
    // Allow user-defined free with 'this' ptr argument.
    Type *ZeroArgType = Call->getArgOperand(0)->getType();
    Type *FirstArgType = Call->getArgOperand(1)->getType();
    if (ZeroArgType->isPointerTy() &&
        ZeroArgType->getPointerElementType()->isStructTy() &&
        FirstArgType->isPointerTy()) {
      PtrArgInd = 1;
      return;
    }
  }
  PtrArgInd = 0;
}

void dtrans::collectSpecialFreeArgs(FreeKind Kind, const CallBase *Call,
                                    SmallPtrSetImpl<const Value *> &OutputSet,
                                    const TargetLibraryInfo &TLI) {
  unsigned PtrArgInd = -1U;
  getFreePtrArg(Kind, Call, PtrArgInd, TLI);

  if (PtrArgInd < Call->arg_size())
    OutputSet.insert(Call->getArgOperand(PtrArgInd));
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
  if (auto *CompTy = dyn_cast<CompositeType>(SrcPointeeTy)) {
    // This avoids problems with opaque types.
    if (!CompTy->indexValid(0u))
      return false;
    auto *ElementZeroTy = CompTy->getTypeAtIndex(0u);
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
  auto *CompTy = dyn_cast<CompositeType>(Ty);
  if (!CompTy)
    return false;
  // This avoids problems with opaque types.
  if (!CompTy->indexValid(0u))
    return false;
  auto *ElementZeroTy = CompTy->getTypeAtIndex(0u);
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
    else
      BaseTy = BaseTy->getSequentialElementType();
  return BaseTy;
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
      dtrans::UnsafePointerStoreConditional |
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
           dtrans::UnsafePointerStoreConditional ^
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
  if (SafetyInfo & dtrans::HasCppHandling)
    SafetyIssues.push_back("Has C++ handling");
  if (SafetyInfo & dtrans::HasZeroSizedArray)
    SafetyIssues.push_back("Has zero-sized array");
  if (SafetyInfo & dtrans::BadCastingPending)
    SafetyIssues.push_back("Bad casting (pending)");
  if (SafetyInfo & dtrans::BadCastingConditional)
    SafetyIssues.push_back("Bad casting (conditional)");
  if (SafetyInfo & dtrans::UnsafePointerStorePending)
    SafetyIssues.push_back("Unsafe pointer store (pending)");
  if (SafetyInfo & dtrans::UnsafePointerStoreConditional)
    SafetyIssues.push_back("Unsafe pointer store (conditional)");
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
  dbgs() << "  Safety data: ";
  printSafetyInfo(SafetyInfo, dbgs());
}

void dtrans::TypeInfo::setSafetyData(SafetyData Conditions) {
  SafetyInfo |= Conditions;
  LLVM_DEBUG(dbgs() << "dtrans-safety-detail: " << *getLLVMType() << " :: ");
  LLVM_DEBUG(printSafetyInfo(Conditions, dbgs()));
}

bool dtrans::FieldInfo::processNewSingleValue(llvm::Constant *C) {
  if (!C)
    return false;
  return ConstantValues.insert(C).second;
}

bool dtrans::FieldInfo::processNewSingleIAValue(llvm::Constant *C) {
  if (!C)
    return false;
  return ConstantIAValues.insert(C).second;
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

