//===- CSALowerParallelIntrinsics.h - Lower section intrinsics into metadata
//-*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file provides declaration of CSALowerParallelIntrinsics.
///
///===---------------------------------------------------------------------===//

#ifndef INTEL_CSA_TRANSFORMS_SCALAR_CSA_LOWER_PARALLEL_INTRINSICS_H
#define INTEL_CSA_TRANSFORMS_SCALAR_CSA_LOWER_PARALLEL_INTRINSICS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
  /// \brief CSALoopTag::Parallel - CSA parallel loop metadata definition.
  ///
  /// "llvm.loop.csa_parallel" is metadata used to mark CSA parallel loops,
  //  so that later phases (e.g. CSAMemopOrdering, CSAInnerLoopPrep)
  /// may use it to detect parallel loops.
  struct CSALoopTag {
    static constexpr const char *Parallel = "llvm.loop.csa_parallel";
  };

  /// \brief CSALowerParallelIntrinsics - new pass manager pass that lowers
  /// CSA specific intrinsics into metadata.
  ///
  /// The pass lookes foer llvm.csa.parallel_section_entry/exit calls
  /// and identifies groups of independent memory accesses located
  /// in different sections.  The pass sets up alias.scope/noalias
  /// metadata for these memory accesses and removes the intrinsic
  /// calls (including the corresponding llvm.csa.parallel_region_entry/exit
  /// and llvm.csa.spmdization_entry/exit calls).
  class CSALowerParallelIntrinsics :
    public PassInfoMixin<CSALowerParallelIntrinsics> {
    friend PassInfoMixin<CSALowerParallelIntrinsics>;

  public:
    typedef PreservedAnalyses Result;

    Result run(Function &, FunctionAnalysisManager &);
  };

} // namespace llvm

#endif  // INTEL_CSA_TRANSFORMS_SCALAR_CSA_LOWER_PARALLEL_INTRINSICS_H
