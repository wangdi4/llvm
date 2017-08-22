//===-- Nios2MCInstLower.cpp - Convert Nios2 MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Nios2 MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "Nios2MCInstLower.h"

#include "Nios2AsmPrinter.h"
#include "Nios2InstrInfo.h"
#include "MCTargetDesc/Nios2BaseInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

Nios2MCInstLower::Nios2MCInstLower(Nios2AsmPrinter &asmprinter)
  : AsmPrinter(asmprinter) {}

void Nios2MCInstLower::Initialize(MCContext* C) {
  Ctx = C;
}

MCOperand Nios2MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
  MCSymbolRefExpr::VariantKind Kind = MCSymbolRefExpr::VK_None;
  Nios2MCExpr::Nios2ExprKind TargetKind = Nios2MCExpr::CEK_None;
  const MCSymbol *Symbol;

  switch(MO.getTargetFlags()) {
  default:                   llvm_unreachable("Invalid target flag!");
  case Nios2II::MO_NO_FLAG:
    break;

// Nios2_GPREL is for llc -march=nios2 -relocation-model=static -nios2-islinux-
//  format=false (global var in .sdata).
  case Nios2II::MO_GPREL:
    TargetKind = Nios2MCExpr::CEK_GPREL;
    break;

  case Nios2II::MO_GOT_CALL:
    TargetKind = Nios2MCExpr::CEK_GOT_CALL;
    break;
  case Nios2II::MO_GOT:
    TargetKind = Nios2MCExpr::CEK_GOT;
    break;
// ABS_HI and ABS_LO is for llc -march=nios2 -relocation-model=static (global 
//  var in .data).
  case Nios2II::MO_ABS_HI:
    TargetKind = Nios2MCExpr::CEK_ABS_HI;
    break;
  case Nios2II::MO_ABS_LO:
    TargetKind = Nios2MCExpr::CEK_ABS_LO;
    break;
  case Nios2II::MO_TLSGD:
    TargetKind = Nios2MCExpr::CEK_TLSGD;
    break;
  case Nios2II::MO_TLSLDM:
    TargetKind = Nios2MCExpr::CEK_TLSLDM;
    break;
  case Nios2II::MO_DTP_HI:
    TargetKind = Nios2MCExpr::CEK_DTP_HI;
    break;
  case Nios2II::MO_DTP_LO:
    TargetKind = Nios2MCExpr::CEK_DTP_LO;
    break;
  case Nios2II::MO_GOTTPREL:
    TargetKind = Nios2MCExpr::CEK_GOTTPREL;
    break;
  case Nios2II::MO_TP_HI:
    TargetKind = Nios2MCExpr::CEK_TP_HI;
    break;
  case Nios2II::MO_TP_LO:
    TargetKind = Nios2MCExpr::CEK_TP_LO;
    break;
  case Nios2II::MO_GOT_HI16:
    TargetKind = Nios2MCExpr::CEK_GOT_HI16;
    break;
  case Nios2II::MO_GOT_LO16:
    TargetKind = Nios2MCExpr::CEK_GOT_LO16;
    break;
  }

  switch (MOTy) {
  case MachineOperand::MO_GlobalAddress:
    Symbol = AsmPrinter.getSymbol(MO.getGlobal());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_BlockAddress:
    Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_ExternalSymbol:
    Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_JumpTableIndex:
    Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
    break;

  default:
    llvm_unreachable("<unknown operand type>");
  }

  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Kind, *Ctx);

  if (Offset) {
    // Assume offset is never negative.
    assert(Offset > 0);
    Expr = MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, *Ctx),
                                   *Ctx);
  }

  if (TargetKind != Nios2MCExpr::CEK_None)
    Expr = Nios2MCExpr::create(TargetKind, Expr, *Ctx);

  return MCOperand::createExpr(Expr);

}

static void CreateMCInst(MCInst& Inst, unsigned Opc, const MCOperand& Opnd0,
                         const MCOperand& Opnd1,
                         const MCOperand& Opnd2 = MCOperand()) {
  Inst.setOpcode(Opc);
  Inst.addOperand(Opnd0);
  Inst.addOperand(Opnd1);
  if (Opnd2.isValid())
    Inst.addOperand(Opnd2);
}

// Lower ".cpload $reg" to
//  "lui   $gp, %hi(_gp_disp)"
//  "addiu $gp, $gp, %lo(_gp_disp)"
//  "addu  $gp, $gp, $t9"
void Nios2MCInstLower::LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts) {
  MCOperand GPReg = MCOperand::createReg(Nios2::GP);
  MCOperand T9Reg = MCOperand::createReg(Nios2::BT);
  StringRef SymName("_gp_disp");
  const MCSymbol *Sym = Ctx->getOrCreateSymbol(SymName);
  const Nios2MCExpr *MCSym;

  MCSym = Nios2MCExpr::create(Sym, Nios2MCExpr::CEK_ABS_HI, *Ctx);
  MCOperand SymHi = MCOperand::createExpr(MCSym);
  MCSym = Nios2MCExpr::create(Sym, Nios2MCExpr::CEK_ABS_LO, *Ctx);
  MCOperand SymLo = MCOperand::createExpr(MCSym);

  MCInsts.resize(3);

  CreateMCInst(MCInsts[0], Nios2::ORHI_R1, GPReg, SymHi); // TODO: Nios2 R2 Support
  CreateMCInst(MCInsts[1], Nios2::ORI_R1, GPReg, GPReg, SymLo); // TODO: Nios2 R2 Support
  CreateMCInst(MCInsts[2], Nios2::ADD_R1, GPReg, GPReg, T9Reg); //TODO: *_R2 should be selected for R2
}

#ifdef ENABLE_GPRESTORE
// Lower ".cprestore offset" to "st $gp, offset($sp)".
void Nios2MCInstLower::LowerCPRESTORE(int64_t Offset,
                                     SmallVector<MCInst, 4>& MCInsts) {
  assert(isInt<32>(Offset) && (Offset >= 0) &&
         "Imm operand of .cprestore must be a non-negative 32-bit value.");

  MCOperand SPReg = MCOperand::createReg(Nios2::SP), BaseReg = SPReg;
  MCOperand GPReg = MCOperand::createReg(Nios2::GP);
  MCOperand ZEROReg = MCOperand::createReg(Nios2::ZERO);

  if (!isInt<16>(Offset)) {
    unsigned Hi = ((Offset + 0x8000) >> 16) & 0xffff;
    Offset &= 0xffff;
    MCOperand ATReg = MCOperand::createReg(Nios2::AT);
    BaseReg = ATReg;

    // lui   at,hi
    // add   at,at,sp
    MCInsts.resize(2);
    CreateMCInst(MCInsts[0], Nios2::ORHI_R1, ATReg, ZEROReg,  // TODO: Nios2 R2 Support
                 MCOperand::createImm(Hi));
    CreateMCInst(MCInsts[1], Nios2::ADD_R1, ATReg, ATReg, SPReg); //TODO: *_R2 should be selected for R2
  }

  MCInst St;
  CreateMCInst(St, Nios2::STW_R1, GPReg, BaseReg, MCOperand::createImm(Offset)); // TODO: Nios2 R2 Support
  MCInsts.push_back(St);
}
#endif

//@LowerOperand {
MCOperand Nios2MCInstLower::LowerOperand(const MachineOperand& MO,
                                        unsigned offset) const {
  MachineOperandType MOTy = MO.getType();

  switch (MOTy) {
  //@2
  default: llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit()) break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm() + offset);
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_BlockAddress:
  case MachineOperand::MO_GlobalAddress:
    return LowerSymbolOperand(MO, MOTy, offset);
  case MachineOperand::MO_RegisterMask:
    break;
 }

  return MCOperand();
}

MCOperand Nios2MCInstLower::createSub(MachineBasicBlock *BB1,
                                     MachineBasicBlock *BB2,
                                     Nios2MCExpr::Nios2ExprKind Kind) const {
  const MCSymbolRefExpr *Sym1 = MCSymbolRefExpr::create(BB1->getSymbol(), *Ctx);
  const MCSymbolRefExpr *Sym2 = MCSymbolRefExpr::create(BB2->getSymbol(), *Ctx);
  const MCBinaryExpr *Sub = MCBinaryExpr::createSub(Sym1, Sym2, *Ctx);

  return MCOperand::createExpr(Nios2MCExpr::create(Kind, Sub, *Ctx));
}

void Nios2MCInstLower::
lowerLongBranchLUi(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(Nios2::ORHI_R1); // TODO: Nios2 R2 support

  // Lower register operand.
  OutMI.addOperand(LowerOperand(MI->getOperand(0)));

  // Create %hi($tgt-$baltgt).
  OutMI.addOperand(createSub(MI->getOperand(1).getMBB(),
                             MI->getOperand(2).getMBB(),
                             Nios2MCExpr::CEK_ABS_HI));
}

void Nios2MCInstLower::
lowerLongBranchADDiu(const MachineInstr *MI, MCInst &OutMI, int Opcode,
                     Nios2MCExpr::Nios2ExprKind Kind) const {
  OutMI.setOpcode(Opcode);

  // Lower two register operands.
  for (unsigned I = 0, E = 2; I != E; ++I) {
    const MachineOperand &MO = MI->getOperand(I);
    OutMI.addOperand(LowerOperand(MO));
  }

  // Create %lo($tgt-$baltgt) or %hi($tgt-$baltgt).
  OutMI.addOperand(createSub(MI->getOperand(2).getMBB(),
                             MI->getOperand(3).getMBB(), Kind));
}

bool Nios2MCInstLower::lowerLongBranch(const MachineInstr *MI,
                                      MCInst &OutMI) const {
  switch (MI->getOpcode()) {
  default:
    return false;
  }
}

void Nios2MCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  if (lowerLongBranch(MI, OutMI))
    return;
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
