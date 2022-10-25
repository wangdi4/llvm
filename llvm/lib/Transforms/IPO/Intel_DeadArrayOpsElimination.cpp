#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_DeadArrayOpsElimination.cpp ----------------------------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the optimization that avoids sorting array elements
// by qsort function if the elements are not really used later.
// This transformation depends on the below:
//  Intel_QsortRecognizer: Recognizes qsort functions and marks them with
//  "is-qsort" attribute.
//
//  Array Range Analysis: For a given array, this provides lowest and highest
//  indexes of the array used after any given point.
//
// Ex:
// Let us assume only first 60 elements of an array are used after sorting
// using qsort function. There is no need to sort elements of the array after
// 60th index. We will change the recursion of the qsort such that the first
// 60 sorted elements will be placed in order from 0 to 59 locations and
// ignore sorting the remaining elements in the array.
//
// Before:
//   {
//     array[461];
//     ...
//     qsort(array, n);
//
//     // Assume only first 60 elements of array are used after qsort.
//     ...
//   }
//
//   qsort(a, n) {
//      ...
//      qsort(a, n1);
//
//      // Iterate rather than recurse
//      // qsort(new_address, n2);
//   }
//
// After:
//   {
//     array[461];
//     ...
//     ptr = array + 60;  // Create new address that points to 61st element.
//
//     qsort(array, n, ptr); // Pass ptr as last argument.
//
//     // Assume only first 60 elements of array are used after qsort.
//     ...
//   }
//
//   qsort(a, n, ptr) { // "ptr" is never changed.
//      ...
//      if (a < ptr)      // Skip calling recursive call if "a" >= "ptr"
//        qsort(a, n1, ptr);
//
//      // Iterate rather than recurse
//      // qsort(new_address, n2);
//   }
//
//

#include "llvm/Transforms/IPO/Intel_DeadArrayOpsElimination.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_ArrayUseAnalysis.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

using DeadArrayUseType = std::function<ArrayUse &(Function &)>;

#define DEAD_ARRAY_OPS_ELIMI "deadarrayopselimination"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This option is mainly used by LIT tests to indicate qsort functions and
// high indexes of arrays used.
//
// ';'-separated list of items:
// - <qsort function name>,<High index of array, which is passed as first
//    argument to the qsort function call, used after call>
//
// Ignore user input if it is invalid.
//
static cl::list<std::string> DeadArrayOpsFunctions("dead-array-ops-functions",
                                                   cl::ReallyHidden);

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Class that has all the info related to each candidate.
class CandidateInfo {
public:
  CandidateInfo(Module &M, DeadArrayUseType &GetAUse)
      : M(M), GetAUse(GetAUse) {}
  bool isValidCandidate(Function *F);
  bool isOnlyPartOfArrayUsed(int64_t HI);
  int64_t getArrayUsedHighIdx(void);
  void transform();

private:
  Module &M;

  DeadArrayUseType GetAUse;

  // Size of array that is passed to qsort. For now, only local arrays are
  // supported. This holds the size of array from allocation (i.e Alloca).
  int64_t ArraySize = -1;

  // This represents the highest index of used elements. After sorting,
  // no elements after this index will be used.
  int64_t UsedHighIndex = -1;

  // Qsort function.
  Function *SortFn = nullptr;

  // Recursive call of qsort. Only one recursive call is expected in qsort.
  // This will be updated multiple times after qsort function and then fixing
  // the callsite.
  CallInst *RecursionCall = nullptr;

  // First call to qsort. Only one call (other than the recursion call) is
  // expected.
  CallInst *FirstCall = nullptr;

  // Cloned Qsort function.
  Function *NewSortFn = nullptr;

  // Element type of Array
  Type *ArrayElemTy = nullptr;

  bool applySanityChecks(void);
  bool isLocalArrayPassedAsFirstArg(void);
  void createNewQsortFunction(void);
  void fixQsortCallsites(void);
  bool wrapRecursionCallUsingExistingCond(void);
  void wrapRecursionCallUnderCond(void);
};

// Main class to implement the transformation.
class DeadArrayOpsElimImpl {

public:
  DeadArrayOpsElimImpl(Module &M, WholeProgramInfo &WPInfo,
                       DeadArrayUseType &GetAUse)
      : M(M), WPInfo(WPInfo), GetAUse(GetAUse){};
  ~DeadArrayOpsElimImpl() {
    for (auto *C : Candidates)
      delete C;
  }
  bool run(void);

private:
  Module &M;
  WholeProgramInfo &WPInfo;
  DeadArrayUseType &GetAUse;

  // Max number of candidates allowed.
  constexpr static int MaxNumCandidates = 2;

  // Set of candidates.
  SmallPtrSet<CandidateInfo *, MaxNumCandidates> Candidates;

  // This is used to maintain user given data using dead-array-elem-functions
  // option. This can be controlled under "!defined(NDEBUG) ||
  // defined(LLVM_ENABLE_DUMP)".
  DenseMap<Function *, int64_t> UserCandInfo;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void parseOption(void);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  void gatherCandidates(void);
  void applyTransformations(void);
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// parse DeadArrayOpsFunctions option and collect used input in
// UserCandInfo. This is mainly used for LIT tests.
void DeadArrayOpsElimImpl::parseOption() {

  if (DeadArrayOpsFunctions.empty())
    return;

  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                  { dbgs() << " Parsing dead-array-ops-functions option\n"; });

  for (auto &Opt : DeadArrayOpsFunctions) {
    if (Opt.empty())
      continue;
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "\tList: " << Opt << "\n"; });

    StringRef OptRef(Opt);
    SmallVector<StringRef, 8> ListCandidates;
    OptRef.split(ListCandidates, ';');

    for (auto &UserCand : ListCandidates) {

      SmallVector<StringRef, 2> CandItem;
      UserCand.split(CandItem, ',');

      if (CandItem.size() == 0)
        continue;

      if (CandItem.size() != 2) {
        DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
          dbgs() << UserCand << "is ignored (Incorrect format)\n";
        });
        continue;
      }
      auto *Func = M.getFunction(CandItem[0]);
      if (!Func) {
        DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
          dbgs() << UserCand << "is ignored (Incorrect function)\n";
        });
        continue;
      }
      int64_t HighIdx = -1;
      if (CandItem[1].getAsInteger(10, HighIdx)) {
        DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
          dbgs() << UserCand << "is ignored (Incorrect highindex)\n";
        });
        continue;
      }
      if (UserCandInfo.count(Func)) {
        DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
          dbgs() << UserCand << "is ignored (duplicate function)\n";
        });
        continue;
      }
      DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
        dbgs() << "   User Candidate: " << Func->getName() << "  " << HighIdx
               << "\n";
      });
      UserCandInfo[Func] = HighIdx;
    }
  }
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Checks if entire BasicBlock of recursive qsort function call can be
// wrapped using existing condition and then add new condition without
// changing CFG. Returns true if the transformation is done.
//
// Before:
//     PredBB:
//       %existing_cond = cmp ...
//       br %existing_cond, label %SortBB, label %SuccBB
//
//     SortBB:
//       %p1 = lsr ...
//       qsort(%a1, %p1, ...)
//       br label %SuccBB
//
//     SuccBB:
//       ...
//
// After:
//     PredBB:
//       %existing_cond = cmp ...
//       %new_cond = cmp ult %a1, %2
//       %merge_cond = and %new_cond, %existing_cond
//       br %merge_cond, label %SortBB, label %SuccBB
//
//     SortBB:
//       %p1 = lsr ...
//       qsort(%a1, %p1, ...)
//       br label %SuccBB
//
//     SuccBB:
//       ...
//
bool CandidateInfo::wrapRecursionCallUsingExistingCond() {
  BasicBlock *SortBB = RecursionCall->getParent();
  BasicBlock *PredBB = SortBB->getSinglePredecessor();
  // Check CFG is okay to wrap entire SortBB.
  if (!PredBB)
    return false;
  auto *PBI = dyn_cast<BranchInst>(PredBB->getTerminator());
  if (!PBI || !PBI->isConditional() || SortBB != PBI->getSuccessor(0) ||
      SortBB->getSingleSuccessor() != PBI->getSuccessor(1))
    return false;
  Value *FirstArg = RecursionCall->getArgOperand(0);
  // Makes sure SortBB has only instructions that related to RecursionCall.
  for (auto &I : *SortBB) {
    // Ignore RecursionCall/Debug/Branch instructions.
    if (&I == RecursionCall || isa<DbgInfoIntrinsic>(I) || isa<BranchInst>(I))
      continue;
    // It is okay if instruction is used as an argument of RecursionCall.
    // Make sure FirstArg is not changed in the SortBB since FirstArg
    // is will be used to create new condition.
    if (I.hasOneUse() && I.use_begin()->getUser() == RecursionCall &&
        &I != FirstArg)
      continue;
    return false;
  }

  // Apply transformation here.
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    Before  wrapping :\n";
    dbgs() << "    Cond BB: \n" << *PredBB << "\n";
    dbgs() << "    Sort BB:\n" << *SortBB << "\n";
  });
  Value *PBICond = PBI->getCondition();
  Value *NewArg = NewSortFn->getArg(NewSortFn->arg_size() - 1);
  IRBuilder<> ICBuilder(PBI);
  Value *Cond;
  Cond = ICBuilder.CreateICmpULT(FirstArg, NewArg);
  Value *MergeCond = ICBuilder.CreateAnd(Cond, PBICond, "");
  PBI->setCondition(MergeCond);

  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    After  wrapping :\n";
    dbgs() << "    Cond BB: \n" << *PredBB << "\n";
    dbgs() << "    Sort BB:\n" << *SortBB << "\n";
  });
  return true;
}

// Wrap RecursionCall in cloned Qsort function under a newly created
// condition to avoid sorting unused elements of array.
//
// Before:
//   qsort(a, n, NewArg) {
//     ...
//     qsort(a1, n1, NewArg)
//   }
//
// After:
//   qsort(a, n, NewArg) {
//     ...
//     if (a1 < NewArg)
//       qsort(a1, n1, NewArg)
//   }
//
void CandidateInfo::wrapRecursionCallUnderCond() {

  assert(RecursionCall && "Invalid new recursion call");
  // Try using exising cond to wrap qsort call.
  if (wrapRecursionCallUsingExistingCond())
    return;

  // If it is not possible to wrap entire SortBB, split SortBB BasicBlock
  // and then wrap only RecursionCall under new condition.
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    Before wrapping:\n" << *RecursionCall->getParent() << "\n";
  });
  // Just using some random branch weights.
  MDNode *BranchWeights =
      MDBuilder(RecursionCall->getContext()).createBranchWeights(1, 65);

  Value *NewArg = NewSortFn->getArg(NewSortFn->arg_size() - 1);
  IRBuilder<> ICBuilder(RecursionCall);
  Value *Cond =
      ICBuilder.CreateICmpULT(RecursionCall->getArgOperand(0), NewArg);

  Instruction *NewInst =
      SplitBlockAndInsertIfThen(Cond, RecursionCall, false, BranchWeights);
  BasicBlock *SortBB = NewInst->getParent();
  BasicBlock *SuccBB = SortBB->getSingleSuccessor();
  (void)SuccBB;
  assert(SuccBB && "The split block should have a single successor");
  RecursionCall->removeFromParent();
  SortBB->getInstList().insert(SortBB->getFirstInsertionPt(), RecursionCall);
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    Cond: " << *Cond << "\n\n";
    dbgs() << "    After  wrapping :\n";
    dbgs() << "    Cond BB: \n" << *SortBB->getSinglePredecessor() << "\n";
    dbgs() << "    Sort BB:\n" << *SortBB << "\n";
    dbgs() << "    Succ BB: \n" << *SuccBB << "\n";
  });
}

// Clone Qsort function and add one more argument at the end of
// argument list. Type of last argument will be same as 1st argument.
//
// Before:
//   Qsort(i8* a, i32 n) { }
//
// After:
//   Qsort(i8* a, i32 n) { }
//   Clone_Of_Qsort(i8* a, i32 n, i8* NewArg) { }
//
void CandidateInfo::createNewQsortFunction() {
  std::vector<Type *> NewParams;
  FunctionType *FTy = SortFn->getFunctionType();
  for (auto I = SortFn->arg_begin(), E = SortFn->arg_end(); I != E; ++I)
    NewParams.push_back(&*I->getType());
  // Add one more Param at the end of argument list.
  NewParams.push_back(SortFn->getArg(0)->getType());

  // Create new Sort function by cloning SortFn
  FunctionType *NewFTy =
      FunctionType::get(FTy->getReturnType(), NewParams, FTy->isVarArg());
  NewSortFn =
      Function::Create(NewFTy, SortFn->getLinkage(), SortFn->getName(), &M);
  ValueToValueMapTy VMap;
  auto A = NewSortFn->arg_begin();
  for (auto I = SortFn->arg_begin(), E = SortFn->arg_end(); I != E; ++I, ++A)
    VMap[&*I] = &*A;
  SmallVector<ReturnInst *, 8> Rets;
  CloneFunctionInto(NewSortFn, SortFn, VMap,
                    CloneFunctionChangeType::LocalChangesOnly, Rets);
  NewSortFn->copyAttributesFrom(SortFn);
  NewSortFn->setComdat(SortFn->getComdat());

  // Update RecursionCall after cloning is done.
  RecursionCall = cast<CallInst>((Value *)VMap[RecursionCall]);

  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    New Qsort function created: " << NewSortFn->getName()
           << "\n";
  });
}

// Fix both call sites of Qsort.
//
// Before:
//   foo()
//   {
//     ...
//     Qsort(a, n);  // FirstCall
//   }
//
//   Cloned_Qsort(a, n, NewArg) {
//     ...
//     Qsort(a, n);  // RecursionCall
//   }
//
// After:
//   foo()
//   {
//     ...
//     NewArg = a + HighUsedIndex;
//     Cloned_Qsort(a, n, NewArg);  // FirstCall
//   }
//
//   Cloned_Qsort(a, n, NewArg) {
//     ...
//     Cloned_Qsort(a, n, NewArg);  // RecursionCall
//   }
//
void CandidateInfo::fixQsortCallsites() {
  // Set function of "CB" to NewSortFn and then add "NewArg" as last
  // argument.
  auto FixCallSite = [this](CallInst *CB, Value *NewArg) {
    std::vector<Value *> NewArgs;
    auto NFPAL = NewSortFn->getAttributes();
    AttributeList Attrs = CB->getAttributes();
    SmallVector<AttributeSet, 4> NewArgAttrs;

    unsigned ArgIdx = 0;
    auto *I = CB->arg_begin();
    auto *E = CB->arg_end();
    for (; I != E; I++, ArgIdx++) {
      NewArgs.push_back(*I);
      NewArgAttrs.push_back(NFPAL.getParamAttrs(ArgIdx));
    }
    NewArgs.push_back(NewArg);
    NewArgAttrs.push_back(NFPAL.getParamAttrs(ArgIdx));
    FunctionType *NFTy = NewSortFn->getFunctionType();
    AttributeList NewPAL =
        AttributeList::get(NFTy->getContext(), Attrs.getFnAttrs(),
                           Attrs.getRetAttrs(), NewArgAttrs);

    CallInst *NewCB;
    NewCB = CallInst::Create(NFTy, NewSortFn, NewArgs, None, "", CB);
    NewCB->setTailCallKind(CB->getTailCallKind());
    NewCB->setCallingConv(CB->getCallingConv());
    NewCB->setDebugLoc(CB->getDebugLoc());
    NewCB->setAttributes(NewPAL);
    if (!CB->use_empty() || CB->isUsedByMetadata())
      CB->replaceAllUsesWith(NewCB);
    CB->eraseFromParent();
    return NewCB;
  };

  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    RecursionCall before: " << *RecursionCall << "\n";
  });
  Value *NewArg = NewSortFn->getArg(NewSortFn->arg_size() - 1);
  RecursionCall = FixCallSite(RecursionCall, NewArg);
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    RecursionCall after: " << *RecursionCall << "\n";
  });

  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    FirstCall before: " << *FirstCall << "\n";
  });
  Value *AArg = FirstCall->getArgOperand(0);
  Type *FirstArgTy = AArg->getType();
  Type *ArrayElemAddrType = ArrayElemTy->getPointerTo(0);
  Value *GEPPtrOp = AArg;
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                  { dbgs() << "    FirstCall after:\n"; });
  if (ArrayElemAddrType != FirstArgTy) {
    GEPPtrOp = CastInst::CreateBitOrPointerCast(AArg, ArrayElemAddrType, "",
                                                FirstCall);
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    " << *GEPPtrOp << "\n"; });
  }
  SmallVector<Value *, 2> Indices;
  Value *NewOp = ConstantInt::get(
      Type::getIntNTy(M.getContext(), M.getDataLayout().getPointerSizeInBits()),
      UsedHighIndex);
  Indices.push_back(NewOp);

  Value *HighIndexAddr =
      GetElementPtrInst::Create(ArrayElemTy, GEPPtrOp, Indices, "", FirstCall);
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                  { dbgs() << "    " << *HighIndexAddr << "\n"; });
  if (ArrayElemAddrType != FirstArgTy) {
    HighIndexAddr = CastInst::CreateBitOrPointerCast(HighIndexAddr, FirstArgTy,
                                                     "", FirstCall);
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    " << *HighIndexAddr << "\n"; });
  }
  FirstCall = FixCallSite(FirstCall, HighIndexAddr);
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, { dbgs() << *FirstCall << "\n"; });
}

// Performs all necessary transformations.
void CandidateInfo::transform() {
  // Create new cloned Qsort function.
  createNewQsortFunction();
  // Fix both callsites of Qsort.
  fixQsortCallsites();
  // Wrap RecursionCall under newly created condition.
  wrapRecursionCallUnderCond();
  // Remove original SortFn.
  SortFn->eraseFromParent();
}

// Return false if SortFn fails any basic checks. These checks are not
// really needed as QsortRecognizer already verified them.
bool CandidateInfo::applySanityChecks(void) {

  if (SortFn->isDeclaration() || !SortFn->hasLocalLinkage() ||
      !SortFn->hasNUses(2) || SortFn->hasAddressTaken() || !SortFn->arg_size())
    return false;
  return true;
}

// Only local array of pointers is allowed as 1st argument when qsort
// function is called first time. Get array size if everything is okay.
//
//  Ex 1: (Array size: 460)
//  %5 = alloca [461 x %struct.b*]
//  %9 = getelementptr [461 x %struct.b*], [461 x %struct.b*]* %5, i64 0, i64 0
//  %210 = getelementptr inbounds %struct.b*, %struct.b** %9, i64 1
//  %211 = bitcast %struct.b** %210 to i8*
//  qsort(i8* nonnull %211, i64 %207)
//
//  or
//
//  Ex 2: (Array size: 461)
//  %5 = alloca [461 x %struct.b*]
//  %9 = getelementptr [461 x %struct.b*], [461 x %struct.b*]* %5, i64 0, i64 0
//  %211 = bitcast %struct.b** %9 to i8*
//  qsort(i8* nonnull %211, i64 %207)
//
//
// NOTE: The above example includes bitcasts and typed pointers. I will update
// it to not have bitcasts and typed pointers when -opaque-pointers is the
// default for xmain.
//
bool CandidateInfo::isLocalArrayPassedAsFirstArg(void) {
  assert(FirstCall && "Expected first call to qsort function");
  unsigned PtrIncrement = 0;
  Value *FirstArg = FirstCall->getArgOperand(0);
  if (!FirstArg->getType()->isPointerTy())
    return false;
  // Skip Bitcasts.
  Value *ArrayArg = FirstArg->stripPointerCasts();
  // Check if ArrayArg is like
  //
  // %0 = bitcast [491 x %struct.b*]* %perm to %struct.b**
  // %add.ptr = getelementptr %struct.b*, %struct.b** %0, i64 1
  //
  // or
  //
  // %add.ptr = getelementptr [491 x %struct.b*], [491 x %struct.b*]* %0, i64 0,
  // i64 1
  //
  if (auto *GEP = dyn_cast<GetElementPtrInst>(ArrayArg)) {
    Value *GEPOp;
    if (GEP->getNumIndices() == 1) {
      GEPOp = GEP->getOperand(1);

    } else if (GEP->getNumIndices() == 2) {
      auto *ATy = dyn_cast<ArrayType>(GEP->getSourceElementType());
      if (!ATy)
        return false;
      GEPOp = GEP->getOperand(2);
    } else {
      return false;
    }
    auto *OpC = dyn_cast<ConstantInt>(GEPOp);
    if (!OpC || !OpC->isOne())
      return false;
    PtrIncrement = 1;
    // Skip BitCasts and GEPs that don't change pointer value.
    ArrayArg = GEP->getOperand(0)->stripPointerCastsSameRepresentation();
  }
  // Check ArrayArg is static AllocaInst
  auto *AI = dyn_cast<AllocaInst>(ArrayArg);
  if (!AI || !AI->isStaticAlloca())
    return false;
  // Make sure it is allocating 1-D array of pointers.
  ArrayType *ATy = dyn_cast<ArrayType>(AI->getAllocatedType());
  if (!ATy)
    return false;
  if (!ATy->getElementType()->isPointerTy())
    return false;
  ArrayElemTy = ATy->getElementType();
  auto *CI = dyn_cast<ConstantInt>(AI->getArraySize());
  if (!CI)
    return false;
  if (CI->getZExtValue() != 1)
    return false;
  // Get size of the array.
  ArraySize = ATy->getNumElements() - PtrIncrement;
  if (ArraySize <= 1)
    return false;
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                  { dbgs() << "    ArraySize: " << ArraySize << "\n"; });
  return true;
}

// Get used high index for the array using "Array Range Analysis".
// Returns -1 if unable to detect Range Info or the array is not
// partially used after Qsort call.
int64_t CandidateInfo::getArrayUsedHighIdx(void) {
  Function *F = FirstCall->getFunction();
  ArrayUse &AU = (GetAUse)(*F);
  Value *FirstArg = FirstCall->getArgOperand(0);
  const ArrayUseInfo *AUI = AU.getSourceArray(FirstArg);
  if (!AUI) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    Array Use Info: Failed \n"; });
    return -1;
  }
  ArrayRangeInfo ARI = AUI->getRangeUseAfterPoint(FirstCall);
  if (ARI.isEmpty() || ARI.isFull()) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    Array Use Info: Full or Empty \n"; });
    return -1;
  }
  auto *HIdx = dyn_cast<SCEVConstant>(ARI.getHighIndex());
  if (!HIdx) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    Array Use Info: Not constant \n"; });
    return -1;
  }
  int64_t HighIdx = HIdx->getAPInt().getSExtValue();
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                  { dbgs() << "    Array Use High Idx:" << HighIdx << "\n"; });
  return HighIdx;
}

// Basic checks are done for UsedHighIndex.
// TODO: May need to add some heuristics for UsedHighIndex if we need
// them in future.
bool CandidateInfo::isOnlyPartOfArrayUsed(int64_t HighIdx) {
  assert(ArraySize != -1 && " Incorrect array  size");

  if (HighIdx < 0 || HighIdx >= ArraySize - 1) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    failed: no partial use of array\n"; });
    return false;
  }
  UsedHighIndex = HighIdx;
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    UsedHighIndex: " << UsedHighIndex << "\n";
  });
  return true;
}

// Apply some basic checks to make sure qsort transformations can be applied
// to "F". Return true if it is okay.
bool CandidateInfo::isValidCandidate(Function *F) {
  SortFn = F;
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "  DeadArrayOpsElimi: Considering qsort function: "
           << F->getName() << "\n";
  });

  if (!applySanityChecks()) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI,
                    { dbgs() << "    failed: sanity checks\n"; });
    return false;
  }
  unsigned ArgSize = SortFn->arg_size();

  // Identify first call and recursive call of SortFn.
  for (User *U : SortFn->users()) {
    auto *CB = dyn_cast<CallInst>(U);
    if (!CB || CB->getCalledFunction() != SortFn || ArgSize != CB->arg_size())
      break;
    if (CB->getFunction() == SortFn)
      RecursionCall = CB;
    else
      FirstCall = CB;
  }
  if (!FirstCall || !RecursionCall) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
      dbgs() << "    failed: can't find first and recursion calls \n";
    });
    return false;
  }
  DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
    dbgs() << "    First Call: " << *FirstCall << "\n";
    dbgs() << "    Recursion Call: " << *RecursionCall << "\n";
  });

  if (!isLocalArrayPassedAsFirstArg()) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
      dbgs() << "    failed:  array argument is not valid \n";
    });
    return false;
  }
  return true;
}

// Collect possible candidates.
void DeadArrayOpsElimImpl::gatherCandidates(void) {
  // Check if F is possible candidate.
  auto IsCandidate = [this](Function *F) {
    // Is this function specified as qsort through option.
    if (UserCandInfo.count(F))
      return true;
    // Is function marked with is-qsort.
    if (F->hasFnAttribute("is-qsort"))
      return true;
    return false;
  };

  for (auto &F : M.functions()) {
    if (!IsCandidate(&F))
      continue;
    std::unique_ptr<CandidateInfo> CandD(new CandidateInfo(M, GetAUse));
    if (!CandD->isValidCandidate(&F))
      continue;
    int64_t HighIdx;
    // Get UsedHighIndex value.
    if (UserCandInfo.count(&F))
      HighIdx = UserCandInfo[&F];
    else
      HighIdx = CandD->getArrayUsedHighIdx();
    if (!CandD->isOnlyPartOfArrayUsed(HighIdx))
      continue;
    Candidates.insert(CandD.release());
  }
}

// Apply transformations to all candidates.
void DeadArrayOpsElimImpl::applyTransformations(void) {
  for (auto *C : Candidates)
    C->transform();
}

bool DeadArrayOpsElimImpl::run(void) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  parseOption();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  gatherCandidates();
  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
      dbgs() << " DeadArrayOpsElimi Failed: No candidates found.\n";
    });
    return false;
  }
  if (Candidates.size() > MaxNumCandidates) {
    DEBUG_WITH_TYPE(DEAD_ARRAY_OPS_ELIMI, {
      dbgs() << "DeadArrayOpsElimi: Candidate found\n";
      dbgs() << "    Failed: More candidates found.\n";
    });
    return false;
  }
  applyTransformations();
  return true;
}

namespace {

struct DeadArrayOpsEliminationLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  DeadArrayOpsEliminationLegacyPass(void) : ModulePass(ID) {
    initializeDeadArrayOpsEliminationLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<ArrayUseWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    DeadArrayUseType GetAUse = [this](Function &F) -> ArrayUse & {
      return this->getAnalysis<ArrayUseWrapperPass>(F).getArrayUse();
    };
    DeadArrayOpsElimImpl DeadArrayElim(M, WPInfo, GetAUse);
    return DeadArrayElim.run();
  }
};

} // namespace

char DeadArrayOpsEliminationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(DeadArrayOpsEliminationLegacyPass,
                      "deadarrayopselimination", "DeadArrayOpsElimination",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ArrayUseWrapperPass)
INITIALIZE_PASS_END(DeadArrayOpsEliminationLegacyPass,
                    "deadarrayopselimination", "DeadArrayOpsElimination", false,
                    false)

ModulePass *llvm::createDeadArrayOpsEliminationLegacyPass(void) {
  return new DeadArrayOpsEliminationLegacyPass();
}

DeadArrayOpsEliminationPass::DeadArrayOpsEliminationPass(void) {}

PreservedAnalyses DeadArrayOpsEliminationPass::run(Module &M,
                                                   ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  DeadArrayUseType GetAUse = [&FAM](Function &F) -> ArrayUse & {
    return FAM.getResult<ArrayUseAnalysis>(F);
  };

  DeadArrayOpsElimImpl DeadArrayElim(M, WPInfo, GetAUse);
  if (!DeadArrayElim.run())
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

#endif // INTEL_FEATURE_SW_ADVANCED
