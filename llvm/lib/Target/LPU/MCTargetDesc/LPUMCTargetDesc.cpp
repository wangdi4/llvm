//===-- LPUMCTargetDesc.cpp - LPU Target Descriptions ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides LPU specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "LPUMCTargetDesc.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUMCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "LPUGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LPUGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LPUGenRegisterInfo.inc"

static MCInstrInfo *createLPUMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitLPUMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createLPUMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitLPUMCRegisterInfo(X, LPU::R0); // TODO: Fix R0 - just picked a reg...
  return X;
}

static MCSubtargetInfo *createLPUMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                    StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitLPUMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createLPUMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                                CodeModel::Model CM,
                                                CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createLPUMCInstPrinter(const Target &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI,
                                                const MCSubtargetInfo &STI) {
  if (SyntaxVariant == 0)
    return new LPUInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" void LLVMInitializeLPUTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<LPUMCAsmInfo> X(TheLPUTarget);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheLPUTarget,
                                        createLPUMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheLPUTarget, createLPUMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheLPUTarget,
                                    createLPUMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheLPUTarget,
                                          createLPUMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheLPUTarget,
                                        createLPUMCInstPrinter);
}
