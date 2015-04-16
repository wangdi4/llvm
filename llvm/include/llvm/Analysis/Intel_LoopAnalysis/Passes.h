//===--------- Passes.h - Constructors for HIR analyses ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

/// createHIRCreationPass - This creates a pass that forms HIR nodes.
FunctionPass *createHIRCreationPass();

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
