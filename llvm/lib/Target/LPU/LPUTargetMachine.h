//===-- LPUTargetMachine.h - TargetMachine for the LPU backend ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TargetMachine that is used by the LPU backend.
//
//===----------------------------------------------------------------------===//

#ifndef LPUTARGETMACHINE_H
#define LPUTARGETMACHINE_H

#include "LPUSubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

class LPUTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  LPUSubtarget Subtarget;
public:
  LPUTargetMachine(const Target &T, StringRef TT,
                 StringRef CPU, StringRef FS, const TargetOptions &Options,
                 Reloc::Model RM, CodeModel::Model CM,
		   CodeGenOpt::Level OL);
  ~LPUTargetMachine() override;

  const LPUSubtarget *getSubtargetImpl() const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

};

extern Target TheLPUTarget;

} // End llvm namespace


#endif
