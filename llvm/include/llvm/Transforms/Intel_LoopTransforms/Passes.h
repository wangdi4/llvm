//===------ Passes.h - Constructors for HIR transformations -----*- C++ -*-===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

/// createHIRCodeGenPass - This creates a pass that generates LLVM IR from HIR.
FunctionPass *createHIRCodeGenWrapperPass();

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

/// createHIRLoopDistributionForMemRecPass - This creates a pass that
/// performs Loop Distribution for breaking memory recurrences.
FunctionPass *createHIRLoopDistributionForMemRecPass();

/// createHIRLoopDistributionForLoopNestPass - This creates a pass that
/// performs Loop Distribution for enabling perfect Loop Nests.
FunctionPass *createHIRLoopDistributionForLoopNestPass();

/// createHIRLoopInterchangePass - This creates a pass that performs Loop
/// Interchange.
FunctionPass *createHIRLoopInterchangePass();

/// createHIRGenerateMKLCAllPass - This creates a pass that performs MKL call
/// generation.
FunctionPass *createHIRGenerateMKLCallPass();

/// createHIRLoopBlockingPass - This creates a pass that performs Loop
/// Blocking.
FunctionPass *createHIRLoopBlockingPass();

/// createHIRLoopRematerializePass - This creates a pass that performs Loop
/// Rematerialize.
FunctionPass *createHIRLoopRematerializePass();

/// createHIRLoopRerollPass - This creates a pass that performs Loop
/// Reroll.
FunctionPass *createHIRLoopRerollPass();

/// createHIRRuntimeDDPass - This creates a HIR Loop pass that is used
/// for Runtime DD transformation.
FunctionPass *createHIRRuntimeDDPass();

/// createHIRDummyTransformationPass - This creates a dummy pass that is used
/// for debugging purposes.
FunctionPass *createHIRDummyTransformationPass();

/// createHIRGeneralUnrollPass - This creates a pass that performs general
/// unrolling for larger trip count HIR loops.
FunctionPass *createHIRGeneralUnrollPass(bool PragmaOnlyUnroll = false);

/// createHIRUnrollAndJamPass - This creates a pass that performs unroll & jam
/// on loops.
FunctionPass *createHIRUnrollAndJamPass(bool PragmaOnlyUnroll = false);

/// createHIRLoopReversalPass - This creates a HIR Loop pass that performs Loop
/// Reversal.
FunctionPass *createHIRLoopReversalPass();

/// createHIRLMMPass - This creates a HIR Loop pass that performs Loop
/// Memory Motion.
FunctionPass *createHIRLMMPass();

/// createHIRLoopCollapsePass - This creates a HIR Loop pass that performs Loop
/// Collapse
FunctionPass *createHIRLoopCollapsePass();

/// createHIRSymbolicTripCountCompleteUnrollLegacyPass - This creates a HIR Loop
/// pass that performs Loop based pattern matching.
FunctionPass *createHIRSymbolicTripCountCompleteUnrollLegacyPass();

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

/// Creates pass that replaces loads and stores with memsets and memcpys.
FunctionPass *createHIRIdiomRecognitionPass();

/// Creates pass that multiversions loop for the probable trip count value.
FunctionPass *createHIRMVForConstUBPass();

/// Creates pass that performs loop concatenation.
FunctionPass *createHIRLoopConcatenationPass();

/// Creates pass that performs array transpose.
FunctionPass *createHIRArrayTransposePass();

/// Creates pass that performs Aos To Soa.
FunctionPass *createHIRAosToSoaPass();

/// Creates pass that fuses loops.
FunctionPass *createHIRLoopFusionPass();

/// This creates a pass that emits HIR opt report.
FunctionPass *createHIROptReportEmitterWrapperPass();

/// Create pass that eliminates dead store.
FunctionPass *createHIRDeadStoreEliminationPass();

/// Create pass that computes last value of a temp.
FunctionPass *createHIRLastValueComputationPass();

/// Create pass that propagate casted IV for memory references.
FunctionPass *createHIRPropagateCastedIVPass();

/// Create pass that rerolls multi-exit loops.
FunctionPass *createHIRMultiExitLoopRerollPass();

/// Creates a pass that recognizes parallel loops.
FunctionPass *createHIRRecognizeParLoopPass();

/// Create pass that recognize identity matrix.
FunctionPass *createHIRIdentityMatrixIdiomRecognitionPass();

/// Create pass that prefetches memrefs.
FunctionPass *createHIRPrefetchingPass();

/// Create pass that enables sinking for perfect Loop nest.
FunctionPass *createHIRSinkingForPerfectLoopnestPass();

/// Create pass that enables undosinking for perfect Loop nest.
FunctionPass *createHIRUndoSinkingForPerfectLoopnestPass();

/// Create pass that enables sinking for conditional temps.
FunctionPass *createHIRConditionalTempSinkingPass();
/// Create pass that sinks loop invariant memory reductions.
FunctionPass *createHIRMemoryReductionSinkingPass();
} // namespace llvm

#endif
