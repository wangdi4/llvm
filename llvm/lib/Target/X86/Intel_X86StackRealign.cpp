//===- Intel_X86StackRealign.cpp - Enable X86 stack realignment on demand  ===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass enable stack realignment using heuristics to estimate extra unalign
// cost of register spill and reload.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterPressure.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "x86-stack-realign"

static cl::opt<bool> DisablePass("disable-x86-stack-realign", cl::Hidden,
                                 cl::init(false),
                                 cl::desc("Disable x86 stack realign pass"));

static cl::opt<double> AvgCostThreshold(
    "x86-stack-realign-avg-cost-threshold", cl::Hidden, cl::init(2.9),
    cl::desc("Min average cost that stack realign will handle"));

namespace {

class X86StackRealign : public MachineFunctionPass {
  MachineLoopInfo *MLI = nullptr;
  const TargetRegisterInfo *TRI = nullptr;
  const TargetFrameLowering *TFI = nullptr;
  const MachineBlockFrequencyInfo *MBFI = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  LiveIntervals *LIS = nullptr;
  RegisterClassInfo RCI;

public:
  static char ID;
  X86StackRealign() : MachineFunctionPass(ID) {}
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnMachineFunction(MachineFunction &MF) override;

  // Estimate extra unalign cost of spill/reload.
  double computeMBBCost(MachineBasicBlock *MBB,
                        unsigned *NumNoDbgInst = nullptr);
};

} // end anonymous namespace

char X86StackRealign::ID;

void X86StackRealign::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineLoopInfo>();
  AU.addRequired<LiveIntervals>();
  AU.addRequired<MachineBlockFrequencyInfo>();
  MachineFunctionPass::getAnalysisUsage(AU);
  AU.setPreservesAll();
}

double X86StackRealign::computeMBBCost(MachineBasicBlock *MBB,
                                       unsigned *NumNoDbgInst) {
  if (NumNoDbgInst)
    *NumNoDbgInst = 0;

  if (MBB->getFirstNonDebugInstr() == MBB->end())
    return 0;

  double Cost = 0.0;
  const MachineFunction *MF = MBB->getParent();
  IntervalPressure Pressure, TopPressure;
  RegPressureTracker RPTracker(Pressure), TopRPTracker(TopPressure);

  // RPTracker tracks all live-in/live-out as well as live through.
  RPTracker.init(MF, &RCI, LIS, MBB, MBB->end(),
                 /*TrackLaneMasks=*/false, /*TrackUntiedDefs=*/true);
  do {
    RPTracker.recede();
  } while (RPTracker.getPos() != MBB->begin());
  RPTracker.closeRegion();

  // TopPressure tracks real time register pressure to calculate extra cost of
  // spill/reload stack object which may be not aligned.
  // We define unalign cost of each spill/reload as 1.
  TopRPTracker.init(MF, &RCI, LIS, MBB, MBB->begin(),
                    /*TrackLaneMasks=*/false, /*TrackUntiedDefs=*/false);
  TopRPTracker.addLiveRegs(RPTracker.getPressure().LiveInRegs);
  TopRPTracker.closeTop();
  TopRPTracker.initLiveThru(RPTracker.getLiveThru());
  for (MachineBasicBlock::iterator MII = next_nodbg(MBB->begin(), MBB->end()),
                                   MIE = MBB->end();
       MII != MIE; MII = next_nodbg(MII, MBB->end())) {
    if (NumNoDbgInst)
      (*NumNoDbgInst)++;

    RegisterOperands RegOpers;
    RegOpers.collect(*MII, *TRI, *MRI, /*TrackLaneMasks=*/false,
                     /*IgnoreDead=*/false);
    RegOpers.detectDeadDefs(*MII, *LIS);

    // All uses are unique and must be in LiveRegs so it won't increase register
    // pressure of TopRPTracker.
    for (const RegisterMaskPair &P : RegOpers.Uses) {
      if (!P.RegUnit.isVirtual())
        continue;
      const TargetRegisterClass *RC = MRI->getRegClass(P.RegUnit);
      if (TFI->getStackAlign() >= TRI->getSpillAlign(*RC))
        continue;

      // If register pressure of uses exceed limit, then this uses may be in
      // stack and RA need to spill one live register then reload the uses from
      // stack. We use (1 - pressure limit / current pressure) to represent its
      // probability.
      for (PSetIterator PSetI = MRI->getPressureSets(P.RegUnit);
           PSetI.isValid(); ++PSetI) {
        unsigned Curr = TopRPTracker.getRegSetPressureAtPos()[*PSetI];
        unsigned Limit = RCI.getRegPressureSetLimit(*PSetI);
        if (Curr > Limit) {
          Cost += 2.0 * (1 - static_cast<double>(Limit) / Curr);
          break;
        }
      }
    }

    // New defined defs increase register pressure. If current pressure exceeds
    // limit, new defs need to spill one live register. Since uses may decrease
    // register pressure, we need to first advance uses, then find out which def
    // exceeds limit when advance each def. Here we record defs liveness before
    // advance, advance through RegOpers, then upward pressure changes of defs
    // to find out those defs which exeeds pressure limit.
    SmallVector<LaneBitmask, 1> PrevMasks;
    for (const RegisterMaskPair &P : RegOpers.Defs)
      PrevMasks.push_back(TopRPTracker.getLiveRegs().contains(P.RegUnit));
    TopRPTracker.advance(RegOpers);
    std::vector<unsigned> SetPressure = TopRPTracker.getRegSetPressureAtPos();
    for (unsigned I = 0, E = RegOpers.Defs.size(); I != E; I++) {
      const RegisterMaskPair &P = RegOpers.Defs[I];
      if (PrevMasks[I].any() || P.LaneMask.none())
        continue;

      bool IsExceed = false;
      PSetIterator PSetI = MRI->getPressureSets(P.RegUnit);
      unsigned Weight = PSetI.getWeight();
      const TargetRegisterClass *RC =
          P.RegUnit.isVirtual() ? MRI->getRegClass(P.RegUnit) : nullptr;
      for (; PSetI.isValid(); ++PSetI) {
        unsigned Curr = SetPressure[*PSetI];
        assert(Curr >= Weight && "Must be not less than weight");
        SetPressure[*PSetI] -= Weight;
        if (RC && TFI->getStackAlign() < TRI->getSpillAlign(*RC) &&
            Curr > RCI.getRegPressureSetLimit(*PSetI))
          IsExceed = true;
      }
      if (IsExceed)
        Cost += 1.0;
    }

    // Dead defs also occupy register and may "create" spill but they are not in
    // LiveRegs.
    SetPressure = TopRPTracker.getRegSetPressureAtPos();
    for (const RegisterMaskPair &P : RegOpers.DeadDefs) {
      if (TopRPTracker.getLiveRegs().contains(P.RegUnit).any() ||
          P.LaneMask.none())
        continue;

      bool IsExceed = false;
      PSetIterator PSetI = MRI->getPressureSets(P.RegUnit);
      unsigned Weight = PSetI.getWeight();
      const TargetRegisterClass *RC =
          P.RegUnit.isVirtual() ? MRI->getRegClass(P.RegUnit) : nullptr;
      for (; PSetI.isValid(); ++PSetI) {
        SetPressure[*PSetI] += Weight;
        if (RC && TFI->getStackAlign() < TRI->getSpillAlign(*RC) &&
            SetPressure[*PSetI] > RCI.getRegPressureSetLimit(*PSetI))
          IsExceed = true;
      }
      if (IsExceed)
        Cost += 1.0;
    }
  }
  return Cost;
}

bool X86StackRealign::runOnMachineFunction(MachineFunction &MF) {
  if (DisablePass || skipFunction(MF.getFunction()))
    return false;

  TFI = MF.getSubtarget().getFrameLowering();
  if (TFI->hasFP(MF) || !MF.getFrameInfo().getX86StackRealignable())
    return false;

  TRI = MF.getSubtarget().getRegisterInfo();
  MLI = &getAnalysis<MachineLoopInfo>();
  MRI = &MF.getRegInfo();
  LIS = &getAnalysis<LiveIntervals>();
  MBFI = &getAnalysis<MachineBlockFrequencyInfo>();
  RCI.runOnMachineFunction(MF);

  // TotalCost is total unalign cost of spill/reload weighted by block
  // frequency.
  double TotalCost = 0.0, InstRetired = 0.0;
  for (MachineBasicBlock &MBB : MF) {
    unsigned NumNoDbgInst;
    uint64_t RelFrequency =
        MBFI->getBlockFreq(&MBB).getFrequency() / MBFI->getEntryFreq();
    TotalCost += computeMBBCost(&MBB, &NumNoDbgInst) * RelFrequency;
    InstRetired += static_cast<double>(RelFrequency) * NumNoDbgInst;
  }

  double AvgCost = TotalCost / InstRetired;

#ifndef NDEBUG
  if (TotalCost > 0) {
    LLVM_DEBUG(dbgs() << "Unalign cost info of " << MF.getName() << ":\n"
                      << "  Total cost = " << TotalCost << "\n"
                      << "  InstRetired = " << InstRetired << "\n"
                      << "  Avg cost = " << AvgCost << "\n");
  }
#endif

  // Rreserve frame-pointer before RA. PEI will automatically realign stack if
  // frame-pointer is reserved and max alignment of stack objects exceeds
  // default stack alignment.
  bool ShouldRealignStack = AvgCost >= AvgCostThreshold;
  if (ShouldRealignStack) {
    LLVM_DEBUG(dbgs() << "Emit frame-pointer=all for " << MF.getName() << "\n");
    MF.getFunction().addFnAttr("frame-pointer", "all");
  }

  return ShouldRealignStack;
}

INITIALIZE_PASS_BEGIN(X86StackRealign, DEBUG_TYPE, "X86 stack realign", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(LiveIntervals)
INITIALIZE_PASS_DEPENDENCY(MachineBlockFrequencyInfo)
INITIALIZE_PASS_END(X86StackRealign, DEBUG_TYPE, "X86 stack realign", false,
                    false)

FunctionPass *llvm::createX86StackRealignPass() { return new X86StackRealign; }
