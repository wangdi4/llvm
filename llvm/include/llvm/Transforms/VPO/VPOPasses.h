#if INTEL_COLLAB // -*- C++ -*-
//===--------- VPOPasses.h - Declarations for VPO passes -*- C++ -*--------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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

#if INTEL_CUSTOMIZATION
FunctionPass *createVPODirectiveCleanupPass();
FunctionPass *createVPOParoptOptimizeDataSharingPass();
#endif // INTEL_CUSTOMIZATION

FunctionPass *createVPOCFGRestructuringPass();
FunctionPass *createVPOParoptLoopCollapsePass();
// 0x5 is equivalent to ParPrepare | OmpPar
FunctionPass *createVPOParoptPreparePass(unsigned Mode = 0x5u);
FunctionPass *createVPORestoreOperandsPass();
// 0x6 is equivalent to ParTrans | OmpPar
ModulePass   *createVPOParoptPass(unsigned Mode = 0x6u);
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
#endif // INTEL_COLLAB
