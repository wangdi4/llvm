//===------ Passes.h - Constructors for HIR transformations -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the Intel_LoopTransforms library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PASSES_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PASSES_H

namespace llvm {

class FunctionPass;

/// createHIRCodeGenPass - This creates a pass that generates LLVM IR from HIR.
FunctionPass *createHIRCodeGenPass();

/// createHIRCompleteUnrollPass - This creates a pass that performs complete 
/// unrolling on small trip count HIR loops.
FunctionPass *createHIRCompleteUnrollPass(int Threshold = -1);
}

#endif
