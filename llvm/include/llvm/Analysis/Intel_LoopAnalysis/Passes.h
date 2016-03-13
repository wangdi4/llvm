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

/// createRegionIdentificationPass - This creates a pass that identifies HIR
/// regions.
FunctionPass *createRegionIdentificationPass();

/// createSCCFormationPass - This creates a pass that identifies SCCs in the
/// regions.
FunctionPass *createSCCFormationPass();

/// createScalarSymbaseAssignmentPass - This creates a pass that assigns
/// symbase to livein/liveout scalars.
FunctionPass *createScalarSymbaseAssignmentPass();

/// createHIRCreationPass - This creates a pass that forms HIR nodes.
FunctionPass *createHIRCreationPass();

/// createHIRCleanupPass - This creates a pass that cleans up redundant HIR
/// nodes.
FunctionPass *createHIRCleanupPass();

/// createLoopFormationPass - This creates a pass that forms HIR loops.
FunctionPass *createLoopFormationPass();

/// createHIRParserPass - This creates a pass that maps populates DDRefs by
/// parsing SCEVs into CanonExprs.
FunctionPass *createHIRParserPass();

/// Assigns a symbase to all ddrefs, which groups dd refs into sets that
/// never alias
FunctionPass *createSymbaseAssignmentPass();

/// createHIRFrameworkPass - This creates a pass which serves as the public
/// interface of HIR Framework.
FunctionPass *createHIRFrameworkPass();

/// Creates analysis which can provide a data dependence graph of an HLNode
FunctionPass *createDDAnalysisPass();

/// Computes the locality cost for HLLoops which are used during
/// transformations.
FunctionPass *createHIRLocalityAnalysisPass();

/// Creates analysis which can provide parallel/vector candidate analysis
FunctionPass *createHIRParVecAnalysisPass();
}

#endif
