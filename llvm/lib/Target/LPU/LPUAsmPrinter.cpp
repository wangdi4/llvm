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

    void printOperand(const MachineInstr *MI, int OpNum,
                      raw_ostream &O, const char* Modifier = nullptr);
    void EmitInstruction(const MachineInstr *MI) override;
  };
} // end of anonymous namespace


void LPUAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                    raw_ostream &O, const char *Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  switch (MO.getType()) {
  default: llvm_unreachable("Not implemented yet!");
  case MachineOperand::MO_Register:
    O << LPUInstPrinter::getRegisterName(MO.getReg());
    return;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    return;
  /*
  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    return;
  case MachineOperand::MO_GlobalAddress: {
    bool isMemOp  = Modifier && !strcmp(Modifier, "mem");
    uint64_t Offset = MO.getOffset();

    // If the global address expression is a part of displacement field with a
    // register base, we should not emit any prefix symbol here, e.g.
    //   mov.w &foo, r1
    // vs
    //   mov.w glb(r1), r2
    // Otherwise (!) msp430-as will silently miscompile the output :(
    if (!Modifier || strcmp(Modifier, "nohash"))
      O << (isMemOp ? '&' : '#');
    if (Offset)
      O << '(' << Offset << '+';

    O << *getSymbol(MO.getGlobal());

    if (Offset)
      O << ')';

    return;
  }
  */
  }
}

void LPUAsmPrinter::EmitFunctionBodyStart() {
  const MachineRegisterInfo *MRI;
  MRI = &MF->getRegInfo();
  OutStreamer.EmitRawText(StringRef("\t.result\tc0, 0, 1"));
  OutStreamer.EmitRawText(StringRef("\t.param\tc1, 0, 1"));

  // iterate over all "registers" (LICs) (should be an iterator for that...)
  // 
  for(unsigned reg=LPU::C2; reg<LPU::C2043; reg++) {
    SmallString<128> Str;
    raw_svector_ostream O(Str);
    if (MRI->isPhysRegUsed(reg)) {
      if (reg<=LPU::C3) {
        O << "\t.result\t"<<LPUInstPrinter::getRegisterName(reg)<<", i64, 1";
      } else if (reg<=LPU::C19) {
        O << "\t.param\t"<<LPUInstPrinter::getRegisterName(reg)<<", i64, 1";
      } else {
        O << "\t.lic\t"<<LPUInstPrinter::getRegisterName(reg)<<", i64, 2";
      }
      OutStreamer.EmitRawText(O.str());
    }
  }

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
