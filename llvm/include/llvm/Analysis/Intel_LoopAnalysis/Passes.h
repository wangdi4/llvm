//===--------- Passes.h - Constructors for HIR analyses ---------*- C++ -*-===//
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

/// Creates analysis which can provide a data dependence graph of an HLNode
FunctionPass *createDDAnalysisPass();

/// Assigns a symbase to all ddrefs, which groups dd refs into sets that
/// never alias
FunctionPass *createSymbaseAssignmentPass();
}

#endif
