//===---------------- SOAToAOSCommon.h - Part of SOAToAOSPass -------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements functionality shared by analysis in SOAToAOSArrays.h
// and SOAToAOSStruct.h
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSCOMMON_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSCOMMON_H

#if !INTEL_INCLUDE_DTRANS
#error SOAToAOSCommon.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "SOAToAOSEffects.h"

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Represents minimal information for Idioms class
struct SummaryForIdiom {
  StructType *StrType;
  StructType *MemoryInterface;
  const Function *Method;
  SummaryForIdiom(StructType *S, StructType *MI, const Function *F)
      : StrType(S), MemoryInterface(MI), Method(F) {}
};

// Number of idioms shared by analysis of of structures representing arrays,
// and structure containing arrays.
//
// See comment for ArrayIdioms. Every check which does not analyse element type
// of array is put here.
struct Idioms {
protected:
  // GEP (Arg ArgNo) FieldInd.
  static bool isArgAddr(const Dep *D, unsigned &ArgNo, unsigned &FieldInd) {
    if (D->Kind != Dep::DK_GEP)
      return false;

    FieldInd = D->Const;
    auto *Addr = D->Arg2;

    if (Addr->Kind != Dep::DK_Argument)
      return false;

    ArgNo = Addr->Const;
    return true;
  }

  // GEP (Arg ArgNo) FieldInd,
  // where OutType is FieldInd'th field of S.StrType.
  static bool isFieldAddr(const Dep *D, const SummaryForIdiom &S,
                          Type *&OutType) {
    unsigned ArgNo = -1U;
    unsigned FieldInd = -1U;
    if (!isArgAddr(D, ArgNo, FieldInd))
      return false;

    auto *ATy =
        dyn_cast<PointerType>((S.Method->arg_begin() + ArgNo)->getType());
    if (!ATy || ATy->getPointerElementType() != S.StrType)
      return false;

    if (FieldInd >= S.StrType->getNumElements())
      return false;

    OutType = S.StrType->getElementType(FieldInd);
    return true;
  }

  // (Arg ArgNo) of integer type.
  static bool isIntegerArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;
    return (S.Method->arg_begin() + D->Const)->getType()->isIntegerTy();
  }

  // GEP (Arg ArgNo) FieldInd,
  // where corresponding type is integer of S.StrType.
  static bool isIntegerFieldAddr(const Dep *D, const SummaryForIdiom &S) {
    Type *Out = nullptr;
    if (!isFieldAddr(D, S, Out))
      return false;
    return Out->isIntegerTy();
  }

  // Load of some field of S.StrType.
  static bool isFieldLoad(const Dep *D, const SummaryForIdiom &S,
                          Type *&OutType) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isFieldAddr(D->Arg1, S, OutType);
  }

  // Some function of several recursive load relative to S.MemoryInterface.
  //
  // Used in parameters check of Alloc/Free,
  //  which are related to access to MemoryInterface and
  //  vtable in MemoryInterface.
  //
  // Alloc size(..)
  //   Remaining operands, 4th field is MemoryInterface:
  //   MemoryInterface as 'this; pointer to virtual function call.
  //   (Func(Load(GEP(Arg 0) 4))
  //        (Load(Func(Load(Load(GEP(Arg 0) 4))))))
  static bool isMemoryInterfaceFieldLoadRec(const Dep *D,
                                            const SummaryForIdiom &S) {
    if (isMemoryInterfaceFieldLoad(D, S))
      return true;

    if (D->Kind != Dep::DK_Function)
      return false;

    for (auto *A : *D->Args)
      // No need to check recursively.
      if (!isMemoryInterfaceFieldLoad(A, S))
        return false;

    return true;
  }

  // Some external side effect not updating fields, because:
  //  terminal nodes are DK_Const and isMemoryInterfaceFieldLoad,
  //  which is assumed to be accessed only for memory allocation/deallocation.
  //
  // No pointers escape and relying on knowing all occurrence of structures
  // representing arrays.
  static bool isExternaSideEffectRec(const Dep *D, const SummaryForIdiom &S,
                                     bool &SeenUnknownTerminal) {
    if (D->Kind == Dep::DK_Function) {
      bool ExtSE = false;
      for (auto *A : *D->Args)
        if (A->Kind == Dep::DK_Const || isMemoryInterfaceFieldLoad(A, S))
          continue;
        else if (isExternaSideEffectRec(A, S, SeenUnknownTerminal))
          ExtSE = true;
        else {
          SeenUnknownTerminal = true;
          return false;
        }
      return ExtSE;
    }

    if (D->Kind == Dep::DK_Call && D->Const != 0) {
      isExternaSideEffectRec(D->Arg2, S, SeenUnknownTerminal);
      return !SeenUnknownTerminal;
    }

    return false;
  }

  // Direct copy of some field to the field of same type,
  //  from one argument to another.
  //
  // Base pointers to base pointers.
  // S.MemoryInterface to S.MemoryInterface.
  //
  // Does not depend on control flow inside S.Method.
  static bool isFieldCopy(const Dep *D, const SummaryForIdiom &S,
                          Type *&FieldType) {
    if (D->Kind != Dep::DK_Store)
      return false;

    Type *ValType = nullptr;
    if (!isFieldLoad(D->Arg1, S, ValType))
      return false;

    Type *AddrType = nullptr;
    if (!isFieldAddr(D->Arg2, S, AddrType))
      return false;

    FieldType = AddrType;
    return AddrType == ValType;
  }

  // Load from integer field of S.StrType.
  static bool isIntegerFieldLoad(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isIntegerFieldAddr(D->Arg1, S);
  }

  // Some arithmetic function on integer argument and integer fields of
  // S.StrType.
  //
  // It should be satisfied for conditions in conditional branches and
  // updates of integer fields of S.StrType.
  //
  // Other checks should implicitly assume this check if control flow
  // dependence is accounted for.
  static bool isDependentOnIntegerFieldsOnly(const Dep *D,
                                             const SummaryForIdiom &S) {
    if (isIntegerFieldLoad(D, S) || isIntegerArg(D, S))
      return true;

    if (D->Kind == Dep::DK_Const)
      return true;

    if (D->Kind != Dep::DK_Function)
      return false;

    for (auto *A : *D->Args)
      if (!isIntegerFieldLoad(A, S) && !isIntegerArg(A, S))
        return false;
    return true;
  }

  // Store of some value dependent on other integer field and integer arguments
  // to integer field. Should consider all integer fields and all arguments,
  // because conditional branches depend on integer fields and integer
  // arguments.
  static bool isIntegerFieldCopyEx(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (!isIntegerFieldAddr(D->Arg2, S))
      return false;

    if (D->Arg1->Kind != Dep::DK_Const &&
        !isDependentOnIntegerFieldsOnly(D->Arg1, S))
      return false;

    return true;
  }

  // Direct copy of S.MemoryInterface from one argument
  // to corresponding field of S.StrType.
  static bool isMemoryInterfaceSetFromArg(const Dep *D,
                                          const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    Type *Out = nullptr;
    if (!isFieldAddr(D->Arg2, S, Out))
      return false;

    if (!isa<PointerType>(Out))
      return false;

    if (Out->getPointerElementType() != S.MemoryInterface)
      return false;

    if (D->Arg1->Kind == Dep::DK_Argument) {
      assert((S.Method->arg_begin() + D->Arg1->Const)
                     ->getType()
                     ->getPointerElementType() == S.MemoryInterface &&
             "Unexpected type cast");
      return true;
    }
    return false;
  }

  // Copy of MemoryInterface from one instance to another.
  static bool isMemoryInterfaceCopy(const Dep *D, const SummaryForIdiom &S) {
    Type *F = nullptr;
    if (!isFieldCopy(D, S, F))
      return false;

    if (!isa<PointerType>(F))
      return false;

    return F->getPointerElementType() == S.MemoryInterface;
  }

  // Whether D is represents returned pointer of Allocation.
  static bool isAlloc(const Dep *D, const SummaryForIdiom &S) {
    return D->Kind == Dep::DK_Alloc;
  }

  // Some allocation call, whose size argument depends on integer fields of
  // S.StrType and integer arguments.
  static bool isAllocBased(const Dep *D, const SummaryForIdiom &S) {
    auto *Alloc = D;

    if (D->Kind == Dep::DK_Function)
      for (auto *A : *D->Args)
        if (A->Kind == Dep::DK_Alloc) {
          // Single alloc is permitted.
          if (Alloc->Kind == Dep::DK_Alloc)
            return false;
          Alloc = A;
        } else if (!isDependentOnIntegerFieldsOnly(A, S))
          return false;

    if (Alloc->Kind != Dep::DK_Alloc)
      return false;

    if (!isDependentOnIntegerFieldsOnly(Alloc->Arg1, S) &&
        Alloc->Arg1->Kind != Dep::DK_Const)
      return false;

    if (Alloc->Arg2->Kind == Dep::DK_Const)
      return true;

    if (!isMemoryInterfaceFieldLoadRec(Alloc->Arg2, S))
      return false;

    return true;
  }

  // Store of constant to newly allocated memory.
  // TODO: extend if needed to memset of base pointer.
  static bool isNewMemoryInit(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (D->Arg1->Kind != Dep::DK_Const)
      return false;

    return isAllocBased(D->Arg2, S);
  }

  // Potential call to MK_Realloc method.
  // Additional checks of arguments is required.
  // See computeDepApproximation.
  static bool isKnownCall(const Dep *D, const SummaryForIdiom &S) {
    return D->Kind == Dep::DK_Call && D->Const == 0;
  }

  static bool isThisLikeArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;

    auto *ATy =
        dyn_cast<PointerType>((S.Method->arg_begin() + D->Const)->getType());
    if (!ATy)
      return false;

    return ATy->getPointerElementType() == S.StrType;
  }

  // Some function of several recursive load relative to S.MemoryInterface:
  //  access to MemoryInterface and vtable.
  static bool isMemoryInterfaceFieldLoad(const Dep *D,
                                         const SummaryForIdiom &S) {

    if (D->Kind != Dep::DK_Load && D->Kind != Dep::DK_Argument)
      return false;

    auto *A = D;
    int Deref = 0;
    do {
      if (A->Kind == Dep::DK_Function) {
        if (A->Args->size() != 1)
          return false;
        A = *A->Args->begin();
      } else if (A->Kind == Dep::DK_Load) {
        A = A->Arg1;
        ++Deref;
      } else if (A->Kind == Dep::DK_Argument || A->Kind == Dep::DK_GEP)
        break;
      else
        return false;
    } while (true);

    {
      Type *Out = nullptr;
      // 1. Access pointer field in outer structure.
      // 2. Access pointer to vtable
      // 3. Access pointer to function
      if (Deref <= 3 && isFieldAddr(A, S, Out) && Out->isPointerTy() &&
          Out->getPointerElementType() == S.MemoryInterface) {
        return true;
      }
    }

    if (A->Kind != Dep::DK_Argument)
      return false;

    if (auto *Out = dyn_cast<PointerType>(
            (S.Method->arg_begin() + A->Const)->getType()))
      if (Deref <= 2 && Out->getPointerElementType() == S.MemoryInterface)
        return true;

    return false;
  }

  static bool isExternaSideEffect(const Dep *D, const SummaryForIdiom &S) {
    bool SeenUnknownTerminal = false;
    return isExternaSideEffectRec(D, S, SeenUnknownTerminal) &&
           !SeenUnknownTerminal;
  }
};
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Offset of memory interface in DTransSOAToAOSApproxTypename.
extern cl::opt<unsigned> DTransSOAToAOSMemoryInterfaceOff;
SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(Function &F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos
} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSCOMMON_H
