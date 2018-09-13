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

  static bool isDirectMemoryInterfaceLoad(const Dep *D,
                                          const SummaryForIdiom &S,
                                          unsigned &ArgNo) {
    if (D->Kind != Dep::DK_Load)
      return false;

    Type* OutType = nullptr;
    if (!Idioms::isFieldAddr(D->Arg1, S, ArgNo, OutType))
      return false;

    return isa<PointerType>(OutType) &&
           OutType->getPointerElementType() == S.MemoryInterface;
  }

  // Returns type array type accessed.
  // Returns nullptr if load is not related to array transformed.
  static StructType *
  isLoadOrStoreOfArrayPtr(const DepMap &DM,
                          // Types of interest
                          const SmallVectorImpl<StructType *> &Arrays,
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

    Type *FieldType = nullptr;
    // Unstructured store.
    if (!StructIdioms::isFieldAddr(DM.getApproximation(Address), S, FieldType))
      return nullptr;

    // Not a pointer to array.
    if (!isa<PointerType>(FieldType))
      return nullptr;

    auto *FPointeeTy = FieldType->getPointerElementType();

    // Layout restriction.
    // See
    // SOAToAOSTransformImpl::CandidateLayoutInfo::populateLayoutInformation.
    auto *ArrStrType = cast<StructType>(FPointeeTy);
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

  // Checks that 'this' argument is dead after a call to CS,
  // because it is passed to deallocation routine immediately after call to CS.
  //
  // Implementation: checks that 'this' is used in free.
  // free should be in successors BasicBlocks, see compareDtorCalls below.
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

    auto *BB = CS.getInstruction()->getParent();

    for (auto *User : post_order(CastDepGraph<const Value *>(ThisArg)))
      for (auto &U : User->uses()) {
        if (isCastUse(U))
          continue;

        if (!isa<Instruction>(U.getUser()))
          return false;

        if (U.getUser() == CS.getInstruction())
          continue;

        auto *Inst = cast<Instruction>(U.getUser());
        if (isFreedPtr(DTInfo, TLI, U))
          DeleteBB.insert(Inst->getParent());
        // Other uses in successors are forbidden.
        else if (find(successors(BB), Inst->getParent()) != succ_end(BB))
          return false;

        // Check same BB.
        if (BB == Inst->getParent())
          for (auto *I = CS.getInstruction(); I; I = I->getNextNode())
            if (I == Inst)
              return true;
      }

    // Simple CFG handling: all successors should contain delete.
    if (!all_of(successors(CS.getInstruction()->getParent()),
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
                   const TargetLibraryInfo &TLI, const Value *V,
                   StructType *ArrType) {
    bool FreeUseSeen = false;
    ImmutableCallSite SingleMethod;
    for (auto *User : post_order(CastDepGraph<const Value *>(V)))
      for (auto &U : User->uses())
        if (auto CS = ImmutableCallSite(U.getUser())) {
          // Direct call.
          if (auto *F = CS.getCalledFunction())
            if (getStructTypeOfMethod(*F) == ArrType) {
              if (SingleMethod)
                return std::make_pair(FreeUseSeen, ImmutableCallSite());
              SingleMethod = CS;
              continue;
            }
          if (CtorDtorCheck::isFreedPtr(DTInfo, TLI, U))
            FreeUseSeen = true;
        }
    return std::make_pair(FreeUseSeen, SingleMethod);
  }

  // Given V, returns single using method call (deallocation functions are
  // ignored), returns nullptr otherwise.
  static ImmutableCallSite getSingleMethodCall(const DTransAnalysisInfo &DTInfo,
                                               const TargetLibraryInfo &TLI,
                                               const Value *V,
                                               StructType *ArrType) {
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
  static const StoreInst& getStoreOfPointer(ImmutableCallSite CS) {
    auto *Alloc = CS.getArgOperand(0)->stripPointerCasts();
    for (auto *User : post_order(CastDepGraph<const Value *>(Alloc)))
      for (auto &U : User->uses())
        if (auto *SI = dyn_cast<StoreInst>(U.getUser()))
          return *SI;
    llvm_unreachable("Incorrect CS provided in getStoreOfPointer.");
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
  bool checkArrPtrLoadUses(const LoadInst &L, StructType *ArrType) const {
    // Transformation restriction:
    // isSafeBitCast/isBitCastLikeGep/isSafeIntToPtr are not permitted.
    if (!isa<GetElementPtrInst>(L.getPointerOperand()))
      return false;

    // Check that all uses are related to array method calls,
    // to null-check or to call to deallocation.
    bool FreeUseSeen = false;
    unsigned NumMethodsCalled = 0;

    // MS compiler complains for missing braces.
    for (auto *User : post_order(CastDepGraph<const Value *>(&L))) {
      for (auto &U : User->uses()) {
        if (isCastUse(U)) {
          continue;
        } else if (auto CS = ImmutableCallSite(U.getUser())) {
          if (auto *F = CS.getCalledFunction()) {
            // CFG restriction, use can be only for 'this' argument.
            //
            // Safety check of CFG based on the 'this' argument.
            // See dtrans::StructInfo::CallSubGraph.
            //
            // Also see populateLayoutInformation.
            //
            // Method call.
            if (ArrType == getStructTypeOfMethod(*F)) {
              ++NumMethodsCalled;
              // insert optimistically
              insertArrayInst(CS.getInstruction(), "Array method call");
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
  bool checkArrPtrStoreUses(const StoreInst &SI, StructType *ArrType) const {
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
    auto *Val = SI.getValueOperand()->stripPointerCasts();
    if (!isa<Instruction>(Val))
      return false;

    if (!StructIdioms::isAlloc(DM.getApproximation(Val), S))
      return false;

    bool MethodSeen = false;
    // MS compiler complains for missing braces.
    for (auto *User : post_order(CastDepGraph<const Value *>(Val))) {
      for (auto &U : User->uses()) {
        if (isCastUse(U)) {
          continue;
        } else if (auto CS = ImmutableCallSite(U.getUser())) {
          // Use of stored value in ctor is checked in CallSiteComparator.
          if (auto *F = CS.getCalledFunction())
            if (ArrType == getStructTypeOfMethod(*F)) {
              // Store should be associated with single method, i.e. with ctor.
              // Called value is checked in CallSiteComparator.
              if (MethodSeen) {
                return false;
              }
              // insert optimistically
              insertArrayInst(CS.getInstruction(), "Array method call");
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
                          const SmallVectorImpl<StructType *> &Arrays,
                          TransformationData &TI)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), DM(DM), S(S), Arrays(Arrays),
        TI(TI) {}

  unsigned getTotal() const { return TI.ArrayInstToTransform.size(); }

  // TODO: there is no need to check all methods of struct,
  // only methods accessing arrays, i.e. isLoadOrStoreOfArrayPtr and memset of
  // structure.
  bool checkStructMethod() const {
    bool Invalid = false;

    for (auto &I : instructions(*S.Method)) {
      if (arith_inst_dep_iterator::isSupportedOpcode(I.getOpcode()))
        continue;

      auto *D = DM.getApproximation(&I);
      if (!D)
        llvm_unreachable(
            "Not synchronized checkStructMethod and computeDepApproximation");

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
          if (auto *ArrType =
                  StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I)) {
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
          // Fall through to Handled = false.
        }
        // memset of memory pointed-to by some field,
        // For example, memset of elements of array, which is not transformed.
        else if (StructIdioms::isStructuredStore(D, S))
          break;
        // Memset of structure itself.
        else if (auto *MI = dyn_cast<MemSetInst>(&I)) {
          // Safety checks guarantee (CFG + legality) that constant address does not
          // refer to array of interest.
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
  const DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;

  // Summary for StructIdioms checks.
  const SummaryForIdiom &S;

  // Array types
  const SmallVectorImpl<StructType *> &Arrays;

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
  constexpr static int MaxNumFieldCandidates = 2;
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

    auto *ElementType1 =
        getSOAElementType(getSOAArrayType(S.StrType, Off1), BasePointerOffset);
    auto *ElementType2 =
        getSOAElementType(getSOAArrayType(S.StrType, Off2), BasePointerOffset);

    // Same BasicBlock for combining.
    if (CS1.getInstruction()->getParent() != CS2.getInstruction()->getParent())
      return false;

    for (auto P : zip_first(CS1.args(), CS2.args())) {
      auto *A1 = std::get<0>(P).get();
      auto *A2 = std::get<1>(P).get();
      if (A1 == A2)
        continue;

      auto *T1 = A1->getType();
      auto *T2 = A2->getType();

      bool IsElemA1 = T1 == ElementType1;
      bool IsElemA2 = T2 == ElementType2;
      if (IsElemA1 != IsElemA2)
        return false;
      if (IsElemA1)
        continue;

      if (!isa<PointerType>(T1) || !isa<PointerType>(T2))
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

      // Potentially conflicting stores to fields containing pointers to arrays
      // are excluded by checking stores associated with ctors.
      // Stores are associated with ctors by check in canCallSitesBeMerged.
    }
    return true;
  }

  // Single BasicBlock:
  //  this1 = str->arr1
  //  call append(this1, elem1)
  //  this2 = str->arr2
  //  call append(this2, elem2)
  bool compareAllAppendCallSites(
      const SmallVectorImpl<ImmutableCallSite> &CSs) const {
    // There could be one call for each field or none at all. Do not permit
    // multiple groups of append methods in a single Struct's method. It
    // simplifies transformation.
    if (CSs.size() != Arrays.size())
      return false;

    auto CSPivot = CSs[0];
    auto OffPivot = Offsets[0];
    for (auto P : zip(make_range(CSs.begin() + 1, CSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end())))
      if (!compareAppendCallSites(CSPivot, std::get<0>(P), OffPivot,
                                  std::get<1>(P)))
        return false;

    // See note about conflicting stores in compareAppendCallSites.
    // Start checking when first load/call/invoke is encountered.
    bool StartCmp = false;
    for (auto &I : *CSPivot.getInstruction()->getParent())
      switch (I.getOpcode()) {
      case Instruction::Load:
        StartCmp = true;
        continue;
      case Instruction::Call:
        StartCmp = true;
        if (std::find_if(CSs.begin(), CSs.end(),
                         [&I](ImmutableCallSite CS) -> bool {
                           return &I == CS.getInstruction();
                         }) == CSs.end())
          return false;
        continue;
      case Instruction::BitCast:
      case Instruction::Br:
      case Instruction::GetElementPtr:
      case Instruction::ICmp:
      case Instruction::Ret:
        continue;
      default:
        if (StartCmp)
          return false;
        continue;
      }
    return true;
  }

  // Compare calls to allocation/deallocation function.
  //
  // FreePtr1 and FreePtr2 are pointers to memory to deallocated (difference
  // ignored). Ignored for allocation function.
  //
  // Remaining arguments are equal or loads of MemoryInterface.
  bool compareAllocDeallocCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                                const Value *FreePtr1,
                                const Value *FreePtr2) const {

    if (CS1.getCalledValue() != CS2.getCalledValue())
      return false;

    auto *Info = DTInfo.getCallInfo(CS1.getInstruction());

    SmallPtrSet<const Value *, 3> Args;
    bool Alloc = false;
    if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free) {
      collectSpecialFreeArgs(cast<FreeCallInfo>(Info)->getFreeKind(), CS1, Args,
                             TLI);
      assert(Args.size() == 1 && "Unexpected deallocation function");

      Alloc = false;
    } else if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc) {
      collectSpecialAllocArgs(cast<AllocCallInfo>(Info)->getAllocKind(), CS1,
                              Args, TLI);

      assert(Args.size() == 1 && "Unexpected allocation function");
      Alloc = true;
    } else
      return false;

    for (auto PA : zip_first(CS1.args(), CS2.args())) {
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
      } else if (A1 != A2) {
        unsigned ArgNo1 = -1U;
        unsigned ArgNo2 = -1U;
        // TODO: check ArgNos
        if (!StructIdioms::isDirectMemoryInterfaceLoad(DM.getApproximation(A1),
                                                       S, ArgNo1) ||
            !StructIdioms::isDirectMemoryInterfaceLoad(DM.getApproximation(A2),
                                                       S, ArgNo2))
          return false;
      }
    }
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
  bool compareCtorDtorBBs(ImmutableCallSite CS1, ImmutableCallSite CS2) const {

    assert(CS1.isInvoke() && CS1.isInvoke() && "Incorrect arguments");
    auto *FreePtr1 = CS1.getArgOperand(0)->stripPointerCasts();
    auto *FreePtr2 = CS2.getArgOperand(0)->stripPointerCasts();

    const BasicBlock *BB1 = nullptr;
    const BasicBlock *BB2 = nullptr;

    BB1 = cast<InvokeInst>(CS1.getInstruction())->getUnwindDest();
    BB2 = cast<InvokeInst>(CS2.getInstruction())->getUnwindDest();

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
    for (auto P : zip_first(successors(BB1), successors(BB2))) {
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
        break;
      default:
        return false;
      }

      // Delete invocations are processed completely.
      if (auto CS1 = ImmutableCallSite(&I1)) {
        if (!compareAllocDeallocCalls(CS1, ImmutableCallSite(&I2), FreePtr1,
                                      FreePtr2))
          return false;
        continue;
      }

      for (auto PO : zip_first(I1.operands(), I2.operands())) {
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
  // Cleanup BasicBlock should be equal as in compareCtorDtorBBs.
  bool compareCtorCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                        unsigned Off1, unsigned Off2, bool Copy) const {

    if (!CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, dyn_cast<Function>(CS1.getCalledValue()),
            DL.getTypeAllocSize(getSOAArrayType(S.StrType, Off1))) ||
        !CtorDtorCheck::isThisArgNonInitialized(
            DTInfo, TLI, dyn_cast<Function>(CS2.getCalledValue()),
            DL.getTypeAllocSize(getSOAArrayType(S.StrType, Off2))))
      return false;

    // Parameters should be equal as Values with 3 exceptions:
    //  - 'this' parameter (checked above);
    //  - memory interface access (relying on fact it is only copied around);
    //  - second parameter in copy-ctor should be derived from 2nd argument.
    bool ThisArg = true;
    for (auto P : zip_first(CS1.args(), CS2.args())) {
      auto *A1 = std::get<0>(P).get()->stripPointerCasts();
      auto *A2 = std::get<1>(P).get()->stripPointerCasts();

      if (ThisArg) {
        ThisArg = false;
        if (!compareAllocDeallocCalls(ImmutableCallSite(A1),
                                      ImmutableCallSite(A2), nullptr, nullptr))
          return false;
        continue;
      }

      if (Copy) {
        unsigned ArgNo1 = -1U;
        unsigned ArgNo2 = -1U;
        if (!StructIdioms::isFieldLoad(
                DM.getApproximation(A1), Off1, ArgNo1) ||
            !StructIdioms::isFieldLoad(
                DM.getApproximation(A2), Off2, ArgNo2) ||
            ArgNo1 != 1 || ArgNo2 != 1)
          return false;

        // Potentially conflicting stores to fields containing pointers to
        // arrays are excluded by checking stores associated with ctors. Stores
        // are associated with ctors by check in canCallSitesBeMerged.
        continue;
      }

      if (A1 == A2)
        continue;

      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      // See isMemoryInterface* checks in checkStructMethod.
      // TODO: check ArgNos
      if (!StructIdioms::isDirectMemoryInterfaceLoad(DM.getApproximation(A1), S,
                                                     ArgNo1) ||
          !StructIdioms::isDirectMemoryInterfaceLoad(DM.getApproximation(A2), S,
                                                     ArgNo2))
        return false;
    }

    if (CS1.isInvoke() != CS2.isInvoke() ||
        (CS1.isInvoke() && !compareCtorDtorBBs(CS1, CS2)))
      return false;

    return true;
  }

  // Compare invokes of ctors and checks adjacency of invokes.
  bool compareAllCtorCallSites(
      const SmallVectorImpl<ImmutableCallSite> &CtorCSs) const {
    // There could be one call for each field or none at all.
    if (CtorCSs.size() != Arrays.size())
      return false;

    auto CSPivot = CtorCSs[0];
    auto OffPivot = Offsets[0];

    for (auto P : zip(make_range(CtorCSs.begin() + 1, CtorCSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end()))) {
      auto CS = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareCtorCalls(CSPivot, CS, OffPivot, Off, false))
        return false;
    }

    if (!checkCtorsCallsAreAdjacent(CtorCSs))
      return false;
    return true;
  }

  // Compare invokes of copy ctors and checks adjacency of invokes.
  bool compareAllCCtorCallSites(
      const SmallVectorImpl<ImmutableCallSite> &CCtorCSs) const {
    // There could be one call for each field or none at all.
    if (CCtorCSs.size() != Arrays.size())
      return false;

    auto CSPivot = CCtorCSs[0];
    auto OffPivot = Offsets[0];

    if (CSPivot.arg_size() != 2)
      return false;

    for (auto P : zip(make_range(CCtorCSs.begin() + 1, CCtorCSs.end()),
                      make_range(Offsets.begin() + 1, Offsets.end()))) {
      auto CS = std::get<0>(P);
      auto Off = std::get<1>(P);
      if (!compareCtorCalls(CSPivot, CS, OffPivot, Off, true))
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
  // Cleanup BasicBlock should be equal as in compareCtorDtorBBs.
  bool compareDtorCalls(ImmutableCallSite CS1, ImmutableCallSite CS2,
                        unsigned Off1, unsigned Off2) const {

    if (!CtorDtorCheck::isThisArgIsDead(DTInfo, TLI, CS1) ||
        !CtorDtorCheck::isThisArgIsDead(DTInfo, TLI, CS2))
      return false;

    if (CS1.arg_size() != 1 || CS2.arg_size() != 1)
      return false;

    // Compare argument.
    {
      unsigned ArgNo1 = -1U;
      unsigned ArgNo2 = -1U;
      if (!StructIdioms::isFieldLoad(
              DM.getApproximation(CS1.getArgOperand(0)), Off1, ArgNo1) ||
          !StructIdioms::isFieldLoad(
              DM.getApproximation(CS2.getArgOperand(0)), Off2, ArgNo2) ||
          ArgNo1 != 0 || ArgNo2 != 0)
        return false;

      // Potentially conflicting stores to fields containing pointers to arrays
      // are excluded by checking stores associated with ctors.
      // Stores are associated with ctors by check in canCallSitesBeMerged.
    }

    // Deallocation on normal path is checked in checkDtorsCallsAreAdjacent.
    if (CS1.isInvoke() != CS2.isInvoke() ||
        (CS1.isInvoke() && !compareCtorDtorBBs(CS1, CS2)))
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
      const SmallVectorImpl<ImmutableCallSite> &DtorCSs) const {
    // There could be one call for each field or none at all.
    if (DtorCSs.size() != Arrays.size())
      return false;

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
  // Exception handling of ctors is checked in compareCtorDtorBBs.
  bool checkCtorsCallsAreAdjacent(
      const SmallVectorImpl<ImmutableCallSite> &CtorCSs) const {

    SmallPtrSet<const BasicBlock *, 2 * MaxNumFieldCandidates> BBs;
    SmallPtrSet<StructType*, MaxNumFieldCandidates> ArrayTypes;
    SmallVector<ImmutableCallSite, MaxNumFieldCandidates> NewCalls;
    const BasicBlock *AllocLandingPad = nullptr;
    // BasicBlock with allocation calls
    for (auto CS : CtorCSs) {
      BBs.insert(CS.getInstruction()->getParent());
      // Relying on CtorDtorCheck::isThisArgNonInitialized
      auto *AllocCall =
          cast<Instruction>(CS.getArgOperand(0)->stripPointerCasts());

      auto *SI = &CtorDtorCheck::getStoreOfPointer(CS);
      ArrayTypes.insert(getArrayType(SI));
      NewCalls.push_back(ImmutableCallSite(AllocCall));
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
            if (ArrayTypes.find(ArrType) == ArrayTypes.end())
              return false;
          continue;
        }
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

      if (BB == FL.second) {
        LastStoreBB = NextBB;
        NextBB = nullptr;
      }
    }

    if (!ArrayTypes.empty() && LastStoreBB)
      for (auto &I : *LastStoreBB) {
        if (ArrayTypes.empty())
          break;

        switch (I.getOpcode()) {
        case Instruction::BitCast:
        case Instruction::GetElementPtr:
          continue;
        case Instruction::Store:
          if (auto *ArrType =
                  StructIdioms::isLoadOrStoreOfArrayPtr(DM, Arrays, S, I))
            ArrayTypes.erase(ArrType);
          continue;
        default:
          return false;
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
  // Exception handling of dtors is checked in compareCtorDtorBBs.
  bool checkDtorsCallsAreAdjacent(
      const SmallVectorImpl<ImmutableCallSite> &DtorCSs) const {

    SmallPtrSet<const BasicBlock *, 2 * MaxNumFieldCandidates> BBs;
    SmallPtrSet<const BasicBlock *, MaxNumFieldCandidates> DtorCalls;
    for (auto CS : DtorCSs) {
      auto *ParentBB = CS.getInstruction()->getParent();
      BBs.insert(ParentBB);
      DtorCalls.insert(ParentBB);
      auto *DeleteBB = ParentBB;
      if (auto *Inv = dyn_cast<InvokeInst>(CS.getInstruction())) {
        // Expect normal delete call in next BB.
        DeleteBB = Inv->getNormalDest();
        BBs.insert(DeleteBB);
      }
    }

    auto FL = getTopSortFirstLastBB(BBs);
    unsigned NumDtorCalls = 0;
    unsigned FreePtrInd = -1U;
    SmallVector<ImmutableCallSite, MaxNumFieldCandidates> RegDeleteCSs;
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
            auto MethodCS = CtorDtorCheck::getSingleMethodCall(
                DTInfo, TLI, Other, getArrayType(Other->stripPointerCasts()));

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
            ThisArgs.insert(
                ImmutableCallSite(&I).getArgOperand(0)->stripPointerCasts());
            continue;
          }

          {
            auto *Info = DTInfo.getCallInfo(&I);
            if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free) {
              unsigned PtrArgInd = -1U;
              getFreePtrArg(cast<FreeCallInfo>(Info)->getFreeKind(),
                            ImmutableCallSite(&I), PtrArgInd, TLI);

              if (FreePtrInd == -1U)
                FreePtrInd = PtrArgInd;

              // Do not postpone till full comparison.
              if (FreePtrInd != PtrArgInd)
                return false;

              RegDeleteCSs.push_back(ImmutableCallSite(&I));
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
    if (NumDtorCalls != DtorCSs.size() || RegDeleteCSs.size() != DtorCSs.size())
      return false;

    auto Pivot = RegDeleteCSs[0];
    auto PivotFreeArg = Pivot.getArgOperand(FreePtrInd)->stripPointerCasts();
    // Checking that deallocation is associated with dtor.
    if (ThisArgs.find(PivotFreeArg) == ThisArgs.end())
      return false;

    for (auto Del : make_range(RegDeleteCSs.begin() + 1, RegDeleteCSs.end())) {
      auto FreeArg = Del.getArgOperand(FreePtrInd)->stripPointerCasts();
      // Checking that deallocation is associated with dtor.
      if (ThisArgs.find(FreeArg) == ThisArgs.end())
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
    auto *SL = DL.getStructLayout(S.StrType);


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
      default:
        return false;
      }
    }
    return InitSet.empty();
  }

  // Extract array type from load or store,
  // ignoring some idioms related to bitcasts.
  StructType *getArrayType(const Value *I) const {
    const Value *Address = nullptr;

    if (auto *SI = dyn_cast<StoreInst>(I))
      Address = SI->getPointerOperand();
    else if (auto *L = dyn_cast<LoadInst>(I))
      Address = L->getPointerOperand();
    else
      return nullptr;

    Type *PArray = nullptr;
    if (!StructIdioms::isFieldAddr(DM.getApproximation(Address), S, PArray))
      return nullptr;
    if (auto *Ptr = dyn_cast<PointerType>(PArray))
      return dyn_cast<StructType>(Ptr->getElementType());

    return nullptr;
  }

  CallSiteComparator(const CallSiteComparator &) = delete;
  CallSiteComparator &operator=(const CallSiteComparator &) = delete;

  const DataLayout &DL;
  const DTransAnalysisInfo &DTInfo;
  const TargetLibraryInfo &TLI;
  const DepMap &DM;
  const SummaryForIdiom &S;

  // Array types
  const SmallVectorImpl<StructType *> &Arrays;
  // Offsets of pointers to array types in S.StrType;
  const SmallVectorImpl<unsigned> &Offsets;

  // Information computed for transformation.
  const StructureMethodAnalysis::TransformationData &TI;

  const CallSitesInfo &CSInfo;
  unsigned BasePointerOffset;

public:
  CallSiteComparator(const DataLayout &DL, const DTransAnalysisInfo &DTInfo,
                     const TargetLibraryInfo &TLI, const DepMap &DM,
                     const SummaryForIdiom &S,
                     const SmallVectorImpl<StructType *> &Arrays,
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
    SmallVector<ImmutableCallSite, MaxNumFieldCandidates> CCtors;
    SmallVector<ImmutableCallSite, MaxNumFieldCandidates> Ctors;
    SmallVector<ImmutableCallSite, MaxNumFieldCandidates> Dtors;
    SmallVector<ImmutableCallSite, MaxNumFieldCandidates> Appends;
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
        auto CS = CtorDtorCheck::getSingleMethodCall(
            DTInfo, TLI, SI->getValueOperand(), getArrayType(I));
        auto *FCalled = CS.getCalledFunction();
        // Not ctor call, hence cannot merge.
        if (std::find_if(CSInfo.Ctors.begin(), CSInfo.Ctors.end(),
                         [FCalled](const Function *FCtor) -> bool {
                           return FCalled == FCtor;
                         }) != CSInfo.Ctors.end())
          Ctors.push_back(CS);
        else if (std::find_if(CSInfo.CCtors.begin(), CSInfo.CCtors.end(),
                         [FCalled](const Function *FCCtor) -> bool {
                           return FCalled == FCCtor;
                         }) != CSInfo.CCtors.end())
          CCtors.push_back(CS);
        else
          return false;
      } else if (auto *L = dyn_cast<LoadInst>(I)) {
        auto IsUsed =
            CtorDtorCheck::isThereUseInFree(DTInfo, TLI, L, getArrayType(I));
        // Load is used in deallocation
        if (IsUsed.first) {
          assert(IsUsed.second && "StructureMethodAnalysis was not run");
          auto CS = IsUsed.second;
          auto FCalled = CS.getCalledFunction();
          if (std::find_if(CSInfo.Dtors.begin(), CSInfo.Dtors.end(),
                           [FCalled](const Function *FDtor) -> bool {
                             return FCalled == FDtor;
                           }) != CSInfo.Dtors.end())
            Dtors.push_back(CS);
          else
            return false;
        }
      } else if (auto *MS = dyn_cast<MemSetInst>(I)) {
        // Support only one memset, may extend analysis.
        if (MI)
          return false;
        MI = MS;
      } else if (auto CS = ImmutableCallSite(I)) {
        auto *FCalled = CS.getCalledFunction();
        auto *StrType = getStructTypeOfMethod(*FCalled);
        assert(std::find(Arrays.begin(), Arrays.end(), StrType) !=
                   Arrays.end() &&
               "Unexpected instruction");

        if (std::find_if(CSInfo.Appends.begin(), CSInfo.Appends.end(),
                         [FCalled](const Function *FAppend) -> bool {
                           return FCalled == FAppend;
                         }) != CSInfo.Appends.end())
          Appends.push_back(CS);

        auto *BasePtrType = StrType->getTypeAtIndex(BasePointerOffset);
        if (BasePtrType != CS.getType())
          continue;

        // Check that methods returning pointers to elements are dereferenced.
        for (auto &U : CS.getInstruction()->uses())
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
      DEBUG_WITH_TYPE(DTRANS_SOASTR, dbgs() << "; Seen nullptr init with memset.\n");
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
  constexpr static int MaxNumFieldCandidates = 2;
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
                        const SmallVectorImpl<StructType *> &Arrays,
                        unsigned AOSOff, unsigned ElemOffset) const {
    IRBuilder<> Builder(Context);

    // Process load/stores first to adjust GEPs.
    // No instruction removed.
    for (auto *I : InstsToTransform.ArrayInstToTransform)
      if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
        const Value *OldPtr = nullptr;
        if (auto *L = dyn_cast<LoadInst>(I))
          OldPtr = L->getPointerOperand();
        else if (auto *SI = dyn_cast<StoreInst>(I))
          OldPtr = SI->getPointerOperand();

        if (isBitCastLikeGep(DL, OldPtr))
          continue;

        if (auto *BC = dyn_cast<BitCastInst>(OldPtr)) {
          assert(isSafeBitCast(DL, OldPtr) && "Unexpected store address");
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
      else if (ImmutableCallSite(I))
        continue;
      else
        llvm_unreachable("Unexpected instruction to transform.");

    // Deal with combined call sites.
    SmallVector<const CallInst*, MaxNumFieldCandidates> OldAppends;
    for (auto *I : InstsToTransform.ArrayInstToTransform) {
      auto *NewI = cast_or_null<Instruction>((Value *)VMap[I]);
      // Removed instructions.
      if (!NewI)
        continue;
      if (auto CS = CallSite(NewI)) {
        auto OldCS = ImmutableCallSite(I);
        // 'this' argument has the same type as pointer to arrays in S.StrType.
        bool ToRemove = getStructTypeOfMethod(*OldCS.getCalledFunction()) !=
                        getSOAArrayType(OldStruct, AOSOff);

        if (std::find(CSInfo.Appends.begin(), CSInfo.Appends.end(),
                      OldCS.getCalledFunction()) != CSInfo.Appends.end())
          // See compareAllAppendCallSites, appends should be in a single
          // BasicBlock and CallInst.
          OldAppends.push_back(cast<CallInst>(I));
        else if (ToRemove)
          if (std::find(CSInfo.Ctors.begin(), CSInfo.Ctors.end(),
                        OldCS.getCalledFunction()) != CSInfo.Ctors.end() ||
              std::find(CSInfo.CCtors.begin(), CSInfo.CCtors.end(),
                        OldCS.getCalledFunction()) != CSInfo.CCtors.end() ||
              std::find(CSInfo.Dtors.begin(), CSInfo.Dtors.end(),
                        OldCS.getCalledFunction()) != CSInfo.Dtors.end())
            removeCtorDtor(CS);
      }
    }
    updateAppends(OldAppends, Arrays, ElemOffset);
  }

private:
  // Coupled with checkArrPtrStoreUses, checkArrPtrLoadUses.
  void removeCtorDtor(CallSite CS) const {
    IRBuilder<> Builder(Context);
    // Alloc or load of a pointer.
    auto *This = cast<Instruction>(CS.getArgOperand(0)->stripPointerCasts());

    SmallVector<Value*, 16> Insts;
    for (auto *User : post_order(CastDepGraph<Value *>(This))) {
      for (auto &U : User->uses())
        if (!isCastUse(U) && !CtorDtorCheck::isNullCheck(U.getUser()))
          Insts.push_back(U.getUser());
      if (!CtorDtorCheck::isNullCheck(User))
        Insts.push_back(User);
    }

    for (auto *V : Insts) {
      if (auto CS = CallSite(V)) {
        if (auto *Inv = dyn_cast<InvokeInst>(V)) {
          Builder.SetInsertPoint(Inv);
          Builder.CreateBr(Inv->getNormalDest());
        }
        DTInfo.deleteCallInfo(CS.getInstruction());
        CS.getInstruction()->eraseFromParent();
      }
      // Some instructions may point to null checks in ICmpInst.
      // Redundant null-checks can be removed if their users (conditional
      // branches are removed as well).
      //
      // Essential instructions (allocation/deallocation/methods) should be
      // removed here due to checkArrPtrLoadUses/checkArrPtrStoreUses.
      else if (V->hasNUses(0))
        // Unused addresses of stores can be removed here.
        cast<Instruction>(V)->eraseFromParent();
    }
  }

  // See compareAllAppendCallSites, appends should be in a single
  // BasicBlock and CallInst. Final instruction can be inserted before
  // terminator.
  //
  // Coupled with FunctionType updates.
  void updateAppends(const SmallVectorImpl<const CallInst *> &OldAppends,
                     const SmallVectorImpl<StructType *> &Arrays,
                     unsigned ElemOffset) const {
    if (OldAppends.empty())
      return;

    assert(OldAppends.size() == Arrays.size() &&
           "Inconsistency with compareAllAppendCallSites");

    // Array to hold call OldAppends in order consistent with Arrays.
    SmallVector<const CallInst *, MaxNumFieldCandidates> SortedAppends(
        Arrays.size(), nullptr);
    for (auto *CI : OldAppends) {
      auto *ArrayTy = getStructTypeOfMethod(*CI->getCalledFunction());

      auto It = std::find(Arrays.begin(), Arrays.end(), ArrayTy);
      assert(It != Arrays.end() && "Incorrect append method");
      SortedAppends[It - Arrays.begin()] = CI;
    }

    auto *NewAppend = cast<CallInst>((Value *)VMap[SortedAppends[0]]);
    auto *OldFunctionTy = SortedAppends[0]->getFunctionType();
    auto *ArrType =
        getStructTypeOfMethod(*SortedAppends[0]->getCalledFunction());
    auto *ElementType = getSOAElementType(ArrType, ElemOffset);

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
