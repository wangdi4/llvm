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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

namespace llvm {

class ModulePass;

namespace vpo {

enum VPOParoptMode {
  PAROPTOFF = 0x00000000,
  OMPVEC = 0x00000001,
  OMPPAR = 0x00000002,
  OMPOFFLOAD = 0x00000004,
  AUTOVEC = 0x00000008,
  AUTOPAR = 0x00000010
};

/// \brief VPOParopt class for performing parallelization and offloading
class VPOParopt : public ModulePass {

public:
  /// Pass Identification
  static char ID;

  VPOParopt();
  ~VPOParopt(){};

  const char *getPassName() const override { return "VPO Paropt Pass"; }

  bool runOnModule(Module &M) override;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;

  // void print(raw_ostream &OS, const Module * = nullptr) const override;

private:
  WRegionInfo *WI;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_H
