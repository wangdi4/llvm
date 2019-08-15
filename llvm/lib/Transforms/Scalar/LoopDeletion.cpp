//===- LoopDeletion.cpp - Dead Loop Deletion Pass ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Dead Loop Deletion Pass. This pass is responsible
// for eliminating loops with non-infinite computable trip counts that have no
// side effects or volatile instructions, and do not contribute to the
// computation of the function's return value.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/LoopDeletion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
using namespace llvm;

#define DEBUG_TYPE "loop-delete"

STATISTIC(NumDeleted, "Number of loops deleted");

enum class LoopDeletionResult {
  Unmodified,
  Modified,
  Deleted,
};

#if INTEL_CUSTOMIZATION
// Analyze a loop for this pattern:
//
// while.header:
//   %src = phi (%ptr , %initial)
//   %ptr = getelementptr inbounds Ty, Ty* %src, CONST
//   ...
//   br %condition, %while.header, %loop.exit
// loop.exit:
//   PN = phi %ptr
//
// "ptr" is a simple pointer-type induction variable that is incremented by
// a constant amount on every loop iteration, and initialized outside the loop.
//
// "PN" is the phi at the loop exit for the loop result. It must have only one
// incoming value, which implies that the loop body dominates the exit, and
// the loop has  executed at least one time.
// "inc" is set to the integer increment value from the GEP.
// The function returns the initial value "%initial".
// If the loop does not fit this pattern, nullptr is returned.
static Value *getInitialValueAndInc(Loop *L, PHINode *PN, uint64_t &inc,
                                    const DataLayout &DL) {
  PHINode *HeaderPhi = nullptr;
  Instruction *UpdateInst = nullptr;
  int64_t Increment = 0;

  // exit phi must have 1 arg
  if (PN->getNumIncomingValues() > 1)
    return nullptr;
  Value *ValInLoop = PN->getIncomingValue(0);

  // Since this is pointer type, we're looking for a GEP increment.
  if (auto *G = dyn_cast<GetElementPtrInst>(ValInLoop)) {
    if (G->getNumIndices() != 1 || !G->hasAllConstantIndices())
      return nullptr;
    if (ConstantInt *IdxCI = dyn_cast<ConstantInt>(G->getOperand(1))) {
      int64_t IdxVal = IdxCI->getSExtValue();
      // Increment = sizeof(Ty) * index
      Increment = DL.getTypeAllocSize(G->getResultElementType()) * IdxVal;
      if (Increment <= 0)
        return nullptr;
      HeaderPhi = dyn_cast<PHINode>(G->getPointerOperand());
      UpdateInst = G;
    }
  }

  if (UpdateInst == nullptr)
    return nullptr;

  // No other updates to the induction pointer are allowed.
  // There should be a phi at the top of the loop with the GEP and the initial
  // value.
  if (!HeaderPhi || HeaderPhi->getParent() != L->getHeader() ||
      HeaderPhi->getNumIncomingValues() != 2)
    return nullptr;

  Value *InitVal = nullptr;
  if (HeaderPhi->getIncomingValue(0) == UpdateInst)
    InitVal = HeaderPhi->getIncomingValue(1);
  else if (HeaderPhi->getIncomingValue(1) == UpdateInst)
    InitVal = HeaderPhi->getIncomingValue(0);
  else
    return nullptr;

  // Make sure the initial value is really outside the loop.
  if (!L->isLoopInvariant(InitVal))
    return nullptr;

  inc = Increment;
  return InitVal;
}

// Remove any bitcasts and ptrtoint instructions defining V, returning the
// original source (or V itself).
static Value *stripCast(Value *V) {
  if (!V)
    return nullptr;
  // backwards walk of sequence: [bitcast], [bitcast],...[ptrtoint]
  if (auto *ConvInst = dyn_cast<PtrToIntInst>(V))
    V = ConvInst->getOperand(0);
  return V->stripPointerCasts();
}

// Used by the array "len" optimization below.
// Given a Value "len" that is the result of a computation with minimum
// value "MinVal", we may be able to prove that "len" can be replaced
// with the constant "MinVal".
// One case is if "len" is ultimately used in a zero-compare
// instruction, and no other instructions along the way, can make "len" zero.
//
// "I" is the use of "len", where we start walking the uses.
// "MinVal" is the constant minimum value of "len".
// "depth" is a recursion limiter (capped at 2 levels).
//
// Example:
//  %sub.ptr.sub.i = sub i64 %sub.ptr.lhs.cast.i, %sub.ptr.rhs.cast.i
//  %9 = lshr exact i64 %sub.ptr.sub.i, 1
//  %conv2.i = trunc i64 %9 to i32
//  %retval.0.i98 = phi i32 [ %conv2.i, %while.end.i ], [ 0, %lor.lhs.false.i ],
//                            [ 0, %if.end ]
//  %13 = or i32 %retval.0.i111, %retval.0.i98
//  %14 = icmp eq i32 %13, 0
//  %tobool5 = icmp ne i32 %retval.0.i98, 0
//
// If we can prove that the subtraction must be at least 2,
// we can replace the subtraction with the constant 2, without changing
// the result of the two compares.
static bool AllUsesCmpZero(Instruction *I, uint64_t MinVal,
                           const DataLayout &DL, uint32_t depth) {
  bool found = false;
  if (!I)
    return false;

  // Walk the uses of I until we hit a compare.
  // We use a while-loop to process a chain of single-uses.
  // Then we handle multiple-uses with recursion.
  while (MinVal > 0) {
    CmpInst *Cmp = dyn_cast<CmpInst>(I);
    if (Cmp) {
      // Found a compare. Must be eq 0 or ne 0.
      if ((Cmp->getPredicate() != ICmpInst::ICMP_EQ) &&
          (Cmp->getPredicate() != ICmpInst::ICMP_NE))
        break;
      ConstantInt *CmpVal = dyn_cast<ConstantInt>(Cmp->getOperand(1));
      if (!CmpVal || !CmpVal->isZero())
        break;
      found = true;
      break;
    }

    // Process other instructions that we can prove won't make MinVal zero.
    ConstantInt *ShVal = nullptr;
    uint64_t TruncBits = 0;
    unsigned Opcode = I->getOpcode();
    switch (Opcode) {
    case Instruction::LShr:
      if ((ShVal = dyn_cast<ConstantInt>(I->getOperand(1))))
        MinVal = MinVal >> ShVal->getZExtValue();
      else
        MinVal = 0;
      break;
    case Instruction::AShr:
      if ((ShVal = dyn_cast<ConstantInt>(I->getOperand(1))))
        MinVal = (int64_t)MinVal >> ShVal->getZExtValue();
      else
        MinVal = 0;
      break;
    case Instruction::Shl:
      if ((ShVal = dyn_cast<ConstantInt>(I->getOperand(1))) &&
          I->hasNoUnsignedWrap())
        MinVal = MinVal << ShVal->getZExtValue();
      else
        MinVal = 0;
      break;
    case Instruction::Trunc:
      TruncBits = DL.getTypeStoreSize(I->getType());
      MinVal = MinVal & ((1ULL << TruncBits) - 1);
      break;
    case Instruction::Or:
      // OR will not change nonzero to zero.
      break;
    case Instruction::PHI:
      // Tracing PHI in forward direction; no-op.
      break;
    default:
      // Anything else, we bail out.
      MinVal = 0;
      break;
    }
    if (!I->hasOneUse())
      break; // multiple uses are OK. Recursive loop is below.

    // One use, look at the next inst in the chain.
    I = I->user_back();
  }

  // Handle instructions with multiple uses by searching each use recursively.
  // Each use must end in a zero-compare with the same allowable instructions.
  if (!found && MinVal > 0 && !I->hasOneUse() && depth < 2) {
    // depth check limits searching to 4 paths: more hits are very unlikely
    for (auto UIter = I->use_begin(), EIter = I->use_end();
         UIter != EIter;) { // inc is in loop
      Use &U = *UIter++;
      Instruction *User = dyn_cast<Instruction>(U.getUser());
      if (User == nullptr) {
        break;
      }
      found = AllUsesCmpZero(User, MinVal, DL, depth + 1);
      if (!found)
        break;
    }
  }
  return found;
}
#endif // INTEL_CUSTOMIZATION

/// Determines if a loop is dead.
///
/// This assumes that we've already checked for unique exit and exiting blocks,
/// and that the code is in LCSSA form.
static bool isLoopDead(Loop *L, ScalarEvolution &SE,
                       SmallVectorImpl<BasicBlock *> &ExitingBlocks,
                       BasicBlock *ExitBlock, bool &Changed,
#if INTEL_CUSTOMIZATION
                       bool &NotInfinite,
#endif // INTEL_CUSTOMIZATION
                       BasicBlock *Preheader) {
  // Make sure that all PHI entries coming from the loop are loop invariant.
  // Because the code is in LCSSA form, any values used outside of the loop
  // must pass through a PHI in the exit block, meaning that this check is
  // sufficient to guarantee that no loop-variant values are used outside
  // of the loop.
  bool AllEntriesInvariant = true;
  bool AllOutgoingValuesSame = true;
  for (PHINode &P : ExitBlock->phis()) {
    Value *incoming = P.getIncomingValueForBlock(ExitingBlocks[0]);

    // Make sure all exiting blocks produce the same incoming value for the exit
    // block.  If there are different incoming values for different exiting
    // blocks, then it is impossible to statically determine which value should
    // be used.
    AllOutgoingValuesSame =
        all_of(makeArrayRef(ExitingBlocks).slice(1), [&](BasicBlock *BB) {
          return incoming == P.getIncomingValueForBlock(BB);
        });

    if (!AllOutgoingValuesSame)
      break;

    if (Instruction *I = dyn_cast<Instruction>(incoming))
      if (!L->makeLoopInvariant(I, Changed, Preheader->getTerminator())) {

#if INTEL_CUSTOMIZATION
        // p = pstart;
        // {
        //   p++; // actually p += sizeof(*p) in bytes
        // } while (*p)
        // pp = PHI(p) // 1 arg only
        // len = pp-pstart;
        // if (len != 0)
        //
        // If "p" and "len" are only used to make the computation above, the
        // loop is dead. "len" is guaranteed to be nonzero, as pointer
        // wraparound is undefined.
        // We can replace p with undef, and len with sizeof(*p), without
        // changing the code behavior. The loop deleter will then remove the
        // loop, if the loop has no side effects, all other results are
        // invariant, etc. (the usual criteria)
        AllEntriesInvariant = false;
        if (!I->getType()->isPointerTy())
          // cannot be generalized to integers without knowing if the loop
          // will terminate.
          break;

        uint64_t MinVal = 0;
        assert(P.getModule() != nullptr && "Null module found!");
        const DataLayout &DL = P.getModule()->getDataLayout();

        // First, analyze the loop and get the initial value "pstart" and
        // the real pointer increment value.
        Value *Initial = getInitialValueAndInc(L, &P, MinVal, DL);
        if (Initial == nullptr || MinVal == 0)
          break;

        // Find the instruction defining "len" above, by starting at the phi
        // at the loop exit, and walking its uses.
        // getInitialValueandInc() checks that the phi has only 1 incoming
        // x value.
        // This is important, as the "len" computation must be dominated
        // by the loop body so that we are guaranteed the loop has executed
        // at least once.
        Instruction *SubResult = nullptr;
        Instruction *CurrInst = &P;
        while (CurrInst->hasOneUse()) {
          // pattern may be:
          //   %P = phi [x]
          //   %1 = ptrtoint %P
          //   %2 = ptrtoint %Initial
          //   %len = sub %1 %2
          // Only ptrtoint and bitcast are allowed.
          Instruction *User = CurrInst->user_back();
          if (User->getOpcode() == Instruction::Sub) {
            // "len" must be as above: operand 0 is directly derived from
            // the phi, and operand 1 must be the initial value of "p".
            if (User->getOperand(0) != CurrInst)
              break;
            // The other sub operand may have some ptrtoints that we need
            // to get through.
            if (stripCast(User->getOperand(1)) == Initial)
              SubResult = User;
            CurrInst = User;
            break;
          }
          if (!isa<BitCastInst>(User) && !isa<PtrToIntInst>(User))
            break;
          CurrInst = User;
        }

        if (SubResult == nullptr)
          break;

        LLVM_DEBUG(
            dbgs()
                << "Loop result is always positive, looking for zero test.\n";
            SubResult->print(dbgs()); dbgs() << "\n");

        if (!CurrInst->hasOneUse())
          break;

        assert(CurrInst == SubResult && "Incorrect subtract pattern match");

        // CurrInst is now the subtraction resulting in "len".
        // We need to prove that "len" is only used to test zero/nonzero.
        // Some other operands may be applied to "len" before the compare.
        // As long as these operations do not change the zero/nonzero property
        // of "len", they can be ignored. More detail in AllUsesCmpZero().
        CurrInst = CurrInst->user_back();
        bool AllCmp = AllUsesCmpZero(CurrInst, MinVal, DL, 0);

        if (AllCmp) {
          // Replace "len" with the loop increment value (the minimum
          // difference if the loop only iterated one time). The subtraction
          // is now dead.
          LLVM_DEBUG(dbgs() << "Loop result only used in zero-test, can "
                            << "replace with constant " << MinVal << "\n");

          Value *MinValIR =
              ConstantInt::get(SubResult->getType(), MinVal, false);
          SubResult->replaceAllUsesWith(MinValIR);

          // I is the result incoming from the loop, which is the phi
          // argument "p" in the example. Replace it with a constant, to
          // force "p" to be dead. This allows the loop to be deleted if
          // suitable.
          I->replaceUsesOutsideBlock(
              UndefValue::get(cast<PointerType>(I->getType())), I->getParent());
          // Pointer increments can't roll over, tell the caller.
          NotInfinite = true;
          // This doesn't mean "all" entries; we will still continue to
          // check the other
          // live-out values of the loop and reset this flag if needed.
          AllEntriesInvariant = true;

          continue; // keep checking other live values
        }
#endif // INTEL_CUSTOMIZATION

        AllEntriesInvariant = false;
        break;
      }
  }

  if (Changed)
    SE.forgetLoopDispositions(L);

  if (!AllEntriesInvariant || !AllOutgoingValuesSame)
    return false;

  // Make sure that no instructions in the block have potential side-effects.
  // This includes instructions that could write to memory, and loads that are
  // marked volatile.
  for (auto &I : L->blocks())
    if (any_of(*I, [](Instruction &I) { return I.mayHaveSideEffects(); }))
      return false;
  return true;
}

/// This function returns true if there is no viable path from the
/// entry block to the header of \p L. Right now, it only does
/// a local search to save compile time.
static bool isLoopNeverExecuted(Loop *L) {
  using namespace PatternMatch;

  auto *Preheader = L->getLoopPreheader();
  // TODO: We can relax this constraint, since we just need a loop
  // predecessor.
  assert(Preheader && "Needs preheader!");

  if (Preheader == &Preheader->getParent()->getEntryBlock())
    return false;
  // All predecessors of the preheader should have a constant conditional
  // branch, with the loop's preheader as not-taken.
  for (auto *Pred: predecessors(Preheader)) {
    BasicBlock *Taken, *NotTaken;
    ConstantInt *Cond;
    if (!match(Pred->getTerminator(),
               m_Br(m_ConstantInt(Cond), Taken, NotTaken)))
      return false;
    if (!Cond->getZExtValue())
      std::swap(Taken, NotTaken);
    if (Taken == Preheader)
      return false;
  }
  assert(!pred_empty(Preheader) &&
         "Preheader should have predecessors at this point!");
  // All the predecessors have the loop preheader as not-taken target.
  return true;
}

/// Remove a loop if it is dead.
///
/// A loop is considered dead if it does not impact the observable behavior of
/// the program other than finite running time. This never removes a loop that
/// might be infinite (unless it is never executed), as doing so could change
/// the halting/non-halting nature of a program.
///
/// This entire process relies pretty heavily on LoopSimplify form and LCSSA in
/// order to make various safety checks work.
///
/// \returns true if any changes were made. This may mutate the loop even if it
/// is unable to delete it due to hoisting trivially loop invariant
/// instructions out of the loop.
static LoopDeletionResult deleteLoopIfDead(Loop *L, DominatorTree &DT,
                                           ScalarEvolution &SE, LoopInfo &LI) {
  assert(L->isLCSSAForm(DT) && "Expected LCSSA!");

  // We can only remove the loop if there is a preheader that we can branch from
  // after removing it. Also, if LoopSimplify form is not available, stay out
  // of trouble.
  BasicBlock *Preheader = L->getLoopPreheader();
  if (!Preheader || !L->hasDedicatedExits()) {
    LLVM_DEBUG(
        dbgs()
        << "Deletion requires Loop with preheader and dedicated exits.\n");
    return LoopDeletionResult::Unmodified;
  }
  // We can't remove loops that contain subloops.  If the subloops were dead,
  // they would already have been removed in earlier executions of this pass.
  if (L->begin() != L->end()) {
    LLVM_DEBUG(dbgs() << "Loop contains subloops.\n");
    return LoopDeletionResult::Unmodified;
  }


  BasicBlock *ExitBlock = L->getUniqueExitBlock();

  if (ExitBlock && isLoopNeverExecuted(L)) {
    LLVM_DEBUG(dbgs() << "Loop is proven to never execute, delete it!");
    // Set incoming value to undef for phi nodes in the exit block.
    for (PHINode &P : ExitBlock->phis()) {
      std::fill(P.incoming_values().begin(), P.incoming_values().end(),
                UndefValue::get(P.getType()));
    }
    deleteDeadLoop(L, &DT, &SE, &LI);
    ++NumDeleted;
    return LoopDeletionResult::Deleted;
  }

  // The remaining checks below are for a loop being dead because all statements
  // in the loop are invariant.
  SmallVector<BasicBlock *, 4> ExitingBlocks;
  L->getExitingBlocks(ExitingBlocks);

  // We require that the loop only have a single exit block.  Otherwise, we'd
  // be in the situation of needing to be able to solve statically which exit
  // block will be branched to, or trying to preserve the branching logic in
  // a loop invariant manner.
  if (!ExitBlock) {
    LLVM_DEBUG(dbgs() << "Deletion requires single exit block\n");
    return LoopDeletionResult::Unmodified;
  }
  // Finally, we have to check that the loop really is dead.
  bool Changed = false;
#if INTEL_CUSTOMIZATION
  bool NotInfinite = false;
  if (!isLoopDead(L, SE, ExitingBlocks, ExitBlock, Changed, NotInfinite,
                  Preheader)) {
#endif // INTEL_CUSTOMIZATION
    LLVM_DEBUG(dbgs() << "Loop is not invariant, cannot delete.\n");
    return Changed ? LoopDeletionResult::Modified
                   : LoopDeletionResult::Unmodified;
  }

  // Don't remove loops for which we can't solve the trip count.
  // They could be infinite, in which case we'd be changing program behavior.
  const SCEV *S = SE.getConstantMaxBackedgeTakenCount(L);
#if INTEL_CUSTOMIZATION
  if (isa<SCEVCouldNotCompute>(S) && !NotInfinite) {
#endif // INTEL_CUSTOMIZATION
    LLVM_DEBUG(dbgs() << "Could not compute SCEV MaxBackedgeTakenCount.\n");
    return Changed ? LoopDeletionResult::Modified
                   : LoopDeletionResult::Unmodified;
  }

  LLVM_DEBUG(dbgs() << "Loop is invariant, delete it!");
  deleteDeadLoop(L, &DT, &SE, &LI);
  ++NumDeleted;

  return LoopDeletionResult::Deleted;
}

PreservedAnalyses LoopDeletionPass::run(Loop &L, LoopAnalysisManager &AM,
                                        LoopStandardAnalysisResults &AR,
                                        LPMUpdater &Updater) {

  LLVM_DEBUG(dbgs() << "Analyzing Loop for deletion: ");
  LLVM_DEBUG(L.dump());
  std::string LoopName = L.getName();
  auto Result = deleteLoopIfDead(&L, AR.DT, AR.SE, AR.LI);
  if (Result == LoopDeletionResult::Unmodified)
    return PreservedAnalyses::all();

  if (Result == LoopDeletionResult::Deleted)
    Updater.markLoopAsDeleted(L, LoopName);

  return getLoopPassPreservedAnalyses();
}

namespace {
class LoopDeletionLegacyPass : public LoopPass {
public:
  static char ID; // Pass ID, replacement for typeid
  LoopDeletionLegacyPass() : LoopPass(ID) {
    initializeLoopDeletionLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  // Possibly eliminate loop L if it is dead.
  bool runOnLoop(Loop *L, LPPassManager &) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    getLoopAnalysisUsage(AU);
  }
};
}

char LoopDeletionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(LoopDeletionLegacyPass, "loop-deletion",
                      "Delete dead loops", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_END(LoopDeletionLegacyPass, "loop-deletion",
                    "Delete dead loops", false, false)

Pass *llvm::createLoopDeletionPass() { return new LoopDeletionLegacyPass(); }

bool LoopDeletionLegacyPass::runOnLoop(Loop *L, LPPassManager &LPM) {
  if (skipLoop(L))
    return false;
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  LLVM_DEBUG(dbgs() << "Analyzing Loop for deletion: ");
  LLVM_DEBUG(L->dump());

  LoopDeletionResult Result = deleteLoopIfDead(L, DT, SE, LI);

  if (Result == LoopDeletionResult::Deleted)
    LPM.markLoopAsDeleted(*L);

  return Result != LoopDeletionResult::Unmodified;
}
