//===--- HIRFramework.h - public interface of HIR framework ---*-- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is the public interface for the HIR framework. HIR
// transformations should add it as a dependency.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRFRAMEWORK_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRFRAMEWORK_H

#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSymbaseAssignment.h"
// Required for making INVALID_SYMBASE and CONSTANT_SYMBASE available.
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRScalarSymbaseAssignment.h"

namespace llvm {

namespace loopopt {

/// This analysis is the public interface for the HIR framework.
///
/// The overall sequence of building the HIR is as follows-
///
/// 1) HIRRegionIdentification - identifies regions in IR.
/// 2) HIRSCCFormation - identifies SCCs in regions.
/// 3) HIRSSADeconstruction - deconstructs SSA for HIR by inserting copies.
/// 4) HIRCreation - populates HIR regions with a sequence of HLNodes (without
///    HIR loops).
/// 5) HIRCleanup - removes redundant gotos/labels from HIR.
/// 6) HIRLoopFormation - Forms HIR loops within HIR regions.
/// 7) HIRScalarSymbaseAssignment - Assigns symbases to livein/liveout values.
/// 8) HIRParser - Creates DDRefs and parses SCEVs into CanonExprs. Also assigns
///    symbases to non livein/liveout scalars using ScalarSymbaseAssignment's
///    interface.
/// 9) HIRSymbaseAssignment - Assigns symbases to memory DDRefs.
///
class HIRFramework : public FunctionPass {
public:
  /// Iterators to iterate over regions
  typedef HLContainerTy::iterator iterator;
  typedef HLContainerTy::const_iterator const_iterator;
  typedef HLContainerTy::reverse_iterator reverse_iterator;
  typedef HLContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  /// Parser for the function.
  HIRParser *HIRP;

  struct MaxTripCountEstimator;

  void estimateMaxTripCounts() const;

public:
  static char ID; // Pass identification
  HIRFramework();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override {}
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void print(bool FrameworkDetails, raw_ostream &OS,
             const Module *M = nullptr) const;
  void verifyAnalysis() const override;

  /// Returns HLNodeUtils object.
  HLNodeUtils &getHLNodeUtils() const { return HIRP->getHLNodeUtils(); }

  /// Returns DDRefUtils object.
  DDRefUtils &getDDRefUtils() const { return HIRP->getDDRefUtils(); }

  /// Returns CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() const {
    return HIRP->getCanonExprUtils();
  }

  /// Returns BlobUtils object.
  BlobUtils &getBlobUtils() const { return HIRP->getBlobUtils(); }

  /// Region iterator methods
  iterator hir_begin() { return HIRP->hir_begin(); }
  const_iterator hir_cbegin() const { return HIRP->hir_cbegin(); }
  iterator hir_end() { return HIRP->hir_end(); }
  const_iterator hir_cend() const { return HIRP->hir_cend(); }

  reverse_iterator hir_rbegin() { return HIRP->hir_rbegin(); }
  const_reverse_iterator hir_crbegin() const { return HIRP->hir_crbegin(); }
  reverse_iterator hir_rend() { return HIRP->hir_rend(); }
  const_reverse_iterator hir_crend() const { return HIRP->hir_crend(); }

  /// Returns true if \p HInst is a livein copy.
  bool isLiveinCopy(const HLInst *HInst) { return HIRP->isLiveinCopy(HInst); }

  /// Returns true if \p HInst is a liveout copy.
  bool isLiveoutCopy(const HLInst *HInst) { return HIRP->isLiveoutCopy(HInst); }

  /// Returns Function object.
  Function &getFunction() const { return HIRP->getFunction(); }

  /// Returns Module object.
  Module &getModule() const { return HIRP->getModule(); }

  /// Returns LLVMContext object.
  LLVMContext &getContext() const { return HIRP->getContext(); }

  /// Returns DataLayout object.
  const DataLayout &getDataLayout() const { return HIRP->getDataLayout(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif
