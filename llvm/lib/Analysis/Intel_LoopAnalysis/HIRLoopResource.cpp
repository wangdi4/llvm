//===----------- HIRLoopResource.cpp - Computes loop resources ------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the loop resource analysis pass.
//
//===----------------------------------------------------------------------===//

// TODO:
// 1. Add complex datatype support when available.
// 2. Add some platform specific support e.g. Atom floating to int
//    ratio is different from Haswell.
// 3. Think about if we want to assign specific cost to int ops such as sdiv.
// 4. Use branch probability information to denote paths of if's and switch.
// 5. Handle more generic types in HLInst such as Vector/Struct types.
// 6. Call instructions might need special handling based on arguments.

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopResource.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-resource"

// Primarily used for verification purpose for lit tests.
// We do not print any information except DEBUG mode.
static cl::opt<bool> verifyLoopResource(
    "hir-verify-loop-resource", cl::init(false), cl::Hidden,
    cl::desc("Pre-computes loop resource for all loops inside function."));

FunctionPass *llvm::createHIRLoopResourcePass() {
  return new HIRLoopResource();
}

char HIRLoopResource::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopResource, "hir-loop-resource",
                      "Loop Resource Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRLoopResource, "hir-loop-resource",
                    "Loop Resource Analysis", false, true)

void HIRLoopResource::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFramework>();
}

bool HIRLoopResource::runOnFunction(Function &F) {
  // This is an on-demand analysis, so we don't perform any analysis here.
  return false;
}

void HIRLoopResource::releaseMemory() { ResourceMap.clear(); }

struct LoopResourceInfo::LoopResourceVisitor final : public HLNodeVisitorBase {
  LoopResourceInfo &LRInfo;
  HIRLoopResource &HLR;

  LoopResourceVisitor(LoopResourceInfo &LRI, HIRLoopResource &HLR)
      : LRInfo(LRI), HLR(HLR) {}

  // Ignore gotos/labels.
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  void visit(const HLDDNode *Node);

  void visit(const HLLoop *Lp) {
    // Add child loop resource.
    const LoopResourceInfo &ChildLRInfo = HLR.getLoopResource(Lp);
    LRInfo += ChildLRInfo;

    // Add child loop's preheader/postexit to parent loop's resource.
    HLNodeUtils::visitRange(*this, Lp->pre_begin(), Lp->pre_end());
    HLNodeUtils::visitRange(*this, Lp->post_begin(), Lp->post_end());
  }

  void visit(const HLIf *If) {
    // Capture the number of compare and logical operations.
    auto Num = If->getNumPredicates();
    LRInfo.IntOps += (2 * Num - 1);

    visit(cast<HLDDNode>(If));
  }

  void visit(const HLInst *HInst);

  bool isDone() const override { return false; }
};

void LoopResourceInfo::LoopResourceVisitor::visit(const HLDDNode *Node) {
  for (auto RefIt = Node->op_ddref_begin(), E = Node->op_ddref_end();
       RefIt != E; ++RefIt) {
    auto Ref = *RefIt;

    if (!Ref->isMemRef()) {
      continue;
    }

    bool IsFloat = Ref->getDestType()->isFPOrFPVectorTy();

    if ((*RefIt)->isLval()) {
      IsFloat ? ++LRInfo.FPMemWrites : ++LRInfo.IntMemWrites;
    } else {
      IsFloat ? ++LRInfo.FPMemReads : ++LRInfo.IntMemReads;
    }
  }
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLInst *HInst) {
  visit(cast<HLDDNode>(HInst));

  auto Inst = HInst->getLLVMInstruction();

  if (!isa<BinaryOperator>(Inst)) {
    return;
  }

  bool IsFloat = HInst->getLvalDDRef()->getDestType()->isFPOrFPVectorTy();

  IsFloat ? ++LRInfo.FPOps : ++LRInfo.IntOps;
}

LoopResourceInfo &LoopResourceInfo::operator+=(const LoopResourceInfo &LRI) {
  IntOps += LRI.IntOps;
  FPOps += LRI.FPOps;
  IntMemReads += LRI.IntMemReads;
  IntMemWrites += LRI.IntMemWrites;
  FPMemReads += LRI.FPMemReads;
  FPMemWrites += LRI.FPMemWrites;

  return *this;
}

void LoopResourceInfo::classify() {

  unsigned MemRefsCost = getMemRefsCost();
  unsigned FPOpsCost = getFPOpsCost();

  if (MemRefsCost && (MemRefsCost >= getOpsCost())) {
    Bound = LoopResourceBound::Memory;

  } else if (FPOpsCost && (FPOpsCost >= getIntOpsCost())) {
    Bound = LoopResourceBound::FP;

  } else {
    Bound = LoopResourceBound::Int;
  }
}

void LoopResourceInfo::compute(HIRLoopResource &HLR, const HLLoop *Lp) {
  LoopResourceVisitor LRV(*this, HLR);

  // Do not directly recurse inside children loops. Loop resource is recursively
  // computed for children loops by the visitor using getLoopResource().
  HLNodeUtils::visitRange<true, false>(LRV, Lp->child_begin(), Lp->child_end());

  // Classify loop into Mem bound, FP bound or Int bound.
  classify();
}

void LoopResourceInfo::print(formatted_raw_ostream &OS, const HLLoop *Lp) const {

  // Indent at one level more than the loop nesting level.
  unsigned Depth = Lp->getNestingLevel() + 1;

  Lp->indent(OS, Depth);
  OS << "Integer Memory Reads: " << IntMemReads << "\n";
  Lp->indent(OS, Depth);
  OS << "Integer Memory Writes: " << IntMemWrites << "\n";
  Lp->indent(OS, Depth);
  OS << "Integer Operations: " << IntOps << "\n";
  Lp->indent(OS, Depth);
  OS << "Floating Point Reads: " << FPMemReads << "\n";
  Lp->indent(OS, Depth);
  OS << "Floating Point Writes: " << FPMemWrites << "\n";
  Lp->indent(OS, Depth);
  OS << "Floating Point Operations: " << FPOps << "\n";

  Lp->indent(OS, Depth);

  switch (Bound) {
  case LoopResourceBound::Memory:
    OS << "Memory Bound \n";
    break;
  case LoopResourceBound::Int:
    OS << "Integer Bound \n";
    break;
  case LoopResourceBound::FP:
    OS << "Floating Point Bound \n";
    break;
  case LoopResourceBound::Unknown:
    OS << "Unknown Bound \n";
    break;
  default:
    llvm_unreachable("Unexpected loop resource bound type!");
  }
}

const LoopResourceInfo &
HIRLoopResource::computeLoopResource(const HLLoop *Loop) {

  // Insert empty resource in cache.
  auto Pair = ResourceMap.insert(std::make_pair(Loop, LoopResourceInfo()));
  LoopResourceInfo &LRInfo = Pair.first->second;

  // Compute resource for this loop.
  LRInfo.compute(*this, Loop);

  return LRInfo;
}

const LoopResourceInfo &HIRLoopResource::getLoopResource(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LRInfoIt = ResourceMap.find(Loop);

  // Return cached resource, if present.
  if (LRInfoIt != ResourceMap.end()) {
    return LRInfoIt->second;
  }

  // Compute and return a new reource.
  return computeLoopResource(Loop);
}

void HIRLoopResource::print(formatted_raw_ostream &OS, const HLLoop *Lp) {
  const LoopResourceInfo &LRI = getLoopResource(Lp);
  LRI.print(OS, Lp);
}

void HIRLoopResource::markLoopBodyModified(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Remove the current and parent loops from the cache.
  while (Loop) {
    ResourceMap.erase(Loop);
    Loop = Loop->getParentLoop();
  }
}
