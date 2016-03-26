//========-- VPOPasses.h - Declarations for for VPO passes -*- C++ -*--=======//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines all the passes for VPO.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_VPO_VPOPASSES_H
#define LLVM_TRANSFORMS_VPO_VPOPASSES_H

namespace llvm {

class FunctionPass;
class ModulePass;

FunctionPass *createVPODriverPass();
FunctionPass *createVPODriverHIRPass();
FunctionPass *createVPODirectiveCleanupPass();
FunctionPass *createVPOCFGRestructuringPass();
ModulePass   *createVPOParoptPass();
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
