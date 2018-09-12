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

#include "SOAToAOSArrays.h"
#include "SOAToAOSCommon.h"
#include "SOAToAOSEffects.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PatternMatch.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

#define DTRANS_SOASTR "dtrans-soatoaos-struct"

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Structure's methods in SOA can have different side-effects.
//
// Preliminary checks filter out not relevant side-effects from accesses to
// pointers to arrays.
//
// Details are in comment for isStructuredLoad.
// Non-nested load/stores are processed in StructureMethodAnalysis.
struct StructIdioms : protected Idioms {
protected:
  // All addresses to S.StrType should be dereferenced.
  static bool isStructuredExpr(const Dep *D, const SummaryForIdiom &S) {
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
  // in checkArrPtrLoadUses.
  //
  // Load (Load(Func(GEP))) are eventually permitted for accesses to fields not
  // related to transformed arrays, for example, to inlined non-transformed
  // arrays.
  static bool isStructuredLoad(const Dep *D, const SummaryForIdiom &S) {
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
  static bool isStructuredCall(const Dep *D, const SummaryForIdiom &S) {
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
  static bool isStructuredStore(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Store)
      return false;
    // Uses of array fields are checked independently
    return isStructuredExpr(D->Arg1, S) && isStructuredExpr(D->Arg2, S);
  }

  static bool isFieldLoad(const Dep *D, unsigned Off, unsigned &ArgNo) {
    if (D->Kind != Dep::DK_Load)
      return false;

    unsigned OffOut = -1U;
    if (!Idioms::isArgAddr(D->Arg1, ArgNo, OffOut))
      return false;

    return OffOut == Off;
  }

private:
  static bool isNonStructArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;

    auto *Ty = S.Method->getFunctionType()->getParamType(D->Const);
    // Simple check that Ty is not related to S.S
    if (isa<PointerType>(Ty))
      Ty = Ty->getPointerElementType();

    // For transformation, it is sufficient that only elements
    // of array and integers are permitted.
    //
    // For legality, it is necessary to prevent unchecked accesses
    // to pointers to arrays (only accessible through S.StrType).
    return isa<IntegerType>(Ty) || (isa<StructType>(Ty) && Ty != S.StrType);
  }
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
  // Implementation: checks that 'this' is returned from malloc with Size as
  // argument.
  static bool isThisArgNonInitialized(const DTransAnalysisInfo &DTInfo,
                                      const TargetLibraryInfo &TLI,
                                      const Function *F, unsigned Size) {
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
    auto *AllocCall = cast<Instruction>(This->stripPointerCasts());
    auto *Info = DTInfo.getCallInfo(AllocCall);
    if (!Info)
      return false;

    if (Info->getCallInfoKind() != dtrans::CallInfo::CIK_Alloc)
      return false;

    auto AK = cast<AllocCallInfo>(Info)->getAllocKind();

    // Malloc, New, UserMalloc are OK: they return non-initialized memory.
    if (AK != AK_Malloc && AK != AK_New && AK != AK_UserMalloc)
      return false;

    SmallPtrSet<const Value *, 3> Args;
    collectSpecialAllocArgs(cast<AllocCallInfo>(Info)->getAllocKind(),
                            ImmutableCallSite(AllocCall), Args, TLI);
    assert(Args.size() == 1 && "Unsupported allocation function");
    if (auto *C = dyn_cast<Constant>(*Args.begin()))
      if (C->getUniqueInteger().getLimitedValue() == Size)
        return true;

    return false;
  }

  // stripPointerCasts from def to single use.
  static const Use &skipPointerCasts(const Use &U) {
    auto *V = &U;
    while (true)
      // stripPointerCasts from def to single use.
      if (auto *GEP = dyn_cast<GetElementPtrInst>(V->getUser())) {
        if (!GEP->hasAllZeroIndices())
          return *V;
        if (!GEP->hasOneUse())
          return *V;
        V = &*GEP->use_begin();
      } else if (auto *BC = dyn_cast<BitCastInst>(V->getUser())) {
        if (!BC->hasOneUse())
          return *V;
        V = &*BC->use_begin();
      } else
        return *V;

    llvm_unreachable("Incorrect logic.");
  }

  // Checks that 'this' argument is dead after a call to CS,
  // because it is passed to deallocation routine immediately after call to CS.
  //
  // Implementation: checks that 'this' is used in free.
  static bool isThisArgIsDead(const DTransAnalysisInfo &DTInfo,
                              const TargetLibraryInfo &TLI,
                              ImmutableCallSite CS) {
    auto *F = CS.getCalledFunction();

    // Simple wrappers only.
    if (!F || !F->hasOneUse())
      return false;

    auto *ThisArg = dyn_cast<Instruction>(CS.getArgument(0));
    if (!ThisArg)
      return false;

    SmallPtrSet<const BasicBlock *, 2> DeleteBB;

    bool HasSameBBDelete = false;
    auto *BB = CS.getInstruction()->getParent();
    for (auto &TU : ThisArg->uses()) {
      auto &U = skipPointerCasts(TU);

      if (!isa<Instruction>(U.getUser()))
        return false;

      if (U.getUser() == CS.getInstruction())
        continue;

      auto *Inst = cast<Instruction>(U.getUser());
      bool IsSameBB = Inst->getParent() == BB;
      // If U follows CS in the same BB.
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

      if (isFreedPtr(DTInfo, TLI, U)) {
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
  static bool isFreedPtr(const DTransAnalysisInfo &DTInfo,
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

  // Returns
  //  - first  - true if there is a use in dellocation, false otherwise.
  //  - second - single use in method candidate, nullptr otherwise.
  static std::pair<bool, const ImmutableCallSite>
  isThereUseInFree(const DTransAnalysisInfo &DTInfo,
                   const TargetLibraryInfo &TLI, const Value *V) {
    bool FreeUseSeen = false;
    ImmutableCallSite SingleMethod;

    for (auto &TU : V->uses()) {
      auto &U = skipPointerCasts(TU);
      if (auto CS = ImmutableCallSite(U.getUser())) {
        if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U))
          FreeUseSeen = true;
        // Direct call.
        else if (CS.getCalledFunction()) {
          if (SingleMethod)
            return std::make_pair(FreeUseSeen, ImmutableCallSite());
          SingleMethod = CS;
        }
      }
    }
    return std::make_pair(FreeUseSeen, SingleMethod);
  }

  // Given V, returns single using method call (deallocation functions are
  // ignored), returns nullptr otherwise.
  static ImmutableCallSite getSingleMethodCall(const DTransAnalysisInfo &DTInfo,
                                               const TargetLibraryInfo &TLI,
                                               const Value *V) {
    return isThereUseInFree(DTInfo, TLI, V).second;
  }

  // Checks if V is a check for equality to null.
  // Non-constant operand returned.
  static const Value *isNullCheck(const Value *V) {
    Value *Other = nullptr;
    ICmpInst::Predicate Pred = CmpInst::ICMP_NE;
    // Unordered capture.
    if (PatternMatch::match(
            V, PatternMatch::m_c_ICmp(Pred, PatternMatch::m_Zero(),
                                      PatternMatch::m_Value(Other))) &&
        Pred == CmpInst::ICMP_EQ)
      return Other;
    return nullptr;
  }
};

// Check properties of structure's method to SOA-to-AOS
// transformation.
//
// Analyses load/stores to fields, which are pointers to arrays.
//  checkArrPtrLoadUses and checkArrPtrStoreUses are of main interest.
// Uses of loads and uses of stored value are analyzed.
//  In summary, uses are allowed in
//    1. method calls;
//    2. deallocation functions;
//    3. null checks.
//
// Analysis of relation of load/stores with respect to special array's method
// (ctors/etc) calls is completed in CallSiteComparator.
class StructureMethodAnalysis : public StructIdioms {
public:
  using InstContainer = SmallSet<const Instruction *, 32>;

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

  // Returns type array type accessed.
  // Returns nullptr if load is not related to array transformed.
  StructType *isLoadOrStoreOfArrayPtr(const Instruction &I) const {
    const Value *Address = nullptr;
    if (auto *L = dyn_cast<LoadInst>(&I))
      Address = L->getPointerOperand();
    else if (auto *SI = dyn_cast<StoreInst>(&I))
      Address = SI->getPointerOperand();
    else
      llvm_unreachable("Incorrect call of isLoadOrStoreOfArrayPtr.");

    auto *D = DM.getApproximation(Address);
    Type *FieldType = nullptr;
    // Unstructured store.
    if (!StructIdioms::isFieldAddr(D, S, FieldType))
      return nullptr;

    // Not a pointer to array.
    if (!isa<PointerType>(FieldType))
      return nullptr;

    auto *FPointeeTy = FieldType->getPointerElementType();

    // Layout restriction.
    // See
    // SOAToAOSTransformImpl::CandidateLayoutInfo::populateLayoutInformation.
    auto *ArrStrType = cast<StructType>(FPointeeTy);
    auto It = std::find(Fields.begin(), Fields.end(), ArrStrType);
    // Access to non-interesting field.
    if (It == Fields.end())
      return nullptr;

    return ArrStrType;
  }

  // Explicitly check non-nested load, i.e. direct loads of
  // pointer to arrays.
  //
  // Restrictions for load of pointer to transformed array:
  //  1. At least 1 use as 'this' argument of array's method.
  //  2. At most 1 use as 'this' argument if used in deallocation.
  //     Done in CallSiteComparator.
  //  3. If used in deallocation, then called method should be dtor.
  //  4. There could be check for nullptr.
  //
  // for transformed array should be used in array's method:
  //  (MethodCalled).
  bool checkArrPtrLoadUses(const LoadInst &L, StructType *StrType) const {
    assert(StrType && "Incorrect parameter");

    // Transformation restriction:
    // isSafeBitCast/isBitCastLikeGep/isSafeIntToPtr are not permitted.
    if (!isa<GetElementPtrInst>(L.getPointerOperand()))
      return false;

    // Check that all uses are related to array method calls,
    // to null-check or to call to deallocation.
    bool FreeUseSeen = false;
    SmallPtrSet<const Function *, 3> MethodsCalled;
    for (auto &LU : L.uses()) {
      auto &U = CtorDtorCheck::skipPointerCasts(LU);
      if (auto CS = ImmutableCallSite(U.getUser())) {
        if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U)) {
          FreeUseSeen = true;
          continue;
        }
        // Direct call.
        else if (auto FCalled = CS.getCalledFunction()) {
          // CFG restriction, use can be only for 'this' argument.
          //
          // Safety check of CFG based on the 'this' argument.
          // See dtrans::StructInfo::CallSubGraph.
          //
          // Also see populateLayoutInformation.
          //
          // No cast from load to method call.
          if (&U == &LU) {
            MethodsCalled.insert(FCalled);
            // insert optimistically
            insertArrayInst(CS.getInstruction(), "Array method call");
            continue;
          }
        } else
          return false;
      } else if (CtorDtorCheck::isNullCheck(U.getUser()))
        continue;
      else
        return false;
    }

    if (FreeUseSeen)
      if (MethodsCalled.size() != 1)
        return false;

    insertArrayInst(&L, "Load of array");
    return true;
  }

  // Function checks that pointer to newly allocated memory is stored to
  // pointer to array field. Uses of stored value is checked.
  //
  // Use of stored value in ctor is checked in CallSiteComparator.
  bool checkArrPtrStoreUses(const StoreInst &SI, StructType *StrType) const {
    assert(StrType && "Incorrect parameter");

    auto *Address = SI.getPointerOperand();
    // Transformation restriction:
    // isSafeIntToPtr are not permitted.
    if (!isa<GetElementPtrInst>(Address) && !isBitCastLikeGep(DL, Address) &&
        !isSafeBitCast(DL, Address))
      return false;

    // Check that value operand is associated with method allocation call:
    //  - value is from allocation call;
    //  - stored only to field;
    //  - single method of array is called (should be ctor).
    //
    // There are other patterns with different bitcast placements possible.
    //
    // Processed one:
    //  %ptr = alloc
    //   %field = bitcast
    //   call %field
    //  store %ptr, ...  <= store to field using i8* type, it is 'SI'.
    //
    // 1-1 correspondence between call %field and store is guaranteed here with
    // MethodSeen here.
    //
    // 1-1 correspondence between alloc and call %field is checked in
    // checkCtorsCallsAreAdjacent.
    auto *Val = SI.getValueOperand();
    if (!isa<Instruction>(Val))
      return false;

    if (!StructIdioms::isAlloc(DM.getApproximation(Val), S))
      return false;

    auto *IVal = cast<Instruction>(Val);
    if (!IVal->hasNUsesOrMore(2))
      return false;

    bool MethodSeen = false;
    for (auto &VU : IVal->uses()) {
      auto &U = CtorDtorCheck::skipPointerCasts(VU);
      if (auto CS = ImmutableCallSite(U.getUser())) {
        if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U))
          continue;
        // Use of stored value in ctor is checked in CallSiteComparator.
        else if (CS.getCalledFunction()) {
          // Store should be associated with single method, i.e. with ctor.
          // Called value is checked in CallSiteComparator.
          if (MethodSeen)
            return false;
          // insert optimistically
          insertArrayInst(CS.getInstruction(), "Array method call");
          MethodSeen = true;
          continue;
        } else
          return false;
      } else if (U.getUser() == &SI)
        continue;
      else
        return false;
    }

    // All stored should be associated with call to ctor-only.
    // Check is completed in canCallSitesBeMerged below.
    insertArrayInst(&SI, "Init ptr to array");
    return MethodSeen;
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

  StructureMethodAnalysis(const StructureMethodAnalysis &) = delete;
  StructureMethodAnalysis &operator=(const StructureMethodAnalysis &) = delete;

public:
  StructureMethodAnalysis(const DataLayout &DL,
                          const DTransAnalysisInfo &DTInfo,
                          const TargetLibraryInfo &TLI, const DepMap &DM,
                          const SummaryForIdiom &S,
                          const SmallVectorImpl<StructType *> &Fields,
                          // Needed for MemSetInst check.
                          const SmallVectorImpl<unsigned> &Offsets,
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

      if (D->isBottom())
        Invalid = true;

      if (Invalid && !DTransSOAToAOSComputeAllDep)
        break;

      // Synchronized with DepCompute::computeDepApproximation
      //
      // For each opcode, there are different cases processed,
      // if instruction is classified, then switch statement is exited,
      // otherwise Handled is cleared and diagnostic is printed.
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
        if (StructIdioms::isStructuredLoad(D, S)) {
          // Explicitly check non-nested load, i.e. direct loads of
          // pointers to arrays.
          if (auto *StrType = isLoadOrStoreOfArrayPtr(I)) {
            // Related to arrays of interest, limited kinds of uses
            // permitted. In particular, analysis of uses prevents nested loads
            // with respect to arrays of interest.
            if (checkArrPtrLoadUses(cast<LoadInst>(I), StrType))
              break;
          } else
            // Not related to arrays of interest of interest and not setting
            // Handled = false here.
            break;
        }
        Handled = false;
        break;
      case Instruction::Store:
        // No writes to any Struct's object, but maybe to objects pointed to by
        // fields, or maybe to newly allocated memory.
        //
        // Also pointer to Struct does not escape.
        if (StructIdioms::isStructuredStore(D, S))
          break;
        else if (StructIdioms::isMemoryInterfaceSetFromArg(D, S) ||
                 StructIdioms::isMemoryInterfaceCopy(D, S))
          break;
        // Explicitly check for Struct's field update.
        else if (auto *StrType = isLoadOrStoreOfArrayPtr(I)) {
          // Related to arrays of interest, limited kinds of uses
          // permitted.
          //
          // Check for usage in ctor is completed in CallSiteComparator.
          if (checkArrPtrStoreUses(cast<StoreInst>(I), StrType))
            break;
        } else
          // Not related to arrays of interest of interest and not setting
          // Handled = false here.
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
        }
        // memset of memory pointed-to by some field,
        // For example, memset of elements of array, which is not transformed.
        else if (StructIdioms::isStructuredStore(D, S))
          break;
        // Memset of structure itself.
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

private:
  const DataLayout &DL;
  const DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;

  // Summary for StructIdioms checks.
  const SummaryForIdiom &S;

  // Array types
  const SmallVectorImpl<StructType *> &Fields;
  // Offsets of pointers to array types in S.StrType;
  const SmallVectorImpl<unsigned> &Offsets;

  // Information computed for transformation.
  TransformationData &TI;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  mutable DenseMap<const Instruction *, std::string> InstDesc;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// Completes analysis of structure's methods:
//  - makes sure that combined array's combined methods have calls amenable to
//    combining;
//  - completes checks of load/stores with respect to special array's methods;
//  - checks that pointers to elements do not escape.
class CallSiteComparator : public StructIdioms {
public:
  using FunctionSet = SmallVector<const Function *, 3>;
  struct CallSitesInfo {
    FunctionSet Ctors;
    FunctionSet CCtors;
    FunctionSet Dtors;
    FunctionSet Appends;
  };

private:
  // Compares 2 append calls:
  // 1.
  //  call append(this1, elem1)
  // 2.
  //  call append(this2, elem2)
  //
  // this1 and this2 are loaded from the same instance of struct.
  bool compareAppendCallSites(ImmutableCallSite CS1, ImmutableCallSite CS2,
                              unsigned Off1, unsigned Off2) const {

    // Relying on layout checks in SOAToAOS.cpp.
    // TODO: avoid explicit computation here.
    StructType *Arr1 = cast<StructType>(
        S.StrType->getTypeAtIndex(Off1)->getPointerElementType());
    StructType *Arr2 = cast<StructType>(
        S.StrType->getTypeAtIndex(Off2)->getPointerElementType());

    assert(Arr1->indexValid(BasePtrOffset) && Arr2->indexValid(BasePtrOffset) &&
           "Incorrect base pointer for array types");

    Type *ElementType1 =
        Arr1->getTypeAtIndex(BasePtrOffset)->getPointerElementType();

    Type *ElementType2 =
        Arr2->getTypeAtIndex(BasePtrOffset)->getPointerElementType();

    // Same BasicBlock for combining.
    if (CS1.getInstruction()->getParent() != CS2.getInstruction()->getParent())
      return false;

    for (auto P : zip_first(CS1.args(), CS2.args())) {
      auto *A1 = std::get<0>(P).get();
      auto *A2 = std::get<1>(P).get();
      if (A1 == A2)
        continue;
      bool IsElemA1 = A1->getType() == ElementType1;
      bool IsElemA2 = A2->getType() == ElementType2;
      if (IsElemA1 != IsElemA2)
        return false;
      if (IsElemA1)
        continue;
      if (!isa<PointerType>(A1->getType()) || !isa<PointerType>(A2->getType()))
        return false;

      auto *PTy1 = A1->getType()->getPointerElementType();
      auto *PTy2 = A2->getType()->getPointerElementType();
      IsElemA1 = PTy1 == ElementType1;
      IsElemA2 = PTy2 == ElementType2;
      if (IsElemA1 != IsElemA2)
        return false;
      if (IsElemA1)
        continue;

      auto *D1 = DM.getApproximation(A1);
      auto *D2 = DM.getApproximation(A2);
      assert(D1 && D2 && "Approximation was not computed completely");
      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      if (!StructIdioms::isFieldLoad(D1, Off1, ArgNo1) ||
          !StructIdioms::isFieldLoad(D2, Off2, ArgNo2) ||
          // Expect 'this' parameter in usual place.
          ArgNo1 != 0 || ArgNo2 != 0)
        return false;

      auto *L1 = dyn_cast<LoadInst>(A1);
      auto *L2 = dyn_cast<LoadInst>(A2);
      if (!L1 || !L2)
        return false;

      if (CS1.getInstruction()->getParent() != L1->getParent())
        return false;

      if (CS2.getInstruction()->getParent() != L2->getParent())
        return false;

    }
    return true;
  }

  // Single BasicBlock:
  //  this1 = str->arr1
  //  call append(this1, elem1)
  //  this2 = str->arr2
  //  call append(this2, elem2)
  bool compareAllAppendCallSites() const {
    SmallVector<ImmutableCallSite, 3> CSs;
    for (auto *F : CSInfo.Appends) {
      if (F->hasOneUse() && F == S.Method)
        CSs.push_back(ImmutableCallSite(F->use_begin()->getUser()));
      else if (F->hasNUsesOrMore(2))
        return false;
    }

    if (CSs.size() == 0)
      return true;

    // There could be one call for each field or none at all. Do not permit
    // multiple groups of append methods in a single Struct's method. It
    // simplifies transformation.
    if (CSs.size() != Fields.size())
      return false;

    auto CSPivot = CSs[0];
    auto OffPivot = Offsets[0];

    for (auto P : zip(make_range(CSs.begin() + 1, CSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end()))) {
      auto CS = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareAppendCallSites(CSPivot, CS, OffPivot, Off))
        return false;
    }

    // There should be no stores in common basic block.
    for (auto &I : *CSPivot.getInstruction()->getParent())
      switch (I.getOpcode()) {
      case Instruction::Load:
      case Instruction::GetElementPtr:
      case Instruction::BitCast:
      case Instruction::ICmp:
      case Instruction::Br:
        continue;
      case Instruction::Call:
        if (std::find_if(CSs.begin(), CSs.end(),
                         [&I](ImmutableCallSite CS) -> bool {
                           return &I == CS.getInstruction();
                         }) == CSs.end())
          return false;
        continue;
      default:
        return false;
      }
    return true;
  }

  // Compare calls to deallocation function.
  //
  // FreePtr1 and FreePtr2 are pointers to memory to deallacate (difference
  // ignored).
  //
  // Remaining arguments are equal or loads of MemoryInterface.
  bool compareDeleteCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                          const Value *FreePtr1, const Value *FreePtr2) const {

    auto *Info = DTInfo.getCallInfo(CS1.getInstruction());
    if (!Info || Info->getCallInfoKind() != dtrans::CallInfo::CIK_Free)
      return false;

    if (CS1.getCalledValue() != CS2.getCalledValue())
      return false;

    SmallPtrSet<const Value *, 3> Args;
    collectSpecialFreeArgs(cast<FreeCallInfo>(Info)->getFreeKind(), CS1, Args,
                           TLI);

    assert(Args.size() == 1 && "Unexpected deallocation function");

    for (auto PA : zip_first(CS1.args(), CS2.args())) {
      auto *A1 = std::get<0>(PA).get();
      auto *A2 = std::get<1>(PA).get();
      if (Args.count(A1)) {
        if (A1 != FreePtr1 || A2 != FreePtr2)
          return false;
      } else if (A1 != A2) {
        auto *D1 = DM.getApproximation(A1);
        auto *D2 = DM.getApproximation(A2);
        // TODO: may want to check that loads are from the same instance of
        // struct.
        if (!D1 || !D2 || !StructIdioms::isMemoryInterfaceFieldLoad(D1, S) ||
            !StructIdioms::isMemoryInterfaceFieldLoad(D2, S))
          return false;
      }
    }
    return true;
  }

  // BasicBlock in clean up may contain:
  //    - landing pad, processing simple processing of caught exception
  //    - (extractvalue) and
  //    - invoke of 'delete' function.
  //
  //  %tmp7 = landingpad { i8*, i32 }
  //          cleanup
  //  %tmp8 = extractvalue { i8*, i32 } %tmp7, 0
  //  %tmp9 = extractvalue { i8*, i32 } %tmp7, 1
  //  ; Special type of bitcast.
  //  %ptr = getelementptr inbounds %class, %class* %this, i64 0, i32 0
  //  invoke void @"XMemory::operator delete(void*)"(i8* %ptr)
  //          to label %resume unwind label %terminate
  bool compareCtorDtorCleanupBBs(const BasicBlock *BB1, const BasicBlock *BB2,
                                 const Value *FreePtr1,
                                 const Value *FreePtr2) const {
    if (pred_size(BB1) != 1)
      return false;

    if (pred_size(BB2) != 1)
      return false;

    if (succ_size(BB1) != 2)
      return false;

    if (succ_size(BB2) != 2)
      return false;

    if (BB1->size() != BB2->size())
      return false;

    for (auto P : zip_first(successors(BB1), successors(BB2))) {
      // Exception paths have the same common successor blocks,
      // which results in immediate return from dtors caller.
      if (std::get<0>(P) != std::get<1>(P))
        return false;
      // Exception path ends in returns from dtors caller.
      // Actually, resume instruction or noreturn function call.
      if (succ_empty(std::get<0>(P)))
        continue;
      // Successors should only lead to function exits.
      for (auto *S : post_order(std::get<0>(P)))
        if (S == BB1 || S == BB2)
          return false;
    }

    DenseMap<const Value *, const Value *> ValueRemapper;
    ValueRemapper[FreePtr1] = FreePtr2;
    // Compare cleanup BBs.
    for (auto P : zip_first(*BB1, *BB2)) {
      auto &I1 = std::get<0>(P);
      auto &I2 = std::get<1>(P);

      if (I1.getOpcode() != I2.getOpcode())
        return false;

      switch (I1.getOpcode()) {
      case Instruction::LandingPad:
      case Instruction::ExtractValue:
      case Instruction::GetElementPtr:
      case Instruction::Invoke:
        break;
      default:
        return false;
      }

      bool Done = false;
      if (auto GEP1 = dyn_cast<GetElementPtrInst>(&I1)) {
        auto GEP2 = cast<GetElementPtrInst>(&I2);
        if (GEP1->hasAllZeroIndices() && GEP2->hasAllZeroIndices() &&
            GEP1->getPointerOperand() == FreePtr1 &&
            GEP2->getPointerOperand() == FreePtr2) {
          FreePtr1 = GEP1;
          FreePtr2 = GEP2;
          Done = true;
        }
      }

      // Delete invocations are processed completely.
      if (auto CS1 = ImmutableCallSite(&I1)) {
        if (!compareDeleteCalls(CS1, ImmutableCallSite(&I2), FreePtr1,
                                FreePtr2))
          return false;
        Done = true;
      }

      if (!Done)
        for (auto PO :
             zip_first(I1.operands(), I2.operands())) {
          auto *Op1 = std::get<0>(PO).get();
          auto *Op2 = std::get<1>(PO).get();
          if (Op1 != Op2 && ValueRemapper[Op1] != Op2)
            return false;
        }

      if (I1.getNumUses() != I2.getNumUses())
        return false;

      for (auto PU : zip_first(I1.uses(), I2.uses())) {
        auto *U1 = dyn_cast<Instruction>(std::get<0>(PU).getUser());
        auto *U2 = dyn_cast<Instruction>(std::get<1>(PU).getUser());
        if (!U1 || !U2)
          return false;
        if (U1->getParent() == BB1 && U2->getParent() == BB2)
          continue;
        if (U1 == U2 && isa<PHINode>(U1))
          continue;
        return false;
      }

      ValueRemapper[&I1] = &I2;
    }
    return true;
  }

  // Compare 2 invokes of regular ctors.
  //
  // 'this' parameters are from allocation function and non-initialized.
  // Remaining arguments are equal or loads of MemoryInterface.
  //
  // Cleanup BasicBlock should be equal as in compareCtorDtorCleanupBBs.
  bool compareCtorCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                        unsigned Off1, unsigned Off2) const {
    // Relying on layout checks in SOAToAOS.cpp.
    // TODO: avoid explicit computation here.
    StructType *Arr1 = cast<StructType>(
        S.StrType->getTypeAtIndex(Off1)->getPointerElementType());
    StructType *Arr2 = cast<StructType>(
        S.StrType->getTypeAtIndex(Off2)->getPointerElementType());
    if (!CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, dyn_cast<Function>(CS1.getCalledValue()),
            DL.getTypeAllocSize(Arr1)))
      return false;
    if (!CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, dyn_cast<Function>(CS2.getCalledValue()),
            DL.getTypeAllocSize(Arr2)))
      return false;

    // Paramters should be equal as Values with 2 exceptions:
    //  - 'this' parameter (checked above);
    //  - memory interface access (relying on fact it is only copied around).
    bool ThisArg = true;
    for (auto P : zip_first(CS1.args(), CS2.args())) {
      if (ThisArg) {
        ThisArg = false;
        continue;
      }

      auto *A1 = std::get<0>(P).get();
      auto *A2 = std::get<1>(P).get();

      if (A1 == A2)
        continue;

      auto *D1 = DM.getApproximation(A1);
      auto *D2 = DM.getApproximation(A2);
      // TODO: may want to check that loads are from the same instance of
      // struct.
      if (!D1 || !D2 || !StructIdioms::isMemoryInterfaceFieldLoad(D1, S) ||
          !StructIdioms::isMemoryInterfaceFieldLoad(D2, S))
        return false;
    }

    auto *Inv1 = dyn_cast<InvokeInst>(CS1.getInstruction());
    auto *Inv2 = dyn_cast<InvokeInst>(CS2.getInstruction());

    if ((Inv1 != nullptr) != (Inv2 != nullptr))
      return false;

    if (!Inv1)
      return true;

    auto *CleanBB1 = Inv1->getUnwindDest();
    auto *CleanBB2 = Inv2->getUnwindDest();
    auto *FreePtr1 = CS1.getArgOperand(0)->stripPointerCasts();
    auto *FreePtr2 = CS2.getArgOperand(0)->stripPointerCasts();
    if (!compareCtorDtorCleanupBBs(CleanBB1, CleanBB2, FreePtr1, FreePtr2))
      return false;

    return true;
  }

  // Compare invokes of ctors and checks adjacency of invokes.
  bool compareAllCtorCallSites() const {
    SmallVector<ImmutableCallSite, 3> CtorCSs;
    for (auto *F : CSInfo.Ctors) {
      if (F->hasOneUse() && F == S.Method)
        CtorCSs.push_back(ImmutableCallSite(F->use_begin()->getUser()));
      else if (F->hasNUsesOrMore(2))
        return false;
    }

    if (CtorCSs.size() == 0)
      return true;

    // There could be one call for each field or none at all.
    if (CtorCSs.size() != Fields.size())
      return false;

    auto CSPivot = CtorCSs[0];
    auto OffPivot = Offsets[0];

    for (auto P : zip(make_range(CtorCSs.begin() + 1, CtorCSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end()))) {
      auto CS = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareCtorCalls(CSPivot, CS, OffPivot, Off))
        return false;
    }

    if (!checkCtorsCallsAreAdjacent(CtorCSs))
      return false;
    return true;
  }

  // Compare 2 invokes of copy ctors.
  //
  // 'this' parameters are from allocation function and non-initialized.
  // Second arguments are loads from instance of struct (argument #1).
  //
  // Cleanup BasicBlock should be equal as in compareCtorDtorCleanupBBs.
  bool compareCCtorCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                         unsigned Off1, unsigned Off2) const {
    // Relying on layout checks in SOAToAOS.cpp.
    // TODO: avoid explicit computation here.
    StructType *Arr1 = cast<StructType>(
        S.StrType->getTypeAtIndex(Off1)->getPointerElementType());
    StructType *Arr2 = cast<StructType>(
        S.StrType->getTypeAtIndex(Off2)->getPointerElementType());
    if (!CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, dyn_cast<Function>(CS1.getCalledValue()),
            DL.getTypeAllocSize(Arr1)))
      return false;
    if (!CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, dyn_cast<Function>(CS2.getCalledValue()),
            DL.getTypeAllocSize(Arr2)))
      return false;

    if (CS1.arg_size() != 2 || CS2.arg_size() != 2)
      return false;

    // Compare second argument.
    {
      auto *A1 = CS1.getArgOperand(1);
      auto *A2 = CS2.getArgOperand(1);
      auto *D1 = DM.getApproximation(A1);
      auto *D2 = DM.getApproximation(A2);

      assert(D1 && D2 && "Approximation was not computed completely");
      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      if (!StructIdioms::isFieldLoad(D1, Off1, ArgNo1) ||
          !StructIdioms::isFieldLoad(D2, Off2, ArgNo2) ||
          ArgNo1 != 1 || ArgNo2 != 1)
        return false;
    }

    auto *Inv1 = dyn_cast<InvokeInst>(CS1.getInstruction());
    auto *Inv2 = dyn_cast<InvokeInst>(CS2.getInstruction());

    if ((Inv1 != nullptr) != (Inv2 != nullptr))
      return false;

    if (!Inv1)
      return true;

    auto *CleanBB1 = Inv1->getUnwindDest();
    auto *CleanBB2 = Inv2->getUnwindDest();
    auto *FreePtr1 = CS1.getArgOperand(0)->stripPointerCasts();
    auto *FreePtr2 = CS2.getArgOperand(0)->stripPointerCasts();
    if (!compareCtorDtorCleanupBBs(CleanBB1, CleanBB2, FreePtr1, FreePtr2))
      return false;

    return true;
  }

  // Compare invokes of copy ctors and checks adjacency of invokes.
  bool compareAllCCtorCallSites() const {
    SmallVector<ImmutableCallSite, 3> CCtorCSs;
    for (auto *F : CSInfo.CCtors) {
      if (F->hasOneUse() && F == S.Method)
        CCtorCSs.push_back(ImmutableCallSite(F->use_begin()->getUser()));
      else if (F->hasNUsesOrMore(2))
        return false;
    }

    if (CCtorCSs.size() == 0)
      return true;

    // There could be one call for each field or none at all.
    if (CCtorCSs.size() != Fields.size())
      return false;

    auto CSPivot = CCtorCSs[0];
    auto OffPivot = Offsets[0];

    for (auto P : zip(make_range(CCtorCSs.begin() + 1, CCtorCSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end()))) {
      auto CS = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareCCtorCalls(CSPivot, CS, OffPivot, Off))
        return false;
    }

    if (!checkCtorsCallsAreAdjacent(CCtorCSs))
      return false;

    return true;
  }

  // Compare 2 invokes of copy ctors.
  //
  // 'this' parameters is dead in all paths after invoke (passed to
  // deallocation function).
  //
  // Arguments are from the same instance of struct.
  //
  // Cleanup BasicBlock should be equal as in compareCtorDtorCleanupBBs.
  bool compareDtorCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                         unsigned Off1, unsigned Off2) const {
    if (!CtorDtorCheck::isThisArgIsDead(DTInfo, TLI, CS1))
      return false;
    if (!CtorDtorCheck::isThisArgIsDead(DTInfo, TLI, CS2))
      return false;

    if (CS1.arg_size() != 1 || CS2.arg_size() != 1)
      return false;

    // Compare argument.
    {
      auto *A1 = CS1.getArgOperand(0);
      auto *A2 = CS2.getArgOperand(0);
      auto *D1 = DM.getApproximation(A1);
      auto *D2 = DM.getApproximation(A2);

      assert(D1 && D2 && "Approximation was not computed completely");
      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      if (!StructIdioms::isFieldLoad(D1, Off1, ArgNo1) ||
          !StructIdioms::isFieldLoad(D2, Off2, ArgNo2) ||
          ArgNo1 != 0 || ArgNo2 != 0)
        return false;
    }

    auto *Inv1 = dyn_cast<InvokeInst>(CS1.getInstruction());
    auto *Inv2 = dyn_cast<InvokeInst>(CS2.getInstruction());

    if ((Inv1 != nullptr) != (Inv2 != nullptr))
      return false;

    if (!Inv1)
      return true;

    auto *CleanBB1 = Inv1->getUnwindDest();
    auto *CleanBB2 = Inv2->getUnwindDest();
    auto *FreePtr1 = CS1.getArgOperand(0)->stripPointerCasts();
    auto *FreePtr2 = CS2.getArgOperand(0)->stripPointerCasts();
    if (!compareCtorDtorCleanupBBs(CleanBB1, CleanBB2, FreePtr1, FreePtr2))
      return false;

    return true;
  }

  // Checks that dtor calls are arranged in a way allowing for combining.
  // if (ptr1) <= is not checked.
  //  invoke dtor1(ptr1)
  //    next1, cleanup1
  //  next1:
  //    call delete ptr1
  // if (ptr2)
  //  invoke dtor2(ptr2)
  //    next2, cleanup2
  //  next2:
  //    call delete ptr2
  // ..
  // ..
  // cleanup1:
  //  ; processing caught object,
  //  ; producing the values for resume and terminate blocks
  //  invoke delete ptr1
  //    resume, terminate
  //
  // cleanup2:
  //  ; processing caught object,
  //  ; producing the values for resume and terminate blocks
  //  invoke delete ptr2
  //    resume, terminate
  bool compareAllDtorCallSites() const {
    SmallVector<ImmutableCallSite, 3> DtorCSs;
    for (auto *F : CSInfo.Dtors) {
      if (F->hasOneUse() && F == S.Method)
        DtorCSs.push_back(ImmutableCallSite(F->use_begin()->getUser()));
      else if (F->hasNUsesOrMore(2))
        return false;
    }

    // There could be one call for each field or none at all.
    if (DtorCSs.size() != 0 && DtorCSs.size() != Fields.size())
      return false;

    if (DtorCSs.size() == 0)
      return true;

    auto CSPivot = DtorCSs[0];
    auto OffPivot = Offsets[0];

    for (auto P : zip(make_range(DtorCSs.begin() + 1, DtorCSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end()))) {
      auto CS = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareDtorCalls(CSPivot, CS, OffPivot, Off))
        return false;
    }

    if (!checkDtorsCallsAreAdjacent(DtorCSs))
      return false;

    return true;
  }

  // Topologically first/last under assumption that there are no cycles in BBs
  // (to be checked separately afterward).
  static std::pair<const BasicBlock *, const BasicBlock *>
  getTopSortFirstLastBB(SmallPtrSetImpl<const BasicBlock *> &BBs) {
    if (BBs.empty())
      return std::pair<const BasicBlock *, const BasicBlock *>(nullptr, nullptr);

    unsigned Curr = 0;
    const BasicBlock *First = nullptr;
    const BasicBlock *Last = nullptr;

    for (auto *S : post_order(&(*BBs.begin())->getParent()->getEntryBlock())) {
      auto It = BBs.find(S);
      if (It == BBs.end())
        continue;
      if (Curr == 0)
        Last = S;
      if (Curr + 1 == BBs.size())
        First = S;
      ++Curr;
    }
    assert(BBs.size() == Curr && "Logic is broken");
    return std::make_pair(First, Last);
  }

  // Checks whether invokes of ctor or cctors are adjacent.
  //
  // Find first and last basic block in sequence and check all instructions on
  // intermediate BasicBlock during normal execution.
  //
  // Exception handling of ctors is checked in compareCtorDtorCleanupBBs.
  bool checkCtorsCallsAreAdjacent(
      const SmallVector<ImmutableCallSite, 3> CtorCSs) const {
    SmallPtrSet<const BasicBlock *, 6> BBs;
    SmallVector<ImmutableCallSite, 3> NewCalls;
    const BasicBlock *AllocLandingPad = nullptr;
    // BasicBlock with allocation calls
    for (auto CS : CtorCSs) {
      BBs.insert(CS.getInstruction()->getParent());
      // Relying on CtorDtorCheck::isThisArgNonInitialized
      auto *AllocCall =
          cast<Instruction>(CS.getArgOperand(0)->stripPointerCasts());
      NewCalls.push_back(ImmutableCallSite(AllocCall));
      if (auto *Inv = dyn_cast<InvokeInst>(AllocCall)) {
        if (!AllocLandingPad)
          AllocLandingPad = Inv->getUnwindDest();
        // The same handler for allocation calls.
        if (AllocLandingPad != Inv->getUnwindDest())
          return false;
        BBs.insert(AllocCall->getParent());
      }
    }

    auto FL = getTopSortFirstLastBB(BBs);

    unsigned NumAllocCalls = 0;
    unsigned NumCtorCalls = 0;
    for (auto *BB = FL.first, *NextBB = BB /*Initial value ignored*/; BB;
         BB = NextBB /*See body of loop, Invoke case*/) {
      NextBB = nullptr;
      // Ignore additional instructions in first and last BB.
      for (auto &I : *BB)
        switch (I.getOpcode()) {
        case Instruction::BitCast:
        case Instruction::GetElementPtr:
        case Instruction::Load:
        case Instruction::Store:
          continue;
        case Instruction::Invoke:
          // No control flow cycles are possible.
          NextBB = cast<InvokeInst>(&I)->getNormalDest();

          LLVM_FALLTHROUGH;
        case Instruction::Call:
          if (std::find_if(CtorCSs.begin(), CtorCSs.end(),
                           [&I](ImmutableCallSite CS) -> bool {
                             return &I == CS.getInstruction();
                           }) != CtorCSs.end()) {
            ++NumCtorCalls;
            continue;
          }
          if (std::find_if(NewCalls.begin(), NewCalls.end(),
                           [&I](ImmutableCallSite CS) -> bool {
                             return &I == CS.getInstruction();
                           }) != NewCalls.end()) {
            ++NumAllocCalls;
            continue;
          }
          return false;
        default:
          return false;
        }

      if (BB == FL.second)
        NextBB = nullptr;
    }

    // Make sure there is 1-1 correspondence between
    //  - allocation calls,
    //  - ctors
    //
    // 1-1 correspondence between ctor and stores is checked in
    // checkArrPtrStoreUses.
    if (NumAllocCalls != NewCalls.size() || NumCtorCalls != CtorCSs.size())
      return false;

    return true;
  }

  // Checks whether invokes of dtor are adjacent.
  //
  // Find first and last basic block in sequence and check all instructions on
  // intermediate BasicBlock during normal execution.
  //
  // Checks for null are also handled.
  //
  // Exception handling of dtors is checked in compareCtorDtorCleanupBBs.
  bool checkDtorsCallsAreAdjacent(
      const SmallVector<ImmutableCallSite, 3> DtorCSs) const {

    SmallPtrSet<const BasicBlock *, 6> BBs;
    SmallPtrSet<const BasicBlock *, 3> DtorCalls;
    for (auto CS : DtorCSs) {
      auto *ParentBB = CS.getInstruction()->getParent();
      BBs.insert(ParentBB);
      DtorCalls.insert(ParentBB);
      auto *DeleteBB  = ParentBB;
      if (auto *Inv = dyn_cast<InvokeInst>(CS.getInstruction())) {
        // Expect normal delete call in next BB.
        DeleteBB = Inv->getNormalDest();
        BBs.insert(DeleteBB);
      }
    }

    auto FL = getTopSortFirstLastBB(BBs);
    unsigned NumDtorCalls = 0;
    unsigned NumDeleteCalls = 0;
    for (auto *BB = FL.first, *NextBB = BB /*Initial value ignored*/; BB;
         BB = NextBB /*See body of loop, Invoke/Br case*/) {
      NextBB = nullptr;
      // Ignore additional instructions in first and last BB.
      for (auto &I : *BB)
        switch (I.getOpcode()) {
        case Instruction::GetElementPtr:
          continue;
        case Instruction::Load:
          continue;
        case Instruction::ICmp:
          if (auto *Other = CtorDtorCheck::isNullCheck(&I)) {
            auto &Cmp = cast<ICmpInst>(I);

            if (!Cmp.hasOneUse())
              return false;

            auto *Br = dyn_cast<BranchInst>(Cmp.use_begin()->getUser());
            if (!Br || Br->getParent() != BB)
              return false;

            // Check that Other is related to dtor call.
            // Simple check, no reloads.
            auto MethodCS = CtorDtorCheck::getSingleMethodCall(DTInfo, TLI, Other);

            if (std::find_if(DtorCSs.begin(), DtorCSs.end(),
                             [MethodCS](ImmutableCallSite CS) -> bool {
                               return MethodCS.getInstruction() ==
                                      CS.getInstruction();
                             }) != DtorCSs.end())
              continue;
          }
          return false;
        case Instruction::Br: {
          auto &Br = cast<BranchInst>(I);
          if (!Br.isConditional()) {
            NextBB = Br.getSuccessor(0);
            continue;
          }
          // Check relation with previous case in switch.
          if (auto *Cmp = dyn_cast<ICmpInst>(Br.getCondition())) {
            if (Cmp->getParent() != BB)
              return false;
          } else
            return false;

          // FL.first already calls dtor.
          // From checkZeroInit() and checkArrPtrStoreUses,
          // we may deduce that null-checks are redundant.
          //
          // Relying on ICMP_EQ predicate.
          NextBB = Br.getSuccessor(1);
          if (DtorCalls.find(NextBB) == DtorCalls.end() &&
              NextBB != FL.second->getUniqueSuccessor())
            return false;

          continue;
        }
        case Instruction::Invoke:
          // No control flow cycles are possible.
          NextBB = cast<InvokeInst>(&I)->getNormalDest();

          LLVM_FALLTHROUGH;
        case Instruction::Call:
          if (std::find_if(DtorCSs.begin(), DtorCSs.end(),
                           [&I](ImmutableCallSite CS) -> bool {
                             return &I == CS.getInstruction();
                           }) != DtorCSs.end()) {
            ++NumDtorCalls;
            continue;
          }

          {
            auto *Info = DTInfo.getCallInfo(&I);
            if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free) {
              ++NumDeleteCalls;
              continue;
            }
          }
          return false;
        default:
          return false;
        }
      if (BB == FL.second)
        NextBB = nullptr;
    }

    // One delete for each dtor call.
    // And following regular path we should encounter all dtors.
    if (NumDtorCalls != DtorCSs.size() || NumDeleteCalls != DtorCSs.size())
      return false;
    return true;
  }

  CallSiteComparator(const CallSiteComparator &) = delete;
  CallSiteComparator &operator=(const CallSiteComparator &) = delete;

  const DataLayout &DL;
  const DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;
  const SummaryForIdiom &S;

  // Array types
  const SmallVectorImpl<StructType *> &Fields;
  // Offsets of pointers to array types in S.StrType;
  const SmallVectorImpl<unsigned> &Offsets;

  // Information computed for transformation.
  const StructureMethodAnalysis::TransformationData &TI;

  const CallSitesInfo &CSInfo;
  unsigned BasePtrOffset;

public:
  CallSiteComparator(const DataLayout &DL, const DTransAnalysisInfo &DTInfo,
                     const TargetLibraryInfo &TLI, const DepMap &DM,
                     const SummaryForIdiom &S,
                     const SmallVectorImpl<StructType *> &Fields,
                     const SmallVectorImpl<unsigned> &Offsets,
                     const StructureMethodAnalysis::TransformationData &TI,
                     const CallSitesInfo &CSInfo, unsigned BasePtrOffset)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), DM(DM), S(S), Fields(Fields),
        Offsets(Offsets), TI(TI), CSInfo(CSInfo), BasePtrOffset(BasePtrOffset) {
  }

  bool canCallSitesBeMerged() const {
    // Check categories of methods called, finalizing uses checks of
    // loads/stores in StructureMethodAnalysis.
    for (auto *I : TI.ArrayInstToTransform)
      if (auto *SI = dyn_cast<StoreInst>(I)) {
        // Store should be processed checkArrPtrStoreUses
        auto *FCalled = CtorDtorCheck::getSingleMethodCall(
            DTInfo, TLI, SI->getValueOperand()).getCalledFunction();
        // Not ctor call, hence cannot merge.
        if (std::find_if(CSInfo.Ctors.begin(), CSInfo.Ctors.end(),
                         [FCalled](const Function *FCtor) -> bool {
                           return FCalled == FCtor;
                         }) == CSInfo.Ctors.end() &&
            std::find_if(CSInfo.CCtors.begin(), CSInfo.CCtors.end(),
                         [FCalled](const Function *FCtor) -> bool {
                           return FCalled == FCtor;
                         }) == CSInfo.CCtors.end())
          return false;
      } else if (auto *L = dyn_cast<LoadInst>(I)) {
        auto IsUsed = CtorDtorCheck::isThereUseInFree(DTInfo, TLI, L);
        // Load is used in deallocation
        if (IsUsed.first) {
          assert(IsUsed.second && "StructureMethodAnalysis was not run");
          auto FCalled = IsUsed.second.getCalledFunction();
          if (std::find_if(CSInfo.Dtors.begin(), CSInfo.Dtors.end(),
                           [FCalled](const Function *FCtor) -> bool {
                             return FCalled == FCtor;
                           }) == CSInfo.Dtors.end())
            return false;
        }
      } else if (auto CS = ImmutableCallSite(I)) {
        auto *StrType = getStructTypeOfMethod(*CS.getCalledFunction());
        assert(std::find(Fields.begin(), Fields.end(), StrType) !=
                   Fields.end() &&
               "Unexpected instruction");
        auto *BasePtrType = StrType->getTypeAtIndex(BasePtrOffset);
        if (BasePtrType != CS.getType())
          continue;

        // Check that methods returning pointers to elements are dereferenced.
        for (auto &U : CS.getInstruction()->uses())
          if (!isa<LoadInst>(U.getUser()))
            return false;
      }

    if (!CSInfo.Appends.empty())
      if (!compareAllAppendCallSites())
        return false;
    if (!CSInfo.Ctors.empty())
      if (!compareAllCtorCallSites())
        return false;
    if (!CSInfo.CCtors.empty())
      if (!compareAllCCtorCallSites())
        return false;
    if (!CSInfo.Dtors.empty())
      if (!compareAllDtorCallSites())
        return false;
    return true;
  }
};

class StructMethodTransformation {
public:
  StructMethodTransformation(
      const DataLayout &DL, DTransAnalysisInfo &DTInfo,
      const TargetLibraryInfo &TLI, ValueToValueMapTy &VMap,
      const CallSiteComparator::CallSitesInfo &CSInfo,
      const StructureMethodAnalysis::TransformationData &InstsToTransform,
      LLVMContext &Context)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), VMap(VMap), CSInfo(CSInfo),
        InstsToTransform(InstsToTransform), Context(Context) {}

  void updateReferences(StructType *OldStruct, StructType *NewArray,
                        const SmallVectorImpl<StructType *> &Fields,
                        unsigned AOSOff, unsigned ElemOffset) const {
    IRBuilder<> Builder(Context);

    for (auto *I : InstsToTransform.ArrayInstToTransform) {
      auto *NewI = cast<Instruction>((Value *)VMap[I]);
      if (auto *L = dyn_cast<LoadInst>(NewI)) {
        auto *Ptr = L->getPointerOperand();
        if (auto *BC = dyn_cast<BitCastInst>(Ptr))
          Ptr = BC->getOperand(0);
        if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr)) {
          assert(GEP->getNumIndices() == 2 &&
                 "Unexpected pointer to array load");
          unsigned Off = cast<Constant>(GEP->getOperand(2)) // 2nd index
                             ->getUniqueInteger()
                             .getLimitedValue();
          // Nothing to change
          if (Off == AOSOff)
            continue;
          // 32-bit offset in struct corresponding to pointer to AOS.
          GEP->setOperand(2, Builder.getIntN(32, AOSOff));
        } else
          llvm_unreachable("Unexpected address.");
      }
      else if (auto *SI = dyn_cast<StoreInst>(NewI)) {
        auto *Ptr = SI->getPointerOperand();
        auto *OldPtr = cast<StoreInst>(I)->getPointerOperand();
        if (isBitCastLikeGep(DL, OldPtr)) {
          assert(AOSOff == 0 && "Unexpected store address");
          continue;
        }
        if (auto *BC = dyn_cast<BitCastInst>(Ptr)) {
          assert(isSafeBitCast(DL, OldPtr) && "Unexpected store address");
          Ptr = BC->getOperand(0);
        }
        if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr)) {
          assert(GEP->getNumIndices() == 2 &&
                 "Unexpected pointer to array store");
          unsigned Off = cast<Constant>(GEP->getOperand(2)) // 2nd index
                             ->getUniqueInteger()
                             .getLimitedValue();
          // Nothing to change
          if (Off == AOSOff)
            continue;
        } else
          llvm_unreachable("Unexpected address.");

        // See checkArrPtrStoreUses: it is store of allocated memory.
        Ptr = SI->getPointerOperand();
        SI->eraseFromParent();
        assert(Ptr->hasNUses(0) && "Inconsistent logic");
        if (auto *BC = dyn_cast<BitCastInst>(Ptr)) {
          Ptr = BC->getOperand(0);
          BC->eraseFromParent();
          assert(Ptr->hasNUses(0) && "Inconsistent logic");
        }
        cast<Instruction>(Ptr)->eraseFromParent();
      } else if (isa<ICmpInst>(NewI))
        continue;
      else if (CallSite(NewI)) {
        continue;
      } else
        llvm_unreachable("Unexpected instruction to transform.");
    }

    // Deal with combined call sites.
    SmallVector<const CallInst*, 3> OldAppends;
    for (auto *I : InstsToTransform.ArrayInstToTransform) {
      auto *NewI = cast_or_null<Instruction>((Value *)VMap[I]);
      if (!NewI)
        continue;
      if (auto CS = CallSite(NewI)) {
        auto OldCS = ImmutableCallSite(I);
        // CS can only be array method call.
        assert(CS.getCalledFunction() && "Inconsistent logic");
        auto *Fld = OldCS.getArgOperand(0)->getType();
        // 'this' argument has the same type as pointer to arrays in S.StrType.
        bool ToRemove = Fld != OldStruct->getTypeAtIndex(AOSOff);

        if (std::find(CSInfo.Ctors.begin(), CSInfo.Ctors.end(),
                      OldCS.getCalledFunction()) != CSInfo.Ctors.end())
          updateCtor(CS, ToRemove);
        else if (std::find(CSInfo.CCtors.begin(), CSInfo.CCtors.end(),
                           OldCS.getCalledFunction()) != CSInfo.CCtors.end())
          updateCtor(CS, ToRemove);
        else if (std::find(CSInfo.Dtors.begin(), CSInfo.Dtors.end(),
                           OldCS.getCalledFunction()) != CSInfo.Dtors.end())
          updateDtor(CS, ToRemove);
        else if (std::find(CSInfo.Appends.begin(), CSInfo.Appends.end(),
                           OldCS.getCalledFunction()) != CSInfo.Appends.end()) {
          // See compareAllAppendCallSites, appends should be in a single
          // BasicBlock and CallInst.
          OldAppends.push_back(cast<CallInst>(I));
        }
      }
    }
    updateAppends(OldAppends, Fields, ElemOffset);
  }

private:
  void updateCtor(CallSite CS, bool ToRemove) const {
    if (!ToRemove) {
      return;
    }

    IRBuilder<> Builder(Context);
    auto Alloc =
        CallSite(cast<Instruction>(CS.getArgOperand(0)->stripPointerCasts()));

    auto *This = CS.getArgOperand(0);
    if (auto *Inv = dyn_cast<InvokeInst>(CS.getInstruction())) {
      Builder.SetInsertPoint(Inv);
      Builder.CreateBr(Inv->getNormalDest());
    }
    CS.getInstruction()->eraseFromParent();
    for (auto *BC = dyn_cast<BitCastInst>(This); BC;) {
      auto *I = BC;
      BC = dyn_cast<BitCastInst>(BC->getOperand(0));
      I->eraseFromParent();
    }

    SmallSet<User*, 6> Users;
    for (auto &U : Alloc.getInstruction()->uses())
      Users.insert(U.getUser());

    for (auto *User : Users)
      if (auto CS = CallSite(User)) {
        if (auto *Inv = dyn_cast<InvokeInst>(User)) {
          Builder.SetInsertPoint(Inv);
          Builder.CreateBr(Inv->getNormalDest());
        }
        DTInfo.deleteCallInfo(CS.getInstruction());
        CS.getInstruction()->eraseFromParent();
      } else if (auto *SI = dyn_cast<StoreInst>(User))
        SI->eraseFromParent();
      else
        llvm_unreachable("Inconsistency with checkArrPtrStoreUses.");

    if (auto *Inv = dyn_cast<InvokeInst>(Alloc.getInstruction())) {
      Builder.SetInsertPoint(Inv);
      Builder.CreateBr(Inv->getNormalDest());
    }
    DTInfo.deleteCallInfo(Alloc.getInstruction());
    Alloc.getInstruction()->eraseFromParent();
  }

  void updateDtor(CallSite CS, bool ToRemove) const {
    if (!ToRemove)
      return;

    auto *This = CS.getArgOperand(0);
    assert(
        isa<LoadInst>(This) &&
        "Inconsistency with checkArrPtrLoadUses and canCallSitesBeMerged");

    IRBuilder<> Builder(Context);
    if (auto *Inv = dyn_cast<InvokeInst>(CS.getInstruction())) {
      Builder.SetInsertPoint(Inv);
      Builder.CreateBr(Inv->getNormalDest());
    }
    CS.getInstruction()->eraseFromParent();

    SmallSet<User *, 6> Users;
    for (auto &U : This->uses())
      Users.insert(U.getUser());

    for (auto *User : Users) {
      Value *FreeCall = nullptr;
      Instruction *Cast = nullptr;
      // See checkArrPtrLoadUses
      if (auto *GEP = dyn_cast<GetElementPtrInst>(User)) {
        assert(GEP->hasAllZeroIndices() && GEP->hasOneUse() &&
               "Inconsistency with checkArrPtrLoadUses");
        FreeCall = GEP->use_begin()->getUser();
        Cast = GEP;
      }
      // See checkArrPtrLoadUses
      else if (auto *BC = dyn_cast<BitCastInst>(User)) {
        assert(BC->hasOneUse() && "Inconsistency with checkArrPtrLoadUses");
        FreeCall = BC->use_begin()->getUser();
        Cast = BC;
      } else
        continue;

      assert(CallSite(FreeCall) && "Inconsistency with checkArrPtrLoadUses");

      if (auto *Inv = dyn_cast<InvokeInst>(FreeCall)) {
        Builder.SetInsertPoint(Inv);
        Builder.CreateBr(Inv->getNormalDest());
      }
      DTInfo.deleteCallInfo(cast<Instruction>(FreeCall));
      cast<Instruction>(FreeCall)->eraseFromParent();
      Cast->eraseFromParent();
    }
  }

  // See compareAllAppendCallSites, appends should be in a single
  // BasicBlock and CallInst. Final instruction can be inserted before
  // terminator.
  //
  // Coupled with FunctionType updates.
  void updateAppends(const SmallVectorImpl<const CallInst *> &OldAppends,
                     const SmallVectorImpl<StructType *> &Fields,
                     unsigned ElemOffset) const {
    if (OldAppends.empty())
      return;

    assert(OldAppends.size() == Fields.size() &&
           "Inconsistency with compareAllAppendCallSites");

    // Array to hold call OldAppends in order consistent with Fields.
    SmallVector<const CallInst *, 6> SortedAppends(Fields.size(), nullptr);
    for (auto *CI: OldAppends) {
      auto *ArrayTy =
          CI->getArgOperand(0)->getType()->getPointerElementType();

      auto It = std::find(Fields.begin(), Fields.end(), ArrayTy);
      assert(It != Fields.end() && "Incorrect append method");
      SortedAppends[It - Fields.begin()] = CI;
    }

    auto *NewAppend = cast<CallInst>((Value *)VMap[SortedAppends[0]]);
    auto *OldFunctionTy = SortedAppends[0]->getFunctionType();
    auto *ElementType =
        cast<StructType>(
            OldFunctionTy->getParamType(0)->getPointerElementType())
            ->getTypeAtIndex(ElemOffset)
            ->getPointerElementType();

    unsigned Offset = -1U;
    SmallVector<const Value *, 6> Args;
    for (auto *Param : OldFunctionTy->params()) {
      ++Offset;
      if (Param == ElementType) {
        for (auto *CI : SortedAppends)
          Args.push_back(CI->getArgOperand(Offset));
        continue;
      }

      if (auto *Ptr = dyn_cast<PointerType>(Param))
        if (Ptr->getElementType() == ElementType) {
          for (auto *CI : SortedAppends)
            Args.push_back(CI->getArgOperand(Offset));
          continue;
        }
      Args.push_back(SortedAppends[0]->getArgOperand(Offset));
    }

    SmallVector<Value *, 6> NewArgs;
    for (auto *V : Args)
      NewArgs.push_back((Value *)VMap[V]);

    IRBuilder<> Builder(Context);
    // Insert at the end of BasicBlock.
    Builder.SetInsertPoint(NewAppend->getParent()->getTerminator());
    Builder.CreateCall(NewAppend->getCalledValue(), NewArgs);

    for (auto *CI: OldAppends) {
      auto *NewI = (Value *)VMap[CI];
      cast<CallInst>(NewI)->eraseFromParent();
    }
  }

  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  ValueToValueMapTy &VMap;
  const CallSiteComparator::CallSitesInfo &CSInfo;
  const StructureMethodAnalysis::TransformationData &InstsToTransform;
  LLVMContext &Context;
};
} // namespace soatoaos
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Instructions to transform.
struct SOAToAOSStructMethodsCheckDebugResult
    : public StructureMethodAnalysis::TransformationData,
      public CallSiteComparator::CallSitesInfo {};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace dtrans
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSSTRUCT_H
