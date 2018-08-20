//===---------------- SOAToAOSArrays.h - Part of SOAToAOSPass -------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements functionality related specifically to array structures
// for SOA-to-AOS: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSARRAYS_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSARRAYS_H

#if !INTEL_INCLUDE_DTRANS
#error SOAToAOSArrays.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "SOAToAOSEffects.h"

#include "llvm/IR/InstIterator.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// ComputeArrayMethodClassification
#define DTRANS_SOAARR "dtrans-soatoaos-arrays"

namespace llvm {
namespace dtrans {
namespace soatoaos {
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

protected:
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

// It is a detailed classification need for guided comparison using
// FunctionComparator in populateSideEffects.
//
// One needs to separate methods updating fields, which are combined later,
// and non-updating fields, which may contain other side effects not
// interfere with array.
//
// Checks in checkMethod does not check all properties in explanations,
// only essential ones, related to combining/not combining methods.
enum MethodKind {
  MK_Unknown,
  // Parameters are 'this' and integer arguments:
  //  - may update integer fields and base_pointer;
  //  - copy elements from base_pointer to newly allocated memory.
  // Should be combined.
  MK_Realloc,
  // Parameters are 'this', some element and integer arguments:
  //  - may call to realloc-like;
  //  - may update integer fields and base_pointer.
  // Should be combined.
  MK_Append,
  // Parameters are 'this', maybe integer and MemoryInterface:
  //  - integer fields can only be set from argument or constants;
  //  - MemoryInterface can only be set from argument;
  //  - base pointer can only be allocated.
  // Should be combined.
  MK_Ctor,
  // Parameters are 'this' and 'other' instance.
  //  - integer field comes depends on integer fields from 'other'
  //  instance;
  //  - base pointer can only be allocated;
  //  - elements can only be copied from 'other' elements.
  // Should be combined.
  MK_CCtor,
  // Single 'this' parameter
  //  - can only deallocate base pointer.
  // Should be combined.
  MK_Dtor,
  // Parameters are 'this', element and integer candidate for index.
  // Does not update fields.
  //
  // Should NOT be combined.
  MK_Set,
  // Single 'this' parameter
  // Returns value of some integer field.
  // Should NOT be combined.
  MK_GetInteger,
  // Parameters are 'this' and integer index
  // Returns pointer to some element. Need to check for immediate
  // dereference. Or returns element by value.
  // Should NOT be combined.
  MK_GetElement,

  MK_Last = MK_GetElement
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
inline raw_ostream &operator<<(raw_ostream &OS, MethodKind MK) {
  switch (MK) {
  case MK_Unknown:
    OS << "Unknown kind";
    break;
  case MK_Append:
    OS << "Append element method";
    break;
  case MK_Realloc:
    OS << "Realloc method";
    break;
  case MK_Ctor:
    OS << "Ctor method";
    break;
  case MK_CCtor:
    OS << "CCtor method";
    break;
  case MK_Dtor:
    OS << "Dtor method";
    break;
  case MK_Set:
    OS << "Set element method";
    break;
  case MK_GetInteger:
    OS << "Get integer field method";
    break;
  case MK_GetElement:
    OS << "Get pointer to element method";
    break;
  };
  return OS;
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// It is expected that array do not change layout, only base pointers are
// adjusted.
// See comments for ArrayIdioms for details regarding analysis of arrays'
// methods.
class ComputeArrayMethodClassification : public ArrayIdioms {
public:
  using InstContainer = SmallSet<const Instruction *, 5>;

  struct TransformationData {
    // Loads/stores of elements, memset instructions.
    // These are instructions, which need address update and possibly
    // duplication.
    InstContainer ElementInstToTransform;
    // Load of elements from arg.
    // These are instructions, which may require duplication in combined
    // methods.
    InstContainer ElementFromArg;
    // Base pointer manipulations. Loads/stores, allocation calls.
    InstContainer BasePtrInst;
    // GEP and Phi instructions to modify for store/loads/ret instructions in
    // ElementInstToTransform.
    InstContainer ElementPtrGEP;
  };

private:
  // Derivation of MethodKind from analysis in checkMethod.
  struct MethodClassification {
    // MK_Append only.
    // Call to method, should be classified as MK_Realloc
    const Function *CalledMethod = nullptr;
    // MK_Get* cannot have stores to any instances of ArrType.
    bool HasStores = false;
    // MK_GetInteger.
    bool ReturnsIntField = false;
    // MK_CCtor
    // MK_Ctor
    // MK_Realloc
    // MK_Append
    // ArrayIdioms::isBasePtrInitFromNewMemory
    bool HasBasePtrInitFromNewMemory = false;
    // MK_GetElement
    // Address to element or element value is returned.
    bool ReturnsElement = false;
    // MK_Append
    // MK_Set
    // ArrayIdioms::isElementValueFromArg
    bool HasElementSetFromArg = false;
    // MK_Dtor/MK_Realloc.
    bool HasBasePtrFree = false;
    // Not MK_Set and MKF_Get*.
    // Should be false to methods not combined.
    bool HasFieldUpdate = false;
    // Should be false to methods not combined.
    bool HasExternalSideEffect = false;

    MethodKind classifyMethod(const Function *F) const {
      if (HasElementSetFromArg) {
        if (CalledMethod)
          return HasExternalSideEffect ? MK_Unknown : MK_Append;
        if (HasBasePtrFree && HasBasePtrInitFromNewMemory)
          return HasExternalSideEffect ? MK_Unknown : MK_Append;
        return HasFieldUpdate ? MK_Unknown : MK_Set;
      }

      if (!HasStores && !HasBasePtrFree && !HasBasePtrInitFromNewMemory) {
        if (ReturnsIntField)
          return MK_GetInteger;
        if (ReturnsElement)
          return MK_GetElement;
        return MK_Unknown;
      }

      if (HasBasePtrInitFromNewMemory && HasBasePtrFree)
        return HasExternalSideEffect ? MK_Unknown : MK_Realloc;

      if (HasBasePtrFree)
        return HasExternalSideEffect ? MK_Unknown : MK_Dtor;

      if (HasBasePtrInitFromNewMemory && !HasExternalSideEffect) {
        bool IsCopyCtor =
            F->arg_size() == 2 &&
            (F->arg_begin() + 1)->getType() == F->arg_begin()->getType();

        return IsCopyCtor ? MK_CCtor : MK_Ctor;
      }
      return MK_Unknown;
    }
  };

  // Additional check of method call parameters.
  // Complements approximate dependency checks from DepCompute.
  // Helper method for checkMethod.
  //
  // Call to method should have argument dependent on integer fields or integer
  // arguments or be 'this' like arguments.
  bool checkMethodCall(ImmutableCallSite CS) const {
    assert(
        ArrayIdioms::isKnownCall(DM.getApproximation(CS.getInstruction()), S) &&
        "Incorrect checkMethodCall");

    for (auto &Op : CS.getInstruction()->operands()) {
      if (isa<Constant>(Op.get()) || isa<BasicBlock>(Op.get()))
        continue;
      const Dep *DO = DM.getApproximation(Op.get());

      // Additional checks for method call.
      if (!ArrayIdioms::isThisLikeArg(DO, S) &&
          !ArrayIdioms::isDependentOnIntegerFieldsOnly(DO, S))
        return false;
    }
    return true;
  }

  // Checks for structured access to elements.
  // Complements approximate dependency checks from DepCompute.
  // Helper method for checkMethod.
  //
  // Check address of element access: it should be equivalent to GEP relative
  // to base pointer or newly allocated pointer to permit address adjustment
  // in transformation.
  //
  //  %base = ..; loaded directly from field containing base pointer,
  //            ; or from allocation.
  //  %address = GEP %class.elem*, %class.elem** %base, %i
  //  There could be optional bitbast:
  //    %a1 = bitcast %address
  //
  // The check is needed for transformation:
  //  base pointers for arrays are merged to single one and elements are
  //  interleaved.
  bool checkElementAccess(const Value *Address, const char *Desc) const {
    if (isSafeBitCast(DL, Address))
      Address = cast<BitCastInst>(Address)->getOperand(0);

    Type *PTy = Address->getType();

    if (!isa<PointerType>(PTy) || PTy->getPointerElementType() != S.ElementType)
      return false;

    if (!isa<Instruction>(Address))
      return false;

    auto *IAddr = cast<Instruction>(Address);
    if (!all_inst_dep_iterator::isSupportedOpcode(IAddr->getOpcode()))
      return false;

    SmallSet<const Value *, 10> GEPs;

    // Check SCC containing GEPs and PHIs only.
    // For practical purpose po-traversal is sufficient here.
    //
    // Sources of the traversed graphs are loads of base pointers and
    // newly allocated memory.
    for (auto SCCIt = scc_begin(GEPDepGraph<const Value *>(IAddr));
         !SCCIt.isAtEnd(); ++SCCIt) {

      for (auto *V : *SCCIt) {
        auto *I = dyn_cast<Instruction>(V);
        if (!I)
          return false;

        if (!gep_inst_dep_iterator::isSupportedOpcode(
                cast<Instruction>(I)->getOpcode())) {
          const Dep *DO = DM.getApproximation(V->stripPointerCasts());
          // Base is direct access of base pointer in array structure,
          // or value returned from allocation.
          if (!ArrayIdioms::isBasePointerLoad(DO, S) &&
              !ArrayIdioms::isAlloc(DO, S))
            return false;
        } else {
          TI.ElementPtrGEP.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
          InstDesc[I] = std::string("MemInstGEP: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

          auto *GEP = dyn_cast<GetElementPtrInst>(V);
          if (!GEP) {
            assert(isa<PHINode>(V) && "Check GEPInstructionsTrait");
            continue;
          }
          // Access should be structured.
          if (GEP->getNumIndices() != 1)
            return false;
        }
      }
    }
    return true;
  }

  // Checks for structured memset.
  // Complements approximate dependency checks from DepCompute.
  // Helper method for checkMethod.
  //
  // Checks that elements are cleared by memset using structured
  // memory access, i.e. memset should be equivalent to stores with
  // addresses satisfying checkElementAccess.
  //
  // Base address is a result of allocation functions and size is divisible by
  // element size.
  bool checkMemset(const Value *Call) const {
    if (!isa<MemSetInst>(Call))
      return false;

    auto *MS = cast<MemSetInst>(Call);

    const Dep *DO = DM.getApproximation(MS->getDest());
    // Base is direct access of base pointer in array structure,
    // or value returned from allocation.
    if (!ArrayIdioms::isBasePointerLoad(DO, S) && !ArrayIdioms::isAlloc(DO, S))
      return false;

    return dtrans::isValueMultipleOfSize(MS->getLength(),
                                         DL.getTypeStoreSize(S.ElementType),
                                         // Permit Shl.
                                         true);
  }

  ComputeArrayMethodClassification(const ComputeArrayMethodClassification &) =
      delete;
  ComputeArrayMethodClassification &
  operator=(const ComputeArrayMethodClassification &) = delete;

  const DataLayout &DL;
  const DepMap &DM;
  const SummaryForIdiom &S;

  // Information computed for transformation.
  TransformationData &TI;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  mutable DenseMap<const Instruction *, std::string> InstDesc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Wrapper for checkElementAccess, stores instruction and address for
  // transformation and duplication.
  bool checkElementAccessForTransformation(const Instruction *I,
                                           const char *Desc) const {
    assert((isa<LoadInst>(I) || isa<StoreInst>(I) || isa<MemSetInst>(I)) &&
           "Only loads/stores supported");

    const Value *Address = nullptr;
    if (auto *L = dyn_cast<LoadInst>(I))
      Address = L->getPointerOperand();
    else if (auto *S = dyn_cast<StoreInst>(I))
      Address = S->getPointerOperand();

    if (Address) {
      if (checkElementAccess(Address, Desc)) {
        TI.ElementInstToTransform.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        InstDesc[I] = std::string("MemInst: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        return true;
      }
    } else if (checkMemset(I)) {
      TI.ElementInstToTransform.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      InstDesc[I] = std::string("MemInst: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      return true;
    }
    return false;
  }

  void insertElementFromArg(const Instruction *I, const char *Desc) {
    TI.ElementFromArg.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    InstDesc[I] = std::string("Arg: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

  void insertBasePtrInst(const Instruction *I, const char *Desc) {
    TI.BasePtrInst.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    InstDesc[I] = std::string("BasePtrInst: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

public:
  ComputeArrayMethodClassification(const DataLayout &DL, const DepMap &DM,
                                   const SummaryForIdiom &S,
                                   // Results of analysis.
                                   TransformationData &TI)
      : DL(DL), DM(DM), S(S), TI(TI) {}

  unsigned getTotal() const {
    return TI.ElementInstToTransform.size() + TI.ElementFromArg.size() +
           TI.BasePtrInst.size() + TI.ElementPtrGEP.size();
  }

  std::pair<MethodKind, const Function *> classify() {
    MethodClassification MC;
    bool Invalid = false;

    for (auto &I : instructions(*S.Method)) {
      if (arith_inst_dep_iterator::isSupportedOpcode(I.getOpcode()))
        continue;

      const Dep *D = DM.getApproximation(&I);
      if (!D) {
        assert(
            !DTransSOAToAOSComputeAllDep &&
            "Not synchronized ComputeArrayMethodClassification::classify and "
            "DepCompute::computeDepApproximation");
        return std::make_pair(MK_Unknown, nullptr);
      }

      if (D->isBottom() && !DTransSOAToAOSComputeAllDep)
        Invalid = true;

      if (Invalid && !DTransSOAToAOSComputeAllDep)
        return std::make_pair(MK_Unknown, nullptr);

      // Synchronized with DepCompute::computeDepApproximation
      // For each opcode, there are different cases processed,
      // if instruction is classified, then switch statement is exited,
      // otherwise Handled is set and diagnostic printed.
      bool Handled = true;
      switch (I.getOpcode()) {

      // Moved as the first case for emphasis.

      // Checking that all all DK_Function's control-flow-depend on integer
      // fields and integer arguments.
      case Instruction::Br:
        if (!cast<BranchInst>(I).isConditional())
          break;

        if (ArrayIdioms::isDependentOnIntegerFieldsOnly(D, S))
          break;
        // No need to perform accurate check is done for load instruction
        else if (ArrayIdioms::isElementLoad(D, S)) {
          MC.HasExternalSideEffect = true;
          break;
        }
        Handled = false;
        break;

      // Checking for loads are needed only to check
      // valid accesses to elements. Otherwise, they are checked when
      // stores/calls/returns/branches are checked
      case Instruction::Load:
        if (ArrayIdioms::isIntegerFieldLoad(D, S))
          break;
        else if (ArrayIdioms::isBasePointerLoad(D, S)) {
          insertBasePtrInst(&I, "Load of base pointer");
          break;
        } else if (ArrayIdioms::isElementLoad(D, S)) {
          if (checkElementAccessForTransformation(&I, "Element load"))
            break;
          DEBUG_WITH_TYPE(DTRANS_SOAARR,
                          dbgs() << "; Unsupported address computations\n");
        } else if (ArrayIdioms::isMemoryInterfaceFieldLoad(D, S))
          break;
        else if (ArrayIdioms::isElementValueFromArg(D, S)) {
          insertElementFromArg(&I, "Load from arg");
          break;
        }

        Handled = false;
        break;
      case Instruction::Store:
        MC.HasStores = true;

        if (ArrayIdioms::isIntegerFieldCopyEx(D, S)) {
          MC.HasFieldUpdate = true;
          break;
        } else if (ArrayIdioms::isElementCopy(D, S)) {
          if (checkElementAccessForTransformation(&I, "Element copy"))
            break;
          DEBUG_WITH_TYPE(DTRANS_SOAARR,
                          dbgs() << "; Unsupported address computations\n");
        } else if (ArrayIdioms::isElementStoreToNewMemory(D, S)) {
          if (checkElementAccessForTransformation(&I,
                                                  "Element store to new mem"))
            break;
          DEBUG_WITH_TYPE(DTRANS_SOAARR,
                          dbgs() << "; Unsupported address computations\n");
        } else if (ArrayIdioms::isElementSetFromArg(D, S)) {
          if (checkElementAccessForTransformation(&I, "Element set from arg")) {
            MC.HasElementSetFromArg = true;
            break;
          }
          DEBUG_WITH_TYPE(DTRANS_SOAARR,
                          dbgs() << "; Unsupported address computations\n");
        } else if (ArrayIdioms::isBasePtrInitFromNewMemory(D, S)) {
          MC.HasBasePtrInitFromNewMemory = true;
          MC.HasFieldUpdate = true;

          auto *VDep = DM.getApproximation(
              cast<StoreInst>(I).getValueOperand()->stripPointerCasts());
          if (ArrayIdioms::isAlloc(VDep, S)) {
            insertBasePtrInst(&I, "Init base pointer with allocated memory");
            break;
          }
          DEBUG_WITH_TYPE(DTRANS_SOAARR,
                          dbgs()
                              << "; Unsupported base pointer initialization\n");
        } else if (ArrayIdioms::isMemoryInterfaceCopy(D, S) ||
                   ArrayIdioms::isMemoryInterfaceSetFromArg(D, S)) {
          MC.HasFieldUpdate = true;
          break;
        } else if (ArrayIdioms::isBasePtrInitFromConst(D, S)) {
          if (auto *C =
                  dyn_cast<Constant>(cast<StoreInst>(I).getValueOperand()))
            if (C->isZeroValue()) {
              MC.HasFieldUpdate = true;
              insertBasePtrInst(&I, "Nullify base pointer");
              break;
            }
        }
        Handled = false;
        break;
      case Instruction::Unreachable:
        break;
      case Instruction::Ret:
        if (I.getNumOperands() == 0)
          break;

        if (ArrayIdioms::isIntegerFieldLoad(D, S)) {
          MC.ReturnsIntField = true;
          break;
        } else if (ArrayIdioms::isElementAddr(D, S)) {
          MC.ReturnsElement = true;
          if (checkElementAccess(I.getOperand(0), "Address in ret"))
            break;
        }
        // Accurate check is done for load instruction
        else if (ArrayIdioms::isElementLoad(D, S)) {
          MC.ReturnsElement = true;
          break;
        }
        Handled = false;
        break;
      case Instruction::LandingPad:
        if (ArrayIdioms::isExternaSideEffect(D, S)) {
          MC.HasExternalSideEffect = true;
          break;
        }
        Handled = false;
        break;
      case Instruction::Resume:
        if (ArrayIdioms::isExternaSideEffect(D, S)) {
          MC.HasExternalSideEffect = true;
          break;
        }
        Handled = false;
        break;
      case Instruction::Call:
      case Instruction::Invoke:
        if (ArrayIdioms::isKnownCall(D, S)) {
          auto CS = ImmutableCallSite(&I);
          if (checkMethodCall(CS)) {
            MC.CalledMethod = cast<Function>(CS.getCalledValue());
            break;
          }
        } else if (ArrayIdioms::isAlloc(D, S)) {
          insertBasePtrInst(&I, "Allocation call");
          break;
        } else if (ArrayIdioms::isBasePtrFree(D, S)) {
          // May want to check that pointer to deallocate is exactly
          // load of base pointer.
          MC.HasBasePtrFree = true;
          break;
        } else if (ArrayIdioms::isNewMemoryInit(D,
                                                S)) { // Corresponds to memset
          if (checkElementAccessForTransformation(&I, "Memset of elements"))
            break;
          DEBUG_WITH_TYPE(DTRANS_SOAARR,
                          dbgs() << "; Unsupported memory initialization\n");
        } else if (ArrayIdioms::isExternaSideEffect(D, S)) {
          MC.HasExternalSideEffect = true;
          break;
        }
        Handled = false;
        break;
      default:
        Handled = false;
        break;
      }

      if (!Handled) {
        DEBUG_WITH_TYPE(DTRANS_SOAARR, {
          dbgs() << "; Unhandled " << I << "\n";
          D->dump();
        });
        Invalid = true;
      }
    }
    return std::make_pair(MC.classifyMethod(S.Method), MC.CalledMethod);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  class AnnotatedWriter : public AssemblyAnnotationWriter {
    const ComputeArrayMethodClassification &MC;

  public:
    AnnotatedWriter(const ComputeArrayMethodClassification &MC) : MC(MC) {
      assert(MC.InstDesc.size() == MC.TI.ElementInstToTransform.size() +
                                       MC.TI.ElementFromArg.size() +
                                       MC.TI.BasePtrInst.size() +
                                       MC.TI.ElementPtrGEP.size() &&
             "Inconsistent descriptions");
    }

    void emitInstructionAnnot(const Instruction *I,
                              formatted_raw_ostream &OS) override {
      auto It = MC.InstDesc.find(I);
      if (It != MC.InstDesc.end())
        OS << "; " << It->second << "\n";
    }
  };
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Offset of base pointer in DTransSOAToAOSApproxTypename.
extern cl::opt<unsigned> DTransSOAToAOSBasePtrOff;
// Offset of memory interface in DTransSOAToAOSApproxTypename.
extern cl::opt<unsigned> DTransSOAToAOSMemoryInterfaceOff;
SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(Function &F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Instructions to transform.
struct SOAToAOSMethodsCheckDebugResult
    : public ComputeArrayMethodClassification::TransformationData {
  MethodKind MK = MK_Unknown;
};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace dtrans
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSARRAYS_H
