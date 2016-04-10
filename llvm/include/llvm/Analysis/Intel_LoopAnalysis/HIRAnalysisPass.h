//===------- HIRAnalysisPass.h - Base class for HIR analyses -*- C++ -*----===//
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
// This header file declares the base class for HIR analyis passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRANALYSISPASS_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRANALYSISPASS_H

#include "llvm/Pass.h"

#include "llvm/Support/Casting.h"

namespace llvm {

namespace loopopt {

class HLRegion;
class HLLoop;

/// \brief - All HIR analysis passes should derive from this class.
///
/// Pass setup requirements (see HIRCompleteUnroll.cpp for ref)-
///
/// - Define under Intel_LoopAnalysiss directory.
/// - Use the INITIALIZE_PASS* macros for initialization.
/// - Declare initialize<PassName>Pass() in llvm/InitializePasses.h and add a
///   call in Intel_LoopAnalysiss.cpp.
/// - Declare create<PassName>Pass() in Intel_LoopAnalysiss/Passes.h, define
///   it in your file and add a call in llvm/LinkAllPasses.h (so it is not
///   optimized away) and PassManagerBuilder.cpp (to add it to clang opt
///   pipeline).
/// - Define pass under loopopt namespace.
/// - Declare HIRFramework pass as required to access HIR.
/// - Always call setPreservesAll() in getAnalysisUsage().
/// - Add a new value for the pass to HIRAnalysisVal enum before 
///   HIRPassCountVal.
/// Pass this value in the constructor.
/// - Add a static classof() member for supporting LLVM's RTTI.
/// - Add function calls for the pass to
/// HIRInvalidationUtils::invalidateLoopBodyAnalysis(),
/// HIRInvalidationUtils::invalidateLoopBoundsAnalysis() and
/// HIRInvalidationUtils::invalidateTopLevelNodeAnalysis() in
/// HIRInvalidationUtils.h.
class HIRAnalysisPass : public FunctionPass {
public:
  /// \brief An enumeration to keep track of the subclasses.
  enum HIRAnalysisVal {
    HIRDDAnalysisVal,
    HIRLocalityAnalysisVal,
    HIRVectVLSAnalysisVal,
    // Should be kept last
    HIRPassCountVal
  };

private:
  /// ID to differentiate between concrete subclasses.
  const HIRAnalysisVal SubClassID;

protected:
  HIRAnalysisPass(char &ID, HIRAnalysisVal SCID)
      : FunctionPass(ID), SubClassID(SCID) {}

public:
  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  /// be used for any other purpose.
  HIRAnalysisVal getHIRAnalysisID() const { return SubClassID; }

  // Interface for derived classes to invalidate analysis for
  // regions/loops/nodes.

  /// \brief This method informs the analysis that the loop body has been
  /// modified. Most analysis would want to implement this function to
  /// invalidate results.
  virtual void markLoopBodyModified(const HLLoop *Lp) = 0;

  /// \brief This method informs the analysis that the loop bounds has been
  /// modified. Default implementation is empty since most analysis would not
  /// care about changes to loop bounds alone.
  virtual void markLoopBoundsModified(const HLLoop *Lp) {}

  /// \brief This methods informs the analysis that one or more nodes which lie
  /// outside any loop in the region have been modified. Default implementation
  /// is empty since most analysis would not care about changes to nodes
  /// outside loops.
  virtual void markNonLoopRegionModified(const HLRegion *Reg) {}
};

} // End namespace loopopt

} // End namespace llvm

#endif
