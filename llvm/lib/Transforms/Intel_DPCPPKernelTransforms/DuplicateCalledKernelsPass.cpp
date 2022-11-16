//==-- DuplicateCalledKernels.cpp - DuplicateCalledKernels pass --*- C++ -*-==//
//
// Copyright (C) 2020 - 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DuplicateCalledKernelsPass.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LocalBufferAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/CallGraphUpdater.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-duplicate-called-kernels"

using FuncPtrSet = SmallPtrSet<Function *, 16>;

static constexpr StringRef CloneNameSuffix = ".clone";

char DuplicateCalledKernelsLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(
    DuplicateCalledKernelsLegacy, DEBUG_TYPE,
    "DuplicateCalledKernelsLegacy Pass - Clone kernels called from "
    "other functions",
    false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LocalBufferAnalysisLegacy)
INITIALIZE_PASS_END(
    DuplicateCalledKernelsLegacy, DEBUG_TYPE,
    "DuplicateCalledKernelsLegacy Pass - Clone kernels called from "
    "other functions",
    false, false)

DuplicateCalledKernelsLegacy::DuplicateCalledKernelsLegacy() : ModulePass(ID) {
  initializeDuplicateCalledKernelsLegacyPass(*PassRegistry::getPassRegistry());
}

void DuplicateCalledKernelsLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<LocalBufferAnalysisLegacy>();
}

bool DuplicateCalledKernelsLegacy::runOnModule(Module &M) {
  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  LocalBufferInfo &LBI = getAnalysis<LocalBufferAnalysisLegacy>().getResult();
  return Impl.runImpl(M, CG, LBI);
}

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
      LocalUseMap[F] = DirectLocalUseMap.lookup(F);
      const CallGraphNode *N = CG[F];
      for (auto Pair : *N) {
        Function *Callee = Pair.second->getFunction();
        if (!Callee || Callee->isDeclaration())
          continue;
        if (auto MapIt = LocalUseMap.find(Callee); MapIt != LocalUseMap.end()) {
          auto Copy = MapIt->second;
          LocalUseMap[F].insert(Copy.begin(), Copy.end());
        }
      }
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

/// Find a pair of kernels that need fix, i.e. they call the same function which
/// directly or indirectly uses local variables.
static Optional<std::pair<Function *, Function *>>
findKernelPairTofix(const FuncSet &KernelSet,
                    const LocalBufferInfo::TUsedLocalsMap &LocalUseMap,
                    FuncPtrSet &SkipKernels) {
  for (auto It1 = KernelSet.begin(), E = std::prev(KernelSet.end()); It1 != E;
       ++It1) {
    Function *K1 = *It1;
    if (SkipKernels.contains(K1))
      continue;
    const auto LocalIt1 = LocalUseMap.find(K1);
    if (LocalIt1 == LocalUseMap.end())
      continue;
    LocalBufferInfo::TUsedLocals Locals1 = LocalIt1->second; // make a copy
    for (auto It2 = std::next(It1), E2 = KernelSet.end(); It2 != E2; ++It2) {
      Function *K2 = *It2;
      if (SkipKernels.contains(K2))
        continue;
      const auto LocalIt2 = LocalUseMap.find(K2);
      if (LocalIt2 == LocalUseMap.end())
        continue;
      set_intersect(Locals1, LocalIt2->second);
      if (!Locals1.empty()) {
        return std::make_pair(K1, K2);
      }
    }

    // K1 doesn't share local variables with other kernels. It can be skipped
    // in the next run.
    SkipKernels.insert(K1);
  }

  return None;
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

/// Find a candidate function to clone.
/// Returns a pair of
///   * candidate function to clone.
///   * a set of functions that keep using original candidate function instead
///     of cloned one.
static Optional<std::pair<Function *, FuncPtrSet>>
findFunctionToClone(const CallGraph &CG, const FuncSet &KernelSet,
                    const LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap,
                    LocalBufferInfo::TUsedLocalsMap &LocalUseMap,
                    FuncPtrSet &SkipKernels) {
  auto KernelPair = findKernelPairTofix(KernelSet, LocalUseMap, SkipKernels);
  if (!KernelPair)
    return None;
  const CallGraphNode *N1 = CG[KernelPair->first];
  const CallGraphNode *N2 = CG[KernelPair->second];

  auto HasLocals = [&](Function *F) {
    return F && !F->isDeclaration() && LocalUseMap.count(F);
  };
  auto Funcs1 = getFunctionsInCGNodeIf(N1, HasLocals);
  auto FuncsShared = getFunctionsInCGNodeIf(N2, HasLocals);
  set_intersect(FuncsShared, Funcs1);
  if (FuncsShared.empty())
    return None;

  // Walk through N1 in reverse post-order to find the first function to clone.
  // Uses of the function in other kernels will be replaced later with cloned
  // function.
  ReversePostOrderTraversal<const CallGraphNode *> RPOT(N1);
  for (auto It = ++RPOT.begin(), E = RPOT.end(); It != E; ++It) {
    Function *F = (*It)->getFunction();
    if (FuncsShared.contains(F)) {
      auto N1Funcs = getFunctionsInCGNodeIf(
          N1, [](Function *Func) { return Func && !Func->isDeclaration(); });
      return std::make_pair(F, N1Funcs);
    }
  }

  return None;
}

/// When cloing a root function, we need to clone local variables used in the
/// function. If a local variable is used in a not-inlined function that is in
/// root function's call graph, we need to clone both the local variable and the
/// not-inlined function.
static Function *cloneFunctionAndLocalVariable(
    Module &M, CallGraph &CG,
    LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap, Function *Root,
    bool RootIsKernel, const Optional<FuncPtrSet> &NotReplaceSet) {
  FuncSet FuncsToClone;
  FuncPtrSet FuncsInRootCG;
  ValueToValueMapTy VMap;
  CallGraphNode *N = CG[Root];
  for (auto SCCIt = scc_begin(N), E = scc_end(N); !SCCIt.isAtEnd(); ++SCCIt) {
    const std::vector<CallGraphNode *> NextSCC = *SCCIt;
    if (NextSCC.empty())
      continue;
    auto NeedClone = [&](CallGraphNode *Node) {
      Function *F = Node->getFunction();
      if (!F || F->isDeclaration())
        return false;

      FuncsInRootCG.insert(F);
      auto DIt = DirectLocalUseMap.find(F);
      // TODO Fix LocalBufferAnalysis so that DIt->second can't be empty.
      if (DIt == DirectLocalUseMap.end() || DIt->second.empty())
        return false;

      // Need to clone the function if it has direct local variable use.
      // Clone local variable's GlobalVariable along the way.
      for (GlobalValue *V : DIt->second) {
        if (VMap.count(V))
          continue;
        auto *GV = cast<GlobalVariable>(V);
        auto *NewGV = new GlobalVariable(
            M, GV->getValueType(), GV->isConstant(), GV->getLinkage(),
            GV->getInitializer(), Twine(GV->getName()) + CloneNameSuffix,
            nullptr, GV->getThreadLocalMode(), GV->getAddressSpace());
        NewGV->copyAttributesFrom(GV);
        VMap.insert({GV, NewGV});
      }
      return true;
    };
    if (none_of(NextSCC, NeedClone))
      continue;

    // All functions in the SCC need to be cloned.
    for (CallGraphNode *Node : NextSCC)
      if (auto *F = Node->getFunction(); F && !F->isDeclaration())
        FuncsToClone.insert(F);
  }

  // If users of the functions to clone are within Root's callgraph,
  // the users also need to be cloned.
  addUserOfFuncsToClone(FuncsToClone,
                        [&](Function *F) { return FuncsInRootCG.contains(F); });

  if (RootIsKernel)
    FuncsToClone.insert(Root);

  // Create functions.
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
    if (auto *SP = NewF->getSubprogram(); SP && SP->getUnit())
      SP->replaceLinkageName(MDString::get(M.getContext(), NewF->getName()));
    std::ignore = CG.getOrInsertFunction(NewF);
    // Update DirectLocalUseMap for cloned function.
    if (auto DIt = DirectLocalUseMap.find(F); DIt != DirectLocalUseMap.end()) {
      LocalBufferInfo::TUsedLocals Locals;
      for (auto *GV : DIt->second) {
        auto It = VMap.find(GV);
        Locals.insert(It == VMap.end() ? GV : cast<GlobalVariable>(It->second));
      }
      DirectLocalUseMap[NewF] = Locals;
    }
  }

  // Update call graph.
  for (const auto &Pair : VMap)
    if (auto *F = dyn_cast<Function>(Pair.first); F && !F->isDeclaration())
      CG.populateCallGraphNode(CG[cast<Function>(Pair.second)]);

  // Replace root function with cloned function. Only replace CallInst user.
  // TODO replace constant user as well.
  Function *NewRoot = cast<Function>(VMap[Root]);
  for (User *U : make_early_inc_range(Root->users())) {
    if (auto *CI = dyn_cast<CallInst>(U)) {
      // Don't replace uses in NotReplaceSet.
      if (NotReplaceSet && NotReplaceSet->contains(CI->getFunction()))
        continue;
      CI->replaceUsesOfWith(Root, NewRoot);
      // Update CallGraph.
      CallGraphNode *N = CG[CI->getFunction()];
      N->removeAllCalledFunctions();
      CG.populateCallGraphNode(N);
    }
  }

  return NewRoot;
}

bool DuplicateCalledKernelsPass::runImpl(Module &M, CallGraph &CG,
                                         LocalBufferInfo &LBI) {
  bool Changed = false;

  auto Kernels = DPCPPKernelMetadataAPI::KernelList(&M);
  if (Kernels.empty())
    return Changed;
  const FuncSet KernelSet{Kernels.begin(), Kernels.end()};

  LocalBufferInfo::TUsedLocalsMap &DirectLocalUseMap = LBI.getDirectLocalsMap();

  // Make a copy of kernels since cloneFunctionAndLocalVariable changes CG.
  SmallVector<Function *, 16> KernelWorkList;
  for (auto I = po_begin(&CG), E = po_end(&CG); I != E; ++I) {
    auto *F = I->getFunction();
    if (!F || F->isDeclaration() || !KernelSet.contains(F))
      continue;
    if (any_of(F->users(), [](User *U) { return isa<CallInst>(U); }))
      KernelWorkList.push_back(F);
  }
  // Clone kernels.
  for (Function *F : KernelWorkList) {
    std::ignore =
        cloneFunctionAndLocalVariable(M, CG, DirectLocalUseMap, F, true, None);
    Changed = true;
  }

  // Find a candidate function, which uses local variable and is used
  // by two kernels, to clone. Its dependent functions are also cloned.
  // Cloning changes call graph and potential candidates, so we need to do this
  // recursively.
  FuncPtrSet SkipKernels;
  LocalBufferInfo::TUsedLocalsMap LocalUseMap;
  getLocalUseMap(CG, KernelSet, DirectLocalUseMap, LocalUseMap);
  while (auto ToClone = findFunctionToClone(CG, KernelSet, DirectLocalUseMap,
                                            LocalUseMap, SkipKernels)) {
    Function *NewClonedRoot = cloneFunctionAndLocalVariable(
        M, CG, DirectLocalUseMap, ToClone->first, false, ToClone->second);
    updateLocalUseMap(CG, KernelSet, DirectLocalUseMap, LocalUseMap,
                      NewClonedRoot);

    Changed = true;
  }

  return Changed;
}

ModulePass *llvm::createDuplicateCalledKernelsLegacyPass() {
  return new llvm::DuplicateCalledKernelsLegacy();
}
