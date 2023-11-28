//===--------- Intel_X86SplitLongBlock.cpp - X86 Split Long Block ---------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass breaks long straight-line basic blocks using jmps to make it more
// friendly to the frontend of cores.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "x86-split-long-block"

// Several performance regressions were observed after applying this pass,
// they haven't been root-caused yet. This heuristic is added to make sure the
// optimization only applies to really long blocks, as they are more likely to
// to create front-end bottlenecks.
// TODO: Replace the restriction on length with more sophisticated analysis.
static cl::opt<unsigned> SplitLongBlockMinLength(
    "x86-split-long-block-min-length",
    cl::desc("Minimum length of block required to trigger split long block, "
             "set to 0 to apply to all blocks."),
    cl::init(2560), cl::Hidden);

static cl::opt<unsigned> SplitLongBlockBlockLength(
    "x86-split-long-block-block-length",
    cl::desc("How many instructions a split block will have, set to 0 to "
             "disable long block split."),
    cl::init(512), cl::Hidden);

namespace {

class X86SplitLongBlockPass : public MachineFunctionPass {
public:
  X86SplitLongBlockPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  static char ID;
};

}

char X86SplitLongBlockPass::ID = 0;

FunctionPass *llvm::createX86SplitLongBlockPass() {
  return new X86SplitLongBlockPass();
}
INITIALIZE_PASS(X86SplitLongBlockPass, DEBUG_TYPE, "X86 Split Long Blocks",
                false, false)

static unsigned numberOfInstruction(const MachineBasicBlock *MBB) {
  unsigned Result = 0;
  for (const MachineInstr &MI: *MBB) {
    (void) MI;
    Result++;
  }
  return Result;
}

bool X86SplitLongBlockPass::runOnMachineFunction(MachineFunction &MF) {
  const TargetMachine &TM = MF.getTarget();
  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  if (skipFunction(MF.getFunction()) || SplitLongBlockBlockLength == 0)
    return false;
  if (!(TM.getOptLevel() == CodeGenOptLevel::Aggressive &&
        TM.Options.IntelAdvancedOptim))
    return false;
  // Currently only enabled for SKL and earlier
  if (!ST.hasDSB() || ST.has2KDSB())
    return false;
  bool Changed = false;

  SmallVector<MachineBasicBlock *, 4> Worklist;
  for (MachineBasicBlock &MBB : MF)
    if (SplitLongBlockMinLength == 0 ||
        numberOfInstruction(&MBB) >= SplitLongBlockMinLength)
      Worklist.push_back(&MBB);

  // TODO: Currently we split MBBs solely based on number of instructions. In
  // the future, it can be enhanced with more inspection to each instruction to
  // determine whether there is a front end bottleneck.
  while (!Worklist.empty()) {
    MachineBasicBlock *MBB = Worklist.back();
    Worklist.pop_back();
    auto I = MBB->begin();
    for (unsigned J = 0; I != MBB->end(); ++I)
      if (!(I->isDebugInstr() || I->isCFIInstruction())) {
        ++J;
        if (J >= SplitLongBlockBlockLength)
          break;
      }
    if (I == MBB->end() || I->isBranch() || I->isReturn())
      continue;
    MachineBasicBlock *NewMBB = nullptr;
    NewMBB = MBB->splitAt(*I, true);
    const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
    TII.insertUnconditionalBranch(*MBB, NewMBB, DebugLoc());
    Changed = true;
    Worklist.push_back(NewMBB);
  }

  return Changed;
}
