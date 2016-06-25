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

  friend class reversal::HIRLoopReversal;

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
};

} // End namespace loopopt

} // End namespace llvm

#endif
