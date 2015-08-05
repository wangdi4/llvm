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

    void EmitFunctionBodyStart() override;
    void EmitFunctionBodyEnd() override;
    void EmitInstruction(const MachineInstr *MI) override;
  };
} // end of anonymous namespace

void LPUAsmPrinter::EmitFunctionBodyStart() {
  const MachineRegisterInfo *MRI;
  MRI = &MF->getRegInfo();
  OutStreamer.EmitRawText("\t.group");
  OutStreamer.EmitRawText("\t.result\t%c0, 0, 1");
  OutStreamer.EmitRawText("\t.param\t%c1, 0, 1");

  // iterate over all "registers" (LICs) (should be an iterator for that...)
  // 
  // FIXME!!!
  for(unsigned reg=LPU::C2; reg<LPU::C4091; reg++) {
    SmallString<128> Str;
    raw_svector_ostream O(Str);
    if (MRI->isPhysRegUsed(reg)) {
      if (reg<=LPU::C3) {
        O << "\t.result\t.reg\t"<<LPUInstPrinter::getRegisterName(reg)<<", .i64";
      } else if (reg<=LPU::C19) {
        O << "\t.param\t.reg\t"<<LPUInstPrinter::getRegisterName(reg)<<", .i64";
      } else {
        O << "\t.reg\t"<<LPUInstPrinter::getRegisterName(reg)<<", .i64";
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
