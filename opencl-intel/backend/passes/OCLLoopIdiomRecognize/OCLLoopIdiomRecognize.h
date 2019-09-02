// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// This pass implements an idiom recognizer that transforms simple loops into a
// non-loop form. In cases that this kicks in, it can be a significant
// performance win.
//

#ifndef __OCL_LOOP_IDIOM_RECOGNIZE_H_
#define __OCL_LOOP_IDIOM_RECOGNIZE_H_

#include "llvm/Analysis/LoopPass.h"
#include "llvm/Pass.h"

namespace intel {
class OCLLoopIdiomRecognize : public llvm::LoopPass {
public:
  ///@brief Pass identification.
  static char ID;

  /// @brief C'tor.
  OCLLoopIdiomRecognize();

  /// @brief destructor.
  ~OCLLoopIdiomRecognize() {}

  llvm::StringRef getPassName() const override { return "ocl-loop-idiom"; }

  /// @brief LLVM interface.
  /// @param L - Loop to analyze.
  /// @param LPM - Loop Pass manager (unused).
  /// @returns true if the pass made changes (no).
  bool runOnLoop(llvm::Loop *L, llvm::LPPassManager &LPM) override;

  /// @brief LLVM interface.
  /// @param AU - usage of analysis.
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

} // namespace intel

#endif // __OCL_LOOP_IDIOM_RECOGNIZE_H_
