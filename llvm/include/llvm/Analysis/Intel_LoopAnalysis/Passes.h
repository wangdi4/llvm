//===--------- Passes.h - Constructors for HIR analyses ---------*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the Intel_LoopAnalysis library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_PASSES_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_PASSES_H

namespace llvm {

class FunctionPass;

/// createHIRRegionIdentificationPass - This creates a pass that identifies HIR
/// regions.
FunctionPass *createHIRRegionIdentificationPass();

/// createHIRSCCFormationPass - This creates a pass that identifies SCCs in the
/// regions.
FunctionPass *createHIRSCCFormationPass();

/// createHIRScalarSymbaseAssignmentPass - This creates a pass that assigns
/// symbase to livein/liveout scalars.
FunctionPass *createHIRScalarSymbaseAssignmentPass();

/// createHIRCreationPass - This creates a pass that forms HIR nodes.
FunctionPass *createHIRCreationPass();

/// createHIRCleanupPass - This creates a pass that cleans up redundant HIR
/// nodes.
FunctionPass *createHIRCleanupPass();

/// createHIRLoopFormationPass - This creates a pass that forms HIR loops.
FunctionPass *createHIRLoopFormationPass();

/// createHIRParserPass - This creates a pass that maps populates DDRefs by
/// parsing SCEVs into CanonExprs.
FunctionPass *createHIRParserPass();

/// Assigns a symbase to all ddrefs, which groups dd refs into sets that
/// never alias
FunctionPass *createHIRSymbaseAssignmentPass();

/// createHIRFrameworkPass - This creates a pass which serves as the public
/// interface of HIR Framework.
FunctionPass *createHIRFrameworkPass();

/// Creates analysis which can provide a data dependence graph of an HLNode
FunctionPass *createHIRDDAnalysisPass();

/// Computes the locality cost for HLLoops which are used during
/// transformations.
FunctionPass *createHIRLocalityAnalysisPass();

/// Compute the loop resource for HIR Loops which help in 
/// cost models of different transformations.
FunctionPass *createHIRResourceAnalysisPass();

/// Creates analysis which can provide parallel/vector candidate analysis
FunctionPass *createHIRParVecAnalysisPass();

/// Computes the cost of VLS Groups for HLLoops which are used during
/// Vectorization.
FunctionPass *createHIRVectVLSAnalysisPass();
}

#endif
