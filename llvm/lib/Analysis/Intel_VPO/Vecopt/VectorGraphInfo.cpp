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
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphPredicator.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraphUtils.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vector-graph-info"

using namespace llvm;

INITIALIZE_PASS_BEGIN(VectorGraphInfo, "vec-graph-info", "Vector Graph Info", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(VectorGraphInfo, "vec-graph-info", "Vector Graph Info", false, true)

char VectorGraphInfo::ID = 0;

static cl::opt<bool>
    VectorGraphFull("vector-graph-stress", cl::init(true),
         cl::desc("Builds vector graph for all inner loops of function."));

FunctionPass *llvm::createVectorGraphInfoPass() { return new VectorGraphInfo(); }

VectorGraphInfo::VectorGraphInfo() : FunctionPass(ID) {
  llvm::initializeVectorGraphInfoPass(*PassRegistry::getPassRegistry());
}

VectorGraphInfo::VectorGraphInfo(ScalarEvolution *SE)
    : FunctionPass(ID), SE(SE) {}

void VectorGraphInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
}

bool VectorGraphInfo::runOnFunction(Function &F) {
 
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

  if (VectorGraphFull)
    constructVectorGraph();

  return false;
}

void VectorGraphInfo::print(raw_ostream &OS, const Module *M) const {
#if !INTEL_PRODUCT_RELEASE

  formatted_raw_ostream FOS(OS);

  if (!isVectorGraphEmpty()) {
    FOS << "Vector Graph\n";

    for(auto Itr = begin(), End = end();  Itr != End; ++Itr) {
      Itr->print(FOS,1);
    }
  }
  else
    FOS << "No Vector Graph Generated\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//TODO: Make sure we are removing all the VGBlocks, including the
//Predicates and Loop Exit
void VectorGraphInfo::releaseMemory() { VectorGraph.clear(); }

void VectorGraphInfo::constructVectorGraph() {
  SmallVector<Loop *, 8> Worklist;

  // Collect innermost loops.
  for (Loop *L : *LI)
    addInnerLoop(*L, Worklist);

  // Build vector graph for innermost loop.
  while (!Worklist.empty()) {
    Loop *L = Worklist.pop_back_val();
    VGLoop *VLoop = VectorGraphUtils::createVGLoop(L);
    VectorGraph.push_back(VLoop);
    LoopMap[L] = VLoop;
  }
}

void VectorGraphInfo::constructVectorGraphForLoop(Loop *Lp) {
  releaseMemory();
  VGLoop *VLoop = VectorGraphUtils::createVGLoop(Lp);
  VectorGraph.push_back(VLoop);
  LoopMap[Lp] = VLoop;
}

bool VectorGraphInfo::canConstructVectorGraphForLoop(Loop *Lp) {

  // TODO: Add more cases?
  for (auto Itr = Lp->block_begin(), End = Lp->block_end(); Itr != End; ++Itr) {

    // Switches are not supported
    if (!isa<BranchInst>((*Itr)->getTerminator())) {
      // TODO
      // emitAnalysis(VectorizationReport((*Itr)->getTerminator())
      //             << "loop contains a switch statement");
      errs() << "loop contains a switch statement";
      return false;
    }
  }

  return true;
}

void VectorGraphInfo::predicateVectorGraph() {

  for (auto VGN = begin(), End = end(); VGN != End; ++VGN) {
    assert(isa<VGLoop>(*VGN) && "Predication is only supported for VGLoops");
    VGLoop *VGL = cast<VGLoop>(VGN);

    VectorGraphPredicator Predicator(SE);
    Predicator.runOnAvr(VGL);
  }
}

void VectorGraphInfo::addInnerLoop(Loop &L, SmallVectorImpl<Loop *> &V) {
  if (L.empty())
    return V.push_back(&L);

  for (Loop *InnerL : L)
    addInnerLoop(*InnerL, V);
}

bool VectorGraphInfo::blockNeedsPredication(Loop *L, BasicBlock *BB) {
  auto VGLoopIt = LoopMap.find(L);
  assert(VGLoopIt != LoopMap.end() && "Loop not found in VectorGraph");

  VGBlock *VGB = VGLoopIt->second->getBlock(L, BB);
  VGPredicate *Predicate = VGB->getPredicate();
  assert(Predicate && "Predicate is null");

  //errs() << "Block :\n";
  //BB->dump();
  //errs() << "has this predicate:\n";
  //Predicate->dump();
  //errs() << "is all-one predicate: " << Predicate->isAllOnes() << "\n";

  return !Predicate->isAllOnes();
}

VGPredicate *VectorGraphInfo::getPredicate(Loop *L,
                                           BasicBlock *BB) {
  auto VGLoopIt = LoopMap.find(L);
  assert(VGLoopIt != LoopMap.end() && "Loop not found in VectorGraph");

  VGBlock *VGB = VGLoopIt->second->getBlock(L, BB);
  return VGB->getPredicate();
}

VGPredicate::IncomingTy &
VectorGraphInfo::getEdgeIncoming(Loop *L, BasicBlock *Src, BasicBlock *Dst) {

  //TODO: Find a better way of getting the IncomingTy from Src to Dst.
  //      Iterating on all the incoming predicates is not very efficient.
  VGPredicate *SrcPred = getPredicate(L, Src);
  VGPredicate *DstPred = getPredicate(L, Dst);

  SmallVectorImpl<VGPredicate::IncomingTy> &IncomingPredicates =
      DstPred->getIncoming();
  assert(IncomingPredicates.size() > 0 && "Unexpected empty predicate");

  for (auto &DstIncIt : IncomingPredicates) {
    if (SrcPred == DstIncIt.first)
      return DstIncIt;
  }

  llvm_unreachable("Incoming predicate from Src to Dst not found");
}
