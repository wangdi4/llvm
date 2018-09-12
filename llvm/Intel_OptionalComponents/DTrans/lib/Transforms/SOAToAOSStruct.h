//===---------------- SOAToAOSStruct.h - Part of SOAToAOSPass -------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements functionality related specifically to structures
// containing arrays for SOA-to-AOS: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSSTRUCT_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSSTRUCT_H

#if !INTEL_INCLUDE_DTRANS
#error SOAToAOSStruct.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "SOAToAOSCommon.h"
#include "SOAToAOSEffects.h"

#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

#define DTRANS_SOASTR "dtrans-soatoaos-struct"

namespace llvm {
namespace dtrans {
namespace soatoaos {

struct StructIdioms : protected Idioms {
protected:
  static inline bool isNonStructArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;

    auto *Ty = (S.Method->arg_begin() + D->Const)->getType();
    // Simple check that Ty is not related to S.S
    if (isa<PointerType>(Ty))
      Ty = Ty->getPointerElementType();

    // Actually, only elements of array and integers are permitted.  Make sure
    // pointers to array of interest (only accessible through S.StrType) are
    // not forbidden.
    return isa<IntegerType>(Ty) || (isa<StructType>(Ty) && Ty != S.StrType);
  }

public:
  // All addresses to S.StrType should be dereferenced.
  static inline bool isStructuredExpr(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind == Dep::DK_Function) {
      for (auto *A : *D->Args)
        if (A->Kind == Dep::DK_Const)
          continue;
        else if (A->Kind == Dep::DK_Argument && isNonStructArg(A, S))
          continue;
        // Will check some time in isStructuredLoad.
        else if (A->Kind == Dep::DK_Load)
          continue;
        // Will check some time in isStructuredCall.
        else if (A->Kind == Dep::DK_Call)
          continue;
        // Will check some time in isStructuredCall.
        else if (A->Kind == Dep::DK_Alloc)
          continue;
        // Will check some time in isStructuredCall.
        else if (A->Kind == Dep::DK_Free)
          continue;
        else
          return false;
      return true;
    } else if (D->Kind == Dep::DK_Const)
      return true;
    else if (D->Kind == Dep::DK_Argument && isNonStructArg(D, S))
      return true;
    else if (D->Kind == Dep::DK_Load)
      return isStructuredLoad(D, S);
    else if (isStructuredCall(D, S))
      return true;
    return false;
  }

  // All loads should be structured in checkStructMethod
  // to avoid expensive recursion.
  //
  // All addresses to S.StrType should be dereferenced.
  // Non-nested loads should be structured, i.e. be explicit loads of field:
  // Load (GEP(..)).
  //
  // For arrays of interest there are only non-nested loads (uses are checked
  // in checkLoadOfStructField.
  //
  // Load (Load(Func(GEP))) are eventually permitted for accesses to fields not
  // related to transformed arrays, for example, to inlined non-transformed
  // arrays.
  static inline bool isStructuredLoad(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Load)
      return false;

    // Will check some time in isStructuredLoad,
    // shortcut to avoid deep recursion of nested loads.
    if (D->Arg1->Kind == Dep::DK_Load)
      return true;
    else if (isStructuredExpr(D->Arg1, S))
      return true;

    Type *OutType = nullptr;
    // Non-nested loads should be structured.
    // This is a crucial part of check for well-structured expressions.
    return Idioms::isFieldAddr(D->Arg1, S, OutType);
  }

  // All calls should be structured in checkStructMethod
  // to avoid expensive recursion.
  //
  // All addresses to S.StrType should be dereferenced.
  static inline bool isStructuredCall(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Call && D->Kind != Dep::DK_Alloc &&
        D->Kind != Dep::DK_Free)
      return false;

    if (!isStructuredExpr(D->Arg2, S))
      return false;

    if (D->Kind == Dep::DK_Alloc || D->Kind == Dep::DK_Free)
      if (!isStructuredExpr(D->Arg1, S))
        return false;

    return true;
  }

  // All addresses to S.StrType should be dereferenced.
  static inline bool isStructuredStore(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;
    // Uses of array fields are checked independently
    return isStructuredExpr(D->Arg1, S) && isStructuredExpr(D->Arg2, S);
  }

  static inline bool isFieldLoad(const Dep *D, unsigned Off, unsigned &ArgNo) {
    if (D->Kind != Dep::DK_Load)
      return false;

    unsigned OffOut = -1U;
    if (!Idioms::isArgAddr(D->Arg1, ArgNo, OffOut))
      return false;

    return OffOut == Off;
  }

  using Idioms::isFieldLoad;
};

// Utility class, see comments for static methods.
//
// We are relying that memory is not initialized before ctor, and all stores to
// fields are observed in methods.
class CtorDtorCheck {
public:
  // Checks whether 'this' argument of F points to non-initialized memory
  // from memory allocation routine.
  //
  // Implementation: checks that 'this' is returned from malloc.
  static inline bool isThisArgNonInitialized(const DTransAnalysisInfo &DTInfo,
                                             const Function *F) {
    // Simple wrappers only.
    if (!F->hasOneUse())
      return false;

    ImmutableCallSite CS(F->use_begin()->getUser());
    if (!CS)
      return false;

    auto *This = dyn_cast<Instruction>(CS.getArgument(0));
    if (!This)
      return false;

    // Extract 'this' actual argument.
    auto *Info =
        DTInfo.getCallInfo(cast<Instruction>(This->stripPointerCasts()));
    if (!Info)
      return false;

    if (Info->getCallInfoKind() != dtrans::CallInfo::CIK_Alloc)
      return false;

    auto AK = cast<AllocCallInfo>(Info)->getAllocKind();

    // Malloc, New, UserMalloc are OK: they return non-initialized memory.
    if (AK != AK_Malloc && AK != AK_New && AK != AK_UserMalloc)
      return false;

    return true;
  }

  // Checks that 'this' argument is dead after a call to F,
  // because it is passed to deallocation routine immediately after call to F.
  //
  // Implementation: checks that 'this' is used in free.
  static inline bool isThisArgIsDead(const DTransAnalysisInfo &DTInfo,
                                     const TargetLibraryInfo &TLI,
                                     const Function *F) {
    // Simple wrappers only.
    if (!F->hasOneUse())
      return false;

    ImmutableCallSite CS(F->use_begin()->getUser());
    if (!CS)
      return false;

    auto *ThisArg = dyn_cast<Instruction>(CS.getArgument(0));
    if (!ThisArg)
      return false;

    SmallPtrSet<const BasicBlock *, 2> DeleteBB;

    bool HasSameBBDelete = false;
    auto *BB = CS.getInstruction()->getParent();
    for (auto &U : ThisArg->uses()) {
      auto *V = &U;

      // stripPointerCasts from def to single use.
      if (auto *GEP = dyn_cast<GetElementPtrInst>(V->getUser())) {
        if (GEP->hasAllZeroIndices()) {
          if (!GEP->hasOneUse())
            return false;
          V = &*GEP->use_begin();
        }
      } else if (auto *BC = dyn_cast<BitCastInst>(V->getUser())) {
        if (!BC->hasOneUse())
          return false;
        V = &*BC->use_begin();
      }

      if (!isa<Instruction>(V->getUser()))
        return false;

      if (V->getUser() == CS.getInstruction())
        continue;

      auto *Inst = cast<Instruction>(V->getUser());
      bool IsSameBB = Inst->getParent() == BB;
      // If V follows CS in the same BB.
      bool IsBBSucc = false;
      if (IsSameBB) {
        IsBBSucc = std::find_if(CS.getInstruction()->getIterator(), BB->end(),
                                [Inst](const Instruction &I) -> bool {
                                  return &I == Inst;
                                }) != BB->end();
        // Uses before F's call are not relevant.
        if (!IsBBSucc)
          continue;
      }

      if (isFreedPtr(DTInfo, TLI, *V)) {
        if (IsSameBB && IsBBSucc) {
          HasSameBBDelete = true;
          continue;
        }
        DeleteBB.insert(Inst->getParent());
      }
      // Do not complicate analysis of successors.
      else if (find(successors(BB), Inst->getParent()) != succ_end(BB))
        return false;
    }

    // Simple CFG handling: all successors should contain delete.
    if (!HasSameBBDelete &&
        !all_of(successors(CS.getInstruction()->getParent()),
                [&DeleteBB](const BasicBlock *BB) -> bool {
                  return DeleteBB.find(BB) != DeleteBB.end();
                }))
      return false;

    return true;
  }

  // Returns true if U is a pointer to memory passed to deallocation routine.
  static inline bool isFreedPtr(const DTransAnalysisInfo &DTInfo,
                                const TargetLibraryInfo &TLI, const Use &U) {

    ImmutableCallSite CS(U.getUser());
    if (!CS)
      return false;

    auto *Info = DTInfo.getCallInfo(CS.getInstruction());
    if (!Info || Info->getCallInfoKind() != dtrans::CallInfo::CIK_Free)
      return false;

    SmallPtrSet<const Value *, 3> Args;
    collectSpecialFreeArgs(cast<FreeCallInfo>(Info)->getFreeKind(), CS, Args,
                           TLI);
    if (Args.size() != 1 || *Args.begin() != U.get())
      return false;

    return true;
  }
};

// Check properties of structure's method to SOA-to-AOS
// transformation.
class StructureMethodAnalysis : public StructIdioms {
public:
  using InstContainer = SmallSet<const Instruction *, 5>;

  struct TransformationData {
    // Loads/stores of pointers to elements.
    InstContainer ArrayInstToTransform;
  };

private:
  void insertArrayInst(const Instruction *I, const char *Desc) const {
    TI.ArrayInstToTransform.insert(I);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    InstDesc[I] = std::string("ArrayInst: ") + Desc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  }

  // Explicitly check non-nested load, i.e. direct loads of
  // pointer to arrays.
  bool checkLoadOfStructField(const LoadInst &L) const {

    auto *D = DM.getApproximation(&L);
    Type *FieldType = nullptr;
    // Not a direct load, but is structured
    if (!StructIdioms::isFieldLoad(D, S, FieldType))
      return true;

    // Not a pointer to array.
    if (!isa<PointerType>(FieldType))
      return true;

    // Layout restriction.
    // See
    // SOAToAOSTransformImpl::CandidateLayoutInfo::populateLayoutInformation.
    auto *StrType = cast<StructType>(FieldType->getPointerElementType());

    auto It = std::find(Fields.begin(), Fields.end(), StrType);
    // Access to non-interesting field.
    if (It == Fields.end())
      return true;

    for (auto &U : L.uses())
      if (auto CS = ImmutableCallSite(U.getUser())) {
        // CFG restriction, use can be only for 'this' argument.
        //
        // Safety check of CFG based on the 'this' argument.
        // See dtrans::StructInfo::CallSubGraph.
        //
        // Also see
        // SOAToAOSTransformImpl::CandidateLayoutInfo::populateLayoutInformation.
        if (isa<Function>(CS.getCalledValue())) {
          // insert optimistically
          insertArrayInst(CS.getInstruction(), "Array method call");
          continue;
        }
        return false;
      } else if (auto *Cmp = dyn_cast<CmpInst>(U.getUser())) {
        // nullptr check is not pointer escape,
        // see checkZeroInit also.
        if (!Cmp->isEquality())
          return false;

        bool IsNull = false;
        for (auto &Op : Cmp->operands())
          if (auto *C = dyn_cast<Constant>(Op.get()))
            if (C->isZeroValue())
              IsNull = true;
        if (IsNull) {
          // insert optimistically
          insertArrayInst(Cmp, "Null check");
          continue;
        }
        return false;
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(U.getUser())) {
        if (GEP->hasAllZeroIndices() && GEP->hasOneUse() &&
            CtorDtorCheck::isFreedPtr(DTInfo, TLI, *GEP->use_begin())) {
          // insert optimistically
          insertArrayInst(GEP, "cast to call deallocate");
          continue;
        }
        return false;
      } else
        return false;

    insertArrayInst(&L, "Load of array");

    return true;
  }

  bool checkStoreToField(const StoreInst &SI) const {
    auto *D = DM.getApproximation(SI.getPointerOperand());
    Type *FieldType = nullptr;
    // Unstructured store.
    if (!StructIdioms::isFieldAddr(D, S, FieldType))
      return false;

    // Not a pointer to array.
    if (!isa<PointerType>(FieldType))
      return true;

    auto *FPointeeTy = FieldType->getPointerElementType();

    // When callsite are analyzed,
    // stores of S.MemoryInterface should be analysed.
    // Instruction should be saved.
    if (FPointeeTy == S.MemoryInterface)
      return true;

    // Layout restriction.
    // See
    // SOAToAOSTransformImpl::CandidateLayoutInfo::populateLayoutInformation.
    auto *StrType = cast<StructType>(FPointeeTy);

    auto It = std::find(Fields.begin(), Fields.end(), StrType);
    // Access to non-interesting field.
    if (It == Fields.end())
      return true;

    // Check that value operand is associated with method allocation call:
    //  - value is from allocation call;
    //  - stored only to field;
    //  - method of array is called.
    //
    //  There are other patterns with different bitcast placements possible.
    //  %ptr = alloc
    //   %field = bitcast
    //   call %field
    //  store %ptr, ...  <= store to field using i8* type, it is 'SI'.
    auto *Val = SI.getValueOperand();
    if (!isa<Instruction>(Val))
      return false;

    if (!StructIdioms::isAlloc(DM.getApproximation(Val), S))
      return false;

    auto *IVal = cast<Instruction>(Val);
    if (!IVal->hasNUsesOrMore(2))
      return false;

    for (auto &U : IVal->uses())
      if (auto *BC = dyn_cast<BitCastInst>(U.getUser())) {
        if (!BC->hasNUses(1))
          return false;
        if (BC->getType() != StrType->getPointerTo())
          return false;

        if (auto CS = ImmutableCallSite(BC->use_begin()->getUser()))
          // Safety check of CFG based on the 'this' argument.
          // See dtrans::StructInfo::CallSubGraph.
          if (isa<Function>(CS.getCalledValue())) {
            // insert optimistically
            insertArrayInst(CS.getInstruction(), "Array method call");
            continue;
          }
        return false;
      }
      else if (U.getUser() == &SI)
        continue;
      else if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U)) {
        // insert optimistically
        insertArrayInst(cast<Instruction>(U.getUser()), "Deallocate");
        continue;
      } else
        return false;

    insertArrayInst(&SI, "Init ptr to array");
    return true;
  }

  // Checks that memset with 0 nullifies pointers to arrays.
  bool checkZeroInit(const MemSetInst &MI) const {
    Constant *L = dyn_cast<Constant>(MI.getLength());
    if (!L)
      return false;

    Constant *V = dyn_cast<Constant>(MI.getValue());
    if (!V || !V->isZeroValue())
      return false;

    uint64_t MaxOffset = L->getUniqueInteger().getLimitedValue();
    auto *SL = DL.getStructLayout(S.StrType);

    // Check that all pointers to arrays of interest are zeroed.
    // TODO:
    for (auto Off : Offsets)
      if (SL->getElementOffset(Off) + DL.getPointerSize(0) > MaxOffset)
        return false;
    return StructIdioms::isThisLikeArg(DM.getApproximation(MI.getDest()), S);
  }

  // Additional check of method call parameters.
  // Complements approximate dependency checks from DepCompute.
  // Helper method for checkStructMethod.
  //
  // Call to method should have argument dependent on integer fields or integer
  // arguments or be 'this' like arguments.
  bool checkMethodCall(ImmutableCallSite CS) const {
    assert(Idioms::isKnownCall(DM.getApproximation(CS.getInstruction()), S) &&
           "Incorrect checkMethodCall");

    for (auto &Op : CS.getInstruction()->operands()) {
      if (isa<Constant>(Op.get()) || isa<BasicBlock>(Op.get()))
        continue;

      const Dep *DO = DM.getApproximation(Op.get());
      // Additional checks for method call.
      if (!StructIdioms::isThisLikeArg(DO, S) &&
          !StructIdioms::isStructuredExpr(DO, S))
        return false;
    }
    return true;
  }

  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;
  const SummaryForIdiom &S;

  // Array types
  const SmallVector<StructType *, 3> &Fields;
  // Offsets of pointers to array types in S.StrType;
  const SmallVector<unsigned, 3> &Offsets;

  // Information computed for transformation.
  TransformationData &TI;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  mutable DenseMap<const Instruction *, std::string> InstDesc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  StructureMethodAnalysis(const StructureMethodAnalysis &) = delete;
  StructureMethodAnalysis &operator=(const StructureMethodAnalysis &) = delete;

public:
  StructureMethodAnalysis(const DataLayout &DL, DTransAnalysisInfo &DTInfo,
                          const TargetLibraryInfo &TLI, const DepMap &DM,
                          const SummaryForIdiom &S,
                          const SmallVector<StructType *, 3> &Fields,
                          const SmallVector<unsigned, 3> &Offsets,
                          TransformationData &TI)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), DM(DM), S(S), Fields(Fields),
        Offsets(Offsets), TI(TI) {}

  unsigned getTotal() const { return TI.ArrayInstToTransform.size(); }

  bool checkStructMethod() const {
    bool Invalid = false;

    for (auto &I : instructions(*S.Method)) {
      if (arith_inst_dep_iterator::isSupportedOpcode(I.getOpcode()))
        continue;

      auto *D = DM.getApproximation(&I);
      if (!D) {
        assert(
            !DTransSOAToAOSComputeAllDep &&
            "Not synchronized checkStructMethod and computeDepApproximation");
        return false;
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
      // Manipulations of pointer to arrays in Struct is processed
      // independently and directly, it should not affect control flow,
      // except for checks for nullptr.
      case Instruction::Br:
        break;
      case Instruction::Ret:
        if (StructIdioms::isStructuredExpr(D, S))
          break;
        Handled = false;
        break;
      case Instruction::Load:
        // All loads should be structured here to avoid
        // expensive recursion in isStructuredLoad/isStructuredCall
        if (StructIdioms::isStructuredLoad(D, S) &&
            // Explicitly check non-nested load, i.e. direct loads of
            // pointers to arrays.
            checkLoadOfStructField(cast<LoadInst>(I)))
          break;
        Handled = false;
        break;
      case Instruction::Store:
        // No writes to any Struct's object, but maybe to objects pointed to by
        // fields, or maybe to newly allocated memory.
        //
        // Also pointer to Struct does not escape.
        if (StructIdioms::isStructuredStore(D, S))
          break;
        // Explicitly check for Struct's field update.
        else if (checkStoreToField(cast<StoreInst>(I)))
          break;
        Handled = false;
        break;
      case Instruction::Unreachable:
        break;
      // Depends on invoke, no need to check.
      case Instruction::LandingPad:
        break;
      case Instruction::Call:
      case Instruction::Invoke:
      case Instruction::Alloca:
        // All calls should be structured here to avoid
        // expensive recursion in isStructuredLoad/isStructuredCall
        if (StructIdioms::isStructuredCall(D, S))
          break;
        else if (StructIdioms::isKnownCall(D, S)) {
          // Check if calls to Struct's method is structured.
          if (auto CS = ImmutableCallSite(&I))
            if (checkMethodCall(CS))
              break;
        } else if (StructIdioms::isStructuredStore(D, S))
          break;
        else if (auto *MI = dyn_cast<MemSetInst>(&I))
          if (checkZeroInit(*MI))
            break;
        Handled = false;
        break;
      case Instruction::Resume:
        break;
      default:
        Handled = false;
        break;
      }

      if (!Handled) {
        DEBUG_WITH_TYPE(DTRANS_SOADEP, {
          dbgs() << "; Unhandled " << I << "\n";
          D->dump();
        });
        Invalid = true;
      }
    }

    return !Invalid;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  class AnnotatedWriter : public AssemblyAnnotationWriter {
    const StructureMethodAnalysis &MC;

  public:
    AnnotatedWriter(const StructureMethodAnalysis &MC) : MC(MC) {
      assert(MC.InstDesc.size() == MC.TI.ArrayInstToTransform.size() &&
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
// Array types for structure containing arrays.
// Structure is specified in DTransSOAToAOSApproxTypename.
extern cl::list<std::string> DTransSOAToAOSArrays;
std::pair<SmallVector<StructType *, 3>, SmallVector<unsigned, 3>>
getArrayTypesForSOAToAOSStructMethodsCheckDebug(Function &F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos
} // namespace dtrans
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSSTRUCT_H
