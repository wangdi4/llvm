//===----- HIRVectVLSAnalysis.h - Provides Locality Analysis -----*- C++-*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// The purpose of this analysis is to provide information on neighbouring and
/// overlapping vector loads and stores in a loop nest.
/// It doesn't store any information.
/// Currently it implement a test vectorizer VLS-client analysis pass to
/// exercise the functionality of the HIRVLSClient routines. The actual
/// analysis pass will be driven by the AVR-vectorizer/VPO-Driver.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRVECTVLS_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRVECTVLS_H

#include <map>

#include "llvm/ADT/DenseMap.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRVLSClient.h"
#include "llvm/Analysis/Intel_OptVLS.h"

namespace llvm {

namespace loopopt {

class HLLoop;

class HIRVectVLSAnalysis : public HIRAnalysisPass {

private:
  DDAnalysis *DDA;

public:
  HIRVectVLSAnalysis()
      : HIRAnalysisPass(ID, HIRAnalysisPass::HIRVectVLSAnalysisVal) {}

  static char ID;
  bool runOnFunction(Function &F) override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void getAnalysisUsage(AnalysisUsage &AU) const;
  void releaseMemory() override;
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// \brief Given a vector of memory references in \p Memrefs, this function
  /// creates an HIRVLSCLientMemref object for each of them to represent
  /// information about the access pattern of the memory reference, in an IR
  /// independent way, as required for VLS grouping analysis.
  /// \param VecContext holds the vectorization context underwhich the
  /// \p MemRefs are analyzed.
  /// \param [out] Mrfs holds the HIRVLSClientMemrefs created by this function.
  void getVLSMemrefs(VectVLSContext *VectContext, LoopMemrefsVector &MemRefs,
                     OVLSMemrefVector &Mrfs);

  /// \brief Find groups of neighbouring memory references.
  /// \param Memrefs holds the memory references that this function will
  /// try to group.
  /// \param VecContext holds the vectorization context underwhich the
  /// \p Memrefs are analyzed.
  /// \param [out] Grps holds the VLS Groups that are found by this function.
  void computeVLSGroups(const OVLSMemrefVector &Memrefs,
                        VectVLSContext *VectContext, OVLSGroupVector &Grps);

  /// \brief Tester for debugging. Drives the routines that are required by
  /// VLS optimization without going through the VLS server itself and without
  /// creating OVLSMemrefs (just working purely on HIR DDRefs).
  void testVLSMemrefAnalysis(VectVLSContext *VectContext,
                             LoopMemrefsVector &MemRefs);

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRVectVLSAnalysisVal;
  }

private:
  void analyze();
  void analyzeVLSInLoop(const HLLoop *Loop);
};

} // End namespace loopopt

} // End namespace llvm

#endif
