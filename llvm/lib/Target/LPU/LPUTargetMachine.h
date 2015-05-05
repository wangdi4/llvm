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

#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

struct LPUTargetMachine : public TargetMachine {
  LPUTargetMachine(const Target &T, StringRef TT,
                 StringRef CPU, StringRef FS, const TargetOptions &Options,
                 Reloc::Model RM, CodeModel::Model CM,
                 CodeGenOpt::Level OL)
    : TargetMachine(T, TT, CPU, FS, Options) { }

  virtual bool addPassesToEmitFile(PassManagerBase &PM,
                                   formatted_raw_ostream &Out,
                                   CodeGenFileType FileType,
                                   bool DisableVerify,
				   AnalysisID StartAfter,
				   AnalysisID StartBefore
				   );
};

extern Target TheLPUTarget;

} // End llvm namespace


#endif
