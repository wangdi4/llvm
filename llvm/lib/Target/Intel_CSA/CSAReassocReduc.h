//===- CSAReassocReduc.h - CSA Reassociating Reductions ---------*- C++ -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file exposes the CSAReassocReduc pass and the function that determines
// whether floating point reduction ops should be generated for it.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAREASSOCREDUC_H
#define LLVM_LIB_TARGET_CSA_CSAREASSOCREDUC_H

namespace llvm {
class MachineFunction;
class MachineFunctionPass;
class PassRegistry;

bool willRunCSAReassocReduc(const MachineFunction &);

MachineFunctionPass *createCSAReassocReducPass();

void initializeCSAReassocReducPass(PassRegistry &);

} // namespace llvm

#endif
