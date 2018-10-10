#if INTEL_COLLAB
//===--- VPOParoptModuleTranform.h - Paropt Module Transforms ---*- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the ParOpt interface to perform module transformations
/// for OpenMP and Auto-parallelization.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H
#define LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#include <functional>

namespace llvm {

class Module;
class Function;
class Constant;
class Instruction;

namespace vpo {

/// Provide all functionalities to perform paropt threadization such as
/// outlining, privatization, loop partitioning, multithreaded code
/// generation for a Module.
class VPOParoptModuleTransform {

public:
  /// ParoptModuleTransform object constructor
  VPOParoptModuleTransform(Module &M, int Mode, unsigned OptLevel = 2,
                           bool SwitchToOffload = false)
    : M(M), Mode(Mode), OptLevel(OptLevel), SwitchToOffload(SwitchToOffload)
  {}

  /// Perform paropt transformation on a module.
  bool doParoptTransforms(
      std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter);

private:
  /// Module which is being transformed.
  Module &M;

  /// Paropt compilation mode.
  int Mode;

  /// Optimization level.
  unsigned OptLevel;

  /// Offload compilation mode.
  bool SwitchToOffload;

private:
  /// Creates the global llvm.global_ctors initialized
  /// with the function .omp_offloading.descriptor_reg
  void genCtorList();

  /// Remove routines and global variables which has no target declare
  /// attribute.
  void removeTargetUndeclaredGlobals();

  /// Transform the use of the tid global into __kmpc_global_thread_num
  /// or the the use of the first argument of the OMP outlined function. The use
  /// of bid global is transformed accordingly.
  void fixTidAndBidGlobals();

  /// The utility to transform the tid/bid global variable.
  void processUsesOfGlobals(Constant *PtrHolder,
                            SmallVectorImpl<Instruction *> &RewriteIns,
                            bool IsTid);

  /// Collect the uses of the given global variable.
  void collectUsesOfGlobals(Constant *PtrHolder,
                            SmallVectorImpl<Instruction *> &RewriteIns);
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_MODULE_TRANSFORMS_H
#endif // INTEL_COLLAB
