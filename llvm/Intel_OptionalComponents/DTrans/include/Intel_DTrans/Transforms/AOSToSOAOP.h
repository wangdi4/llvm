//====--- AOSToSOAOP.h - AOS-to-SOA with support for opaque pointers ---====//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
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

#if !INTEL_FEATURE_SW_DTRANS
#error AOSToSOAOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_AOSTOSOAOP_H
#define INTEL_DTRANS_TRANSFORMS_AOSTOSOAOP_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class BitCastInst;
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

  bool runImpl(Module &M, DTransSafetyInfo *DTInfo, WholeProgramInfo &WPInfo,
               GetTLIFuncType &GetTLI, DominatorTreeFuncType &GetDT);

private:
  void gatherCandidateTypes(DTransSafetyInfo &DTInfo,
                            StructInfoVecImpl &CandidateTypes);

  void qualifyCandidates(StructInfoVecImpl &CandidateTypes, Module &M,
                         DTransSafetyInfo &DTInfo, WholeProgramInfo &WPInfo,
                         DominatorTreeFuncType &GetDT);

  bool qualifyCandidatesTypes(StructInfoVecImpl &CandidateTypes,
                              DTransSafetyInfo &DTInfo);

  bool qualifyCalls(Module &M, WholeProgramInfo &WPInfo,
                    StructInfoVecImpl &CandidateTypes, DTransSafetyInfo &DTInfo,
                    DominatorTreeFuncType &GetDT);

  bool qualifyInstructions(Module &M, StructInfoVecImpl &CandidateTypes,
                           DTransSafetyInfo &DTInfo);

  bool checkAllocationUsers(Instruction *AllocCall, llvm::Type *StructTy,
                            Value **Unsupported);

  bool collectCallChain(
      WholeProgramInfo &WPInfo, Instruction *I,
      SmallVectorImpl<std::pair<Function *, Instruction *>> &CallChain);

  // BitCast instructions that are safe because they are simply used for the
  // allocation/free calls on the type being transformed. These instructions
  // will be removed during the transformation.
  SmallPtrSet<BitCastInst*, 2> SafeBitCasts;
};

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_AOSTOSOAOP_H
