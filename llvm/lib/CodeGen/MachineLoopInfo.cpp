//===- MachineLoopInfo.cpp - Natural Loop Calculator ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the MachineLoopInfo class that is used to identify natural
// loops and determine the loop depth of various nodes of the CFG.  Note that
// the loops identified may actually be several natural loops that share the
// same header node... not just a single natural loop.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/Analysis/LoopInfoImpl.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#ifdef INTEL_CUSTOMIZATION
#define DEBUG_TYPE "machine-loop-info"
#endif  // INTEL_CUSTOMIZATION

// Explicitly instantiate methods in LoopInfoImpl.h for MI-level Loops.
template class llvm::LoopBase<MachineBasicBlock, MachineLoop>;
template class llvm::LoopInfoBase<MachineBasicBlock, MachineLoop>;

char MachineLoopInfo::ID = 0;
INITIALIZE_PASS_BEGIN(MachineLoopInfo, "machine-loops",
                "Machine Natural Loop Construction", true, true)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_END(MachineLoopInfo, "machine-loops",
                "Machine Natural Loop Construction", true, true)

char &llvm::MachineLoopInfoID = MachineLoopInfo::ID;

bool MachineLoopInfo::runOnMachineFunction(MachineFunction &) {
  releaseMemory();
  LI.analyze(getAnalysis<MachineDominatorTree>().getBase());
  return false;
}

void MachineLoopInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineDominatorTree>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

MachineBasicBlock *MachineLoop::getTopBlock() {
  MachineBasicBlock *TopMBB = getHeader();
  MachineFunction::iterator Begin = TopMBB->getParent()->begin();
  if (TopMBB->getIterator() != Begin) {
    MachineBasicBlock *PriorMBB = &*std::prev(TopMBB->getIterator());
    while (contains(PriorMBB)) {
      TopMBB = PriorMBB;
      if (TopMBB->getIterator() == Begin)
        break;
      PriorMBB = &*std::prev(TopMBB->getIterator());
    }
  }
  return TopMBB;
}

MachineBasicBlock *MachineLoop::getBottomBlock() {
  MachineBasicBlock *BotMBB = getHeader();
  MachineFunction::iterator End = BotMBB->getParent()->end();
  if (BotMBB->getIterator() != std::prev(End)) {
    MachineBasicBlock *NextMBB = &*std::next(BotMBB->getIterator());
    while (contains(NextMBB)) {
      BotMBB = NextMBB;
      if (BotMBB == &*std::next(BotMBB->getIterator()))
        break;
      NextMBB = &*std::next(BotMBB->getIterator());
    }
  }
  return BotMBB;
}

MachineBasicBlock *MachineLoop::findLoopControlBlock() const { // INTEL
  if (MachineBasicBlock *Latch = getLoopLatch()) {
    if (isLoopExiting(Latch))
      return Latch;
    else
      return getExitingBlock();
  }
  return nullptr;
}

DebugLoc MachineLoop::getStartLoc() const {
  // Try the pre-header first.
  if (MachineBasicBlock *PHeadMBB = getLoopPreheader())
    if (const BasicBlock *PHeadBB = PHeadMBB->getBasicBlock())
      if (DebugLoc DL = PHeadBB->getTerminator()->getDebugLoc())
        return DL;

  // If we have no pre-header or there are no instructions with debug
  // info in it, try the header.
  if (MachineBasicBlock *HeadMBB = getHeader())
    if (const BasicBlock *HeadBB = HeadMBB->getBasicBlock())
      return HeadBB->getTerminator()->getDebugLoc();

  return DebugLoc();
}

MachineBasicBlock *
MachineLoopInfo::findLoopPreheader(MachineLoop *L,
                                   bool SpeculativePreheader) const {
  if (MachineBasicBlock *PB = L->getLoopPreheader())
    return PB;

  if (!SpeculativePreheader)
    return nullptr;

  MachineBasicBlock *HB = L->getHeader(), *LB = L->getLoopLatch();
  if (HB->pred_size() != 2 || HB->hasAddressTaken())
    return nullptr;
  // Find the predecessor of the header that is not the latch block.
  MachineBasicBlock *Preheader = nullptr;
  for (MachineBasicBlock *P : HB->predecessors()) {
    if (P == LB)
      continue;
    // Sanity.
    if (Preheader)
      return nullptr;
    Preheader = P;
  }

  // Check if the preheader candidate is a successor of any other loop
  // headers. We want to avoid having two loop setups in the same block.
  for (MachineBasicBlock *S : Preheader->successors()) {
    if (S == HB)
      continue;
    MachineLoop *T = getLoopFor(S);
    if (T && T->getHeader() == S)
      return nullptr;
  }
  return Preheader;
}

#ifdef INTEL_CUSTOMIZATION
MDNode *MachineLoop::getLoopID() const {
  MDNode *LoopID = nullptr;

  if (auto *MBB = findLoopControlBlock()) {
    // If there is a single latch block, then the metadata
    // node is attached to its terminating instruction.
    const auto *BB = MBB->getBasicBlock();
    if (const auto *TI = BB->getTerminator()) {
      LoopID = TI->getMetadata(LLVMContext::MD_loop);
      DEBUG(dbgs() << "Fetched MD_loop from the MachineLoop's single latch.");
    }
  } else if (auto *MBB = getHeader()) {
    // There seem to be multiple latch blocks, so we have to
    // visit all predecessors of the loop header and check
    // their terminating instructions for the metadata.
    if (const auto *H = MBB->getBasicBlock()) {
      // Walk over all blocks in the loop.
      for (auto *MBB : this->blocks()) {
        const auto *BB = MBB->getBasicBlock();

        if (!BB) {
          DEBUG(dbgs() << "Invalid MachineBasicBlock -> BasicBlock mapping.");
          return nullptr;
        }

        const auto *TI = BB->getTerminator();

        if (!TI) {
          DEBUG(dbgs() << "Invalid (nullptr) terminating instruction.");
          return nullptr;
        }

        MDNode *MD = nullptr;

        // Check if this terminating instruction jumps to the loop header.
        for (const auto *S : TI->successors()) {
          if (S == H) {
            // This is a jump to the header - gather the metadata from it.
            MD = TI->getMetadata(LLVMContext::MD_loop);
            DEBUG(dbgs() << "Fetched MD_loop from the MachineLoop's latch.");
            break;
          }
        }

        if (!MD) {
          DEBUG(dbgs() << "Dropped inconsistent MD_loop (nullptr).");
          return nullptr;
        }

        if (!LoopID)
          LoopID = MD;
        else if (MD != LoopID) {
          DEBUG(dbgs() << "Dropped inconsistent MD_loop (mismatch).");
          return nullptr;
        }
      }
    }
  }

  if (LoopID &&
      (LoopID->getNumOperands() == 0 || LoopID->getOperand(0) != LoopID)) {
    LoopID = nullptr;
    DEBUG(dbgs() << "Dropped inconsistent MD_loop (self-ref).");
  }

  if (!LoopID)
    DEBUG(dbgs() << "Returning nullptr MD_loop.");
  else
    DEBUG(dbgs() << "Returning valid MD_loop.");

  return LoopID;
}
#endif  // INTEL_CUSTOMIZATION

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void MachineLoop::dump() const {
  print(dbgs());
}
#endif
