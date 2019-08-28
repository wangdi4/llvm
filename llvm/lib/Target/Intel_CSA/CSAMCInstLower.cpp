//===-- CSAMCInstLower.cpp - Convert CSA MachineInstr to an MCInst --------===//
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
// This file contains code to lower CSA MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "CSAMCInstLower.h"
#include "CSAMachineFunctionInfo.h"
#include "MCTargetDesc/CSAMCTargetExpr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"


using namespace llvm;

#define DEBUG_TYPE "csa-instlowering"

MCSymbol *
CSAMCInstLower::GetGlobalAddressSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  return Printer.getSymbol(MO.getGlobal());
}

MCSymbol *
CSAMCInstLower::GetExternalSymbolSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  return Printer.GetExternalSymbolSymbol(MO.getSymbolName());
}

MCSymbol *CSAMCInstLower::GetJumpTableSymbol(const MachineOperand &MO) const {
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<256> Name;
  raw_svector_ostream(Name)
    << DL.getPrivateGlobalPrefix() << "JTI" << Printer.getFunctionNumber()
    << '_' << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  // Create a symbol for the name.
  return Ctx.getOrCreateSymbol(Name.str());
}

MCSymbol *
CSAMCInstLower::GetConstantPoolIndexSymbol(const MachineOperand &MO) const {
  return Printer.GetCPISymbol(MO.getIndex());
}

MCSymbol *
CSAMCInstLower::GetBlockAddressSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  return Printer.GetBlockAddressSymbol(MO.getBlockAddress());
}

MCOperand CSAMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                             MCSymbol *Sym) const {
  // FIXME: We would like an efficient form for this, so we don't have to do a
  // lot of extra uniquing.
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case 0:
    break;
  }

  if (!MO.isJTI() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(
      Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  return MCOperand::createExpr(Expr);
}

void CSAMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      LLVM_DEBUG(MI->dump());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register: {
      // Ignore all implicit register operands.
      if (MO.isImplicit())
        continue;
      unsigned reg = MO.getReg();
      const CSAMachineFunctionInfo &LMFI =
        *MI->getParent()->getParent()->getInfo<CSAMachineFunctionInfo>();
      if (Register::isVirtualRegister(reg)) {
        MCOp = MCOperand::createExpr(
          CSAMCExpr::create(reg, LMFI.getLICName(reg), Ctx));
      } else {
        MCOp = MCOperand::createReg(reg);
      }
      break;
    }
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_FPImmediate: {
      const APInt Bits = MO.getFPImm()->getValueAPF().bitcastToAPInt();
      if (Bits.getBitWidth() > 64)
        report_fatal_error("Unsupported FP type");
      MCOp = MCOperand::createImm(Bits.getLimitedValue());
      break;
    }
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
        MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = LowerSymbolOperand(MO, GetGlobalAddressSymbol(MO));
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = LowerSymbolOperand(MO, GetExternalSymbolSymbol(MO));
      break;
    case MachineOperand::MO_BlockAddress:
      MCOp = LowerSymbolOperand(MO, GetBlockAddressSymbol(MO));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = LowerSymbolOperand(MO, GetJumpTableSymbol(MO));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = LowerSymbolOperand(MO, GetConstantPoolIndexSymbol(MO));
      break;
      /*
    case MachineOperand::MO_RegisterMask:
      continue;
      */
    }

    OutMI.addOperand(MCOp);
  }
}
