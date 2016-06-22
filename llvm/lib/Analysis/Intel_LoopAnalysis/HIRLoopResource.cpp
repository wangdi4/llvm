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

static cl::opt<bool> PrintTotalResource(
    "hir-print-total-resource", cl::init(false), cl::Hidden,
    cl::desc("Prints total loop resource instead of self loop resource"));

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

void HIRLoopResource::releaseMemory() {
  SelfResourceMap.clear();
  TotalResourceMap.clear();
}

struct LoopResourceInfo::LoopResourceVisitor final : public HLNodeVisitorBase {
  HIRLoopResource &HLR;
  const HLLoop *Lp;
  LoopResourceInfo &SelfLRI;
  LoopResourceInfo *TotalLRI;

  LoopResourceVisitor(HIRLoopResource &HLR, const HLLoop *Lp,
                      LoopResourceInfo &SelfLRI, LoopResourceInfo *TotalLRI)
      : HLR(HLR), Lp(Lp), SelfLRI(SelfLRI), TotalLRI(TotalLRI) {}

  // Main entry function to compute loop resource.
  void compute();

  // Accounts for 'num' integer or FP predicates in self resource.
  void addPredicateOps(unsigned Num, bool IsInt);

  // Ignore gotos/labels.
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool visit(const HLDDNode *Node);
  void visit(const HLInst *HInst);

  void visit(const HLIf *If);
  void visit(const HLLoop *Lp);
};

bool LoopResourceInfo::LoopResourceVisitor::visit(const HLDDNode *Node) {

  // Valid SelfLRI is available.
  if (!SelfLRI.isUnknownBound()) {
    return false;
  }

  for (auto RefIt = Node->op_ddref_begin(), E = Node->op_ddref_end();
       RefIt != E; ++RefIt) {
    auto Ref = *RefIt;

    if (!Ref->isMemRef()) {
      continue;
    }

    bool IsFloat = Ref->getDestType()->isFPOrFPVectorTy();

    if ((*RefIt)->isLval()) {
      IsFloat ? ++SelfLRI.FPMemWrites : ++SelfLRI.IntMemWrites;
    } else {
      IsFloat ? ++SelfLRI.FPMemReads : ++SelfLRI.IntMemReads;
    }
  }

  return true;
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLInst *HInst) {

  // Valid SelfLRI is available.
  if (!visit(cast<HLDDNode>(HInst))) {
    return;
  }

  auto Inst = HInst->getLLVMInstruction();

  if (!isa<BinaryOperator>(Inst)) {
    return;
  }

  bool IsFloat = HInst->getLvalDDRef()->getDestType()->isFPOrFPVectorTy();

  IsFloat ? ++SelfLRI.FPOps : ++SelfLRI.IntOps;
}

void LoopResourceInfo::LoopResourceVisitor::addPredicateOps(unsigned Num,
                                                           bool IsInt) {
  // Add number of logical operations as IntOps.
  SelfLRI.IntOps += Num - 1;

  // Add number of compares.
  if (IsInt) {
    SelfLRI.IntOps += Num;
  } else {
    SelfLRI.FPOps += Num;
  }
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLIf *If) {

  // Valid SelfLRI is available.
  if (!visit(cast<HLDDNode>(If))) {
    return;
  }

  // Capture the number of compare and logical operations.
  addPredicateOps(If->getNumPredicates(),
                 CmpInst::isIntPredicate(*If->pred_begin()));
}

void LoopResourceInfo::LoopResourceVisitor::visit(const HLLoop *Lp) {

  if (SelfLRI.isUnknownBound()) {
    // Add ztt predicates to self resource.
    if (Lp->hasZtt()) {
      addPredicateOps(Lp->getNumZttPredicates(),
                     CmpInst::isIntPredicate(*Lp->ztt_pred_begin()));
    }

    // Add child loop's preheader/postexit to parent loop's self resource.
    HLNodeUtils::visitRange(*this, Lp->pre_begin(), Lp->pre_end());
    HLNodeUtils::visitRange(*this, Lp->post_begin(), Lp->post_end());
  }

  // No need to process children loops in self-only mode.
  if (TotalLRI) {
    *TotalLRI += HLR.getTotalLoopResource(Lp);
  }
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

LoopResourceInfo &LoopResourceInfo::operator*=(unsigned Multiplier) {
  IntOps *= Multiplier;
  FPOps *= Multiplier;
  IntMemReads *= Multiplier;
  IntMemWrites *= Multiplier;
  FPMemReads *= Multiplier;
  FPMemWrites *= Multiplier;

  return *this;
}

void LoopResourceInfo::classify() {

  unsigned IntOpsCost = getIntOpsCost();
  unsigned FPOpsCost = getFPOpsCost();
  unsigned MemOpsCost = getMemOpsCost();

  if (MemOpsCost && (MemOpsCost >= FPOpsCost) && (MemOpsCost >= IntOpsCost)) {
    Bound = LoopResourceBound::Memory;

  } else if (FPOpsCost && (FPOpsCost >= IntOpsCost)) {
    Bound = LoopResourceBound::FP;

  } else {
    Bound = LoopResourceBound::Int;
  }
}

void LoopResourceInfo::LoopResourceVisitor::compute() {

  // Do not directly recurse inside children loops. Total resource is
  // recursively computed for children loops by the visitor using
  // getTotalLoopResource().
  HLNodeUtils::visitRange<true, false>(*this, Lp->child_begin(),
                                       Lp->child_end());

  // Classify self reource into Mem bound, FP bound or Int bound.
  if (SelfLRI.isUnknownBound()) {
    SelfLRI.classify();
  }

  // Add self reource to total resource and classify it.
  if (TotalLRI) {
    *TotalLRI += SelfLRI;
    TotalLRI->classify();
  }
}

void LoopResourceInfo::print(formatted_raw_ostream &OS,
                             const HLLoop *Lp) const {

  // Indent at one level more than the loop nesting level.
  unsigned Depth = Lp->getNestingLevel() + 1;

  if (IntMemReads) {
    Lp->indent(OS, Depth);
    OS << "Integer Memory Reads: " << IntMemReads << "\n";
  }
  if (IntMemWrites) {
    Lp->indent(OS, Depth);
    OS << "Integer Memory Writes: " << IntMemWrites << "\n";
  }
  if (IntOps) {
    Lp->indent(OS, Depth);
    OS << "Integer Operations: " << IntOps << "\n";
  }
  if (FPMemReads) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Reads: " << FPMemReads << "\n";
  }
  if (FPMemWrites) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Writes: " << FPMemWrites << "\n";
  }
  if (FPOps) {
    Lp->indent(OS, Depth);
    OS << "Floating Point Operations: " << FPOps << "\n";
  }

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

const LoopResourceInfo &HIRLoopResource::computeLoopResource(const HLLoop *Loop,
                                                             bool SelfOnly) {

  // These will be set below using the cache and SelfOnly paramter.
  LoopResourceInfo *TotalLRI = nullptr;

  // Get or Insert self resource.
  auto SelfPair =
      SelfResourceMap.insert(std::make_pair(Loop, LoopResourceInfo()));
  LoopResourceInfo &SelfLRI = SelfPair.first->second;

  if (!SelfOnly) {
    // Set TotalLRI to indicate that total resource need to be computed.
    auto TotalPair =
        TotalResourceMap.insert(std::make_pair(Loop, LoopResourceInfo()));
    TotalLRI = &TotalPair.first->second;
  }

  LoopResourceInfo::LoopResourceVisitor LRV(*this, Loop, SelfLRI, TotalLRI);

  LRV.compute();

  return SelfOnly ? SelfLRI : *TotalLRI;
}

const LoopResourceInfo &
HIRLoopResource::getSelfLoopResource(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LRInfoIt = SelfResourceMap.find(Loop);

  // Return cached resource, if present.
  if (LRInfoIt != SelfResourceMap.end()) {
    return LRInfoIt->second;
  }

  // Compute and return a new reource.
  return computeLoopResource(Loop, true);
}

const LoopResourceInfo &
HIRLoopResource::getTotalLoopResource(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  auto LRInfoIt = TotalResourceMap.find(Loop);

  // Return cached resource, if present.
  if (LRInfoIt != TotalResourceMap.end()) {
    return LRInfoIt->second;
  }

  // Compute and return a new reource.
  return computeLoopResource(Loop, false);
}

void HIRLoopResource::print(formatted_raw_ostream &OS, const HLLoop *Lp) {
  const LoopResourceInfo &LRI =
      PrintTotalResource ? getTotalLoopResource(Lp) : getSelfLoopResource(Lp);
  LRI.print(OS, Lp);
}

void HIRLoopResource::markLoopBodyModified(const HLLoop *Loop) {
  assert(Loop && " Loop parameter is null.");

  // Remove current loop's self resource from the cache.
  SelfResourceMap.erase(Loop);

  // Remove current and parent loops total resouce from the cache.
  while (Loop) {
    TotalResourceMap.erase(Loop);
    Loop = Loop->getParentLoop();
  }
}
