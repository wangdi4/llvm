//===- CSAInstBuilder.cpp - MIR instruction builder -------------*- C++ -*-===//
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

#include "CSAInstBuilder.h"
#include "CSAMachineFunctionInfo.h"

using namespace llvm;

bool MachineOp::isImm() const {
  switch (Kind) {
  case Variant::Imm:
    return true;
  case Variant::MachineOp:
    return Op->isImm();
  default:
    return false;
  }
}

int64_t MachineOp::getImm() const {
  switch (Kind) {
  case Variant::Imm:
    return Immediate;
  case Variant::MachineOp:
    return Op->getImm();
  default:
    llvm_unreachable("Cannot call MachineOp::getImm() on non-imm values");
    return -1;
  }
}

bool MachineOp::isReg() const {
  switch (Kind) {
  case Variant::RegUse:
  case Variant::RegDef:
    return true;
  case Variant::MachineOp:
    return Op->isReg();
  default:
    return false;
  }
}

unsigned MachineOp::getReg() const {
  switch (Kind) {
  case Variant::RegUse:
  case Variant::RegDef:
    return RegNo;
  case Variant::MachineOp:
    return Op->getReg();
  default:
    llvm_unreachable("Cannot call MachineOp::getReg() on non-reg values");
    return -1;
  }
}

bool MachineOp::operator==(const MachineOp &rhs) const {
  const MachineOp &lhs = *this;
  if (lhs.isImm())
    return rhs.isImm() && lhs.getImm() == rhs.getImm();
  else if (rhs.isImm())
    return false;
  if (lhs.isReg())
    return rhs.isReg() && lhs.getReg() == rhs.getReg();
  else if (rhs.isReg())
    return false;
  if (lhs.Kind == Variant::Null || rhs.Kind == Variant::Null)
    return lhs.Kind == rhs.Kind;
  assert(lhs.Kind == Variant::MachineOp && rhs.Kind == Variant::MachineOp &&
      "Unknown kind of MachineOp");
  return lhs.Op->isIdenticalTo(*rhs.Op);
}

MachineOp CSAInstBuilder::makeOrConstantFold(CSAMachineFunctionInfo &LMFI,
                                             unsigned opcode,
                                             const MachineOp &lhs,
                                             const MachineOp &rhs) {
  unsigned outputSize = TII.getLicSize(opcode);
  auto opClass        = TII.getOpcodeClass(opcode);
  if (opClass != CSA::VARIANT_FLOAT && lhs.isImm() && rhs.isImm()) {
    int64_t lhsVal = lhs.getImm(), rhsVal = rhs.getImm();
    int64_t resVal;
    switch (TII.getGenericOpcode(opcode)) {
    case CSA::Generic::ADD:
      resVal = lhsVal + rhsVal;
      break;
    case CSA::Generic::SUB:
      resVal = lhsVal - rhsVal;
      break;
    case CSA::Generic::MUL:
      resVal = lhsVal * rhsVal;
      break;
    case CSA::Generic::AND:
      resVal = lhsVal & rhsVal;
      break;
    case CSA::Generic::OR:
      resVal = lhsVal | rhsVal;
      break;
    case CSA::Generic::XOR:
      resVal = lhsVal ^ rhsVal;
      break;
    case CSA::Generic::SLL:
      resVal = lhsVal << rhsVal;
      break;
    case CSA::Generic::SRA:
      resVal = lhsVal >> rhsVal;
      break;
    case CSA::Generic::SRL:
      resVal = (uint64_t)lhsVal >> (uint64_t)rhsVal;
      break;
    case CSA::Generic::DIV:
      if (opClass == CSA::VARIANT_SIGNED)
        resVal = lhsVal / rhsVal;
      else
        resVal = (uint64_t)lhsVal / (uint64_t)rhsVal;
      break;
    default:
      goto build_mi;
    }
    // Truncate to outputSize bits.
    int64_t mask = outputSize == 64 ? ~0ULL : (1ULL << outputSize) - 1;
    return OpImm(resVal & mask);
  }

build_mi:
  unsigned newLic = LMFI.allocateLIC(TII.getLicClassForSize(outputSize));
  LMFI.setLICGroup(newLic,
      LMFI.getLICGroup(lhs.isReg() ? lhs.getReg() : rhs.getReg()));
  makeInstruction(opcode, OpRegDef(newLic), lhs, rhs);
  return OpReg(newLic);
}
