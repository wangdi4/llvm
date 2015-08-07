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
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"

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
  /// WRegions - WRegions collected out of incoming LLVM IR.
  WRContainerTy WRegions;

  /// Func - The function we are analyzing.
  Function *Func;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// PDT - The post-dominator tree.
  PostDominatorTree *PDT;

  /// WRC - WRegionCollection
  WRegionCollection* WRC;

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

  /// \brief Visit WRegion Graph and fill up Info based on the incoming LLVM IR.
  void doFillUpWRegionInfo(WRegionCollection *R);

  /// WRegion iterator methods
  iterator begin() { return WRegions.begin(); }
  const_iterator begin() const { return WRegions.begin(); }
  iterator end() { return WRegions.end(); }
  const_iterator end() const { return WRegions.end(); }

  reverse_iterator rbegin() { return WRegions.rbegin(); }
  const_reverse_iterator rbegin() const { return WRegions.rbegin(); }
  reverse_iterator rend() { return WRegions.rend(); }
  const_reverse_iterator rend() const { return WRegions.rend(); }

  /// \brief Returns the size of the WRegions iplist.
  unsigned getWRegionsSize() const { return WRegions.size();}

  /// \brief Returns true if the WRegions iplist is empty.
  bool WRegionsIsEmpty() const { return WRegions.empty(); }

  // Temporary setting of WRegion container from WRegionCollection.
  // This should be properly fixed when full WRegions(kind) is supported
  /// \brief Sets the list of WRegion graphs collected in WRegionCollection
  void setWRegions();

};

} // End namespace vpo

} // End namespace llvm

#endif
