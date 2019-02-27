//===-- CSAInstPrinter.cpp - Convert CSA MCInst to assembly syntax --------===//
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
// This class prints an CSA MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "CSAInstPrinter.h"
#include "CSA.h"
#include "CSAInstrInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

#include <array>

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

static cl::opt<bool> WrapAsmOpt("csa-wrap-asm", cl::Hidden, cl::ZeroOrMore,
                                cl::desc("CSA Specific: Wrap assembly for x86"),
                                cl::init(false));

static std::map<int, const char *> FUName;

// Pin the vtable to this file
void CSAInstPrinter::anchor() {}

CSAInstPrinter::CSAInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                               const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {
  // Should match list in CSAInstrInfo.h
  FUName[CSA::FUNCUNIT::Auto] = "";    // Automatic unit assignment
  FUName[CSA::FUNCUNIT::VIR]  = "vir"; // Virtual unit - doesn't really exist
  FUName[CSA::FUNCUNIT::ALU]  = "alu"; // Integer arithmetic and logical
  FUName[CSA::FUNCUNIT::SHF]  = "shf"; // Shift unit
  FUName[CSA::FUNCUNIT::IMA]  = "ima"; // Integer multiply/accumulate
  FUName[CSA::FUNCUNIT::FMA]  = "fma"; // Floating Multiply Accumulate
  FUName[CSA::FUNCUNIT::FCM]  = "fcm"; // Floating point comparisons
  FUName[CSA::FUNCUNIT::CFI]  = "cfi"; // Conversion to Floating from Integer
  FUName[CSA::FUNCUNIT::CIF]  = "cif"; // Conversion to Integer of Floating
  FUName[CSA::FUNCUNIT::DIV]  = "div"; // Division
  FUName[CSA::FUNCUNIT::MEM]  = "mem"; // Memory access
  FUName[CSA::FUNCUNIT::SXU]  = "sxu"; // Sequential eXecution Unit
  FUName[CSA::FUNCUNIT::SPD]  = "spd"; // Scratchpad
}

bool CSAInstPrinter::WrapCsaAsm() { return WrapAsmOpt; }

void CSAInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                               StringRef Annot, const MCSubtargetInfo &STI) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

// Include the auto-generated portion of the assembly writer.
#include "CSAGenAsmWriter.inc"

void CSAInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    O << "%" << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    int64_t imm = Op.getImm();
    if (-65536 <= imm && imm <= 65535) {
      O << Op.getImm();
    } else {
      char str[20] = {};
      sprintf(str, "0x%llx", (long long)imm);
      O << str;
    }
  } else if (Op.isFPImm()) {
    O << Op.getFPImm();
  } else {
    assert(Op.isExpr() && "unknown operand kind in printOperand");
    O << *Op.getExpr(); // Branch targets...
  }
}

void CSAInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  // Load/Store memory operands -- $reg, $reg || $reg, $imm
  printOperand(MI, OpNo, O);
  O << ", ";
  printOperand(MI, OpNo + 1, O);
}

void CSAInstPrinter::printUnitOperand(const MCInst *MI, unsigned OpNo,
                                      raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  O << FUName[Op.getImm()];
}

#define CSA_ASM_OPERAND_VALUE(x) #x
#define CSA_ASM_OPERAND(Asm, Enum, Default, ...) \
void CSAInstPrinter::print##Asm##Operand(const MCInst *MI, unsigned OpNo, \
                                         raw_ostream &O, const char *Modifier) { \
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported"); \
  uint64_t value = CSA::Default; \
  if (OpNo < MI->getNumOperands()) \
    value = MI->getOperand(OpNo).getImm(); \
  static const char *names[] = { \
    __VA_ARGS__ \
  }; \
  assert(value < sizeof(names) / sizeof(names[0]) && \
    #Asm " operand is outside range of permissible values"); \
  O << names[value]; \
}

#include "AsmOperands.h"

void CSAInstPrinter::printPrioOrderOperand(const MCInst *MI, unsigned OpNo,
                                           raw_ostream &O,
                                          const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");

  // This is an optional operand; the compiler may never add it. The optional
  // MO is added at selection, so if this instruction was added later, it may
  // still not have this operand. In this case, just print the default, 0.

  if (OpNo < MI->getNumOperands()) {
    printOperand(MI, OpNo, O, Modifier);
  } else {
    // There are no fancy names for this one -- just "0" or "1".
    O << 0;
  }
}

