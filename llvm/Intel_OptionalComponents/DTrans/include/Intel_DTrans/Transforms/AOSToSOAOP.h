//====--- AOSToSOAOP.h - AOS-to-SOA with support for opaque pointers ---====//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file defines the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass with support for IR using either opaque or
// non-opaque pointers.
//===---------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error AOSToSOAOP.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_AOSTOSOAOP_H
#define INTEL_DTRANS_TRANSFORMS_AOSTOSOAOP_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class DominatorTree;
class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtrans {
class StructInfo;
} // namespace dtrans

namespace dtransOP {
class DTransSafetyInfo;

/// Pass to perform DTrans AOS to SOA optimizations.
class AOSToSOAOPPass : public PassInfoMixin<AOSToSOAOPPass> {
public:
  using DominatorTreeFuncType = std::function<DominatorTree &(Function &)>;
  using GetTLIFuncType =
      std::function<const TargetLibraryInfo &(const Function &)>;

  using StructInfoVec = SmallVector<dtrans::StructInfo *, 8>;
  using StructInfoVecImpl = SmallVectorImpl<dtrans::StructInfo *>;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransSafetyInfo *DTInfo, WholeProgramInfo &WPInfo,
               GetTLIFuncType &GetTLI, DominatorTreeFuncType &GetDT);

private:
  void gatherCandidateTypes(DTransSafetyInfo *DTInfo,
                            StructInfoVecImpl &CandidateTypes);

  void qualifyCandidates(StructInfoVecImpl &CandidateTypes, Module &M,
                         DTransSafetyInfo *DTInfo,
                         DominatorTreeFuncType &GetDT);
};

} // namespace dtransOP

ModulePass *createDTransAOSToSOAOPWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_AOSTOSOAOP_H
