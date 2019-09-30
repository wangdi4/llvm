//===--------------- AOSToSOA.h - DTransAOSToSOAPass  ---------------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error AOSToSOA.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_AOSTOSOA_H
#define INTEL_DTRANS_TRANSFORMS_AOSTOSOA_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class DominatorTree;

namespace dtrans {

/// Pass to perform DTrans AOS to SOA optimizations.
class AOSToSOAPass : public PassInfoMixin<dtrans::AOSToSOAPass> {
public:
  using StructInfoVec = SmallVector<dtrans::StructInfo *, 16>;
  using StructInfoVecImpl = SmallVectorImpl<dtrans::StructInfo *>;
  using DominatorTreeFuncType = std::function<DominatorTree &(Function &)>;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransAnalysisInfo &DTInfo,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo, DominatorTreeFuncType &GetDT);

private:
  // Populate the \p CandidateTypes vector with all the structure types
  // that should be considered for transformation.
  void gatherCandidateTypes(DTransAnalysisInfo &DTInfo,
                            StructInfoVecImpl &CandidateTypes);

  // This function filters the \p CandidateTypes list
  // to remove types that are not supported for transformation.
  void qualifyCandidates(StructInfoVecImpl &CandidateTypes, Module &M,
                         DTransAnalysisInfo &DTInfo,
                         DominatorTreeFuncType &GetDT);

  // Filter the \p CandidateTypes list based on data types and usage of the
  // structure.
  bool qualifyCandidatesTypes(StructInfoVecImpl &CandidateTypes,
                              DTransAnalysisInfo &DTInfo);

  // Filter the \p CandidateTypes list based on whether the dynamic memory
  // allocation of the type is supported.
  bool qualifyAllocations(StructInfoVecImpl &CandidateTypes,
                          DTransAnalysisInfo &DTInfo,
                          DominatorTreeFuncType &GetDT);

  // Filter the \p CandidateTypes list based on whether the type meets
  // the criteria of the profitability heuristics
  bool qualifyHeuristics(StructInfoVecImpl &CandidateTypes, Module &M,
                         DTransAnalysisInfo &DTInfo);

  // Helper routine to verify whether all the users of an allocation
  // call \p AllocCall are supported for the transformation of \p StructTy. If
  // an unsupported use is found, return false, and save the unsupported use in
  // \p Unsupported.
  bool supportedAllocationUsers(Instruction *AllocCall, llvm::Type *StructTy,
                                Value **Unsupported);

  // Helper routine used when analyzing the call graph to reach the memory
  // allocation.
  bool collectCallChain(
      Instruction *I,
      SmallVectorImpl<std::pair<Function *, Instruction *>> &CallChain);
};

} // namespace dtrans

ModulePass *createDTransAOSToSOAWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_AOSTOSOA_H
