#if INTEL_COLLAB
//===------ WRegionCollection.cpp - Build WRN Graph -----*- C++ -*---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the WRegion Collection pass.
///
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrncollection"

AnalysisKey WRegionCollectionAnalysis::Key;

WRegionCollection WRegionCollectionAnalysis::run(Function &F,
                                                 FunctionAnalysisManager &AM) {

  LLVM_DEBUG(dbgs() << "\nENTER WRegionCollectionAnalysis::run: " << F.getName()
                    << "{\n");

  auto &DI = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  auto &AA = AM.getResult<AAManager>(F);
#if INTEL_CUSTOMIZATION
  auto *HIRF = AM.getCachedResult<loopopt::HIRFrameworkAnalysis>(F);
  WRegionCollection WRC(&F, &DI, &LI, &SE, &TTI, &AC, &TLI, &AA, HIRF);
#else
  WRegionCollection WRC(&F, &DI, &LI, &SE, &TTI, &AC, &TLI, &AA);
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\n}EXIT WRegionCollectionAnalysis::run: " << F.getName()
                    << "\n");
  return WRC;
}

INITIALIZE_PASS_BEGIN(WRegionCollectionWrapperPass, "vpo-wrncollection",
                      "VPO Work-Region Collection", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(WRegionCollectionWrapperPass, "vpo-wrncollection",
                    "VPO Work-Region Collection", false, true)

char WRegionCollectionWrapperPass::ID = 0;

FunctionPass *llvm::createWRegionCollectionWrapperPassPass() {
  return new WRegionCollectionWrapperPass();
}

WRegionCollectionWrapperPass::WRegionCollectionWrapperPass()
    : FunctionPass(ID) {
  initializeWRegionCollectionWrapperPassPass(*PassRegistry::getPassRegistry());
}

void WRegionCollectionWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
}

bool WRegionCollectionWrapperPass::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "\nENTER WRegionCollectionWrapperPass::runOnFunction: "
                    << F.getName() << "{\n");

  auto &DI = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  auto &TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  auto &AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
  auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  auto &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
#if INTEL_CUSTOMIZATION
  auto *HIRFA = getAnalysisIfAvailable<loopopt::HIRFrameworkWrapperPass>();
  WRC.reset(
      new WRegionCollection(&F, &DI, &LI, &SE, &TTI, &AC, &TLI, &AA,
                            HIRFA != nullptr ? &HIRFA->getHIR() : nullptr));
#else
  WRC.reset(new WRegionCollection(&F, &DI, &LI, &SE, &TTI, &AC, &TLI, &AA));
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\n}EXIT WRegionCollectionWrapperPass::runOnFunction: "
                    << F.getName() << "\n");
  return false;
}

/// \brief TBD: get associated Loop Info for a given W-Region
bool WRegionCollection::isCandidateLoop(Loop &Lp) {

  /// Return false if we cannot handle this loop
  if (!Lp.isLoopSimplifyForm())
    return false;

  const SCEV *BETC = SE->getBackedgeTakenCount(&Lp);

  /// Only allow single BB loop for now.
  if (Lp.getBlocks().size() != 1)
    return false;

  /// Only allow constant trip count loop for now.
  if (!isa<SCEVConstant>(BETC))
    return false;

  return true;
}

/// Invoking this routine with the entry BBs of a CFG puts all the BBs
/// of the CFG in the BBStack in a topologically sorted manner (ignoring
/// backedges). Popping BBStack gives BBs in sorted order. This ordering
/// allows buildWRGraph() to build nested OMP constructs correctly,
/// visiting all the inner constructs' directives before processing the END
/// directive of an enclosing outer construct.
void topSortBasicBlocks(
  BasicBlock *BB,
  WRStack<BasicBlock *> &BBStack,
  SmallPtrSetImpl<BasicBlock *> &Visited,
  bool DoVerifyBB
)
{
  // LLVM_DEBUG(dbgs() << "\n=== topSortBasicBlocks visiting this BB: " << *BB);

  // Skip visited nodes.
  if (Visited.count(BB))
    return;

  // Verify that the directives in BB, if any, follow design rules
  if (DoVerifyBB) {
    bool PassedVerify = VPOAnalysisUtils::verifyBB(*BB, true);
    assert(PassedVerify && "Malformed directives in BBlock");
    (void) PassedVerify;
  }

  // Mark BB as "visited".
  Visited.insert(BB);

  //
  Instruction *FirstInstr = BB->getFirstNonPHI();
  BasicBlock *EndBB = nullptr;
  bool IsOmpDir = VPOAnalysisUtils::isOpenMPDirective(FirstInstr);
  if (IsOmpDir) {
    // FirstInstr must be a BEGIN directive
    EndBB= VPOAnalysisUtils::getEndRegionDirBB(FirstInstr);
    assert(EndBB && "topSortBasicBlocks: End Directive not found");
    BBStack.push(EndBB);
    Visited.insert(EndBB);
  }

  // Visit all the successors first
  for (succ_iterator I = succ_begin(BB), E = succ_end(BB); I != E; ++I)
    topSortBasicBlocks(*I, BBStack, Visited, DoVerifyBB);

  // We are only interested in BBs that start with OMP directives. Paying the
  // cost now to look at BB's first instruction allows us to save memory by
  // only pushing BBs with such directives onto BBStack. It will also save
  // compile time later in buildWRGraph, which no longer has to look at all
  // the BBs in the CFG. For typical OpenMP programs where the percentage of
  // BBs with OMP directives is small, this should result in net savings of
  // compile time.
  if (IsOmpDir) {
    // LLVM_DEBUG(dbgs() << "\n=== topSortBasicBlocks pushed this BB: " << *BB);
    BBStack.push(BB);

    // Visit all successors of EndBB
    for (succ_iterator I = succ_begin(EndBB), E = succ_end(EndBB); I!=E; ++I)
      topSortBasicBlocks(*I, BBStack, Visited, DoVerifyBB);
  }
}

void WRegionCollection::buildWRGraphImpl(Function &F) {
  WRGraph = new (WRContainerTy);
  WRStack<WRegionNode *> S;

  // First, sort the CFG's BBs in topological order (ignoring backedges)
  // and put them in BBStack
  BasicBlock *RootBB = &F.getEntryBlock();
  WRStack<BasicBlock *> BBStack;
  SmallPtrSet<BasicBlock *, 32> Visited;

  LLVM_DEBUG(dbgs() << "\nENTER buildWRGraph {\n");

  // Having the last argument==true turns on the verifier by default.
  // TODO: guard it under a flag (or debug mode) when VPO is more stable.
  topSortBasicBlocks(RootBB, BBStack, Visited, true);

  // Then, visit the BBs in sorted order (by popping BBStack) to build WRNs
  while (!BBStack.empty()) {
    BasicBlock *BB = BBStack.top();
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      LLVM_DEBUG(dbgs() << "\n=== buildWRGraph: processing BB: " << *BB);
      IntrinsicInst *Call = dyn_cast<IntrinsicInst>(I);
      if (Call)
        WRegionUtils::updateWRGraph(Call, WRGraph, S, LI, DT, BB);
    }
    BBStack.pop();
  }

  LLVM_DEBUG(dbgs() << "\n} EXIT buildWRGraph\n");
  return;
}

WRegionCollection::WRegionCollection(Function *F, DominatorTree *DT,
                                     LoopInfo *LI, ScalarEvolution *SE,
                                     const TargetTransformInfo *TTI,
                                     AssumptionCache *AC,
                                     const TargetLibraryInfo *TLI,
#if INTEL_CUSTOMIZATION
                                     AliasAnalysis *AA,
                                     loopopt::HIRFramework *HIRF)
    : WRGraph(nullptr), Func(F), DT(DT), LI(LI), SE(SE), TTI(TTI), AC(AC),
      TLI(TLI), AA(AA), HIRF(HIRF) {
}
#else
                                     AliasAnalysis *AA)
    : WRGraph(nullptr), Func(F), DT(DT), LI(LI), SE(SE), TTI(TTI), AC(AC),
      TLI(TLI), AA(AA) {
}
#endif // INTEL_CUSTOMIZATION


#if INTEL_CUSTOMIZATION
void WRegionCollection::buildWRGraph(IRKind IR) {
#else
void WRegionCollection::buildWRGraph() {
#endif // INTEL_CUSTOMIZATION

  // Delete any existing WRGraph if present
  releaseMemory();

#if INTEL_CUSTOMIZATION
  if (IR == HIR) {
    assert(HIRF && "HIR framework not available!");
    WRGraph = WRegionUtils::buildWRGraphFromHIR(*HIRF);
  } else
#endif // INTEL_CUSTOMIZATION
  buildWRGraphImpl(*Func);
}

void WRegionCollectionWrapperPass::releaseMemory() {
  WRC.reset();
#if 0
  for (auto &I : WRegions) {
    delete I;
  }
  WRegions.clear();
#endif
}

void WRegionCollection::print(raw_ostream &OS) const {
  formatted_raw_ostream FOS(OS);

  if (WRGraph == nullptr) {
    // For lit-tests with -analyze the WRGraph can be empty because the
    // graph construction is called by demand only. In such cases, we
    // need to call the graph builder here.

    // Need a non-const pointer to force build for opt -analyze mode.
    auto NonConstWRC = const_cast<WRegionCollection *>(this);
    NonConstWRC->buildWRGraphImpl(*Func);
  }

  if (WRegionUtils::hasTargetDirective(*WRGraph))
    FOS << "\nFunction contains OpenMP Target construct(s).\n";
  else
    FOS << "\nFunction does not contain OpenMP Target constructs.\n";

  for (auto I = begin(), E = end(); I != E; ++I) {
#if INTEL_CUSTOMIZATION
  #if !INTEL_PRODUCT_RELEASE
    FOS << "\n";
    (*I)->print(FOS, 0);
  #endif // !INTEL_PRODUCT_RELEASE
#else
    FOS << "\n";
    (*I)->print(FOS, 0);
#endif // INTEL_CUSTOMIZATION
  }
}

#endif // INTEL_COLLAB
