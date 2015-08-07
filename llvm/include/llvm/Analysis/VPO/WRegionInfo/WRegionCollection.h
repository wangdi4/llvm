//===------- WRegionCollection.h - Collect W-Regions -----------*- C++ --*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to identify W-Regions of LLVM IR on which OpenMP,
// Cilk, Offload, Parallel and Vector transformations can be applied.
//
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

namespace vpo {

/// \brief template classe for WRStack
template <class T>
class WRStack
{
public:
  WRStack() {}
  void push(T x);
  void pop();
  T    top();
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

  /// WRegions - Vector of WRegions.
  WRContainerTy WRegionList;

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

  /// \brief Identifies WRegions and build W-Region Graph for LLVM Dom-Tree
  void doBuildWRegionGraph(Function &F);

  /// \brief Returns the size of the WRegionList container
  unsigned getWRegionListSize() {return WRegionList.size();}


  // Iterators to traverse WRegionList
  typedef WRContainerTy::iterator iterator;
  typedef WRContainerTy::const_iterator const_iterator;
  typedef WRContainerTy::reverse_iterator reverse_iterator;
  typedef WRContainerTy::const_reverse_iterator const_reverse_iterator;

  iterator begin() { return WRegionList.begin(); }
  const_iterator begin() const { return WRegionList.begin(); }
  iterator end() { return WRegionList.end(); }
  const_iterator end() const { return WRegionList.end(); }

  reverse_iterator rbegin() { return WRegions.rbegin(); }
  const_reverse_iterator rbegin() const { return WRegionList.rbegin(); }
  reverse_iterator rend() { return WRegionList.rend(); }
  const_reverse_iterator rend() const { return WRegionList.rend(); }

};

} // End namespace vpo

} // End namespace llvm

#endif
