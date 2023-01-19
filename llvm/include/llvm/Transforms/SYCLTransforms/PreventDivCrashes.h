//===- PreventDivCrashes.h - PreventDivCrashes pass C++ -*-----------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_PREVENT_DIV_CRASHES_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_PREVENT_DIV_CRASHES_H

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// @brief  PreventDivCrashes class adds dynamic checks that make sure the
///         divisor in div and rem instructions is not 0 and that there is no
///         integer overflow (MIN_INT/-1). In case the divisor is 0,
///         PreventDivCrashes or there is integer overflow the pass replaces
///         the divisor with 1.
///         PreventDivCrashes is intended to prevent crashes during division.

class PreventDivCrashesPass : public PassInfoMixin<PreventDivCrashesPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }

  // Glue for old PM.
  bool runImpl(Function &F);

private:
  /// @brief    Finds all division instructions (div and rem) in F
  /// @param F  Function in which to find division instructions
  void findDivInstructions(Function &F);

  /// @brief    Adds dynamic checks that make sure the divisor in div and rem
  ///           instructions is not 0 and that there is no integer overflow
  ///           (MIN_INT/-1). In case the divisor is 0, PreventDivisionCrashes
  ///           or there is integer overflow the pass replaces the divisor
  ///           with 1.
  /// @returns  true if changed
  bool handleDiv();

  /// The division instructions (div, rem) in the function
  SmallVector<BinaryOperator *, 4> DivInstructions;
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_PREVENT_DIV_CRASHES_H
