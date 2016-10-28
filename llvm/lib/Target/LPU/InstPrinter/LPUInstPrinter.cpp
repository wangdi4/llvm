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
#include "LPUInstrInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

static cl::opt<bool>
WrapAsmOpt("lpu-wrap-asm", cl::Hidden,
           cl::desc("LPU Specific: Wrap assembly for x86"),
           cl::init(false));

static std::map<int,const char*> FUName;

// Pin the vtable to this file
void LPUInstPrinter::anchor() {}

LPUInstPrinter::LPUInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                      const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {
  // Should match list in LPUInstrInfo.h
  FUName[LPU::FUNCUNIT::VIR] = "vir"; // Virtual unit - doesn't really exist
  FUName[LPU::FUNCUNIT::ALU] = "alu"; // Integer arithmetic and logical
  FUName[LPU::FUNCUNIT::SHF] = "shf"; // Shift unit
  FUName[LPU::FUNCUNIT::IMA] = "ima"; // Integer multiply/accumulate
  FUName[LPU::FUNCUNIT::FMA] = "fma"; // Floating Multiply Accumulate
  FUName[LPU::FUNCUNIT::FCM] = "fcm"; // Floating point comparisons
  FUName[LPU::FUNCUNIT::CFI] = "cfi"; // Conversion to Floating from Integer
  FUName[LPU::FUNCUNIT::CIF] = "cif"; // Conversion to Integer of Floating
  FUName[LPU::FUNCUNIT::DIV] = "div"; // Division
  FUName[LPU::FUNCUNIT::MEM] = "mem"; // Memory access
  FUName[LPU::FUNCUNIT::SXU] = "sxu"; // Sequential eXecution Unit
  FUName[LPU::FUNCUNIT::SPD] = "spd";// Scratchpad
}

bool LPUInstPrinter::WrapLpuAsm() {
  return WrapAsmOpt;
}

const char *LPUInstPrinter::WrapLpuAsmLinePrefix() {
  if (WrapAsmOpt) {
    return "\t.ascii \"";
  } else {
    return "";
  }
}

const char *LPUInstPrinter::WrapLpuAsmLineSuffix() {
  if (WrapAsmOpt) {
    return "\\n\"";
  } else {
    return "";
  }
}

void LPUInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                  StringRef Annot, const MCSubtargetInfo &STI) {
  O << WrapLpuAsmLinePrefix();
  printInstruction(MI, O);
  O << WrapLpuAsmLineSuffix();
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
  O << ",";
  printOperand(MI, OpNo+1, O);
}

void LPUInstPrinter::printUnitOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  O << FUName[Op.getImm()];
}
