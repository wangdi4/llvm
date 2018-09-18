//===-- CSATargetMachine.h - TargetMachine for the CSA backend --*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

void initializeCSAAllocUnitPassPass(PassRegistry &);
void initializeCSACvtCFDFPassPass(PassRegistry &);
void initializeCSADataflowCanonicalizationPassPass(PassRegistry &);
void initializeCSADeadInstructionElimPass(PassRegistry &);
void initializeCSAExpandInlineAsmPass(PassRegistry &);
void initializeCSAFortranIntrinsicsPass(PassRegistry &);
void initializeCSAInnerLoopPrepPass(PassRegistry &);
void initializeCSALowerAggrCopiesPass(PassRegistry &);
void initializeCSAMemopOrderingPass(PassRegistry &);
void initializeCSANameLICsPassPass(PassRegistry &);
void initializeCSANormalizeDebugPass(PassRegistry &);
void initializeCSAOptDFPassPass(PassRegistry &);
void initializeCSARedundantMovElimPass(PassRegistry &);
void initializeCSAStreamingMemoryConversionPassPass(PassRegistry &);
void initializeControlDependenceGraphPass(PassRegistry &);

class CSATargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  CSASubtarget Subtarget;

  bool addAsmPrinterWithAsmWrapping(PassManagerBase &PM, raw_pwrite_stream &Out,
                                    raw_pwrite_stream *DwoOut,
                                    CodeGenFileType FileType,
                                    MCContext &Context);

public:
  CSATargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                   CodeGenOpt::Level OL, bool JIT);
  ~CSATargetMachine() override;

  const CSASubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  const CSASubtarget *getSubtargetImpl() const { return &Subtarget; }

  TargetPassConfig *createPassConfig(legacy::PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  // This is used to inject passes before other LLVM optimizations run, which is
  // important for the loop intrinsic expansion pass because some optimizations
  // move parallel loop intrinsics in unhelpful ways.
  void adjustPassManager(PassManagerBuilder &) override;

  // This is overridden to set up assembly wrapping.
  bool addPassesToEmitFile(PassManagerBase &PM, raw_pwrite_stream &Out,
                           raw_pwrite_stream *DwoOut, CodeGenFileType FileType,
                           bool DisableVerify, MachineModuleInfo *MMI) override;
};

} // namespace llvm

#endif
