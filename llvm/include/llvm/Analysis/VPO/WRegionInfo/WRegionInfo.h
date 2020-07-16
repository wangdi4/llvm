#if INTEL_COLLAB // -*- C++ -*-
//===--------- WRegionInfo.h - Build WRegionInfo Graph --------*-- C++ --*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// The analysis used to create WRegion Node Graph of identified WRegions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONINFO_H
#define LLVM_ANALYSIS_VPO_WREGIONINFO_H

#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/TargetTransformInfo.h"

namespace llvm {

class AAResults;
class Function;
class DominatorTree;
class PostDominatorTree;

namespace vpo {

class WRegion;
class WRegionCollection;

/// \brief Implementation of WRegionInfo analysis pass. This analysis creates
/// W-Region Graph, and fills up WRegion nodes with the information in the
/// directive intrinsics. The overall sequence of building the W-Region Graph is
/// as follows:
///
/// 1) WRegionCollection - collect work regions and build W-Region Graph.
/// 2) WRegionInfo - fill Up W-Region Info by parsing directive intrinsics
///
class WRegionInfo{
public:
  /// Iterators to iterate over regions
  typedef WRContainerTy::iterator iterator;
  typedef WRContainerTy::const_iterator const_iterator;
  typedef WRContainerTy::reverse_iterator reverse_iterator;
  typedef WRContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  Function *Func;
  DominatorTree *DT;
  LoopInfo *LI;
  ScalarEvolution *SE;
  const TargetTransformInfo *TTI;
  AssumptionCache *AC;
  const TargetLibraryInfo *TLI;
  AAResults *AA;
  WRegionCollection *WRC;
  OptimizationRemarkEmitter &ORE;

  /// \brief Populates W-Region with WRegionNodes.
  void populateWRegion(WRegion *W, BasicBlock *EntryBB, BasicBlock **ExitBB);

public:
  WRegionInfo(Function *F, DominatorTree *DT, LoopInfo *LI, ScalarEvolution *SE,
              const TargetTransformInfo *TTI, AssumptionCache *AC,
              const TargetLibraryInfo *TLI, AAResults *AA,
              WRegionCollection *WRC, OptimizationRemarkEmitter &ORE);

  void print(raw_ostream &OS) const;

  /// \brief Entry point for on-demand call to gather WRegion info from the IR
#if INTEL_CUSTOMIZATION
  /// If IR==HIR, it walks the HIR; else, it walks the LLVM IR
  void
  buildWRGraph(IRKind IR = LLVMIR);
#else
  void buildWRGraph();
#endif // INTEL_CUSTOMIZATION

  /// WRN Graph
  WRContainerImpl *getWRGraph() const { return WRC->getWRGraph(); }

  /// \brief Getter methods for analyses done
  DominatorTree *getDomTree() { return DT; }
  LoopInfo *getLoopInfo()     { return LI; }
  ScalarEvolution *getSE()    { return SE; }
  const TargetTransformInfo *getTargetTransformInfo() { return TTI; }
  AssumptionCache *getAssumptionCache() { return AC; }
  const TargetLibraryInfo *getTargetLibraryInfo() { return TLI; }
  AAResults *getAliasAnalysis() { return AA; }
  OptimizationRemarkEmitter &getORE() { return ORE; }

  /// WRN Graph iterator methods
  iterator begin() { return getWRGraph()->begin(); }
  const_iterator begin() const { return getWRGraph()->begin(); }
  iterator end() { return getWRGraph()->end(); }
  const_iterator end() const { return getWRGraph()->end(); }

  reverse_iterator rbegin() { return getWRGraph()->rbegin(); }
  const_reverse_iterator rbegin() const { return getWRGraph()->rbegin(); }
  reverse_iterator rend() { return getWRGraph()->rend(); }
  const_reverse_iterator rend() const { return getWRGraph()->rend(); }

  /// \brief Returns the number of top-level WRegionNodes in the WRGraph
  unsigned getWGraphSize() const { return getWRGraph()->size(); }

  /// \brief Returns true if the WRGraph is empty.
  bool WRGraphIsEmpty() const { return getWRGraph()->empty(); }
};

/// \brief WRegionInfo Pass for the legacy Pass Manager.
class WRegionInfoWrapperPass : public FunctionPass {
  std::unique_ptr<WRegionInfo> WRI;

public:
  static char ID;
  WRegionInfoWrapperPass();

  WRegionInfo &getWRegionInfo() { return *WRI; }
  const WRegionInfo &getWRegionInfo() const { return *WRI; }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override {
    WRI->print(OS);
  }
};

} // End namespace vpo

/// \brief WRegionInfo Pass for the new Pass Manager.
class WRegionInfoAnalysis : public AnalysisInfoMixin<WRegionInfoAnalysis> {
  friend struct AnalysisInfoMixin<WRegionInfoAnalysis>;

  static AnalysisKey Key;

public:
  using Result = vpo::WRegionInfo;

  vpo::WRegionInfo run(Function &F, FunctionAnalysisManager &AM);
};

} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_WREGIONINFO_H
#endif // INTEL_COLLAB
