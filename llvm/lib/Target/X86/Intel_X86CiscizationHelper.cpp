//====-- Intel_X86CiscizationHelper.cpp ----------------====
//
//      Copyright (c) 2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines the pass which perform below transform to help ciscization.
// So that ISel will choose 1 RMW instruction instead of 3 instructions for
// read, modification and write operations in BB1 and BB3 for better code size.

//  BB1:                                                   BB1:
//  %v0 = load %addr                                       %v0 = load %addr
//  %v1 = op %v0, #cv                                      %v1 = op %v0, #cv
//  store %v1, %addr                                       store %v1, %addr
//     /   \                                                  /   \
//    /     \                                                /     \
//   |       BB*        ==== [ Ciscization ] ==== >         |       BB*
//   |       |                                              |       |
//   |       BB2:                                           |       BB2
//   \       %v2 = load %addr                               \       |
//    \     /                                                \     /
//     \   /                                                  \   /
//      BB3:                                                   BB3:
//      %pn = phi i32 [ %v1, %BB1 ], [ %v2, %BB2 ]             %v2 = load %addr
//      %v3 = op %pn, #cv                                      v3 = op %v2, #cv
//      store %v3, %addr                                       store %v3, %addr
//

#include "X86.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/PatternMatch.h"

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "x86-ciscization-helper"

STATISTIC(NumCiscInst, "Number of Ciscization instructions");

static cl::opt<unsigned> CiscHelperBBInstNumThreshold(
    "x86-ciscization-helper-bb-inst-number-threshold", cl::Hidden, cl::init(4),
    cl::desc("Basic Block Instruction Number Threshold for X86 Ciscization Helper"));
static cl::opt<unsigned> CiscHelperSearchDistanceThreshold(
    "x86-ciscization-helper-search-distance-threshold", cl::Hidden, cl::init(16),
    cl::desc("Search Distance Threshold for X86 Ciscization Helper"));

namespace {

class X86CiscizationHelperPass : public FunctionPass {
public:
  struct Candidate {
    PHINode *PN;
    LoadInst *Inst2;
    Instruction *Inst3;
    StoreInst *SI;
    Value *Addr;
    Candidate(PHINode *P, LoadInst *I2, Instruction *I3, StoreInst *S,
              Value *A) : PN(P), Inst2(I2), Inst3(I3), SI(S), Addr(A) {}
  };

  static char ID; // Pass identification, replacement for typeid.

  X86CiscizationHelperPass() : FunctionPass(ID), AA(nullptr) { }

  bool runOnFunction(Function &Fn) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  StringRef getPassName() const override {
    return "X86 Ciscization Helper";
  }

private:

  SmallVector<Candidate, 10> Cands;
  AAResults *AA;

  bool accessIdenticalAddr(LoadInst *LI, StoreInst *SI);
  bool accessIdenticalAddr(StoreInst *SI1, StoreInst *SI2);

  bool canPathModify(const BasicBlock *TopBB, const BasicBlock *CurBB,
                     const MemoryLocation &Loc, unsigned depth,
                     SmallPtrSetImpl<const BasicBlock *> &VisitedBlocks);

  LoadInst * isCandBB1BB2(PHINode *PN, StoreInst *SI);

  bool helpCiscization(BasicBlock *BB);

  // Return true if it is possible for any instruction in the range (I, E)
  // INCLUSIVE to store value to the same address as SI.
  bool canStoreAddr(const Instruction &I, const Instruction &E,
                    StoreInst *SI) {
    MemoryLocation Loc = MemoryLocation::get(SI);
    return AA->canInstructionRangeModRef(I, E, Loc, ModRefInfo::Mod);
  }
};
} // end anonymous namespace

char X86CiscizationHelperPass::ID = 0;

INITIALIZE_PASS_BEGIN(X86CiscizationHelperPass, "x86-ciscization",
                      "Insert load for ciscization", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(X86CiscizationHelperPass, "x86-ciscization",
                    "Insert load for ciscization", false, false)

FunctionPass *llvm::createX86CiscizationHelperPass() {
  return new X86CiscizationHelperPass();
}

//===----------------------------------------------------------------------===//
// Matchers for Cisc operators.
//

template <typename LHS_t, typename RHS_t>
struct CiscOp_match {
  LHS_t L;
  RHS_t R;

  CiscOp_match(const LHS_t &LHS, const RHS_t &RHS) : L(LHS), R(RHS) {}

  template <typename OpTy> bool match(OpTy *V) {
    if (Instruction *Inst = dyn_cast<Instruction>(V)) {
      switch (Inst->getOpcode()) {
        case Instruction::Add:
        case Instruction::Sub:
          return L.match(Inst->getOperand(0)) && R.match(Inst->getOperand(1));
        default:
          return false;
      }
      return false;
    }
    return false;
  }
};

template <typename LHS, typename RHS>
inline CiscOp_match<LHS, RHS> m_Cisc(const LHS &L, const RHS &R) {
  return CiscOp_match<LHS, RHS>(L, R);
}

void X86CiscizationHelperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AAResultsWrapperPass>();
  // This pass should only move load instructions
  AU.setPreservesCFG();
}

bool X86CiscizationHelperPass::runOnFunction(Function &F) {
  if (!F.hasFnAttribute("contains-rec-pro-clone"))
    return false;

  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  bool changed = false;

  for (BasicBlock &BB : F) {
    changed |= helpCiscization(&BB);
  }

  return changed;
}

bool X86CiscizationHelperPass::accessIdenticalAddr(LoadInst *LI,
                                                   StoreInst *SI) {
  MemoryLocation Loc = MemoryLocation::get(SI);
  if (isRefSet(AA->getModRefInfo(LI, Loc)))
    return true;

  return false;
}

bool X86CiscizationHelperPass::accessIdenticalAddr(StoreInst *SI1,
                                                   StoreInst *SI2) {
  MemoryLocation Loc = MemoryLocation::get(SI2);
  if (isModSet(AA->getModRefInfo(SI1, Loc)))
    return true;

  return false;
}

// TopBB: the block in which a value is created or loaded.
// CurBB: incoming block for a PHI node where the value is referenced.
//
// The function check all basic blocks in the control flow from TopBB to CurBB
// (searching upward from the bottom) for blocks that might modify the
// specified memory location (Loc). TopBB itself will not be checked.
//
// The function calls itself recursively, updating CurBB as it traverses up the
// call graph, but TopBB remains the same at all levels. The original caller
// will check for memory accesses in TopBB below the value of interest and in
// the block containing the PHI node (curBB immediate successor). The original
// caller should pass in a depth of zero and an empty visited blocks set.
bool X86CiscizationHelperPass::canPathModify(const BasicBlock *TopBB,
    const BasicBlock *CurBB, const MemoryLocation &Loc, unsigned depth,
    SmallPtrSetImpl<const BasicBlock *> &VisitedBlocks) {
  // Conservatively assume alias store if search depth exceeds threshold
  if (depth > CiscHelperSearchDistanceThreshold)
    return true;

  // If we've reach TopBB or we've looked at this block before there are
  // no modification on this part of the path.
  if ((CurBB == TopBB) || !VisitedBlocks.insert(CurBB).second)
    return false;

  // If this block can modify the memory, we have the final answer.
  if (AA->canBasicBlockModify(*CurBB, Loc))
    return true;

  // Search backwards since we know that every path starting from the original
  // PHI node's incoming block (CurBB) will reach TopBB.
  for (const BasicBlock *PredBB : make_range(pred_begin(CurBB), pred_end(CurBB)))
    if (canPathModify(TopBB, PredBB, Loc, depth + 1, VisitedBlocks))
      return true;

  // We didn't find any modifications in this path.
  return false;
}

// Check candidates: BB1 and BB2 for following patterns
//
//   BB1:
//   %v0 = load %addr1
//   %v1 = op %v0, #cv
//   store %v1, %addr
//      /   \
//     /     \
//    |       BB*
//    |       |
//    |       BB2:
//    \       %v2 = load %addr
//     \     /
//      \   /
//       BB3:
//       %pn = phi i32 [ %v1, %BB1 ], [ %v2, %BB2 ]
//       %v3 = op %pn, #cv
//       store %v3, %addr3
//
//   parameters:
//     PN: %pn = phi i32 [ %v1, %BB1 ], [ %v2, %BB2 ]
//     SI: store %v3, %addr
//   Return nullptr if the patterns don't match
//   Return Inst2 (%v2) if the patterns match
//
LoadInst * X86CiscizationHelperPass::isCandBB1BB2(PHINode *PN, StoreInst *SI) {
  Value *V1 = nullptr;
  BasicBlock *PredBB1 = nullptr;
  Value *V2 = PN->getIncomingValue(1);
  BasicBlock *PredBB2 = PN->getIncomingBlock(1);
  LoadInst *Inst2 = dyn_cast<LoadInst>(V2);
  if (!Inst2) {
    V1 = V2;
    PredBB1 = PredBB2;
    V2 = PN->getIncomingValue(0);
    PredBB2 = PN->getIncomingBlock(0);
    Inst2 = dyn_cast<LoadInst>(V2);
    if (!Inst2 || !Inst2->hasOneUse() || !Inst2->isSimple() ||
        !accessIdenticalAddr(Inst2, SI))
      return nullptr;
  } else if (!Inst2->hasOneUse() || !Inst2->isSimple() ||
             !accessIdenticalAddr(Inst2, SI)) {
      return nullptr;
  } else {
    V1 = PN->getIncomingValue(0);
    PredBB1 = PN->getIncomingBlock(0);
  }

  // Make sure no other stores to the same address after Inst2 in BB2
  BasicBlock *BB2 = Inst2->getParent();
  BasicBlock::iterator II = Inst2->getIterator();
  if (canStoreAddr(*(++II), BB2->back(), SI))
    return nullptr;

  // Make sure no other stores to the address on the path between BB2 and BB3
  MemoryLocation Loc = MemoryLocation::get(SI);
  SmallPtrSet<const BasicBlock *, 32> VisitedBlocks;
  if (canPathModify(BB2, PredBB2, Loc, 0, VisitedBlocks))
    return nullptr;

  Instruction *Inst1 = dyn_cast<Instruction>(V1);
  if (!Inst1)
    return nullptr;

  Value *V0;
  if (!Inst1->hasNUses(2) ||
      !match(Inst1, m_Cisc(m_Value(V0), m_ConstantInt())))
    return nullptr;

  BasicBlock *BB1 = Inst1->getParent();
  Value::user_iterator I = Inst1->user_begin();
  if (*I == PN)
    ++I;

  StoreInst *SI1 = dyn_cast<StoreInst>(*I);
  if (!SI1 || SI1->getParent() != BB1 || SI1->getOperand(0) != Inst1 ||
      !SI->isSimple() || !accessIdenticalAddr(SI1, SI))
    return nullptr;

  LoadInst *LI1 = dyn_cast<LoadInst>(V0);
  if (!LI1 || !LI1->hasOneUse() || LI1->getParent() != BB1 ||
      !LI1->isSimple() || !accessIdenticalAddr(LI1, SI))
    return nullptr;

  // Make sure no other stores to the same address after SI1 in BB1
  II = SI1->getIterator();
  if (canStoreAddr(*(++II), BB1->back(), SI))
    return nullptr;

  // Make sure no other stores to the address on the path between BB1 and BB3
  VisitedBlocks.clear();
  if (canPathModify(BB1, PredBB1, Loc, 0, VisitedBlocks))
    return nullptr;

  return Inst2;
}

bool X86CiscizationHelperPass::helpCiscization(BasicBlock *BB) {
  assert(Cands.empty());
  // Loop over all of the PHI nodes, looking for candidates.
  for (BasicBlock::iterator I = BB->begin(); isa<PHINode>(I); ++I) {
    // Search for following candidate pattern:
    //
    //       BB3:
    //       %pn = phi i32 [ %v1, %BB1 ], [ %v2, %BB2 ]
    //       %v3 = op %pn, #cv
    //       store %v3, %addr

    PHINode *PN = cast<PHINode>(I);
    if (PN->getNumOperands() != 2)
      continue;

    if (!PN->hasOneUse())
      continue;

    Instruction *Inst3 = dyn_cast<Instruction>(*(PN->user_begin()));
    if (!Inst3 || Inst3->getParent() != BB || Inst3->getOperand(0) != PN)
      continue;

    if (!match(Inst3, m_OneUse(m_Cisc(m_Specific(PN), m_ConstantInt()))))
      continue;

    StoreInst *SI = dyn_cast<StoreInst>(*(Inst3->user_begin()));
    if (!SI || SI->getParent() != BB || SI->getOperand(0) != Inst3 ||
        !SI->isSimple())
      continue;

    // Make sure no other stores to the same address in current BB before Inst3
    BasicBlock::reverse_iterator IRI = Inst3->getReverseIterator();
    if (canStoreAddr(BB->front(), *(++IRI), SI))
      continue;

    LoadInst *Inst2 = isCandBB1BB2(PN, SI);

    if (!Inst2)
      continue;

    Cands.push_back(Candidate(PN, Inst2, Inst3, SI, SI->getOperand(1)));
  }

  if (Cands.size() < CiscHelperBBInstNumThreshold) {
    Cands.clear();
    return false;
  }

  while (!Cands.empty()) {
    Candidate Cand = Cands.back();
    Cands.pop_back();
    // Adjust BB3 instructions to get following instructions for Ciscization:
    //
    //       BB3:
    //       %v2 = load %addr
    //       %v3 = op %v2, #cv
    //       store %v3, %addr
    //
    // Make sure address calculation is done before Inst3
    Cand.Inst3->moveBefore(Cand.SI);
    // Move Inst2 from BB2 to BB3 and put it just before Inst3
    Cand.Inst2->moveBefore(Cand.Inst3);
    LLVM_DEBUG(dbgs() << "Move one load instruction: " <<
               *(Cand.Inst2) <<"\n");

    Cand.Inst3->setOperand(0, Cand.Inst2);
    Cand.Inst2->setOperand(0, Cand.Addr);
    Cand.PN->eraseFromParent();

    ++NumCiscInst;
  }

  LLVM_DEBUG(dbgs() << "Ciscization for one BB:\n" << *BB);

  return true;
}
