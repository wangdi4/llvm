//===-- CSATargetMachine.h - TargetMachine for the CSA backend ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TargetMachine that is used by the CSA backend.
//
//===----------------------------------------------------------------------===//

#ifndef CSATARGETMACHINE_H
#define CSATARGETMACHINE_H

#include "CSASubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class CSATargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  CSASubtarget Subtarget;
public:
  CSATargetMachine(const Target &T, const Triple &TT,
                 StringRef CPU, StringRef FS, const TargetOptions &Options,
                 Optional<Reloc::Model> RM, CodeModel::Model CM,
		   CodeGenOpt::Level OL);
  ~CSATargetMachine() override;

  const CSASubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  const CSASubtarget *getSubtargetImpl() const {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(legacy::PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

};

} // End llvm namespace


#endif
