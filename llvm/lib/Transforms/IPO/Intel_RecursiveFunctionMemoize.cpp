//===- Intel_RecursiveFunctionMemoize.cpp --------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_RecursiveFunctionMemoize.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

/**
 * This transformation converts functions implementing recursive
 * algorithm into dynamic programming ones.
 * Basic idea of transformation is to create a cache to store already
 * calculated function results and use them again if the function is
 * called with the same parameters.
 *
 * Example of the transformation in pseudo code:
 * Consider a function which calculates fibonacci numbers.
 *
 * Straight forward implementation looks like:
 *
 *    unsigned fib(unsigned n) {
 *      if (n < 2)
 *        return n;
 *      return fib(n-1) + fib(n-2);
 *    }
 *
 * Transformed pseudo code:
 *
 *    unsigned fib_cached(unsigned n, Cache& cache) {
 *      if (cache.contains(n))
 *        return cache.get(n);
 *
 *      if (n < 2)
 *        return n;
 *      unsigned res = fib_cached(n-1, cache) + fib_cached(n-2, cache);
 *      cache.put(n, res);
 *      return res;
 *    }
 *
 *    unsigned fib(unsigned n) {
 *      Cache cache;
 *      return fib_cached(n, cache);
 *    }
 *
 * Transformation description:
 * 1. Select candidates.
 *    General requirements for candidate selection:
 *    a. Function is pure
 *    b. Function has more than one self-recursive call.
 *       The motivation of this requirement is assumption
 *       that the other call will get a cached value.
 *       Nevertheless the transformation is safe if this
 *       requirement is not satisfied, since this is only
 *       about performance considerations.
 * 2. Create a "cached" function.
 *    The "cached" function is build according the following procedure.
 *    a. On "cached" function begin, add the code to check
 *       if required value is present in the cache and return it
 *       if it is true.
 *    b. Clone original function into "cached" one.
 *    c. Replace self-recursive calls in cloned part by calls to
 *       to "cached" version.
 *    d. Insert cache updates before each return in cloned part of
 *       "cached" function.
 * 3. Replace original function with
 *    a. Initialize cache
 *    b. Call "cached" function and return its result
 *
 **/

/**
 *  Current implementation details:
 *
 *  Limitations:
 *  For the simplicity of analysis, This implementation limits candidate
 *  function selection to the functions meeting the following criteria:
 *  1. Function returns integer.
 *  2. Function has single integer argument.
 *  3. Function has only self recursive calls.
 *  4. Function has more than 1 recursive call.
 *  5. Function has no memory operations.
 *
 *  Cache implementation:
 *  Cache is an array of
 *    {
 *      ArgTy    - key (function argument)
 *      ReturnTy - value (function value for the key)
 *      bool     - isEngaged (flag to check if the entry is valid)
 *    }.
 *
 *  Element selection for key
 *    1. Select element [key%cacheSize]
 *    2. if element isEngaged and element's key is equal query key
 *       return the element.
 *    3. Cache contains no requested element otherwise.
 **/

using namespace llvm;

static cl::opt<bool>
    EnableRecursiveFuncMemoization("enable-recursive-function-memoization",
                                   cl::init(true), cl::Hidden);
static cl::opt<unsigned>
    FuncMemoizationCacheSize("function-memoization-cache-size", cl::init(8),
                             cl::Hidden);
static cl::opt<unsigned>
    FuncMemoizationFuncSizeLimit("function-memoization-func-size-limit",
                                 cl::init(1000), cl::Hidden);

#define DEBUG_TYPE "recursive-function-memoize"

static bool isCandidate(const Function *F) {
  if (F->isDeclaration())
    return false;
  if (F->isVarArg())
    return false;
  if (!F->getReturnType()->isIntegerTy())
    return false;
  if (F->arg_size() != 1 || !F->getArg(0)->getType()->isIntegerTy())
    return false;

  unsigned RecursionCount = 0;
  unsigned InstCount = 0;
  for (auto &I : instructions(F)) {
    if (++InstCount > FuncMemoizationFuncSizeLimit)
      return false;
    if (const auto *CB = dyn_cast<CallBase>(&I)) {
      const auto *Callee = CB->getCalledFunction();
      if (Callee != F)
        return false;
      ++RecursionCount;
      continue;
    }
    if (I.mayReadOrWriteMemory())
      return false;
  }
  return RecursionCount > 1;
}

namespace {
class Transformer : private IRBuilder<> {
  Function *OrigFn;
  Module *M;

  unsigned short CacheSize;

  Type *CacheTy = nullptr;
  Value *CacheSizeVal = nullptr;
  Function *GetCacheIDFunc = nullptr;
  Function *GetCacheEntryPtrFunc = nullptr;
  Function *CacheUpdateFunc = nullptr;
  Function *CacheInitFunc = nullptr;
  Function *CachedFunc = nullptr;

public:
  Transformer(Function *OrigFn, unsigned CacheSize)
      : IRBuilder<>(OrigFn->getParent()->getContext()), OrigFn(OrigFn),
        M(OrigFn->getParent()), CacheSize(CacheSize) {

    assert(OrigFn->arg_size() == 1 &&
           "Current implementation of memoization works "
           "with single argument functions.");

    assert(
        OrigFn->getArg(0)->getType()->isIntegerTy() &&
        "Current implementation of memoization works with integer types only.");

    assert(
        OrigFn->getReturnType()->isIntegerTy() &&
        "Current implementation of memoization works with integer types only.");

    CacheSizeVal = getInt32(CacheSize);
  }

  Function *createGetCacheIDFunc() {
    Twine FName = Twine(OrigFn->getName()) + ".get_cache_id";
    Type *ArgTy = OrigFn->getArg(0)->getType();
    FunctionType *FTy = FunctionType::get(getInt32Ty(), {ArgTy}, false);
    Function *F = Function::Create(FTy, Function::PrivateLinkage, FName, M);
    auto *Key = F->getArg(0);
    Key->setName("key");
    SetInsertPoint(BasicBlock::Create(getContext(), "entry", F));
    getInlineReport()->addFunction(F);
    getMDInlineReport()->addFunction(F);
    auto *Key32 = CreateZExtOrTrunc(Key, getInt32Ty(), "key.32");
    auto *Idx = CreateURem(Key32, CacheSizeVal, "idx");
    CreateRet(Idx);
    return F;
  }

  Function *createGetCacheEntryPtrFunc() {
    assert(GetCacheIDFunc && "GetCacheIDFunc should be defined.");
    Twine FName = Twine(OrigFn->getName()) + ".get_cache_entry_ptr";
    Type *PtrTy = getPtrTy();
    Type *ArgTy = OrigFn->getArg(0)->getType();
    FunctionType *FTy = FunctionType::get(PtrTy, {ArgTy, PtrTy}, false);
    Function *F = Function::Create(FTy, Function::PrivateLinkage, FName, M);
    SetInsertPoint(BasicBlock::Create(getContext(), "entry", F));
    getInlineReport()->addFunction(F);
    getMDInlineReport()->addFunction(F);
    auto *Key = F->getArg(0);
    Key->setName("key");
    auto *Cache = F->getArg(1);
    Cache->setName("cache");
    auto *Idx = CreateCall(GetCacheIDFunc, {Key}, "idx");
    getInlineReport()->addCallSite(Idx);
    getMDInlineReport()->addCallSite(Idx);
    auto *Idx64 = CreateZExt(Idx, getInt64Ty(), "idx.64");
    auto *Gep = CreateGEP(CacheTy, Cache, {Idx64}, "cache.entry");
    CreateRet(Gep);
    return F;
  }

  Function *createCacheUpdateFunc() {
    assert(CacheTy && "CacheTy should be defined.");
    assert(GetCacheEntryPtrFunc && "GetCacheEntryPtrFunc should be defined.");

    Twine FName = Twine(OrigFn->getName()) + ".cache_update";
    Type *VoidTy = getVoidTy();
    Type *PtrTy = getPtrTy();
    Type *ArgTy = OrigFn->getArg(0)->getType();
    Type *RetTy = OrigFn->getReturnType();
    FunctionType *FTy = FunctionType::get(VoidTy, {ArgTy, RetTy, PtrTy}, false);
    Function *F = Function::Create(FTy, Function::PrivateLinkage, FName, M);

    auto *Key = F->getArg(0);
    Key->setName("key");
    auto *Val = F->getArg(1);
    Val->setName("value");
    auto *Cache = F->getArg(2);
    Cache->setName("cache");
    SetInsertPoint(BasicBlock::Create(getContext(), "entry", F));
    getInlineReport()->addFunction(F);
    getMDInlineReport()->addFunction(F);
    auto *EntryPtr =
        CreateCall(GetCacheEntryPtrFunc, {Key, Cache}, "entry.ptr");
    getInlineReport()->addCallSite(EntryPtr);
    getMDInlineReport()->addCallSite(EntryPtr);
    auto *KeyPtr =
        CreateGEP(CacheTy, EntryPtr, {getInt32(0), getInt32(0)}, "key.ptr");
    CreateStore(Key, KeyPtr);
    auto *ValPtr =
        CreateGEP(CacheTy, EntryPtr, {getInt32(0), getInt32(1)}, "value.ptr");
    CreateStore(Val, ValPtr);
    auto *EngPtr =
        CreateGEP(CacheTy, EntryPtr, {getInt32(0), getInt32(2)}, "engaged.ptr");
    CreateStore(getTrue(), EngPtr);
    CreateRetVoid();
    return F;
  }

  Function *createCacheInitFunc() {
    assert(CacheTy && "CacheTy should be defined.");

    Twine FName = Twine(OrigFn->getName()) + ".cache_init";
    Type *VoidTy = getVoidTy();
    Type *PtrTy = getPtrTy();
    FunctionType *FTy = FunctionType::get(VoidTy, {PtrTy}, false);
    Function *F = Function::Create(FTy, Function::PrivateLinkage, FName, M);

    auto *Cache = F->getArg(0);
    Cache->setName("cache");

    auto *EntryBB = BasicBlock::Create(getContext(), "entry", F);
    getInlineReport()->addFunction(F);
    getMDInlineReport()->addFunction(F);
    auto *ExitBB = BasicBlock::Create(getContext(), "exit", F);
    auto *LoopCondBB = BasicBlock::Create(getContext(), "loop.cond", F);
    auto *LoopBodyBB = BasicBlock::Create(getContext(), "loop.body", F);

    SetInsertPoint(EntryBB);
    CreateBr(LoopCondBB);

    SetInsertPoint(LoopCondBB);
    auto *LoopI = CreatePHI(getInt64Ty(), 2, "loop.i");
    LoopI->addIncoming(getInt64(0), EntryBB);
    auto *Cmp = CreateICmpULT(LoopI, getInt64(CacheSize), "cmp");
    CreateCondBr(Cmp, LoopBodyBB, ExitBB);

    SetInsertPoint(LoopBodyBB);
    auto *EngPtr =
        CreateGEP(CacheTy, Cache, {LoopI, getInt32(2)}, "engaged.ptr");
    CreateStore(getFalse(), EngPtr);
    auto *LoopINext = CreateAdd(LoopI, getInt64(1), "loop.i.next");
    CreateBr(LoopCondBB);

    LoopI->addIncoming(LoopINext, LoopBodyBB);

    SetInsertPoint(ExitBB);
    CreateRetVoid();
    return F;
  }

  Function *createCachedFunc() {
    assert(CacheTy && "CacheTy should be defined.");
    assert(GetCacheEntryPtrFunc && "GetCacheEntryPtrFunc should be defined.");
    assert(CacheUpdateFunc && "CacheUpdateFunc should be defined.");

    auto CachedFnSuffix = ".cached";
    Twine FName = Twine(OrigFn->getName()) + CachedFnSuffix;
    Type *ArgTy = OrigFn->getArg(0)->getType();
    Type *RetTy = OrigFn->getReturnType();
    FunctionType *FTy = FunctionType::get(RetTy, {ArgTy, getPtrTy()}, false);
    Function *F = Function::Create(FTy, Function::PrivateLinkage, FName, M);

    auto *Key = F->getArg(0);
    Key->setName(OrigFn->getArg(0)->getName());
    auto *Cache = F->getArg(1);
    Cache->setName("cache");

    auto *EntryBB = BasicBlock::Create(getContext(), "entry", F);
    getInlineReport()->addFunction(F);
    getMDInlineReport()->addFunction(F);
    auto *Check2BB = BasicBlock::Create(getContext(), "check.2", F);
    auto *GetCacheValBB = BasicBlock::Create(getContext(), "get.cache.val", F);
    auto *CalcValBB = BasicBlock::Create(getContext(), "calc.val", F);

    SetInsertPoint(EntryBB);
    auto *EntryPtr =
        CreateCall(GetCacheEntryPtrFunc, {Key, Cache}, "entry.ptr");
    getInlineReport()->addCallSite(EntryPtr);
    getMDInlineReport()->addCallSite(EntryPtr);
    auto *EngPtr = CreateStructGEP(CacheTy, EntryPtr, 2, "engaged.ptr");
    auto *EngVal = CreateLoad(getInt1Ty(), EngPtr, "engaged");
    auto *IsEngaged = CreateICmpEQ(EngVal, getTrue(), "is.engaged");
    CreateCondBr(IsEngaged, Check2BB, CalcValBB);

    SetInsertPoint(Check2BB);
    auto *KeyPtr = CreateStructGEP(CacheTy, EntryPtr, 0, "key.ptr");
    auto *KeyVal = CreateLoad(ArgTy, KeyPtr, "key.val");
    auto *KeyCmp = CreateICmpEQ(KeyVal, Key, "cache.entry.found");
    CreateCondBr(KeyCmp, GetCacheValBB, CalcValBB);

    SetInsertPoint(GetCacheValBB);
    auto CachedValPtr = CreateStructGEP(CacheTy, EntryPtr, 1, "val.ptr");
    auto *CachedVal = CreateLoad(RetTy, CachedValPtr, "cached.val");
    CreateRet(CachedVal);

    // Clone original function to calculate result value since
    // it was not found in value cache.
    ValueToValueMapTy VMap;
    VMap[OrigFn->getArg(0)] = Key;

    Function::iterator LastBB = --F->end();
    SmallVector<ReturnInst *, 8> Rets;
    CloneFunctionInto(F, OrigFn, VMap,
                      CloneFunctionChangeType::LocalChangesOnly, Rets);
    Function::iterator FirstNewBB = LastBB;
    ++FirstNewBB;

    // Find recursive call sites of original function cloned into cached
    // function code.
    SmallVector<CallBase *, 8> InlinedRecursiveCallSites;
    for (auto BB = FirstNewBB, E = F->end(); BB != E; BB++) {
      for (auto &I : *BB) {
        auto *CB = dyn_cast<CallBase>(&I);
        if (CB && CB->getCalledFunction() == OrigFn)
          InlinedRecursiveCallSites.push_back(CB);
      }
    }

    // Replace calls of original function by calls to the cached function
    for (auto *CB : InlinedRecursiveCallSites) {
      SetInsertPoint(CB);
      Twine NewName =
          CB->hasName() ? Twine{CB->getName()} + CachedFnSuffix : "";
      SmallVector<Value *, 8> NewArgs{CB->args()};
      NewArgs.push_back(Cache);
      auto *NewCall = CreateCall(F, NewArgs, NewName);
      getInlineReport()->replaceCallBaseWithCallBase(CB, NewCall);
      getMDInlineReport()->replaceCallBaseWithCallBase(CB, NewCall);
      CB->replaceAllUsesWith(NewCall);
      CB->eraseFromParent();
    }

    // Link cloned function with "cached" function code
    auto *ClonedEntryBB = cast<BasicBlock>(VMap[&OrigFn->getEntryBlock()]);
    ClonedEntryBB->setName(Twine("entry.") + OrigFn->getName());
    SetInsertPoint(CalcValBB);
    CreateBr(ClonedEntryBB);

    // Add newly calculated value in value cache, before cloned function returns
    for (auto *R : Rets) {
      SetInsertPoint(R);
      auto *CB = CreateCall(CacheUpdateFunc, {Key, R->getReturnValue(), Cache});
      getInlineReport()->addCallSite(CB);
      getMDInlineReport()->addCallSite(CB);
    }
    return F;
  }

  void transform() {
    // Generate helper functions, and types.
    Type *KeyTy = OrigFn->getArg(0)->getType();
    Type *ValTy = OrigFn->getReturnType();
    CacheTy = StructType::create(getContext(), {KeyTy, ValTy, getInt1Ty()},
                                 "struct.cache");
    GetCacheIDFunc = createGetCacheIDFunc();
    GetCacheEntryPtrFunc = createGetCacheEntryPtrFunc();
    CacheUpdateFunc = createCacheUpdateFunc();
    CacheInitFunc = createCacheInitFunc();
    CachedFunc = createCachedFunc();

    // Perform modification of original function.
    MDNode *SaveMetadata = nullptr;
    if (auto CMD = OrigFn->getMetadata(llvm::MDInliningReport::FunctionTag)) {
      SaveMetadata = CMD;
      OrigFn->setMetadata(llvm::MDInliningReport::FunctionTag, nullptr);
    }
    OrigFn->deleteBody();
    if (SaveMetadata)
      OrigFn->setMetadata(llvm::MDInliningReport::FunctionTag, SaveMetadata);
    auto *EntryBB = BasicBlock::Create(getContext(), "entry", OrigFn);
    auto *Key = OrigFn->getArg(0);
    SetInsertPoint(EntryBB);
    auto *Cache = CreateAlloca(CacheTy, getInt32(CacheSize));
    auto *CB = CreateCall(CacheInitFunc, {Cache});
    getInlineReport()->addCallSite(CB);
    getMDInlineReport()->addCallSite(CB);
    auto *Res = CreateCall(CachedFunc, {Key, Cache}, "res");
    getInlineReport()->addCallSite(Res);
    getMDInlineReport()->addCallSite(Res);
    CreateRet(Res);
  }
};
} // namespace

PreservedAnalyses
RecursiveFunctionMemoizePass::run(Module &M, ModuleAnalysisManager &MAM) {
  if (!EnableRecursiveFuncMemoization)
    return PreservedAnalyses::all();

  SmallVector<Function *, 256> Candidates;
  for (auto &F : M) {
    bool Accepted = isCandidate(&F);
    if (Accepted)
      Candidates.push_back(&F);
    LLVM_DEBUG({
      dbgs() << "MEMOIZE: Function(" << F.getName() << ") ";
      dbgs() << ((Accepted) ? "ACCEPTED" : "SKIPPED") << "\n";
    });
  }

  if (Candidates.empty())
    return PreservedAnalyses::all();

  for (auto *C : Candidates) {
    Transformer T{C, FuncMemoizationCacheSize};
    T.transform();
  }

  auto PA = PreservedAnalyses();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
