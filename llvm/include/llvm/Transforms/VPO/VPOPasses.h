#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
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
FunctionPass *createVPOParoptSharedPrivatizationPass(unsigned Mode = 0u);
ModulePass   *createVPOParoptTargetInlinePass();
FunctionPass *createVPOParoptApplyConfigPass();
#endif // INTEL_CUSTOMIZATION

FunctionPass *createVPOCFGRestructuringPass();
FunctionPass *createVPOCFGSimplifyPass();
FunctionPass *createVPOParoptLoopCollapsePass();
FunctionPass *createVPOParoptLoopTransformPass();
// 0x5 is equivalent to ParPrepare | OmpPar
FunctionPass *createVPOParoptPreparePass(unsigned Mode = 0x5u);
FunctionPass *createVPORestoreOperandsPass();
FunctionPass *createVPORenameOperandsPass();
FunctionPass *createVPOParoptGuardMemoryMotionPass();
// 0x6 is equivalent to ParTrans | OmpPar
ModulePass   *createVPOParoptPass(unsigned Mode = 0x6u);
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
#endif // INTEL_COLLAB
