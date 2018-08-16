//===-- CSA.h - Top-level interface for CSA representation ------*- C++ -*-===//
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
// This file contains the entry points for global functions defined in
// the LLVM CSA back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSA_H
#define LLVM_LIB_TARGET_CSA_CSA_H

#include "MCTargetDesc/CSAMCTargetDesc.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class CSATargetMachine;
class FunctionPass;
class MachineFunctionPass;

FunctionPass *createCSAISelDag(CSATargetMachine &TM,
                               llvm::CodeGenOpt::Level OptLevel);
MachineFunctionPass *createCSAConvertControlPass();
MachineFunctionPass *createControlDepenceGraph();
MachineFunctionPass *createCSACvtCFDFPass();
MachineFunctionPass *createCSAStatisticsPass();
MachineFunctionPass *createCSAProcCallsPass();
MachineFunctionPass *createCSAOptDFPass();
MachineFunctionPass *createCSAMultiSeqPass();
MachineFunctionPass *createCSADFParLoopPass();
MachineFunctionPass *createCSARedundantMovElimPass();
MachineFunctionPass *createCSADeadInstructionElimPass();
MachineFunctionPass *createCSAAllocUnitPass();
MachineFunctionPass *createCSAPrologEpilogPass();
MachineFunctionPass *createCSAExpandInlineAsmPass();
MachineFunctionPass *createCSAMemopOrderingPass();
MachineFunctionPass *createCSANormalizeDebugPass();
MachineFunctionPass *createCSADataflowCanonicalizationPass();
MachineFunctionPass *createCSAStreamingMemoryConversionPass();
MachineFunctionPass *createCSANameLICsPass();
MachineFunctionPass *createCSAReassocReducPass();
MachineFunctionPass *createCSARASReplayableLoadsDetectionPass();
// FunctionPass *createCSALowerStructArgsPass();
Pass *createCSAInnerLoopPrepPass();
Pass *createCSAReplaceAllocaWithMallocPass(CSATargetMachine &TM);
Pass *createCSAStreamingMemoryPrepPass();

} // namespace llvm

#endif
