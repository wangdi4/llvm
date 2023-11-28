//===------ Passes.h - Constructors for HIR transformations -----*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

/// This creates a pass which desconstructs SSA for HIR creation.
FunctionPass *createHIRSSADeconstructionLegacyPass();

/// Creates a pass which cleans up unnecessary temps in HIR.
FunctionPass *createHIRTempCleanupPass();

/// createHIRPreVecCompleteUnrollPass - This creates a pass that performs
/// complete unrolling before vectorizer.
FunctionPass *createHIRPreVecCompleteUnrollPass(unsigned OptLevel = 0,
                                                bool PragmaOnlyUnroll = false);

/// createHIRPostVecCompleteUnrollPass - This creates a pass that performs
/// complete
/// unrolling after vectorizer.
FunctionPass *createHIRPostVecCompleteUnrollPass(unsigned OptLevel = 0,
                                                 bool PragmaOnlyUnroll = false);

/// createHIRUnrollAndJamPass - This creates a pass that performs unroll & jam
/// on loops.
FunctionPass *createHIRUnrollAndJamPass(bool PragmaOnlyUnroll = false);

/// createHIRVecDirInsertPass - This creates a pass that injects
/// directives for auto vectorization candidate loops.
FunctionPass *createHIRVecDirInsertPass(bool OuterVec = true);

/// Create pass that enables sinking for perfect Loop nest.
FunctionPass *createHIRSinkingForPerfectLoopnestPass();

/// Create pass that enables undosinking for perfect Loop nest.
FunctionPass *createHIRUndoSinkingForPerfectLoopnestPass();

/// Create pass that performs row-wise multiversioning.
FunctionPass *createHIRRowWiseMVPass();

/// Create pass that stores result into a temp array.
FunctionPass *createHIRStoreResultIntoTempArrayPass();

/// Create pass that performs sum window reuse.
FunctionPass *createHIRSumWindowReusePass();

} // namespace llvm

#endif
