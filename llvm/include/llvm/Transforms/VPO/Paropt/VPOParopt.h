#if INTEL_COLLAB // -*- C++ -*-
//===------ VPOParopt.h - Paropt Class for OpenMP Support -*- C++ -*- -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

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
  ParoptOff     = 0x00000000,
  ParPrepare    = 0x00000001,
  ParTrans      = 0x00000002,
  OmpPar        = 0x00000004,
  OmpVec        = 0x00000008,
  OmpTpv        = 0x00000010, // thread-private legacy mode
  OmpOffload    = 0x00000020,
  AutoVec       = 0x00000040,
  AutoPar       = 0x00000080,
  OmpTbb        = 0x00000100, // emit tbb_omp_task_* calls (vs kmpc_task_*)
  OmpNoCollapse = 0x00000200, // FE doesn't collapse loops
  OmpSimt       = 0x00000400  // SIMT mode
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
          std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter,
          std::function<TargetLibraryInfo &(Function &F)> TLIGetter);

private:
  // Paropt mode.
  unsigned Mode;

#if INTEL_CUSTOMIZATION
  // Verbosity level for generating remarks using Loop Opt Report
  // framework (under -qopt-report).
  OptReportVerbosity::Level ORVerbosity;
#endif  // INTEL_CUSTOMIZATION

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
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  // void print(raw_ostream &OS, const Module * = nullptr) const override;

private:
  VPOParoptPass Impl;

#if INTEL_CUSTOMIZATION
  // Verbosity level for generating remarks using Loop Opt Report
  // framework (under -qopt-report).
  OptReportVerbosity::Level ORVerbosity;
#endif  // INTEL_CUSTOMIZATION
};

#if INTEL_CUSTOMIZATION
// External storage for -loopopt-use-omp-region.
extern bool UseOmpRegionsInLoopoptFlag;
#endif  // INTEL_CUSTOMIZATION

class ParoptDiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;

public:
  ParoptDiagInfo(const Function &F, const DiagnosticLocation &Loc,
                          const Twine &Msg, DiagnosticSeverity DS = DS_Warning)
    : DiagnosticInfoWithLocationBase(
          static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind()),
          DS, F, Loc),
      Msg(Msg)
  {}

  void print(DiagnosticPrinter &DP) const override {
    if (isLocationAvailable())
      DP << getLocationStr() << ": ";
    DP << Msg;
    if (!isLocationAvailable())
      DP << " (use -g for location info)";
  }
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_H
#endif // INTEL_COLLAB
