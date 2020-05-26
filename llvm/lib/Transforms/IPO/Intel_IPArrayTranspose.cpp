//===----  Intel_IPArrayTranspose.cpp - Intel IPO Array Transpose  --------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file implements "Array Transpose" at LTO to improve cache locality.
// Only dynamically allocated huge arrays are considered as candidates. Tracks
// all uses of return pointers of candidate mallocs to collect all memory
// references of the array. Number of rows and columns are determined by
// analyzing all memory references.
//
//  double srcA[SIZE_Z*SIZE_Y*SIZE_X*N_FIELDS]*;
//  ...
//  size_t inc = 2*SIZE_X*SIZE_Y*N_FIELDS;
//  size_t size = sizeof(SIZE_Z*SIZE_Y*SIZE_X*N_FIELDS) + 2 * inc;
//  ptr = malloc((size + 2 * inc) * sizeof(double));
//  if (ptr == nullptr) {
//  }
//  ptr = ptr + inc;
//  ...
//  ...
//
//  ptr = ptr - inc;
//  free(ptr);
//
//  Array access before transformation for ptr[x, y, z, e]:
//     ptr + (e + N_FIELDS*((x)+ (y)*SIZE_X+(z)*SIZE_X*SIZE_Y))
//
//  Array access after transformation for ptr[x, y, z, e]:
//    ((e)*SIZE_X*SIZE_Y*SIZE_Z+((x)+ (y)*SIZE_X+(z)*SIZE_X*SIZE_Y))
//
// N_FIELDS will be determined by analyzing memory references of the array
// and SIZE_X*SIZE_Y*SIZE_Z will be computed from N_FIELDS and the size of
// memory allocation.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_IPArrayTranspose.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

using namespace llvm;

#define DEBUG_TYPE "iparraytranspose"

static cl::opt<bool>
    IPArrayTransposeHeuristic("ip-array-transpose-heuristic", cl::init(true),
                              cl::Hidden,
                              cl::desc("Enable IP Array Transpose Heuristic."));

namespace {

// Main class to implement "Array Transpose".
class ArrayTransposeImpl {
public:
  ArrayTransposeImpl(
      Module &M, WholeProgramInfo &WPInfo,
      std::function<const TargetLibraryInfo &(Function &)> GetTLI,
      std::function<LoopInfo &(Function &)> GetLI,
      std::function<ScalarEvolution &(Function &)> GetSE, const DataLayout &DL)
      : M(M), WPInfo(WPInfo), GetTLI(GetTLI), GetLI(GetLI), GetSE(GetSE),
        DL(DL){};

  bool run();

private:
  using MemRefSet = SmallPtrSet<Instruction *, 32>;

  // Heuristics for the transpose transformation:

  // Maximum allowed malloc calls
  constexpr static int MaxMallocCallCountLimit = 2;

  // Minimum allocated size to consider a malloc call as candidate
  constexpr static int MinMallocSizeLimit = 0xC000000;

  // Minimum number of fields to trigger the transpose transformation.
  constexpr static int MinNumberElements = 8;

  // Minimum limit for cache reuse.
  constexpr static int64_t MinCacheReuse = 1000000000;

  // Minimum limit of gain factor with transpose.
  constexpr static int64_t MinGainFactor = 7;

  // Cache line size.
  constexpr static int64_t CacheLineSize = 64;

  // Default trip count will be used when we can't determine the trip count
  // at compile-time.
  constexpr static unsigned DefaultTripCount = 100;

  Module &M;
  WholeProgramInfo &WPInfo;
  std::function<const TargetLibraryInfo &(Function &)> GetTLI;
  std::function<LoopInfo &(Function &)> GetLI;
  std::function<ScalarEvolution &(Function &)> GetSE;
  const DataLayout &DL;

  // Allocated size of array using malloc. Since pointers from different
  // malloc calls can be swapped in the middle of the program, we don't
  // allow candidates array with different sizes. That means, all candidate
  // arrays have same size.
  int64_t MallocSize = -1U;

  // This is minimum stride value of all memory references of candidate
  // arrays.
  int64_t Stride = -1U;

  // This is maximum element size of all memory references of candidate
  // arrays.
  int64_t MaxElemSize = 1;

  // Routine that has all malloc and free calls of candidate arrays.
  Function *MainRtn = nullptr;

  // List of malloc calls
  SmallVector<CallInst *, 2> MallocCalls;

  // List of free calls
  SmallPtrSet<CallInst *, 2> FreeCalls;

  // Collection of all aliases of return pointers of malloc calls after
  // incrementing by some value. Map of alias pointer and the displacement
  // from the original return pointer of malloc.
  // Ex:
  //     ptr = malloc();  // Displacement of ptr is 0
  //     ptr1 += 128;     // Displacement of ptr1 is 128
  //
  DenseMap<Value *, int64_t> MallocPtrIncrAliases;

  // Processed instructions during the collection of memory references.
  SmallPtrSet<Instruction *, 16> ProcessedInsts;

  // All memory references of candidate arrays for each function.
  DenseMap<Function *, MemRefSet> FunctionMemRefs;

  // Set of pointer increment and decrement instructions.
  SmallPtrSet<GetElementPtrInst *, 4> PtrIncDecGEPInsts;

  // Cache reuse of original memory layout
  double OriginalReuse = 0.0;

  // Cache reuse of transposed memory layout
  double TransposedReuse = 0.0;

  // Number of rows in transposed array.
  int64_t TransposedNumRows = 0;

  // Number of columns in transposed array.
  int64_t TransposedNumCols = 0;

  bool collectMallocCalls();
  bool isKmpcLibCall(Function *, const TargetLibraryInfo *, LibFunc);
  bool computePointerAliases();
  bool collectAllMemRefs();
  bool validateAllMemRefs();
  bool isTransposeProfitable(void);
  void transformMemRefs(void);
  int64_t computeTransposedOffset(int64_t Idx);
};

// Returns true if F is "LibF" library function.
bool ArrayTransposeImpl::isKmpcLibCall(Function *F,
                                       const TargetLibraryInfo *TLI,
                                       LibFunc LibF) {
  LibFunc LF;
  if (!F || !TLI->getLibFunc(*F, LF) || !TLI->has(LF) || LibF != LF)
    return false;
  return true;
}

// Collects all candidate malloc calls in the entire module.
bool ArrayTransposeImpl::collectMallocCalls(void) {

  // Returns true if "CInst" is a valid candidate array.
  auto IsCandidateMalloc = [this](CallInst *CInst, const TargetLibraryInfo *TLI,
                                  LoopInfo *LI, Function &F) {
    if (!isMallocLikeFn(CInst, TLI))
      return false;
    auto CI = dyn_cast<ConstantInt>(CInst->getArgOperand(0));
    if (!CI)
      return false;
    int64_t MSize = CI->getSExtValue();
    if (MSize < 0 || MSize < MinMallocSizeLimit)
      return false;
    // Set MallocSize when first candidate malloc call is found. Makes sure
    // all candidate malloc calls have same allocation size.
    if (MallocSize == -1U)
      MallocSize = MSize;
    else if (MallocSize != MSize)
      return false;
    // Make sure call is not in loop.
    if (!LI->empty() && LI->getLoopFor(CInst->getParent()))
      return false;
    if (!F.doesNotRecurse())
      return false;
    return true;
  };

  // Collect all candidate malloc calls.
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;

    auto &TLI = GetTLI(F);
    auto &LI = GetLI(F);
    unsigned MallocCurCount = MallocCalls.size();
    for (Instruction &I : instructions(&F)) {
      if (auto *CB = dyn_cast<CallInst>(&I)) {
        if (IsCandidateMalloc(CB, &TLI, &LI, F)) {
          MallocCalls.push_back(CB);
          MallocPtrIncrAliases[CB] = 0;
          continue;
        }
        if (isAllocationFn(CB, &TLI)) {
          LLVM_DEBUG(dbgs() << "    Call is not qualified as candidate: " << *CB
                            << "\n");
          return false;
        }
      }
    }
    if (MallocCalls.size() != MallocCurCount) {
      // Makes sure all candidate malloc calls are in the same routine.
      if (MallocCurCount == 0) {
        MainRtn = &F;
      } else {
        LLVM_DEBUG(dbgs() << "    All candidate malloc calls are not in "
                          << MainRtn->getName() << "\n");
        return false;
      }
    }
  }
  int32_t NumCalls = MallocCalls.size();
  if (NumCalls == 0 || NumCalls > MaxMallocCallCountLimit) {
    LLVM_DEBUG(dbgs() << "    Number of malloc calls not in range\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "        Candidate malloc calls: \n";
    for (auto *CI : MallocCalls)
      dbgs() << "            " << *CI << "\n";
    dbgs() << "\n";
  });

  return true;
}

// Collect aliases of malloc return pointers. A pointer value is considered
// as an alias if there is a constant increment or decrement from the original
// malloc return pointer. For each pointer alias, it keeps track of the
// difference between the pointer and the original malloc return value.
//
// Ex:
//  %78 = call noalias malloc()     ; offset 0
//  %79 = ptrtoint i8* %78 to i64   ; offset 0
//  %80 = icmp eq i8* %78, null
//  %84 = getelementptr inbounds i8, i8* %78, i64 3200000  ; offset 3200000
//  %120 = bitcast i8*i %84 to double*                     ; offset 3200000
//  %121 = getelementptr double, double* %120, i64 -400000 ; offset 0
//  %122 = bitcast double* %123 to i8*                     ; offset 0
//  free(i8* %122)
//
//  1. BitCastInst, PtrToIntInst and IntToPtrInst
//  2. PHINodeInst (later we will prove that all operands of PHI are in
//     pointer alias set with same displacement)
//  3. GetElementPtr with all zero indices and GetElementPtr with single
//  constant index value will be considered as aliases.
//  4. Argument of a function is considered as alias if pointer alais is passed
//  as argument at the call-sites of the function either directly or indirectly.
//
// Return false if "Val" is used by any other instruction.
//
bool ArrayTransposeImpl::computePointerAliases() {

  auto ProcessSingleIndexGEP = [this](GetElementPtrInst *GEPI,
                                      int64_t &Offset) {
    if (!GEPI || GEPI->getNumIndices() != 1)
      return false;
    auto CInt = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CInt)
      return false;
    Type *ElemType = GEPI->getResultElementType();
    if (!ElemType->isSized())
      return false;
    int64_t IdxSize = DL.getTypeSizeInBits(ElemType);
    Offset = CInt->getSExtValue() * IdxSize / 8;
    return true;
  };

  // Returns true if "Ptr" is used by only calls. Uses of "Ptr" are ignored
  // in "SI" and "lifetime_start / lifetime_end / DbgInfo" intrinsics. All
  // uses of "Ptr" are collected in "Calls".
  auto IsPtrUsedByOnlyCalls = [](Value *Ptr, StoreInst *SI,
                                 SmallPtrSetImpl<CallInst *> &Calls) {
    for (auto *UI : Ptr->users()) {
      if (SI == UI)
        continue;
      if (auto CB = dyn_cast<CallInst>(UI)) {
        Calls.insert(CB);
      } else if (isa<BitCastInst>(UI)) {
        if (!UI->hasOneUse())
          return false;
        auto *II = dyn_cast<IntrinsicInst>(*UI->user_begin());
        if (!II || (II->getIntrinsicID() != Intrinsic::lifetime_start &&
                    II->getIntrinsicID() != Intrinsic::lifetime_end &&
                    !isa<DbgInfoIntrinsic>(II)))
          return false;
      } else {
        return false;
      }
    }
    return true;
  };

  // If "PtrOp" is passed as argument to "CI", collect corresponding argument
  // of Callee of "CI" in "Args".
  auto CollectAliasArguments = [this](CallInst *CI, Value *PtrOp,
                                      SmallPtrSetImpl<Argument *> &Args) {
    Value *CVal = CI->getCalledOperand()->stripPointerCasts();
    Function *CalledF = dyn_cast<Function>(CVal);
    if (!CalledF)
      return false;
    auto *I = CI->arg_begin();
    int32_t ArgPos = 0;
    auto &TLI = GetTLI(*CI->getFunction());

    // __kmpc_fork_call call is generated by compiler. 3rd argument is the
    // actual micro task function that is called at runtime. The actual
    // arguments for the target function starts from 4th argument. So,
    // 4th (%A4), 5th (%A5) and 6th(%A6) are the actual arguments "foo".
    //   call void (@__kmpc_fork_call(ident_t* @loc, i32 argc, @foo,
    //              double** %A4, i64 %A5, i64 %A6)
    //
    // Argument 1 and 2 are added by compiler. Actual arguments are mapped
    // starting from 3rd argument. So, %P3 will be mapped to %A4, %P4 will
    // be mapped to %A5 and %P5 will be mapped to %A6.
    //   foo(i32* %0, i32* %1, double** %P3, i64 %P4, i64 %P5)
    //
    if (isKmpcLibCall(CalledF, &TLI, LibFunc_kmpc_fork_call)) {
      CalledF = dyn_cast<Function>(CI->getArgOperand(2)->stripPointerCasts());
      assert(CalledF && "Expected function argument");
      I += 3;
      ArgPos = 2;
    }
    if (CalledF->isDeclaration())
      return false;
    auto *E = CI->arg_end();
    for (; I != E; I++, ArgPos++) {
      if (PtrOp != *I)
        continue;
      Args.insert(CalledF->getArg(ArgPos));
    }
    return Args.size() != 0;
  };

  // Handles only BitCastInst, PtrToIntInst, IntToPtrInst, CallInst,
  // StoreInst, ICmpInst, PHI, free call and GetElementPtrInst.
  // Return false if "Val" is used by any other instruction.
  auto CollectPtrAliases = [this, &IsPtrUsedByOnlyCalls, &CollectAliasArguments,
                            &ProcessSingleIndexGEP](Value *Val) {
    SmallPtrSet<Value *, 32> Visited;
    SmallVector<Value *, 32> WorkList;

    WorkList.push_back(Val);
    while (!WorkList.empty()) {
      Value *V = WorkList.pop_back_val();
      for (auto &UI : V->uses()) {
        Value *Ptr = UI.getUser();
        if (isa<ICmpInst>(Ptr))
          continue;
        if (auto CI = dyn_cast<CallInst>(Ptr)) {
          if (isFreeCall(CI, &GetTLI(*CI->getFunction()))) {
            FreeCalls.insert(CI);
            continue;
          }
          // Handle cases like
          //   foo(.., Ptr, ..)
          SmallPtrSet<Argument *, 16> Args;
          if (!CollectAliasArguments(CI, V, Args))
            return false;
          ProcessedInsts.insert(CI);
          for (auto A : Args) {
            if (Visited.insert(A).second) {
              WorkList.push_back(A);
              MallocPtrIncrAliases[A] = MallocPtrIncrAliases[V];
            }
          }
        } else if (auto *SI = dyn_cast<StoreInst>(Ptr)) {
          // Handle cases like
          //
          // %PtrOp = alloca double*
          // store %Ptr, %PtrOp
          // foo(..,%PtrOp, ...);
          // ...
          // __kmpc_fork_call(..., bar, %PtrOp, ...)
          // ...
          Value *PtrOp = SI->getPointerOperand();
          if (PtrOp == V)
            return false;
          if (!isa<AllocaInst>(PtrOp))
            return false;
          SmallPtrSet<CallInst *, 4> Calls;
          if (!IsPtrUsedByOnlyCalls(PtrOp, SI, Calls))
            return false;
          ProcessedInsts.insert(SI);
          for (auto CB : Calls) {
            SmallPtrSet<Argument *, 16> Args;
            if (!CollectAliasArguments(CB, PtrOp, Args))
              return false;
            ProcessedInsts.insert(CB);
            for (auto A : Args) {
              for (auto *UU : A->users()) {
                // Makes sure the arguments are only used to load pointer
                // value.
                auto *LD = dyn_cast<LoadInst>(UU);
                if (!LD)
                  return false;
                if (Visited.insert(UU).second) {
                  WorkList.push_back(UU);
                  MallocPtrIncrAliases[UU] = MallocPtrIncrAliases[V];
                }
              }
            }
          }
        } else if (auto *GEPI = dyn_cast<GetElementPtrInst>(Ptr)) {
          int64_t Offset;
          if (GEPI->hasAllZeroIndices() && Visited.insert(Ptr).second) {
            WorkList.push_back(Ptr);
            MallocPtrIncrAliases[Ptr] = MallocPtrIncrAliases[V];
          } else if (ProcessSingleIndexGEP(GEPI, Offset)) {
            WorkList.push_back(Ptr);
            MallocPtrIncrAliases[Ptr] = MallocPtrIncrAliases[V] + Offset;
            PtrIncDecGEPInsts.insert(GEPI);
          }
        } else if (isa<PHINode>(Ptr) || isa<BitCastInst>(Ptr) ||
                   isa<PtrToIntInst>(Ptr) || isa<IntToPtrInst>(Ptr)) {
          if (Visited.insert(Ptr).second) {
            WorkList.push_back(Ptr);
            MallocPtrIncrAliases[Ptr] = MallocPtrIncrAliases[V];
          }
        } else {
          return false;
        }
      }
    }
    return true;
  };

  // If "U" is a BitCast operator and is not an Instruction, collect all uses of
  // the BitCast in "ISet". If "U" is an Instruction, only "U" is considered
  // as use.
  // Ex:
  //   foo(..., bitcast , ...)
  //
  auto GetUseInstruction = [](User *U, SmallPtrSetImpl<Instruction *> &ISet) {
    if (auto II = dyn_cast<Instruction>(U)) {
      ISet.insert(II);
      return true;
    }
    auto BC = dyn_cast<BitCastOperator>(U);
    if (!BC)
      return false;
    for (auto UU : BC->users()) {
      auto II = dyn_cast<Instruction>(UU);
      if (!II)
        return false;
      ISet.insert(II);
    }
    return true;
  };

  // Collect all aliases of MallocCalls.
  for (auto CI : MallocCalls)
    if (!CollectPtrAliases(CI))
      return false;

  // Ensure that all operands of PHI nodes are also in "MallocPtrIncrAliases"
  for (auto Pair : MallocPtrIncrAliases) {
    auto *PHI = dyn_cast<PHINode>(Pair.first);
    if (!PHI)
      continue;
    int64_t Offset = Pair.second;
    for (Value *PI : PHI->incoming_values()) {
      auto It = MallocPtrIncrAliases.find(PI);
      if (It == MallocPtrIncrAliases.end() || It->second != Offset) {
        LLVM_DEBUG(dbgs() << "    Invalid pointer aliases\n");
        return false;
      }
    }
  }

  // An argument of a function is considered as pointer alias of a candidate
  // array only if pointer alias of the candidate array is passed at all
  // call-sites.
  for (auto PI : ProcessedInsts) {
    auto CI = dyn_cast<CallInst>(PI);
    if (!CI || isFreeCall(CI, &GetTLI(*CI->getFunction())))
      continue;
    Function *Callee = CI->getCalledFunction();
    assert(Callee && "Unexpected indirect call");
    for (auto CUse : Callee->users()) {
      SmallPtrSet<Instruction *, 4> ISet;
      if (!GetUseInstruction(CUse, ISet))
        return false;
      for (auto II : ISet) {
        if (!ProcessedInsts.count(II)) {
          LLVM_DEBUG(dbgs() << "    Unable to process some calls \n");
          return false;
        }
      }
    }
  }

  LLVM_DEBUG({
    dbgs() << "        All collected pointer aliases: \n";
    for (auto Pair : MallocPtrIncrAliases) {
      dbgs() << "    Pointer: " << *Pair.first << "  Offset: " << Pair.second
             << "\n";
    }
    dbgs() << "\n";
  });
  return true;
}

// Collect all memory references of candidate arrays.
// Return false if it is unable to collect.
bool ArrayTransposeImpl::collectAllMemRefs() {

  // Collect all uses "V". Only the following instructions are allowed
  // as uses:
  // 1. Load / Store: Added these instructions to "FunctionMemRefs". For Store,
  // makes sure the pointer is not esacped. Later, we will verify that base
  // pointer of pointer operand of store/load instruction is in
  // MallocPtrIncrAliases set.
  // 2. free() call
  // 3. BitCast/GetElementPtr: We will allow all GEPs here but will verify
  // later that base pointer is in MallocPtrIncrAliases set.
  // 4. ICmp Inst.
  //
  // Return false if any other instruction is noticed as user of "V".
  //
  auto CollectPtrMemRefs = [this](Value *V) {
    SmallPtrSet<Value *, 32> Visited;
    SmallVector<Value *, 32> WorkList;

    WorkList.push_back(V);
    while (!WorkList.empty()) {
      Value *Val = WorkList.pop_back_val();
      for (auto &UI : Val->uses()) {
        Value *Ptr = UI.getUser();
        if (MallocPtrIncrAliases.find(Ptr) != MallocPtrIncrAliases.end())
          continue;
        auto I = dyn_cast<Instruction>(Ptr);
        if (!I)
          return false;
        if (ProcessedInsts.count(I))
          continue;
        switch (I->getOpcode()) {
        case Instruction::ICmp:
          break;

        case Instruction::Call: {
          auto CI = cast<CallInst>(I);
          if (!isFreeCall(CI, &GetTLI(*CI->getFunction())))
            return false;
          break;
        }

        case Instruction::Load:
          FunctionMemRefs[I->getFunction()].insert(I);
          break;

        case Instruction::Store: {
          auto SI = cast<StoreInst>(I);
          Value *PtrOp = SI->getPointerOperand();
          if (PtrOp != Val)
            return false;
          FunctionMemRefs[I->getFunction()].insert(I);
          break;
        }

        case Instruction::BitCast:
        case Instruction::GetElementPtr:
          if (Visited.insert(I).second)
            WorkList.push_back(cast<Instruction>(I));
          break;

        default:
          LLVM_DEBUG(dbgs() << "    Unhandled: " << *I << "\n");
          return false;
        }
      }
    }
    return true;
  };

  // Returns true if "I" is loop or not in "MainRtn".
  auto IsInstLocOkay = [this](Instruction *I, LoopInfo *LI) {
    if (I->getFunction() != MainRtn ||
        (!LI->empty() && LI->getLoopFor(I->getParent()))) {
      LLVM_DEBUG(dbgs() << "    Inst is in unexpected routine or loop\n");
      return true;
    }
    return false;
  };

  // Returns true if 'FI' is in 'BB' and there are no instructions after 'FI'
  // in 'BB' which have side effects.
  auto IsCallInAtEndOfBlock = [this](Instruction *FI, BasicBlock *BB) {
    // Walk through "BB" in reverse order to find "FI" is at the end of
    // "BB".
    for (auto I = BB->rbegin(), E = BB->rend(); I != E; I++) {
      Instruction *II = &*I;
      if (II == FI)
        return true;
      // Ignore free calls in the "FreeCalls"
      if (auto CI = dyn_cast<CallInst>(II))
        if (FreeCalls.count(CI))
          continue;
      if (II->mayWriteToMemory())
        return false;
    }
    return false;
  };

  // Collect all memory references if possible.
  for (auto Pair : MallocPtrIncrAliases)
    if (!CollectPtrMemRefs(Pair.first))
      return false;

  if (FunctionMemRefs.size() == 0) {
    LLVM_DEBUG(dbgs() << "    No memory references found\n");
    return false;
  }

  if (FreeCalls.size() != MallocCalls.size()) {
    LLVM_DEBUG(dbgs() << "    Malloc/free calls don't match\n");
    return false;
  }

  // Makes sure all references are post-dominated by free calls. Instead of
  // using DT info, just go with simple analysis to prove the same. We will
  // check the "MainRtn" has single exit block and free calls are in the exit
  // block.
  BasicBlock *SingleReturningBlock = nullptr;
  for (BasicBlock &BB : *MainRtn)
    if (isa<ReturnInst>(BB.getTerminator())) {
      if (SingleReturningBlock) {
        LLVM_DEBUG(dbgs() << "    More than one exit block\n");
        return false;
      }
      SingleReturningBlock = &BB;
    }
  assert(SingleReturningBlock && "Expected single exit block");
  LoopInfo &LI = (GetLI)(*MainRtn);
  for (auto FC : FreeCalls) {
    if (MallocPtrIncrAliases[FC->getArgOperand(0)] != 0) {
      LLVM_DEBUG(dbgs() << "    free argument is not valid\n");
      return false;
    }
    // Makes sure free calls are in the same routine that has malloc calls.
    if (IsInstLocOkay(FC, &LI))
      return false;
    // Makes sure free call is at the end of the exit routine.
    if (!IsCallInAtEndOfBlock(FC, SingleReturningBlock))
      return false;
  }
  // Check pointer increment/decrement are in the same routine that has malloc
  // calls.
  for (auto GEP : PtrIncDecGEPInsts)
    if (IsInstLocOkay(GEP, &LI))
      return false;

  LLVM_DEBUG({
    dbgs() << "    All collected pointer aliases: \n";
    for (auto Pair : FunctionMemRefs) {
      dbgs() << "            MemRefs in " << Pair.first->getName() << "\n";
      for (auto II : Pair.second)
        dbgs() << "            " << *II << "\n";
    }
    dbgs() << "\n";
  });
  return true;
}

// Check all memory references for legality issues using SCEV.
// TODO: Will add more code in next change-set.
bool ArrayTransposeImpl::validateAllMemRefs() { return true; }

// Returns true if transpose is profitable based on cache reuse.
// TODO: Will add more code in next change-set.
bool ArrayTransposeImpl::isTransposeProfitable(void) { return false; }

// Compute transposed offset for the given original "Idx" offset.
int64_t ArrayTransposeImpl::computeTransposedOffset(int64_t Idx) {
  int64_t NRows = Idx / TransposedNumCols;
  int64_t NCols = Idx % TransposedNumCols;
  return (NRows + NCols * TransposedNumRows);
}

// Apply transformations to all memory references.
// TODO: Will add more code in next change-set.
void ArrayTransposeImpl::transformMemRefs(void) {}

bool ArrayTransposeImpl::run(void) {

  LLVM_DEBUG(dbgs() << "  IP Array Transpose: Started\n");
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(dbgs() << "    Failed: Whole Program or target\n");
    return false;
  }

  if (!collectMallocCalls()) {
    LLVM_DEBUG(dbgs() << "    Failed: No Candidate mallocs found \n");
    return false;
  }
  if (!computePointerAliases()) {
    LLVM_DEBUG(dbgs() << "    Failed: Unable to collect pointer aliases\n");
    return false;
  }
  if (!collectAllMemRefs()) {
    LLVM_DEBUG(dbgs() << "    Failed: Unable to collect MemRefs\n");
    return false;
  }
  if (!validateAllMemRefs()) {
    LLVM_DEBUG(dbgs() << "    Failed: MemRefs validation failed\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "    Passed: All validations \n");
  if (!isTransposeProfitable()) {
    LLVM_DEBUG(dbgs() << "    Transpose not profitable\n");
    return false;
  }
  transformMemRefs();
  LLVM_DEBUG(dbgs() << "  IP Array Transpose: Done\n");
  return true;
}

// Legacy pass manager implementation
class IPArrayTransposeLegacyPass : public ModulePass {
public:
  static char ID;
  IPArrayTransposeLegacyPass() : ModulePass(ID) {
    initializeIPArrayTransposeLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    auto GetTLI = [this](Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    auto GetLI = [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };
    auto GetSE = [this](Function &F) -> ScalarEvolution & {
      return this->getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();
    };

    ArrayTransposeImpl Impl(M, WPInfo, GetTLI, GetLI, GetSE, M.getDataLayout());
    return Impl.run();
  }
};

} // End anonymous namespace

char IPArrayTransposeLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(IPArrayTransposeLegacyPass, "iparraytranspose",
                      "LTO Array Transpose", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(IPArrayTransposeLegacyPass, "iparraytranspose",
                    "LTO Array Transpose", false, false)

namespace llvm {

ModulePass *createIPArrayTransposeLegacyPass() {
  return new IPArrayTransposeLegacyPass();
}

PreservedAnalyses IPArrayTransposePass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> const TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  auto GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  auto GetSE = [&FAM](Function &F) -> ScalarEvolution & {
    return FAM.getResult<ScalarEvolutionAnalysis>(F);
  };

  ArrayTransposeImpl Impl(M, WPInfo, GetTLI, GetLI, GetSE, M.getDataLayout());
  bool Changed = Impl.run();
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // End namespace llvm
