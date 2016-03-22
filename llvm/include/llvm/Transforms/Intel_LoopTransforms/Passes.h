//===------ Passes.h - Constructors for HIR transformations -----*- C++ -*-===//
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
// in the Intel_LoopTransforms library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PASSES_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_PASSES_H

namespace llvm {

class FunctionPass;

/// createHIRSSADeconstructionPass - This creates a pass which desconstructs SSA
/// for HIR creation.
FunctionPass *createHIRSSADeconstructionPass();

/// createHIRPrinterPass - This creates a pass that prints HIR.
FunctionPass *createHIRPrinterPass(raw_ostream &OS, const std::string &Banner);

/// createHIRCodeGenPass - This creates a pass that generates LLVM IR from HIR.
FunctionPass *createHIRCodeGenPass();

/// createHIRCodeGenPass - This creates a pass that performs OptPredicate 
/// transformation on HIR.
FunctionPass *createHIROptPredicatePass(int Threshold = -1);

/// createHIRCompleteUnrollPass - This creates a pass that performs complete
/// unrolling on small trip count HIR loops.
FunctionPass *createHIRCompleteUnrollPass(int Threshold = -1);

/// createHIRDistributionPass - This creates a pass that performs Loop
/// Distribution for perfect nest formation or breaking memory recurrences
FunctionPass *createHIRLoopDistributionPass(bool FormPerfectNest = true);

/// createHIRInterchangePass - This creates a pass that performs Loop
/// Interchange
FunctionPass *createHIRLoopInterchangePass();

/// createHIRDummyTransformationPass - This creates a dummy pass that is used
/// for debugging purposes.
FunctionPass *createHIRRuntimeDDPass();

/// createHIRDummyTransformationPass - This creates a dummy pass that is used
/// for debugging purposes.
FunctionPass *createHIRDummyTransformationPass();

/// createHIRGeneralUnrollPass - This creates a pass that performs general
/// unrolling for larger trip count HIR loops.
FunctionPass *createHIRGeneralUnrollPass(int Threshold = -1, int UFactor = -1);
}

#endif
