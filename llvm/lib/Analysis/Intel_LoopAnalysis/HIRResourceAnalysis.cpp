//===--------- HIRResourceAnalysis.cpp - Computes loop resources ----------===//
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
// 6. CallInstructions might need special handling based on arguments.

#include "llvm/Analysis/Intel_LoopAnalysis/HIRResourceAnalysis.h"

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

#define DETAILED_LOOPRESOURCE 1

// Primarily used for verification purpose for lit tests.
// We do not print any information except DEBUG mode.
static cl::opt<bool> verifyLoopResource(
    "hir-verify-loop-resource", cl::init(false), cl::Hidden,
    cl::desc("Pre-computes loop resource for all loops inside function."));

FunctionPass *llvm::createHIRResourceAnalysisPass() {
  return new HIRResourceAnalysis();
}

char HIRResourceAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(HIRResourceAnalysis, "hir-loop-resource",
                      "Loop Resource Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRResourceAnalysis, "hir-loop-resource",
                    "Loop Resource Analysis", false, true)

void HIRResourceAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRFramework>();
}

// Performs a basic setup without actually running the loop resource
// analysis.
bool HIRResourceAnalysis::runOnFunction(Function &F) {

  // This is an on-demand analysis, so we don't perform
  // any analysis here unless for verification purpose.
  if (verifyLoopResource) {
    verifyAllLoops();
  }
  return false;
}

// Only used when verifyLoopResource is on.
void HIRResourceAnalysis::verifyAllLoops() {

  // For any changes, please run the llvm-lit tests
  // to check for any breakage.
  SmallVector<const HLLoop *, 16> Loops;
  HLNodeUtils::gatherOutermostLoops(Loops);
  for (auto &I : Loops) {
    getLoopResource(I);
  }
}

void HIRResourceAnalysis::releaseMemory() {

  for (auto &I : ResourceMap) {
    delete I.second;
  }
  ResourceMap.clear();
  LoopModificationMap.clear();
}

bool HIRResourceAnalysis::isLoopModified(const HLLoop *Loop) const {

  assert(Loop && " Loop parameter is null.");

  // Checks if status exist and returns the status.
  auto Iter = LoopModificationMap.find(Loop);
  if (Iter != LoopModificationMap.end()) {
    bool Status = Iter->second;
    assert((!Status || (Status && ResourceMap.count(Loop))) &&
           " Loop resource information not found for unmodified loop.");
    return Status;
  }

  // Default is to return true.
  return true;
}

// Clears out the resource map information if it exists, otherwise
// it creates a new one for first time visits to the loop. We do not deallocate
// the loop resource if it is  modified since this might too many allocation and
// deallocation across the transformations.
LoopResourceInfo *HIRResourceAnalysis::resetResourceMap(const HLLoop *Loop) {

  LoopResourceInfo *LRInfo = nullptr;
  auto Iter = ResourceMap.find(Loop);
  if (Iter != ResourceMap.end()) {
    // Clear existing info.
    LRInfo = Iter->second;
    LRInfo->clear();
  } else {
    // Create a new info.
    LRInfo = new LoopResourceInfo();
    ResourceMap[Loop] = LRInfo;
  }

  return LRInfo;
}

int64_t HIRResourceAnalysis::getTripCount(const HLLoop *Loop) {

  // TODO: Add support when getEstimatedTripCount is available.
  int64_t TripCnt = 0;
  if (Loop->isConstTripLoop(&TripCnt)) {
    return std::min(TripCnt, (int64_t)SymbolicConst);
  }

  // If Trip count is non-constant, it indicates 'N' trip count.
  return SymbolicConst;
}

void HIRResourceAnalysis::addLoopResource(const HLLoop *ParentLoop,
                                           const HLLoop *ChildLoop) {
  assert((ResourceMap.count(ParentLoop) > 0) &&
         " Parent Loop Resource Info not found.");
  assert((ResourceMap.count(ChildLoop) > 0) &&
         " Parent Loop Resource Info not found.");
  LoopResourceInfo *ParLRInfo = ResourceMap[ParentLoop];
  ParLRInfo->add(*ResourceMap[ChildLoop]);
}

// Visit the instructions inside the loops.
struct HIRResourceAnalysis::LoopVisitor final : public HLNodeVisitorBase {

  LoopResourceInfo *LRInfo;
  HIRResourceAnalysis *LRAnalysisPtr;

  void visit(const HLInst *Inst) {
    DEBUG(dbgs() << " Visiting Inst:\n");
    DEBUG(Inst->dump());
    LRAnalysisPtr->processInstruction(Inst, LRInfo);
  }

  void visit(const HLIf *If) {
    // We capture only the number of logical operations here
    // and do not traverse inside the predicates.
    uint64_t Num = If->getNumPredicates();
    if (Num != 0) {
      LRInfo->IntOps += ((Num - 1) * LRAnalysisPtr->IntOpCost);
    }
  }

  // Child loops are visited as they are computed separately.
  // For switch, we do not do any special processing.
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
  bool isDone() const override { return false; }
  LoopVisitor(LoopResourceInfo *LR, HIRResourceAnalysis *LRA)
      : LRInfo(LR), LRAnalysisPtr(LRA) {}
};

bool HIRResourceAnalysis::isFloatTy(const Type *Ty) {
  return Ty->isFloatingPointTy();
}

bool HIRResourceAnalysis::isIntTy(const Type *Ty) { return Ty->isIntegerTy(); }

bool HIRResourceAnalysis::isTypeSupported(const Type *Ty) {
  if (Ty->isFloatingPointTy() || Ty->isIntegerTy()) {
    return true;
  }
  return false;
}

void HIRResourceAnalysis::processCanonExpr(const CanonExpr *CE,
                                            LoopResourceInfo *LRInfo) {

  // Don't process for any constant
  if (CE->isConstant()) {
    return;
  }

  if (!isTypeSupported(CE->getSrcType())) {
    return;
  }

  // Determine if this is integer or float operations.
  // There cannot be mixed types in a single canon expr. The mixed type
  // computation would be split into separate instructions
  // e.g. t = I1 + I2 + FP1, here I1+I2 would be a separate instruction.
  bool IsIntOp = (CE->hasIV() || isIntTy(CE->getSrcType()));

  // Count the number of operations. Ignoring the multiplication inside the
  // canon expr as it would create large number of operations.
  uint64_t NumOps =
      CE->numIVs() + CE->numBlobs() - 1 + (CE->getConstant() != 0);

  // Count the division
  NumOps += (CE->getDenominator() != 1) ? 1 : 0;

  IsIntOp ? LRInfo->IntOps += (NumOps * IntOpCost) : LRInfo->FPOps +=
                                                     (NumOps * FPOpCost);
}

void HIRResourceAnalysis::processInstruction(const HLInst *HLInst,
                                              LoopResourceInfo *LRInfo) {

  auto LLVMInst = HLInst->getLLVMInstruction();

  // Only handle Load, Store and Binary instructions.
  // Ignore cast and other instructions.
  /*if(!isa<StoreInst>(LLVMInst) && !isa<LoadInst>(LLVMInst) &&
  !isa<BinaryOperator>(LLVMInst)) {
          DEBUG(dbgs() << " Not a load/store/binary operator.\n");
          return;
  }*/

  const RegDDRef *LVal = HLInst->getLvalDDRef();
  assert(LVal && " LVal doesn't exist");
  Type *LValDestTy = LVal->getDestType();

  // Handle only Int and FP Type.
  if (!isTypeSupported(LValDestTy)) {
    return;
  }

  bool isFloat = isFloatTy(LValDestTy);

  if (isa<StoreInst>(LLVMInst)) {
    isFloat ? LRInfo->FPMemWrites += FPMemWriteCost : LRInfo->IntMemWrites +=
                                                      IntMemWriteCost;
  } else if (isa<LoadInst>(LLVMInst)) {
    isFloat ? LRInfo->FPMemReads += FPMemReadCost : LRInfo->IntMemReads +=
                                                    IntMemReadCost;
  } else if (isa<BinaryOperator>(LLVMInst)) {
    isFloat ? LRInfo->FPOps += FPOpCost : LRInfo->IntOps += IntOpCost;
  } else {
    // We process the DDRefs.
    // This captures cases such as %t = sitofp.i64.float(i1 + 2); A[i] = %t.
  }

  for (auto RegIter = HLInst->op_ddref_begin(),
            EndIter = HLInst->op_ddref_end();
       RegIter != EndIter; ++RegIter) {
    const RegDDRef *DD = *RegIter;
    // Ignore Lval.
    if (DD->isLval() || !DD->isSingleCanonExpr() || DD->isMemRef()) {
      continue;
    }
    // This is used to compute operations for cases, such
    // as %t1 = (%8 + %9)/2
    for (auto CEIter = DD->canon_begin(), CEEnd = DD->canon_end();
         CEIter != CEEnd; ++CEIter) {
      const CanonExpr *CE = *CEIter;
      processCanonExpr(CE, LRInfo);
    }
  }
}

void HIRResourceAnalysis::classifyLoopResource(LoopResourceInfo *LRInfo) {

  // Zero condition for empty loops.
  bool ZeroCheck =
      (LRInfo->getNumOps() == 0) && (LRInfo->getNumMemoryRefs() == 0);
  assert(!ZeroCheck && " Loop resource with zero values.");
  if (ZeroCheck) {
    LRInfo->Bound = LoopResourceBound::Unknown;
    return;
  }

  // Memory Reads/Writs vs. ( FPOps + IntOps )
  if (LRInfo->getNumMemoryRefs() > LRInfo->getNumOps()) {
    LRInfo->Bound = LoopResourceBound::Memory;
    return;
  }

  // FP vs. Int
  if (LRInfo->FPOps > LRInfo->IntOps) {
    LRInfo->Bound = LoopResourceBound::FP;
  } else {
    LRInfo->Bound = LoopResourceBound::Int;
  }
}

void HIRResourceAnalysis::computeLoopResource(const HLLoop *Loop) {

  assert(Loop && " Loop parameter is null.");

  // Early exit if loop resource computation is not needed.
  if (!isLoopModified(Loop)) {
    return;
  }

  // Create/reset loop resource info.
  LoopResourceInfo *LRInfo = resetResourceMap(Loop);
  assert(LRInfo && " Loop Resource Info is null.");

  // Empty loops. All LRInfo values would be 0.
  if (!Loop->hasChildren()) {
    return;
  }

  // Visit the loop without its children.
  LoopVisitor LVisit(LRInfo, this);
  HLNodeUtils::visitRange<true, false>(LVisit, Loop->getFirstChild(),
                                       Loop->getLastChild());

  // Traverse the child loops.
  // Collect all the immediate child loops inside the loop nest and visit them.
  SmallVector<const HLLoop *, 16> ChildLoops;
  HLNodeUtils::gatherLoopsWithLevel(Loop, ChildLoops,
                                    Loop->getNestingLevel() + 1);
  for (auto Iter = ChildLoops.begin(), End = ChildLoops.end(); Iter != End;
       ++Iter) {

    const HLLoop *CurLoop = *Iter;
    computeLoopResource(CurLoop);

    // Add it to the current Loop resource info.
    addLoopResource(Loop, CurLoop);
  }

  DEBUG(dbgs() << "Before Multiply \n");
  DEBUG(LRInfo->dump());
  // Multiply by the trip count.
  LRInfo->multiply(getTripCount(Loop));

  // Preheader and post exit.
  if (Loop->hasPreheader()) {
    HLNodeUtils::visitRange<true, false>(LVisit, Loop->getFirstPreheaderNode(),
                                         Loop->getLastPreheaderNode());
  }
  if (Loop->hasPostexit()) {
    HLNodeUtils::visitRange<true, false>(LVisit, Loop->getFirstPostexitNode(),
                                         Loop->getLastPostexitNode());
  }

  DEBUG(LRInfo->dump());

  // Classify loop into FP Bound, Mem Bound or Int Bound.
  classifyLoopResource(LRInfo);
}

const LoopResourceInfo *
HIRResourceAnalysis::getLoopResource(const HLLoop *Loop) {

  assert(Loop && " Loop parameter is null.");

  // Compute the loop resource if necessary.
  computeLoopResource(Loop);

  return ResourceMap[Loop];
}

void HIRResourceAnalysis::print(raw_ostream &OS, const Module *M) const {

// Only used for verification in lit test in debug mode.
#ifndef NDEBUG
  formatted_raw_ostream FOS(OS);
  FOS << "Loop Resource Information for all loops(sorted based on loop "
         "number):\n";
  SmallVector<const HLLoop *, 16> Loops;
  for (auto &Iter : ResourceMap) {
    Loops.push_back(Iter.first);
  }
  struct {
    bool operator()(const HLLoop *Loop1, const HLLoop *Loop2) {
      return Loop1->getNumber() < Loop2->getNumber();
    }
  } compareLevel;
  std::sort(Loops.begin(), Loops.end(), compareLevel);
  for (auto &Iter : Loops) {
    const HLLoop *Loop = Iter;
    FOS << "Loop Number: " << Loop->getNumber()
        << " Level: " << Loop->getNestingLevel() << "\n";
    DEBUG(Loop->dump(true));
    const LoopResourceInfo *LRInfo = ResourceMap.find(Loop)->second;
    LRInfo->print(FOS);
  }
#endif
}

void HIRResourceAnalysis::markLoopBodyModified(const HLLoop *Loop) {

  assert(Loop && " Loop parameter is null.");

  // Mark loop as modified.
  LoopModificationMap[Loop] = true;

  // Mark all the parents as modified.
  const HLLoop *CurLoop = Loop->getParentLoop();
  while (CurLoop) {
    LoopModificationMap[CurLoop] = true;
    CurLoop = CurLoop->getParentLoop();
  }
}
