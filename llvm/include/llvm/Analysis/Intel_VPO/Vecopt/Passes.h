//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   Passes.h -- Declares all Vecopt Analysis Passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_PASSES_H
#define LLVM_ANALYSIS_VPO_PASSES_H

namespace llvm {

class FunctionPass;

/// createAVRGeneratePass - This creates a pass that generates AVRs needed
/// for vectorization from LLVM IR.
FunctionPass *createAVRGeneratePass();

/// createAVRGeneratePass - This creates a pass that generates AVRs needed
/// for vectorization from HIR.
FunctionPass *createAVRGenerateHIRPass();

/// createAvrDefUsePass - This creates a pass that provides Def-Use/Use-Def
/// information on an AVR program.
FunctionPass *createAvrDefUsePass();

/// createAvrDefUseHIRPass - This creates a pass that provides Def-Use/Use-Def
/// information on an AVR program.
FunctionPass *createAvrDefUseHIRPass();

/// createAvrCFGPass - This creates a pass that constructs a
/// control-flow-graph for an AVR program.
FunctionPass *createAvrCFGPass();

/// createAvrCFGHIRPass - This creates a pass that constructs a
/// control-flow-graph for an AVR program.
FunctionPass *createAvrCFGHIRPass();

/// createSIMDLaneEvolutionPass - This creates a pass that calculates the
/// SIMD lane evolution of an AVR program.
FunctionPass *createSIMDLaneEvolutionPass();

/// createSIMDLaneEvolutionHIRPass - This creates a pass that calculates the
/// SIMD lane evolution of an AVR program.
FunctionPass *createSIMDLaneEvolutionHIRPass();

///
FunctionPass *createVectorGraphInfoPass();

FunctionPass *createVectorGraphPredicatorPass();
}

#endif // LLVM_ANALYSIS_VPO_PASSES_H
