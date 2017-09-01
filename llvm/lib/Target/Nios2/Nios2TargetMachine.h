//===-- Nios2TargetMachine.h - Define TargetMachine for Nios2 -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Nios2 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2TARGETMACHINE_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2TARGETMACHINE_H

#include "MCTargetDesc/Nios2ABIInfo.h"
#include "Nios2Subtarget.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class formatted_raw_ostream;
class Nios2RegisterInfo;

class Nios2TargetMachine : public LLVMTargetMachine {
  bool isLittle;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  // Selected ABI
  Nios2ABIInfo ABI;
  Nios2Subtarget DefaultSubtarget;

  mutable StringMap<std::unique_ptr<Nios2Subtarget>> SubtargetMap;
public:
  Nios2TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL, bool isLittle);
  ~Nios2TargetMachine() override;

  const Nios2Subtarget *getSubtargetImpl() const {
    return &DefaultSubtarget;
  }

  const Nios2Subtarget *getSubtargetImpl(const Function &F) const override;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
  bool isLittleEndian() const { return isLittle; }
  const Nios2ABIInfo &getABI() const { return ABI; }
};

/// Nios2elTargetMachine - Nios2 little endian target machine.
///
class Nios2elTargetMachine : public Nios2TargetMachine {
  virtual void anchor();
public:
  Nios2elTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      Optional<Reloc::Model> RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);
};
} // End llvm namespace

#endif
