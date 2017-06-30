//===-- CSA.h - Top-level interface for CSA representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM CSA back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSA_H
#define LLVM_LIB_TARGET_CSA_CSA_H

#include "MCTargetDesc/CSAMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class CSATargetMachine;
  class FunctionPass;
  class MachineFunctionPass;

  //ImmutablePass *createCSATargetTransformInfoPass(const CSATargetMachine *TM);
  FunctionPass *createCSAISelDag(CSATargetMachine &TM,
                                 llvm::CodeGenOpt::Level OptLevel);
  MachineFunctionPass *createCSAConvertControlPass();
  MachineFunctionPass *createControlDepenceGraph();
  MachineFunctionPass *createCSACvtCFDFPass();
  MachineFunctionPass *createCSAStatisticsPass();
  MachineFunctionPass *createCSAOptDFPass();
  MachineFunctionPass *createCSADFParLoopPass();
  MachineFunctionPass *createCSARedundantMovElimPass();
  MachineFunctionPass *createCSADeadInstructionElimPass();
  MachineFunctionPass *createCSAAllocUnitPass();
  MachineFunctionPass *createCSAPrologEpilogPass();
  MachineFunctionPass *createCSAExpandInlineAsmPass();
  MachineFunctionPass *createCSAMemopOrderingPass();
  MachineFunctionPass *createCSANormalizeDebugPass();
  /// \brief Creates an CSA-specific Target Transformation Info pass.
  ImmutablePass *createCSATargetTransformInfoPass(const CSATargetMachine *TM);
  //FunctionPass *createCSALowerStructArgsPass();

} // end namespace llvm;

#endif
