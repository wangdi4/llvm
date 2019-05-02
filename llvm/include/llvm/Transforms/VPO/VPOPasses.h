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
#endif // INTEL_CUSTOMIZATION

FunctionPass *createVPOCFGRestructuringPass();
// 0x5 is equivalent to ParPrepare | OmpPar
FunctionPass *createVPOParoptPreparePass(unsigned Mode = 0x5u);
FunctionPass *createVPORestoreOperandsPass();
// 0x6 is equivalent to ParTrans | OmpPar
ModulePass   *createVPOParoptPass(unsigned Mode = 0x6u, unsigned OptLevel = 2);
  // The default value of OptLevel is set to 2 so that the test case in
  // the llvm-lit tests can pass. Currently there is no mechanism
  // to pass the vaule of OptLevel into the VPOParoptTrasnform from the opt
  // command line.
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
#endif // INTEL_COLLAB
