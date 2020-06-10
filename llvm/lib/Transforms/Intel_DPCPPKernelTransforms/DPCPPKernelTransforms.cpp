//==----- DPCPPKernelTransforms.cpp - passes initialization -*- C++ -*------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/InitializePasses.h"

using namespace llvm;

void llvm::initializeIntel_DPCPPKernelTransforms(PassRegistry &Registry) {
  initializeDPCPPKernelVecClonePass(Registry);
  initializeDPCPPKernelPostVecPass(Registry);
  initializeDPCPPKernelWGLoopCreatorLegacyPassPass(Registry);
  initializeDPCPPKernelAnalysisPass(Registry);
  initializePhiCanonicalizationPass(Registry);
  initializeRedundantPhiNodePass(Registry);
  initializeSplitBBonBarrierPass(Registry);
  initializeWIRelatedValuePass(Registry);
  initializeDataPerBarrierPass(Registry);
  initializeDataPerValuePass(Registry);
  initializeKernelBarrierPass(Registry);
  initializeBarrierInFunctionPass(Registry);
  initializePostBarrierPass(Registry);
}

