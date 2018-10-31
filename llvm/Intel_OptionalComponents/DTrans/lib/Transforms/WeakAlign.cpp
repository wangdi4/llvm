//===---------------- WeakAlign.cpp - DTransWeakAlignPass------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Weak Align pass
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/WeakAlign.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-weakalign"

namespace {
class DTransWeakAlignWrapper : public ModulePass {
private:
  dtrans::WeakAlignPass Impl;

public:
  static char ID;

  DTransWeakAlignWrapper() : ModulePass(ID) {
    initializeDTransWeakAlignWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    return Impl.runImpl(M, TLI, WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};
} //  end anonymous namespace

PreservedAnalyses dtrans::WeakAlignPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  if (!runImpl(M, TLI, WPInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

namespace llvm {
namespace dtrans {

class WeakAlignImpl {
public:
  // Analyze and perform the transform, if possible. Return 'true'
  // if IR changes are made.
  bool run(Module &M, const TargetLibraryInfo &TLI);

private:
  bool analyzeModule(Module &M, const TargetLibraryInfo &TLI);
  bool analyzeFunction(Function &F);
  Constant *getMalloptFunction(Module &M, const TargetLibraryInfo &TLI);
};

bool WeakAlignImpl::run(Module &M, const TargetLibraryInfo &TLI) {
  // Make sure the mallopt function is available before analyzing the IR.
  Constant *MalloptFunc = getMalloptFunction(M, TLI);
  if (!MalloptFunc)
    return false;

  // Check for safety issues that prevent the transform.
  if (!analyzeModule(M, TLI))
    return false;

  bool Changed = false;
  // TODO: Add insertion of mallopt call at the start of main.
  return Changed;
}

// Get a handle the mallopt() function, if it is available. Otherwise,
// return nullptr.
Constant *WeakAlignImpl::getMalloptFunction(Module &M,
                                            const TargetLibraryInfo &TLI) {
  LibFunc MalloptLF;
  bool Found = TLI.getLibFunc("mallopt", MalloptLF);
  if (!Found) {
    LLVM_DEBUG(
        dbgs() << "DTRANS Weak Align: inhibited -- mallopt() not available\n");
    return nullptr;
  }

  // Verify the function is available
  if (!TLI.has(MalloptLF)) {
    LLVM_DEBUG(
        dbgs() << "DTRANS Weak Align: inhibited -- mallopt() not available\n");
    return nullptr;
  }

  LLVMContext &Ctx = M.getContext();
  llvm::Type *Int32Ty = IntegerType::getInt32Ty(Ctx);
  auto *MalloptFunc =
      M.getOrInsertFunction("mallopt", Int32Ty, Int32Ty, Int32Ty);
  if (!MalloptFunc) {
    LLVM_DEBUG(
        dbgs()
        << "DTRANS Weak Align: inhibited -- mallopt() mismatched signature\n");
    return nullptr;
  }

  return MalloptFunc;
}

// Check if there are issues within the module that should inhibit setting
// qkmalloc allocator to use the weak memory allocation mode. Return 'true'
// if the function is safe, 'false' otherwise.
bool WeakAlignImpl::analyzeModule(Module &M, const TargetLibraryInfo &TLI) {

  // List of allocation functions that are allowed to be seen in the program.
  // Any reference to an allocation function (as identified in the
  // AllocationFnData table within Analysis/MemoryBuiltins.cpp) will inhibit the
  // transformation.
  static const LibFunc SupportedAllocFns[] = {
      LibFunc_malloc,
      LibFunc_Znwj,
      LibFunc_ZnwjRKSt9nothrow_t,
      LibFunc_Znwm,
      LibFunc_ZnwmRKSt9nothrow_t,
      LibFunc_Znaj,
      LibFunc_ZnajRKSt9nothrow_t,
      LibFunc_Znam,
      LibFunc_ZnamRKSt9nothrow_t,
      LibFunc_calloc,
      LibFunc_realloc,
  };

  auto IsSupportedAllocationFn = [](LibFunc LF) {
    auto Fns = makeArrayRef(SupportedAllocFns);
    return std::any_of(Fns.begin(), Fns.end(),
                       [&LF](LibFunc Elem) { return Elem == LF; });
  };

  // Check for functions that allocate memory to make sure there are only calls
  // to specific routines. This is to ensure there are no uses of a function
  // which may take an alignment argument. It is sufficient to just see if a
  // declaration exists, because that is enough to know that it may be called
  // directly or indirectly without checking each call site since we know we
  // have the whole program. In other words, if it's not seen, there are no
  // calls to it.
  LibFunc TheLibFunc;
  for (auto &F : M) {
    if (TLI.getLibFunc(F.getName(), TheLibFunc) && TLI.has(TheLibFunc) &&
        llvm::isAllocationLibFunc(TheLibFunc) &&
        !IsSupportedAllocationFn(TheLibFunc)) {
      LLVM_DEBUG(dbgs() << "DTRANS Weak Align: inhibited -- May allocate "
                           "alignment memory:\n  "
                        << F.getName() << "\n");
      return false;
    }
  }

  for (auto &F : M)
    if (!analyzeFunction(F))
      return false;

  return true;
}

// Check if there are issues within the function that should inhibit setting
// qkmalloc allocator to use the weak memory allocation mode. Return 'true'
// if the function is safe, 'false' otherwise.
bool WeakAlignImpl::analyzeFunction(Function &F) {
  // Check if a load instruction is supported. Currently, this just checks
  // whether a vector type is loaded because a vector load instruction could
  // require a specific alignment, so we will disable the transform if any
  // are seen.
  auto IsSupportedLoad = [](LoadInst &LI) {
    return !LI.getType()->isVectorTy();
  };

  // Check if a store instruction is supported. Currently, this just checks
  // whether a vector type is stored because a vector store instruction could
  // require a specific alignment.
  auto IsSupportedStore = [](StoreInst &SI) {
    return !SI.getValueOperand()->getType()->isVectorTy();
  };

  LLVM_DEBUG(dbgs() << "DTRANS Weak Align: Analyzing " << F.getName() << "\n");
  for (auto &I : instructions(&F)) {
    if (auto *LI = dyn_cast<LoadInst>(&I)) {
      if (!IsSupportedLoad(*LI)) {
        LLVM_DEBUG(
            dbgs() << "DTRANS Weak Align: inhibited -- Unsupported LoadInst:\n  "
                   << I << "\n");
        return false;
      }
    } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
      if (!IsSupportedStore(*SI)) {
        LLVM_DEBUG(
            dbgs() << "DTRANS Weak Align: inhibited -- Unsupported StoreInst:\n  "
                   << I << "\n");
        return false;
      }
    } else if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
      if (II->getIntrinsicID() == Intrinsic::assume) {
        // The __assume_aligned expression turns into an assume intrinsic
        // call in the IR, so inhibit this transform for any case involving
        // an assume intrinsic. This is more conservative than strictly
        // necessary.
        LLVM_DEBUG(dbgs() << "DTRANS Weak Align: inhibited -- Contains "
                             "unsupported intrinsic:\n  "
                          << I << "\n");
        return false;
      }
    } else if (auto *CI = dyn_cast<CallInst>(&I)) {
      if (CI->isInlineAsm()) {
        LLVM_DEBUG(
            dbgs() << "DTRANS Weak Align: inhibited -- Contains inline asm:\n  "
                   << I << "\n");
        return false;
      }
    }
  }

  return true;
}
bool WeakAlignPass::runImpl(Module &M, const TargetLibraryInfo &TLI,
                            WholeProgramInfo &WPInfo) {
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(
        dbgs() << "DTRANS Weak Align: inhibited -- not whole program safe");
    return false;
  }

  WeakAlignImpl Impl;
  return Impl.run(M, TLI);
}

} // end namespace dtrans
} // end namespace llvm

char DTransWeakAlignWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransWeakAlignWrapper, "dtrans-weakalign",
                      "DTrans weak align", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransWeakAlignWrapper, "dtrans-weakalign",
                    "DTrans weak align", false, false)

ModulePass *llvm::createDTransWeakAlignWrapperPass() {
  return new DTransWeakAlignWrapper();
}
