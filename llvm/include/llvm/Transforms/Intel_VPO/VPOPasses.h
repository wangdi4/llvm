#if INTEL_COLLAB // -*- C++ -*-
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

#if INTEL_CUSTOMIZATION
FunctionPass *createVPODriverPass();
FunctionPass *createVPODriverHIRPass();
FunctionPass *createVPODirectiveCleanupPass();
#endif // INTEL_CUSTOMIZATION

FunctionPass *createVPOCFGRestructuringPass();
// 0x5 is equivalent to ParPrepare | OmpPar
FunctionPass *createVPOParoptPreparePass(unsigned Mode = 0x5u,
  const std::vector<std::string> &OffloadTargets = {});
// 0x6 is equivalent to ParTrans | OmpPar
ModulePass   *createVPOParoptPass(unsigned Mode = 0x6u,
  const std::vector<std::string> &OffloadTargets = {}, unsigned OptLevel = 2);
  // The default value of OptLevel is set to 2 so that the test case in
  // the llvm-lit tests can pass. Currently there is no mechanism
  // to pass the vaule of OptLevel into the VPOParoptTrasnform from the opt
  // command line.
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
#endif // INTEL_COLLAB
