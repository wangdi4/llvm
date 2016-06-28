//===------ HIRLoopTransformUtils.h ---------------------- --*- C++ -*---===//
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

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRLOOPTRANSFORM_UTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HIRLOOPTRANSFORM_UTILS_H

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

namespace llvm {

namespace loopopt {

/// \brief Defines HIRLoopTransformationUtils class.
/// It contains static member functions to analyze and transform a loop.
class HIRLoopTransformUtils : public HIRUtils {
private:
  /// \brief Do not allow instantiation
  HIRLoopTransformUtils() = delete;

  /// \brief Updates bound DDRef by setting the correct defined at level and
  /// adding a blob DDref for the newly created temp.
  static void updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                               unsigned DefLevel);

  /// Returns true if unrolling \p OrigLoop by UnrollOrVecFactor requires a
  /// remainder loop. It also creates new bounds for unrolled loops.
  /// \p NewTripCountP is used to return new loop trip count for constant trip
  /// loops \p  NewTCRef is used for non-constant trip loops.
  static bool isRemainderLoopNeeded(HLLoop *OrigLoop,
                                    unsigned UnrollOrVecFactor,
                                    uint64_t *NewTripCountP, RegDDRef **NewTCRef);

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
  // *** HIR-Loop-Reversal-Related Section ***

  /// \brief is the given Loop legal for Reversal
  static bool isHIRLoopReversalLegal(const HLLoop *Lp);

  /// \brief conduct HIR Loop-Reversal Tests and reverse the loop if requested.
  //
  // If all reversal-related tests are positive and the caller indicates a wish
  // to reverse, the given loop will then be reversed.
  //
  // Details:
  // 1. Check if a given loop is suitable for HIR Loop Reversal;
  // 2. If the loop is suitable for reversal and the user indicates a need to
  // reverse (DoReverse is true), then the given loop is reversed.
  //
  // PARAMS:
  // HLLoop * Lp:    a given loop;
  // bool DoReverse: true if the client wants to reverse the loop (provided the
  //                 loop is good to reverse);
  // HIRDDAnalysis &DDAnalysis:client provides HIRDDAnalysis result;
  // bool &Reversed: true if the loop is successfully reversed;
  //
  // Return: bool
  // - true:  the given loop is good/suitable to reverse;
  // - false: otherwise;
  //
  bool checkAndReverseLoop(
      HLLoop *Lp,     // INPUT + OUTPUT: a given loop
      bool DoReverse, // INPUT: client's intention to reverse the loop if the
                      // loop
                      // is suitable
      HIRDDAnalysis &DDAnalysis, // INPUT: client provides a HIRDDAnalysis
      bool &LoopReversed // OUTPUT: true if the loop is successfully reversed
      );

  /// This function creates and returns a  new loop that will be used as the
  ///  main loop for unrolling or vectorization(current clients). The bounds
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
};

} // End namespace loopopt

} // End namespace llvm

#endif
