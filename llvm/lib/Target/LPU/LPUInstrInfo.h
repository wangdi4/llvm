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

  // Convert opcode of LD/ST into a corresponding opcode for OLD/OST.
  // Returns current_opcode if it is not a LD or ST.
  unsigned get_ordered_opcode_for_LDST(unsigned current_opcode) const;
  
  unsigned getPickSwitchOpcode(const TargetRegisterClass *RC, bool isPick) const;
  bool isSwitch(MachineInstr *) const;
  bool isCopy(MachineInstr *) const;
  bool isMOV(MachineInstr *) const;
  bool isPick(MachineInstr *) const;
  bool isInit(MachineInstr *) const;
  bool isLoad(MachineInstr *) const;
  bool isStore(MachineInstr *) const;
  bool isOrderedLoad(MachineInstr *) const;
  bool isOrderedStore(MachineInstr *) const;
  bool isAdd(MachineInstr *) const;
  bool isSub(MachineInstr *) const;
  bool isMul(MachineInstr *) const;
  bool isDiv(MachineInstr *) const;
  bool isFMA(MachineInstr *) const;
  bool isShift(MachineInstr *) const;
  bool isCmp(MachineInstr *) const;
  bool isAtomic(MachineInstr *) const;
  bool isOrderedAtomic(MachineInstr *) const;
  bool isSeqOT(MachineInstr *) const;
  
  unsigned getCopyOpcode(const TargetRegisterClass *RC) const;
  unsigned getMoveOpcode(const TargetRegisterClass *RC) const;
  unsigned getInitOpcode(const TargetRegisterClass *RC) const;


  
  // Methods for mapping opcodes, primarily for sequence optimization.

  // Takes an opcode for a compare instruction, and returns the opcode
  // that you would use if you swapped the input operands to the
  // comparison.
  //
  //  >=  maps to <
  //  >   maps to <=
  //  <=  maps to >
  //  <   maps to >=
  //  ==  maps to ==
  //  !=  maps to !=
  //
  // This method covers:
  //    Signed/unsigned integer comparison 8, 16, 32, 64-bit types
  //    Floating-point comparisons for 16, 32, 64-bit types.
  //
  // This method dies if passed in an opcode which is not an
  // appropriate compare.
  unsigned commuteCompareOpcode(unsigned cmp_opcode) const;


  // Takes in an opcode for a comparison operation, and returns the
  // opcode for a sequence instruction corresponding to that op.
  //
  // 
  //  CMPGT maps to SEQOTGT
  //  CMPGE maps to SEQOTGE
  //    etc...
  unsigned convertCompareOpToSeqOTOp(unsigned cmp_opcode) const;


  // Takes in a sequence opcode, and a bitwidth, and returns a
  // corresponding sequence opcode whose bitwidth size is
  //   min(size of seq_opcode, bitwidth)
  //
  // For example:
  //     SEQOTGTS32, 8   --> SEQOTGTS32
  //     SEQOTGTS16, 32  --> SEQOTGTS32
  unsigned promoteSeqOTOpBitwidth(unsigned seq_opcode, int bitwidth) const;


  // Convert an add opcode to the corresponding stride opcode.
  // Returns true if we did a successful conversion, false otherwise.
  // Saves output into *strideOpcode when returning true.
  bool
  convertAddToStrideOp(unsigned add_opcode,
                       unsigned* strideOpcode) const;

  // Convert a pick opcode into a matching repeat opcode (of the same
  // size).
  bool
  convertPickToRepeatOp(unsigned pick_opcode,
                        unsigned* repeat_opcode) const;

  // Convert an ADD/SUB/FMA code into a matching reduction opcode of
  // the same size. TBD(jsukha): Not implemented yet!
  bool
  convertTransformToReductionOp(unsigned transform_opcode,
                                unsigned* reduction_opcode) const;
  
};

}

#endif
