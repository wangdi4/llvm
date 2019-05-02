#if INTEL_COLLAB // -*- C++ -*-
//===--- VPOParoptPrepare.h --- Paropt Prepare Class Support -*- C++ --*---===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
// July 2016: Initial Implementation of Paropt Prepare Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the Paropt Prepare for Parallelization and OpenMP.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_PREPARE_H
#define LLVM_TRANSFORMS_VPO_PAROPT_PREPARE_H

#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif  // INTEL_CUSTOMIZATION

namespace llvm {

class FunctionPass;

/// \brief VPOParopt Prepare class for performing parallelization and offloading
class VPOParoptPreparePass : public PassInfoMixin<VPOParoptPreparePass> {

public:

  /// \brief ParoptPrepare object constructor
  /// \brief 0x5 is equivalent to ParPrepare | OmpPar
  explicit VPOParoptPreparePass(unsigned MyMode = 0x5u);
  ~VPOParoptPreparePass() {};

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  bool runImpl(Function &F, vpo::WRegionInfo &WI,
               OptimizationRemarkEmitter &ORE);

private:

  // VPO-mode
  unsigned Mode;

#if INTEL_CUSTOMIZATION
  // Verbosity level for generating remarks using Loop Opt Report
  // framework (under -qopt-report).
  OptReportVerbosity::Level ORVerbosity;
#endif  // INTEL_CUSTOMIZATION
};

namespace vpo {

/// \brief VPOParopt Prepare class for performing parallelization and offloading
class VPOParoptPrepare : public FunctionPass {

public:
  /// Pass Identification
  static char ID;

  /// \brief ParoptPrepare object constructor
  /// \brief 0x5 is equivalent to ParPrepare | OmpPar
  explicit VPOParoptPrepare(unsigned MyMode = 0x5u);
  ~VPOParoptPrepare() {};

  StringRef getPassName() const override { return "VPO Paropt Prepare"; }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  //void print(raw_ostream &OS, const Module * = nullptr) const override;

private:
  VPOParoptPreparePass Impl;

#if INTEL_CUSTOMIZATION
  // Verbosity level for generating remarks using Loop Opt Report
  // framework (under -qopt-report).
  OptReportVerbosity::Level ORVerbosity;
#endif  // INTEL_CUSTOMIZATION
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_PREPARE_H
#endif // INTEL_COLLAB
