//===-- LPUInstrInfo.h - LPU Instruction Information ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LPU implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_LPUINSTRINFO_H
#define LLVM_LIB_TARGET_LPU_LPUINSTRINFO_H

#include "LPURegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "LPUGenInstrInfo.inc"

namespace llvm {

namespace LPU {
  enum CondCode {
    COND_T,
    COND_F,
    COND_INVALID
  };

  // This should match the LPUSchedule.td FuncUnit list.  This is
  // intended to be a temporary placeholder until that method works
  // without needing this separate copy.
  // Note: There is a mapping from these index values to text in
  // LPUInstPrinter.cpp - do not reorder/change without adjusting that table.
  namespace FUNCUNIT {
    enum FuncUnit {
      None,
      VIR, // Virtual unit - doesn't really exist
      ALU, // Integer arithmetic and logical
      SHF, // Shift unit
      IMA, // Integer multiply/accumulate
      FMA, // Floating Multiply Accumulate
      FCM, // Floating point comparisons
      CFI, // Conversion to Floating from Integer
      CIF, // Conversion to Integer of Floating
      DIV, // Division
      MEM, // Memory access
      SXU, // Sequential eXecution Unit
      SPD, // Scratchpad
        Count
    };
  }
}

class LPUSubtarget;

class LPUInstrInfo : public LPUGenInstrInfo {
  const LPURegisterInfo RI;
  virtual void anchor();
public:
  explicit LPUInstrInfo(LPUSubtarget &STI);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const TargetRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB,
                   MachineBasicBlock::iterator I, DebugLoc DL,
                   unsigned DestReg, unsigned SrcReg,
                   bool KillSrc) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI,
                           unsigned SrcReg, bool isKill,
                           int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            unsigned DestReg, int FrameIdx,
                            const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

  /*
  unsigned GetInstSizeInBytes(const MachineInstr *MI) const;
  */

  // ifConversion 
  bool isPredicable(MachineInstr *MI) const override {
    return true;
  }

  bool isProfitableToIfCvt(MachineBasicBlock &MBB,
                          unsigned NumCycles, unsigned ExtraPredCycles,
                          const BranchProbability &Probability) const override {
    return true;
  }

  bool isProfitableToIfCvt(MachineBasicBlock &TMBB,
                           unsigned NumT, unsigned ExtraT,
                           MachineBasicBlock &FMBB,
                           unsigned NumF, unsigned ExtraF,
                           const BranchProbability &Probability)
                           const override {
    return true;
  }

  bool isProfitableToDupForIfCvt(MachineBasicBlock &MBB,
                                 unsigned NumCycles,
                                 const BranchProbability
                                 &Probability) const override {
    return true;
  }


  // Branch folding goodness
  bool
  ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;
  /*
  bool isUnpredicatedTerminator(const MachineInstr *MI) const override;
  */
  bool AnalyzeBranch(MachineBasicBlock &MBB,
                     MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  unsigned RemoveBranch(MachineBasicBlock &MBB) const override;

  unsigned InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB,
                        const SmallVectorImpl<MachineOperand> &Cond,
                        DebugLoc DL) const override;

  unsigned getPickSwitchOpcode(const TargetRegisterClass *RC, bool isPick) const;
  bool isSwitch(MachineInstr *) const;

  unsigned getCopyOpcode(const TargetRegisterClass *RC) const;
};

}

#endif
