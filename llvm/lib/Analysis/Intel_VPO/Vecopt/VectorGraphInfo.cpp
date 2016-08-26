//===-- VPOAvrGenerate.cpp ------------------------------------------------===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphInfo.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphUtils.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vector-graph-info"

using namespace llvm;

INITIALIZE_PASS_BEGIN(VectorGraphInfo, "vec-graph-info", "Vector Graph Info", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(VectorGraphInfo, "vec-graph-info", "Vector Graph Info", false, true)

char VectorGraphInfo::ID = 0;

static cl::opt<bool>
    VectorGraphFull("vector-graph-stress", cl::init(true),
         cl::desc("Builds vector graph for all inner loops of function."));

FunctionPass *llvm::createVectorGraphInfoPass() { return new VectorGraphInfo(); }

VectorGraphInfo::VectorGraphInfo() : FunctionPass(ID) {
  llvm::initializeVectorGraphInfoPass(*PassRegistry::getPassRegistry());
}

void VectorGraphInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool VectorGraphInfo::runOnFunction(Function &F) {
 
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  if (VectorGraphFull)
    constructVectorGraph();

  return false;
}

void VectorGraphInfo::print(raw_ostream &OS, const Module *M) const {

  formatted_raw_ostream FOS(OS);

  if (!isVectorGraphEmpty()) {
    FOS << "Vector Graph\n";

    for(auto Itr = begin(), End = end();  Itr != End; ++Itr) {
      Itr->print(FOS,1);
    }
  }
  else
    FOS << "No Vector Graph Generated\n";
}

void VectorGraphInfo::releaseMemory() { VectorGraph.clear(); }

void VectorGraphInfo::constructVectorGraph() {
  SmallVector<Loop *, 8> Worklist;

  // Collect innermost loops.
  for (Loop *L : *LI)
    addInnerLoop(*L, Worklist);

  // Build vector graph for innermost loop.
  while(!Worklist.empty()) {

    VGLoop *VLoop = VectorGraphUtils::createVGLoop(Worklist.pop_back_val());
    VectorGraph.push_back(VLoop);
  }
}

void VectorGraphInfo::constructVectorGraphForLoop(Loop *Lp) {
  releaseMemory();
  VGLoop *VLoop = VectorGraphUtils::createVGLoop(Lp);
  VectorGraph.push_back(VLoop);
}

void VectorGraphInfo::addInnerLoop(Loop &L, SmallVectorImpl<Loop *> &V) {
  if (L.empty())
    return V.push_back(&L);

  for (Loop *InnerL : L)
    addInnerLoop(*InnerL, V);
}
