#if INTEL_COLLAB // -*- C++ -*-
//===-- VPOParopt.h - Paropt Class for AutoPar / OpenMP Support -*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Nov 2015: Initial Implementation of Paropt Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the Paropt Driver for Parallelization and OpenMP.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_H
#define LLVM_TRANSFORMS_VPO_PAROPT_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#include <functional>

namespace llvm {

class ModulePass;

namespace vpo {

//
// If OmpTbb==false, emit the regular OMP task runtime calls:
//
//   __kmpc_omp_task_alloc
//   __kmpc_taskloop
//   __kmpc_task_reduction_init
//   __kmpc_task_reduction_get_th_data
//
// If OmmTbb==true, emit calls to their TBB implementations:
//
//   __tbb_omp_task_alloc
//   __tbb_omp_taskloop
//   __tbb_omp_task_reduction_init
//   __tbb_omp_task_reduction_get_th_data

enum VPOParoptMode {
  ParoptOff  = 0x00000000,
  ParPrepare = 0x00000001,
  ParTrans   = 0x00000002,
  OmpPar     = 0x00000004,
  OmpVec     = 0x00000008,
  OmpTpv     = 0x00000010, // thread-private legacy mode
  OmpOffload = 0x00000020,
  AutoVec    = 0x00000040,
  AutoPar    = 0x00000080,
  OmpTbb     = 0x00000100  // emit tbb_omp_task_* calls (instead of kmpc_task_*)
};

} // end namespace vpo

/// \brief VPOParopt Pass for the new Pass Manager. Performs parallelization and
/// offloading transformations.
class VPOParoptPass : public PassInfoMixin<VPOParoptPass> {

public:
  explicit VPOParoptPass(unsigned MyMode = vpo::ParTrans | vpo::OmpPar |
                                           vpo::OmpVec,
                         unsigned OptLevel = 2);
  ~VPOParoptPass(){};

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  bool
  runImpl(Module &M,
          std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter);

private:
  /// \brief Creates the global llvm.global_ctors initialized
  /// with the function .omp_offloading.descriptor_reg
  void genCtorList(Module &M);

  /// \brief Remove routines and global variables which has no target declare
  /// attribute.
  void removeTargetUndeclaredGlobals(Module &M);

  /// \brief Transform the use of the tid global into __kmpc_global_thread_num
  /// or the the use of the first argument of the OMP outlined function. The use
  /// of bid global is transformed accordingly.
  void fixTidAndBidGlobals(Module &M);

  /// \brief The utility to transform the tid/bid global variable.
  void processUsesOfGlobals(Constant *PtrHolder,
                            SmallVectorImpl<Instruction *> &RewriteIns,
                            bool IsTid);

  // \brief Collect the uses of the given global variable.
  void collectUsesOfGlobals(Constant *PtrHolder,
                            SmallVectorImpl<Instruction *> &RewriteIns);

  // Paropt mode.
  unsigned Mode;

  // Optimization level.
  unsigned OptLevel;
};

namespace vpo {
/// \brief VPOParopt Pass wrapper for the old Pass Manager. Performs
/// parallelization and offloading transformation.
class VPOParopt : public ModulePass {

public:
  /// Pass Identification
  static char ID;

  explicit VPOParopt(unsigned MyMode = ParTrans | OmpPar | OmpVec,
                     unsigned OptLevel = 2);
  ~VPOParopt(){};

  StringRef getPassName() const override { return "VPO Paropt Pass"; }

  bool runOnModule(Module &M) override;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  // void print(raw_ostream &OS, const Module * = nullptr) const override;

private:
  VPOParoptPass Impl;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_H
#endif // INTEL_COLLAB
