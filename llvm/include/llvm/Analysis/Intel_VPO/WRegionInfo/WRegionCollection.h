//===-- WRegionCollection.h ------------------------------------*- C++ --*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This analysis is used to identify W-Regions of LLVM IR on which OpenMP,
/// Cilk, Offload, Parallel and Vector transformations can be applied.
////
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H
#define LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"
#include <set>

namespace llvm {

class Function;
class Loop;
class LoopInfo;
class DominatorTree;
class ScalarEvolution;

namespace loopopt {
class HIRFramework;
}

namespace vpo {

/// \brief template classe for WRStack
template <class T> class WRStack {
public:
  WRStack() {}
  void push(T x);
  void pop();
  T top();
  size_t size();
  bool empty();

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

template <class T> size_t WRStack<T>::size() { return Stack_.size(); }

template <class T> bool WRStack<T>::empty() {
  return Stack_.size() == 0 ? true : false;
}

/// \brief Implementation of WRegionCollection analysis pass. This is the first
/// step in building W-Region Graph. We start by collecting regions as a set of
/// basic blocks in the incoming LLVM IR. This information is then used by
/// WRegionInfo pass to create and populate W-Region nodes.
class WRegionCollection {

private:
  /// WRGraph - vector of WRegionNodes.
  WRContainerImpl *WRGraph;

  Function *Func;
  DominatorTree *DT;
  LoopInfo *LI;
  ScalarEvolution *SE;
  const TargetTransformInfo *TTI;
  AssumptionCache *AC;
  const TargetLibraryInfo *TLI;
  loopopt::HIRFramework *HIRF;

public:
  enum InputIRKind{
    LLVMIR,
    HIR
  };
  friend class WRegionNode;

  WRegionCollection(Function *F, DominatorTree *DT, LoopInfo *LI,
                    ScalarEvolution *SE, const TargetTransformInfo *TTI,
                    AssumptionCache *AC, const TargetLibraryInfo *TLI,
                    loopopt::HIRFramework *HIRF);

  void print(raw_ostream &OS) const;

  /// \brief Entry point for on-demand call to build the WRGraph.
  /// If FromHIR==true, it walks the HIR; else, it walks the LLVM IR
  void buildWRGraph(InputIRKind IR);

  /// \brief Returns true if ParOpt/VecOpt is able to handle this loop.
  bool isCandidateLoop(Loop &Lp);

  /// \brief Process a BB to extract W-Region information
  void getWRegionFromBB(BasicBlock *BB, WRStack<WRegionNode *> *S);

  /// \brief Identifies WRegionNodes and builds WRGraph for LLVM Dom-Tree
  void buildWRGraphFromLLVMIR(Function &F);

  //TODO: move buildWRGraphFromHIR() from WRegionUtils to WRegionCollection

  /// \brief Getter methods
  WRContainerImpl *getWRGraph() { return WRGraph; }
  DominatorTree *getDomTree() { return DT; }
  LoopInfo *getLoopInfo()     { return LI; }
  ScalarEvolution *getSE()    { return SE; }
  const TargetTransformInfo *getTargetTransformInfo() { return TTI; }
  AssumptionCache *getAssumptionCache() { return AC; }
  const TargetLibraryInfo *getTargetLibraryInfo() { return TLI; }

  /// \brief Returns the size of the WRGraph container
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
};

/// \brief WRegionCollection Pass for the legacy Pass Manager.
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

/// \brief WRegionCollection Pass for the new Pass Manager.
class WRegionCollectionAnalysis
    : public AnalysisInfoMixin<WRegionCollectionAnalysis> {
  friend struct AnalysisInfoMixin<WRegionCollectionAnalysis>;

  static AnalysisKey Key;

public:
  using Result = vpo::WRegionCollection;

  vpo::WRegionCollection run(Function &F, FunctionAnalysisManager &AM);
};

} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H
