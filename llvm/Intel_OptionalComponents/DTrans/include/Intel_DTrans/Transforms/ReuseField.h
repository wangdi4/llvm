//===--------------- ReuseField.h - DTransReuseFieldPass  ---------------===//
//
// Copyright (C) 2022-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans reuse field optimization pass.
//
// The main goal of this transformation is try to reuse one structure field
// in instructions where multiple fields are set to the same value together.
// However, this pass doesn't delete the redundant fields in the structure,
// but remaps the redundant fields of load's GEP to one of them. Other work
// will be done by the delete field pass when it is run after this pass in the
// normal execution pipeline.
//
// Use of "org_cost" is replaced by use of "cost":
// Before:
// struct arc_t { int flow, int cost, int org_cost };
//
// arc[i].cost = arc[i].org_cost = 5;
// ...
// lb = arc[i].org_cost;
// ...
// arc[i].cost = arc[i].org_cost = (net->bigM+15);
// ...
// la = arc[i].cost;
//
// After:
// struct arc_t { int flow, int cost, int org_cost };
//
// arc[i].cost = arc[i].org_cost = 5;
// ...
// lb = arc[i].cost;
// ...
// arc[i].cost = arc[i].org_cost = (net->bigM+15);
// ...
// la = arc[i].cost;
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error ReuseField.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_REUSEFIELD_H
#define INTEL_DTRANS_TRANSFORMS_REUSEFIELD_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans optimizations.
class ReuseFieldPass : public PassInfoMixin<dtrans::ReuseFieldPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool
  runImpl(Module &M, DTransAnalysisInfo &Info,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_REUSEFIELD_H
