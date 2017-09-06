//===-- CSAMCTargetDesc.cpp - CSA Target Descriptions ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides CSA specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "CSAMCTargetDesc.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "CSAMCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace llvm {
namespace MCOI {
  enum /* OperandType */ {
    // Target specific operand types.
    OPERAND_REG_IMM = llvm::MCOI::OPERAND_FIRST_TARGET
  };
}
}

#define GET_INSTRINFO_MC_DESC
#include "CSAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "CSAGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "CSAGenRegisterInfo.inc"

static MCInstrInfo *createCSAMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitCSAMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createCSAMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitCSAMCRegisterInfo(X, CSA::FP); // TODO: Fix R0 - just picked a reg...
  return X;
}

static MCSubtargetInfo *createCSAMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                                    StringRef FS) {
  return createCSAMCSubtargetInfoImpl(TT, CPU, FS);
}

static MCInstPrinter *createCSAMCInstPrinter(const Triple &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new CSAInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" void LLVMInitializeCSATargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<CSAMCAsmInfo> X(getTheCSATarget());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheCSATarget(), createCSAMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheCSATarget(),
                                    createCSAMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheCSATarget(),
                                          createCSAMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheCSATarget(),
                                        createCSAMCInstPrinter);
}
