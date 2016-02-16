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

/// \brief This analysis is the first step in building W-Region Graph. We
/// start by collecting regions as a set of basic blocks in the incoming
/// LLVM IR. This information is then used by WRegionInfo pass to create
/// and populate W-Region nodes.
class WRegionCollection : public FunctionPass {

private:
  /// WRGraph - vector of WRegionNodes.
  WRContainerTy *WRGraph;

  /// Func - The function we are analyzing.
  Function *Func;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

public:
  friend class WRegionNode;

  static char ID; // Pass identification
  WRegionCollection();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// \brief Returns true if ParOpt/VecOpt is able to handle this loop.
  bool isCandidateLoop(Loop &Lp);

  /// \brief performs pre-order visit in LLVM Dom-Tree to get W-Regions

  void doPreOrderDomTreeVisit(BasicBlock *BB, WRStack<WRegionNode *> *S);

  /// \brief Identifies WRegionNodes and builds WRGraph for LLVM Dom-Tree
  void doBuildWRegionGraph(Function &F);

  /// \brief Returns WRGraph
  WRContainerTy *getWRGraph() { return WRGraph; }

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

} // End namespace vpo
} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_WREGIONCOLLECTION_H
