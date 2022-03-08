#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===------- WRegionInfo.cpp - Build WRegion Graph ---------*- C++ -*------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the W-Region Information Graph build pass.
//
//===----------------------------------------------------------------------===//
#include "llvm/Pass.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/AliasAnalysis.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrninfo"

AnalysisKey WRegionInfoAnalysis::Key;

WRegionInfo WRegionInfoAnalysis::run(Function &F, FunctionAnalysisManager &AM) {

  LLVM_DEBUG(dbgs() << "\nENTER WRegionInfoAnalysis::run: " << F.getName()
                    << "{\n");

  auto &WRC  = AM.getResult<WRegionCollectionAnalysis>(F);
  auto *DT   = WRC.getDomTree();
  auto *LI   = WRC.getLoopInfo();
  auto *SE   = WRC.getSE();
  auto *AA   = WRC.getAliasAnalysis();
  auto &ORE  = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  WRegionInfo WRI(&F, DT, LI, SE, AA, &WRC, ORE);

  LLVM_DEBUG(dbgs() << "\n}EXIT WRegionInfoAnalysis::run: " << F.getName()
                    << "\n");
  return WRI;
}

INITIALIZE_PASS_BEGIN(WRegionInfoWrapperPass, "vpo-wrninfo",
                      "VPO Work-Region Information", false, true)
INITIALIZE_PASS_DEPENDENCY(WRegionCollectionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(WRegionInfoWrapperPass, "vpo-wrninfo",
                    "VPO Work-Region Information", false, true)

char WRegionInfoWrapperPass::ID = 0;

FunctionPass *llvm::createWRegionInfoWrapperPassPass() {
  return new WRegionInfoWrapperPass();
}

WRegionInfoWrapperPass::WRegionInfoWrapperPass() : FunctionPass(ID) {
  // LLVM_DEBUG(dbgs() << "\nStart W-Region Information Collection Pass\n\n");
  initializeWRegionInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

void WRegionInfoWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<WRegionCollectionWrapperPass>();
  AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
}

bool WRegionInfoWrapperPass::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "\nENTER WRegionInfoWrapperPass::runOnFunction: "
                    << F.getName() << "{\n");

  auto &WRC  = getAnalysis<WRegionCollectionWrapperPass>().getWRegionCollection();
  auto *DT   = WRC.getDomTree();
  auto *LI   = WRC.getLoopInfo();
  auto *SE   = WRC.getSE();
  auto *AA   = WRC.getAliasAnalysis();
  auto &ORE  = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

  WRI.reset(new WRegionInfo(&F, DT, LI, SE, AA, &WRC, ORE));

  LLVM_DEBUG(dbgs() << "\n}EXIT WRegionInfoWrapperPass::runOnFunction: "
                    << F.getName() << "\n");
  return false;
}

void WRegionInfoWrapperPass::releaseMemory() { WRI.reset(); }

WRegionInfo::WRegionInfo(Function *F, DominatorTree *DT, LoopInfo *LI,
                         ScalarEvolution *SE, AAResults *AA,
                         WRegionCollection *WRC, OptimizationRemarkEmitter &ORE)
    : Func(F), DT(DT), LI(LI), SE(SE), AA(AA), WRC(WRC), ORE(ORE) {}

#if INTEL_CUSTOMIZATION
void WRegionInfo::setupAAWithOptLevel(unsigned OptLevel) {
  if (AA)
    AA->setupWithOptLevel(OptLevel);
}

void WRegionInfo::buildWRGraph(IRKind IR) {
#else
void WRegionInfo::buildWRGraph() {
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\nENTER WRegionInfo::buildWRGraph{\n");

#if INTEL_CUSTOMIZATION
  WRC->buildWRGraph(IR);
#else
  WRC->buildWRGraph();
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\nRC Size = " << WRC->getWRGraphSize() << "\n");
  for (auto I = WRC->begin(), E = WRC->end(); I != E; ++I)
    LLVM_DEBUG((*I)->dump());

  LLVM_DEBUG(dbgs() << "\n}EXIT WRegionInfo::buildWRGraph\n");
}

void WRegionInfo::print(raw_ostream &OS) const {
  formatted_raw_ostream FOS(OS);

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

bool WRegionInfo::invalidate(Function &F, const PreservedAnalyses &PA,
                             FunctionAnalysisManager::Invalidator &Inv) {
  // We do not care if the analysis is not preserved or CFG was modified,
  // because buildWRGraph() will recompute the graph anyway.
  // We only care about the analysis references that we keep in
  // WRegionCollection.
  return Inv.invalidate<WRegionCollectionAnalysis>(F, PA) ||
      Inv.invalidate<OptimizationRemarkEmitterAnalysis>(F, PA);
}

#endif // INTEL_COLLAB
