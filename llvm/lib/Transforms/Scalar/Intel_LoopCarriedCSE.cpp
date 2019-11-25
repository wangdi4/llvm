//===---- Intel_LoopCarriedCSE.cpp - Implements Loop Carried CSE Pass -----===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass groups two Phi Nodes in a binary operation by a new Phi Node if
// their latch values have the same binary operation.
//
// For example:
// Convert
//
// for.preheader:
//   %gepload =
//   %gepload37 =
//   br %loop.25
//
// loop.25:
//   %t32.0 = phi i32 [ %gepload37, %for.preheader ], [ %gepload41, %loop.25 ]
//   %t30.0 = phi i32 [ %gepload, %for.preheader ], [%gepload39, %loop.25 ]
//   %1 = add i32 %t30.0, %t32.0
//   %4 = add i32 %gepload39, %gepload41
//
// To -
//
// for.preheader:
//   %gepload =
//   %gepload37 =
//   %1 = add i32 %gepload37, %gepload
//   br %loop.25
//
// loop.25:
//   %t32.0.lccse = phi i32 [ %1, %for.preheader ], [ %4, %loop.25 ]
//   %4 = add i32 %gepload39, %gepload41
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_LoopCarriedCSE.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;

#define LDIST_NAME "loop-carried-cse"
#define DEBUG_TYPE LDIST_NAME

// Returns user instruction which has opcode \p OpCode and operands \p LatchVal1
// and \p LatchVal2.
static BinaryOperator *
findMatchedLatchBinOp(Value *LatchVal1, Value *LatchVal2, FPMathOperator *FPOp,
                      Instruction::BinaryOps OpCode, bool IsSwappedOrder,
                      BasicBlock *LoopLatch, DominatorTree *DT) {
  for (User *U : LatchVal1->users()) {
    BinaryOperator *LatchBOp = dyn_cast<BinaryOperator>(U);

    if (!LatchBOp) {
      continue;
    }

    unsigned LatchOpCode = LatchBOp->getOpcode();

    if (LatchOpCode != OpCode) {
      continue;
    }

    FPMathOperator *LatchFPOp = dyn_cast<FPMathOperator>(U);

    if (FPOp && LatchFPOp && FPOp->isFast() != LatchFPOp->isFast()) {
      continue;
    }

    Value *V0 = LatchBOp->getOperand(0);
    Value *V1 = LatchBOp->getOperand(1);

    bool LatchIsSwappedOrder = V0 != LatchVal1;
    Value *LatchVal2Use = LatchIsSwappedOrder ? V0 : V1;

    if (LatchVal2Use != LatchVal2) {
      continue;
    }

    if (LatchIsSwappedOrder != IsSwappedOrder && !LatchBOp->isCommutative()) {
      continue;
    }

    if (!DT->dominates(LatchBOp->getParent(), LoopLatch)) {
      continue;
    }

    return LatchBOp;
  }

  return nullptr;
}

// Transfer the chain like this-
// t1 = t0 + phi2
// t3 = t1 + t2
// t4 = t3 + phi
//
// To the following format by searching the def in the instruction chain
//
// t1 = phi + phi2
// t3 = t1 + t2
// t4 = t3 + t0
static PHINode *findSecondHeaderPhiInDef(Value *DefBinOp,
                                         Instruction::BinaryOps OpCode,
                                         BasicBlock *Header, unsigned Depth,
                                         BinaryOperator *&LastBinOp) {
  if (Depth > 3) {
    return nullptr;
  }

  LastBinOp = dyn_cast<BinaryOperator>(DefBinOp);

  if (!LastBinOp || !LastBinOp->isAssociative() || !LastBinOp->hasOneUse()) {
    return nullptr;
  }

  if (LastBinOp->getOpcode() != OpCode) {
    return nullptr;
  }

  Value *Operand1 = LastBinOp->getOperand(0);

  PHINode *CandidatePhi1 = dyn_cast<PHINode>(Operand1);

  if (CandidatePhi1 && CandidatePhi1->getParent() == Header) {
    return CandidatePhi1;
  }

  Value *Operand2 = LastBinOp->getOperand(1);
  PHINode *CandidatePhi2 = dyn_cast<PHINode>(Operand2);

  if (CandidatePhi2 && CandidatePhi2->getParent() == Header) {
    return CandidatePhi2;
  }

  PHINode *PhiNode = nullptr;

  PhiNode =
      findSecondHeaderPhiInDef(Operand1, OpCode, Header, Depth + 1, LastBinOp);

  if (PhiNode) {
    return PhiNode;
  }

  PhiNode =
      findSecondHeaderPhiInDef(Operand2, OpCode, Header, Depth + 1, LastBinOp);

  if (PhiNode) {
    return PhiNode;
  }

  return nullptr;
}

static PHINode *
findSecondHeaderPhi(PHINode *Phi, BasicBlock *Header, BinaryOperator *&BinOp,
                    BinaryOperator *&LastBinOp, bool &IsSwappedOrder,
                    Value *&ReassociativeOperand, Use *&ReassociativeUse) {
  Instruction::BinaryOps OpCode = BinOp->getOpcode();

  Value *Op2 =
      BinOp->getOperand(0) == Phi ? BinOp->getOperand(1) : BinOp->getOperand(0);

  PHINode *Phi2 = dyn_cast<PHINode>(Op2);

  if (Phi2 && Phi2->getParent() == Header) {
    IsSwappedOrder = BinOp->getOperand(1) == Phi;
    return Phi2;
  }

  // The following handles the case when only one Phi node in two operands. We
  // seach the use in the instruction chain, so that We can handle single use
  // instruction chains like this-
  //
  // t1 = fadd fast phi, t0
  // t2 = fadd fast t1, t3
  // t4 = fadd fast phi2, t2
  //
  //
  // It can be reassociated to-
  //
  // t1 = fadd fast phi, phi2
  // t2 = fadd fast t1, t3
  // t4 = fadd fast t0, t2
  //
  if (!BinOp->isAssociative()) {
    return nullptr;
  }

  BinaryOperator *TempBinOp = BinOp;

  while (TempBinOp->hasOneUse()) {
    LastBinOp = dyn_cast<BinaryOperator>(*TempBinOp->users().begin());

    if (!LastBinOp || !LastBinOp->isAssociative()) {
      break;
    }

    if (LastBinOp->getOpcode() != OpCode) {
      break;
    }

    Phi2 = LastBinOp->getOperand(0) == TempBinOp
               ? dyn_cast<PHINode>(LastBinOp->getOperand(1))
               : dyn_cast<PHINode>(LastBinOp->getOperand(0));

    if (Phi2 && Phi2->getParent() == Header) {
      for (auto UserIt = Phi2->user_begin(), EndIt = Phi2->user_end();
           UserIt != EndIt; ++UserIt) {
        auto *UserInst = cast<Instruction>(*UserIt);
        if (UserInst == LastBinOp) {
          ReassociativeUse = &(UserIt.getUse());
          break;
        }
      }

      ReassociativeOperand = Op2;
      return Phi2;
    }

    TempBinOp = LastBinOp;
  }

  Phi2 = findSecondHeaderPhiInDef(Op2, OpCode, Header, 0, LastBinOp);

  if (Phi2) {
    ReassociativeOperand = LastBinOp->getOperand(0) == Phi2
                               ? LastBinOp->getOperand(1)
                               : LastBinOp->getOperand(0);
    std::swap(LastBinOp, BinOp);

    // Phi only has one user
    ReassociativeUse = &(Phi->user_begin().getUse());

    return Phi2;
  }

  return nullptr;
}

// Go through the instruction chains and remove nsw and nuw flags when
// reassociation happens
static void removeNoWrapFlags(BinaryOperator *BinOp,
                              BinaryOperator *LastBinOp) {
  while (BinOp != LastBinOp) {
    BinOp->setHasNoSignedWrap(false);
    BinOp->setHasNoUnsignedWrap(false);
    BinOp = cast<BinaryOperator>(*BinOp->users().begin());
  }
  LastBinOp->setHasNoSignedWrap(false);
  LastBinOp->setHasNoUnsignedWrap(false);
}

static bool processLoop(Loop *L, DominatorTree *DT) {
  assert(L->empty() && "Only process inner loops.");

  LLVM_DEBUG(dbgs() << "\nLDist: In \""
                    << L->getHeader()->getParent()->getName() << "\" checking "
                    << *L << "\n");

  bool Modified = false;
  BasicBlock *Preheader = L->getLoopPreheader();
  BasicBlock *LoopLatch = L->getLoopLatch();

  if (!Preheader || !LoopLatch) {
    return false;
  }

  bool HasChanged;
  BasicBlock *Header = L->getHeader();

  do {
    // The flag showing whether a grouping is happened in the iteration
    HasChanged = false;

    for (PHINode &Phi : Header->phis()) {
      if (!Phi.hasOneUse()) {
        continue;
      }

      BinaryOperator *BinOp = dyn_cast<BinaryOperator>(*Phi.users().begin());

      if (!BinOp) {
        continue;
      }

      Instruction::BinaryOps OpCode = BinOp->getOpcode();

      Value *ReassociativeOperand = nullptr;
      BinaryOperator *LastBinOp = nullptr;
      bool IsSwappedOrder = false;
      Use *ReassociativeUse = nullptr;

      PHINode *Phi2 =
          findSecondHeaderPhi(&Phi, Header, BinOp, LastBinOp, IsSwappedOrder,
                              ReassociativeOperand, ReassociativeUse);

      if (!Phi2) {
        continue;
      }

      Value *LatchVal1 = nullptr;
      Value *LatchVal2 = nullptr;
      Value *PreheaderValue1 = nullptr;
      Value *PreheaderValue2 = nullptr;

      if (Phi.getIncomingBlock(0) == LoopLatch) {
        LatchVal1 = Phi.getIncomingValue(0);
        PreheaderValue1 = Phi.getIncomingValue(1);
      } else {
        LatchVal1 = Phi.getIncomingValue(1);
        PreheaderValue1 = Phi.getIncomingValue(0);
      }

      if (Phi2->getIncomingBlock(0) == LoopLatch) {
        LatchVal2 = Phi2->getIncomingValue(0);
        PreheaderValue2 = Phi2->getIncomingValue(1);
      } else {
        LatchVal2 = Phi2->getIncomingValue(1);
        PreheaderValue2 = Phi2->getIncomingValue(0);
      }

      FPMathOperator *FPOp = dyn_cast<FPMathOperator>(*Phi.users().begin());

      BinaryOperator *LatchBinOp = findMatchedLatchBinOp(
          LatchVal1, LatchVal2, FPOp, OpCode, IsSwappedOrder, LoopLatch, DT);

      if (!LatchBinOp) {
        continue;
      }

      if (ReassociativeOperand) {

        if (LatchBinOp == LastBinOp) {
          continue;
        }

        bool IsLatchBinOpInChain = false;

        BinaryOperator *TempBinOp = BinOp;

        do {

          if (TempBinOp == LatchBinOp) {
            IsLatchBinOpInChain = true;
            break;
          }

          TempBinOp = cast<BinaryOperator>(*TempBinOp->users().begin());

        } while (TempBinOp != LastBinOp);

        if (IsLatchBinOpInChain) {
          continue;
        }
      }

      IRBuilder<> Builder(Preheader->getTerminator());
      Value *V = nullptr;

      if (!IsSwappedOrder) {
        V = Builder.CreateBinOp(OpCode, PreheaderValue1, PreheaderValue2);
      } else {
        V = Builder.CreateBinOp(OpCode, PreheaderValue2, PreheaderValue1);
      }

      IRBuilder<> PHIBuilder(&Phi);

      PHINode *NewPhi =
          PHIBuilder.CreatePHI(Phi.getType(), 2, Phi.getName() + ".lccse");
      NewPhi->addIncoming(V, Preheader);
      NewPhi->addIncoming(LatchBinOp, LoopLatch);

      // Check whether Phi2 has one use before we erase BinOp below
      bool CanErasePhi2 = Phi2->hasOneUse();

      if (ReassociativeOperand) {
        ReassociativeUse->set(ReassociativeOperand);

        if (isa<OverflowingBinaryOperator>(BinOp)) {
          removeNoWrapFlags(BinOp, LastBinOp);
        }
      }

      BinOp->replaceAllUsesWith(NewPhi);
      BinOp->eraseFromParent();

      Phi.dropAllReferences();
      Phi.eraseFromParent();

      if (CanErasePhi2) {
        Phi2->dropAllReferences();
        Phi2->eraseFromParent();
      }

      HasChanged = true;
      Modified = true;
      break;
    }
  } while (HasChanged);

  return Modified;
}

static bool runImpl(LoopInfo *LI, DominatorTree *DT) {
  bool Changed = false;

  auto Loops = LI->getLoopsInPreorder();

  for (Loop *Lp : Loops) {
    if (Lp->empty()) {
      Changed |= processLoop(Lp, DT);
    }
  }

  return Changed;
}

namespace {

/// The pass class.
class LoopCarriedCSELegacy : public FunctionPass {
public:
  static char ID;

  LoopCarriedCSELegacy() : FunctionPass(ID) {
    // The default is set by the caller.
    initializeLoopCarriedCSELegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    return runImpl(LI, DT);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.setPreservesCFG();

    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
  }
};

} // end anonymous namespace

PreservedAnalyses LoopCarriedCSEPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {
  auto &LI = AM.getResult<LoopAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

  bool Changed = runImpl(&LI, &DT);
  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<LoopAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<AndersensAA>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

char LoopCarriedCSELegacy::ID;

static const char ldist_name[] = "Loop Carried CSE";

INITIALIZE_PASS_BEGIN(LoopCarriedCSELegacy, LDIST_NAME, ldist_name, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(LoopCarriedCSELegacy, LDIST_NAME, ldist_name, false, false)

FunctionPass *llvm::createLoopCarriedCSEPass() {
  return new LoopCarriedCSELegacy();
}
