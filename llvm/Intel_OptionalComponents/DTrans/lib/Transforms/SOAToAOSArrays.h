//===---------------- SOAToAOSArrays.h - Part of SOAToAOSPass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements iterators and other helper classes for
// SOAToAOSTransformImpl::CandidateSideEffectsInfo included to SOAToAOSPass.cpp
// only. This file is for analysis of arrays' methods and transformation of
// those.
//
//===----------------------------------------------------------------------===//

namespace
{
// Represents structure, which is array, see CandidateLayoutInfo.
struct SummaryForIdiom {
  StructType *ArrType;
  Type *ElementType;
  StructType *MemoryInterface;
  const Function *Method;
  SummaryForIdiom(StructType *A, Type *E, StructType *MI, const Function *F)
      : ArrType(A), ElementType(E), MemoryInterface(MI), Method(F) {}
};

// Number of idioms related analysis of structures representing arrays.
// Such structures contain only base pointer, integer fields and
// MemoryInterface optionally.
//
// Given these checks one can analyze evolution of
// integer fields.
//
// These checks (in addition to callsite analysis of methods)
// permit to create single structure, which has one copy of
// integer fields and MemoryInterface field.
//
// Element accesses need to be checked for wellformedness
// if one is going to combine base pointers to single one.
struct ArrayIdioms {
protected:
  // GEP (Arg ArgNo) FieldInd.
  static inline bool isArgAddr(const Dep *D, unsigned &ArgNo,
                               unsigned &FieldInd) {
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
  // where OutType is FieldInd'th field of S.ArrType.
  static inline bool isFieldAddr(const Dep *D, const SummaryForIdiom &S,
                                 Type *&OutType) {
    unsigned ArgNo = -1U;
    unsigned FieldInd = -1U;
    if (!isArgAddr(D, ArgNo, FieldInd))
      return false;

    auto *ATy =
        dyn_cast<PointerType>((S.Method->arg_begin() + ArgNo)->getType());
    if (!ATy || ATy->getPointerElementType() != S.ArrType)
      return false;

    if (FieldInd >= S.ArrType->getNumElements())
      return false;

    OutType = S.ArrType->getElementType(FieldInd);
    return true;
  }

  // (Arg ArgNo) of integer type.
  static inline bool isIntegerArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;
    return (S.Method->arg_begin() + D->Const)->getType()->isIntegerTy();
  }

  // GEP (Arg ArgNo) FieldInd,
  // where corresponding type is integer of S.ArrType.
  static inline bool isIntegerFieldAddr(const Dep *D,
                                        const SummaryForIdiom &S) {
    Type *Out = nullptr;
    if (!isFieldAddr(D, S, Out))
      return false;
    return Out->isIntegerTy();
  }

  // GEP (Arg ArgNo) FieldInd,
  // where corresponding type is base pointer of S.ArrType.
  static inline bool isBasePointerAddr(const Dep *D, const SummaryForIdiom &S) {
    Type *Out = nullptr;
    if (!isFieldAddr(D, S, Out))
      return false;
    return Out->isPointerTy() && Out->getPointerElementType() == S.ElementType;
  }

  // Load of some field of S.ArrType.
  static inline bool isFieldLoad(const Dep *D, const SummaryForIdiom &S,
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
  static inline bool isMemoryInterfaceFieldLoadRec(const Dep *D,
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
  static inline bool isExternaSideEffectRec(const Dep *D,
                                            const SummaryForIdiom &S,
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

  // Load from base pointer field of S.ArrType.
  static inline bool isBasePointerLoadBased(const Dep *D,
                                            const SummaryForIdiom &S) {

    if (isBasePointerLoad(D, S))
      return true;

    if (D->Kind != Dep::DK_Function || D->Args->size() != 1)
      return false;

    return isBasePointerLoad(*D->Args->begin(), S);
  }

  // Direct copy of some field to the field of same type,
  //  from one argument to another.
  //
  // Base pointers to base pointers.
  // S.MemoryInterface to S.MemoryInterface.
  //
  // Does not depend on control flow inside S.Method.
  static inline bool isFieldCopy(const Dep *D, const SummaryForIdiom &S,
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

public:
  // Load from base pointer field of S.ArrType.
  static inline bool isBasePointerLoad(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isBasePointerAddr(D->Arg1, S);
  }

  // Load from integer field of S.ArrType.
  static inline bool isIntegerFieldLoad(const Dep *D,
                                        const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isIntegerFieldAddr(D->Arg1, S);
  }

  // Some arithmetic function on integer argument and integer fields of
  // S.ArrType.
  //
  // It should be satisfied for conditions in conditional branches and
  // updates of integer fields of S.ArrType.
  //
  // Other checks should implicitly assume this check if control flow
  // dependence is accounted for.
  static inline bool isDependentOnIntegerFieldsOnly(const Dep *D,
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
  static inline bool isIntegerFieldCopyEx(const Dep *D,
                                          const SummaryForIdiom &S) {
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
  // to corresponding field of S.ArrType.
  static inline bool isMemoryInterfaceSetFromArg(const Dep *D,
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
  static inline bool isMemoryInterfaceCopy(const Dep *D,
                                           const SummaryForIdiom &S) {
    Type *F = nullptr;
    if (!isFieldCopy(D, S, F))
      return false;

    if (!isa<PointerType>(F))
      return false;

    return F->getPointerElementType() == S.MemoryInterface;
  }

  // Some function of
  //  - base pointer, corresponding to argument ArgNo;
  //  - integer fields of ArrTy (given as parameter);
  //  - integer parameters.
  static inline bool isElementAddr(const Dep *D, const SummaryForIdiom &S) {
    bool BaseSeen = false;
    auto Addr = D;
    if (Addr->Kind == Dep::DK_Function) {
      for (auto *A : *Addr->Args)
        if (isIntegerFieldLoad(A, S) || isIntegerArg(A, S))
          continue;
        else if (isBasePointerLoad(A, S)) {
          if (BaseSeen)
            return false;
          BaseSeen = true;
        } else
          return false;
      return BaseSeen;
    } else if (Addr->Kind == Dep::DK_Load)
      return isBasePointerLoad(Addr->Arg1, S);
    return false;
  }

  // Load relative to base pointer from S.ArrType.
  static inline bool isElementLoad(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isElementAddr(D->Arg1, S);
  }

  // Copy of some element in S.ArrType to another element of S.ArrType,
  // addresses are relative to base pointers.
  static inline bool isElementCopy(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (!isElementLoad(D->Arg1, S))
      return false;

    if (!isElementAddr(D->Arg2, S))
      return false;

    return true;
  }

  // Access to S.ElementType from some argument.
  static inline bool isElementValueFromArg(const Dep *D,
                                           const SummaryForIdiom &S) {
    auto *A = D;
    if (A->Kind == Dep::DK_Load)
      A = A->Arg1;

    if (A->Kind != Dep::DK_Argument)
      return false;

    auto *ATy = (S.Method->arg_begin() + A->Const)->getType();

    if (D->Kind == Dep::DK_Load)
      return isa<PointerType>(ATy) &&
             ATy->getPointerElementType() == S.ElementType;

    return ATy == S.ElementType;
  }

  // Store of argument to array (address is relative to base pointer of
  // S.ArrType).
  static inline bool isElementSetFromArg(const Dep *D,
                                         const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;
    if (!isElementValueFromArg(D->Arg1, S))
      return false;
    return isElementAddr(D->Arg2, S);
  }

  // Whether D is represents returned pointer of Allocation.
  static inline bool isAlloc(const Dep *D, const SummaryForIdiom &S) {
    return D->Kind == Dep::DK_Alloc;
  }

  // Some allocation call, whose size argument depends on integer fields of
  // S.ArrType and integer arguments.
  static inline bool isAllocBased(const Dep *D, const SummaryForIdiom &S) {
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

  // Store some element of S.ArrType to newly allocated memory.
  // Value stored is accessed relative to base pointer.
  // Store address is relative to newly allocated memory.
  static inline bool isElementStoreToNewMemory(const Dep *D,
                                               const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (!isElementLoad(D->Arg1, S))
      return false;

    auto *Addr = D->Arg2;

    if (Addr->Kind == Dep::DK_Function) {
      if (Addr->Args->size() != 1)
        return false;
      Addr = *Addr->Args->begin();
    }

    return isAllocBased(Addr, S);
  }

  // Store of constant to newly allocated memory.
  // TODO: extend if needed to memset of base pointer.
  static inline bool isNewMemoryInit(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (D->Arg1->Kind != Dep::DK_Const)
      return false;

    return isAllocBased(D->Arg2, S);
  }

  // Initialize base pointer with newly allocated memory.
  static inline bool isBasePtrInitFromNewMemory(const Dep *D,
                                                const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (!isAllocBased(D->Arg1, S))
      return false;

    Type *Out = nullptr;
    if (!isFieldAddr(D->Arg2, S, Out))
      return false;

    if (!isa<PointerType>(Out))
      return false;

    return S.ElementType == Out->getPointerElementType();
  }

  // Initialize base pointer with constant
  static inline bool isBasePtrInitFromConst(const Dep *D,
                                            const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (D->Arg1->Kind != Dep::DK_Const)
      return false;

    Type *Out = nullptr;
    if (!isFieldAddr(D->Arg2, S, Out))
      return false;

    if (!isa<PointerType>(Out))
      return false;

    return S.ElementType == Out->getPointerElementType();
  }

  // Deallocation of memory pointed to base pointer.
  static inline bool isBasePtrFree(const Dep *D, const SummaryForIdiom &S) {
    auto *Free = D;

    if (D->Kind == Dep::DK_Function && D->Args->size() == 1)
      Free = *D->Args->begin();

    if (Free->Kind != Dep::DK_Free)
      return false;

    if (!isBasePointerLoadBased(Free->Arg1, S))
      return false;

    if (!isMemoryInterfaceFieldLoadRec(Free->Arg2, S))
      return false;

    return true;
  }

  // Potential call to MK_Realloc method.
  // Additional checks of arguments is required.
  // See computeDepApproximation.
  static inline bool isKnownCall(const Dep *D, const SummaryForIdiom &S) {
    return D->Kind == Dep::DK_Call && D->Const == 0;
  }

  static inline bool isThisLikeArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;

    auto *ATy =
        dyn_cast<PointerType>((S.Method->arg_begin() + D->Const)->getType());
    if (!ATy)
      return false;

    return ATy->getPointerElementType() == S.ArrType;
  }

  // Some function of several recursive load relative to S.MemoryInterface:
  //  access to MemoryInterface and vtable.
  static inline bool isMemoryInterfaceFieldLoad(const Dep *D,
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

  static inline bool isExternaSideEffect(const Dep *D,
                                         const SummaryForIdiom &S) {
    bool SeenUnknownTerminal = false;
    return isExternaSideEffectRec(D, S, SeenUnknownTerminal) &&
           !SeenUnknownTerminal;
  }
};
} // namespace


