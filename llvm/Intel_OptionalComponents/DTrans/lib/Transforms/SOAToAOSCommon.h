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
// Assorted utilities for SOA-to-AOS transformation.
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
  // Struct type provided as DepCompute::ClassType during Dep
  // computations.
  StructType *StrType;
  // Special type responsible for allocation/deallocation.
  // May be null if allocation/deallocation is performed using library
  // functions.
  StructType *MemoryInterface;
  // Function being analyzed, it is needed to access arguments of function.
  const Function *Method;
  SummaryForIdiom(StructType *S, StructType *MI, const Function *F)
      : StrType(S), MemoryInterface(MI), Method(F) {}
};

// Number of idioms shared by analysis of of structures representing arrays,
// and structure containing arrays.
//
// It is a part of checks related side-effect analysis.
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
                          unsigned &ArgNo, Type *&OutType) {
    ArgNo = -1U;
    unsigned FieldInd = -1U;
    if (!isArgAddr(D, ArgNo, FieldInd))
      return false;

    auto *ATy =
        dyn_cast<PointerType>(S.Method->getFunctionType()->getParamType(ArgNo));
    if (!ATy || ATy->getElementType() != S.StrType)
      return false;

    if (FieldInd >= S.StrType->getNumElements())
      return false;

    OutType = S.StrType->getElementType(FieldInd);
    return true;
  }

  // Wrapper for isFieldAddr above.
  static bool isFieldAddr(const Dep *D, const SummaryForIdiom &S,
                          Type *&OutType) {
    unsigned ArgNo = -1U;
    return isFieldAddr(D, S, ArgNo, OutType);
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
      assert(S.Method->getFunctionType()
                     ->getParamType(D->Arg1->Const)
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

  // Call to method, i.e. to method of S.StrType,
  // see getStructTypeOfMethod().
  //
  // Additional checks of arguments is required.
  // See computeDepApproximation.
  static bool isKnownCall(const Dep *D, const SummaryForIdiom &S) {
    return D->Kind == Dep::DK_Call && D->Const == 0;
  }

  // Checks if Dep is DK_Argument representing pointer to S.StrType.
  static bool isThisLikeArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;

    auto *ATy = dyn_cast<PointerType>(
        S.Method->getFunctionType()->getParamType(D->Const));
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
            S.Method->getFunctionType()->getParamType(A->Const)))
      if (Deref <= 2 && Out->getPointerElementType() == S.MemoryInterface)
        return true;

    return false;
  }

  static bool isExternaSideEffect(const Dep *D, const SummaryForIdiom &S) {
    bool SeenUnknownTerminal = false;
    return isExternaSideEffectRec(D, S, SeenUnknownTerminal) &&
           !SeenUnknownTerminal;
  }

private:
  // Direct copy of some field to the field of same type:
  //  - S.MemoryInterface to S.MemoryInterface.
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

  // Some external side effect not updating fields, because:
  //  terminal nodes are DK_Const and isMemoryInterfaceFieldLoad,
  //  which is assumed to be accessed only for memory allocation/deallocation.
  //
  // No pointers escape and relying on knowing all occurrence of structures
  // representing arrays.
  //
  // SeenUnknownTerminal implies returned value is false, but not the opposite.
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
};

// Utility to extract array type at offset Off from Struct,
// given Struct is a candidate for SOA-to-AOS.
inline StructType *getSOAArrayType(StructType *Struct, unsigned Off) {
  return cast<StructType>(
      Struct->getElementType(Off)->getPointerElementType());
}

// Utility to extract array type's element,
// given struct representing array.
inline PointerType *getSOAElementType(StructType *ArrType,
                                      unsigned BasePointerOffset) {
  return cast<PointerType>(
      ArrType->getElementType(BasePointerOffset)->getPointerElementType());
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Offset of memory interface in structure.
extern cl::opt<unsigned> DTransSOAToAOSMemoryInterfaceOff;
SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(const Function &F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos
} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSCOMMON_H
