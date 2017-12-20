//===- CSAInstBuilder.cpp - MIR instruction builder -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

MachineOp CSAInstBuilder::makeOrConstantFold(CSAMachineFunctionInfo &LMFI,
    unsigned opcode, const MachineOp &lhs, const MachineOp &rhs) {
  unsigned outputSize = TII.getLicSize(opcode);
  auto opClass = TII.getOpcodeClass(opcode);
  if (opClass != CSA::VARIANT_FLOAT && lhs.isImm() && rhs.getImm()) {
    int64_t lhsVal = lhs.getImm(), rhsVal = rhs.getImm();
    int64_t resVal;
    switch (TII.getGenericOpcode(opcode)) {
    case CSA::Generic::ADD: resVal = lhsVal + rhsVal; break;
    case CSA::Generic::SUB: resVal = lhsVal - rhsVal; break;
    case CSA::Generic::MUL: resVal = lhsVal * rhsVal; break;
    case CSA::Generic::AND: resVal = lhsVal & rhsVal; break;
    case CSA::Generic::OR:  resVal = lhsVal | rhsVal; break;
    case CSA::Generic::XOR: resVal = lhsVal ^ rhsVal; break;
    case CSA::Generic::SLL: resVal = lhsVal << rhsVal; break;
    case CSA::Generic::SRA: resVal = lhsVal >> rhsVal; break;
    case CSA::Generic::SRL: resVal = (uint64_t)lhsVal >> (uint64_t)rhsVal; break;
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
    int mask = (1 << outputSize) - 1;
    return OpImm(resVal & mask);
  }

build_mi:
  unsigned newLic = LMFI.allocateLIC(TII.getLicClassForSize(outputSize));
  makeInstruction(opcode, OpRegDef(newLic), lhs, rhs);
  return OpReg(newLic);
}
