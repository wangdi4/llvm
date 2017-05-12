//===------ HIRTransformUtils.h ---------------------------- --*- C++ -*---===//
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
// This file defines utility functions for the following transformations:
// - HIR Loop Reversal;
// -
// -
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRTRANSFORM_UTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRTRANSFORM_UTILS_H

#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/Intel_LoopIR/HLNode.h"

#include <stdint.h>

namespace llvm {

namespace loopopt {

class RegDDRef;
class HLLoop;
class HLIf;
class HIRDDAnalysis;
class HIRSafeReductionAnalysis;
class HIRLoopStatistics;

/// Defines HIRLoopTransformationUtils class.
/// It contains static member functions to analyze and transform a loop.
class HIRTransformUtils {
private:
  /// Do not allow instantiation
  HIRTransformUtils() = delete;

  /// Updates bound DDRef by setting the correct defined at level and
  /// adding a blob DDref for the newly created temp.
  static void updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                               unsigned DefLevel);

  /// Returns true if unrolling \p OrigLoop by UnrollOrVecFactor requires a
  /// remainder loop. It also creates new bounds for unrolled loops.
  /// \p NewTripCountP is used to return new loop trip count for constant trip
  /// loops \p  NewTCRef is used for non-constant trip loops.
  static bool isRemainderLoopNeeded(HLLoop *OrigLoop,
                                    unsigned UnrollOrVecFactor,
                                    uint64_t *NewTripCountP,
                                    RegDDRef **NewTCRef);

  /// \brief Creates a new loop for unrolling or vectorization. \p NewTripCount
  /// contains the new loop trip count if the original loop is a constant trip
  /// count. For a original non-constant trip count loop, the new loop trip
  /// count is specified in \p NewTCRef.
  static HLLoop *createUnrollOrVecLoop(HLLoop *OrigLoop,
                                       unsigned UnrollOrVecFactor,
                                       uint64_t NewTripCount,
                                       const RegDDRef *NewTCRef, bool VecMode);

  /// \brief Processes the remainder loop for general unrolling and
  /// vectorization. The loop passed in \p OrigLoop is set up to be
  /// the remainder loop with lowerbound set using \p NewTripCount or
  /// \p NewTCRef.
  static void processRemainderLoop(HLLoop *OrigLoop, unsigned UnrollOrVecFactor,
                                   uint64_t NewTripCount,
                                   const RegDDRef *NewTCRef);

public:
  ///
  /// Do Reversal Tests for a given HIR inner-most loop and return true if
  /// the loop is reversible.
  //
  // Parameters:
  // -HLLoop*: the HLLoop *
  // -HIRDDAnalysis &: Existing DD Analysis
  // -HIRSafeReductionAnalysis &: Existing SafeReduction analysis
  // -HIRLoopStatistics &: existing LoopStatistics analysis
  // -DoProfitTest:
  //    Whether to conduct profitable test using reverser's profit model.
  //    Default is false, which ignores Reverser's profit model and don't do
  //    profit test.
  //
  // Return: bool
  // - true:  the loop is reversible;
  // - false: otherwise;
  //
  // Note: the following decisions are made after group (HPO+Vectorization)
  //       discussions.
  //
  // - DoProfitTest:
  //   This is an option to the client, and can be ignored if the client decides
  //   to proceed without using Reversal's profit model.
  //
  // - NO DoLegalTest flag
  //   Can't allow a client to skip this test. The client may not have a legal
  //   model thus has to rely on Reversal's legal model instead.
  //   Even if the client does have one, it may be quite different from the
  //   reversal's legal model, and may not want to spend effort maintaining.
  //   As a result, this function will implicitly perform legal test.
  //
  // There is also an idea of passing context from this API to the next one in
  // order to avoid doing repetitive work on collection and analysis (save some
  // compile time). This idea is currently on hold due to unclear usage cases
  // from client.
  // Will revisit the situation once there is any request from potential client.
  //
  static bool isHIRLoopReversible(
      HLLoop *InnermostLp,            // INPUT + OUTPUT: a given loop
      HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
      HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
      HIRLoopStatistics &HLS,         // INPUT: Existing HIRLoopStatitics
      bool DoProfitTest = false       // INPUT: Control Profit Tests
      );

  /// Do Certain Reversal Tests for a given HIR inner-most loop.
  /// Reverse the loop if the loop is legal to reverse.
  //
  // Parameters:
  // -HLLoop*: the HLLoop *
  // -HIRDDAnalysis &: Existing DD Analysis
  // -HIRSafeReductionAnalysis &: Existing SafeReduction analysis
  // -HIRLoopStatistics &: existing LoopStatistics analysis
  //
  // Return: void
  // - assert if any required reversal tests fail.
  // - reverse the loop if reversal tests are successful.
  //
  // Note: the following decisions are made after group discussions.
  //
  // Ideally, a client will call this function after a previous call to
  // isHIRLoopReverible(-) and the loop is indeed reversible.
  // However, in case the client calls this function directly without proper
  // preparation, this function will implicitly assert on preliminary check
  // and legal check, only ignoring profit check.
  //
  static void doHIRLoopReversal(
      HLLoop *InnermostLp,            // INPUT + OUTPUT: an inner-most loop
      HIRDDAnalysis &HDDA,            // INPUT: HIR DDAnalysis
      HIRSafeReductionAnalysis &HSRA, // INPUT: HIRSafeReductionAnalysis
      HIRLoopStatistics &HLS          // INPUT: Existing HIRLoopStatitics
      );

  // Do HIR Loop-Invariant Memory Motion (LIMM) on a given loop
  //
  // Return: bool
  // -true: if any LIMM is promoted;
  // -false: otherwise;
  //
  static bool doHIRLoopInvariantMemoryMotion(
      HLLoop *InnermostLp,   // INPUT + OUTPUT: a given innermost loop
      HIRDDAnalysis &HDDA,   // INPUT: HIR DDAnalysis
      HIRLoopStatistics &HLS // INPUT: Existing HIRLoopStatitics Analysis
      );

  // Do HIR Loop-Redundant Memory Motion (LRMM) on a given loop
  //
  // Return: bool
  // -true: if any LRMM is promoted;
  // -false: otherwise;
  //
  static bool doHIRLoopRedundantMemoryMotion(
      HLLoop *InnermostLp,   // INPUT + OUTPUT: a given innermost loop
      HIRDDAnalysis &HDDA,   // INPUT: HIR DDAnalysis
      HIRLoopStatistics &HLS // INPUT: Existing HIRLoopStatitics Analysis
      );

  // Do HIR Loop Memory Motion on a given innermost loop
  //
  // This will promote both Loop-Invariant Memory Motion (LIMM) and
  // Loop-Redundant Memory Motion (LRMM) if available.
  //
  // Return: bool
  // -true: if any LIMM or LRMM is promoted;
  // -false: otherwise;
  //
  static bool doHIRLoopMemoryMotion(
      HLLoop *InnermostLp,   // INPUT + OUTPUT: a given innermost loop
      HIRDDAnalysis &HDDA,   // INPUT: HIR DDAnalysis
      HIRLoopStatistics &HLS // INPUT: Existing HIRLoopStatitics Analysis
      );

  /// This function creates and returns a new loop that will be used as the
  /// main loop for unrolling or vectorization(current clients). The bounds
  /// for this newly created loop are set appropriately using the bounds of
  /// \p OrigLoop and \p UnrollOrVecFactor. If a remainder loop is needed,
  /// \p NeedRemainderLoop is set to true and the bounds of \p OrigLoop are
  /// updated appropriately. Client is responsible for deleting OrigLoop if
  /// a remainder loop is not needed. \p VecMode specifies whether the
  /// client is vectorizer - which is used to set loop bounds and stride
  /// appropriately as vectorizer uses \p UnrollOrVecFactor as stride whereas
  /// unroller users a stride of 1. The default client is assumed to be the
  /// unroller.
  static HLLoop *setupMainAndRemainderLoops(HLLoop *OrigLoop,
                                            unsigned UnrollOrVecFactor,
                                            bool &NeedRemainderLoop,
                                            bool VecMode = false);

  /// Updates Loop properties (Bounds, etc) based on input Permutations
  /// Used by Interchange now. Could be used later for blocking.
  /// Loops are added to \p LoopPermutation in the desired permuted order.
  static void
  permuteLoopNests(HLLoop *OutermostLoop,
                   const SmallVectorImpl<const HLLoop *> &LoopPermutation);

  /// Updates target HLLabel in every HLGoto node according to the mapping.
  static void remapLabelsRange(const HLNodeMapper &Mapper, HLNode *Begin,
                               HLNode *End);
};

} // End namespace loopopt

} // End namespace llvm

#endif
