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

#include "SOAToAOSCommon.h"
#include "SOAToAOSEffects.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

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
struct ArraySummaryForIdiom : public SummaryForIdiom {
  Type *ElementType;
  ArraySummaryForIdiom(StructType *A, Type *E, StructType *MI,
                       const Function *F)
      : SummaryForIdiom(A, MI, F), ElementType(E) {}
  ArraySummaryForIdiom(const SummaryForIdiom &S, Type *E)
      : SummaryForIdiom(S), ElementType(E) {}
};

// Number of idioms related to analysis of structures representing arrays.
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
struct ArrayIdioms : public Idioms {
protected:
  // GEP (Arg ArgNo) FieldInd,
  // where corresponding type is base pointer of S.StrType.
  static bool isBasePointerAddr(const Dep *D, const ArraySummaryForIdiom &S) {
    Type *Out = nullptr;
    if (!isFieldAddr(D, S, Out))
      return false;
    return Out->isPointerTy() && Out->getPointerElementType() == S.ElementType;
  }

  // Load from base pointer field of S.StrType.
  // Same as isBasePointerLoad, but some arithmetic function computed from
  // load's result is permitted.
  static bool isBasePointerLoadBased(const Dep *D,
                                     const ArraySummaryForIdiom &S) {

    if (isBasePointerLoad(D, S))
      return true;

    if (D->Kind != Dep::DK_Function || D->Args->size() != 1)
      return false;

    return isBasePointerLoad(*D->Args->begin(), S);
  }

  // Load from base pointer field of S.StrType.
  static bool isBasePointerLoad(const Dep *D, const ArraySummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isBasePointerAddr(D->Arg1, S);
  }

  // Some function of
  //  - base pointer, corresponding to argument ArgNo;
  //  - integer fields of ArrTy (given as parameter);
  //  - integer parameters.
  static bool isElementAddr(const Dep *D, const ArraySummaryForIdiom &S) {
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

  // Load relative to base pointer from S.StrType.
  static bool isElementLoad(const Dep *D, const ArraySummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;
    return isElementAddr(D->Arg1, S);
  }

  // Copy of some element in S.StrType to another element of S.StrType,
  // addresses are relative to base pointers.
  static bool isElementCopy(const Dep *D, const ArraySummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;

    if (!isElementLoad(D->Arg1, S))
      return false;

    if (!isElementAddr(D->Arg2, S))
      return false;

    return true;
  }

  // Access to S.ElementType from some argument.
  static bool isElementValueFromArg(const Dep *D,
                                    const ArraySummaryForIdiom &S) {
    auto *A = D;
    if (A->Kind == Dep::DK_Load)
      A = A->Arg1;

    if (A->Kind != Dep::DK_Argument)
      return false;

    auto *ATy = S.Method->getFunctionType()->getParamType(A->Const);

    if (D->Kind == Dep::DK_Load)
      return isa<PointerType>(ATy) &&
             ATy->getPointerElementType() == S.ElementType;

    return ATy == S.ElementType;
  }

  // Store of argument to array (address is relative to base pointer of
  // S.StrType).
  static bool isElementSetFromArg(const Dep *D, const ArraySummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;
    if (!isElementValueFromArg(D->Arg1, S))
      return false;
    return isElementAddr(D->Arg2, S);
  }

  // Store some element of S.StrType to newly allocated memory.
  // Value stored is accessed relative to base pointer.
  // Store address is relative to newly allocated memory.
  static bool isElementStoreToNewMemory(const Dep *D,
                                        const ArraySummaryForIdiom &S) {
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

  // Initialize base pointer with newly allocated memory.
  static bool isBasePtrInitFromNewMemory(const Dep *D,
                                         const ArraySummaryForIdiom &S) {
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
  static bool isBasePtrInitFromConst(const Dep *D,
                                     const ArraySummaryForIdiom &S) {
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
  static bool isBasePtrFree(const Dep *D, const ArraySummaryForIdiom &S) {
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
  using InstContainer = SmallSet<const Instruction *, 32>;

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
    // MK_Get* cannot have stores to any instances of StrType.
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
      bool HasVoidRes = F->getReturnType()->isVoidTy();
      if (HasElementSetFromArg) {
        if (CalledMethod)
          return HasExternalSideEffect || !HasVoidRes ? MK_Unknown : MK_Append;
        if (HasBasePtrFree && HasBasePtrInitFromNewMemory)
          return HasExternalSideEffect || !HasVoidRes ? MK_Unknown : MK_Append;
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
        auto *FuncTy = F->getFunctionType();
        bool IsCopyCtor =
            F->arg_size() == 2 &&
            FuncTy->getParamType(1) == FuncTy->getParamType(0);

        return IsCopyCtor ? MK_CCtor : MK_Ctor;
      }
      return MK_Unknown;
    }
  };

  // Additional check of method call parameters.
  // Complements approximate dependency checks from DepCompute.
  // Helper method for classify.
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

  void insertElementFromArg(const Instruction *I, const char *Desc) const {
    TI.ElementFromArg.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    InstDesc[I] = std::string("Arg: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

  void insertBasePtrInst(const Instruction *I, const char *Desc) const {
    TI.BasePtrInst.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    InstDesc[I] = std::string("BasePtrInst: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

  ComputeArrayMethodClassification(const ComputeArrayMethodClassification &) =
      delete;
  ComputeArrayMethodClassification &
  operator=(const ComputeArrayMethodClassification &) = delete;

  const DataLayout &DL;
  const DepMap &DM;
  const ArraySummaryForIdiom &S;

  // Information computed for transformation.
  TransformationData &TI;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  mutable DenseMap<const Instruction *, std::string> InstDesc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

public:
  ComputeArrayMethodClassification(const DataLayout &DL, const DepMap &DM,
                                   const ArraySummaryForIdiom &S,
                                   // Results of analysis.
                                   TransformationData &TI)
      : DL(DL), DM(DM), S(S), TI(TI) {}

  unsigned getTotal() const {
    return TI.ElementInstToTransform.size() + TI.ElementFromArg.size() +
           TI.BasePtrInst.size() + TI.ElementPtrGEP.size();
  }

  std::pair<MethodKind, const Function *> classify() const {
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
        break;

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
      case Instruction::Alloca: // Not handled here
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

    return Invalid
               ? std::make_pair(MK_Unknown, nullptr)
               : std::make_pair(MC.classifyMethod(S.Method), MC.CalledMethod);
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

class ArrayMethodTransformation {
public:
  ArrayMethodTransformation(
      const DataLayout &DL, const DTransAnalysisInfo &DTInfo,
      const TargetLibraryInfo &TLI, ValueToValueMapTy &VMap,
      const ComputeArrayMethodClassification::TransformationData
          &InstsToTransform,
      LLVMContext &Context)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), VMap(VMap),
        InstsToTransform(InstsToTransform), Context(Context) {}

  using OrigToCopyTy = DenseMap<Value *, Value *>;

  // Update instructions related to base pointer manipulations.
  // There are several kind of instructions:
  //  - load of base pointer;
  //  - store of pointer allocated to base pointer field;
  //  - zero initialization of base pointer;
  //  - memory allocation for array.
  //
  // isSafeBitCast-idiom is processed for loads and stores.
  void updateBasePointerInsts(bool IsCombined, int NumArrays,
                              PointerType *NewBaseType) {
    IRBuilder<> Builder(Context);
    auto *PNewBaseType = PointerType::get(NewBaseType, 0);

    auto &LDL = DL;
    auto GetGEP = [&LDL](Value *V, bool &BC) -> GetElementPtrInst * {
      BC = false;
      if (auto *GEP = dyn_cast<GetElementPtrInst>(V))
        return GEP;
      if (isSafeBitCast(LDL, V)) {
        BC = true;
        return cast<GetElementPtrInst>(cast<BitCastInst>(V)->getOperand(0));
      }
      llvm_unreachable("Check DepCompute/ComputeArrayMethodClassification for "
                       "counter peep-hole analysis");
    };

    for (auto *I : InstsToTransform.BasePtrInst) {
      auto *NewI = cast<Instruction>((Value *)VMap[I]);
      if (auto *NewLoad = dyn_cast<LoadInst>(NewI)) {
        // %base_off = getelementptr inbounds %class, %class* %this, i64 0, i32 3
        // %base = laad %pelem*, %pelem** %base_off
        //
        // Special base with safe bitcast (to pass to deallocation function):
        // %base_off = getelementptr inbounds %class, %class* %this, i64 0, i32 3
        // %i8ptr = bitcast %pelem** %base_off to i8**
        // %i8base = load i8*, i8** %i8ptr
        bool BC = false;
        auto *Addr = GetGEP(NewLoad->getPointerOperand(), BC);
        Addr->mutateType(PNewBaseType);
        Addr->setResultElementType(NewBaseType);
        // No need to make type consistent if load has non-specific type.
        if (!BC)
          NewLoad->mutateType(NewBaseType);
      } else if (auto *NewStore = dyn_cast<StoreInst>(NewI)) {
        // Zero initialization of base pointer:
        // %base_off = getelementptr inbounds %class, %class* %this, i64 0, i32 3
        // store %pelem* null, %pelem** %base_off
        //
        // Store of pointer to newly allocated memory to base pointer
        // %base_off = getelementptr inbounds %class, %class* %this, i64 0, i32 3
        // %new_mem  = bitbast %i8 %alloc to %pelem*
        // store %pelem* %newmem, %pelem** %base_off
        bool BC = false;
        auto *Addr = GetGEP(NewStore->getPointerOperand(), BC);
        Addr->mutateType(PNewBaseType);
        Addr->setResultElementType(NewBaseType);
        // No need to make type consistent if load has non-specific type.
        if (!BC) {
          auto *Val = NewStore->getValueOperand();
          if (auto *C = dyn_cast<Constant>(Val)) {
            assert(C->isZeroValue() && "Incorrect analysis");
            NewStore->replaceUsesOfWith(C,
                                        ConstantPointerNull::get(NewBaseType));
          } else
            Val->mutateType(NewBaseType);
        }
      } else if (auto CS = CallSite(NewI)) {
        auto *Info = DTInfo.getCallInfo(NewI);
        assert(Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc &&
               "Incorrect analysis");

        assert(IsCombined && "Incorrect analysis");
        unsigned S1 = -1U;
        unsigned S2 = -1U;
        getAllocSizeArgs(cast<AllocCallInfo>(Info)->getAllocKind(), CS, S1, S2,
                         TLI);
        assert((S1 == -1U) != (S2 == -1U) && "Unexpected allocation routine");
        auto *OldSize = S1 == -1U ? CS.getArgument(S2) : CS.getArgument(S1);
        Builder.SetInsertPoint(NewI);
        // Allocation size is multiplied by NumArrays to reserve sufficient
        // space in AOS.
        auto *NewSize = Builder.CreateTruncOrBitCast(
            Builder.CreateNUWMul(
                Builder.CreateZExtOrBitCast(OldSize, Builder.getIntPtrTy(DL, 0),
                                            "nsz"),
                ConstantInt::get(Builder.getIntPtrTy(DL, 0), NumArrays), "nsz"),
            OldSize->getType(), "nsz");
        CS.getInstruction()->replaceUsesOfWith(OldSize, NewSize);
      } else
        llvm_unreachable("Unexpected instruction encountered");
    }
  }

  // Copy and link def-use chains of instructions related to element accesses
  // in combined methods.
  //
  // ElementFromArg, ElementPtrGEP and ElementInstToTransform are processed.
  //
  // ElementPtrGEP: addresses of loads/stores are processed (GEPs and PHIs).
  // ElementInstToTransform: loads/stores/memsets.
  //  - memset is not duplicated (UniqueInsts specified the moment of
  //  transformation), but size argument is multiplied.
  //  - addresses of loads/stores may be adjusted from ElementPtrGEP with
  //  safe bitcast (see checkElementAccess).
  //
  // In checkElementAccess the following sequences are permitted:
  // %base = load %pelem*, %pelem** %base_off
  // %base1 = load %pelem*, %pelem** %base_off1
  // %arrayidx = getelementptr inbounds %pelem, %pelem* %base, i64 %iv
  // %arrayidx1 = getelementptr inbounds %pelem, %pelem* %base1, i64 %iv1
  // %tmp = bitcast %pelem* %arrayidx to i64*    <== safe bitcast
  // %tmp1 = bitcast %pelem* %arrayidx1 to i64*  <== safe bitcast
  // %val = load i64, i64* %tmp1
  // store i64 %tmp12, i64* %tmp
  void rawCopyAndRelink(OrigToCopyTy &OrigToCopy, bool UniqueInsts,
                        unsigned NumArrays, Type *OtherElemType,
                        unsigned NewParamOffset) {
    IRBuilder<> Builder(Context);

    auto ProcessSafeBitCast = [&](const Value *Old,
                                  BitCastInst *NewBC) -> void {
      assert(isSafeBitCast(DL, Old) && "Some peephole idiom is not processed");
      assert((isa<GetElementPtrInst>(NewBC->getOperand(0)) ||
              isa<Argument>(NewBC->getOperand(0))) &&
             "Some peephole idiom is not processed");
      auto It = OrigToCopy.find(NewBC);
      if (It == OrigToCopy.end()) {
        Builder.SetInsertPoint(NewBC);
        auto *CopyBC = Builder.CreateBitCast(
            // Assumed all pointer types have same alignment.
            NewBC->getOperand(0), NewBC->getType(), "copy");
        OrigToCopy[NewBC] = CopyBC;
        return;
      }
    };

    for (auto *I : InstsToTransform.ElementFromArg) {
      auto *NewI = cast<Instruction>((Value *)VMap[I]);
      auto *NewLoad = cast<LoadInst>(NewI);
      auto *NewPtr = NewLoad->getPointerOperand();
      auto *F = NewI->getParent()->getParent();

      Builder.SetInsertPoint(NewLoad);
      auto *CopyLoad = Builder.CreateAlignedLoad(
          // Assumed all pointer types have same alignment.
          NewPtr, NewLoad->getAlignment(), false, "copy");

      if (auto *NewBC = dyn_cast<BitCastInst>(NewPtr)) {
        ProcessSafeBitCast(cast<LoadInst>(I)->getPointerOperand(), NewBC);
        OrigToCopy[NewBC->getOperand(0)] = &F->arg_begin()[NewParamOffset];
      } else {
        assert(isa<Argument>(NewPtr) && "Some peephole idiom is not processed");
        OrigToCopy[NewPtr] = &F->arg_begin()[NewParamOffset];
        CopyLoad->mutateType(OtherElemType);
      }

      OrigToCopy[NewLoad] = CopyLoad;
    }

    for (auto *I : InstsToTransform.ElementInstToTransform) {
      auto *NewI = cast<Instruction>((Value *)VMap[I]);
      auto *F = NewI->getParent()->getParent();
      if (auto *NewLoad = dyn_cast<LoadInst>(NewI)) {
        auto *NewPtr = cast<Instruction>(NewLoad->getPointerOperand());
        Builder.SetInsertPoint(NewLoad);
        auto *CopyLoad = Builder.CreateAlignedLoad(
            // Assumed all pointer types have same alignment.
            NewPtr, NewLoad->getAlignment(), false, "copy");
        OrigToCopy[NewLoad] = CopyLoad;
        if (auto *NewBC = dyn_cast<BitCastInst>(NewPtr))
          ProcessSafeBitCast(cast<LoadInst>(I)->getPointerOperand(), NewBC);
        else {
          assert(isa<GetElementPtrInst>(NewPtr) &&
                 "Some peephole idiom is not processed");
          CopyLoad->mutateType(OtherElemType);
        }
      } else if (auto *NewStore = dyn_cast<StoreInst>(NewI)) {
        Builder.SetInsertPoint(NewStore);
        auto *NewPtr = cast<Instruction>(NewStore->getPointerOperand());
        auto *CopyStore = Builder.CreateAlignedStore(
            NewStore->getValueOperand(), NewPtr,
            // Assumed all pointer types have same alignment.
            NewStore->getAlignment(), false);
        OrigToCopy[NewStore] = CopyStore;

        if (auto *NewBC = dyn_cast<BitCastInst>(NewPtr))
          ProcessSafeBitCast(cast<StoreInst>(I)->getPointerOperand(), NewBC);
        else
          assert(isa<GetElementPtrInst>(NewPtr) &&
                 "Some peephole idiom is not processed");

        // Element set from arg.
        if (isa<Argument>(NewStore->getValueOperand()))
          OrigToCopy[NewStore->getValueOperand()] =
              &F->arg_begin()[NewParamOffset];
      } else if (auto MS = dyn_cast<MemSetInst>(NewI)) {
        if (!UniqueInsts)
          continue;

        Builder.SetInsertPoint(MS);
        auto *OldSize = MS->getLength();
        auto *NewSize = Builder.CreateTruncOrBitCast(
            Builder.CreateNUWMul(
                Builder.CreateZExtOrBitCast(OldSize, Builder.getIntPtrTy(DL, 0),
                                            "nsz"),
                ConstantInt::get(Builder.getIntPtrTy(DL, 0), NumArrays), "nsz"),
            OldSize->getType(), "nsz");
        MS->replaceUsesOfWith(OldSize, NewSize);
      } else
        llvm_unreachable("Unexpected instruction encountered");
    }

    // Relink copies.
    for (auto P : OrigToCopy) {
      auto *NewI = P.first;
      auto *CopyI = P.second;
      if (auto *CopyLoad = dyn_cast<LoadInst>(CopyI)) {
        auto It =
            OrigToCopy.find(CopyLoad->getPointerOperand());
        if (It != OrigToCopy.end())
          CopyLoad->setOperand(0, It->second);
      } else if (auto *CopyStore = dyn_cast<StoreInst>(CopyI)) {
        auto It =
            OrigToCopy.find(CopyStore->getPointerOperand());
        if (It != OrigToCopy.end())
          CopyStore->setOperand(1, It->second);
        auto *CopyVal = CopyStore->getValueOperand();
        if (!isa<Constant>(CopyVal)) {
          assert((isa<Instruction>(CopyVal) || isa<Argument>(CopyVal)) &&
                 "Unexpected value of StoreInst");
          auto It = OrigToCopy.find(CopyVal);
          if (It != OrigToCopy.end())
            CopyStore->setOperand(0, It->second);
        }
      } else if (auto *CopyBC = dyn_cast<BitCastInst>(CopyI)) {
        auto It = OrigToCopy.find(CopyBC->getOperand(0));
        if (It != OrigToCopy.end())
          CopyBC->setOperand(0, It->second);
      } else if (isa<MemSetInst>(NewI))
        continue;
      else if (isa<Argument>(NewI))
        continue;
      else
        llvm_unreachable("Unexpected instruction encountered");
    }
  }

  // Updating GEPs, whose base pointer is result of allocation or load from
  // base pointer. Coupled with checkElementAccess.
  //
  // Uses outside SCC should be updated.
  //
  // From:
  //  %base = load %pelem*, %pelem** %base_off
  //  %arrayidx = getelementptr inbounds %pelem, %pelem* %base, i64 %iv
  //  %arrayidx1 = getelementptr inbounds %pelem, %pelem* %arrayidx, i64 %iv1
  //  load %pelem, %pelem* %arrayidx1
  // To:
  //  %base = load %new_elem**, %new_elem*** %base_off
  //  %arrayidx = getelementptr inbounds %new_elem*, %pelem* %base, i64 %iv
  //  %arrayidx1 = getelementptr inbounds %new_elem*, %pelem* %arrayidx,
  //              i64 %iv1, 0
  //  load %pelem, %pelem* %arrayidx1
  void gepRAUW(bool Copy, const OrigToCopyTy &OrigToCopy, unsigned Offset,
               PointerType *NewBaseType) {
    IRBuilder<> Builder(Context);

    auto &ElementPtrGEP = InstsToTransform.ElementPtrGEP;
    for (auto *I : ElementPtrGEP) {
      auto *NewI = cast<Instruction>((Value *)VMap[I]);
      if (auto *NewGEP = dyn_cast<GetElementPtrInst>(NewI)) {
        NewGEP->mutateType(NewBaseType);
        NewGEP->getPointerOperand()->mutateType(NewBaseType);
        NewGEP->setResultElementType(NewBaseType->getPointerElementType());
        NewGEP->setSourceElementType(NewBaseType->getPointerElementType());
      } else if (auto *NewPHI = dyn_cast<PHINode>(NewI)) {
        NewPHI->mutateType(NewBaseType);
      } else
        llvm_unreachable("Unexpected instruction encountered");
    }

    for (auto *I : ElementPtrGEP) {
      auto *NewI = cast<Instruction>((Value *)VMap[I]);
      Value *ReplGEP = nullptr;
      for (auto &U : I->uses()) {
        if (ElementPtrGEP.count(cast<Instruction>(U.getUser())))
          continue;
        assert(!isa<PHINode>(U.getUser()) &&
               "PHINode should be element of ElementPtrGEP");

        auto *NewU = cast<Instruction>((Value *)VMap[U.getUser()]);

        if (Copy)
          NewU = cast<Instruction>(OrigToCopy.find(NewU)->second);

        if (!ReplGEP) {
          assert(NewI->getParent()->getTerminator() != NewI &&
                 "Unexpected use of for element access");
          Builder.SetInsertPoint(
              isa<PHINode>(NewI) ? &*NewI->getParent()->getFirstInsertionPt()
                                 : NewI->getNextNonDebugInstruction());

          Value *Idx[] = {
              ConstantInt::get(Context, APInt(DL.getPointerSizeInBits(0), 0)),
              ConstantInt::get(Context, APInt(32, Offset))};

          ReplGEP =
              Builder.CreateInBoundsGEP(NewI, ArrayRef<Value *>(Idx), "elem");
        }
        NewU->replaceUsesOfWith(NewI, ReplGEP);
      }
    }
  }

private:
  const DataLayout &DL;
  const DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  ValueToValueMapTy &VMap;
  const ComputeArrayMethodClassification::TransformationData &InstsToTransform;
  LLVMContext &Context;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Offset of base pointer in DTransSOAToAOSApproxTypename.
extern cl::opt<unsigned> DTransSOAToAOSBasePtrOff;
ArraySummaryForIdiom
getParametersForSOAToAOSArrayMethodsCheckDebug(Function &F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Instructions to transform.
struct SOAToAOSArrayMethodsCheckDebugResult
    : public ComputeArrayMethodClassification::TransformationData {
  MethodKind MK = MK_Unknown;
};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace dtrans
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSARRAYS_H
