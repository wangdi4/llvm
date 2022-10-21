//===-------------- SOAToAOSOPStruct.h - Part of SOAToAOSOPPass -----------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOPSTRUCT_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOPSTRUCT_H

#if !INTEL_FEATURE_SW_DTRANS
#error SOAToAOSOPStruct.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "SOAToAOSOPArrays.h"
#include "SOAToAOSOPCommon.h"
#include "SOAToAOSOPEffects.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Utils/Local.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

#define DTRANS_SOASTR "dtrans-soatoaosop-struct"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {

// Structure's methods in SOA can have different side-effects.
//
// Preliminary checks filter out irrelevant side-effects from accesses to
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

    // Not calling isStructuredLoad again for nested loads to avoid deep
    // recursion of nested loads. All loads are processed in checkStructMethod.
    if (D->Arg1->Kind == Dep::DK_Load)
      return true;
    else if (isStructuredExpr(D->Arg1, S))
      return true;

    DTransType *OutType = nullptr;
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

  static bool isDirectMemoryInterfaceLoad(const Dep *D,
                                          const SummaryForIdiom &S,
                                          unsigned &ArgNo) {
    if (D->Kind != Dep::DK_Load)
      return false;

    DTransType *OutType = nullptr;
    if (!Idioms::isFieldAddr(D->Arg1, S, ArgNo, OutType))
      return false;

    return isa<DTransPointerType>(OutType) &&
           OutType->getPointerElementType() == S.MemoryInterface;
  }

  // Returns type array type accessed.
  // Returns nullptr if load is not related to array transformed.
  static DTransStructType *
  isLoadOrStoreOfArrayPtr(const DepMap &DM,
                          // Types of interest
                          const SmallVectorImpl<DTransStructType *> &Arrays,
                          const SummaryForIdiom &S, const Instruction &I) {
    const Value *Address = nullptr;
    if (auto *L = dyn_cast<LoadInst>(&I))
      Address = L->getPointerOperand();
    else if (auto *SI = dyn_cast<StoreInst>(&I))
      Address = SI->getPointerOperand();
    else
      llvm_unreachable("Incorrect call of isLoadOrStoreOfArrayPtr.");

    // Safety checks guarantee (CFG + legality) that constant address does not
    // refer to array of interest.
    if (isa<Constant>(Address))
      return nullptr;

    DTransType *FieldType = nullptr;
    // Unstructured store.
    if (!StructIdioms::isFieldAddr(DM.getApproximation(Address), S, FieldType))
      return nullptr;

    // Not a pointer to array.
    if (!isa<DTransPointerType>(FieldType))
      return nullptr;

    auto *FPointeeTy = FieldType->getPointerElementType();

    // Layout restriction.
    // See
    // SOAToAOSOPTransformImpl::CandidateLayoutInfo::populateLayoutInformation.
    auto *ArrStrType = cast<DTransStructType>(FPointeeTy);
    auto It = std::find(Arrays.begin(), Arrays.end(), ArrStrType);
    // Access to non-interesting field.
    if (It == Arrays.end())
      return nullptr;

    return ArrStrType;
  }

private:
  static bool isNonStructArg(const Dep *D, const SummaryForIdiom &S) {
    if (D->Kind != Dep::DK_Argument)
      return false;

    DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
        S.DTInfo->getTypeMetadataReader().getDTransTypeFromMD(
            const_cast<llvm::Function *>(S.Method)));
    if (!DFnTy)
      return false;

    auto *Ty = DFnTy->getArgType(D->Const);
    // Simple check that Ty is not related to S.S
    if (isa<DTransPointerType>(Ty))
      Ty = Ty->getPointerElementType();

    // For transformation, it is sufficient that only elements
    // of array, integers and (float for tests) are permitted.
    //
    // For legality, it is necessary to prevent unchecked accesses
    // to pointers to arrays (only accessible through S.StrType).
    return Ty->isIntegerTy() || Ty->isFloatingPointTy() ||
           (isa<DTransStructType>(Ty) && Ty != S.StrType);
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
  static bool isThisArgNonInitialized(const DTransSafetyInfo &DTInfo,
                                      const TargetLibraryInfo &TLI,
                                      const Function *F, unsigned Size) {
    // Simple wrappers only.
    if (!F->hasOneUse())
      return false;

    auto *Call = dyn_cast<CallBase>(F->use_begin()->getUser());
    if (!Call)
      return false;

    auto *This = dyn_cast<Instruction>(Call->getArgOperand(0));
    if (!This)
      return false;

    // Extract 'this' actual argument.
    auto *AllocCall = cast<Instruction>(This->stripPointerCasts());
    auto *Info = DTInfo.getCallInfo(AllocCall);
    if (!Info)
      return false;

    if (Info->getCallInfoKind() != dtrans::CallInfo::CIK_Alloc)
      return false;

    auto AK = cast<dtrans::AllocCallInfo>(Info)->getAllocKind();

    // Malloc, New, UserMalloc are OK: they return non-initialized memory.
    if (AK != dtrans::AK_Malloc && AK != dtrans::AK_New && !isUserAllocKind(AK))
      return false;

    SmallPtrSet<const Value *, 3> Args;
    collectSpecialAllocArgs(cast<dtrans::AllocCallInfo>(Info)->getAllocKind(),
                            cast<CallBase>(AllocCall), Args, TLI);
    assert(Args.size() == 1 && "Unsupported allocation function");
    if (auto *C = dyn_cast<Constant>(*Args.begin()))
      if (C->getUniqueInteger().getLimitedValue() == Size)
        return true;

    return false;
  }

  // Checks that 'this' argument is dead after a call, because it is passed to
  // deallocation routine immediately after \p Call instruction.
  //
  // Implementation: checks that 'this' is used in free.
  // free should be in successors BasicBlocks, see compareDtorCalls below.
  static bool isThisArgIsDead(const DTransSafetyInfo &DTInfo,
                              const TargetLibraryInfo &TLI,
                              const CallBase *Call) {
    auto *F = dtrans::getCalledFunction(*Call);

    // Simple wrappers only.
    if (!F || !F->hasOneUse())
      return false;

    auto *ThisArg = dyn_cast<Instruction>(Call->getArgOperand(0));
    if (!ThisArg)
      return false;

    SmallPtrSet<const BasicBlock *, 2> DeleteBB;

    auto *BB = Call->getParent();

    for (auto *User : post_order(CastDepGraph<const Value *>(ThisArg)))
      for (auto &U : User->uses()) {
        if (isCastUse(U))
          continue;

        if (!isa<Instruction>(U.getUser()))
          return false;

        if (U.getUser() == Call)
          continue;

        auto *Inst = cast<Instruction>(U.getUser());
        if (isFreedPtr(DTInfo, TLI, U))
          DeleteBB.insert(Inst->getParent());
        // Other uses in successors are forbidden.
        else if (find(successors(BB), Inst->getParent()) != succ_end(BB))
          return false;

        // Check same BB.
        if (BB == Inst->getParent())
          for (const Instruction *I = Call; I; I = I->getNextNode())
            if (I == Inst)
              return true;
      }

    // Simple CFG handling: all successors should contain delete.
    if (!all_of(successors(Call->getParent()),
                [&DeleteBB](const BasicBlock *BB) -> bool {
                  return DeleteBB.find(BB) != DeleteBB.end();
                }))
      return false;

    return true;
  }

  // Returns true if U is a pointer to memory passed to deallocation routine.
  static bool isFreedPtr(const DTransSafetyInfo &DTInfo,
                         const TargetLibraryInfo &TLI, const Use &U) {

    const auto *Call = dyn_cast<CallBase>(U.getUser());
    if (!Call)
      return false;

    auto *Info = DTInfo.getCallInfo(Call);
    if (!Info || Info->getCallInfoKind() != dtrans::CallInfo::CIK_Free)
      return false;

    SmallPtrSet<const Value *, 3> Args;
    collectSpecialFreeArgs(cast<dtrans::FreeCallInfo>(Info)->getFreeKind(),
                           Call, Args, TLI);
    if (Args.size() != 1 || *Args.begin() != U.get())
      return false;

    return true;
  }

  // Returns
  //  - first  - true if there is a use in dellocation, false otherwise.
  //  - second - single use in method candidate, nullptr otherwise.
  static std::pair<bool, const CallBase *>
  isThereUseInFree(DTransSafetyInfo &DTInfo, const TargetLibraryInfo &TLI,
                   const Value *V, DTransStructType *ArrType) {
    bool FreeUseSeen = false;
    const CallBase *SingleMethod = nullptr;
    for (auto *User : post_order(CastDepGraph<const Value *>(V)))
      for (auto &U : User->uses())
        if (const auto *Call = dyn_cast<CallBase>(U.getUser())) {
          // Direct call.
          if (auto *F = dtrans::getCalledFunction(*Call))
            if (getOPStructTypeOfMethod(F, &DTInfo) == ArrType) {
              if (SingleMethod)
                return std::make_pair(FreeUseSeen, nullptr);
              SingleMethod = Call;
              continue;
            }
          if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U))
            FreeUseSeen = true;
        }
    return std::make_pair(FreeUseSeen, SingleMethod);
  }

  // Given V, returns single using method call (deallocation functions are
  // ignored), returns nullptr otherwise.
  static const CallBase *getSingleMethodCall(DTransSafetyInfo &DTInfo,
                                             const TargetLibraryInfo &TLI,
                                             const Value *V,
                                             DTransStructType *ArrType) {
    return isThereUseInFree(DTInfo, TLI, V, ArrType).second;
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

  // Argument should be call to ctor.
  // See StructureMethodAnalysis::checkArrPtrStoreUses
  static const StoreInst &getStoreOfPointer(const CallBase *Call) {
    auto *Alloc = Call->getArgOperand(0)->stripPointerCasts();
    for (auto *User : post_order(CastDepGraph<const Value *>(Alloc)))
      for (auto &U : User->uses())
        if (auto *SI = dyn_cast<StoreInst>(U.getUser()))
          return *SI;
    llvm_unreachable("Incorrect Call provided in getStoreOfPointer.");
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
  using InstContainer = SmallPtrSet<const Instruction *, 32>;

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
  bool checkArrPtrLoadUses(const LoadInst &L, DTransStructType *ArrType) const {
    // Transformation restriction:
    // isSafeBitCast/isSafeIntToPtr are not permitted.
    if (!isa<GetElementPtrInst>(L.getPointerOperand()))
      return false;

    // Check that all uses are related to array method calls,
    // to null-check or to call to deallocation.
    bool FreeUseSeen = false;
    unsigned NumMethodsCalled = 0;

    // MS compiler complains about missing braces.
    for (auto *User : post_order(CastDepGraph<const Value *>(&L))) {
      for (auto &U : User->uses()) {
        if (isCastUse(U)) {
          continue;
        } else if (const CallBase *Call = dyn_cast<CallBase>(U.getUser())) {
          if (auto *F = dtrans::getCalledFunction(*Call)) {
            // CFG restriction, use can be only for 'this' argument.
            //
            // Safety check of CFG based on the 'this' argument.
            // See dtrans::StructInfo::CallSubGraph.
            //
            // Also see populateLayoutInformation.
            //
            // Method call.
            if (ArrType == getOPStructTypeOfMethod(F, &DTInfo)) {
              ++NumMethodsCalled;
              // insert optimistically
              insertArrayInst(Call, "Array method call");
              continue;
            }
          }
          if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U)) {
            FreeUseSeen = true;
            continue;
          }
          return false;
        } else if (CtorDtorCheck::isNullCheck(U.getUser())) {
          continue;
        } else {
          return false;
        }
      }
    }

    if (FreeUseSeen && NumMethodsCalled != 1)
      return false;

    insertArrayInst(&L, "Load of array");
    return true;
  }

  // Function checks that pointer to newly allocated memory is stored to
  // pointer to array field. Uses of stored value is checked.
  //
  // Use of stored value in ctor is checked in CallSiteComparator.
  bool checkArrPtrStoreUses(const StoreInst &SI,
                            DTransStructType *ArrType) const {
    auto *Address = SI.getPointerOperand();
    // Transformation restriction:
    // isSafeIntToPtr are not permitted.
    if (!isa<GetElementPtrInst>(Address) &&
        !isSafeBitCast(DL, Address, DTInfo.getPtrTypeAnalyzer()))
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
    auto *Val = SI.getValueOperand()->stripPointerCasts();
    if (!isa<Instruction>(Val))
      return false;

    if (!StructIdioms::isAlloc(DM.getApproximation(Val), S))
      return false;

    bool MethodSeen = false;
    // MS compiler complains about missing braces.
    for (auto *User : post_order(CastDepGraph<const Value *>(Val))) {
      for (auto &U : User->uses()) {
        if (isCastUse(U)) {
          continue;
        } else if (const CallBase *Call = dyn_cast<CallBase>(U.getUser())) {
          // Use of stored value in ctor is checked in CallSiteComparator.
          if (auto *F = dtrans::getCalledFunction(*Call))
            if (ArrType == getOPStructTypeOfMethod(F, &DTInfo)) {
              // Store should be associated with single method, i.e. with ctor.
              // Called value is checked in CallSiteComparator.
              if (MethodSeen) {
                return false;
              }
              // insert optimistically
              insertArrayInst(Call, "Array method call");
              MethodSeen = true;
              continue;
            }
          if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U)) {
            continue;
          }
          return false;
        }
        // MethodSeen and this check guarantees 1-1 correspondence.
        else if (U.getUser() == &SI) {
          continue;
        } else {
          return false;
        }
      }
    }

    // Non-null stores should be associated with call to ctor-only.
    // Check is completed in canCallSitesBeMerged below.
    insertArrayInst(&SI, "Init ptr to array");
    return MethodSeen;
  }

  // Checks that memset with 0 nullifies some pointers.
  // Nullptr initialization is checked in CallSiteComparator.
  bool checkZeroInit(const MemSetInst &MI) const {
    Constant *V = dyn_cast<Constant>(MI.getValue());
    if (!V || !V->isZeroValue())
      return false;

    insertArrayInst(&MI, "Nullptr with memset of array");
    return StructIdioms::isThisLikeArg(DM.getApproximation(MI.getDest()), S);
  }

  // Additional check of method call parameters.
  // Complements approximate dependency checks from DepCompute.
  // Helper method for checkStructMethod.
  //
  // Call to method should have argument dependent on integer fields or integer
  // arguments or be 'this' like arguments.
  bool checkMethodCall(const CallBase *Call) const {
    assert(Idioms::isKnownCall(DM.getApproximation(Call), S) &&
           "Incorrect checkMethodCall");

    for (auto &Op : Call->operands()) {
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
  StructureMethodAnalysis(const DataLayout &DL, DTransSafetyInfo &DTInfo,
                          const TargetLibraryInfo &TLI, const DepMap &DM,
                          const SummaryForIdiom &S,
                          const SmallVectorImpl<DTransStructType *> &Arrays,
                          TransformationData &TI)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), DM(DM), S(S), Arrays(Arrays), TI(TI) {
  }

  unsigned getTotal() const { return TI.ArrayInstToTransform.size(); }

  bool checkStructMethod(bool &SeenArrays) const {
    bool Invalid = false;

    SeenArrays = false;

    for (auto &I : instructions(*S.Method)) {
      if (arith_inst_dep_iterator::isSupportedOpcode(I.getOpcode()))
        continue;

      auto *D = DM.getApproximation(&I);
      if (!D)
        llvm_unreachable(
            "Not synchronized checkStructMethod and computeDepApproximation");

      if (D->isBottom())
        Invalid = true;

      if (Invalid && !DTransSOAToAOSOPComputeAllDep)
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
        else if (Idioms::isThisArg(D, S))
          break;
        Handled = false;
        break;
      case Instruction::Load:
        // All loads should be structured here to avoid
        // expensive recursion in isStructuredLoad/isStructuredCall
        if (StructIdioms::isStructuredLoad(D, S)) {
          // Explicitly check non-nested load, i.e. direct loads of
          // pointers to arrays.
          if (auto *ArrType =
                  StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I)) {
            SeenArrays = true;
            // Related to arrays of interest, limited kinds of uses
            // permitted. In particular, analysis of uses prevents nested loads
            // with respect to arrays of interest.
            if (checkArrPtrLoadUses(cast<LoadInst>(I), ArrType))
              break;
            // Fall through to Handled = false.
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
                 StructIdioms::isMemoryInterfaceCopy(D, S)) {
          // Avoid checking conflicts of load/stores to MemoryInterface.
          if (I.getParent() == &S.Method->getEntryBlock())
            break;
        }
        // Explicitly check for Struct's field update.
        else if (auto *ArrType =
                     StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I)) {
          SeenArrays = true;
          if (auto *C = dyn_cast<Constant>(
                  cast<StoreInst>(I).getValueOperand()->stripPointerCasts())) {
            if (C->isZeroValue()) {
              insertArrayInst(&I, "Nullptr of array");
              break;
            }
            // Fall through to Handled = false.
          }
          // Related to arrays of interest, limited kinds of uses
          // permitted.
          //
          // Check for usage in ctor is completed in CallSiteComparator.
          else if (checkArrPtrStoreUses(cast<StoreInst>(I), ArrType))
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
      case Instruction::CleanupPad:
      case Instruction::CleanupRet:
      case Instruction::CatchSwitch:
      case Instruction::CatchPad:
        break;
      case Instruction::Call:
      case Instruction::Invoke:
      case Instruction::Alloca:
        if (isa<DbgInfoIntrinsic>(I))
          break;
        // All calls should be structured here to avoid
        // expensive recursion in isStructuredLoad/isStructuredCall
        else if (StructIdioms::isStructuredCall(D, S))
          break;
        else if (StructIdioms::isKnownCall(D, S)) {
          // Check if calls to Struct's method is structured.
          if (const auto *Call = dyn_cast<CallBase>(&I))
            if (checkMethodCall(Call))
              break;
          // Fall through to Handled = false.
        }
        // memset of memory pointed-to by some field,
        // For example, memset of elements of array, which is not transformed.
        else if (StructIdioms::isStructuredStore(D, S))
          break;
        // Memset of structure itself.
        else if (auto *MI = dyn_cast<MemSetInst>(&I)) {
          SeenArrays = true;
          // Safety checks guarantee (CFG + legality) that constant address does
          // not refer to array of interest.
          if (isa<Constant>(MI->getDest()))
            // Not related to arrays of interest of interest and not setting
            // Handled = false here.
            break;
          if (checkZeroInit(*MI))
            break;
        }
        Handled = false;
        break;
      case Instruction::Resume:
        break;
      default:
        Handled = false;
        break;
      }

      if (!Handled) {
        DEBUG_WITH_TYPE(DTRANS_SOASTR, {
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
  DTransSafetyInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;

  // Summary for StructIdioms checks.
  const SummaryForIdiom &S;

  // Array types
  const SmallVectorImpl<DTransStructType *> &Arrays;

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
  constexpr static int MaxNumFieldCandidates =
      SOAToAOSOPLayoutInfo::MaxNumFieldCandidates;

public:
  using FunctionSet = SmallVector<const Function *, MaxNumFieldCandidates>;
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
  bool compareAppendCallSites(const CallBase *Call1, const CallBase *Call2,
                              unsigned Off1, unsigned Off2) const {

    auto *ElementType1 = getOPSOAElementType(getOPSOAArrayType(S.StrType, Off1),
                                             BasePointerOffset);
    auto *ElementType2 = getOPSOAElementType(getOPSOAArrayType(S.StrType, Off2),
                                             BasePointerOffset);

    // Same BasicBlock for combining.
    if (Call1->getParent() != Call2->getParent())
      return false;

    PtrTypeAnalyzer &PTA = DTInfo.getPtrTypeAnalyzer();
    int32_t ArgIdx = -1;
    for (const auto &P : zip_first(Call1->args(), Call2->args())) {
      auto *A1 = std::get<0>(P).get();
      auto *A2 = std::get<1>(P).get();
      ArgIdx++;
      if (A1 == A2)
        continue;

      auto *Info1 = PTA.getValueTypeInfo(Call1, ArgIdx);
      auto *Info2 = PTA.getValueTypeInfo(Call2, ArgIdx);
      assert(Info1 && "Expected PointerTypeAnalyzer to collect type");
      assert(Info2 && "Expected PointerTypeAnalyzer to collect type");
      DTransType *T1 = PTA.getDominantType(*Info1, ValueTypeInfo::VAT_Use);
      DTransType *T2 = PTA.getDominantType(*Info2, ValueTypeInfo::VAT_Use);
      if (!T1 || !T2)
        return false;

      bool IsElemA1 = T1 == ElementType1;
      bool IsElemA2 = T2 == ElementType2;
      if (IsElemA1 != IsElemA2)
        return false;
      if (IsElemA1)
        continue;

      if (!isa<DTransPointerType>(T1) || !isa<DTransPointerType>(T2))
        return false;

      IsElemA1 = T1->getPointerElementType() == ElementType1;
      IsElemA2 = T2->getPointerElementType() == ElementType2;
      if (IsElemA1 != IsElemA2)
        return false;
      if (IsElemA1)
        continue;

      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      if (!StructIdioms::isFieldLoad(DM.getApproximation(A1), Off1, ArgNo1) ||
          !StructIdioms::isFieldLoad(DM.getApproximation(A2), Off2, ArgNo2) ||
          // Expect 'this' parameter in usual place.
          ArgNo1 != 0 || ArgNo2 != 0)
        return false;

      auto *L1 = dyn_cast<LoadInst>(A1);
      auto *L2 = dyn_cast<LoadInst>(A2);
      if (!L1 || !L2)
        return false;

      if (Call1->getParent() != L1->getParent() ||
          Call2->getParent() != L2->getParent())
        return false;

      // Potentially conflicting stores to fields containing pointers to arrays
      // are excluded by checks in compareAllAppendCallSites.
    }
    return true;
  }

  // Single BasicBlock:
  //  this1 = str->arr1
  //  call append(this1, elem1)
  //  this2 = str->arr2
  //  call append(this2, elem2)
  bool compareAllAppendCallSites(
      const SmallVectorImpl<const CallBase *> &CSs) const {
    // There could be one call for each field or none at all. Do not permit
    // multiple groups of append methods in a single Struct's method. It
    // simplifies transformation.
    if (CSs.size() != Arrays.size())
      return false;

    SmallPtrSet<const Instruction *, 4> KnownInsts;
    auto CallPivot = CSs[0];
    auto OffPivot = Offsets[0];
    for (const auto &P : zip(make_range(CSs.begin() + 1, CSs.end()),
                             make_range(Offsets.begin() + 1, Offsets.end())))
      if (!compareAppendCallSites(CallPivot, std::get<0>(P), OffPivot,
                                  std::get<1>(P)))
        return false;

    // See note about conflicting stores in compareAppendCallSites.
    // Start checking when first load/call/invoke is encountered.
    bool StartCmp = false;
    for (auto &I : *CallPivot->getParent())
      switch (I.getOpcode()) {
      case Instruction::Load:
        StartCmp = true;
        continue;
      case Instruction::Call:
        if (isa<DbgInfoIntrinsic>(I))
          break;

        StartCmp = true;
        if (std::find_if(CSs.begin(), CSs.end(),
                         [&I](const CallBase *Call) -> bool {
                           return &I == Call;
                         }) == CSs.end()) {
          // Make sure the non-Append call is safe.
          Function *AuxF = dtrans::getCalledFunction(cast<CallBase>(I));
          if (AuxF && isSafeCallForAppend(AuxF, &DTInfo, TLI)) {

            if (!I.hasOneUse())
              break;
            auto *SI = dyn_cast<StoreInst>(I.user_back());
            if (!SI || !isa<AllocaInst>(SI->getPointerOperand()))
              break;
            KnownInsts.insert(SI);
            continue;
          }
          return false;
        }
        continue;
      case Instruction::BitCast:
      case Instruction::Br:
      case Instruction::GetElementPtr:
      case Instruction::ICmp:
      case Instruction::Ret:
        continue;
      default:
        if (KnownInsts.count(&I))
          continue;
        if (StartCmp)
          return false;
        continue;
      }
    return true;
  }

  // Checks that loads of MemoryInterface result in the same value.
  //
  // One needs only to check conflict with stores.
  // checkStructMethod guarantees that stores are only in entry block,
  // see isMemoryInterfaceSetFromArg and isMemoryInterfaceCopy calls.
  //
  // Load of MemoryInterface are essentially before all stores:
  //  - not in entry block (no stores after entry block);
  //  - after all stores in entry block;
  //  - before some store in entry block, but stored value is from load being
  //  checked.
  bool checkDirectMemoryInterfaceLoads(const Value *A1, const Value *A2) const {
    unsigned ArgNo1 = -1U;
    unsigned ArgNo2 = -1U;
    if (!StructIdioms::isDirectMemoryInterfaceLoad(DM.getApproximation(A1), S,
                                                   ArgNo1) ||
        !StructIdioms::isDirectMemoryInterfaceLoad(DM.getApproximation(A2), S,
                                                   ArgNo2))
      return false;

    A1 = A1->stripPointerCasts();
    A2 = A2->stripPointerCasts();
    if (auto *I1 = dyn_cast<IntToPtrInst>(A1))
      A1 = I1->getOperand(0);
    if (auto *I2 = dyn_cast<IntToPtrInst>(A2))
      A2 = I2->getOperand(0);

    auto *L1 = dyn_cast<LoadInst>(A1);
    auto *L2 = dyn_cast<LoadInst>(A2);

    if (!L1 || !L2)
      return false;

    if (ArgNo1 != ArgNo2) {
      const Instruction *Loads[] = {L1, L2};
      for (auto *Load : Loads)
        if (Load->getParent() == &S.Method->getEntryBlock())
          // See isMemoryInterfaceSetFromArg/isMemoryInterfaceCopy check in
          // checkStructMethod.
          for (auto *I = Load; I; I = I->getNextNode())
            if (auto *SI = dyn_cast<StoreInst>(I))
              // If store is of the form:
              //  store %Load, %Addr
              // then it does not conflict with load.
              if (SI->getValueOperand() != Load)
                return false;
    }

    return true;
  }

  // Compare calls to allocation/deallocation function.
  //
  // FreePtr1 and FreePtr2 are pointers to memory to deallocated (difference
  // ignored). Ignored for allocation function.
  //
  // Remaining arguments are equal or loads of MemoryInterface.
  bool compareAllocDeallocCalls(const CallBase *Call1, const CallBase *Call2,
                                const Value *FreePtr1,
                                const Value *FreePtr2) const {

    if (Call1->getCalledOperand() != Call2->getCalledOperand())
      return false;

    auto *Info = DTInfo.getCallInfo(Call1);

    SmallPtrSet<const Value *, 3> Args;
    bool Alloc = false;
    if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free) {
      collectSpecialFreeArgs(cast<dtrans::FreeCallInfo>(Info)->getFreeKind(),
                             Call1, Args, TLI);
      assert(Args.size() == 1 && "Unexpected deallocation function");

      Alloc = false;
    } else if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc) {
      collectSpecialAllocArgs(cast<dtrans::AllocCallInfo>(Info)->getAllocKind(),
                              Call1, Args, TLI);

      assert(Args.size() == 1 && "Unexpected allocation function");
      Alloc = true;
    } else
      return false;

    for (const auto &PA : zip_first(Call1->args(), Call2->args())) {
      auto *A1 = std::get<0>(PA).get();
      auto *A2 = std::get<1>(PA).get();
      if (Args.count(A1)) {
        if (Alloc) {
          if (A1 != A2)
            return false;
          continue;
        }

        if (A1->stripPointerCasts() != FreePtr1->stripPointerCasts() ||
            A2->stripPointerCasts() != FreePtr2->stripPointerCasts())
          return false;
      } else if (A1 != A2 && !checkDirectMemoryInterfaceLoads(A1, A2))
        return false;
    }
    return true;
  }

  // Compares EH related BBs of destructors. This basically returns true
  // if EH related BBs are in any of the below patterns.
  //
  // Pattern 1:
  //  BB:
  //    %21 = landingpad { i8*, i32 }
  //          cleanup
  //   %22 = bitcast %"RefVector"* %17 to i8*
  //   invoke void @Free(i8* %22)
  //          to label %BB1 unwind label %BB2
  //
  //  BB1:                          ; preds = %BB
  //    resume { i8*, i32 } %21
  //
  //  BB2:                          ; preds = %BB
  //     %25 = landingpad { i8*, i32 }
  //         catch i8* null
  //     %26 = extractvalue { i8*, i32 } %25, 0
  //     tail call void @__clang_call_terminate(i8* %26) #51
  //     unreachable
  //
  // Pattern 2:
  //  BB:
  //    %31 = landingpad { i8*, i32 }
  //           cleanup
  //    %32 = extractvalue { i8*, i32 } %31, 0
  //    %33 = extractvalue { i8*, i32 } %31, 1
  //    %34 = getelementptr %"ValVector", %"ValVector"* %3, i64 0, i32 0
  //    invoke void @Free(i8* %34)
  //             to label %40 unwind label %45
  //
  //  BB1:                                               ; preds = %BB,
  //     %41 = phi i8* [ %37, %35 ], [ %32, %30 ]
  //     %42 = phi i32 [ %38, %35 ], [ %33, %30 ]
  //     %43 = insertvalue { i8*, i32 } undef, i8* %41, 0
  //     %44 = insertvalue { i8*, i32 } %43, i32 %42, 1
  //     resume { i8*, i32 } %44
  //
  //  BB2:                                               ; preds = %BB,
  //     %46 = landingpad { i8*, i32 }
  //             catch i8* null
  //     %47 = extractvalue { i8*, i32 } %46, 0
  //     tail call void @__clang_call_terminate(i8* %47) #51
  //     unreachable
  //
  // TODO: We could improve SOAToAOSOPPrepare pass to convert pattern 1 to
  // pattern 2 if any potential SOAToAOSOP candidate's destructor has pattern 1
  // EH code. That will help us to avoid this pattern match.
  //
  bool compareDtorBBs(const CallBase *Call1, const CallBase *Call2) const {

    // Check if there are any instructions in BB that are not visited.
    auto CheckUnVisitedInst = [](BasicBlock *BB,
                                 SmallPtrSetImpl<Instruction *> &Visited) {
      for (auto &I : *BB) {
        if (isa<DbgInfoIntrinsic>(&I) || isa<BitCastInst>(&I) ||
            isa<GetElementPtrInst>(&I))
          continue;
        if (!Visited.count(&I))
          return false;
      }
      return true;
    };

    // Check this is Unreachable BB.
    //  Ex:  BB2 in the above example.
    //
    auto CheckUnreachableBB = [this, &CheckUnVisitedInst](BasicBlock *BB) {
      SmallPtrSet<Instruction *, 8> Visited;

      auto *UnR = dyn_cast<UnreachableInst>(BB->getTerminator());
      if (!UnR)
        return false;

      auto *CallT =
          dyn_cast_or_null<CallBase>(UnR->getPrevNonDebugInstruction());
      if (!CallT)
        return false;
      Function *F = dtrans::getCalledFunction(cast<CallBase>(*CallT));
      LibFunc LibF;
      if (!F || !TLI.getLibFunc(*F, LibF) || !TLI.has(LibF))
        return false;
      if (LibF != LibFunc_clang_call_terminate)
        return false;

      auto *EVI = dyn_cast<ExtractValueInst>(CallT->getArgOperand(0));
      if (!EVI || EVI->getNumIndices() != 1 || *EVI->idx_begin() != 0)
        return false;
      auto *LPI = dyn_cast<LandingPadInst>(EVI->getOperand(0));
      if (!LPI || LPI->getNumClauses() != 1 || !LPI->isCatch(0))
        return false;

      Visited.insert(UnR);
      Visited.insert(CallT);
      Visited.insert(EVI);
      Visited.insert(LPI);
      if (!CheckUnVisitedInst(BB, Visited))
        return false;

      return true;
    };

    // Try to find given "Val" is return value of LandingPad instruction.
    // Return the LandingPad instruction if it finds one.
    // Ex: BB and BB1 of pattern 2 in the above example.
    //
    auto GetLandingPadInstForResumeVal =
        [](Value *Val, BasicBlock *BB,
           SmallPtrSetImpl<Instruction *> &Visited) -> LandingPadInst * {
      auto *IV1 = dyn_cast<InsertValueInst>(Val);
      if (!IV1 || IV1->getNumIndices() != 1 || *IV1->idx_begin() != 1)
        return nullptr;
      auto *PH1 = dyn_cast<PHINode>(IV1->getOperand(1));
      if (!PH1)
        return nullptr;
      auto *IV0 = dyn_cast<InsertValueInst>(IV1->getAggregateOperand());
      if (!IV0 || IV0->getNumIndices() != 1 || *IV0->idx_begin() != 0)
        return nullptr;
      auto *PH0 = dyn_cast<PHINode>(IV0->getOperand(1));
      if (!PH0)
        return nullptr;
      auto *EV0 = dyn_cast<ExtractValueInst>(PH0->getIncomingValueForBlock(BB));
      if (!EV0 || EV0->getNumIndices() != 1 || *EV0->idx_begin() != 0)
        return nullptr;
      auto *EV1 = dyn_cast<ExtractValueInst>(PH1->getIncomingValueForBlock(BB));
      if (!EV1 || EV1->getNumIndices() != 1 || *EV1->idx_begin() != 1)
        return nullptr;
      if (EV0->getAggregateOperand() != EV1->getAggregateOperand())
        return nullptr;
      Visited.insert(IV1);
      Visited.insert(IV0);
      Visited.insert(PH0);
      Visited.insert(PH1);
      Visited.insert(EV0);
      Visited.insert(EV1);
      return dyn_cast<LandingPadInst>(EV1->getAggregateOperand());
    };

    // Parse given LandingPad BB and check it is either in pattern 1 or
    // pattern 2. "FreePtr" is the pointer that is being passed to
    // destructor call.
    auto CheckResume = [this, &CheckUnreachableBB, &CheckUnVisitedInst,
                        &GetLandingPadInstForResumeVal](BasicBlock *BB,
                                                        Value *FreePtr) {
      SmallPtrSet<Instruction *, 16> Visited;
      auto *Inv = dyn_cast<InvokeInst>(BB->getTerminator());
      if (!Inv)
        return false;
      auto *Info = DTInfo.getCallInfo(Inv);
      if (!Info || Info->getCallInfoKind() != dtrans::CallInfo::CIK_Free)
        return false;
      SmallPtrSet<const Value *, 3> Args;
      collectSpecialFreeArgs(cast<dtrans::FreeCallInfo>(Info)->getFreeKind(),
                             Inv, Args, TLI);
      assert(Args.size() == 1 && "Unexpected deallocation function");
      auto *A = *Args.begin();
      if (A->stripPointerCasts() != FreePtr->stripPointerCasts())
        return false;

      BasicBlock *BB1 = Inv->getNormalDest();
      BasicBlock *BB2 = Inv->getUnwindDest();
      if (!CheckUnreachableBB(BB2))
        return false;
      auto *Res = dyn_cast<ResumeInst>(BB1->getTerminator());
      if (!Res)
        return false;
      Value *Val = Res->getValue();
      auto *LPI = dyn_cast<LandingPadInst>(Val);
      if (!LPI)
        LPI = GetLandingPadInstForResumeVal(Val, BB, Visited);

      if (!LPI || LPI->getNumClauses() != 0 || !LPI->isCleanup())
        return false;

      Visited.insert(Inv);
      Visited.insert(Res);
      Visited.insert(LPI);

      if (!CheckUnVisitedInst(BB, Visited) || !CheckUnVisitedInst(BB1, Visited))
        return false;

      return true;
    };

    assert(isa<InvokeInst>(Call1) && isa<InvokeInst>(Call2) &&
           "Incorrect arguments");
    auto *FreePtr1 = Call1->getArgOperand(0)->stripPointerCasts();
    auto *FreePtr2 = Call2->getArgOperand(0)->stripPointerCasts();

    BasicBlock *BB1 = cast<InvokeInst>(Call1)->getUnwindDest();
    BasicBlock *BB2 = cast<InvokeInst>(Call2)->getUnwindDest();
    if (pred_size(BB1) != 1 || pred_size(BB2) != 1)
      return false;

    if (succ_size(BB1) != succ_size(BB2))
      return false;

    if (succ_size(BB1) > 2)
      return false;
    if (!CheckResume(BB1, FreePtr1) || !CheckResume(BB2, FreePtr2))
      return false;

    return true;
  }

  // BasicBlock in cleanup successor of ctor/dtor may contain:
  //    - landing pad, processing simple processing of caught exception
  //    - (extractvalue) and
  //    - invoke of 'delete' function.
  //
  //  %tmp7 = landingpad { i8*, i32 }
  //          cleanup
  //  %tmp8 = extractvalue { i8*, i32 } %tmp7, 0
  //  %tmp9 = extractvalue { i8*, i32 } %tmp7, 1
  //  ; bitcasts and geps
  //  invoke void @"XMemory::operator delete(void*)"(i8* %ptr)
  //          to label %resume unwind label %terminate
  //
  // BasicBlocks in normal successor of dtor have subset of instructions above.
  bool compareCtorBBs(const CallBase *Call1, const CallBase *Call2) const {

    assert(isa<InvokeInst>(Call1) && isa<InvokeInst>(Call2) &&
           "Incorrect arguments");
    auto *FreePtr1 = Call1->getArgOperand(0)->stripPointerCasts();
    auto *FreePtr2 = Call2->getArgOperand(0)->stripPointerCasts();

    const BasicBlock *BB1 = nullptr;
    const BasicBlock *BB2 = nullptr;

    BB1 = cast<InvokeInst>(Call1)->getUnwindDest();
    BB2 = cast<InvokeInst>(Call2)->getUnwindDest();

    if (BB1->size() != BB2->size())
      return false;

    if (pred_size(BB1) != 1 || pred_size(BB2) != 1)
      return false;

    if (succ_size(BB1) != succ_size(BB2))
      return false;

    // See switch below for instructions in BB1 and BB2.
    if (succ_size(BB1) > 2)
      return false;

    // Exception paths have the same common successor blocks,
    // which results in immediate return from dtors caller.
    for (const auto &P : zip_first(successors(BB1), successors(BB2))) {
      // Exception paths should be confluent.
      if (std::get<0>(P) != std::get<1>(P))
        return false;
      // Exception path ends in returns from dtors caller.
      // Actually, resume instruction or noreturn function call.
      if (succ_empty(std::get<0>(P)))
        continue;
      // Successors should only lead to function exits.
      // No need to check second BB, because control flow is confluent.
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
      case Instruction::BitCast:
      case Instruction::Br:
      case Instruction::Call:
      case Instruction::ExtractValue:
      case Instruction::GetElementPtr:
      case Instruction::Invoke:
      case Instruction::LandingPad:
      case Instruction::CleanupPad:
      case Instruction::CleanupRet:
        break;
      default:
        return false;
      }

      // Delete invocations are processed completely.
      if (const CallBase *Call1 = dyn_cast<CallBase>(&I1)) {
        if (isa<DbgInfoIntrinsic>(I1))
          continue;

        if (!compareAllocDeallocCalls(Call1, cast<CallBase>(&I2), FreePtr1,
                                      FreePtr2))
          return false;
        continue;
      }

      for (const auto &PO : zip_first(I1.operands(), I2.operands())) {
        auto *Op1 = std::get<0>(PO).get();
        auto *Op2 = std::get<1>(PO).get();
        if (Op1 != Op2 && ValueRemapper[Op1] != Op2)
          return false;
      }

      if (I1.getNumUses() != I2.getNumUses())
        return false;

      for (const auto &PU : zip_first(I1.uses(), I2.uses())) {
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
  // Cleanup BasicBlock should be equal as in compareCtorBBs.
  bool compareCtorCalls(const CallBase *Call1, const CallBase *Call2,
                        unsigned Off1, unsigned Off2, bool Copy) const {

    auto *DTy1 = getOPSOAArrayType(S.StrType, Off1);
    auto *DTy2 = getOPSOAArrayType(S.StrType, Off2);
    if (!CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, cast<Function>(Call1->getCalledOperand()),
            DL.getTypeAllocSize(DTy1->getLLVMType())) ||
        !CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, cast<Function>(Call2->getCalledOperand()),
            DL.getTypeAllocSize(DTy2->getLLVMType())))
      return false;

    // Parameters should be equal as Values with 3 exceptions:
    //  - 'this' parameter (checked above);
    //  - memory interface access (relying on fact it is only copied around);
    //  - second parameter in copy-ctor should be derived from 2nd argument.
    bool ThisArg = true;
    for (const auto &P : zip_first(Call1->args(), Call2->args())) {
      auto *A1 = std::get<0>(P).get()->stripPointerCasts();
      auto *A2 = std::get<1>(P).get()->stripPointerCasts();

      if (ThisArg) {
        ThisArg = false;
        if (!compareAllocDeallocCalls(cast<CallBase>(A1), cast<CallBase>(A2),
                                      nullptr, nullptr))
          return false;
        continue;
      }

      if (Copy) {
        unsigned ArgNo1 = -1U;
        unsigned ArgNo2 = -1U;
        if (!StructIdioms::isFieldLoad(DM.getApproximation(A1), Off1, ArgNo1) ||
            !StructIdioms::isFieldLoad(DM.getApproximation(A2), Off2, ArgNo2) ||
            ArgNo1 != 1 || ArgNo2 != 1)
          return false;

        // Potentially conflicting stores to fields containing pointers to
        // arrays are excluded by checking stores associated with ctors. Stores
        // are associated with ctors by check in canCallSitesBeMerged.
        continue;
      }

      if (A1 != A2 && !checkDirectMemoryInterfaceLoads(A1, A2))
        return false;
    }

    bool Call1IsInvoke = isa<InvokeInst>(Call1);
    if (Call1IsInvoke != isa<InvokeInst>(Call2) ||
        (Call1IsInvoke && !compareCtorBBs(Call1, Call2)))
      return false;

    return true;
  }

  // Compare invokes of ctors and checks adjacency of invokes.
  bool compareAllCtorCallSites(
      const SmallVectorImpl<const CallBase *> &CtorCSs) const {
    // There could be one call for each field or none at all.
    if (CtorCSs.size() != Arrays.size())
      return false;

    auto CallPivot = CtorCSs[0];
    auto OffPivot = Offsets[0];

    for (const auto &P : zip(make_range(CtorCSs.begin() + 1, CtorCSs.end()),
                             make_range(Offsets.begin() + 1, Offsets.end()))) {
      const CallBase *Call = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareCtorCalls(CallPivot, Call, OffPivot, Off, false))
        return false;
    }

    if (!checkCtorsCallsAreAdjacent(CtorCSs))
      return false;
    return true;
  }

  // Compare invokes of copy ctors and checks adjacency of invokes.
  bool compareAllCCtorCallSites(
      const SmallVectorImpl<const CallBase *> &CCtorCSs) const {
    // There could be one call for each field or none at all.
    if (CCtorCSs.size() != Arrays.size())
      return false;

    const CallBase *CallPivot = CCtorCSs[0];
    auto OffPivot = Offsets[0];

    if (CallPivot->arg_size() != 2)
      return false;

    for (const auto &P : zip(make_range(CCtorCSs.begin() + 1, CCtorCSs.end()),
                             make_range(Offsets.begin() + 1, Offsets.end()))) {
      const CallBase *Call = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareCtorCalls(CallPivot, Call, OffPivot, Off, true))
        return false;
    }

    if (!checkCtorsCallsAreAdjacent(CCtorCSs))
      return false;

    return true;
  }

  // Compare 2 invokes of dtors
  //
  // 'this' parameters is dead in all paths after invoke (passed to
  // deallocation function).
  //
  // Arguments are from the same instance of struct.
  //
  // Cleanup BasicBlock should be equal as in compareDtorBBs.
  bool compareDtorCalls(const CallBase *Call1, const CallBase *Call2,
                        unsigned Off1, unsigned Off2) const {

    if (!CtorDtorCheck::isThisArgIsDead(DTInfo, TLI, Call1) ||
        !CtorDtorCheck::isThisArgIsDead(DTInfo, TLI, Call2))
      return false;

    if (Call1->arg_size() != 1 || Call2->arg_size() != 1)
      return false;

    // Compare argument.
    {
      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      if (!StructIdioms::isFieldLoad(
              DM.getApproximation(Call1->getArgOperand(0)), Off1, ArgNo1) ||
          !StructIdioms::isFieldLoad(
              DM.getApproximation(Call2->getArgOperand(0)), Off2, ArgNo2) ||
          ArgNo1 != 0 || ArgNo2 != 0)
        return false;

      // Potentially conflicting stores to fields containing pointers to arrays
      // are excluded by checking stores associated with ctors.
      // Stores are associated with ctors by check in canCallSitesBeMerged.
    }

    // Deallocation on normal path is checked in checkDtorsCallsAreAdjacent.
    bool Call1IsInvoke = isa<InvokeInst>(Call1);
    if (Call1IsInvoke != isa<InvokeInst>(Call2) ||
        (Call1IsInvoke && !compareDtorBBs(Call1, Call2)))
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
  bool compareAllDtorCallSites(
      const SmallVectorImpl<const CallBase *> &DtorCSs) const {
    // There could be one call for each field or none at all.
    if (DtorCSs.size() != Arrays.size())
      return false;

    const CallBase *CallPivot = DtorCSs[0];
    auto OffPivot = Offsets[0];

    for (const auto &P : zip(make_range(DtorCSs.begin() + 1, DtorCSs.end()),
                             make_range(Offsets.begin() + 1, Offsets.end()))) {
      const CallBase *Call = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareDtorCalls(CallPivot, Call, OffPivot, Off))
        return false;
    }

    if (!checkDtorsCallsAreAdjacent(DtorCSs))
      return false;

    return true;
  }

  // Topologically first/last under assumption that there are no cycles in BBs
  // (to be checked separately afterward or implied by traversal from first to
  // last).
  static std::pair<const BasicBlock *, const BasicBlock *>
  getTopSortFirstLastBB(SmallPtrSetImpl<const BasicBlock *> &BBs) {
    if (BBs.empty())
      return std::pair<const BasicBlock *, const BasicBlock *>(nullptr,
                                                               nullptr);

    unsigned Curr = 0;
    const BasicBlock *First = nullptr;
    const BasicBlock *Last = nullptr;

    for (auto *S : post_order(&(*BBs.begin())->getParent()->getEntryBlock())) {
      if (BBs.count(S) == 0)
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
  // Exception handling of ctors is checked in compareCtorBBs.
  bool checkCtorsCallsAreAdjacent(
      const SmallVectorImpl<const CallBase *> &CtorCSs) const {

    SmallPtrSet<const BasicBlock *, 2 * MaxNumFieldCandidates> BBs;
    SmallPtrSet<DTransStructType *, MaxNumFieldCandidates> ArrayTypes;
    SmallVector<const CallBase *, MaxNumFieldCandidates> NewCalls;
    const BasicBlock *AllocLandingPad = nullptr;
    // BasicBlock with allocation calls
    for (const CallBase *Call : CtorCSs) {
      BBs.insert(Call->getParent());
      // Relying on CtorDtorCheck::isThisArgNonInitialized
      auto *AllocCall =
          cast<Instruction>(Call->getArgOperand(0)->stripPointerCasts());

      auto *SI = &CtorDtorCheck::getStoreOfPointer(Call);
      ArrayTypes.insert(getArrayType(SI));
      NewCalls.push_back(dyn_cast<CallBase>(AllocCall));
      // Simple check of exception path of allocation function
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
    // One of the stores can be next basic block.
    const BasicBlock *LastStoreBB = nullptr;
    for (auto *BB = FL.first, *NextBB = BB /*Initial value ignored*/; BB;
         BB = NextBB /*See body of loop, Invoke case*/) {
      NextBB = nullptr;
      // Ignore additional instructions in first and last BB.
      for (auto &I : *BB)
        switch (I.getOpcode()) {
        case Instruction::BitCast:
        case Instruction::GetElementPtr:
          continue;
        case Instruction::Store:
          if (auto *ArrType =
                  StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I))
            ArrayTypes.erase(ArrType);
          continue;
        // Only need to make sure that load is not related to potential store
        // of newly allocated memory to fields, which are pointers to
        // transformed arrays.
        //
        // Checking that all loads are before aliasing stores.
        case Instruction::Load: {
          if (auto *ArrType =
                  StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I))
            // Load after store
            if (ArrayTypes.count(ArrType) == 0)
              return false;
          continue;
        }
        case Instruction::Invoke:
          // No control flow cycles are possible.
          NextBB = cast<InvokeInst>(&I)->getNormalDest();

          LLVM_FALLTHROUGH;
        case Instruction::Call:
          if (isa<DbgInfoIntrinsic>(I))
            continue;

          if (std::find_if(CtorCSs.begin(), CtorCSs.end(),
                           [&I](const CallBase *Call) -> bool {
                             return &I == Call;
                           }) != CtorCSs.end()) {
            ++NumCtorCalls;
            continue;
          }
          if (std::find_if(NewCalls.begin(), NewCalls.end(),
                           [&I](const CallBase *Call) -> bool {
                             return &I == Call;
                           }) != NewCalls.end()) {
            ++NumAllocCalls;
            continue;
          }
          if (FL.first == I.getParent()) {
            // Not required to check instructions in first basicblock
            // before AllocCall, which is called before actual constructor
            // is invoked. All instructions before first AllocCall can be
            // ignored to prove that Ctos can be combined safely. Since
            // I is neither AllocCall nor Ctor in first BB, just ignore
            // instructions till I. For now, just ignoring BitCast/
            // GetElementPtr/loads as they are before first Alloc call.
            BasicBlock::const_iterator EndIt = I.getIterator();
            BasicBlock::const_iterator It = FL.first->begin();
            for (; It != EndIt; ++It) {
              const Instruction &II = *It;
              switch (II.getOpcode()) {
              case Instruction::BitCast:
              case Instruction::GetElementPtr:
              case Instruction::Load:
                continue;
              default:
                if (isa<DbgInfoIntrinsic>(II))
                  continue;
                return false;
              }
            }
            continue;
          }
          return false;
        case Instruction::Ret:
          NextBB = nullptr;
          continue;
        default:
          return false;
        }

      if (BB == FL.second) {
        LastStoreBB = NextBB;
        NextBB = nullptr;
      }
    }

    if (!ArrayTypes.empty() && LastStoreBB) {
      // Ignore almost empty BB.
      if (LastStoreBB->size() == 1) {
        auto *Branch = dyn_cast<BranchInst>(LastStoreBB->getTerminator());
        if (Branch && Branch->isUnconditional())
          LastStoreBB = Branch->getSuccessor(0);
      }
      for (auto &I : *LastStoreBB) {
        if (ArrayTypes.empty())
          break;

        switch (I.getOpcode()) {
        case Instruction::BitCast:
        case Instruction::GetElementPtr:
        case Instruction::Br:
          continue;
        case Instruction::Store:
          if (auto *ArrType =
                  StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I))
            ArrayTypes.erase(ArrType);
          continue;
        case Instruction::Call:
          if (isa<DbgInfoIntrinsic>(I))
            continue;
          return false;
        default:
          return false;
        }
      }
    }

    // Make sure there is 1-1 correspondence between
    //  - allocation calls,
    //  - ctors
    //
    // 1-1 correspondence between ctor and stores is checked in
    // checkArrPtrStoreUses.
    if (NumAllocCalls != NewCalls.size() || NumCtorCalls != CtorCSs.size() ||
        !ArrayTypes.empty())
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
  // Exception handling of dtors is checked in compareDtorBBs.
  bool checkDtorsCallsAreAdjacent(
      const SmallVectorImpl<const CallBase *> &DtorCSs) const {

    SmallPtrSet<const BasicBlock *, 2 * MaxNumFieldCandidates> BBs;
    SmallPtrSet<const BasicBlock *, MaxNumFieldCandidates> DtorCalls;
    for (const CallBase *Call : DtorCSs) {
      auto *ParentBB = Call->getParent();
      BBs.insert(ParentBB);
      DtorCalls.insert(ParentBB);
      auto *DeleteBB = ParentBB;
      if (auto *Inv = dyn_cast<InvokeInst>(Call)) {
        // Expect normal delete call in next BB.
        DeleteBB = Inv->getNormalDest();
        BBs.insert(DeleteBB);
      }
    }

    auto FL = getTopSortFirstLastBB(BBs);
    unsigned NumDtorCalls = 0;
    unsigned FreePtrInd = -1U;
    SmallVector<const CallBase *, MaxNumFieldCandidates> RegDeleteCSs;
    SmallPtrSet<const Value *, MaxNumFieldCandidates> ThisArgs;
    for (auto *BB = FL.first, *NextBB = BB /*Initial value ignored*/; BB;
         BB = NextBB /*See body of loop, Invoke/Br case*/) {
      NextBB = nullptr;
      // Ignore additional instructions in first and last BB.
      for (auto &I : *BB)
        switch (I.getOpcode()) {
        case Instruction::BitCast:
        case Instruction::GetElementPtr:
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

            auto *L = dyn_cast<LoadInst>(Other->stripPointerCasts());
            if (!L)
              return false;

            // Check that Other is related to dtor call.
            // Simple check, no reloads.
            const CallBase *MethodCall = CtorDtorCheck::getSingleMethodCall(
                DTInfo, TLI, Other, getArrayType(Other->stripPointerCasts()));

            if (std::find_if(DtorCSs.begin(), DtorCSs.end(),
                             [MethodCall](const CallBase *Call) -> bool {
                               return MethodCall == Call;
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
          if (isa<DbgInfoIntrinsic>(I))
            continue;

          if (std::find_if(DtorCSs.begin(), DtorCSs.end(),
                           [&I](const CallBase *Call) -> bool {
                             return &I == Call;
                           }) != DtorCSs.end()) {
            ++NumDtorCalls;
            ThisArgs.insert(
                cast<CallBase>(&I)->getArgOperand(0)->stripPointerCasts());
            continue;
          }

          {
            auto *Info = DTInfo.getCallInfo(&I);
            if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free) {
              unsigned PtrArgInd = -1U;
              const CallBase *Call = cast<CallBase>(&I);
              getFreePtrArg(cast<dtrans::FreeCallInfo>(Info)->getFreeKind(),
                            Call, PtrArgInd, TLI);

              if (FreePtrInd == -1U)
                FreePtrInd = PtrArgInd;

              // Do not postpone till full comparison.
              if (FreePtrInd != PtrArgInd)
                return false;

              RegDeleteCSs.push_back(Call);
              continue;
            }
          }
          return false;
        case Instruction::Ret:
          NextBB = nullptr;
          continue;
        default:
          return false;
        }
      if (BB == FL.second)
        NextBB = nullptr;
    }

    // One delete for each dtor call.
    // And following regular path we should encounter all dtors.
    if (NumDtorCalls != DtorCSs.size() || RegDeleteCSs.size() != DtorCSs.size())
      return false;

    const CallBase *Pivot = RegDeleteCSs[0];
    auto PivotFreeArg = Pivot->getArgOperand(FreePtrInd)->stripPointerCasts();
    // Checking that deallocation is associated with dtor.
    if (ThisArgs.count(PivotFreeArg) == 0)
      return false;

    for (auto Del : make_range(RegDeleteCSs.begin() + 1, RegDeleteCSs.end())) {
      auto FreeArg = Del->getArgOperand(FreePtrInd)->stripPointerCasts();
      // Checking that deallocation is associated with dtor.
      if (ThisArgs.count(FreeArg) == 0)
        return false;
      if (!compareAllocDeallocCalls(Pivot, Del, PivotFreeArg, FreeArg))
        return false;
    }
    return true;
  }

  // See checkZeroInit.
  bool checkNullptrInits(const MemSetInst &MI) const {

    Constant *L = dyn_cast<Constant>(MI.getLength());
    if (!L)
      return false;
    uint64_t MaxOffset = L->getUniqueInteger().getLimitedValue();
    auto *SL = DL.getStructLayout(cast<StructType>(S.StrType->getLLVMType()));

    // Check that all pointers to arrays of interest are zeroed.
    for (auto Off : Offsets)
      if (SL->getElementOffset(Off) + DL.getPointerSize(0) > MaxOffset)
        return false;
    return true;
  }

  // Checks that initialization of pointers to arrays with nullptr are
  // adjacent.
  bool
  checkNullptrInits(const SmallVectorImpl<const StoreInst *> &Inits) const {
    if (Inits.size() != Arrays.size())
      return false;

    auto *PivotBB = Inits[0]->getParent();
    for (auto *NI : make_range(Inits.begin() + 1, Inits.end()))
      if (NI->getParent() != PivotBB)
        return false;

    SmallPtrSet<const StoreInst *, MaxNumFieldCandidates> InitSet(Inits.begin(),
                                                                  Inits.end());

    for (auto &I : *PivotBB) {
      if (InitSet.empty())
        break;
      switch (I.getOpcode()) {
      case Instruction::GetElementPtr:
        continue;
      case Instruction::Store:
        InitSet.erase(cast<StoreInst>(&I));
        continue;
      case Instruction::Call:
        if (isa<DbgInfoIntrinsic>(I))
          continue;
        return false;
      default:
        return false;
      }
    }
    return InitSet.empty();
  }

  // Extract array type from load or store,
  // ignoring some idioms related to bitcasts.
  DTransStructType *getArrayType(const Value *I) const {
    const Value *Address = nullptr;

    if (auto *SI = dyn_cast<StoreInst>(I))
      Address = SI->getPointerOperand();
    else if (auto *L = dyn_cast<LoadInst>(I))
      Address = L->getPointerOperand();
    else
      return nullptr;

    DTransType *PArray = nullptr;
    if (!StructIdioms::isFieldAddr(DM.getApproximation(Address), S, PArray))
      return nullptr;
    if (auto *Ptr = dyn_cast<DTransPointerType>(PArray))
      return dyn_cast<DTransStructType>(Ptr->getPointerElementType());

    return nullptr;
  }

  CallSiteComparator(const CallSiteComparator &) = delete;
  CallSiteComparator &operator=(const CallSiteComparator &) = delete;

  const DataLayout &DL;
  DTransSafetyInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;
  const SummaryForIdiom &S;

  // Array types
  const SmallVectorImpl<DTransStructType *> &Arrays;
  // Offsets of pointers to array types in S.StrType;
  const SmallVectorImpl<unsigned> &Offsets;

  // Information computed for transformation.
  const StructureMethodAnalysis::TransformationData &TI;

  const CallSitesInfo &CSInfo;
  unsigned BasePointerOffset;

public:
  CallSiteComparator(const DataLayout &DL, DTransSafetyInfo &DTInfo,
                     const TargetLibraryInfo &TLI, const DepMap &DM,
                     const SummaryForIdiom &S,
                     const SmallVectorImpl<DTransStructType *> &Arrays,
                     const SmallVectorImpl<unsigned> &Offsets,
                     const StructureMethodAnalysis::TransformationData &TI,
                     const CallSitesInfo &CSInfo, unsigned BasePointerOffset)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), DM(DM), S(S), Arrays(Arrays),
        Offsets(Offsets), TI(TI), CSInfo(CSInfo),
        BasePointerOffset(BasePointerOffset) {}

  bool canCallSitesBeMerged() const {
    // Check categories of methods called, finalizing uses checks of
    // loads/stores in StructureMethodAnalysis.
    SmallVector<const StoreInst *, MaxNumFieldCandidates> NullptrInits;
    SmallVector<const CallBase *, MaxNumFieldCandidates> CCtors;
    SmallVector<const CallBase *, MaxNumFieldCandidates> Ctors;
    SmallVector<const CallBase *, MaxNumFieldCandidates> Dtors;
    SmallVector<const CallBase *, MaxNumFieldCandidates> Appends;
    const MemSetInst *MI = nullptr;
    for (auto *I : TI.ArrayInstToTransform)
      if (auto *SI = dyn_cast<StoreInst>(I)) {

        // 1st kind of store: nullptr initialization.
        if (auto *C = dyn_cast<Constant>(SI->getValueOperand())) {
          if (C->isZeroValue()) {
            // Checking that null-ptr initializations are adjacent later.
            NullptrInits.push_back(SI);
            continue;
          } else
            return false;
        }

        // 2nd kind of store: newly allocated memory initialization.
        // Store should be processed checkArrPtrStoreUses.
        const CallBase *MethodCall = CtorDtorCheck::getSingleMethodCall(
            DTInfo, TLI, SI->getValueOperand(), getArrayType(I));
        if (!MethodCall)
          return false;
        auto *FCalled = dtrans::getCalledFunction(*MethodCall);
        // Not ctor call, hence cannot merge.
        if (std::find_if(CSInfo.Ctors.begin(), CSInfo.Ctors.end(),
                         [FCalled](const Function *FCtor) -> bool {
                           return FCalled == FCtor;
                         }) != CSInfo.Ctors.end())
          Ctors.push_back(MethodCall);
        else if (std::find_if(CSInfo.CCtors.begin(), CSInfo.CCtors.end(),
                              [FCalled](const Function *FCCtor) -> bool {
                                return FCalled == FCCtor;
                              }) != CSInfo.CCtors.end())
          CCtors.push_back(MethodCall);
        else
          return false;
      } else if (auto *L = dyn_cast<LoadInst>(I)) {
        auto IsUsed =
            CtorDtorCheck::isThereUseInFree(DTInfo, TLI, L, getArrayType(I));
        // Load is used in deallocation
        if (IsUsed.first) {
          assert(IsUsed.second && "StructureMethodAnalysis was not run");
          const CallBase *Call = IsUsed.second;
          auto FCalled = dtrans::getCalledFunction(*Call);
          if (std::find_if(CSInfo.Dtors.begin(), CSInfo.Dtors.end(),
                           [FCalled](const Function *FDtor) -> bool {
                             return FCalled == FDtor;
                           }) != CSInfo.Dtors.end())
            Dtors.push_back(Call);
          else
            return false;
        }
      } else if (auto *MS = dyn_cast<MemSetInst>(I)) {
        // Support only one memset, may extend analysis.
        if (MI)
          return false;
        MI = MS;
      } else if (const CallBase *Call = dyn_cast<CallBase>(I)) {
        auto *FCalled = dtrans::getCalledFunction(*Call);
        if (!FCalled)
          return false;
        auto *StrType = getOPStructTypeOfMethod(FCalled, &DTInfo);
        assert(StrType && "Expected class type for array method");
        assert(std::find(Arrays.begin(), Arrays.end(), StrType) !=
                   Arrays.end() &&
               "Unexpected instruction");

        if (std::find_if(CSInfo.Appends.begin(), CSInfo.Appends.end(),
                         [FCalled](const Function *FAppend) -> bool {
                           return FCalled == FAppend;
                         }) != CSInfo.Appends.end())
          Appends.push_back(Call);

        PtrTypeAnalyzer &PTA = DTInfo.getPtrTypeAnalyzer();
        auto *BasePtrType = StrType->getFieldType(BasePointerOffset);
        auto *Info = PTA.getValueTypeInfo(Call);
        assert(Info && "Expected PointerTypeAnalyzer to collect type");
        DTransType *Ty = PTA.getDominantType(*Info, ValueTypeInfo::VAT_Use);
        if (!Ty || BasePtrType != Ty)
          continue;

        DEBUG_WITH_TYPE(DTRANS_SOASTR,
                        dbgs() << "; Seen pointer to element returned.\n");

        // Check that methods returning pointers to elements are dereferenced.
        for (auto &U : Call->uses())
          if (!isa<LoadInst>(U.getUser()))
            return false;
      }

    if (!NullptrInits.empty()) {
      // Support only one memset, may extend analysis.
      if (MI)
        return false;
      DEBUG_WITH_TYPE(DTRANS_SOASTR, dbgs() << "; Seen nullptr init.\n");
      if (!checkNullptrInits(NullptrInits))
        return false;
    }
    if (MI) {
      DEBUG_WITH_TYPE(DTRANS_SOASTR,
                      dbgs() << "; Seen nullptr init with memset.\n");
      if (!checkNullptrInits(*MI))
        return false;
    }
    if (!Appends.empty()) {
      DEBUG_WITH_TYPE(DTRANS_SOASTR, dbgs() << "; Seen appends.\n");
      if (!compareAllAppendCallSites(Appends))
        return false;
    }
    if (!Ctors.empty()) {
      DEBUG_WITH_TYPE(DTRANS_SOASTR, dbgs() << "; Seen ctor.\n");
      if (!compareAllCtorCallSites(Ctors))
        return false;
    }
    if (!CCtors.empty()) {
      DEBUG_WITH_TYPE(DTRANS_SOASTR, dbgs() << "; Seen cctor.\n");
      if (!compareAllCCtorCallSites(CCtors))
        return false;
    }
    if (!Dtors.empty()) {
      DEBUG_WITH_TYPE(DTRANS_SOASTR, dbgs() << "; Seen dtor.\n");
      if (!compareAllDtorCallSites(Dtors))
        return false;
    }
    return true;
  }
};

class StructMethodTransformation {
  constexpr static int MaxNumFieldCandidates =
      SOAToAOSOPLayoutInfo::MaxNumFieldCandidates;

public:
  StructMethodTransformation(
      DTransSafetyInfo &DTInfo, ValueToValueMapTy &VMap,
      const CallSiteComparator::CallSitesInfo &CSInfo,
      const StructureMethodAnalysis::TransformationData &InstsToTransform,
      LLVMContext &Context, bool IsCloned,
      DenseMap<Function *, Function *> &ReverseVMap)
      : DTInfo(DTInfo), VMap(VMap), CSInfo(CSInfo),
        InstsToTransform(InstsToTransform), Context(Context),
        IsCloned(IsCloned), ReverseVMap(ReverseVMap) {}

  void updateReferences(DTransStructType *OldDTStruct,
                        DTransStructType *NewDTArray,
                        const SmallVectorImpl<DTransStructType *> &Arrays,
                        unsigned AOSOff, unsigned ElemOffset) const {
    IRBuilder<> Builder(Context);

    // Process load/stores first to adjust GEPs.
    // No instruction removed.
    for (auto *I : InstsToTransform.ArrayInstToTransform)
      if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
        const Value *OldPtr = getLoadStorePointerOperand(I);
        assert(OldPtr && "Load/Store must have pointer operand");

        if (auto *BC = dyn_cast<BitCastInst>(OldPtr)) {
          assert(isSafeBitCast(I->getFunction()->getParent()->getDataLayout(),
                               OldPtr, DTInfo.getPtrTypeAnalyzer()) &&
                 "Unexpected store address");
          OldPtr = BC->getOperand(0);
        }

        auto *Ptr = cast<Instruction>((Value *)VMap[OldPtr]);
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
      } else if (isa<ICmpInst>(I))
        continue;
      else if (isa<CallBase>(I))
        continue;
      else
        llvm_unreachable("Unexpected instruction to transform.");

    // Deal with combined call sites.
    SmallVector<const CallInst *, MaxNumFieldCandidates> OldAppends;
    for (auto *I : InstsToTransform.ArrayInstToTransform) {
      auto *NewI = cast_or_null<Instruction>((Value *)VMap[I]);
      // Removed instructions.
      if (!NewI)
        continue;
      if (auto *Call = dyn_cast<CallBase>(NewI)) {
        const auto *OldCall = dyn_cast<CallBase>(I);
        auto *FCalled = dtrans::getCalledFunction(*OldCall);
        assert(FCalled && "Expected direct call");
        // 'this' argument has the same type as pointer to arrays in S.StrType.
        bool ToRemove = getOPStructTypeOfMethod(FCalled, &DTInfo) !=
                        getOPSOAArrayType(OldDTStruct, AOSOff);

        // When routine is not cloned, original "Append" call is replaced
        // by new "Append" function. ReverseVMap is used to get original
        // "Append" function.
	Function *OrgF = IsCloned ? FCalled : ReverseVMap.lookup(FCalled);
        if (OrgF && std::find(CSInfo.Appends.begin(), CSInfo.Appends.end(),
                              OrgF) != CSInfo.Appends.end())
          // See compareAllAppendCallSites, appends should be in a single
          // BasicBlock and CallInst.
          OldAppends.push_back(cast<CallInst>(I));
        else if (ToRemove)
          if (std::find(CSInfo.Ctors.begin(), CSInfo.Ctors.end(),
                        dtrans::getCalledFunction(*OldCall)) !=
                  CSInfo.Ctors.end() ||
              std::find(CSInfo.CCtors.begin(), CSInfo.CCtors.end(),
                        dtrans::getCalledFunction(*OldCall)) !=
                  CSInfo.CCtors.end() ||
              std::find(CSInfo.Dtors.begin(), CSInfo.Dtors.end(),
                        dtrans::getCalledFunction(*OldCall)) !=
                  CSInfo.Dtors.end())
            removeCtorDtor(Call);
      }
    }
    updateAppends(OldAppends, Arrays, ElemOffset);
  }

private:
  // Coupled with checkArrPtrStoreUses, checkArrPtrLoadUses.
  void removeCtorDtor(CallBase *Call) const {
    IRBuilder<> Builder(Context);
    // Alloc or load of a pointer.
    auto *This = cast<Instruction>(Call->getArgOperand(0)->stripPointerCasts());

    SmallSetVector<Instruction *, 16> Insts;
    // Trivial uses are removed in RecursivelyDeleteTriviallyDeadInstructions.
    for (auto *User : post_order(CastDepGraph<Value *>(This))) {
      for (auto &U : User->uses())
        if (auto *I = dyn_cast<Instruction>(U.getUser()))
          // Avoid potential double removal.
          if (!wouldInstructionBeTriviallyDead(I) &&
              !CtorDtorCheck::isNullCheck(I))
            Insts.insert(I);
      if (auto *I = dyn_cast<Instruction>(User))
        // Avoid potential double removal.
        if (!wouldInstructionBeTriviallyDead(I) &&
            !CtorDtorCheck::isNullCheck(I))
          Insts.insert(I);
    }

    for (auto *Inst : Insts) {
      bool ToRemove = false;
      if (auto *Call = dyn_cast<CallBase>(Inst)) {
        if (auto *Inv = dyn_cast<InvokeInst>(Inst)) {
          Builder.SetInsertPoint(Inv);
          Builder.CreateBr(Inv->getNormalDest());
        }
        DTInfo.deleteCallInfo(Call);
        ToRemove = true;
      }
      // Some instructions may point to null checks in ICmpInst.
      // Redundant null-checks can be removed if their users (conditional
      // branches are removed as well).
      //
      // Essential instructions (allocation/deallocation/methods) should be
      // removed here due to checkArrPtrLoadUses/checkArrPtrStoreUses.
      else if (Inst->hasNUses(0))
        ToRemove = true;

      if (ToRemove) {
        auto *I = cast<Instruction>(Inst);

        SmallPtrSet<Instruction *, 5> Ops;
        for (auto &Op : I->operands())
          if (auto *OpI = dyn_cast<Instruction>(Op.get()))
            Ops.insert(OpI);

        salvageDebugInfo(*I);
        I->eraseFromParent();
        for (auto *OpI : Ops)
          RecursivelyDeleteTriviallyDeadInstructions(OpI);
      }
    }
  }

  // See compareAllAppendCallSites, appends should be in a single
  // BasicBlock and CallInst. Final instruction can be inserted before
  // terminator.
  //
  // Coupled with FunctionType updates.
  void updateAppends(const SmallVectorImpl<const CallInst *> &OldAppends,
                     const SmallVectorImpl<DTransStructType *> &Arrays,
                     unsigned ElemOffset) const {
    if (OldAppends.empty())
      return;

    assert(OldAppends.size() == Arrays.size() &&
           "Inconsistency with compareAllAppendCallSites");

    // Array to hold call OldAppends in order consistent with Arrays.
    SmallVector<const CallInst *, MaxNumFieldCandidates> SortedAppends(
        Arrays.size(), nullptr);
    for (auto *CI : OldAppends) {
      // When routine is not cloned, original "Append" call is replaced
      // by new "Append" function. ReverseVMap is used to get original
      // "Append" function.
      auto *Func = IsCloned ? dtrans::getCalledFunction(*CI)
                            : ReverseVMap[dtrans::getCalledFunction(*CI)];
      auto *ArrayTy = getOPStructTypeOfMethod(Func, &DTInfo);

      auto It = std::find(Arrays.begin(), Arrays.end(), ArrayTy);
      assert(It != Arrays.end() && "Incorrect append method");
      SortedAppends[It - Arrays.begin()] = CI;
    }

    auto *FCalled =
        IsCloned ? dtrans::getCalledFunction(*SortedAppends[0])
                 : ReverseVMap[dtrans::getCalledFunction(*SortedAppends[0])];
    assert(FCalled && "Expected direct call");
    auto *OldFunctionTy = dyn_cast_or_null<DTransFunctionType>(
        DTInfo.getTypeMetadataReader().getDTransTypeFromMD(FCalled));
    assert(OldFunctionTy && "Must have type if function is being transformed");

    auto *NewAppend = cast<CallInst>((Value *)VMap[SortedAppends[0]]);
    auto *ArrType = getOPStructTypeOfMethod(FCalled, &DTInfo);
    assert(ArrType && "Expected class type for array method");
    auto *ElementType = getOPSOAElementType(ArrType, ElemOffset);

    unsigned Offset = -1U;
    SmallVector<const Value *, 6> Args;
    for (unsigned I = 0, E = OldFunctionTy->getNumArgs(); I != E; ++I) {
      auto *Param = OldFunctionTy->getArgType(I);
      ++Offset;
      if (Param == ElementType) {
        for (auto *CI : SortedAppends)
          Args.push_back(CI->getArgOperand(Offset));
        continue;
      }

      if (auto *Ptr = dyn_cast<DTransPointerType>(Param))
        if (Ptr->getPointerElementType() == ElementType) {
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
    auto *NewF = dtrans::getCalledFunction(*NewAppend);
    assert(NewF && "Expected direct call");
    Builder.CreateCall(NewF->getFunctionType(), NewF, NewArgs);

    for (auto *CI : OldAppends) {
      auto *NewCI = cast<CallInst>((Value *)VMap[CI]);

      SmallPtrSet<Instruction *, 5> Ops;
      for (auto &Op : NewCI->operands())
        if (auto *OpI = dyn_cast<Instruction>(Op.get()))
          Ops.insert(OpI);
      salvageDebugInfo(*NewCI);
      NewCI->eraseFromParent();
      for (auto *OpI : Ops)
        RecursivelyDeleteTriviallyDeadInstructions(OpI);
    }
  }

  DTransSafetyInfo &DTInfo;
  ValueToValueMapTy &VMap;
  const CallSiteComparator::CallSitesInfo &CSInfo;
  const StructureMethodAnalysis::TransformationData &InstsToTransform;
  LLVMContext &Context;
  bool IsCloned;
  DenseMap<Function *, Function *> &ReverseVMap;
};
} // namespace soatoaosOP
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Instructions to transform.
struct SOAToAOSOPStructMethodsCheckDebugResult
    : public StructureMethodAnalysis::TransformationData,
      public CallSiteComparator::CallSitesInfo {};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace dtransOP
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOPSTRUCT_H
