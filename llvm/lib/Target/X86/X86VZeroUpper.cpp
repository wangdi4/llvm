//===- X86VZeroUpper.cpp - AVX vzeroupper instruction inserter ------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the pass which inserts x86 AVX vzeroupper instructions
// before calls to SSE encoded functions. This avoids transition latency
// penalty when transferring control between AVX encoded instructions and old
// SSE encoding mode.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

#if INTEL_CUSTOMIZATION
#include "llvm/CodeGen/LivePhysRegs.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

#define DEBUG_TYPE "x86-vzeroupper"

static cl::opt<bool>
UseVZeroUpper("x86-use-vzeroupper", cl::Hidden,
  cl::desc("Minimize AVX to SSE transition penalty"),
  cl::init(true));

STATISTIC(NumVZU, "Number of vzeroupper instructions inserted");

namespace {

  class VZeroUpperInserter : public MachineFunctionPass {
  public:
    VZeroUpperInserter() : MachineFunctionPass(ID) {}

    bool runOnMachineFunction(MachineFunction &MF) override;

    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().set(
          MachineFunctionProperties::Property::NoVRegs);
    }

    StringRef getPassName() const override { return "X86 vzeroupper inserter"; }

  private:
    void processBasicBlock(MachineBasicBlock &MBB);
    void insertVZeroUpper(MachineBasicBlock::iterator I,
                          MachineBasicBlock &MBB);
    void addDirtySuccessor(MachineBasicBlock &MBB);

    using BlockExitState = enum { PASS_THROUGH, EXITS_CLEAN, EXITS_DIRTY };

    static const char* getBlockExitStateName(BlockExitState ST);

    // Core algorithm state:
    // BlockState - Each block is either:
    //   - PASS_THROUGH: There are neither YMM/ZMM dirtying instructions nor
    //                   vzeroupper instructions in this block.
    //   - EXITS_CLEAN: There is (or will be) a vzeroupper instruction in this
    //                  block that will ensure that YMM/ZMM is clean on exit.
    //   - EXITS_DIRTY: An instruction in the block dirties YMM/ZMM and no
    //                  subsequent vzeroupper in the block clears it.
    //
    // AddedToDirtySuccessors - This flag is raised when a block is added to the
    //                          DirtySuccessors list to ensure that it's not
    //                          added multiple times.
    //
#if INTEL_CUSTOMIZATION
    // FirstUnguardedCallOrSSEInstruction - Records the location of the first
    //                                      unguarded call or SSE instruction
    //                                      in each basic block that may need
    //                                      to be guarded by a vzeroupper. We
    //                                      won't know whether it actually
    //                                      needs to be guarded until we
    //                                      discover a predecessor that is
    //                                      DIRTY_OUT.
#endif // INTEL_CUSTOMIZATION
    struct BlockState {
      BlockExitState ExitState = PASS_THROUGH;
      bool AddedToDirtySuccessors = false;
#if INTEL_CUSTOMIZATION
      MachineBasicBlock::iterator FirstUnguardedCallOrSSEInstruction;
#endif // INTEL_CUSTOMIZATION

      BlockState() = default;
    };

    using BlockStateMap = SmallVector<BlockState, 8>;
    using DirtySuccessorsWorkList = SmallVector<MachineBasicBlock *, 8>;

    BlockStateMap BlockStates;
    DirtySuccessorsWorkList DirtySuccessors;
    bool EverMadeChange;
#if INTEL_CUSTOMIZATION
    bool HasYmmOrZmmCSR;
#endif // INTEL_CUSTOMIZATION
    const TargetInstrInfo *TII;

    static char ID;
  };

} // end anonymous namespace

char VZeroUpperInserter::ID = 0;

FunctionPass *llvm::createX86IssueVZeroUpperPass() {
  return new VZeroUpperInserter();
}

#ifndef NDEBUG
const char* VZeroUpperInserter::getBlockExitStateName(BlockExitState ST) {
  switch (ST) {
    case PASS_THROUGH: return "Pass-through";
    case EXITS_DIRTY: return "Exits-dirty";
    case EXITS_CLEAN: return "Exits-clean";
  }
  llvm_unreachable("Invalid block exit state.");
}
#endif

/// VZEROUPPER cleans state that is related to Y/ZMM0-15 only.
/// Thus, there is no need to check for Y/ZMM16 and above.
static bool isYmmOrZmmReg(unsigned Reg) {
  return (Reg >= X86::YMM0 && Reg <= X86::YMM15) ||
         (Reg >= X86::ZMM0 && Reg <= X86::ZMM15);
}

static bool checkFnHasLiveInYmmOrZmm(MachineRegisterInfo &MRI) {
  for (std::pair<unsigned, unsigned> LI : MRI.liveins())
    if (isYmmOrZmmReg(LI.first))
      return true;

  return false;
}

static bool clobbersAllYmmAndZmmRegs(const MachineOperand &MO) {
  for (unsigned reg = X86::YMM0; reg <= X86::YMM15; ++reg) {
    if (!MO.clobbersPhysReg(reg))
      return false;
  }
  for (unsigned reg = X86::ZMM0; reg <= X86::ZMM15; ++reg) {
    if (!MO.clobbersPhysReg(reg))
      return false;
  }
  return true;
}

static bool hasYmmOrZmmReg(MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (MI.isCall() && MO.isRegMask() && !clobbersAllYmmAndZmmRegs(MO))
      return true;
    if (!MO.isReg())
      continue;
    if (MO.isDebug())
      continue;
    if (isYmmOrZmmReg(MO.getReg()))
      return true;
  }
  return false;
}

#if INTEL_CUSTOMIZATION
static bool isXmmReg(unsigned Reg) {
  return Reg >= X86::XMM0 && Reg <= X86::XMM15;
}

static bool clobbersAllXmmRegs(const MachineOperand &MO) {
  for (unsigned Reg = X86::XMM0; Reg <= X86::XMM15; ++Reg)
    if (!MO.clobbersPhysReg(Reg))
      return false;
  return true;
}

static bool hasXmmReg(const MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (MI.isCall() && MO.isRegMask() && !clobbersAllXmmRegs(MO))
      return true;
    if (!MO.isReg())
      continue;
    if (MO.isDebug())
      continue;
    if (isXmmReg(MO.getReg()))
      return true;
  }
  return false;
}

// Determine whether an instruction is an SSE instruction (not using the more
// recent VEX/EVEX encodings and may incur penalty in runtime)
static bool isSSEInstruction(const MachineInstr &MI) {
  // An instruction with a XMM operand that is not using VEX/EVEX is an SSE
  // instruction
  // Instructions with generic opcodes are ignored since most of them in this
  // stage are noop. Inline asm is an exception but there is no way to know
  // what's inside.
  // Calls and returns may also carry XMM operands but they're not SSE
  // instructions.
  uint64_t TSFlags = MI.getDesc().TSFlags;
  if (TSFlags & X86II::VEX || TSFlags & X86II::EVEX)
    return false;
  if (MI.isCall() || MI.isReturn() || !isTargetSpecificOpcode(MI.getOpcode()))
    return false;
  return hasXmmReg(MI);
}

// Returns true if a YMM or ZMM register is live in current LivePhysRegs
static bool checkInstructionHasLiveInYmmOrZmm(const LivePhysRegs &LiveRegs) {
  for (MCPhysReg Reg : LiveRegs)
    if (isYmmOrZmmReg(Reg))
      return true;
  return false;
}
#endif // INTEL_CUSTOMIZATION

/// Check if given call instruction has a RegMask operand.
static bool callHasRegMask(MachineInstr &MI) {
  assert(MI.isCall() && "Can only be called on call instructions.");
  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isRegMask())
      return true;
  }
  return false;
}

#if INTEL_CUSTOMIZATION
// Check if the function has Y/ZMM0-15 in its callee-saved registers.
static bool checkFnHasYmmOrZmmCSR(MachineRegisterInfo &MRI) {
  const MCPhysReg *CSR = MRI.getCalleeSavedRegs();
  for (unsigned I = 0; CSR[I] != 0; ++I)
    if (isYmmOrZmmReg(CSR[I]))
      return true;
  return false;
}
#endif // INTEL_CUSTOMIZATION

/// Insert a vzeroupper instruction before I.
void VZeroUpperInserter::insertVZeroUpper(MachineBasicBlock::iterator I,
                                          MachineBasicBlock &MBB) {
  BuildMI(MBB, I, I->getDebugLoc(), TII->get(X86::VZEROUPPER));
  ++NumVZU;
  EverMadeChange = true;
}

/// Add MBB to the DirtySuccessors list if it hasn't already been added.
void VZeroUpperInserter::addDirtySuccessor(MachineBasicBlock &MBB) {
  if (!BlockStates[MBB.getNumber()].AddedToDirtySuccessors) {
    DirtySuccessors.push_back(&MBB);
    BlockStates[MBB.getNumber()].AddedToDirtySuccessors = true;
  }
}

/// Loop over all of the instructions in the basic block, inserting vzeroupper
/// instructions before function calls.
void VZeroUpperInserter::processBasicBlock(MachineBasicBlock &MBB) {
  // Start by assuming that the block is PASS_THROUGH which implies no unguarded
  // calls.
  BlockExitState CurState = PASS_THROUGH;
#if INTEL_CUSTOMIZATION
  BlockStates[MBB.getNumber()].FirstUnguardedCallOrSSEInstruction = MBB.end();
  LivePhysRegs LiveRegs(*MBB.getParent()->getSubtarget().getRegisterInfo());
  LiveRegs.addLiveIns(MBB);
#endif // INTEL_CUSTOMIZATION

  for (MachineInstr &MI : MBB) {
    bool IsCall = MI.isCall();
    bool IsReturn = MI.isReturn();
    bool IsControlFlow = IsCall || IsReturn;
#if INTEL_CUSTOMIZATION
    bool IsSSE = isSSEInstruction(MI);
    SmallVector<std::pair<MCPhysReg, const MachineOperand *>> Clobbers;
    LiveRegs.stepForward(MI, Clobbers);
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
    // If the function have one of YMM/ZMM0-15 registers as callee-saved
    // register (e.g. interrupt handler function), there is no need to insert
    // vzeroupper since the register will be restored. The state is going to be
    // dirty anyway.
    if (HasYmmOrZmmCSR && IsReturn)
      continue;
#endif // INTEL_CUSTOMIZATION

    // An existing VZERO* instruction resets the state.
    if (MI.getOpcode() == X86::VZEROALL || MI.getOpcode() == X86::VZEROUPPER) {
      CurState = EXITS_CLEAN;
      continue;
    }

    // Shortcut: don't need to check regular instructions in dirty state.
#if INTEL_CUSTOMIZATION
    if ((!IsControlFlow && !IsSSE) && CurState == EXITS_DIRTY)
#endif // INTEL_CUSTOMIZATION
      continue;

    if (hasYmmOrZmmReg(MI)) {
      // We found a ymm/zmm-using instruction; this could be an AVX/AVX512
      // instruction, or it could be control flow.
      CurState = EXITS_DIRTY;
      continue;
    }

#if INTEL_CUSTOMIZATION
    // Check for SSE instructions and control-flow out of the current function
    // (which might indirectly execute SSE instructions).
    if (!IsControlFlow && !IsSSE)
      continue;

    // We can't insert VZEROUPPER before an SSE instruction if a YMM/ZMM
    // register lives through it.
    if (IsSSE && checkInstructionHasLiveInYmmOrZmm(LiveRegs))
#endif // INTEL_CUSTOMIZATION
      continue;

    // If the call has no RegMask, skip it as well. It usually happens on
    // helper function calls (such as '_chkstk', '_ftol2') where standard
    // calling convention is not used (RegMask is not used to mark register
    // clobbered and register usage (def/implicit-def/use) is well-defined and
    // explicitly specified.
    if (IsCall && !callHasRegMask(MI))
      continue;

    // The VZEROUPPER instruction resets the upper 128 bits of YMM0-YMM15
    // registers. In addition, the processor changes back to Clean state, after
    // which execution of SSE instructions or AVX instructions has no transition
    // penalty. Add the VZEROUPPER instruction before any function call/return
    // that might execute SSE code.
    // FIXME: In some cases, we may want to move the VZEROUPPER into a
    // predecessor block.
    if (CurState == EXITS_DIRTY) {
      // After the inserted VZEROUPPER the state becomes clean again, but
      // other YMM/ZMM may appear before other subsequent calls or even before
      // the end of the BB.
      insertVZeroUpper(MI, MBB);
      CurState = EXITS_CLEAN;
    } else if (CurState == PASS_THROUGH) {
#if INTEL_CUSTOMIZATION
      // If this block is currently in pass-through state and we encounter a
      // SSE or call instruction then whether we need a vzeroupper or not
      // depends on whether this block has successors that exit dirty. Record
      // its location, and set the state to EXITS_CLEAN, but do not insert the
      // vzeroupper yet. It will be inserted later if necessary.
      BlockStates[MBB.getNumber()].FirstUnguardedCallOrSSEInstruction = MI;
#endif // INTEL_CUSTOMIZATION
      CurState = EXITS_CLEAN;
    }
  }

  LLVM_DEBUG(dbgs() << "MBB #" << MBB.getNumber() << " exit state: "
                    << getBlockExitStateName(CurState) << '\n');

  if (CurState == EXITS_DIRTY)
    for (MachineBasicBlock *Succ : MBB.successors())
      addDirtySuccessor(*Succ);

  BlockStates[MBB.getNumber()].ExitState = CurState;
}

/// Loop over all of the basic blocks, inserting vzeroupper instructions before
/// function calls.
bool VZeroUpperInserter::runOnMachineFunction(MachineFunction &MF) {
  if (!UseVZeroUpper)
    return false;

  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  if (!ST.hasAVX() || !ST.insertVZEROUPPER())
    return false;
  TII = ST.getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  EverMadeChange = false;
#if INTEL_CUSTOMIZATION
  HasYmmOrZmmCSR = checkFnHasYmmOrZmmCSR(MRI);
#endif // INTEL_CUSTOMIZATION

  bool FnHasLiveInYmmOrZmm = checkFnHasLiveInYmmOrZmm(MRI);

  // Fast check: if the function doesn't use any ymm/zmm registers, we don't
  // need to insert any VZEROUPPER instructions.  This is constant-time, so it
  // is cheap in the common case of no ymm/zmm use.
  bool YmmOrZmmUsed = FnHasLiveInYmmOrZmm;
  for (const auto *RC : {&X86::VR256RegClass, &X86::VR512_0_15RegClass}) {
    if (!YmmOrZmmUsed) {
      for (MCPhysReg R : *RC) {
        if (!MRI.reg_nodbg_empty(R)) {
          YmmOrZmmUsed = true;
          break;
        }
      }
    }
  }
  if (!YmmOrZmmUsed)
    return false;

  assert(BlockStates.empty() && DirtySuccessors.empty() &&
         "X86VZeroUpper state should be clear");
  BlockStates.resize(MF.getNumBlockIDs());

  // Process all blocks. This will compute block exit states, record the first
  // unguarded call in each block, and add successors of dirty blocks to the
  // DirtySuccessors list.
  for (MachineBasicBlock &MBB : MF)
    processBasicBlock(MBB);

  // If any YMM/ZMM regs are live-in to this function, add the entry block to
  // the DirtySuccessors list
  if (FnHasLiveInYmmOrZmm)
    addDirtySuccessor(MF.front());

  // Re-visit all blocks that are successors of EXITS_DIRTY blocks. Add
  // vzeroupper instructions to unguarded calls, and propagate EXITS_DIRTY
  // through PASS_THROUGH blocks.
  while (!DirtySuccessors.empty()) {
    MachineBasicBlock &MBB = *DirtySuccessors.back();
    DirtySuccessors.pop_back();
    BlockState &BBState = BlockStates[MBB.getNumber()];

#if INTEL_CUSTOMIZATION
    // MBB is a successor of a dirty block, so its first SSE instruction or
    // call needs to be guarded.
    if (BBState.FirstUnguardedCallOrSSEInstruction != MBB.end())
      insertVZeroUpper(BBState.FirstUnguardedCallOrSSEInstruction, MBB);
#endif // INTEL_CUSTOMIZATION

    // If this successor was a pass-through block, then it is now dirty. Its
    // successors need to be added to the worklist (if they haven't been
    // already).
    if (BBState.ExitState == PASS_THROUGH) {
      LLVM_DEBUG(dbgs() << "MBB #" << MBB.getNumber()
                        << " was Pass-through, is now Dirty-out.\n");
      for (MachineBasicBlock *Succ : MBB.successors())
        addDirtySuccessor(*Succ);
    }
  }

  BlockStates.clear();
  return EverMadeChange;
}
