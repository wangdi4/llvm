//===--------- WRegionInfo.h - Build WRegionInfo Graph --------*-- C++ --*-===//
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
// This analysis is used to create WRegion Node Graph of identified W-Regions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONINFO_H
#define LLVM_ANALYSIS_VPO_WREGIONINFO_H

#include "llvm/Pass.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"

namespace llvm {

class Function;
class DominatorTree;
struct PostDominatorTree;

namespace vpo {

class WRegion;
class WRegionCollection;

/// \brief This analysis creates W-Region Graph with fills up WRegion node
/// with the information in the directive intrinsics. The overall sequence
/// of building the W-Region Graph is as follows:
///
/// 1) WRegionCollection - collect work regions and build W-Region Graph.
/// 2) WRegionInfo - fill Up W-Region Info by parsing directive intrinsics
///
class WRegionInfo : public FunctionPass {
public:
  /// Iterators to iterate over regions
  typedef WRContainerTy::iterator iterator;
  typedef WRContainerTy::const_iterator const_iterator;
  typedef WRContainerTy::reverse_iterator reverse_iterator;
  typedef WRContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  /// Func - The function we are analyzing.
  Function *Func;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// PDT - The post-dominator tree.
  PostDominatorTree *PDT;

  /// WRC - WRegionCollection
  WRegionCollection *WRC;

  /// \brief Populates W-Region with WRegionNodes.
  void populateWRegion(WRegion *W, BasicBlock *EntryBB, BasicBlock **ExitBB);

public:
  static char ID; // Pass identification
  WRegionInfo();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// \brief Entry point for on-demand call to gather WRegion info out of the 
  /// IR. If FromHIR==true, it walks the HIR; else, it walks the LLVM IR
  void buildWRGraph(bool FromHIR);

  /// WRN Graph
  WRContainerTy *getWRGraph() const { return WRC->getWRGraph(); }

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

} // End namespace vpo

} // End namespace llvm

#endif
