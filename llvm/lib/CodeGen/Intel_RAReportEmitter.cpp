//===- RAReportEmitter.cpp - Prints Register Allocation reports -------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Register Allocation Report emitter.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define DEBUG_TYPE "intel-ra-report-emitter"

using namespace llvm;

namespace RAReportVerbosity {
  enum Level { None = 0, Low = 1, Medium = 2, High = 3 };
}

static cl::opt<RAReportVerbosity::Level> RAReportVerbosityOption(
    "intel-ra-spillreport",
    cl::desc("Option for enabling the RA spill/reload report "
             "and controling its verbosity"),
    cl::init(RAReportVerbosity::None),
    cl::values(
        clEnumValN(RAReportVerbosity::None, "none",
                   "RA spill/reload report is disabled"),
        clEnumValN(RAReportVerbosity::Low, "low",
                   "Only generate RA spill/reload summary report for each loop"),
        clEnumValN(RAReportVerbosity::Medium, "medium",
                   "Low + generate details about each spill/reload instruction"),
        clEnumValN(RAReportVerbosity::High, "high",
                   "Medium + all extra details about RA")));

namespace {
struct RAReportEmitter : public MachineFunctionPass {
  static char ID;
  MachineFunction *MF;
  MachineLoopInfo *Loops;
  MachineDominatorTree *DomTree;

  RAReportEmitter() : MachineFunctionPass(ID) {
    initializeRAReportEmitterPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &F) override;

  // We don't modify the program, so we preserve all analyses.
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  // Record instruction level spill/reload information
  struct SpillInstr {
    bool Reload;
    bool Folded;
    unsigned Size;
    int Slot;
    MachineInstr *MI;
    MachineBasicBlock *MBB;
    SpillInstr() : Reload(false), Folded(false), Size(0), Slot(-1),
        MI(nullptr), MBB(nullptr) {}
  };

  // Record instruction or loop level spill/reload information
  struct SpillNode;

  // Record loop level spill/reload information
  struct SpillLoop {
    // All spill/reload information contained in this loop
    SmallVector< std::shared_ptr<SpillNode>, 4> Nodes;
    // Loop header
    MachineBasicBlock *Header;
    MachineLoop *Loop;
    // Loop depth
    unsigned Depth;
    // Statistics for this loop
    unsigned NumReloads;
    unsigned NumSpills;

    SpillLoop(): Header(nullptr), Loop(nullptr), Depth(0),
      NumReloads(0), NumSpills(0) {}

    SpillLoop(MachineBasicBlock *H, MachineLoop *L, unsigned D)
      : Header(H), Loop(L), Depth(D), NumReloads(0),
        NumSpills(0) {}

    void addSpillNode(SpillNode *SN) {
      Nodes.push_back(std::shared_ptr<SpillNode>(SN));
    }

    void print(raw_ostream &OS) const;
  };

  // Record instruction or loop level spill/reload information
  struct SpillNode {
    bool IsLoop;
    SpillInstr Instr;
    SpillLoop Loop;

    SpillNode(SpillLoop SL)
      : IsLoop(true) { Loop = SL; }

    SpillNode(SpillInstr SI)
      : IsLoop(false) { Instr = SI; }

    // Decide display order of two Nodes based on domination
    bool before(const SpillNode &B, MachineDominatorTree &MDT) {
      MachineBasicBlock *BBA = IsLoop ? Loop.Header : Instr.MBB;
      MachineBasicBlock *BBB = B.IsLoop ? B.Loop.Header : B.Instr.MBB;
      if (BBA == BBB) {
        // Both Nodes are instructions
        assert(!IsLoop && !B.IsLoop);
        return MDT.dominates(Instr.MI, B.Instr.MI);
      }
      return MDT.dominates(BBA, BBB);
    }
  };

  /// Analyze the spills and reloads for the loop: L and all its sub loops.
  SpillNode AnalyzeLoopSpillRecursive(MachineLoop *L, unsigned Depth);
};

bool RAReportEmitter::runOnMachineFunction(MachineFunction &MFunc) {
  if (RAReportVerbosityOption < RAReportVerbosity::Low)
    return false;

  MF = &MFunc;
  Loops = &getAnalysis<MachineLoopInfo>();
  DomTree = &getAnalysis<MachineDominatorTree>();
  raw_ostream &OS = OptReportOptions::getOutputStream();
  OS << "Register allocation report for: " << MF->getName() << "\n";
  OS << "FUNCTION BEGIN\n";
  SpillNode SN = AnalyzeLoopSpillRecursive(nullptr, 0);
  assert(SN.IsLoop);
  SN.Loop.print(OS);
  OS << "FUNCTION END\n";
  return false;
}

void RAReportEmitter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineLoopInfo>();
  AU.addRequired<MachineDominatorTree>();
  MachineFunctionPass::getAnalysisUsage(AU);
}
} // namespace

RAReportEmitter::SpillNode RAReportEmitter::AnalyzeLoopSpillRecursive(
    MachineLoop *L, unsigned Depth) {
  SpillLoop SL;
  SL.Depth = Depth;
  SL.Header = L ? L->getHeader(): nullptr;
  SL.Loop = L;
  SL.NumReloads = 0;
  SL.NumSpills = 0;
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  const TargetInstrInfo *TII = MF->getSubtarget().getInstrInfo();

  // Collect candidate basic blocks for current Loop
  // BB not contained by any loop is collected into the virtual loop of depth: 0
  SmallVector<MachineBasicBlock*, 4> MBBS;
  if (L) {
    for (MachineBasicBlock *MBB : L->getBlocks())
      if (Loops->getLoopFor(MBB) == L)
        MBBS.push_back(MBB);
  } else {
    for (MachineBasicBlock &MBB : *MF)
      if (!Loops->getLoopFor(&MBB))
        MBBS.push_back(&MBB);
  }

  for (MachineBasicBlock *MBB : MBBS)
    for (MachineInstr &MI : *MBB) {
      SmallVector<const MachineMemOperand *, 2> Accesses;
      int AFI = 0;
      unsigned Sz = 0;
      auto isSpillSlotAccess = [&MFI, &AFI, &Sz](const MachineMemOperand *A) {
        Sz = A->getSize();
        AFI = cast<FixedStackPseudoSourceValue>(A->getPseudoValue())
            ->getFrameIndex();
        return MFI.isSpillSlotObjectIndex(AFI);
      };

      int FI;
      SpillInstr SI;
      if (TII->isLoadFromStackSlot(MI, FI) && MFI.isSpillSlotObjectIndex(FI)) {
        SL.NumReloads++;
        SI.Folded = false;
        SI.Reload = true;
        SI.Size = *MI.getRestoreSize(TII);
      } else if (TII->hasLoadFromStackSlot(MI, Accesses) &&
                 llvm::any_of(Accesses, isSpillSlotAccess)) {
        SL.NumReloads++;
        SI.Folded = true;
        SI.Reload = true;
        FI = AFI;
        SI.Size = Sz;
      } else if (TII->isStoreToStackSlot(MI, FI) &&
                 MFI.isSpillSlotObjectIndex(FI)) {
        SL.NumSpills++;
        SI.Folded = false;
        SI.Reload = false;
        SI.Size = *MI.getSpillSize(TII);
      } else if (TII->hasStoreToStackSlot(MI, Accesses) &&
                 llvm::any_of(Accesses, isSpillSlotAccess)) {
        SL.NumSpills++;
        SI.Folded = true;
        SI.Reload = false;
        FI = AFI;
        SI.Size = Sz;
      } else
        continue;

      SI.MBB = MBB;
      SI.MI = &MI;
      SI.Slot = FI;
      SL.addSpillNode(new SpillNode(SI));
    }

  if (L) {
    // Handle subloops.
    for (MachineLoop *SubLoop : *L) {
      SL.addSpillNode(new SpillNode(
          AnalyzeLoopSpillRecursive(SubLoop, Depth+1)));
    }
  } else {
    // Handle function top loops.
    for (MachineLoop *SubLoop : *Loops) {
      SL.addSpillNode(new SpillNode(
          AnalyzeLoopSpillRecursive(SubLoop, Depth+1)));
    }
  }

  // Decide spill/reload node display order based on domination
  auto Cmp = [this](const std::shared_ptr<SpillNode> &A,
                    const std::shared_ptr<SpillNode> &B) {
    return A.get()->before(*(B.get()), *DomTree);
  };
  llvm::stable_sort(SL.Nodes, Cmp);
  return SpillNode(std::move(SL));
}

// Display loop level spill/reload informations
void RAReportEmitter::SpillLoop::print(raw_ostream &OS) const {
  if (Loop) {
    OS.indent(Depth);
    OS << "LOOP" << Depth << " BEGIN at (";
    Loop->getStartLoc().print(OS);
    OS << ")\n";
  }

  if (NumReloads || NumSpills) {
    OS.indent(Depth+1);
    if (NumReloads)
      OS << NumReloads << " reloads ";
    if (NumSpills)
      OS << NumSpills << " spills ";
    OS << "\n";
  }

  for (auto const &It: Nodes) {
    const SpillNode *N = It.get();
    if (N->IsLoop) {
      const SpillLoop &SL = N->Loop;
      SL.print(OS);
    } else {
      const SpillInstr &SI = N->Instr;
      if (RAReportVerbosityOption >= RAReportVerbosity::Medium) {
        OS.indent(Depth+1);
        OS << (SI.Folded ? "folded ": "")
            << (SI.Reload ? "reload ": "spill ")
            << SI.Size << " byte -- slot: " << SI.Slot << "\n";
      }
    }
  }

  if (Loop) {
    OS.indent(Depth);
    OS << "LOOP" << Depth << " END\n";
  }
}

char RAReportEmitter::ID = 0;
char &llvm::RAReportEmitterID = RAReportEmitter::ID;
static const char ra_spill_name[] = "The pass emits RA spill reports";

INITIALIZE_PASS_BEGIN(RAReportEmitter, DEBUG_TYPE,
    ra_spill_name, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(RAReportEmitter, DEBUG_TYPE,
    ra_spill_name, false, false)
