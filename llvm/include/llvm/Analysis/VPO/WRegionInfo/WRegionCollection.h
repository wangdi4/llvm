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
//===-- WRegionCollection.h ------------------------------------*- C++ --*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This analysis is used to identify WRegions of LLVM IR on which OpenMP,
/// Cilk, Offload, Parallel and Vector transformations can be applied.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H
#define LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"
#include <set>

namespace llvm {

class Function;
class Loop;
class LoopInfo;
class DominatorTree;
class ScalarEvolution;

#if INTEL_CUSTOMIZATION
namespace loopopt {
class HIRFramework;
}
#endif // INTEL_CUSTOMIZATION

namespace vpo {

/// template class for WRStack
template <class T> class WRStack {
public:
  WRStack() {}
  void push(T x);
  void pop();
  T top();
  size_t size();
  bool empty();
  T operator[](size_t i);

private:
  std::vector<T> Stack_;
};

template <class T> void WRStack<T>::push(T X) {
  Stack_.push_back(X);
  return;
}

template <class T> void WRStack<T>::pop() {
  if (Stack_.size() > 0)
    Stack_.erase(Stack_.end() - 1);

  return;
}

template <class T> T WRStack<T>::top() {
  assert(Stack_.size() > 0);
  return Stack_.at(Stack_.size() - 1);
}

template <class T> T WRStack<T>::operator[](size_t i) {
  assert(Stack_.size() > 0 && i < Stack_.size());
  return Stack_.at(i);
}

template <class T> size_t WRStack<T>::size() { return Stack_.size(); }

template <class T> bool WRStack<T>::empty() {
  return Stack_.size() == 0 ? true : false;
}

/// Implementation of WRegionCollection analysis pass. This is the first
/// step in building W-Region Graph. We start by collecting regions as a set of
/// basic blocks in the incoming LLVM IR. This information is then used by
/// WRegionInfo pass to create and populate W-Region nodes.
class WRegionCollection {

private:
  /// Vector of WRegionNodes.
  WRContainerImpl *WRGraph = nullptr;

  Function *Func = nullptr;
  DominatorTree *DT = nullptr;
  LoopInfo *LI = nullptr;
  ScalarEvolution *SE = nullptr;
  AAResults *AA = nullptr;
#if INTEL_CUSTOMIZATION
  loopopt::HIRFramework *HIRF = nullptr;
#endif // INTEL_CUSTOMIZATION

  /// Delete WRGraph with all its WRegionNodes.
  void releaseMemory() {
    if (!WRGraph)
      return;

    for (auto *WRN : *WRGraph)
      delete WRN;

    delete (WRContainerTy *)WRGraph;
    WRGraph = nullptr;
  }

public:
  friend class WRegionNode;

  WRegionCollection(Function *F, DominatorTree *DT, LoopInfo *LI,
#if INTEL_CUSTOMIZATION
                    ScalarEvolution *SE, AAResults *AA,
                    loopopt::HIRFramework *HIRF);
#else
                    ScalarEvolution *SE, AAResults *AA);
#endif // INTEL_CUSTOMIZATION

  ~WRegionCollection() { releaseMemory(); }

  void print(raw_ostream &OS) const;

  /// Entry point for on-demand call to build the WRGraph.
#if INTEL_CUSTOMIZATION
  /// If IR==HIR, it walks the HIR; else, it walks the LLVM IR
  void buildWRGraph(IRKind IR = LLVMIR);
#else
  void buildWRGraph();
#endif // INTEL_CUSTOMIZATION

  /// Builds the WRGraph by walking the CFG to extract WRegionNodes
  void buildWRGraphImpl(Function &F);

  /// Returns true if ParOpt/VecOpt is able to handle this loop.
  bool isCandidateLoop(Loop &Lp);

  /// Getter methods
  WRContainerImpl *getWRGraph() { return WRGraph; }
  DominatorTree *getDomTree() { return DT; }
  LoopInfo *getLoopInfo()     { return LI; }
  ScalarEvolution *getSE()    { return SE; }
  AAResults *getAliasAnalysis() { return AA; }

  /// Returns the size of the WRGraph container
  unsigned getWRGraphSize() { return WRGraph->size(); }

  // Iterators to traverse WRGraph
  typedef WRContainerTy::iterator iterator;
  typedef WRContainerTy::const_iterator const_iterator;
  typedef WRContainerTy::reverse_iterator reverse_iterator;
  typedef WRContainerTy::const_reverse_iterator const_reverse_iterator;

  iterator begin() { return WRGraph->begin(); }
  const_iterator begin() const { return WRGraph->begin(); }
  iterator end() { return WRGraph->end(); }
  const_iterator end() const { return WRGraph->end(); }

  reverse_iterator rbegin() { return WRGraph->rbegin(); }
  const_reverse_iterator rbegin() const { return WRGraph->rbegin(); }
  reverse_iterator rend() { return WRGraph->rend(); }
  const_reverse_iterator rend() const { return WRGraph->rend(); }

  /// Return true, if WRegionCollectionAnalysis must be invalidated.
  bool invalidate(Function &F, const PreservedAnalyses &PA,
                  FunctionAnalysisManager::Invalidator &Inv);
};

/// WRegionCollection Pass for the legacy Pass Manager.
class WRegionCollectionWrapperPass: public FunctionPass {
  std::unique_ptr<WRegionCollection> WRC;

public:
  static char ID;
  WRegionCollectionWrapperPass();

  WRegionCollection &getWRegionCollection() {return *WRC; }
  const WRegionCollection &getWRegionCollection() const {return *WRC; }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override {
    WRC->print(OS);
  }
};

} // End namespace vpo

/// WRegionCollection Pass for the new Pass Manager.
class WRegionCollectionAnalysis
    : public AnalysisInfoMixin<WRegionCollectionAnalysis> {
  friend struct AnalysisInfoMixin<WRegionCollectionAnalysis>;

  static AnalysisKey Key;

public:
  using Result = vpo::WRegionCollection;

  vpo::WRegionCollection run(Function &F, FunctionAnalysisManager &AM);
};

class WRegionCollectionAnalysisPrinterPass
    : public PassInfoMixin<WRegionCollectionAnalysisPrinterPass> {
  raw_ostream &OS;

public:
  explicit WRegionCollectionAnalysisPrinterPass(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<WRegionCollectionAnalysis>(F).print(OS);
    return PreservedAnalyses::all();
  }
  static bool isRequired() { return true; }
};

} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H
#endif // INTEL_COLLAB
