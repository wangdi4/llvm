//===-- CSAInstPrinter.cpp - Convert CSA MCInst to assembly syntax --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

static cl::opt<bool>
WrapAsmOpt("csa-wrap-asm", cl::Hidden,
           cl::desc("CSA Specific: Wrap assembly for x86"),
           cl::init(false));

static std::map<int,const char*> FUName;
static std::map<int,const char*> RMName;
static std::map<int,const char*> MLName;

// Pin the vtable to this file
void CSAInstPrinter::anchor() {}

CSAInstPrinter::CSAInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                      const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {
  // Should match list in CSAInstrInfo.h
  FUName[CSA::FUNCUNIT::Auto] = "";   // Automatic unit assignment
  FUName[CSA::FUNCUNIT::VIR] = "vir"; // Virtual unit - doesn't really exist
  FUName[CSA::FUNCUNIT::ALU] = "alu"; // Integer arithmetic and logical
  FUName[CSA::FUNCUNIT::SHF] = "shf"; // Shift unit
  FUName[CSA::FUNCUNIT::IMA] = "ima"; // Integer multiply/accumulate
  FUName[CSA::FUNCUNIT::FMA] = "fma"; // Floating Multiply Accumulate
  FUName[CSA::FUNCUNIT::FCM] = "fcm"; // Floating point comparisons
  FUName[CSA::FUNCUNIT::CFI] = "cfi"; // Conversion to Floating from Integer
  FUName[CSA::FUNCUNIT::CIF] = "cif"; // Conversion to Integer of Floating
  FUName[CSA::FUNCUNIT::DIV] = "div"; // Division
  FUName[CSA::FUNCUNIT::MEM] = "mem"; // Memory access
  FUName[CSA::FUNCUNIT::SXU] = "sxu"; // Sequential eXecution Unit
  FUName[CSA::FUNCUNIT::SPD] = "spd";// Scratchpad

  // Should match lists in CSAInstrInfo.h and csa.h in the simulator.
  RMName[CSA::ROUND_NEAREST]    = "ROUND_NEAREST";
  RMName[CSA::ROUND_DOWNWARD]   = "ROUND_DOWNWARD";
  RMName[CSA::ROUND_UPWARD]     = "ROUND_UPWARD";
  RMName[CSA::ROUND_TOWARDZERO] = "ROUND_TOWARDZERO";

  // Should also match the names in CSAInstrInfo.h and the simulator.
  MLName[CSA::MEMLEVEL_NTA] = "MEMLEVEL_NTA";
  MLName[CSA::MEMLEVEL_T2]  = "MEMLEVEL_T2";
  MLName[CSA::MEMLEVEL_T1]  = "MEMLEVEL_T1";
  MLName[CSA::MEMLEVEL_T0]  = "MEMLEVEL_T0";
}

bool CSAInstPrinter::WrapCsaAsm() {
  return WrapAsmOpt;
}

const char *CSAInstPrinter::WrapCsaAsmLinePrefix() {
  if (WrapAsmOpt) {
    return "\t.ascii \"";
  } else {
    return "";
  }
}

const char *CSAInstPrinter::WrapCsaAsmLineSuffix() {
  if (WrapAsmOpt) {
    return "\\n\"";
  } else {
    return "";
  }
}

void CSAInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                  StringRef Annot, const MCSubtargetInfo &STI) {
  O << WrapCsaAsmLinePrefix();
  printInstruction(MI, O);
  O << WrapCsaAsmLineSuffix();
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

void CSAInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  // Load/Store memory operands -- $reg, $reg || $reg, $imm
  printOperand(MI, OpNo, O);
  O << ",";
  printOperand(MI, OpNo+1, O);
}

void CSAInstPrinter::printUnitOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  O << FUName[Op.getImm()];
}

void CSAInstPrinter::printRModeOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");

  // This is an optional operand. The optional MO is added at selection, so if
  // this instruction was added later, it may still not have this operand. In
  // this case, just print the default, 0.
  int64_t immV = 0;

  if (OpNo < MI->getNumOperands()) {
    const MCOperand &Op = MI->getOperand(OpNo);
    immV = Op.getImm();
  }

  auto it = RMName.find(immV);
  if (it != RMName.end())
    O << RMName[immV];
  else
    printOperand(MI, OpNo, O, Modifier);
}

void CSAInstPrinter::printMemLvlOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");

  int64_t immV = MI->getOperand(OpNo).getImm();
  auto it = MLName.find(immV);
  if (it != MLName.end())
    O << it->second;
  else
    printOperand(MI, OpNo, O, Modifier);
}
