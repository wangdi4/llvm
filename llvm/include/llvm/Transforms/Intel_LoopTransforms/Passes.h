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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// createHIRPrinterPass - This creates a pass that prints HIR.
FunctionPass *createHIRPrinterPass(raw_ostream &OS, const std::string &Banner);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

/// createOptPredicatePass - This creates a pass that performs OptPredicate
/// transformation on HIR.
FunctionPass *createHIROptPredicatePass(bool EnablePartialUnswitch = true,
                                        bool KeepLoopnestPerfect = false);

/// createHIRPreVecCompleteUnrollPass - This creates a pass that performs
/// complete unrolling before vectorizer.
FunctionPass *createHIRPreVecCompleteUnrollPass(unsigned OptLevel = 0,
                                                bool PragmaOnlyUnroll = false);

/// createHIRPostVecCompleteUnrollPass - This creates a pass that performs
/// complete
/// unrolling after vectorizer.
FunctionPass *createHIRPostVecCompleteUnrollPass(unsigned OptLevel = 0,
                                                 bool PragmaOnlyUnroll = false);

/// createHIRRuntimeDDPass - This creates a HIR Loop pass that is used
/// for Runtime DD transformation.
FunctionPass *createHIRRuntimeDDPass();

/// createHIRUnrollAndJamPass - This creates a pass that performs unroll & jam
/// on loops.
FunctionPass *createHIRUnrollAndJamPass(bool PragmaOnlyUnroll = false);

/// createHIRSymbolicTripCountCompleteUnrollLegacyPass - This creates a HIR Loop
/// pass that performs Loop based pattern matching.
FunctionPass *createHIRPMSymbolicTripCountCompleteUnrollLegacyPass();

/// createHIRParDirInsertPass - This creates a pass that injects
/// directives for auto parallelization loops.
FunctionPass *createHIRParDirInsertPass();

/// createHIRVecDirInsertPass - This creates a pass that injects
/// directives for auto vectorization candidate loops.
FunctionPass *createHIRVecDirInsertPass(bool OuterVec = true);

/// createHIRScalarReplArrayPass - This creates a HIR Loop pass that performs
/// Scalar Replacement over Array access
FunctionPass *createHIRScalarReplArrayPass();

/// Creates pass that splits loops based on variant predicates.
FunctionPass *createHIROptVarPredicatePass();

/// This creates a pass that emits HIR opt report.
FunctionPass *createHIROptReportEmitterWrapperPass();

/// Create pass that propagate casted IV for memory references.
FunctionPass *createHIRPropagateCastedIVPass();

/// Creates a pass that recognizes parallel loops.
FunctionPass *createHIRRecognizeParLoopPass();

/// Create pass that prefetches memrefs.
FunctionPass *createHIRPrefetchingPass();

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

/// Create pass that implements non-zero sinking for perfect loopnest.
FunctionPass *createHIRNonZeroSinkingForPerfectLoopnestPass();

/// Create pass that marks stores as nontemporal where appropriate.
FunctionPass *createHIRNontemporalMarkingPass();

} // namespace llvm

#endif
