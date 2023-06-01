//===- BasicAliasAnalysis.h - Stateless, local Alias Analysis ---*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
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
/// \file
/// This is the interface for LLVM's primary stateless and local alias analysis.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_BASICALIASANALYSIS_H
#define LLVM_ANALYSIS_BASICALIASANALYSIS_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h" // INTEL
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h" // INTEL
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace llvm {

class AssumptionCache;
class DataLayout;
class DominatorTree;
class Function;
class AddressOperator; // INTEL
class GEPOperator;
class PHINode;
class SelectInst;
class SubscriptInst; // INTEL
class TargetLibraryInfo;
class PhiValues;
class Value;

/// This is the AA result object for the basic, local, and stateless alias
/// analysis. It implements the AA query interface in an entirely stateless
/// manner. As one consequence, it is never invalidated due to IR changes.
/// While it does retain some storage, that is used as an optimization and not
/// to preserve information from query to query. However it does retain handles
/// to various other analyses and must be recomputed when those analyses are.
class BasicAAResult : public AAResultBase {
  const DataLayout &DL;
  const Function &F;
  const TargetLibraryInfo &TLI;
  AssumptionCache &AC;
  DominatorTree *DT;
  PhiValues *PV; // INTEL

#if INTEL_CUSTOMIZATION
  /// The maximum number of uses to explore for PointerMayBeCaptured() calls.
  unsigned PtrCaptureMaxUses;

  /// Command line option to define the maximum number of pointer uses to
  /// explore when checking for capture. This value overrides the default if
  /// optimization level is >= 3 or if this option was explicitly specified
  /// in command line.
  static cl::opt<unsigned> OptPtrMaxUsesToExplore;

public:
  /// Do opt-level based variable initialization for a BasicAAResult instance.
  void setupWithOptLevel(unsigned OptLevel) {
    DEBUG_WITH_TYPE("basicaa-opt-level",
                    dbgs() << "BasicAAResult: using OptLevel = " << OptLevel
                           << "\n");
    PtrCaptureMaxUses =
        OptLevel < 3 && OptPtrMaxUsesToExplore.getNumOccurrences() == 0
            ? getDefaultMaxUsesToExploreForCaptureTracking()
            : OptPtrMaxUsesToExplore;
  }

private:
#endif // INTEL_CUSTOMIZATION

public:
  BasicAAResult(const DataLayout &DL, const Function &F,
                const TargetLibraryInfo &TLI, AssumptionCache &AC,
#if INTEL_CUSTOMIZATION
                DominatorTree *DT = nullptr, unsigned OptLevel = 2u,
                PhiValues *PV = nullptr)
      : DL(DL), F(F), TLI(TLI), AC(AC), DT(DT), PV(PV) {
    setupWithOptLevel(OptLevel);
  }
#endif // INTEL_CUSTOMIZATION

  BasicAAResult(const BasicAAResult &Arg)
      : AAResultBase(Arg), DL(Arg.DL), F(Arg.F), TLI(Arg.TLI), AC(Arg.AC),
#if INTEL_CUSTOMIZATION
        DT(Arg.DT), PV(Arg.PV), PtrCaptureMaxUses(Arg.PtrCaptureMaxUses) {
  }
#endif // INTEL_CUSTOMIZATION
  BasicAAResult(BasicAAResult &&Arg)
      : AAResultBase(std::move(Arg)), DL(Arg.DL), F(Arg.F), TLI(Arg.TLI),
#if INTEL_CUSTOMIZATION
        AC(Arg.AC), DT(Arg.DT), PV(Arg.PV),
        PtrCaptureMaxUses(Arg.PtrCaptureMaxUses) {
  }
#endif // INTEL_CUSTOMIZATION

  /// Handle invalidation events in the new pass manager.
  bool invalidate(Function &Fn, const PreservedAnalyses &PA,
                  FunctionAnalysisManager::Invalidator &Inv);

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI, const Instruction *CtxI);

#if INTEL_COLLAB
  AliasResult loopCarriedAlias(const MemoryLocation &LocA,
                               const MemoryLocation &LocB, AAQueryInfo &AAQI);
#endif // INTEL_COLLAB

  ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc,
                           AAQueryInfo &AAQI);

  ModRefInfo getModRefInfo(const CallBase *Call1, const CallBase *Call2,
                           AAQueryInfo &AAQI);

  /// Returns a bitmask that should be unconditionally applied to the ModRef
  /// info of a memory location. This allows us to eliminate Mod and/or Ref
  /// from the ModRef info based on the knowledge that the memory location
  /// points to constant and/or locally-invariant memory.
  ///
  /// If IgnoreLocals is true, then this method returns NoModRef for memory
  /// that points to a local alloca.
  ModRefInfo getModRefInfoMask(const MemoryLocation &Loc, AAQueryInfo &AAQI,
                               bool IgnoreLocals = false);

  /// Get the location associated with a pointer argument of a callsite.
  ModRefInfo getArgModRefInfo(const CallBase *Call, unsigned ArgIdx);

  /// Returns the behavior when calling the given call site.
  MemoryEffects getMemoryEffects(const CallBase *Call, AAQueryInfo &AAQI);

  /// Returns the behavior when calling the given function. For use when the
  /// call site is not known.
  MemoryEffects getMemoryEffects(const Function *Fn);

private:
  struct DecomposedGEP;

  /// Tracks instructions visited by pointsToConstantMemory.
  SmallPtrSet<const Value *, 16> Visited;

#if INTEL_CUSTOMIZATION
  /// Corresponds to processing of single GEP in DecomposeGEPExpression.
  static void DecomposeSubscript(const SubscriptInst *V,
                                 DecomposedGEP &Decomposed,
                                 const DataLayout &DL, AssumptionCache *AC,
                                 DominatorTree *DT);
#endif // INTEL_CUSTOMIZATION

  static DecomposedGEP
  DecomposeGEPExpression(const Value *V, const DataLayout &DL,
                         AssumptionCache *AC, DominatorTree *DT);

  /// A Heuristic for aliasGEP that searches for a constant offset
  /// between the variables.
  ///
  /// GetLinearExpression has some limitations, as generally zext(%x + 1)
  /// != zext(%x) + zext(1) if the arithmetic overflows. GetLinearExpression
  /// will therefore conservatively refuse to decompose these expressions.
  /// However, we know that, for all %x, zext(%x) != zext(%x + 1), even if
  /// the addition overflows.
  bool constantOffsetHeuristic(const DecomposedGEP &GEP, LocationSize V1Size,
                               LocationSize V2Size, AssumptionCache *AC,
                               DominatorTree *DT, const AAQueryInfo &AAQI);

  bool isValueEqualInPotentialCycles(const Value *V1, const Value *V2,
                                     const AAQueryInfo &AAQI);

  void subtractDecomposedGEPs(DecomposedGEP &DestGEP,
                              const DecomposedGEP &SrcGEP,
                              const AAQueryInfo &AAQI);

  AliasResult aliasGEP(const AddressOperator *V1,   // INTEL
                       LocationSize V1Size,         // INTEL
                       const Value *V2, LocationSize V2Size,
                       const Value *UnderlyingV1, const Value *UnderlyingV2,
                       AAQueryInfo &AAQI);

  AliasResult aliasPHI(const PHINode *PN, LocationSize PNSize,
                       const Value *V2, LocationSize V2Size, AAQueryInfo &AAQI);

  AliasResult aliasSelect(const SelectInst *SI, LocationSize SISize,
                          const Value *V2, LocationSize V2Size,
                          AAQueryInfo &AAQI);

#if INTEL_CUSTOMIZATION
  // Check if the input value O1 is captured by the input value O2
  bool valueIsNotCapturedBeforeOrAt(const Value *O1, const Value *O2);
#endif // INTEL_CUSTOMIZATION

  AliasResult aliasCheck(const Value *V1, LocationSize V1Size, const Value *V2,
                         LocationSize V2Size, AAQueryInfo &AAQI,
                         const Instruction *CtxI);

  const Value* getBaseValue(const Value *V1); // INTEL

  AliasResult aliasCheckRecursive(const Value *V1, LocationSize V1Size,
                                  const Value *V2, LocationSize V2Size,
                                  AAQueryInfo &AAQI, const Value *O1,
                                  const Value *O2);
};

/// Analysis pass providing a never-invalidated alias analysis result.
class BasicAA : public AnalysisInfoMixin<BasicAA> {
  friend AnalysisInfoMixin<BasicAA>;

  static AnalysisKey Key;

public:
  using Result = BasicAAResult;

  BasicAAResult run(Function &F, FunctionAnalysisManager &AM);
};

/// Legacy wrapper pass to provide the BasicAAResult object.
class BasicAAWrapperPass : public FunctionPass {
  std::unique_ptr<BasicAAResult> Result;

  virtual void anchor();

public:
  static char ID;

  BasicAAWrapperPass();

  BasicAAResult &getResult() { return *Result; }
  const BasicAAResult &getResult() const { return *Result; }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

FunctionPass *createBasicAAWrapperPass();

} // end namespace llvm

#endif // LLVM_ANALYSIS_BASICALIASANALYSIS_H
