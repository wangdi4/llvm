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
//===--- VPOParoptPrepare.h --- Paropt Prepare Class Support -*- C++ --*---===//
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
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
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

  static bool isRequired() { return true; }

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
