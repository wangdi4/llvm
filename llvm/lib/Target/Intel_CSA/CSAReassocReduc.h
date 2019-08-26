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

/// Contains functions/types controlling reduction generation.
namespace csa_reduc {

/// Identifies which possible CSA reduction instructions should be
/// enabled/forced.
enum ReducLevel {

  /// No possible CSA reduction instructions.
  REDUC_LEVEL_NONE,

  /// Only add/sub instructions, possibly with fissioned fmas.
  REDUC_LEVEL_ADD,

  /// All possible reduction instructions, including muls and fmas.
  REDUC_LEVEL_ALL,
};

/// Which sets of reduction instructions should be enabled in seqopt.
ReducLevel reducsEnabled();

/// Which sets of reduction instructions should be forced in seqopt and
/// and generated even in the absence of relevant FP flags.
ReducLevel reducsForced();

/// Whether to break FMA reductions into mulf + redaddf.
bool fissionFMAs();

} // namespace csa_reduc

namespace llvm {
class MachineFunctionPass;
class PassRegistry;

MachineFunctionPass *createCSAReassocReducPass();

void initializeCSAReassocReducPass(PassRegistry &);

} // namespace llvm

#endif
