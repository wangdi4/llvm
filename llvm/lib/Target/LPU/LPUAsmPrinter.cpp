//===-- LPUAsmPrinter.cpp - LPU LLVM assembly writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the LPU assembly language.
//
//===----------------------------------------------------------------------===//

#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUMCInstLower.h"
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
  class LPUAsmPrinter : public AsmPrinter {
  public:
    LPUAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {}

    const char *getPassName() const override {
      return "LPU Assembly Printer";
    }

    void EmitStartOfAsmFile(Module &) override;
    void EmitFunctionBodyStart() override;
    void EmitFunctionBodyEnd() override;
    void EmitInstruction(const MachineInstr *MI) override;
  };
} // end of anonymous namespace

void LPUAsmPrinter::EmitStartOfAsmFile(Module &) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  O << "\t.processor ";
  O << TM.getSubtarget<LPUSubtarget>().lpuName();
  OutStreamer.EmitRawText(O.str());
}


void LPUAsmPrinter::EmitFunctionBodyStart() {
  const MachineRegisterInfo *MRI;
  MRI = &MF->getRegInfo();
  OutStreamer.EmitRawText("\t.group");
  OutStreamer.EmitRawText("\t.result .lic@1 .nil %c0");
  OutStreamer.EmitRawText("\t.param .lic@1 .nil %c1");

  // iterate over all "registers" (LICs) (should be an iterator for that...)
  // 
  // FIXME!!!
  for(unsigned reg=LPU::C2; reg<LPU::C4091; reg++) {
    SmallString<128> Str;
    raw_svector_ostream O(Str);
    if (MRI->isPhysRegUsed(reg)) {
      if (reg<=LPU::C3) {
        O << "\t.result .reg .i64 "<<LPUInstPrinter::getRegisterName(reg);
      } else if (reg<=LPU::C19) {
        O << "\t.param .reg .i64 "<<LPUInstPrinter::getRegisterName(reg);
      } else {
        O << "\t.reg .i64 "<<LPUInstPrinter::getRegisterName(reg);
      }
      OutStreamer.EmitRawText(O.str());
    }
  }

  // Temporary hack - the start label and the backward branch should be in the code
  // for serial units
  OutStreamer.EmitRawText(".start:");
  OutStreamer.EmitRawText("\tmov0\t%ign, %c1");
}

void LPUAsmPrinter::EmitFunctionBodyEnd() {
  // Because code is serially reusable, 
  OutStreamer.EmitRawText("\t.endgroup");
}

//===----------------------------------------------------------------------===//
void LPUAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  LPUMCInstLower MCInstLowering(OutContext, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeLPUAsmPrinter() {
  RegisterAsmPrinter<LPUAsmPrinter> X(TheLPUTarget);
}
