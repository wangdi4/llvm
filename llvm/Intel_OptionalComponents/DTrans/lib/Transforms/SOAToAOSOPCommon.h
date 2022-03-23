//===-------------- SOAToAOSOPCommon.h - Part of SOAToAOSOPPass -----------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements functionality shared by analysis in SOAToAOSArrays.h
// and SOAToAOSOPStruct.h
//
// Assorted utilities for SOA-to-AOS-OP transformation.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOPCOMMON_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOPCOMMON_H

#if !INTEL_FEATURE_SW_DTRANS
#error SOAToAOSOPCommon.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Transforms/SOAToAOSOP.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "SOAToAOSOPEffects.h"
#include "SOAToAOSOPInternal.h"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {

// Represents minimal information for Idioms class
struct SummaryForIdiom {
  // Struct type provided as DepCompute::ClassType during Dep
  // computations.
  DTransStructType *StrType;
  // Special type responsible for allocation/deallocation.
  // May be null if allocation/deallocation is performed using library
  // functions.
  DTransStructType *MemoryInterface;
  // Function being analyzed, it is needed to access arguments of function.
  const Function *Method;
  DTransSafetyInfo *DTInfo;
  SummaryForIdiom(DTransStructType *S, DTransStructType *MI, const Function *F,
                  DTransSafetyInfo *DTInfo)
      : StrType(S), MemoryInterface(MI), Method(F), DTInfo(DTInfo) {}
};

// Number of idioms shared by analysis of of structures representing arrays,
// and structure containing arrays.
//
// It is a part of checks related side-effect analysis.
struct Idioms {
protected:
  // (Arg 0).
  static bool isThisArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;
    if (D->Const != 0)
      return false;
    return true;
  }

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
                          unsigned &ArgNo, DTransType *&OutType) {
    ArgNo = -1U;
    unsigned FieldInd = -1U;
    if (!isArgAddr(D, ArgNo, FieldInd))
      return false;

    DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
        S.DTInfo->getTypeMetadataReader().getDTransTypeFromMD(
            const_cast<llvm::Function *>(S.Method)));
    if (!DFnTy)
      return false;

    auto *ATy = dyn_cast<DTransPointerType>(DFnTy->getArgType(ArgNo));
    if (!ATy || ATy->getPointerElementType() != S.StrType)
      return false;

    if (FieldInd >= S.StrType->getNumFields())
      return false;

    OutType = S.StrType->getFieldType(FieldInd);
    return true;
  }

  // Wrapper for isFieldAddr above.
  static bool isFieldAddr(const Dep *D, const SummaryForIdiom &S,
                          DTransType *&OutType) {
    unsigned ArgNo = -1U;
    return isFieldAddr(D, S, ArgNo, OutType);
  }

  // Load of some field of S.StrType.
  static bool isFieldLoad(const Dep *D, const SummaryForIdiom &S,
                          DTransType *&OutType) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isFieldAddr(D->Arg1, S, OutType);
  }

  // Some function of several recursive loads relative to S.MemoryInterface.
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

    DTransType *Out = nullptr;
    if (!isFieldAddr(D->Arg2, S, Out))
      return false;

    if (!isa<DTransPointerType>(Out))
      return false;

    if (Out->getPointerElementType() != S.MemoryInterface)
      return false;

    DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
        S.DTInfo->getTypeMetadataReader().getDTransTypeFromMD(
            const_cast<llvm::Function *>(S.Method)));
    if (!DFnTy)
      return false;

    if (D->Arg1->Kind == Dep::DK_Argument) {
      assert(DFnTy->getArgType(D->Arg1->Const)->getPointerElementType() ==
                 S.MemoryInterface &&
             "Unexpected type cast");
      return true;
    }
    return false;
  }

  // Copy of MemoryInterface from one instance to another.
  static bool isMemoryInterfaceCopy(const Dep *D, const SummaryForIdiom &S) {
    DTransType *F = nullptr;
    if (!isFieldCopy(D, S, F))
      return false;

    if (!isa<DTransPointerType>(F))
      return false;

    return F->getPointerElementType() == S.MemoryInterface;
  }

  // Whether D represents returned pointer of Allocation.
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

    DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
        S.DTInfo->getTypeMetadataReader().getDTransTypeFromMD(
            const_cast<llvm::Function *>(S.Method)));
    if (!DFnTy)
      return false;

    auto *ATy = dyn_cast<DTransPointerType>(DFnTy->getArgType(D->Const));
    if (!ATy)
      return false;

    return ATy->getPointerElementType() == S.StrType;
  }

  // Some function of several recursive loads relative to S.MemoryInterface:
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
      DTransType *Out = nullptr;
      // 1. Access pointer field in outer structure.
      // 2. Access pointer to vtable.
      // 3. Access pointer to function.
      if (Deref <= 3 && isFieldAddr(A, S, Out) && Out->isPointerTy() &&
          Out->getPointerElementType() == S.MemoryInterface) {
        return true;
      }
    }

    if (A->Kind != Dep::DK_Argument)
      return false;

    DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
        S.DTInfo->getTypeMetadataReader().getDTransTypeFromMD(
            const_cast<llvm::Function *>(S.Method)));
    if (!DFnTy)
      return false;

    if (auto *Out = dyn_cast<DTransPointerType>(DFnTy->getArgType(A->Const)))
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
                          DTransType *&FieldType) {
    if (D->Kind != Dep::DK_Store)
      return false;

    DTransType *ValType = nullptr;
    if (!isFieldLoad(D->Arg1, S, ValType))
      return false;

    DTransType *AddrType = nullptr;
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
        if (A->Kind == Dep::DK_Const || isMemoryInterfaceFieldLoad(A, S) ||
            isAlloc(A, S))
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

// Verifies that function "F" is safe to apply transformations
// on member functions of vector class. We basically need to
// prove that the function doesn't change fields of vector class.
// Return false if F have any instruction that may write to memory
// (like Store etc) except:
//   1. Alloc instructions
//   2. MemCpy if first argument is allocated memory in the routine.
//
inline bool isSafeCallForAppend(Function *F, DTransSafetyInfo *DTInfo,
                                const TargetLibraryInfo &TLI) {

  // Returns true if "I" is Alloc instruction.
  auto IsAllocCall = [](Instruction *I, DTransSafetyInfo *DTInfo,
                        const TargetLibraryInfo &TLI) {
    auto *CB = dyn_cast_or_null<CallBase>(I);
    if (!CB)
      return false;
    if (DTransAllocCollector::isDummyFuncWithThisAndIntArgs(
            CB, TLI, DTInfo->getTypeMetadataReader()))
      return true;
    auto *CallInfo = DTInfo->getCallInfo(CB);
    if (CallInfo && CallInfo->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc)
      return true;
    return false;
  };

  // Returns true if "V" is return value of any alloc call.
  auto IsAllocPtr = [&IsAllocCall](Value *V, DTransSafetyInfo *DTInfo,
                                   const TargetLibraryInfo &TLI) {
    auto *PN = dyn_cast<PHINode>(V);
    if (!PN)
      return IsAllocCall(PN, DTInfo, TLI);
    for (unsigned I = 0, E = PN->getNumIncomingValues(); I < E; I++) {
      if (!IsAllocCall(dyn_cast<Instruction>(PN->getIncomingValue(I)), DTInfo,
                       TLI))
        return false;
    }
    return true;
  };

  for (Instruction &I : instructions(F)) {
    // Treat llvm.type_test and llvm.assume as safe calls.
    if (isa<DbgInfoIntrinsic>(&I) || dtrans::isTypeTestRelatedIntrinsic(&I))
      continue;
    if (IsAllocCall(&I, DTInfo, TLI))
      continue;
    auto *Memcpy = dyn_cast<MemCpyInst>(&I);
    // Just check 1st argument is coming from alloc call. No need
    // to check for size.
    if (Memcpy && IsAllocPtr(Memcpy->getArgOperand(0), DTInfo, TLI))
      continue;
    if (I.mayWriteToMemory())
      return false;
  }
  return true;
}

// Create new function declaration with “NewDTFunctionTy” and then map it to
// “F” so that OptBase will clone this function.
inline void createAndMapNewAppendFunc(
    Function *F, Module &M, DTransFunctionType *NewDTFunctionTy,
    ValueToValueMapTy &VMap,
    DenseMap<Function *, Function *> &OrigFuncToCloneFuncMap,
    DenseMap<Function *, Function *> &CloneFuncToOrigFuncMap,
    SmallDenseMap<Function *, DTransFunctionType *> &AppendsFuncToDTransTyMap) {

  // For non-opaque pointers case, new update function was also created
  // by DTransOPOptBase similar to the other member functions.
  // Since update function is handled differently, delete the
  // old function and create new update function with NewDTFunctionTy.
  auto *OldF = OrigFuncToCloneFuncMap[F];
  if (OldF) {
    CloneFuncToOrigFuncMap[OldF] = nullptr;
    OrigFuncToCloneFuncMap[F] = nullptr;
    OldF->eraseFromParent();
  }
  Function *NewF =
      Function::Create(cast<FunctionType>(NewDTFunctionTy->getLLVMType()),
                       F->getLinkage(), F->getName(), &M);
  NewF->copyAttributesFrom(F);
  VMap[F] = NewF;
  OrigFuncToCloneFuncMap[F] = NewF;
  CloneFuncToOrigFuncMap[NewF] = F;
  Function::arg_iterator DestI = NewF->arg_begin();
  for (Argument &I : F->args()) {
    DestI->setName(I.getName());
    VMap[&I] = &*DestI++;
  }
  AppendsFuncToDTransTyMap[NewF] = NewDTFunctionTy;
}

// Replace CallInsts of OrigFunc in CallInfo with CallInsts of cloned
// function using NewVMap.
inline void fixCallInfo(Function &OrigFunc, DTransSafetyInfo *DTInfo,
                        ValueToValueMapTy &NewVMap) {
  SmallPtrSet<dtrans::CallInfo *, 8> CallInfoSet;
  for (auto *CInfo : DTInfo->call_info_entries()) {
    if (&OrigFunc != CInfo->getInstruction()->getFunction())
      continue;
    CallInfoSet.insert(CInfo);
  }
  for (auto *CInfo : CallInfoSet)
    DTInfo->replaceCallInfoInstruction(
        CInfo, cast<Instruction>(NewVMap[CInfo->getInstruction()]));
}

// Replace OrigFunc's body with Cloned's body and remove Cloned function.
inline void replaceOrigFuncBodyWithClonedFuncBody(Function &OrigFunc,
                                                  Function &Cloned) {
  // Delete original IR from OrigFunc.
  OrigFunc.deleteBody();
  // Move modified IR from Cloned to the OrigFunc.
  OrigFunc.getBasicBlockList().splice(OrigFunc.begin(),
                                      Cloned.getBasicBlockList());

  // Move users of arguments over to the OrigFunc.
  for (Function::arg_iterator I = Cloned.arg_begin(), E = Cloned.arg_end(),
                              I2 = OrigFunc.arg_begin();
       I != E; ++I, ++I2)
    I->replaceAllUsesWith(&*I2);

  // Clone metadata to the OrigFunc, including debug info descriptor.
  SmallVector<std::pair<unsigned, MDNode *>, 1> MDs;
  Cloned.getAllMetadata(MDs);
  for (auto MD : MDs)
    OrigFunc.addMetadata(MD.first, *MD.second);
  if (Cloned.hasPersonalityFn())
    OrigFunc.setPersonalityFn(Cloned.getPersonalityFn());

  // Remove temporary Cloned function.
  Cloned.eraseFromParent();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Offset of memory interface in structure.
extern cl::opt<unsigned> DTransSOAToAOSOPMemoryInterfaceOff;
SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(Function &F,
                                                          DTransSafetyInfo *);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaosOP
} // namespace dtransOP
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOPCOMMON_H
