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
  MachineFunctionPass *createCSAOptDFPass();
  MachineFunctionPass *createCSADFParLoopPass();
  MachineFunctionPass *createCSARedundantMovElimPass();
  MachineFunctionPass *createCSADeadInstructionElimPass();
  MachineFunctionPass *createCSAAllocUnitPass();
  MachineFunctionPass *createCSAPrologEpilogPass();
  MachineFunctionPass *createCSAExpandInlineAsmPass();
  MachineFunctionPass *createCSAMemopOrderingPass();
  MachineFunctionPass *createCSAIndependentMemopOrderingPass();
  MachineFunctionPass *createCSANormalizeDebugPass();
  MachineFunctionPass *createCSAStreamingMemoryConversionPass();
  //FunctionPass *createCSALowerStructArgsPass();

} // end namespace llvm;

// Options that are currently shared between both memory ordering passes (and
// CSATargetMachine to select which one to run).
namespace csa_memop_ordering_shared_options {

// Flag for controlling code that deals with memory ordering.
enum OrderMemopsMode {
  // No extra code added at all for ordering.  Often incorrect.
  none = 0,

  // Linear ordering of all memops.  Dumb but should be correct.
  linear = 1,

  //  Stores inside a basic block are totally ordered.
  //  Loads ordered between the stores, but
  //  unordered with respect to each other.
  //  No reordering across basic blocks.
  wavefront = 2,

  // Optimal (but larger) ordering chains.
  independent = 3
};

extern llvm::cl::opt<OrderMemopsMode> OrderMemopsType;
extern llvm::cl::opt<int> OrderMemops;
extern llvm::cl::opt<bool> ParallelOrderMemops;

}



#endif
