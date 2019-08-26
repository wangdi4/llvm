//===-- CSAInstrInfo.h - CSA Instruction Information ------------*- C++ -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CSA implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAINSTRINFO_H
#define LLVM_LIB_TARGET_CSA_CSAINSTRINFO_H

#include "CSARegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "CSAGenInstrInfo.inc"

#define GET_CSAOPGENERIC_ENUM
#include "CSAGenCSAOpInfo.inc"

namespace llvm {

namespace CSA {
enum CondCode { COND_T, COND_F, COND_INVALID };

// This should match the CSASchedule.td FuncUnit list.  This is
// intended to be a temporary placeholder until that method works
// without needing this separate copy.
// Note: There is a mapping from these index values to text in
// CSAInstPrinter.cpp - do not reorder/change without adjusting that table.
namespace FUNCUNIT {
enum FuncUnit {
  None,
  Auto, // Automatic unit assignment
  VIR,  // Virtual unit - doesn't really exist
  ALU,  // Integer arithmetic and logical
  SHF,  // Shift unit
  IMA,  // Integer multiply/accumulate
  FMA,  // Floating Multiply Accumulate
  FCM,  // Floating point comparisons
  CFI,  // Conversion to Floating from Integer
  CIF,  // Conversion to Integer of Floating
  DIV,  // Division
  MEM,  // Memory access
  SXU,  // Sequential eXecution Unit
  SPD,  // Scratchpad
  Count
};
}

#define CSA_ASM_OPERAND(Asm, Enum, Default, ...) \
  enum Enum { __VA_ARGS__ };
#include "AsmOperands.h"

/// This indicates the kind of output when distinguishing between different
/// opcodes that generate the same generic opcode.
enum OpcodeClass {
  /// This is an opcode that doesn't distinguish itself in anyway. Usually,
  /// the operands are integers (as distinguished from floats). In effect,
  /// this refers to TiN classes in the tablegen.
  VARIANT_INT = 0,
  /// This is an opcode that specifically refers to floats, for example a
  /// ADDF operation. This means that the corresponding operand use
  VARIANT_FLOAT = 1,
  /// This is an opcode that refers specifically to a signed integer.
  VARIANT_SIGNED = 2,
  /// This is an opcode that refers specifically to an unsigned integer.
  VARIANT_UNSIGNED = 3,
  /// This is an opcode that refers specifically to a SIMD value.
  VARIANT_SIMD = 4,
  /// This is used in some function calls to indicate that the desired class
  /// doesn't matter and any opcode suffices. This is the default argument,
  /// so it generally only matters if this doesn't appear.
  VARIANT_DONTCARE = ~0U
};

/// Flag bits for target-specific flags on the MCInstrDesc objects. This needs
/// to be kept up-to-date with the definitions in CSAInstrFormats.td
enum {
  MultiTriggered = 1 << 0
};
} // namespace CSA

class CSASubtarget;

class CSAInstrInfo : public CSAGenInstrInfo {
  const CSARegisterInfo RI;
  virtual void anchor();

public:
  explicit CSAInstrInfo(CSASubtarget &STI);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const TargetRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, unsigned DestReg, unsigned SrcReg,
                   bool KillSrc) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, unsigned SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, unsigned DestReg,
                            int FrameIdx, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

  /*
  unsigned GetInstSizeInBytes(const MachineInstr *MI) const;
  */

  // ifConversion
  bool isPredicable(const MachineInstr &MI) const override { return true; }

  bool isProfitableToIfCvt(MachineBasicBlock &MBB, unsigned NumCycles,
                           unsigned ExtraPredCycles,
                           BranchProbability Probability) const override {
    return true;
  }

  bool isProfitableToIfCvt(MachineBasicBlock &TMBB, unsigned NumT,
                           unsigned ExtraT, MachineBasicBlock &FMBB,
                           unsigned NumF, unsigned ExtraF,
                           BranchProbability Probability) const override {
    return true;
  }

  bool isProfitableToDupForIfCvt(MachineBasicBlock &MBB, unsigned NumCycles,
                                 BranchProbability Probability) const override {
    return true;
  }

  // Branch folding goodness
  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;
  /*
  bool isUnpredicatedTerminator(const MachineInstr &MI) const override;
  */
  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL, int *BytesAdded) const override;

  /// Return the generic opcode for the relevant opcode. If the opcode is not
  /// valid, INVALID_OP is returned.
  CSA::Generic getGenericOpcode(unsigned opcode) const;

  /// Return the lic size, in bits, of the opcode. If the opcode is not valid,
  /// 0 is returned. The lic size of convert and sign-extend operations refers
  /// to their destination value.
  unsigned getLicSize(unsigned opcode) const;

  /// Return whether or not this opcode is a variant that operates on floats,
  /// signed integers, unsigned integers, or integers. This returns a value that
  /// is one of the first 4 entries of the CSA::OpcodeClass enumeration, and is
  /// used to distinguish between different operations that reuse the same
  /// generic opcode.
  CSA::OpcodeClass getOpcodeClass(unsigned opcode) const;

  /// Construct an opcode for a MachineInstr given the generic opcode and a
  /// desired licSize. If such an operation cannot be constructed, then the
  /// result is CSA::INVALID_OPCODE.
  unsigned
  makeOpcode(CSA::Generic genericOpcode, unsigned licSize,
             CSA::OpcodeClass opcodeClass = CSA::VARIANT_DONTCARE) const;

  /// Variant of makeOpcode that works on register classes instead of fixed
  /// bit sizes.
  unsigned
  makeOpcode(CSA::Generic genericOpcode, const TargetRegisterClass *RC,
             CSA::OpcodeClass opcodeClass = CSA::VARIANT_DONTCARE) const {
    return makeOpcode(genericOpcode, getSizeOfRegisterClass(RC), opcodeClass);
  }

  /// \brief This returns a new opcode flavor for newOpcode
  /// with the same bitwidth and classification as the input opcode.
  /// override_class parameter allows overriding class achieved
  /// from the input opcode, if it is set to anything but
  /// CSA::VARIANT_DONTCARE.
  /// If no such opcode exists, then INVALID_OPCODE is returned.
  unsigned adjustOpcode(unsigned opcode, CSA::Generic newOpcode,
                        CSA::OpcodeClass override_class =
                        CSA::VARIANT_DONTCARE) const;

  /// Get the lic size for the given register class.
  unsigned getSizeOfRegisterClass(const TargetRegisterClass *RC) const;

  bool isSwitch(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::SWITCH;
  }
  bool isCopy(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::GCOPY;
  }
  bool isMOV(const MachineInstr *MI) const;
  bool isPick(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::PICK;
  }
  bool isPickany(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::PICKANY;
  }
  bool isInit(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::INIT;
  }
  bool isLoad(const MachineInstr *) const;
  bool isStore(const MachineInstr *) const;
  bool isAdd(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::ADD;
  }
  bool isSub(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::SUB;
  }
  bool isMul(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::MUL;
  }
  bool isDiv(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::DIV;
  }
  bool isFMA(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::FMA;
  }
  bool isShift(const MachineInstr *MI) const {
    auto opcode = getGenericOpcode(MI->getOpcode());
    return opcode == CSA::Generic::SLL || opcode == CSA::Generic::SRL ||
           opcode == CSA::Generic::SRA;
  }
  bool isCmp(const MachineInstr *MI) const { return MI->isCompare(); }
  bool isRepeatO(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::REPEATO;
  }
  bool isSext(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::SEXT;
  }
  bool isNot(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::NOT;
  }
  bool isStride(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::STRIDE;
  }
  bool isFilter(const MachineInstr *MI) const {
    return getGenericOpcode(MI->getOpcode()) == CSA::Generic::FILTER;
  }
  bool isAtomic(const MachineInstr *) const;
  bool isSeqOT(const MachineInstr *MI) const;
  bool isSeqZT(const MachineInstr *MI) const;
  bool isSeq(const MachineInstr *MI) const {
    return isSeqOT(MI) || isSeqZT(MI);
  }
  bool isReduction(const MachineInstr *) const;

  /// Returns true if the machine operation is multi-triggered.
  bool isMultiTriggered(const MachineInstr *MI) const {
    return MI->getDesc().TSFlags & CSA::MultiTriggered;
  }

  // Gets the opcode for a memory token MOV.  This method is defined
  // here so that all the places which need to know which opcode is a
  // MOV of a memory token agree upon the answer.
  unsigned getMemTokenMOVOpcode() const;
  // Returns true if this instruction is a MOV of a memory token.
  bool isMemTokenMOV(const MachineInstr *) const;

  unsigned getCopyOpcode(const TargetRegisterClass *RC) const {
    return makeOpcode(CSA::Generic::GCOPY, RC);
  }
  unsigned getMoveOpcode(const TargetRegisterClass *RC) const {
    return makeOpcode(CSA::Generic::MOV, RC);
  }
  unsigned getInitOpcode(const TargetRegisterClass *RC) const {
    return makeOpcode(CSA::Generic::INIT, RC);
  }
  unsigned getRepeatOpcode(const TargetRegisterClass *RC) const {
    return makeOpcode(CSA::Generic::REPEAT, RC);
  }
  unsigned getPickanyOpcode(const TargetRegisterClass *RC) const {
    return makeOpcode(CSA::Generic::PICKANY, RC);
  }

  // Takes an opcode for a compare instruction, and returns the opcode
  // that you would use if (a) you swapped the input operands to the
  // comparison, and/or (b) you negate the output of the compare.
  //
  //
  // The method for transforming a compare for commuting the operands
  // or negating the output.
  //
  // Let swap_ltgt = (commute_compare_operands ^ negate_eq)
  //
  // If swap_ltgt:
  //     >= maps to <
  //     > maps to <=
  //     <= maps to >
  //     < maps to >=
  // else:
  //     >=, >, <=, < remains the same.
  //
  // if negate_eq:
  //     == maps to !=
  //     != maps to ==
  // else:
  //     ==  maps to ==
  //     !=  maps to !=
  //
  // This method covers:
  //    Signed/unsigned integer comparison 8, 16, 32, 64-bit types
  //    Floating-point comparisons for 16, 32, 64-bit types.
  unsigned commuteNegateCompareOpcode(unsigned cmp_opcode,
                                      bool commute_compare_operands,
                                      bool negate_eq) const;

  // Takes in an opcode for a comparison operation, and returns the
  // opcode for a sequence instruction corresponding to that op.
  //
  //
  //  CMPGT maps to SEQOTGT
  //  CMPGE maps to SEQOTGE
  //    etc...
  unsigned convertCompareOpToSeqOTOp(unsigned cmp_opcode) const;

  // Takes in an opcode for a SEQOT operation, and returns the
  // opcode for a zero-trip sequence instruction corresponding to that op.
  //
  //
  //  SEQOTGT maps to SEQGT
  //  SEQOTGE maps to SEQGE
  //    etc...
  unsigned convertSeqOTToSeqOp(unsigned seqot_opcode) const;

  // Takes in a sequence opcode, and a bitwidth, and returns a
  // corresponding sequence opcode whose bitwidth size is
  //   min(size of seq_opcode, bitwidth)
  //
  // For example:
  //     SEQOTGTS32, 8   --> SEQOTGTS32
  //     SEQOTGTS16, 32  --> SEQOTGTS32
  unsigned promoteSeqOTOpBitwidth(unsigned seq_opcode, int bitwidth) const;

  // Get channel register class that is the same size as this stride
  // operation.
  const TargetRegisterClass *getStrideInputRC(unsigned stride_opcode) const;

  // Returns true if this op is one of the transform ops that
  // corresponds to a 3-operand commuting reduction.
  //
  //  ADD, MUL, AND, OR, XOR
  bool isCommutingReductionTransform(const MachineInstr *) const;

  // Convert an ADD/SUB/FMA code into a matching reduction opcode of
  // the same size. TBD(jsukha): Not implemented yet!
  unsigned convertTransformToReductionOp(unsigned transform_opcode) const;

  const TargetRegisterClass *getLicClassForSize(unsigned size) const;

  /// Return true if the register class is a LIC class rather than an SXU
  /// register.
  bool isLICClass(const TargetRegisterClass *RC) const;

  /// Return the register class for a LIC register.
  const TargetRegisterClass *
  getRegisterClass(unsigned reg, const MachineRegisterInfo &MRI) const;

  /// Return true if the MachineOperand represents a LIC.
  bool isLIC(const MachineOperand &MO, const MachineRegisterInfo &MRI) const {
    return MO.isReg() && isLICClass(getRegisterClass(MO.getReg(), MRI));
  }
};

} // namespace llvm

#endif
