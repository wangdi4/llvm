//===-- IVSplit.cpp - IV Split -------------------------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass splits IV live ranges which span a lot of nesting inner loops.
// It can decrease register pressure of inner loops and void register allocator
// generating bad split/eviction/reload code.
//
//   DO I1                                            DO I1
//     ... = I1                                         ... = I1
//                                                      [spill I1]
//     DO I2                                            DO I2
//       ... = I2                                         ... = I2
//                                                        [spill I2]
//       DO I3          [ Insert spill/reload  ]          DO I3
//         ...     ===> [ to split each IV to  ] ===>       ...
//       END DO         [ disjoint live ranges ]          END DO
//                                                        [reload I2]
//       ... = I2                                         ... = I2
//     END DO                                           END DO
//                                                      I1_r = [reload I1]
//                                                      I1' = Ø(I1, I1_r)
//     ... = I1                                         ... = I1'
//   END DO                                           END DO
//
//   Original IV (I1) and reload IV (I1_r) are merged to a new IV (I1') for
//   use with dominance frontier logic.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/IVSplit.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/IteratedDominanceFrontier.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace PatternMatch;

#define DEBUG_TYPE "iv-split"

STATISTIC(NumIVSplit, "Number of IV split");

static cl::opt<unsigned> IVSplitLoopDepthThreshold(
    "iv-split-loop-depth-threshold", cl::Hidden, cl::init(8),
    cl::desc("Loop depth threshold for enabling IV Split"));

namespace {

  class IVSplit {

    Function *FN;
    DominatorTree *DT;
    LoopInfo *LI;

    SmallVector<PHINode *, 2> IVs;
    SmallVector<Value *, 2> Allocs;
    SmallVector<Value *, 2> Reloads;
    SmallVector<Value *, 2> ReDefs;

    BasicBlock *SpillBB;

    BasicBlock *ReloadBB;
    BasicBlock *ReloadFrom;
    BasicBlock *ReloadTo;

    unsigned CurDepth;

  public:
    IVSplit(Function *Fn, DominatorTree *Dt, LoopInfo *Li)
        : FN(Fn), DT(Dt), LI(Li), SpillBB(nullptr), ReloadBB(nullptr),
        ReloadFrom(nullptr), ReloadTo(nullptr), CurDepth(0) {
    }

    bool hasLoopSplitInductionVariables(Loop * L);

    bool isIVSplitIdealLoop(Loop * L);

    void spillIV(Loop * L, IRBuilder<> &Builder);

    void reloadIV(Loop * L, IRBuilder<> &Builder);

    void updateIVUser(Loop * L, IRBuilder<> &Builder);

    void loopIVComputationSink();

    bool loopIVSplitRecursion(Loop * L);

    unsigned maxLoopDepth(Loop * L) {
      unsigned MaxDepth = 0;
      if (L->empty())
        return 1;
      for (auto *SubLoop : *L) {
        unsigned Depth = maxLoopDepth(SubLoop);
        if (Depth > MaxDepth)
          MaxDepth = Depth;
      }
      return MaxDepth + 1;
    }

    // Split IV live ranges spanning across a lot of inner nesting loops.
    // It can decrease register pressure and void register allocator generating
    // bad split/eviction code.
    bool loopIVSplit(Loop * L) {
        if(maxLoopDepth(L) < IVSplitLoopDepthThreshold)
          return false;

        auto result = loopIVSplitRecursion(L);

        return result;
    }

    bool performLoopIVSplit() {
      bool Changed = false;
      for(auto *Lp: *LI)
        Changed |= loopIVSplit(Lp);

      return Changed;
    }

  };

} // end anonymous namespace

bool IVSplit::hasLoopSplitInductionVariables(Loop * L) {

  BasicBlock *H = L->getHeader();
  BasicBlock *Incoming = nullptr, *Backedge = nullptr;
  if (!L->getIncomingAndBackEdge(Incoming, Backedge))
    return 0;

  int NumPN = 0;
  // Loop over all of the PHI nodes, looking for interested indvar.
  for (BasicBlock::iterator I = H->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    if (Instruction *Inc =
          dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge)))
      if (Inc->getOpcode() == Instruction::Add && Inc->getOperand(0) == PN)
        if (ConstantInt *CI = dyn_cast<ConstantInt>(Inc->getOperand(1)))
          if (CI->isOne() || CI->isMinusOne()) {
            // It's ok that the IV has uses from inner Loops
            IVs.push_back(PN);
            NumPN++;
          }
  }
  return NumPN > 0;
}

bool IVSplit::isIVSplitIdealLoop(Loop * L) {

  if (!hasLoopSplitInductionVariables(L))
    return false;

  // Don't handle the case where loop has multiple subloops or no subloops
  if (std::distance(L->begin(), L->end()) != 1) {
    return false;
  }

  SpillBB = (*L->begin())->getLoopPreheader();

  if (!SpillBB)
    return false;

  CurDepth = L->getLoopDepth();

  ReloadFrom = nullptr;
  ReloadTo = nullptr;

  // Note: We can't use "(*L->begin())->getExitingBlock()" and
  // "(*L->begin())->getExitBlock()" here since there are many
  // exiting Blocks in the nesting subloops (consider the "return").
  // What we care about is just outing edge from inner loop to current
  // loop instead of to outside current loop where "return" will jump to.
  SmallVector<std::pair<BasicBlock *, BasicBlock *>, 8> ExitEdges;
  (*L->begin())->getExitEdges(ExitEdges);
  for (auto &ExitEdge : ExitEdges) {
    if(L->contains(ExitEdge.second)) {
      // Only handle case with one edge into outer loop
      if(ReloadFrom)
        return false;

      ReloadFrom = const_cast<BasicBlock *>(ExitEdge.first);
      ReloadTo = const_cast<BasicBlock *>(ExitEdge.second);
    } else {
      auto TermBB = ExitEdge.second;

      // There can be cases where it exits from the parent loop but not out
      // of the function. For example, in following case it exits out of i3
      // and i2 loop to the i1 loop:
      //
      // DO i1
      //  DO i2
      //    DO i3
      //      if ()
      //        goto L;
      //      }
      //    END DO
      //  END DO
      // L:
      // END DO
      //
      // For simplicity, just bail out if exit to some loop other than L.
      if (LI->getLoopFor(TermBB))
        return false;
    }
  }

  if (!ReloadFrom)
    return false;

  // For simplicity, only handle situation with one edge into inner loop
  // and one edge out inner loop
  return true;
}

void IVSplit::spillIV(Loop * L, IRBuilder<> &Builder) {
  assert (SpillBB);
  // Spill as early as possible
  Instruction *InsertPt = &*(SpillBB->getFirstInsertionPt());
  Builder.SetInsertPoint(InsertPt);
  // Insert IVs spilling
  for (size_t I = 0, E = IVs.size(); I < E; I++) {
    Builder.CreateStore(IVs[I], Allocs[I]);
    LLVM_DEBUG(dbgs() << "Spill IV " << IVs[I]->getName() << "\n");
  }
  LLVM_DEBUG(dbgs() << *SpillBB << "\n");

  // Replace user of IVs in BB dominated by SpillBB with reload
  for (size_t I = 0, E = IVs.size(); I < E; I++) {
    Value::user_iterator IUser = IVs[I]->user_begin();
    Value::user_iterator IEnd = IVs[I]->user_end();
    while (IUser != IEnd) {
      Instruction *Inst = cast<Instruction>(*IUser);
      Use &IVUse = IUser.getUse();
      IUser++;
      auto UserBB = Inst->getParent();
      if((*L->begin())->contains(UserBB)) {
        if (isa<PHINode>(Inst) && (UserBB == (*L->begin())->getHeader())) {
          /* Use in loop header phi is as good as use outside the loop so we
             can ignore it. One example is as below:

            LOOP57:
              %1916 = phi i32 [ %2452, %2449 ], [ %1914, %1Inst5 ]
              %1918 = phi i64 [ %1Inst6, %1Inst5 ], [ %2450, %2449 ]

            LOOP58:
              %1928 = phi i32 [ %2391, %2388 ], [ %1916, %1924 ]

           */
          continue;
        }
        // A single load in the loop preheader will increase register pressure
        // for the inner loops and reload in the inner loops is better.
        Builder.SetInsertPoint(Inst);
        auto reload = Builder.CreateLoad(Allocs[I], "iv-inner-reload-var");
        IVUse.set(reload);
        LLVM_DEBUG(dbgs() << "Update inner Loop IV user in Inst: ");
        LLVM_DEBUG(dbgs() << *Inst << "\n" << *UserBB << "\n");
      }
    }
  }
}

void IVSplit::reloadIV(Loop * L, IRBuilder<> &Builder) {
  assert(ReloadFrom);
  ReloadBB = SplitEdge(ReloadFrom, ReloadTo, DT, LI);
  Instruction *InsertPt = &*(ReloadBB->getFirstInsertionPt());
  Builder.SetInsertPoint(InsertPt);
  // Insert IVs reloading
  for (size_t I = 0, E = IVs.size(); I < E; I++) {
    auto Inst = Builder.CreateLoad(Allocs[I], "iv-reload-var");
    Reloads.push_back(Inst);
    LLVM_DEBUG(dbgs() << "Reload IV " << IVs[I]->getName() << "\n");
  }
  // Replace user of IVs in ReloadBB with new reloads
  for (size_t I = 0, E = IVs.size(); I < E; I++) {
    Value::user_iterator IUser = IVs[I]->user_begin();
    Value::user_iterator IEnd = IVs[I]->user_end();
    while (IUser != IEnd) {
      Instruction *Inst = cast<Instruction>(*IUser);
      Use &IVUse = IUser.getUse();
      IUser++;
      auto UserBB = Inst->getParent();
      // If ReloadTo has single predecessor, all the instructions move from
      // ReloadTo to the new BB (ReloadBB).
      // we can't update the uses inside ReloadTo at this point since there
      // can be multiple predecessors for ReloadTo and we need to insert phi
      // firstly and then update the uses, which is the work of updateIVUser.
      if (UserBB == ReloadBB) {
        LLVM_DEBUG(dbgs() << "Update bottom IV user in Inst: "
                          << *Inst << "\n");
        IVUse.set(Reloads[I]);
      }
    }
  }
  LLVM_DEBUG(dbgs() << *ReloadBB << "\n");
}

// This function updates all IV users (in BB3 and BB4) reachable by reload.
// For simplicity, we only consider case of 548 :each interested BB (such as
// BB3 and BB4) has only 2 predecessors (one provide original IV and the
// other provide reload one or merged one by inserting phi)
//
//   /---BB1
//   |  /   \
//   |  |   BB2
//   |  |    | \
//   |  |    |  Inner Loop
//   |  |    |  |
//   |  |    |  [reload]
//   |  |    |  /
//   |  \    BB3
//   |   \  /
//   |    BB4
//   |___/ \
//
// TODO: handle more complicated IV uses after reload other than 548
void IVSplit::updateIVUser(Loop * L, IRBuilder<> &Builder) {
  DT->updateDFSNumbers();
  ForwardIDFCalculator IDF(*DT);
  SmallPtrSet<BasicBlock *, 1> DefBlocks;
  DefBlocks.insert(ReloadBB);
  IDF.setDefiningBlocks(DefBlocks);

  SmallPtrSet<BasicBlock *, 32> LiveInBlocks;
  SmallVector<BasicBlock *, 32> IDFBlocks;
  for (size_t I = 0, E = IVs.size(); I < E; I++) {
    for (auto User : make_range(IVs[I]->user_begin(), IVs[I]->user_end())) {
      Instruction *Inst = cast<Instruction>(User);
      auto UserBB = Inst->getParent();
      if(UserBB != L->getHeader()) {
        LiveInBlocks.insert(UserBB);
      }
    }
  }

  IDF.resetLiveInBlocks();
  IDF.setLiveInBlocks(LiveInBlocks);
  IDF.calculate(IDFBlocks);

  ReDefs = Reloads;

  BasicBlock * PredBB = ReloadBB;
  for(BasicBlock * BB: IDFBlocks) {
    if (!BB->hasNPredecessors(2))
      return; // Bail out if it's not the interested case
    if (PredBB->getSingleSuccessor() != BB)
      return; // Bail out if it's not the interested case
    // Currently only support following case for 548:
    //
    //       Reload BB <--- Pred BB
    //    \   /
    //     \ /
    //      BB1 <--- current BB
    //  \   /
    //   \ /
    //    BB2
    //
    for (size_t I = 0, E = IVs.size(); I < E; I++) {

      PHINode *OpPN =
        PHINode::Create(IVs[I]->getType(), 2, "iv-split-phi",
                        &*(BB->getFirstInsertionPt()));
      for (auto PI: make_range(pred_begin(BB), pred_end(BB))) {
        if(PI == PredBB) {
          OpPN->addIncoming(ReDefs[I], PI);
        } else {
          OpPN->addIncoming(IVs[I], PI);
        }
      }
      ReDefs[I] = OpPN;

      LLVM_DEBUG(dbgs() << "Create new phi for IV: "
                        << IVs[I]->getName() << "\n");

      // Replace user of IV in current BB with new phi
      Value::user_iterator IUser = IVs[I]->user_begin();
      Value::user_iterator IEnd = IVs[I]->user_end();
      while (IUser != IEnd) {
        Instruction *Inst = cast<Instruction>(*IUser);
        Use &IVUse =IUser.getUse();
        IUser++;
        if (Inst == OpPN)
          continue;
        auto UserBB = Inst->getParent();
        if (UserBB == BB) {
          LLVM_DEBUG(dbgs() << "Update DF IV user in Inst: "
                            << *Inst << "\n");
          IVUse.set(ReDefs[I]);
        }
      }
    }
    PredBB = BB;
    LLVM_DEBUG(dbgs() << *BB << "\n");
  }
}

// Sink IV i9 computation (t1 - t7) to the inner most loop:
// DO i1
//   t1 = 45 -i1
//   DO i2
//     t2 = t1 - i2
//     ...
//     DO i8
//       i9 = t7 - i8
//
// There are 2 purposes for the sink:
// 1: Reduce spill/reload for t1 - t7.
// 2: Avoid unnecessary computation (the inner most loop for i9 is cold).
//
// It's nontrivial to disable above code motion in HIR/LICM/GVN/CSE/LoopOpt.
// So a quick solution is to match the pattern and replace i9 with sink value.
void IVSplit::loopIVComputationSink() {
  /*
    Use heuristics to find following pattern for i9 computation.
    Bail out if fail to match the pattern.

    %Const_45_shl_32 = bitcast i64 0x2d00000000 to i64

Loop1
    %IV1_32bit = trunc i64 %IV1 to i32
    %I9_32bit_1 = sub i32 45, %IV1_32bit
Loop2
    %IV2_32bit = trunc i64 %IV2 to i32
    %I9_32bit_2 = sub i32 %I9_32bit_1, %IV2_32bit
Loop3
    %IV3_32bit = trunc i64 %IV3 to i32
    %I9_32bit_3 = sub i32 %I9_32bit_2, %IV3_32bit
Loop4
    %IV4_32bit = trunc i64 %IV4 to i32
    %I9_32bit_4 = sub i32 %I9_32bit_3, %IV4_32bit
Loop5
    %IV5_32bit = trunc i64 %IV5 to i32
    %I9_32bit_5 = sub i32 %I9_32bit_4, %IV5_32bit
Loop6
    %IV6_32bit = trunc i64 %IV6 to i32
    %I9_32bit_6 = sub i32 %I9_32bit_5, %IV6_32bit
Loop7
    %IV7_32bit = trunc i64 %IV7 to i32
    %I9_32bit_7 = sub i32 %I9_32bit_6, %IV7_32bit
Loop8
    %IV8_32bit = trunc i64 %IV8 to i32
    %I9_32bit = sub i32 %I9_32bit_7, %IV8_32bit
    %Inst3 = sext i32 %I9_32bit to i64
    %Inst4 = add nsw i64 %Inst3, -1
    %Inst5 = getelementptr @brute_force_mp_block_, i64 0, i64 0, i64 0,
                                                   i64 %Inst4
    %Inst6 = getelementptr inbounds i32, i32* %Inst5, i64 72
    %Inst7 = load i32, i32* %Inst6, align 4
    %Inst8 = icmp slt i32 %Inst7, 1
    br i1 %Inst8, label %BB0, label %BB1

BB1:
    %I9_1 = add i64 %IV8, %IV7
    %I9_2 = add i64 %I9_1, %IV6
    %I9_3 = add i64 %I9_2, %IV5
    %I9_4 = add i64 %I9_3, %IV4
    %I9_5 = add i64 %I9_4, %IV3
    %I9_6 = add i64 %I9_5, %IV2
    %I9_7 = add i64 %I9_6, %IV1
    %Inst1 = shl i64 %I9_7, 32
    %I9_shl32 = sub i64 %Const_45_shl_32, %Inst1
    %I9 = ashr exact i64 %I9_shl32, 32
    %Inst2 = add nsw i64 %I9, -1

The transformed code is as below:

Loop8
    %I9_1 = add i64 %IV8, %IV7
    %I9_2 = add i64 %I9_1, %IV6
    %I9_3 = add i64 %I9_2, %IV5
    %I9_4 = add i64 %I9_3, %IV4
    %I9_5 = add i64 %I9_4, %IV3
    %I9_6 = add i64 %I9_5, %IV2
    %I9_7 = add i64 %I9_6, %IV1
    %I9 = sub 45, i64 %I9_7
    %I9_32bit = trunc i64 %I9 to i32
    %Inst2 = add nsw i64 %I9, -1
    %Inst5 = getelementptr @brute_force_mp_block_, i64 0, i64 0, i64 0,
                                                   i64 %Inst2
    %Inst6 = getelementptr inbounds i32, i32* %Inst5, i64 72
    %Inst7 = load i32, i32* %Inst6, align 4
    %Inst8 = icmp slt i32 %Inst7, 1
    br i1 %Inst8, label %BB0, label %BB1
   */

  // We know that i9 appears for each 8 loop (loop1 - loop8)
  if ((CurDepth % 8 != 1) || (IVs.size() != 1))
    return;

  PHINode *PN = cast<PHINode>(IVs[0]);
  Instruction *I9_7 = nullptr;
  BasicBlock * BB1;
  for (auto User : make_range(PN->user_begin(), PN->user_end())) {
   I9_7 = dyn_cast<Instruction>(User);
   BB1 = I9_7->getParent();
   Loop *UserLoop = LI->getLoopFor(BB1);
   // Search for loop8 user of IV which is a part of i9 computation
   if(UserLoop->getLoopDepth() == CurDepth+7)
     break;
  }

  if (!I9_7 || !BB1->hasNPredecessors(1))
    return;

  /* Check %I9_7 to %I9_1
    %I9_1 = add i64 %IV8, %IV7
    %I9_2 = add i64 %I9_1, %IV6
    %I9_3 = add i64 %I9_2, %IV5
    %I9_4 = add i64 %I9_3, %IV4
    %I9_5 = add i64 %I9_4, %IV3
    %I9_6 = add i64 %I9_5, %IV2
    %I9_7 = add i64 %I9_6, %IV1
  */
  BasicBlock::reverse_iterator IRI = I9_7->getReverseIterator();
  BasicBlock::reverse_iterator IRE = BB1->rend();
  Instruction *I9_n = I9_7;
  Value *IV;
  for (int Cnt= 7; Cnt > 1; Cnt-- ) {
    Instruction *Inst = &*IRI;
    if (Inst != I9_n)
      return;
    if (!match(Inst, m_Add(m_Instruction(I9_n), m_Value(IV))))
      return;
    if (++IRI == IRE)
      return;
  }
  if (IRI == IRE)
    return;
  Instruction *I9_1 = &*IRI;
  Value *IV8;
  if (!match(I9_1, m_Add(m_Value(IV8), m_Value(IV))))
    return;

  Instruction *Inst1 = nullptr;
  if (I9_7->hasOneUse())
    Inst1 = dyn_cast<Instruction>(*I9_7->user_begin());
  if (!Inst1 || Inst1->getOpcode() != Instruction::Shl)
    return;

  Instruction *I9_shl32 = nullptr;
  if (Inst1->hasOneUse())
    I9_shl32 = dyn_cast<Instruction>(*Inst1->user_begin());
  if (!I9_shl32 || I9_shl32->getOpcode() != Instruction::Sub)
    return;

  Instruction *IConst_45_shl_32 = dyn_cast<Instruction>(
    I9_shl32->getOperand(0));
  if (!IConst_45_shl_32)
    return;
  if (IConst_45_shl_32->getOpcode() != Instruction::BitCast)
    return;

  // Check the magic number to make sure it's the pattern we're searching for.
  ConstantInt *CV = dyn_cast<ConstantInt>(IConst_45_shl_32->getOperand(0));
  if (!CV || CV->getSExtValue() != 0x2d00000000)
    return;

  Instruction *I9 = nullptr;
  if (I9_shl32->hasOneUse())
    I9 = dyn_cast<Instruction>(*I9_shl32->user_begin());
  if (!I9 || I9->getOpcode() != Instruction::AShr)
    return;

  BasicBlock::iterator II = I9->getIterator();
  BasicBlock::iterator IE = BB1->end();
  if (++II == IE)
    return;
  Instruction *Inst2 = &*II;
  ConstantInt *CI;
  if (!match(Inst2, m_Add(m_Specific(I9), m_ConstantInt(CI))) ||
      !CI->isMinusOne())
    return;

  BasicBlock *PredBB = BB1->getSinglePredecessor();
  Instruction *Ibr = PredBB->getTerminator();
  if (Ibr->getOpcode() != Instruction::Br)
    return;

  Instruction *IInst8 = dyn_cast<Instruction>(Ibr->getOperand(0));
  if (!IInst8 || IInst8->getOpcode() != Instruction::ICmp)
    return;

  Instruction *Inst7 = dyn_cast<Instruction>(IInst8->getOperand(0));
  if (!Inst7 || Inst7->getOpcode() != Instruction::Load)
    return;

  Instruction *IInst6 = dyn_cast<Instruction>(Inst7->getOperand(0));
  if (!IInst6 || IInst6->getOpcode() != Instruction::GetElementPtr)
    return;

  Instruction *Inst5 = dyn_cast<Instruction>(IInst6->getOperand(0));
  if (!Inst5 || Inst5->getOpcode() != Instruction::GetElementPtr)
    return;

  Instruction *Inst4 = dyn_cast<Instruction>(
    Inst5->getOperand(Inst5->getNumOperands()-1));
  if (!Inst4 || Inst4->getOpcode() != Instruction::Add)
    return;

  Instruction *Inst3 = dyn_cast<Instruction>(Inst4->getOperand(0));
  if (!Inst3 || Inst3->getOpcode() != Instruction::SExt)
    return;

  Instruction *I9_32bit = dyn_cast<Instruction>(Inst3->getOperand(0));
  if (!I9_32bit || I9_32bit->getOpcode() != Instruction::Sub)
    return;

  Instruction *IIV8_32bit = dyn_cast<Instruction>(I9_32bit->getOperand(1));
  if (!IIV8_32bit || !match(IIV8_32bit, m_Trunc(m_Specific(IV8))))
    return;

  // ??? Need more checks to confirm the pattern ???

  // 32bit i9 (%844) should have 2 uses:
  // 1st use: %Inst3 = sext i32 %I9_32bit to i64
  // 2nd use: store i32 %I9_32bit, i32* getelementptr @brute_force_mp_sudoku2_
  //   Fortran code --- 1110: sudoku2(row, 9) = i9
  if (!I9_32bit->hasNUses(2))
    return;

  /* The i9 computation pattern is found and start to replace it with sink one
   * Create 64bit i9 new sink Value and discard shl/ashr since IV is 32 bit.
      %I9_7 = add i64 %I9_6, %IV1
      %I9_new = sub 45, %I9_7
      %Inst2 = add nsw i64 %I9_new, -1
   */
  Instruction *I9_new = I9_shl32;
  Instruction *I9_old = I9;
  I9_new->setOperand(0, ConstantInt::getSigned(I9->getOperand(0)->getType(),
      45));
  I9_new->setOperand(1, I9_7);
  // Replace all uses of old %I9 with new %I9
  I9_old->replaceAllUsesWith(I9_new);

  /* Move i9 sink computation to the position before its first user: %Inst5
   *
    %I9_1 = add i64 %IV8, %IV7
    %I9_2 = add i64 %I9_1, %IV6
    %I9_3 = add i64 %I9_2, %IV5
    %I9_4 = add i64 %I9_3, %IV4
    %I9_5 = add i64 %I9_4, %IV3
    %I9_6 = add i64 %I9_5, %IV2
    %I9_7 = add i64 %I9_6, %IV1
    %I9 = sub 45, i64 %I9_7
    %Inst2 = add nsw i64 %I9, -1
    %Inst5 = getelementptr @brute_force_mp_block_, i64 0, i64 0, i64 0,
                                                   i64 %Inst2
    %Inst6 = getelementptr inbounds i32, i32* %Inst5, i64 72
    %Inst7 = load i32, i32* %Inst6, align 4
    %Inst8 = icmp slt i32 %Inst7, 1
    br i1 %Inst8, label %BB0, label %BB1
  */

  II = I9_1->getIterator();
  do {
    Instruction &curI = *II++;
    curI.moveBefore(Inst5);
    if (&curI == Inst2)
      break;
  } while(true);

  // Use (sink) I9 - 1 as index
  Inst5->setOperand(Inst5->getNumOperands()-1, Inst2);

  // Search the second use of 32bit i9 (%I9_32bit)
  //   1102: if(block(row, 9, i9) cycle
  //   1110: sudoku2(row, 9) = i9
  //
  // 1st use: %Inst3 = sext i32 %I9_32bit to i64
  // 2nd use: store i32 %I9_32bit, i32* getelementptr @brute_force_mp_sudoku2_
  Value::user_iterator Second = I9_32bit->user_begin();
  if (*Second == Inst3)
    Second++; // The first use (%Inst3) will be deleted

  // Create new 32bit sink i9: Trunc
  // Replace the second use of %I9_32bit with new sink 32bit i9: Trunc
  Instruction *I9_32bit_user = dyn_cast<Instruction>(*Second);
  for (unsigned Op = 0, NumOps = I9_32bit_user->getNumOperands();
      Op != NumOps; ++Op) {
    if (I9_32bit_user->getOperand(Op) == I9_32bit) {
      Instruction *Trunc = CastInst::Create(Instruction::Trunc, I9_new,
        I9_32bit_user->getOperand(Op)->getType(), "", Inst2);
      I9_32bit_user->setOperand(Op, Trunc);
    }
  }

  // Remove all instructions only for code motion of old 32bit i9 computation
  I9_old->eraseFromParent();
  Inst1->eraseFromParent();
  Inst4->eraseFromParent();
  Inst3->eraseFromParent();
  SmallVector<Instruction *, 16> DelList;
  DelList.push_back(I9_32bit);
  while (!DelList.empty()) {
    Instruction *Del = DelList.back();
    DelList.pop_back();
    for (unsigned Op = 0, NumOps = Del->getNumOperands(); Op != NumOps; ++Op) {
      if (Instruction *Cand =
          dyn_cast<Instruction>(Del->getOperand(Op))) {
        if (Cand->hasOneUse())
          DelList.push_back(Cand);
      }
    }
    Del->eraseFromParent();
  }

  LLVM_DEBUG(dbgs() << "Sink i9 computation for Loop" << CurDepth << "\n");
}

bool IVSplit::loopIVSplitRecursion(Loop * L) {
  IVs.clear();
  Allocs.clear();
  Reloads.clear();
  ReDefs.clear();
  bool Changed = false;

  if(isIVSplitIdealLoop(L)) {
    IRBuilder<> Builder(&*(FN->getEntryBlock().getFirstInsertionPt()));

    for (size_t I = 0, E = IVs.size(); I < E; I++) {
      Allocs.push_back(Builder.CreateAlloca(IVs[I]->getType(), nullptr,
                                            "iv-split-var"));
    }

    loopIVComputationSink();

    LLVM_DEBUG(dbgs() << "======= Begin IV Split for Loop"
                      << CurDepth << "\n");

    spillIV(L, Builder);
    reloadIV(L, Builder);
    updateIVUser(L, Builder);

    NumIVSplit += IVs.size();
    Changed = true;

    LLVM_DEBUG(dbgs() << "======= End IV Split for Loop" << CurDepth
                      << ": split " << IVs.size() << " IV\n");
  }

  for (Loop *SubLoop : L->getSubLoops()) {
      Changed |= loopIVSplitRecursion(SubLoop);
  }

  return Changed;
}

//===----------------------------------------------------------------------===//
//
// Pass Manager integration code
//
//===----------------------------------------------------------------------===//
PreservedAnalyses IVSplitPass::run(Function &F, FunctionAnalysisManager &FAM) {
  if (!F.hasFnAttribute("contains-rec-pro-clone"))
    return PreservedAnalyses::all();

  auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  if (!IVSplit(&F, &DT, &LI).performLoopIVSplit())
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LoopAnalysis>();
  return PA;
}

namespace {

struct IVSplitLegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  IVSplitLegacyPass() : FunctionPass(ID) {
    initializeIVSplitLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (!F.hasFnAttribute("contains-rec-pro-clone"))
      return false;

    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    return IVSplit(&F, DT, LI).performLoopIVSplit();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }
};

} // end anonymous namespace

char IVSplitLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(IVSplitLegacyPass, "iv-split", "IV Split", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(IVSplitLegacyPass, "iv-split", "IV Split", false, false)

FunctionPass *llvm::createIVSplitLegacyPass() {
  return new IVSplitLegacyPass();
}
