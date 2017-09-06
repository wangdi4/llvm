//===-- CSAIfConversion.cpp - CSA If Conversion ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains If Conversion support (used for dataflow conversion.)
//
//===----------------------------------------------------------------------===//

#include "CSAIfConversion.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "ifcvt"

// Hidden options for help debugging.
//static cl::opt<int> IfCvtFnStart("ifcvt-fn-start", cl::init(-1), cl::Hidden);
//static cl::opt<int> IfCvtFnStop("ifcvt-fn-stop", cl::init(-1), cl::Hidden);
//static cl::opt<int> IfCvtLimit("ifcvt-limit", cl::init(-1), cl::Hidden);
//static cl::opt<bool> DisableSimple("disable-ifcvt-simple",
//                                   cl::init(false), cl::Hidden);
//static cl::opt<bool> DisableSimpleF("disable-ifcvt-simple-false",
//                                    cl::init(false), cl::Hidden);
//static cl::opt<bool> DisableTriangle("disable-ifcvt-triangle",
//                                     cl::init(false), cl::Hidden);
//static cl::opt<bool> DisableTriangleR("disable-ifcvt-triangle-rev",
//                                      cl::init(false), cl::Hidden);
//static cl::opt<bool> DisableTriangleF("disable-ifcvt-triangle-false",
//                                      cl::init(false), cl::Hidden);
//static cl::opt<bool> DisableTriangleFR("disable-ifcvt-triangle-false-rev",
//                                       cl::init(false), cl::Hidden);
//static cl::opt<bool> DisableDiamond("disable-ifcvt-diamond",
//                                    cl::init(false), cl::Hidden);
//static cl::opt<bool> IfCvtBranchFold("ifcvt-branch-fold",
//                                    cl::init(true), cl::Hidden);

using namespace llvm;

char CSAIfConversion::ID = 0;

/// findFalseBlock - BB has a fallthrough. Find its 'false' successor given
/// its 'true' successor.
static MachineBasicBlock *findFalseBlock(MachineBasicBlock *BB,
                                         MachineBasicBlock *TrueBB) {
  for (MachineBasicBlock::succ_iterator SI = BB->succ_begin(),
         E = BB->succ_end(); SI != E; ++SI) {
    MachineBasicBlock *SuccBB = *SI;
    if (SuccBB != TrueBB)
      return SuccBB;
  }
  return nullptr;
}

/// reverseBranchCondition - Reverse the condition of the end of the block
/// branch. Swap block's 'true' and 'false' successors.
bool CSAIfConversion::reverseBranchCondition(BBInfo &BBI) {
  DebugLoc dl;  // FIXME: this is nowhere
  if (!TII->reverseBranchCondition(BBI.BrCond)) {
    TII->removeBranch(*BBI.BB);
    TII->insertBranch(*BBI.BB, BBI.FalseBB, BBI.TrueBB, BBI.BrCond, dl);
    std::swap(BBI.TrueBB, BBI.FalseBB);
    return true;
  }
  return false;
}

/// getNextBlock - Returns the next block in the function blocks ordering. If
/// it is the end, returns NULL.
static inline MachineBasicBlock *getNextBlock(MachineBasicBlock *BB) {
  MachineFunction::iterator I = BB->getIterator();
  MachineFunction::iterator E = BB->getParent()->end();
  if (++I == E)
    return nullptr;
  return &*I;
}

/// ValidSimple - Returns true if the 'true' block (along with its
/// predecessor) forms a valid simple shape for ifcvt. It also returns the
/// number of instructions that the ifcvt would need to duplicate if performed
/// in Dups.
bool CSAIfConversion::ValidSimple(BBInfo &TrueBBI, unsigned &Dups,
                              const BranchProbability &Prediction) const {
  Dups = 0;
  if (TrueBBI.IsBeingAnalyzed || TrueBBI.IsDone)
    return false;

  if (TrueBBI.IsBrAnalyzable)
    return false;

  if (TrueBBI.BB->pred_size() > 1) {
    if (TrueBBI.CannotBeCopied ||
        !TII->isProfitableToDupForIfCvt(*TrueBBI.BB, TrueBBI.NonPredSize,
                                        Prediction))
      return false;
    Dups = TrueBBI.NonPredSize;
  }

  return true;
}

/// ValidTriangle - Returns true if the 'true' and 'false' blocks (along
/// with their common predecessor) forms a valid triangle shape for ifcvt.
/// If 'FalseBranch' is true, it checks if 'true' block's false branch
/// branches to the 'false' block rather than the other way around. It also
/// returns the number of instructions that the ifcvt would need to duplicate
/// if performed in 'Dups'.
bool CSAIfConversion::ValidTriangle(BBInfo &TrueBBI, BBInfo &FalseBBI,
                                bool FalseBranch, unsigned &Dups,
                                const BranchProbability &Prediction) const {
  Dups = 0;
  if (TrueBBI.IsBeingAnalyzed || TrueBBI.IsDone)
    return false;

  if (TrueBBI.BB->pred_size() > 1) {
    if (TrueBBI.CannotBeCopied)
      return false;

    unsigned Size = TrueBBI.NonPredSize;
    if (TrueBBI.IsBrAnalyzable) {
      if (TrueBBI.TrueBB && TrueBBI.BrCond.empty())
        // Ends with an unconditional branch. It will be removed.
        --Size;
      else {
        MachineBasicBlock *FExit = FalseBranch
          ? TrueBBI.TrueBB : TrueBBI.FalseBB;
        if (FExit)
          // Require a conditional branch
          ++Size;
      }
    }
    if (!TII->isProfitableToDupForIfCvt(*TrueBBI.BB, Size, Prediction))
      return false;
    Dups = Size;
  }

  MachineBasicBlock *TExit = FalseBranch ? TrueBBI.FalseBB : TrueBBI.TrueBB;
  if (!TExit && blockAlwaysFallThrough(TrueBBI)) {
    MachineFunction::iterator I = TrueBBI.BB->getIterator();
    if (++I == TrueBBI.BB->getParent()->end())
      return false;
    TExit = &*I;
  }
  return TExit && TExit == FalseBBI.BB;
}

/// ValidDiamond - Returns true if the 'true' and 'false' blocks (along
/// with their common predecessor) forms a valid diamond shape for ifcvt.
bool CSAIfConversion::ValidDiamond(BBInfo &TrueBBI, BBInfo &FalseBBI,
                               unsigned &Dups1, unsigned &Dups2) const {
  Dups1 = Dups2 = 0;
  if (TrueBBI.IsBeingAnalyzed || TrueBBI.IsDone ||
      FalseBBI.IsBeingAnalyzed || FalseBBI.IsDone)
    return false;

  MachineBasicBlock *TT = TrueBBI.TrueBB;
  MachineBasicBlock *FT = FalseBBI.TrueBB;

  if (!TT && blockAlwaysFallThrough(TrueBBI))
    TT = getNextBlock(TrueBBI.BB);
  if (!FT && blockAlwaysFallThrough(FalseBBI))
    FT = getNextBlock(FalseBBI.BB);
  if (TT != FT)
    return false;
  if (!TT && (TrueBBI.IsBrAnalyzable || FalseBBI.IsBrAnalyzable))
    return false;
  if  (TrueBBI.BB->pred_size() > 1 || FalseBBI.BB->pred_size() > 1)
    return false;

  // FIXME: Allow true block to have an early exit?
  if (TrueBBI.FalseBB || FalseBBI.FalseBB ||
      (TrueBBI.ClobbersPred && FalseBBI.ClobbersPred))
    return false;

  // Count duplicate instructions at the beginning of the true and false blocks.
  MachineBasicBlock::iterator TIB = TrueBBI.BB->begin();
  MachineBasicBlock::iterator FIB = FalseBBI.BB->begin();
  MachineBasicBlock::iterator TIE = TrueBBI.BB->end();
  MachineBasicBlock::iterator FIE = FalseBBI.BB->end();
  while (TIB != TIE && FIB != FIE) {
    // Skip dbg_value instructions. These do not count.
    if (TIB->isDebugValue()) {
      while (TIB != TIE && TIB->isDebugValue())
        ++TIB;
      if (TIB == TIE)
        break;
    }
    if (FIB->isDebugValue()) {
      while (FIB != FIE && FIB->isDebugValue())
        ++FIB;
      if (FIB == FIE)
        break;
    }
    if (!TIB->isIdenticalTo(*FIB))
      break;
    ++Dups1;
    ++TIB;
    ++FIB;
  }

  // Now, in preparation for counting duplicate instructions at the ends of the
  // blocks, move the end iterators up past any branch instructions.
  while (TIE != TIB) {
    --TIE;
    if (!TIE->isBranch())
      break;
  }
  while (FIE != FIB) {
    --FIE;
    if (!FIE->isBranch())
      break;
  }

  // If Dups1 includes all of a block, then don't count duplicate
  // instructions at the end of the blocks.
  if (TIB == TIE || FIB == FIE)
    return true;

  // Count duplicate instructions at the ends of the blocks.
  while (TIE != TIB && FIE != FIB) {
    // Skip dbg_value instructions. These do not count.
    if (TIE->isDebugValue()) {
      while (TIE != TIB && TIE->isDebugValue())
        --TIE;
      if (TIE == TIB)
        break;
    }
    if (FIE->isDebugValue()) {
      while (FIE != FIB && FIE->isDebugValue())
        --FIE;
      if (FIE == FIB)
        break;
    }
    if (!TIE->isIdenticalTo(*FIE))
      break;
    ++Dups2;
    --TIE;
    --FIE;
  }

  return true;
}

/// ScanInstructions - Scan all the instructions in the block to determine if
/// the block is predicable. In most cases, that means all the instructions
/// in the block are isPredicable(). Also checks if the block contains any
/// instruction which can clobber a predicate (e.g. condition code register).
/// If so, the block is not predicable unless it's the last instruction.
void CSAIfConversion::ScanInstructions(BBInfo &BBI) {
  if (BBI.IsDone)
    return;

  bool AlreadyPredicated = !BBI.Predicate.empty();
  // First analyze the end of BB branches.
  BBI.TrueBB = BBI.FalseBB = nullptr;
  BBI.BrCond.clear();
  BBI.IsBrAnalyzable =
    !TII->analyzeBranch(*BBI.BB, BBI.TrueBB, BBI.FalseBB, BBI.BrCond);
  BBI.HasFallThrough = BBI.IsBrAnalyzable && BBI.FalseBB == nullptr;

  if (BBI.BrCond.size()) {
    // No false branch. This BB must end with a conditional branch and a
    // fallthrough.
    if (!BBI.FalseBB)
      BBI.FalseBB = findFalseBlock(BBI.BB, BBI.TrueBB);
    if (!BBI.FalseBB) {
      // Malformed bcc? True and false blocks are the same?
      BBI.IsUnpredicable = true;
      return;
    }
  }

  // Then scan all the instructions.
  BBI.NonPredSize = 0;
  BBI.ExtraCost = 0;
  BBI.ExtraCost2 = 0;
  BBI.ClobbersPred = false;
  for (MachineBasicBlock::iterator I = BBI.BB->begin(), E = BBI.BB->end();
       I != E; ++I) {
    if (I->isDebugValue())
      continue;

    if (I->isNotDuplicable())
      BBI.CannotBeCopied = true;

    bool isPredicated = TII->isPredicated(*I);
    bool isCondBr = BBI.IsBrAnalyzable && I->isConditionalBranch();

    // A conditional branch is not predicable, but it may be eliminated.
    if (isCondBr)
      continue;

    if (!isPredicated) {
      BBI.NonPredSize++;
      unsigned ExtraPredCost = TII->getPredicationCost(*I);
      unsigned NumCycles = SchedModel.computeInstrLatency(&*I, false);
      if (NumCycles > 1)
        BBI.ExtraCost += NumCycles-1;
      BBI.ExtraCost2 += ExtraPredCost;
    } else if (!AlreadyPredicated) {
      // FIXME: This instruction is already predicated before the
      // if-conversion pass. It's probably something like a conditional move.
      // Mark this block unpredicable for now.
      BBI.IsUnpredicable = true;
      return;
    }

    if (BBI.ClobbersPred && !isPredicated) {
      // Predicate modification instruction should end the block (except for
      // already predicated instructions and end of block branches).
      // Predicate may have been modified, the subsequent (currently)
      // unpredicated instructions cannot be correctly predicated.
      BBI.IsUnpredicable = true;
      return;
    }

    // FIXME: Make use of PredDefs? e.g. ADDC, SUBC sets predicates but are
    // still potentially predicable.
    std::vector<MachineOperand> PredDefs;
    if (TII->DefinesPredicate(*I, PredDefs))
      BBI.ClobbersPred = true;

    if (!TII->isPredicable(*I)) {
      BBI.IsUnpredicable = true;
      return;
    }
  }
}

/// FeasibilityAnalysis - Determine if the block is a suitable candidate to be
/// predicated by the specified predicate.
bool CSAIfConversion::FeasibilityAnalysis(BBInfo &BBI,
                                      SmallVectorImpl<MachineOperand> &Pred,
                                      bool isTriangle, bool RevBranch) {
  // If the block is dead or unpredicable, then it cannot be predicated.
  if (BBI.IsDone || BBI.IsUnpredicable)
    return false;

  // If it is already predicated, check if the new predicate subsumes
  // its predicate.
  if (BBI.Predicate.size() && !TII->SubsumesPredicate(Pred, BBI.Predicate))
    return false;

  if (BBI.BrCond.size()) {
    if (!isTriangle)
      return false;

    // Test predicate subsumption.
    SmallVector<MachineOperand, 4> RevPred(Pred.begin(), Pred.end());
    SmallVector<MachineOperand, 4> Cond(BBI.BrCond.begin(), BBI.BrCond.end());
    if (RevBranch) {
      if (TII->reverseBranchCondition(Cond))
        return false;
    }
    if (TII->reverseBranchCondition(RevPred) ||
        !TII->SubsumesPredicate(Cond, RevPred))
      return false;
  }

  return true;
}

/// AnalyzeBlock - Analyze the structure of the sub-CFG starting from
/// the specified block. Record its successors and whether it looks like an
/// if-conversion candidate.
CSAIfConversion::BBInfo &CSAIfConversion::AnalyzeBlock(MachineBasicBlock *BB,
                                             std::vector<IfcvtToken*> &Tokens) {
  BBInfo &BBI = BBAnalysis[BB->getNumber()];

  if (BBI.IsAnalyzed || BBI.IsBeingAnalyzed)
    return BBI;

  BBI.BB = BB;
  BBI.IsBeingAnalyzed = true;

  ScanInstructions(BBI);

  // Unanalyzable or ends with fallthrough or unconditional branch, or if is not
  // considered for ifcvt anymore.
  if (!BBI.IsBrAnalyzable || BBI.BrCond.empty() || BBI.IsDone) {
    BBI.IsBeingAnalyzed = false;
    BBI.IsAnalyzed = true;
    return BBI;
  }

  // Do not ifcvt if either path is a back edge to the entry block.
  if (BBI.TrueBB == BB || BBI.FalseBB == BB) {
    BBI.IsBeingAnalyzed = false;
    BBI.IsAnalyzed = true;
    return BBI;
  }

  // Do not ifcvt if true and false fallthrough blocks are the same.
  if (!BBI.FalseBB) {
    BBI.IsBeingAnalyzed = false;
    BBI.IsAnalyzed = true;
    return BBI;
  }

  BBInfo &TrueBBI  = AnalyzeBlock(BBI.TrueBB, Tokens);
  BBInfo &FalseBBI = AnalyzeBlock(BBI.FalseBB, Tokens);

  if (TrueBBI.IsDone && FalseBBI.IsDone) {
    BBI.IsBeingAnalyzed = false;
    BBI.IsAnalyzed = true;
    return BBI;
  }

  SmallVector<MachineOperand, 4> RevCond(BBI.BrCond.begin(), BBI.BrCond.end());
  bool CanRevCond = !TII->reverseBranchCondition(RevCond);

  unsigned Dups = 0;
  unsigned Dups2 = 0;
  bool TNeedSub = !TrueBBI.Predicate.empty();
  bool FNeedSub = !FalseBBI.Predicate.empty();
  bool Enqueued = false;

  BranchProbability Prediction = MBPI->getEdgeProbability(BB, TrueBBI.BB);

  if (CanRevCond && ValidDiamond(TrueBBI, FalseBBI, Dups, Dups2) &&
      MeetIfcvtSizeLimit(*TrueBBI.BB, (TrueBBI.NonPredSize - (Dups + Dups2) +
                                       TrueBBI.ExtraCost), TrueBBI.ExtraCost2,
                         *FalseBBI.BB, (FalseBBI.NonPredSize - (Dups + Dups2) +
                                        FalseBBI.ExtraCost),FalseBBI.ExtraCost2,
                         Prediction) &&
      FeasibilityAnalysis(TrueBBI, BBI.BrCond) &&
      FeasibilityAnalysis(FalseBBI, RevCond)) {
    // Diamond:
    //   EBB
    //   / \_
    //  |   |
    // TBB FBB
    //   \ /
    //  TailBB
    // Note TailBB can be empty.
    Tokens.push_back(new IfcvtToken(BBI, ICDiamond, TNeedSub|FNeedSub, Dups,
                                    Dups2));
    Enqueued = true;
  }

  if (ValidTriangle(TrueBBI, FalseBBI, false, Dups, Prediction) &&
      MeetIfcvtSizeLimit(*TrueBBI.BB, TrueBBI.NonPredSize + TrueBBI.ExtraCost,
                         TrueBBI.ExtraCost2, Prediction) &&
      FeasibilityAnalysis(TrueBBI, BBI.BrCond, true)) {
    // Triangle:
    //   EBB
    //   | \_
    //   |  |
    //   | TBB
    //   |  /
    //   FBB
    Tokens.push_back(new IfcvtToken(BBI, ICTriangle, TNeedSub, Dups));
    Enqueued = true;
  }

  if (ValidTriangle(TrueBBI, FalseBBI, true, Dups, Prediction) &&
      MeetIfcvtSizeLimit(*TrueBBI.BB, TrueBBI.NonPredSize + TrueBBI.ExtraCost,
                         TrueBBI.ExtraCost2, Prediction) &&
      FeasibilityAnalysis(TrueBBI, BBI.BrCond, true, true)) {
    Tokens.push_back(new IfcvtToken(BBI, ICTriangleRev, TNeedSub, Dups));
    Enqueued = true;
  }

  if (ValidSimple(TrueBBI, Dups, Prediction) &&
      MeetIfcvtSizeLimit(*TrueBBI.BB, TrueBBI.NonPredSize + TrueBBI.ExtraCost,
                         TrueBBI.ExtraCost2, Prediction) &&
      FeasibilityAnalysis(TrueBBI, BBI.BrCond)) {
    // Simple (split, no rejoin):
    //   EBB
    //   | \_
    //   |  |
    //   | TBB---> exit
    //   |
    //   FBB
    Tokens.push_back(new IfcvtToken(BBI, ICSimple, TNeedSub, Dups));
    Enqueued = true;
  }

  if (CanRevCond) {
    // Try the other path...
    if (ValidTriangle(FalseBBI, TrueBBI, false, Dups,
                      Prediction.getCompl()) &&
        MeetIfcvtSizeLimit(*FalseBBI.BB,
                           FalseBBI.NonPredSize + FalseBBI.ExtraCost,
                           FalseBBI.ExtraCost2, Prediction.getCompl()) &&
        FeasibilityAnalysis(FalseBBI, RevCond, true)) {
      Tokens.push_back(new IfcvtToken(BBI, ICTriangleFalse, FNeedSub, Dups));
      Enqueued = true;
    }

    if (ValidTriangle(FalseBBI, TrueBBI, true, Dups,
                      Prediction.getCompl()) &&
        MeetIfcvtSizeLimit(*FalseBBI.BB,
                           FalseBBI.NonPredSize + FalseBBI.ExtraCost,
                           FalseBBI.ExtraCost2, Prediction.getCompl()) &&
        FeasibilityAnalysis(FalseBBI, RevCond, true, true)) {
      Tokens.push_back(new IfcvtToken(BBI, ICTriangleFRev, FNeedSub, Dups));
      Enqueued = true;
    }

    if (ValidSimple(FalseBBI, Dups, Prediction.getCompl()) &&
        MeetIfcvtSizeLimit(*FalseBBI.BB,
                           FalseBBI.NonPredSize + FalseBBI.ExtraCost,
                           FalseBBI.ExtraCost2, Prediction.getCompl()) &&
        FeasibilityAnalysis(FalseBBI, RevCond)) {
      Tokens.push_back(new IfcvtToken(BBI, ICSimpleFalse, FNeedSub, Dups));
      Enqueued = true;
    }
  }

  BBI.IsEnqueued = Enqueued;
  BBI.IsBeingAnalyzed = false;
  BBI.IsAnalyzed = true;
  return BBI;
}

bool CSAIfConversion::IfConvertTriangle(BBInfo &BBI, IfcvtKind Kind) {
  BBInfo &TrueBBI = BBAnalysis[BBI.TrueBB->getNumber()];
  BBInfo &FalseBBI = BBAnalysis[BBI.FalseBB->getNumber()];
  BBInfo *CvtBBI = &TrueBBI;
  BBInfo *NextBBI = &FalseBBI;
  DebugLoc dl;  // FIXME: this is nowhere

  SmallVector<MachineOperand, 4> Cond(BBI.BrCond.begin(), BBI.BrCond.end());
  if (Kind == ICTriangleFalse || Kind == ICTriangleFRev)
    std::swap(CvtBBI, NextBBI);

  if (CvtBBI->IsDone ||
      (CvtBBI->CannotBeCopied && CvtBBI->BB->pred_size() > 1)) {
    // Something has changed. It's no longer safe to predicate this block.
    BBI.IsAnalyzed = false;
    CvtBBI->IsAnalyzed = false;
    return false;
  }

  if (CvtBBI->BB->hasAddressTaken())
    // Conservatively abort if-conversion if BB's address is taken.
    return false;

  if (Kind == ICTriangleFalse || Kind == ICTriangleFRev)
    if (TII->reverseBranchCondition(Cond))
      llvm_unreachable("Unable to reverse branch condition!");

  if (Kind == ICTriangleRev || Kind == ICTriangleFRev) {
    if (reverseBranchCondition(*CvtBBI)) {
      // BB has been changed, modify its predecessors (except for this
      // one) so they don't get ifcvt'ed based on bad intel.
      for (MachineBasicBlock::pred_iterator PI = CvtBBI->BB->pred_begin(),
             E = CvtBBI->BB->pred_end(); PI != E; ++PI) {
        MachineBasicBlock *PBB = *PI;
        if (PBB == BBI.BB)
          continue;
        BBInfo &PBBI = BBAnalysis[PBB->getNumber()];
        if (PBBI.IsEnqueued) {
          PBBI.IsAnalyzed = false;
          PBBI.IsEnqueued = false;
        }
      }
    }
  }


  /*if (CvtBBI->BB->pred_size() > 1) {
    BBI.NonPredSize -= TII->removeBranch(*BBI.BB);
    // Copy instructions in the true block, predicate them, and add them to
    // the entry block.
    //CopyAndPredicateBlock(BBI, *CvtBBI, Cond, true);

    // RemoveExtraEdges won't work if the block has an unanalyzable branch, so
    // explicitly remove CvtBBI as a successor.
    BBI.BB->removeSuccessor(CvtBBI->BB);
  } else {
    // Predicate the 'true' block after removing its branch.
    CvtBBI->NonPredSize -= TII->removeBranch(*CvtBBI->BB);
    //PredicateBlock(*CvtBBI, CvtBBI->BB->end(), Cond);

    // Now merge the entry of the triangle with the true block.
    BBI.NonPredSize -= TII->removeBranch(*BBI.BB);
    MergeBlocks(BBI, *CvtBBI, false);
  }*/


  return true;
}

