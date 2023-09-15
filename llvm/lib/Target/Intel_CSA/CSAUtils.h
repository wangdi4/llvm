//===-- llvm/lib/Target/Intel_CSA/CSAUtils.h --------------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains code common to multiple passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAUTILS_H
#define LLVM_LIB_TARGET_CSA_CSAUTILS_H

#include "CSA.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
using namespace llvm;

namespace csa_utils {
bool isAlwaysDataFlowLinkageSet(void);
bool createSCG(void);
bool verifyBackedges(void);
bool markHLLICsAsBackedges(void);
bool reportWarningForExtCalls(void);
unsigned createUseTree(MachineBasicBlock *, MachineBasicBlock::iterator,
                       unsigned, const SmallVector<unsigned, 4>,
                       unsigned unusedReg = CSA::IGN);
unsigned createPickTree(MachineBasicBlock *, MachineBasicBlock::iterator,
                        const TargetRegisterClass *,
                        const SmallVector<unsigned, 4>,
                        const SmallVector<unsigned, 4>,
                        unsigned unusedReg = CSA::IGN);
void createSwitchTree(MachineBasicBlock *, MachineBasicBlock::iterator,
                      const TargetRegisterClass *,
                      const SmallVector<unsigned, 4>,
                      SmallVector<unsigned, 4> &, unsigned, unsigned,
                      unsigned unusedReg = CSA::IGN);
} // namespace csa_utils

#endif
