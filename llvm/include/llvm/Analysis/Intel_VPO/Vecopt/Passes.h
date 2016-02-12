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

/// createIdentifyVectCandidatesPass - This creates a pass that idenitifies
/// candidate loops for vectorization.
FunctionPass *createIdentifyVectorCandidatesPass();
}

#endif // LLVM_ANALYSIS_VPO_PASSES_H
