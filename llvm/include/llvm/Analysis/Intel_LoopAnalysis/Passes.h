//===--------- Passes.h - Constructors for HIR analyses ---------*- C++ -*-===//
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
// in the Intel_LoopAnalysis library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_PASSES_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_PASSES_H

namespace llvm {

class FunctionPass;

/// This creates a pass that identifies HIR regions.
FunctionPass *createHIRRegionIdentificationWrapperPass();

/// This creates a pass that identifies SCCs in the regions.
FunctionPass *createHIRSCCFormationWrapperPass();

/// This creates a pass which serves as the public interface of HIR Framework.
FunctionPass *createHIRFrameworkWrapperPass();

/// Creates analysis which can provide a data dependence graph of an HLNode
FunctionPass *createHIRDDAnalysisPass();

/// Identify Safe Reduction Chain
FunctionPass *createHIRSafeReductionAnalysisPass();

/// Identify Sparse Array Reduction Chain
FunctionPass *createHIRSparseArrayReductionAnalysisPass();

/// Computes the locality cost for HLLoops which are used during
/// transformations.
FunctionPass *createHIRLocalityAnalysisPass();

/// Compute the loop resource for HIR Loops which help in cost models of
/// different transformations.
FunctionPass *createHIRLoopResourceWrapperPass();

/// Compute statistics for HIR Loops.
FunctionPass *createHIRLoopStatisticsWrapperPass();

/// Creates analysis which can provide parallel/vector candidate analysis
FunctionPass *createHIRParVecAnalysisPass();
}

#endif
