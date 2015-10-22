//===-- LPUInstPrinter.cpp - Convert LPU MCInst to assembly syntax --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an LPU MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "LPUInstPrinter.h"
#include "LPU.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"


// Pin the vtable to this file
void LPUInstPrinter::anchor() {}

LPUInstPrinter::LPUInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                      const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

void LPUInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                  StringRef Annot) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

// Include the auto-generated portion of the assembly writer.
#include "LPUGenAsmWriter.inc"

void LPUInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    int64_t imm = Op.getImm();
    if (-65536 <= imm && imm <= 65535) {
      O << Op.getImm();
    } else {
      char str[20] = {};
      sprintf(str,"0x%llx",(long long)imm);
      O << str;
    }
  } else if (Op.isFPImm()) {
    O << Op.getFPImm();
  } else {
    assert(Op.isExpr() && "unknown operand kind in printOperand");
    O << *Op.getExpr(); // Branch targets...
  }
}

void LPUInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  // Load/Store memory operands -- $reg, $reg || $reg, $imm
  printOperand(MI, OpNo, O);
  O << ", ";
  printOperand(MI, OpNo+1, O);
}

  /*
void LPUInstPrinter::printPCRelImmOperand(const MCInst *MI, unsigned OpNo,
                                             raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm())
    O << Op.getImm();
  else {
    assert(Op.isExpr() && "unknown pcrel immediate operand");
    O << *Op.getExpr();
  }
}
  */

  /*
void LPUInstPrinter::printSrcMemOperand(const MCInst *MI, unsigned OpNo,
                                           raw_ostream &O,
                                           const char *Modifier) {
  const MCOperand &Base = MI->getOperand(OpNo);
  const MCOperand &Disp = MI->getOperand(OpNo+1);

  // Print displacement first

  // If the global address expression is a part of displacement field with a
  // register base, we should not emit any prefix symbol here, e.g.
  //   mov.w &foo, r1
  // vs
  //   mov.w glb(r1), r2
  // Otherwise (!) msp430-as will silently miscompile the output :(
  if (!Base.getReg())
    O << '&';

  if (Disp.isExpr())
    O << *Disp.getExpr();
  else {
    assert(Disp.isImm() && "Expected immediate in displacement field");
    O << Disp.getImm();
  }

  // Print register base field
  if (Base.getReg())
    O << '(' << getRegisterName(Base.getReg()) << ')';
}
  */

  /*
void LPUInstPrinter::printCCOperand(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O) {
  unsigned CC = MI->getOperand(OpNo).getImm();

  switch (CC) {
  default:
   llvm_unreachable("Unsupported CC code");
  case LPUCC::COND_E:
   O << "eq";
   break;
  case LPUCC::COND_NE:
   O << "ne";
   break;
  case LPUCC::COND_HS:
   O << "hs";
   break;
  case LPUCC::COND_LO:
   O << "lo";
   break;
  case LPUCC::COND_GE:
   O << "ge";
   break;
  case LPUCC::COND_L:
   O << 'l';
   break;
  }
}
  */
