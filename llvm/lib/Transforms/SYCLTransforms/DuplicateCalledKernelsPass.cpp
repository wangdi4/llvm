//==-- DuplicateCalledKernels.cpp - DuplicateCalledKernels pass --*- C++ -*-==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/DuplicateCalledKernelsPass.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Transforms/SYCLTransforms/LocalBufferAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/CallGraphUpdater.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-duplicate-called-kernels"

using FuncPtrSet = SmallPtrSet<Function *, 16>;

static constexpr StringRef CloneNameSuffix = ".clone";

PreservedAnalyses DuplicateCalledKernelsPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  CallGraph &CG = AM.getResult<CallGraphAnalysis>(M);
  LocalBufferInfo &LBI = AM.getResult<LocalBufferAnalysis>(M);
  return runImpl(M, CG, LBI) ? PreservedAnalyses::none()
                             : PreservedAnalyses::all();
}

/// For each function in FuncsToClone, add its all user functions that
/// satisfies the predicate to FuncsToClone.
static void addUserOfFuncsToClone(FuncSet &FuncsToClone,
                                  function_ref<bool(Function *)> Predicate) {
  SmallVector<Function *, 16> WorkList(FuncsToClone.begin(),
                                       FuncsToClone.end());
  FuncPtrSet Visited;
  while (!WorkList.empty()) {
    Function *FuncToClone = WorkList.pop_back_val();
    for (User *U : FuncToClone->users()) {
      for (auto It = df_begin(U), E = df_end(U); It != E;) {
        if (auto *I = dyn_cast<Instruction>(*It)) {
          Function *F = I->getFunction();
          if (Predicate(F) && !Visited.contains(F) &&
              !FuncsToClone.contains(F)) {
            FuncsToClone.insert(F);
            WorkList.push_back(F);
          }
          Visited.insert(F);
          It.skipChildren();
          continue;
        }
        ++It;
      }
    }
  }
}

/// Get the map from function to local variables which are directly or
/// indirectly used in the function.
static void
getLocalUseMap(const CallGraph &CG, const FuncSet &KernelSet,
               const LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap,
               LocalBufferInfo::TUsedLocalsMap &LocalUseMap) {
  for (const Function *Kernel : KernelSet) {
    const CallGraphNode *N = CG[Kernel];
    for (auto It = po_begin(N), E = po_end(N); It != E; ++It) {
      Function *F = It->getFunction();
      if (!F || F->isDeclaration())
        continue;
      LocalBufferInfo::TUsedLocals Locals = DirectLocalUseMap.lookup(F);
      const CallGraphNode *N = CG[F];
      for (auto &Pair : *N) {
        Function *Callee = Pair.second->getFunction();
        if (!Callee || Callee->isDeclaration())
          continue;
        if (auto MapIt = LocalUseMap.find(Callee); MapIt != LocalUseMap.end())
          Locals.insert(MapIt->second.begin(), MapIt->second.end());
      }
      if (!Locals.empty())
        LocalUseMap.insert({F, std::move(Locals)});
    }
  }
}

// Update LocalUseMap incrementally on kernels that use newly cloned function.
void updateLocalUseMap(const CallGraph &CG, const FuncSet &KernelSet,
                       const LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap,
                       LocalBufferInfo::TUsedLocalsMap &LocalUseMap,
                       Function *NewClonedRoot) {
  // Find kernels that call NewClonedRoot.
  FuncSet KernelUsers;
  FuncPtrSet Visited;
  FuncVec WorkList;
  WorkList.push_back(NewClonedRoot);
  while (!WorkList.empty()) {
    Function *F = WorkList.pop_back_val();
    Visited.insert(F);
    for (User *U : F->users()) {
      for (auto It = df_begin(U), E = df_end(U); It != E;) {
        if (auto *I = dyn_cast<Instruction>(*It)) {
          Function *UserF = I->getFunction();
          if (!Visited.contains(UserF)) {
            Visited.insert(UserF);
            if (KernelSet.contains(UserF))
              KernelUsers.insert(UserF);
            else
              WorkList.push_back(UserF);
          }
          It.skipChildren();
          continue;
        }
        ++It;
      }
    }
  }

  getLocalUseMap(CG, KernelUsers, DirectLocalUseMap, LocalUseMap);
}

static FuncPtrSet
getFunctionsInCGNodeIf(const CallGraphNode *N,
                       function_ref<bool(Function *)> Predicate) {
  FuncPtrSet Funcs;
  for (auto It = df_begin(N), E = df_end(N); It != E; ++It) {
    Function *F = It->getFunction();
    if (Predicate(F))
      Funcs.insert(F);
  }
  return Funcs;
}

/// Find a kernel that needs fix, i.e. it and another kernel call the same
/// function which directly or indirectly uses the same local variables. Returns
/// a pair of a kernel to fix and a set of shared local variables.
static std::optional<std::tuple<Function *, FuncPtrSet>>
findKernelToFix(const CallGraph &CG, const FuncSet &KernelSet,
                const LocalBufferInfo::TUsedLocalsMap &LocalUseMap,
                FuncPtrSet &SkipKernels) {
  auto HasLocals = [&](Function *F) {
    return F && !F->isDeclaration() && LocalUseMap.count(F);
  };
  for (auto It1 = KernelSet.begin(), E = std::prev(KernelSet.end()); It1 != E;
       ++It1) {
    Function *K1 = *It1;
    if (SkipKernels.contains(K1))
      continue;
    const CallGraphNode *N1 = CG[K1];
    auto Funcs1 = getFunctionsInCGNodeIf(N1, HasLocals);
    if (Funcs1.empty())
      continue;
    for (auto It2 = std::next(It1), E2 = KernelSet.end(); It2 != E2; ++It2) {
      Function *K2 = *It2;
      if (SkipKernels.contains(K2))
        continue;
      const CallGraphNode *N2 = CG[K2];
      auto Funcs2 = getFunctionsInCGNodeIf(N2, HasLocals);
      if (Funcs2.empty())
        continue;
      set_intersect(Funcs1, Funcs2);
      if (!Funcs1.empty()) {
        LLVM_DEBUG(dbgs() << "Found kernel to fix: " << K1->getName() << "\n");
        return std::make_tuple(K1, std::move(Funcs1));
      }
    }

    // K1 doesn't share local variables with other kernels. It can be skipped
    // in the next run.
    SkipKernels.insert(K1);
  }

  return std::nullopt;
}

/// Find a candidate non-kernel function to clone.
/// Returns a pair of
///   * candidate function to clone.
///   * a set of functions that keep using original candidate function instead
///     of cloned one.
static std::optional<std::pair<Function *, FuncPtrSet>>
findFunctionToClone(const CallGraph &CG, const FuncSet &KernelSet,
                    LocalBufferInfo::TUsedLocalsMap &LocalUseMap,
                    FuncPtrSet &SkipKernels) {
  auto KernelToFix = findKernelToFix(CG, KernelSet, LocalUseMap, SkipKernels);
  if (!KernelToFix)
    return std::nullopt;
  const CallGraphNode *N1 = CG[std::get<0>(*KernelToFix)];
  auto &FuncsShared = std::get<1>(*KernelToFix);

  // Walk through N1 in reverse post-order to find the first function to clone.
  // Uses of the function in other kernels will be replaced later with cloned
  // function.
  ReversePostOrderTraversal<const CallGraphNode *> RPOT(N1);
  for (auto It = ++RPOT.begin(), E = RPOT.end(); It != E; ++It) {
    Function *F = (*It)->getFunction();
    if (FuncsShared.contains(F)) {
      auto N1Funcs = getFunctionsInCGNodeIf(
          N1, [](Function *Func) { return Func && !Func->isDeclaration(); });
      LLVM_DEBUG(dbgs() << "Found function to clone: " << F->getName() << "\n");
      return std::make_pair(F, N1Funcs);
    }
  }

  return std::nullopt;
}

/// When cloing a root function, we need to clone local variables used in the
/// function. If a local variable is used in a not-inlined function that is in
/// root function's call graph, we need to clone both the local variable and the
/// not-inlined function.
static Function *
cloneFunctions(Module &M, CallGraph &CG,
               LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap,
               Function *Root, bool RootIsKernel,
               const std::optional<FuncPtrSet> &NotReplaceSet) {
  FuncSet FuncsToClone;
  FuncPtrSet FuncsInRootCG;
  CallGraphNode *N = CG[Root];
  for (auto SCCIt = scc_begin(N), E = scc_end(N); !SCCIt.isAtEnd(); ++SCCIt) {
    const std::vector<CallGraphNode *> NextSCC = *SCCIt;
    if (NextSCC.empty())
      continue;
    auto UseLocals = [&](Function *F) {
      if (!F || F->isDeclaration())
        return false;
      FuncsInRootCG.insert(F);
      return DirectLocalUseMap.count(F) != 0;
    };
    bool NeedClone = false;
    for (CallGraphNode *Node : NextSCC)
      NeedClone |= UseLocals(Node->getFunction());
    if (NeedClone) {
      // All functions in the SCC need to be cloned.
      for (CallGraphNode *Node : NextSCC)
        if (auto *F = Node->getFunction(); F && !F->isDeclaration())
          FuncsToClone.insert(F);
    }
  }

  // If users of the functions to clone are within Root's callgraph,
  // the users also need to be cloned.
  addUserOfFuncsToClone(FuncsToClone,
                        [&](Function *F) { return FuncsInRootCG.contains(F); });

  if (RootIsKernel)
    FuncsToClone.insert(Root);

  // Create functions.
  ValueToValueMapTy VMap;
  for (Function *F : FuncsToClone) {
    Function *NewF = Function::Create(F->getFunctionType(), F->getLinkage(),
                                      F->getAddressSpace(), F->getName(), &M);
    NewF->copyAttributesFrom(F);
    NewF->setComdat(F->getComdat());
    VMap.insert({F, NewF});

    auto ArgNewF = NewF->arg_begin();
    for (auto ArgIt = F->arg_begin(), E = F->arg_end(); ArgIt != E; ++ArgIt) {
      ArgNewF->setName(ArgIt->getName());
      VMap.insert({&*ArgIt, &*(ArgNewF++)});
    }
  }

  // Clone function body in top-down manner. If order is bottom-up and a
  // noinline function is cloned, global variable's scope is visited in
  // DebugInfoFinder::processCompileUnit in CloneFunctionInto and kernel
  // function's Subprogram is cached. Then there are more than one Subprograms
  // available when cloning the noinline function, which changes the behavior
  // and causes debug info isn't cloned for the kernel function.
  // TODO this probably needs further investigation.
  for (Function *F : reverse(FuncsToClone)) {
    SmallVector<ReturnInst *, 8> Returns;
    Function *NewF = cast<Function>(VMap[F]);
    CloneFunctionInto(NewF, F, VMap, CloneFunctionChangeType::LocalChangesOnly,
                      Returns);

    NewF->setName(Twine(F->getName()) + CloneNameSuffix);
    NewF->setLinkage(GlobalValue::InternalLinkage);
    std::ignore = CG.getOrInsertFunction(NewF);
    // Update DirectLocalUseMap for cloned function.
    if (auto DIt = DirectLocalUseMap.find(F); DIt != DirectLocalUseMap.end())
      DirectLocalUseMap.insert({NewF, DIt->second});
  }

  // Update call graph.
  for (const auto &Pair : VMap)
    if (auto *F = dyn_cast<Function>(Pair.first); F && !F->isDeclaration())
      CG.populateCallGraphNode(CG[cast<Function>(Pair.second)]);

  // Replace root function with cloned function. Only replace CallInst user.
  // TODO replace constant user as well.
  Function *NewRoot = cast<Function>(VMap[Root]);
  LLVM_DEBUG(dbgs() << "Clone Root: " << Root->getName()
                    << ", NewRoot: " << NewRoot->getName());
  for (User *U : make_early_inc_range(Root->users())) {
    if (auto *CI = dyn_cast<CallInst>(U)) {
      Function *UserF = CI->getFunction();
      // Don't replace uses in NotReplaceSet.
      if (NotReplaceSet && NotReplaceSet->contains(UserF))
        continue;
      CI->replaceUsesOfWith(Root, NewRoot);
      // Update CallGraph.
      CallGraphNode *N = CG[UserF];
      N->removeAllCalledFunctions();
      CG.populateCallGraphNode(N);
    }
  }

  return NewRoot;
}

/// Clone local variable if it is used in multiple kernels. At this point, a
/// function using local variable won't be called by two kernels.
/// For each kernel, we collect local variables that need to be cloned. Then
/// they are cloned and replaced in functions that are called by this kernel.
static bool cloneLocalVariables(Module &M, const CallGraph &CG,
                                const FuncSet &KernelSet,
                                LocalBufferInfo::TUsedLocalsMap &LocalUseMap) {
  bool Changed = false;
  unsigned NumKernels = KernelSet.size();
  SmallVector<FuncPtrSet> FuncsInKernelsCG(NumKernels);
  SmallVector<LocalBufferInfo::TUsedLocals> LocalsUsedInKernels(NumKernels);
  LocalBufferInfo::TUsedLocals AllLocals;
  for (unsigned Idx = 0; Idx < NumKernels; ++Idx) {
    Function *Kernel = KernelSet[Idx];
    auto Funcs1 = getFunctionsInCGNodeIf(
        CG[Kernel], [](Function *F) { return F && !F->isDeclaration(); });
    assert(Funcs1.contains(Kernel));
    FuncsInKernelsCG[Idx] = std::move(Funcs1);
    auto Locals = LocalUseMap.lookup(Kernel);
    AllLocals.insert(Locals.begin(), Locals.end());
    LocalsUsedInKernels[Idx] = std::move(Locals);
  }

  // Clone local variables.
  SmallVector<DenseMap<GlobalVariable *, GlobalVariable *>> FuncToGV(
      NumKernels);
  for (auto *GV : AllLocals) {
    bool First = true;
    for (unsigned Idx = 0; Idx < NumKernels; ++Idx) {
      if (LocalsUsedInKernels[Idx].contains(GV)) {
        if (First) {
          First = false;
          continue;
        }
        // Clone the local variable for current kernel.
        auto *NewGV = new GlobalVariable(
            M, GV->getValueType(), GV->isConstant(), GV->getLinkage(),
            GV->getInitializer(), Twine(GV->getName()) + CloneNameSuffix,
            nullptr, GV->getThreadLocalMode(), GV->getAddressSpace());
        NewGV->copyAttributesFrom(GV);
        FuncToGV[Idx].insert({GV, NewGV});
        LLVM_DEBUG(dbgs() << "Clone GV: " << GV->getName()
                          << ", NewGV: " << NewGV->getName() << "\n");
        Changed = true;
      }
    }
  }

  // Remap old global variable to cloned global variable.
  // Skip the first kernel since it will use old global variables.
  for (unsigned Idx = 1; Idx < NumKernels; ++Idx) {
    ValueToValueMapTy VMap;
    for (auto &Pair : FuncToGV[Idx])
      VMap[Pair.first] = Pair.second;
    for (auto &Pair : FuncToGV[Idx]) {
      GlobalVariable *GV = Pair.first;
      GlobalVariable *NewGV = Pair.second;
      SmallVector<User *> Users(GV->users());
      for (User *U : Users) {
        if (auto *I = dyn_cast<Instruction>(U);
            I && FuncsInKernelsCG[Idx].contains(I->getFunction())) {
          I->replaceUsesOfWith(GV, NewGV);
          continue;
        }
        for (auto It = df_begin(U), E = df_end(U); It != E;) {
          if (auto *I = dyn_cast<Instruction>(*It)) {
            if (FuncsInKernelsCG[Idx].contains(I->getFunction())) {
              RemapInstruction(
                  I, VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);
            }
            It.skipChildren();
            continue;
          }
          ++It;
        }
      }
    }
  }

  return Changed;
}

bool DuplicateCalledKernelsPass::runImpl(Module &M, CallGraph &CG,
                                         LocalBufferInfo &LBI) {
  bool Changed = false;
  auto Kernels = SYCLKernelMetadataAPI::KernelList(&M);
  if (Kernels.size() < 2)
    return Changed;

  const FuncSet KernelSet{Kernels.begin(), Kernels.end()};
  // Make a copy of kernels since cloneFunctions changes CG.
  SmallVector<Function *, 16> KernelWorkList;
  for (auto I = po_begin(&CG), E = po_end(&CG); I != E; ++I) {
    auto *F = I->getFunction();
    if (!F || F->isDeclaration() || !KernelSet.contains(F))
      continue;
    if (any_of(F->users(), [](User *U) { return isa<CallInst>(U); }))
      KernelWorkList.push_back(F);
  }

  // Clone kernels.
  LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap = LBI.getDirectLocalsMap();
  for (Function *F : KernelWorkList) {
    std::ignore =
        cloneFunctions(M, CG, DirectLocalUseMap, F, true, std::nullopt);
    Changed = true;
  }

  // Return if there is no local variable.
  if (DirectLocalUseMap.empty())
    return Changed;

  // Find a candidate non-kernel function, which uses local variable and is used
  // by two kernels, to clone. Its dependent functions are also cloned.
  // Cloning changes call graph and potential candidates, so we need to do this
  // recursively.
  FuncPtrSet SkipKernels;
  LocalBufferInfo::TUsedLocalsMap LocalUseMap;
  getLocalUseMap(CG, KernelSet, DirectLocalUseMap, LocalUseMap);
  while (auto ToClone =
             findFunctionToClone(CG, KernelSet, LocalUseMap, SkipKernels)) {
    Function *NewClonedRoot = cloneFunctions(
        M, CG, DirectLocalUseMap, ToClone->first, false, ToClone->second);
    updateLocalUseMap(CG, KernelSet, DirectLocalUseMap, LocalUseMap,
                      NewClonedRoot);

    Changed = true;
  }

  // Clone local variables which are used in multiple kernels.
  Changed |= cloneLocalVariables(M, CG, KernelSet, LocalUseMap);

  return Changed;
}
