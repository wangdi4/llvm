//===-- LPUTargetMachine.cpp - Define TargetMachine for LPU ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the LPU target.
//
//===----------------------------------------------------------------------===//

#include "LPUTargetMachine.h"
#include "LPU.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeLPUTarget() {
  // Register the target.
  RegisterTargetMachine<LPUTargetMachine> X(TheLPUTarget);
}

LPUTargetMachine::LPUTargetMachine(const Target &T, StringRef TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

LPUTargetMachine::~LPUTargetMachine() {}

namespace {
/// LPU Code Generator Pass Configuration Options.
class LPUPassConfig : public TargetPassConfig {
public:
  LPUPassConfig(LPUTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  LPUTargetMachine &getLPUTargetMachine() const {
    return getTM<LPUTargetMachine>();
  }

  bool addInstSelector() override;
  //void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *LPUTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new LPUPassConfig(this, PM);
}

bool LPUPassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createLPUISelDag(getLPUTargetMachine(), getOptLevel()));
  return false;
}
/*
void LPUPassConfig::addPreEmitPass() {
  // Must run branch selection immediately preceding the asm printer.
  //addPass(createLPUBranchSelectionPass(), false);
}
*/
