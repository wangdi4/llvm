//===- CSAInstBuilder.h - MIR instruction builder ---------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction builder for CSA that does a limited amount
// of constant folding.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAINSTBUILDER_H
#define LLVM_LIB_TARGET_CSA_CSAINSTBUILDER_H

#include "CSAInstrInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

namespace llvm {

class CSAMachineFunctionInfo;

/// A lightweight value class that tracks different kind of machineops for
/// building instructions.
struct MachineOp {
  enum class Variant { RegUse, RegDef, Imm, MachineOp, Null };
  Variant Kind;
  union {
    unsigned RegNo;
    int64_t Immediate;
    const MachineOperand *Op;
  };

  MachineOp(Variant Kind, unsigned Reg) : Kind(Kind), RegNo(Reg) {}
  MachineOp(Variant Kind, int64_t Imm) : Kind(Kind), Immediate(Imm) {}
  MachineOp(const MachineOperand &MO) : Kind(Variant::MachineOp), Op(&MO) {}
  MachineOp(std::nullptr_t) : Kind(Variant::Null) {}

  // Make sure integer values don't get caught up.
  MachineOp(int64_t)  = delete;
  MachineOp(unsigned) = delete;

  explicit operator bool() const { return Kind != Variant::Null; }
  bool operator==(const MachineOp &rhs) const;

  bool isImm() const;
  int64_t getImm() const;

  bool isReg() const;
  unsigned getReg() const;
};

static inline bool operator==(const MachineOp &lhs, const MachineOperand &rhs) {
  return lhs == MachineOp(rhs);
}
static inline bool operator==(const MachineOperand &lhs, const MachineOp &rhs) {
  return MachineOp(lhs) == rhs;
}

static inline MachineOp OpReg(unsigned reg) {
  return MachineOp{MachineOp::Variant::RegUse, reg};
}

static inline MachineOp OpRegDef(unsigned reg) {
  return MachineOp{MachineOp::Variant::RegDef, reg};
}

static inline MachineOp OpImm(int64_t imm) {
  return MachineOp{MachineOp::Variant::Imm, imm};
}

static inline MachineOp OpIf(bool cond, MachineOp ifTrue) {
  return cond ? ifTrue : nullptr;
}

static inline MachineOp OpIf(bool cond, MachineOp ifTrue, MachineOp ifFalse) {
  return cond ? ifTrue : ifFalse;
}

static inline MachineOp OpDef(const MachineOperand &MOP) {
  if (MOP.isReg())
    return OpRegDef(MOP.getReg());
  return MOP;
}

static inline MachineOp OpUse(const MachineOperand &MOP) {
  if (MOP.isReg())
    return OpReg(MOP.getReg());
  return MOP;
}

namespace detail {
template <typename... Args>
static void addOpsToBuilder(MachineInstrBuilder builder, const MachineOp &op,
                            Args... operands) {
  addOpsToBuilder(builder, op);
  addOpsToBuilder(builder, operands...);
}

template <>
void addOpsToBuilder(MachineInstrBuilder builder, const MachineOp &op) {
  switch (op.Kind) {
  case MachineOp::Variant::RegUse:
    builder.addReg(op.RegNo);
    break;
  case MachineOp::Variant::RegDef:
    builder.addReg(op.RegNo, RegState::Define);
    break;
  case MachineOp::Variant::Imm:
    builder.addImm(op.Immediate);
    break;
  case MachineOp::Variant::MachineOp:
    builder.add(*op.Op);
    break;
  case MachineOp::Variant::Null:
    break;
  }
}
} // namespace detail

/// A class to help building MachineInstr instances.
class CSAInstBuilder final {
  MachineInstr *insertionPoint;
  const CSAInstrInfo &TII;

public:
  CSAInstBuilder(const CSAInstrInfo &TII) : TII(TII) {}

  /// Insert all instructions immediately before this instruction.
  void setInsertionPoint(MachineInstr *MI) { insertionPoint = MI; }

  /// Insert a simple binary instruction and return a MachineOp pointing to the
  /// result. If the two operands are both constant, then return a constant
  /// instead of inserting the opcode. If an instruction is added, it will
  /// run on the dataflow fabric.
  MachineOp makeOrConstantFold(CSAMachineFunctionInfo &LMFI, unsigned opcode,
                               const MachineOp &lhs, const MachineOp &rhs);

  /// Create a MachineInstr with the given opcode at the insertion point that
  /// will run on the dataflow fabric instead of the SXU.
  MachineInstrBuilder makeDFInstruction(unsigned opcode) {
    MachineInstrBuilder builder =
      BuildMI(*insertionPoint->getParent(),
              MachineBasicBlock::instr_iterator(*insertionPoint),
              insertionPoint->getDebugLoc(), TII.get(opcode));
    builder.setMIFlag(MachineInstr::NonSequential);
    return builder;
  }

  /// Create and return a MachineInstr that will run on the dataflow fabric
  /// having the MachineOp values as operands.
  template <typename... Args>
  MachineInstr *makeInstruction(unsigned opcode, Args... operands) {
    MachineInstrBuilder builder = makeDFInstruction(opcode);
    detail::addOpsToBuilder(builder, operands...);
    return builder;
  }
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_CSA_CSAINSTBUILDER_H
