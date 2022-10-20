//===- TypeBasedAliasAnalysis.h - Type-Based Alias Analysis -----*- C++ -*-===//
// INTEL_CUSTOMIZATION
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
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This is the interface for a metadata-based TBAA. See the source file for
/// details on the algorithm.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_TYPEBASEDALIASANALYSIS_H
#define LLVM_ANALYSIS_TYPEBASEDALIASANALYSIS_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include <memory>

namespace llvm {

class CallBase;
class Function;
class MDNode;
class MemoryLocation;

/// A simple AA result that uses TBAA metadata to answer queries.
class TypeBasedAAResult : public AAResultBase {
public:
  /// Handle invalidation events from the new pass manager.
  ///
  /// By definition, this result is stateless and so remains valid.
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &) {
    return false;
  }

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI);
  bool pointsToConstantMemory(const MemoryLocation &Loc, AAQueryInfo &AAQI,
                              bool OrLocal);
  MemoryEffects getModRefBehavior(const CallBase *Call, AAQueryInfo &AAQI);
  MemoryEffects getModRefBehavior(const Function *F);
  ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc,
                           AAQueryInfo &AAQI);
  ModRefInfo getModRefInfo(const CallBase *Call1, const CallBase *Call2,
                           AAQueryInfo &AAQI);
#if INTEL_CUSTOMIZATION
  AliasResult loopCarriedAlias(const MemoryLocation &LocA,
                               const MemoryLocation &LocB, AAQueryInfo &AAQI) {
    return alias(LocA, LocB, AAQI);
  }
#endif // INTEL_CUSTOMIZATION

private:
  bool Aliases(const MDNode *A, const MDNode *B) const;
};

/// Analysis pass providing a never-invalidated alias analysis result.
class TypeBasedAA : public AnalysisInfoMixin<TypeBasedAA> {
  friend AnalysisInfoMixin<TypeBasedAA>;

  static AnalysisKey Key;

public:
  using Result = TypeBasedAAResult;

  TypeBasedAAResult run(Function &F, FunctionAnalysisManager &AM);
};

/// Legacy wrapper pass to provide the TypeBasedAAResult object.
class TypeBasedAAWrapperPass : public ImmutablePass {
  std::unique_ptr<TypeBasedAAResult> Result;

public:
  static char ID;

  TypeBasedAAWrapperPass();

  TypeBasedAAResult &getResult() { return *Result; }
  const TypeBasedAAResult &getResult() const { return *Result; }

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

//===--------------------------------------------------------------------===//
//
// createTypeBasedAAWrapperPass - This pass implements metadata-based
// type-based alias analysis.
//
ImmutablePass *createTypeBasedAAWrapperPass();


#if INTEL_CUSTOMIZATION
/// Given two consequtive GEPs annotated with \p BaseGepNode and \p GepMD
/// !intel-tbaa metadata, calculate the most precise possible annotation for the
/// memory operation using \p GepMD.
MDNode *mergeIntelTBAA(MDNode *BaseGepNode, MDNode *GepMD);

/// Try to refine the tbaa annotation for the load/store with \p MemOpNode tbaa
/// annotation (might be nullptr if there is no annotation) from/to the GEP with
/// \p GepNode !intel-tbaa annotation.
///
/// \Returns the refined metadata or original \p MemOpNode if no refinement was
/// possible.
MDNode *getMostSpecificTBAA(MDNode *GepNode, MDNode *MemOpNode);
#endif // INTEL_CUSTOMIZATION

} // end namespace llvm

#endif // LLVM_ANALYSIS_TYPEBASEDALIASANALYSIS_H
