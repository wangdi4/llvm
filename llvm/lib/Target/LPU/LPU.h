//===-- LPU.h - Top-level interface for LPU representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM LPU back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_LPU_H
#define LLVM_LIB_TARGET_LPU_LPU_H

#include "MCTargetDesc/LPUMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class LPUTargetMachine;
  class FunctionPass;
  class MachineFunctionPass;

  //ImmutablePass *createLPUTargetTransformInfoPass(const LPUTargetMachine *TM);
  FunctionPass *createLPUISelDag(LPUTargetMachine &TM,
                                 llvm::CodeGenOpt::Level OptLevel);
  MachineFunctionPass *createLPUConvertControlPass();
  MachineFunctionPass *createLPUCvtCFDFPass();
  MachineFunctionPass *createLPUOptDFPass();
  MachineFunctionPass *createLPUAllocUnitPass();
  MachineFunctionPass *createLPUPrologEpilogPass();
  /// \brief Creates an LPU-specific Target Transformation Info pass.
  ImmutablePass *createLPUTargetTransformInfoPass(const LPUTargetMachine *TM);
  //FunctionPass *createLPULowerStructArgsPass();

} // end namespace llvm;

#endif
