//===----  Intel_IPArrayTranspose.cpp - Intel IPO Array Transpose  --------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
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

// If all MemRefs are in a single routine, it is better to apply the transpose
// transformation in HIR. This option helps to control whether to enable this
// transformation or not if all MemRefs are in a single routine. By default, it
// is disabled so that HIR will apply array transpose if all MemRefs are in a
// single routine.
static cl::opt<bool> IPArrayAllRefsInSingleRoutine(
    "ip-array-all-memrefs-in-single-routine", cl::init(false), cl::Hidden,
    cl::desc("Enable IP Array Transpose even if all MemRefs are in a single "
             "routine."));

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

  // Set when __kmpc_fork_call call is processed. This is used as a
  // heuristic to insert kmp_set_blocktime(0) in "main".
  bool KmpLibCallSeen = false;

  bool collectMallocCalls();
  bool isKmpcLibCall(Function *, const TargetLibraryInfo *, LibFunc);
  bool computePointerAliases();
  bool collectAllMemRefs();
  bool validateAllMemRefs();
  bool isTransposeProfitable(void);
  void transformMemRefs(void);
  CallInst *insertKmpSetBlocktimeCall();
  int64_t computeTransposedOffset(int64_t Idx);
  bool checkConstantMulExpr(const SCEV *, int64_t &, const SCEV *&);
  bool parseSCEVSignZeroExtExpr(const SCEV *, int64_t &, const SCEV *&);
  bool parseSCEVExprs(const SCEV *, SmallVectorImpl<int64_t> &,
                      SmallVectorImpl<const Loop *> &, SmallSet<int64_t, 4> &,
                      SmallSet<int64_t, 2> &, SmallSet<int64_t, 1> &,
                      const SCEV *, ScalarEvolution &);
  bool parseUnoptimizedSCEVExprs(const SCEV *, SmallVectorImpl<int64_t> &,
                                 SmallVectorImpl<const Loop *> &,
                                 SmallSet<int64_t, 4> &, SmallSet<int64_t, 2> &,
                                 SmallSet<int64_t, 1> &, const SCEV *,
                                 ScalarEvolution &);
  bool parseAddRecSCEVExprs(const SCEV *, SmallVectorImpl<int64_t> &,
                            SmallVectorImpl<const Loop *> &,
                            SmallSet<int64_t, 4> &, SmallSet<int64_t, 2> &,
                            SmallSet<int64_t, 1> &, const SCEV *,
                            ScalarEvolution &);
  const SCEV *fixSCEVConst(int64_t, const SCEV *, ScalarEvolution &SE);
  const SCEV *fixSCEVAddRecExpr(const SCEV *, const SCEV *, ScalarEvolution &);
  const SCEV *fixSCEVAddExpr(const SCEV *, const SCEV *, ScalarEvolution &);
  const SCEV *fixSCEVMulExpr(const SCEV *S, ScalarEvolution &SE);
  const SCEV *fixSCEVMulSignZeroExpr(const SCEV *, const SCEV *,
                                     ScalarEvolution &SE);
  const SCEV *fixSCEVExpr(const SCEV *, const SCEV *, ScalarEvolution &);
  const SCEV *fixUnoptimizedSCEVExpr(const SCEV *, const SCEV *,
                                     ScalarEvolution &);
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

    auto &LI = GetLI(F);
    auto &TLI = GetTLI(F);
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

  // Returns true if "Ptr" is used by only calls and loads. Uses of "Ptr" are
  // ignored in "SI" and "lifetime_start / lifetime_end / DbgInfo" intrinsics.
  // All call users of "Ptr" are collected in "Calls" and load users of "Ptr"
  // are collected in "Loads".
  auto IsPtrUsedByOnlyCalls = [](Value *Ptr, StoreInst *SI,
                                 SmallPtrSetImpl<CallInst *> &Calls,
                                 SmallPtrSetImpl<LoadInst *> &Loads) {
    for (auto *UI : Ptr->users()) {
      if (SI == UI)
        continue;
      if (auto Ld = dyn_cast<LoadInst>(UI)) {
        Loads.insert(Ld);
      } else if (auto CB = dyn_cast<CallInst>(UI)) {
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
      KmpLibCallSeen = true;
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
          if (getFreedOperand(CI, &GetTLI(*CI->getFunction()))) {
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
          //
          //  or
          //
          // %PtrOp = alloca double*
          // store %Ptr, %PtrOp
          // ...
          // load double*, double** %PtrOp
          // ...
          // load double*, double** %PtrOp
          //
          Value *PtrOp = SI->getPointerOperand();
          if (PtrOp == V)
            return false;
          if (!isa<AllocaInst>(PtrOp))
            return false;
          SmallPtrSet<CallInst *, 4> Calls;
          SmallPtrSet<LoadInst *, 4> Loads;
          if (!IsPtrUsedByOnlyCalls(PtrOp, SI, Calls, Loads))
            return false;
          ProcessedInsts.insert(SI);
          if (Loads.size() > 0) {
            // Don't allow if stack address is used by both calls and loads.
            // There are no legality issues to allow both but not required for
            // now to allow both.
            if (Calls.size() > 0)
              return false;
            for (auto Ld : Loads)
              if (Visited.insert(Ld).second) {
                WorkList.push_back(Ld);
                MallocPtrIncrAliases[Ld] = MallocPtrIncrAliases[V];
              }
          }
          for (auto CB : Calls) {
            SmallPtrSet<Argument *, 16> Args;
	    if (CB->isLifetimeStartOrEnd())
              continue;
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
    if (!CI || getFreedOperand(CI, &GetTLI(*CI->getFunction())))
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
          if (!getFreedOperand(CI, &GetTLI(*CI->getFunction())))
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

// Return true if "SC" is SCEVMulExpr with only two operands and the first
// operand is constant. Constant operand is saved in "CVal" and the second
// operand is saved in "MOp".
bool ArrayTransposeImpl::checkConstantMulExpr(const SCEV *SC, int64_t &CVal,
                                              const SCEV *&MOp) {
  auto *M = dyn_cast<SCEVMulExpr>(SC);
  if (!M || M->getNumOperands() != 2)
    return false;
  auto C = dyn_cast<SCEVConstant>(M->getOperand(0));
  if (!C)
    return false;
  CVal = C->getAPInt().getSExtValue();
  MOp = M->getOperand(1);
  return true;
}

// Returns true if "SC" is in "Constant * SExt(some_expr)"  or "Constant *
// ZExt(some_expr)" format when "some_expr" is not terminal. SignZeroExtOp is
// updated with "some_expr" and CVal is updated with the constant value.
//  Ex:    (8 * (sext i32 (20 * %10)))
//         (8 * (zext i32 (20 * %10)))
bool ArrayTransposeImpl::parseSCEVSignZeroExtExpr(const SCEV *SC, int64_t &CVal,
                                                  const SCEV *&SignZeroExtOp) {
  int64_t MulVal;
  const SCEV *MulOp;
  if (!checkConstantMulExpr(SC, MulVal, MulOp))
    return false;
  if (auto SignExt = dyn_cast<SCEVSignExtendExpr>(MulOp)) {
    // Don't need to treat it as special SignExtend expression if operand
    // is terminal.
    // Ex: (8 * (sext i32 %11))
    if (isa<SCEVUnknown>(SignExt->getOperand()))
      return false;
    SignZeroExtOp = SignExt->getOperand();
  } else if (auto ZeroExt = dyn_cast<SCEVZeroExtendExpr>(MulOp)) {
    // Don't need to treat it as special ZeroExtend expression if operand
    // is terminal.
    if (isa<SCEVUnknown>(ZeroExt->getOperand()))
      return false;
    SignZeroExtOp = ZeroExt->getOperand();
  } else {
    return false;
  }
  CVal = MulVal;
  return true;
}

// This recursive function checks if "S" is in expected format and collects
// strides, loops, constant exprs and start value. This allows only AddRecExpr
// like "{Start, +, Step}". "Step" is basically stride. Only constant values
// are allowed for "Step". "Start" can be either another AddRecExpr or
// another SCEV expression with Add and Mul operations.
//
// Ex:
//   {152,+,6400000}
//   {{{152,+,6400000},+,32000},+,160}
//   {{{(320152 + (160 * %lb3) + (16000 * %lb2) + (1600000 *
//   %lb1)),+,1600000},+,16000},+,160}
bool ArrayTransposeImpl::parseAddRecSCEVExprs(
    const SCEV *S, SmallVectorImpl<int64_t> &Strides,
    SmallVectorImpl<const Loop *> &Loops, SmallSet<int64_t, 4> &AllConstExprs,
    SmallSet<int64_t, 2> &AllUnscaledMultipliers,
    SmallSet<int64_t, 1> &SignZeroExtMultipliers, const SCEV *Base,
    ScalarEvolution &SE) {

  // Returns true if "S" is in one of the below formats.
  //   1. Base
  //   2. "Constant0 + Constant1 * Some_scev_expr + Constant2 *
  //   3. "Constant0 + 8 * sext (Constant1 * Some_scev_expr) + ..."
  //
  //   Ex: (320152 + (160 * %lb3) + (16000 * %lb2) + (1600000 * %lb1))
  //
  //   This collects all constant exprs if there are any.
  auto IsStartOkay = [&](const SCEV *S, const Loop *L) {
    if (!SE.isLoopInvariant(S, L))
      return false;
    if (S == Base)
      return true;
    auto A = dyn_cast<SCEVAddExpr>(S);
    if (!A)
      return false;
    for (const SCEV *Op : A->operands()) {
      if (isa<SCEVConstant>(Op) || Op == Base)
        continue;
      int64_t MulVal;
      const SCEV *NewOp = Op;
      int64_t ScaleV = 1;
      if (parseSCEVSignZeroExtExpr(Op, MulVal, NewOp)) {
        // The constant is added to "SignZeroExtMultipliers". Later, we will
        // verify that the constant is equal to "MaxElemSize".
        SignZeroExtMultipliers.insert(MulVal);
        ScaleV = MulVal;
      }
      // Only MulExpr is allowed as operand of SignExt.
      auto *M = dyn_cast<SCEVMulExpr>(NewOp);
      if (!M)
        return false;
      auto C = dyn_cast<SCEVConstant>(M->getOperand(0));
      if (!C)
        return false;
      int64_t MVal = C->getAPInt().getSExtValue();
      AllUnscaledMultipliers.insert(MVal);
      AllConstExprs.insert(MVal * ScaleV);
    }
    return true;
  };

  // We allow only SCEVAddRecExpr expressions, which have both start and
  // step expressions. Only constant step values (stride) are allowed.
  // Start expressions can either be SCEVAddExpr or SCEVAddRecExpr.
  //  Ex: {(152 + %baseptr)<nsw>,+,160}<nsw><%b1>
  //   Step: 160
  //   Start: (152 + %baseptr)
  //
  const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S);
  if (!AR || !AR->isAffine())
    return false;
  auto Step = dyn_cast<SCEVConstant>(AR->getStepRecurrence(SE));
  if (!Step)
    return false;
  Strides.push_back(Step->getAPInt().getSExtValue());
  auto L = AR->getLoop();
  if (!L) {
    LLVM_DEBUG(dbgs() << "    Memref not in loop\n");
    return false;
  }
  Loops.push_back(L);
  const SCEV *StartS = AR->getStart();
  if (IsStartOkay(StartS, L)) {
    return true;
  }
  return parseAddRecSCEVExprs(StartS, Strides, Loops, AllConstExprs,
                              AllUnscaledMultipliers, SignZeroExtMultipliers,
                              Base, SE);
}

// Due to SExt in IR, SCEV expressions may not be optimized. Until SExt
// are eliminated by the earlier phases, we try to parse and fix
// unoptimized SCEV expressions. We mainly check for the following during
// parsing.
//   1. All stride values are constants.
//   2. Except base pointer and constant values, all other values
//      have constant multiplier that is evenly divisible by Stride.
//      Ex: (Stride * ({{((40000 * %11)),+,40000}<%16>,+,200}<%22> + %49))
//
// For now, we allow the below patterns:
// 1. Expressions with SCEVAdd/SCEVAddRec/SCEVMul operands
// 2. Only very limited use of "MaxElemSize * SExt (some_scev_expr)" pattern
// Ex:
// (24 + (8 * (sext i32 (16 + (20 * ({{(-80000 + (40000 *
// %11)),+,40000}<%16>,+,200}<%22> + %49))) to i64))<nsw> + %2)
//
// ((8 * (sext i32 {(19 + (20 * ({{(-80000 + (40000 *
// %11)),+,40000}<%16>,+,200}<%22> + %61))),+,20}<%40> to i64))<nsw> + %2)
//
// When a SCEV expression is multiplied with a constant, the value of the
// constant is propagated while collecting strides.
bool ArrayTransposeImpl::parseUnoptimizedSCEVExprs(
    const SCEV *S, SmallVectorImpl<int64_t> &Strides,
    SmallVectorImpl<const Loop *> &Loops, SmallSet<int64_t, 4> &AllConstExprs,
    SmallSet<int64_t, 2> &AllUnscaledMultipliers,
    SmallSet<int64_t, 1> &SignZeroExtMultipliers, const SCEV *Base,
    ScalarEvolution &SE) {

  std::function<bool(const SCEV *, int64_t)> ParseScaledAddRecExpr;
  std::function<bool(const SCEV *, int64_t)> ParseScaledExpr;

  // Returns true if "SC" is terminal or "SExt (terminal)" / "ZExt (terminal)".
  auto CheckTerminal = [&](const SCEV *SC) {
    // Allowing base pointer only at top level expression tree.
    if (SC == Base)
      return false;
    if (isa<SCEVUnknown>(SC))
      return true;
    auto SignExt = dyn_cast<SCEVSignExtendExpr>(SC);
    if (SignExt && isa<SCEVUnknown>(SignExt->getOperand()))
      return true;
    auto ZeroExt = dyn_cast<SCEVZeroExtendExpr>(SC);
    if (ZeroExt && isa<SCEVUnknown>(ZeroExt->getOperand()))
      return true;
    return false;
  };

  // Parse operands of "SC" SCEVAdd expression and propagate "ScaledV".
  auto ParseScaledAddExpr = [&](const SCEV *SC, int64_t ScaledV) {
    auto A = dyn_cast<SCEVAddExpr>(SC);
    assert(A && "Expected SCEVAdd Expr");
    for (const SCEV *S1 : A->operands()) {
      if (isa<SCEVConstant>(S1))
        continue;
      if (CheckTerminal(S1)) {
        AllConstExprs.insert(ScaledV);
        continue;
      }
      if (!ParseScaledExpr(S1, ScaledV))
        return false;
    }
    return true;
  };

  // Parse operands of "SC" SCEVAddRec expression and propagate "ScaledV".
  ParseScaledAddRecExpr = [&](const SCEV *SC, int64_t ScaledV) {
    const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(SC);
    assert(AR && "Expected SCEVAddRec Expr");
    if (!AR->isAffine())
      return false;
    auto Step = dyn_cast<SCEVConstant>(AR->getStepRecurrence(SE));
    if (!Step)
      return false;
    // Scaled stride value is collected.
    Strides.push_back(Step->getAPInt().getSExtValue() * ScaledV);
    auto L = AR->getLoop();
    if (!L)
      return false;
    Loops.push_back(L);
    const SCEV *StartS = AR->getStart();
    // Check if StartS is loop invariant.
    if (!SE.isLoopInvariant(StartS, L))
      return false;
    return ParseScaledExpr(StartS, ScaledV);
  };

  // Parse SCEV expressions recursively. Only SCEVAdd/SCEVAddRec/SCEVMul
  // are allowed.
  ParseScaledExpr = [&](const SCEV *SC, int64_t ScaledV) {
    int64_t CVal;
    const SCEV *MulOp;
    if (checkConstantMulExpr(SC, CVal, MulOp)) {
      int64_t NewScaledV = CVal * ScaledV;
      // Collect unscaled constant multiplier and propagate scaled
      // constant value.
      AllConstExprs.insert(NewScaledV);
      AllUnscaledMultipliers.insert(CVal);
      if (CheckTerminal(MulOp))
        return true;
      return ParseScaledExpr(MulOp, NewScaledV);
    }
    if (isa<SCEVAddExpr>(SC))
      return ParseScaledAddExpr(SC, ScaledV);
    else if (isa<SCEVAddRecExpr>(SC))
      return ParseScaledAddRecExpr(SC, ScaledV);
    return false;
  };

  auto A = dyn_cast<SCEVAddExpr>(S);
  if (!A)
    return false;
  int64_t CVal;
  const SCEV *SignZeroExtOp;
  for (const SCEV *S1 : A->operands()) {
    // Allow only base pointer and constants here.
    if (S1 == Base || isa<SCEVConstant>(S1))
      continue;
    // Allow "MaxElemSize * SExt (some_expr)" only at the top level
    // of SCEV expression tree.
    if (parseSCEVSignZeroExtExpr(S1, CVal, SignZeroExtOp)) {
      SignZeroExtMultipliers.insert(CVal);
      return ParseScaledExpr(SignZeroExtOp, CVal);
    }

    return ParseScaledExpr(S1, 1);
  }
  return true;
}

bool ArrayTransposeImpl::parseSCEVExprs(
    const SCEV *S, SmallVectorImpl<int64_t> &Strides,
    SmallVectorImpl<const Loop *> &Loops, SmallSet<int64_t, 4> &AllConstExprs,
    SmallSet<int64_t, 2> &AllUnscaledMultipliers,
    SmallSet<int64_t, 1> &SignZeroExtMultipliers, const SCEV *Base,
    ScalarEvolution &SE) {
  if (isa<SCEVAddRecExpr>(S))
    return parseAddRecSCEVExprs(S, Strides, Loops, AllConstExprs,
                                AllUnscaledMultipliers, SignZeroExtMultipliers,
                                Base, SE);

  return parseUnoptimizedSCEVExprs(S, Strides, Loops, AllConstExprs,
                                   AllUnscaledMultipliers,
                                   SignZeroExtMultipliers, Base, SE);
}

// Check all memory references for legality issues using SCEV.
bool ArrayTransposeImpl::validateAllMemRefs() {

  // Returns size of given Load / Store instruction "Inst".
  auto GetElementSize = [this](Instruction *Inst) {
    Type *Ty = nullptr;
    if (auto *Store = dyn_cast<StoreInst>(Inst))
      Ty = Store->getValueOperand()->getType();
    else if (auto *Load = dyn_cast<LoadInst>(Inst))
      Ty = Load->getType();
    assert(Ty && "Expected LoadInst or StoreInst");
    return DL.getTypeSizeInBits(Ty) >> 3;
  };

  // Returns estimated trip count for "L" using "SE".
  auto GetTripCount = [](const Loop *L, ScalarEvolution &SE) {
    if (unsigned TC = SE.getSmallConstantTripCount(L))
      return TC;
    if (unsigned TC = SE.getSmallConstantMaxTripCount(L))
      return TC;
    return DefaultTripCount;
  };

  int64_t TripCount = 1;
  // Possible stride values for all MemRefs.
  // Ex: Possible strides for the below MemRef are 160, 16000 and 1600000.
  // {{{(320152 + %p1),+,1600000}<%b1>,+,16000}<%b2>,+,160}<%b3>
  SmallSet<int64_t, 4> AllStrides;

  // Possible constant expression values for all MemRefs.
  // Ex: Possible constant expression for the below MemRef is 160.
  // {(32 + (160 * ((100 * %lb2) + (10000 * %lb1) + %lb3)) + %p1),+,1600000}
  SmallSet<int64_t, 4> AllConstExprs;

  // Collection of all unscaled constant multipliers.
  // Ex: 20 will be collected as unscaled constant multiplier for the below
  // example and 160 ( i.e 8 times 20) will be collected in "AllConstExprs".
  // We will check all unscaled constant multipliers are evenly divisible
  // by "TransposedNumCols" to simplify the transformations.
  //
  //    (8 * (sext i32 (16 + (20 * (some_expr)))))
  //
  SmallSet<int64_t, 2> AllUnscaledMultipliers;

  // Collection of all constant multipliers of SExt expressions.
  // Ex: 8 will be collected in "SignZeroExtMultipliers" for below example.
  // We will make sure the value of the constant is equal to "MaxElemSize".
  //  (8 * (sext i32 (some_expr)))
  //
  SmallSet<int64_t, 1> SignZeroExtMultipliers;

  for (auto Pair : FunctionMemRefs) {
    Function *F = Pair.first;
    LLVM_DEBUG(dbgs() << " Processing MemRefs in " << F->getName() << ":\n");
    ScalarEvolution &SE = (GetSE)(*F);
    for (auto II : Pair.second) {
      LLVM_DEBUG(dbgs() << "  Load / Store: " << *II << "\n");
      Value *PtrOp = getLoadStorePointerOperand(II);
      assert(PtrOp && "Expected valid PtrOp");
      if (auto *BC = dyn_cast<BitCastInst>(PtrOp))
        PtrOp = BC->getOperand(0);
      if (!isa<GetElementPtrInst>(PtrOp)) {
        LLVM_DEBUG(dbgs() << "    No gep as Ptr\n");
        return false;
      }
      const SCEV *SC = SE.getSCEV(PtrOp);
      const SCEVUnknown *Base = dyn_cast<SCEVUnknown>(SE.getPointerBase(SC));
      if (!Base) {
        LLVM_DEBUG(dbgs() << "    No SCEV for base pointer in MemRef\n");
        return false;
      }
      // Make sure BasePtr is in MallocPtrIncrAliases set.
      Value *BasePtr = Base->getValue();
      if (!BasePtr ||
          (MallocPtrIncrAliases.find(BasePtr) == MallocPtrIncrAliases.end())) {
        LLVM_DEBUG(dbgs() << "    Unknown Base pointer in MemRef\n");
        return false;
      }
      SmallVector<const Loop *, 4> Loops;
      SmallVector<int64_t, 4> Strides;
      LLVM_DEBUG(dbgs() << "Processing SCEVExpr: " << *SC << "\n");
      if (!parseSCEVExprs(SC, Strides, Loops, AllConstExprs,
                          AllUnscaledMultipliers, SignZeroExtMultipliers, Base,
                          SE) ||
          Strides.size() < 1) {
        LLVM_DEBUG(dbgs() << "    Index expr is complex to process: " << *SC
                          << "\n");
        return false;
      }
      if (Loops.size() != Strides.size()) {
        LLVM_DEBUG(dbgs() << "    Strides and loops don't match\n");
        return false;
      }
      // Collect possible strides in all MemRef. Once we find the minimum Stride
      // of all MemRefs, we need to make sure all possible strides are
      // evenly divisible by the minimum stride (i.e Stride).
      for (auto S : Strides)
        AllStrides.insert(S);

      // Update MaxElemSize.
      int64_t ElementSize = GetElementSize(II);
      MaxElemSize = std::max(MaxElemSize, ElementSize);
      // Get the minimum of current Stride and the stride of the MemRef
      // in the innermost loop (i.e Strides[0]).
      //
      Stride = std::min(Stride, Strides[0]);

      TripCount = 1;
      for (auto L : Loops)
        TripCount *= GetTripCount(L, SE);
      LLVM_DEBUG(dbgs() << "    TripCount: " << TripCount << "\n");
      int64_t OriginalFactor = 1;
      if (Strides[0] < CacheLineSize)
        OriginalFactor = CacheLineSize / Strides[0];
      int64_t TransposedFactor = CacheLineSize / ElementSize;
      OriginalReuse += TripCount * OriginalFactor;
      TransposedReuse += TripCount * TransposedFactor;
    }
  }
  LLVM_DEBUG(dbgs() << "    Stride: " << Stride << "\n");
  LLVM_DEBUG(dbgs() << "    Max Elem Size: " << MaxElemSize << "\n");
  LLVM_DEBUG({
    dbgs() << "    All possible strides: \n";
    for (auto S : AllStrides)
      dbgs() << "          " << S << " ";
    dbgs() << "\n";
  });
  // Check all Strides are evenly divisible by Stride.
  for (auto S : AllStrides) {
    if (S < 0 || S % Stride != 0) {
      LLVM_DEBUG(
          dbgs() << "    Strides are not divisible by number of rows \n");
      return false;
    }
  }
  LLVM_DEBUG({
    dbgs() << "    All possible constant multipliers: \n";
    for (auto CE : AllConstExprs)
      dbgs() << "          " << CE << " ";
    dbgs() << "\n";
  });
  LLVM_DEBUG({
    dbgs() << "    All unscaled constant multipliers: \n";
    for (auto CE : AllUnscaledMultipliers)
      dbgs() << "          " << CE << " ";
    dbgs() << "\n";
  });
  LLVM_DEBUG({
    dbgs() << "    All SExt/ZExt constant multipliers: \n";
    for (auto CE : SignZeroExtMultipliers)
      dbgs() << "          " << CE << " ";
    dbgs() << "\n";
  });
  // Check that all unknown constant multipliers are evenly divisible by Stride.
  for (auto CE : AllConstExprs) {
    if (CE % Stride != 0) {
      LLVM_DEBUG(
          dbgs() << "    Constant Value is not divisible by number of rows \n");
      return false;
    }
  }
  LLVM_DEBUG(dbgs() << "    MallocSize: " << MallocSize << "\n");
  // Make sure malloc size is evenly divided by Stride.
  int64_t MallocSizeInElem = MallocSize / MaxElemSize;
  if (MallocSize % MaxElemSize != 0 || MallocSizeInElem % Stride != 0) {
    LLVM_DEBUG(dbgs() << "    Malloc-size is not divisible by Stride \n");
    return false;
  }
  // Make sure pointer increment values are evenly divisible by MaxElemSize.
  for (auto Pair : MallocPtrIncrAliases) {
    if (Pair.second < 0 || Pair.second % MaxElemSize != 0) {
      LLVM_DEBUG(dbgs() << "     Increment is not divisible by MaxElemSize \n");
      return false;
    }
  }

  TransposedNumCols = Stride / MaxElemSize;
  TransposedNumRows = MallocSizeInElem / TransposedNumCols;

  LLVM_DEBUG(dbgs() << "  TransposedNumRows: " << TransposedNumRows
                    << "  TransposedNumCols: " << TransposedNumCols << "\n");

  // Check that all unscaled constant multipliers are evenly divisible by
  // TransposedNumCols.
  for (auto CE : AllUnscaledMultipliers) {
    if (CE % TransposedNumCols != 0) {
      LLVM_DEBUG(
          dbgs()
          << "    Constant Value is not divisible by TransposedNumCols \n");
      return false;
    }
  }
  // Check that constant multiplier of SExt is equal to MaxElemSize.
  for (auto CE : SignZeroExtMultipliers) {
    if (CE != MaxElemSize) {
      LLVM_DEBUG(
          dbgs() << "    SExt/ZExt multiplier is not equal to MaxElemSize \n");
      return false;
    }
  }
  return true;
}

// Returns true if transpose is profitable based on cache reuse.
bool ArrayTransposeImpl::isTransposeProfitable(void) {
  // Return true if all memory allocations and memory references are in a
  // single routine.
  auto AllMemRefsInSingleRoutine = [this]() {
    if (FunctionMemRefs.size() != 1)
      return false;
    if ((*FunctionMemRefs.begin()).first == MainRtn)
      return true;
    return false;
  };

  if (!IPArrayTransposeHeuristic) {
    LLVM_DEBUG(dbgs() << "    Disabled heuristics \n");
    return true;
  }
  LLVM_DEBUG(dbgs() << "    OriginalReuse: " << OriginalReuse
                    << "  TransposedReuse: " << TransposedReuse << "\n");
  if (Stride / MaxElemSize < MinNumberElements)
    return false;
  if (TransposedReuse < MinCacheReuse)
    return false;
  if (TransposedReuse < MinGainFactor * OriginalReuse)
    return false;
  LLVM_DEBUG(dbgs() << "    Transpose is Profitable\n");

  if (!IPArrayAllRefsInSingleRoutine && AllMemRefsInSingleRoutine()) {
    LLVM_DEBUG(dbgs() << "    Failed: All MemRefs are in single routine\n");
    return false;
  }
  return true;
}

// Compute transposed offset for the given original "Idx" offset.
int64_t ArrayTransposeImpl::computeTransposedOffset(int64_t Idx) {
  int64_t NRows = Idx / TransposedNumCols;
  int64_t NCols = Idx % TransposedNumCols;
  return (NRows + NCols * TransposedNumRows);
}

// Returns transposed SE constant value for the "StartC".
const SCEV *ArrayTransposeImpl::fixSCEVConst(int64_t ElemOffset,
                                             const SCEV *BasePtr,
                                             ScalarEvolution &SE) {
  int32_t TotalElemOffset;
  // Get displacement of BasePtr from original malloc return pointer.
  auto Pair = MallocPtrIncrAliases.find(cast<SCEVUnknown>(BasePtr)->getValue());
  int64_t Disp = Pair->second;
  // Add the displacement to the constant offset so that BasePtr will refer
  // to original malloc return pointer. Later, we will change increment
  // values in PtrIncDecGEPInsts to zero.
  TotalElemOffset = ElemOffset + Disp / MaxElemSize;
  // Compute transposed offset.
  int64_t TransposedElemOffset = computeTransposedOffset(TotalElemOffset);
  return SE.getConstant(BasePtr->getType(), TransposedElemOffset * MaxElemSize);
}

// Return expressions like "Mul" / "TransposedNumCols"
// "Mul" is expected to be like "Constant * some_expr", which will
// be converted to "Constant / TransposedNumCols * some_expr".
//
const SCEV *ArrayTransposeImpl::fixSCEVMulExpr(const SCEV *S,
                                               ScalarEvolution &SE) {
  SmallVector<const SCEV *, 4> Ops;
  auto Mul = dyn_cast<SCEVMulExpr>(S);
  assert(Mul && "Expected MulExpr");
  const SCEVConstant *SC = dyn_cast<SCEVConstant>(Mul->getOperand(0));
  assert(SC && "Expected SCEVConstant");
  const APInt &LA = SC->getAPInt();
  Ops.push_back(SE.getConstant(LA.sdiv(TransposedNumCols)));
  for (int32_t I = 1, E = Mul->getNumOperands(); I < E; I++)
    Ops.push_back(Mul->getOperand(I));
  return SE.getMulExpr(Ops);
}

// Converts the given expression "S" from "Mul Op0, (Sext some_expr)" to
// "Mul Op0, (SExt NewSignExtOp)".
const SCEV *ArrayTransposeImpl::fixSCEVMulSignZeroExpr(const SCEV *S,
                                                       const SCEV *NewSignExtOp,
                                                       ScalarEvolution &SE) {
  SmallVector<const SCEV *, 4> MulOps;
  assert(isa<SCEVMulExpr>(S) && "Unexpected SCEV Expr");
  auto Mul = cast<SCEVMulExpr>(S);
  MulOps.push_back(Mul->getOperand(0));
  const SCEV *MulOp1 = Mul->getOperand(1);
  assert((isa<SCEVSignExtendExpr>(MulOp1) || isa<SCEVZeroExtendExpr>(MulOp1)) &&
         "Unexpected SCEV expression");
  if (isa<SCEVSignExtendExpr>(MulOp1)) {
    auto SignExt = cast<SCEVSignExtendExpr>(MulOp1);
    MulOps.push_back(SE.getSignExtendExpr(NewSignExtOp, SignExt->getType()));
  } else {
    auto ZeroExt = cast<SCEVZeroExtendExpr>(MulOp1);
    MulOps.push_back(SE.getZeroExtendExpr(NewSignExtOp, ZeroExt->getType()));
  }
  return SE.getMulExpr(MulOps);
}

// This routine returns new AddExpr by fixing the given Constant/AddExpr "S" to
// represent transposed memory access. No change is needed to the "BasePtr",
// which is in "MallocPtrIncrAliases". Only constant and constant expressions
// are fixed.
//
//  Before and after example SCEV expressions when stride is 20.
//
//  Before: (320152 + (160 * ((100 * %lb2) + (10000 * %lb1) + %lb3)) + %p1)
//  After: (203696000 + (8 * ((100 * %lb2) + (10000 * %lb1) + %lb3)) + %p1)
//
const SCEV *ArrayTransposeImpl::fixSCEVAddExpr(const SCEV *S,
                                               const SCEV *BasePtr,
                                               ScalarEvolution &SE) {
  int64_t Offset = 0;
  SmallVector<const SCEV *, 4> Ops;
  if (S == BasePtr) {
    // Since BasePtr refers to the original malloc return pointer, "S" needs to
    // be converted as "S + computeTransposedOffset(displacement_of_base_ptr)".
    Ops.push_back(S);
  } else {
    auto A = dyn_cast<SCEVAddExpr>(S);
    assert(A && "Expected AddExpr");
    for (const SCEV *S : A->operands()) {
      if (S == BasePtr) {
        Ops.push_back(S);
      } else if (auto StartC = dyn_cast<SCEVConstant>(S)) {
        // I don't think AddExpr has more than one constant but accumulate
        // offsets if it has to be on the safe side.
        Offset += StartC->getAPInt().getSExtValue();
      } else {
        int64_t MulVal;
        const SCEV *SignExtOp;
        if (!parseSCEVSignZeroExtExpr(S, MulVal, SignExtOp)) {
          Ops.push_back(fixSCEVMulExpr(S, SE));
          continue;
        }
        // Convert special SignExt or ZeroExt.
        //  Before:  (8 * sext (20 * %11))
        //  After:   (8 * sext (%11))

        // We already proved that it is in "MaxElemSize * sext (Const * expr)"
        // pattern. So, we just divide "Const" by "TransposedNumCols".
        const SCEV *NewSignExtOp = fixSCEVMulExpr(SignExtOp, SE);
        Ops.push_back(fixSCEVMulSignZeroExpr(S, NewSignExtOp, SE));
      }
    }
  }
  Ops.push_back(fixSCEVConst(Offset / MaxElemSize, BasePtr, SE));
  return SE.getAddExpr(Ops);
}

// Fix all strides and constants in "S" to make it transposed memory reference.
//
// Example SCEV expressions when stride is 20
// Before: {{{(56 + %3010),+,1600000},+,16000},+,160}
// After: {{{(75200000 + %3010),+,80000},+,800},+,8}
const SCEV *ArrayTransposeImpl::fixSCEVAddRecExpr(const SCEV *S,
                                                  const SCEV *BasePtr,
                                                  ScalarEvolution &SE) {
  const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S);
  assert(AR && AR->isAffine() && "Expected AddRecExpr");
  auto L = AR->getLoop();
  assert(L && "Expected Valid loop");
  auto Start = AR->getStart();
  if (isa<SCEVAddRecExpr>(Start))
    Start = fixSCEVAddRecExpr(Start, BasePtr, SE);
  else
    Start = fixSCEVAddExpr(Start, BasePtr, SE);
  auto StepC = cast<SCEVConstant>(AR->getStepRecurrence(SE));
  const SCEV *St =
      SE.getConstant(AR->getStepRecurrence(SE)->getType(), TransposedNumCols);
  assert(StepC && "Expected Step AddRecExpr");
  const SCEV *Step = SE.getConstant(
      StepC->getAPInt().sdiv(cast<SCEVConstant>(St)->getAPInt()));
  return SE.getAddRecExpr(Start, Step, L, AR->getNoWrapFlags(SCEV::FlagNW));
}

// This routine makes unoptimized SCEV expression "S" of a memory reference
// to the corresponding transposed array memory reference.
// Ex:
// Before:
//   (24 + (8 * (sext i32 (16 + (20 * ({{(-80000 + (40000 *
//   %11)),+,40000}<%19>,+,200}<nw><%27> + %73))) to i64))<nsw> + %2)
// After:
//  (254080000 + (8 * (sext i32 ({{(168880000 + (40000 *
//  %11)),+,40000}<%19>,+,200}<nw><%27> + %73) to i64))<nsw> + %2)
//
// Constants: SCEV expression may have multiple constant values, which may be
// processed independently. In the above example, 24 and 160 (i.e 8 * 16).
// Displacement of base pointer to the original memory allocation is added
// only once at the top level of expression tree.
//
// Other exprs: All other exprs are expected to have constant multipliers that
// are divisible by TransposedNumCols. During transformation, we just divide
// the constant multiplier by TransposedNumCols and don't process the remaining
// expression.
// Ex:
//  "(20 * %12)" will be converted to "%12".
//
//  "(20 * ({{(const + (40000 * %11)),+,40000}<%21>,+,200}<nw><%30> + %78))"
//  will be converted to
//  "({{(const + (40000 * %11)),+,40000}<%21>,+,200}<nw><%30> + %78)"
//
// Strides: Just divide stride values by TransposedNumCols. If a AddRec
// expression is multiplied by constant value, we don't need to process the
// the AddRec during the transformation because the constant multiplier is
// divided by TransposedNumCols.
//
const SCEV *ArrayTransposeImpl::fixUnoptimizedSCEVExpr(const SCEV *S,
                                                       const SCEV *BasePtr,
                                                       ScalarEvolution &SE) {

  std::function<const SCEV *(const SCEV *S, int64_t ScaledV)> FixScaledExpr;

  // Convert constant "StartC" to transposed constant.
  auto FixScaledConst = [&, this](const SCEVConstant *StartC, int64_t ScaledV) {
    auto Pair =
        MallocPtrIncrAliases.find(cast<SCEVUnknown>(BasePtr)->getValue());
    int64_t Disp = Pair->second;
    // First, get scaled value.
    int64_t Offset = StartC->getAPInt().getSExtValue() * ScaledV / MaxElemSize;
    // Since some offsets may be negative, add displacement of base pointer
    // before computing transposed constant and then subtract the transposed
    // displacement to get correct element offset in the transposed
    // array.
    int64_t TotalOffset = Offset + Disp / MaxElemSize;
    int64_t TransposedTotalOffset = computeTransposedOffset(TotalOffset);
    int64_t TransposedOffset =
        TransposedTotalOffset - computeTransposedOffset(Disp / MaxElemSize);
    // Fix scaling if there is any.
    return SE.getConstant(StartC->getType(),
                          TransposedOffset * MaxElemSize / ScaledV);
  };

  // Fix SCEVAdd expression.
  auto FixScaledAdd = [&](const SCEV *SC, int64_t ScaledV) {
    SmallVector<const SCEV *, 4> Ops;
    auto A = dyn_cast<SCEVAddExpr>(SC);
    assert(A && "Expected AddExpr");
    for (const SCEV *S : A->operands())
      if (auto StartC = dyn_cast<SCEVConstant>(S))
        Ops.push_back(FixScaledConst(StartC, ScaledV));
      else
        Ops.push_back(FixScaledExpr(S, ScaledV));
    return SE.getAddExpr(Ops);
  };

  // Fix SCEVAddRec expression.
  auto FixScaledAddRecExpr = [&, this](const SCEV *S, int64_t ScaledV) {
    const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S);
    assert(AR && AR->isAffine() && "Expected AddRecExpr");
    auto Start = AR->getStart();
    // Fix start expression.
    Start = FixScaledExpr(Start, ScaledV);
    auto L = AR->getLoop();
    assert(L && "Expected Valid loop");
    // Fix step expression: StepC / TransposedNumCols
    auto StepC = cast<SCEVConstant>(AR->getStepRecurrence(SE));
    assert(StepC && "Expected Step AddRecExpr");
    const SCEV *St = SE.getConstant(StepC->getType(), TransposedNumCols);
    const SCEV *Step = SE.getConstant(
        StepC->getAPInt().sdiv(cast<SCEVConstant>(St)->getAPInt()));
    // Create new AddRec expression.
    return SE.getAddRecExpr(Start, Step, L, AR->getNoWrapFlags(SCEV::FlagNW));
  };

  // Fix expressions recursively.
  FixScaledExpr = [&, this](const SCEV *S, int64_t ScaledV) {
    // Since we already proved that it is just constant multiplier, just
    // fix the multiplier.
    if (isa<SCEVMulExpr>(S))
      return fixSCEVMulExpr(S, SE);
    else if (isa<SCEVAddExpr>(S))
      return FixScaledAdd(S, ScaledV);
    else if (isa<SCEVAddRecExpr>(S))
      return FixScaledAddRecExpr(S, ScaledV);
    llvm_unreachable("Unexpected SCEV Type");
  };
  auto A = dyn_cast<SCEVAddExpr>(S);
  assert(A && "Expected AddExpr");
  int64_t Offset = 0;
  SmallVector<const SCEV *, 4> Ops;
  int64_t MulVal;
  const SCEV *SignExtOp;
  // Fix top level expression tree.
  for (const SCEV *S1 : A->operands())
    if (S1 == BasePtr) {
      Ops.push_back(S1);
    } else if (auto StartC = dyn_cast<SCEVConstant>(S1)) {
      Offset += StartC->getAPInt().getSExtValue();
    } else if (parseSCEVSignZeroExtExpr(S1, MulVal, SignExtOp)) {
      // Fix SExt/ZExt expressions like "8 * SExt()".
      const SCEV *NewSignZeroExtOp = FixScaledExpr(SignExtOp, MulVal);
      Ops.push_back(fixSCEVMulSignZeroExpr(S1, NewSignZeroExtOp, SE));
    } else {
      Ops.push_back(FixScaledExpr(S1, 1));
    }

  // Invoking fixSCEVConst here to add displacement of base pointer to the
  // original malloc allocation.
  Ops.push_back(fixSCEVConst(Offset / MaxElemSize, BasePtr, SE));
  return SE.getAddExpr(Ops);
}

const SCEV *ArrayTransposeImpl::fixSCEVExpr(const SCEV *S, const SCEV *BasePtr,
                                            ScalarEvolution &SE) {
  if (isa<SCEVAddRecExpr>(S))
    return fixSCEVAddRecExpr(S, BasePtr, SE);

  // Fix unoptimized SCEV expression.
  return fixUnoptimizedSCEVExpr(S, BasePtr, SE);
}

// Apply transformations to all memory references.
// SCEVExpander is used to generate transposed memory references.
void ArrayTransposeImpl::transformMemRefs(void) {
  for (auto Pair : FunctionMemRefs) {
    Function *F = Pair.first;
    LLVM_DEBUG(dbgs() << " Transforming MemRefs in " << F->getName() << ":\n");
    ScalarEvolution &SE = (GetSE)(*F);
    SCEVExpander Exp(SE, DL, "transpose");
    for (auto II : Pair.second) {
      LLVM_DEBUG(dbgs() << "MemRef before: " << *II << "\n");
      Value *PtrOp = getLoadStorePointerOperand(II);
      assert(PtrOp && "Expected valid PtrOp");
      if (auto *BC = dyn_cast<BitCastInst>(PtrOp))
        PtrOp = BC->getOperand(0);
      LLVM_DEBUG(dbgs() << "Memory Addr before: " << *PtrOp << "\n");
      const SCEV *SC = SE.getSCEV(PtrOp);
      const SCEV *Base = SE.getPointerBase(SC);
      assert(Base && "Expected valid Base");

      LLVM_DEBUG(dbgs() << "SCEV before: " << *SC << "\n");
      const SCEV *NewSC = fixSCEVExpr(SC, Base, SE);
      LLVM_DEBUG(dbgs() << "SCEV after: " << *NewSC << "\n");
      // Expand code for NewSC using Expander.
      Value *NewPtrOp = Exp.expandCodeFor(NewSC, NewSC->getType(), II);
      LLVM_DEBUG(dbgs() << "Memory Addr after: " << *NewPtrOp << "\n");
      Value *APtrOp = getLoadStorePointerOperand(II);
      if (APtrOp->getType() != NewPtrOp->getType()) {
        NewPtrOp = new BitCastInst(cast<Instruction>(NewPtrOp),
                                   APtrOp->getType(), "", II);
        LLVM_DEBUG(dbgs() << "BitCast generated: " << *NewPtrOp << "\n");
      }
      II->replaceUsesOfWith(APtrOp, NewPtrOp);
      LLVM_DEBUG(dbgs() << "MemRef after: " << *II << "\n");
    }
  }

  // Since offset is already included in memory references, reset pointer
  // increment and decrement.
  for (auto GEP : PtrIncDecGEPInsts) {
    LLVM_DEBUG(dbgs() << "Before resetting offset: " << *GEP << "\n");
    GEP->setOperand(1, ConstantInt::get(GEP->getOperand(1)->getType(), 0));
    LLVM_DEBUG(dbgs() << "After resetting offset: " << *GEP << "\n");
  }
}

// Insert kmp_set_blocktime(0) at the beginning of “main” routine
// to avoid performance variance. The following heuristics are used
// to insert the call.
//   IPArrayTranspose transformation is triggered and
//   Array pointer is passed through __kmpc_fork_call call.
//
CallInst *ArrayTransposeImpl::insertKmpSetBlocktimeCall(void) {
  if (!KmpLibCallSeen)
    return nullptr;
  assert(MainRtn && "MainRtn expected");
  auto &TLI = GetTLI(*MainRtn);
  if (!TLI.has(LibFunc_kmp_set_blocktime))
    return nullptr;
  LLVMContext &Ctx = M.getContext();
  Type *Int32Ty = Type::getInt32Ty(Ctx);
  FunctionCallee SetFunc = M.getOrInsertFunction(
      TLI.getName(LibFunc_kmp_set_blocktime), Type::getVoidTy(Ctx), Int32Ty);
  if (!SetFunc)
    return nullptr;
  auto IP = MainRtn->getEntryBlock().getFirstInsertionPt();
  Value *Params[] = {ConstantInt::get(Int32Ty, 0)};
  CallInst *CI = CallInst::Create(SetFunc, Params, "", &*IP);
  return CI;
}

bool ArrayTransposeImpl::run(void) {

  LLVM_DEBUG(dbgs() << "  IP Array Transpose: Started\n");
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
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

  CallInst *CI = insertKmpSetBlocktimeCall();
  if (CI)
    LLVM_DEBUG(dbgs() << "Created kmp_set_blocktime: " << *CI << "\n");
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
