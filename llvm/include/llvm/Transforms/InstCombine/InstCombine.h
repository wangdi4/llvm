//===- InstCombine.h - InstCombine pass -------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file provides the primary interface to the instcombine pass. This pass
/// is suitable for use in the new pass manager. For a pass that works with the
/// legacy pass manager, use \c createInstructionCombiningPass().
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTCOMBINE_INSTCOMBINE_H
#define LLVM_TRANSFORMS_INSTCOMBINE_INSTCOMBINE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "instcombine"
#include "llvm/Transforms/Utils/InstructionWorklist.h"

namespace llvm {

static constexpr unsigned InstCombineDefaultMaxIterations = 1;

struct InstCombineOptions {
  bool UseLoopInfo = false;
  // Verify that a fix point has been reached after MaxIterations.
  bool VerifyFixpoint = false;
  unsigned MaxIterations = InstCombineDefaultMaxIterations;
#if INTEL_CUSTOMIZATION
  bool PreserveForDTrans;
  bool PreserveAddrCompute;
  bool EnableFcmpMinMaxCombine;
  bool EnableUpCasting;
  bool EnableCanonicalizeSwap;
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  InstCombineOptions(bool PreserveForDTrans = false,
                     bool PreserveAddrCompute = false,
                     bool EnableFcmpMinMaxCombine = true,
                     bool EnableUpCasting = true,
                     bool EnableCanonicalizeSwap = true)
      : UseLoopInfo(false), MaxIterations(InstCombineDefaultMaxIterations),
        PreserveForDTrans(PreserveForDTrans),
        PreserveAddrCompute(PreserveAddrCompute),
        EnableFcmpMinMaxCombine(EnableFcmpMinMaxCombine),
        EnableUpCasting(EnableUpCasting),
        EnableCanonicalizeSwap(EnableCanonicalizeSwap) {}
  #endif // INTEL_CUSTOMIZATION

  InstCombineOptions &setUseLoopInfo(bool Value) {
    UseLoopInfo = Value;
    return *this;
  }

  InstCombineOptions &setVerifyFixpoint(bool Value) {
    VerifyFixpoint = Value;
    return *this;
  }

  InstCombineOptions &setMaxIterations(unsigned Value) {
    MaxIterations = Value;
    return *this;
  }

#if INTEL_CUSTOMIZATION
  // TODO: Currently, these options aren't handled by the opt command at the
  // moment. They need to be integrated in PassRegistry.def in order to enable
  // them through opt.
  InstCombineOptions &setIntelDefaultParams() {
    PreserveForDTrans = false;
    PreserveAddrCompute = false;
    EnableFcmpMinMaxCombine = true;
    EnableUpCasting = true;
    EnableCanonicalizeSwap = true;
    return *this;
  }
#endif // INTEL_CUSTOMIZATION

};

class InstCombinePass : public PassInfoMixin<InstCombinePass> {
private:
  InstructionWorklist Worklist;
  InstCombineOptions Options;

public:
  explicit InstCombinePass(InstCombineOptions Opts = {});
  void printPipeline(raw_ostream &OS,
                     function_ref<StringRef(StringRef)> MapClassName2PassName);

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// The legacy pass manager's instcombine pass.
///
/// This is a basic whole-function wrapper around the instcombine utility. It
/// will try to combine all instructions in the function.
class InstructionCombiningPass : public FunctionPass {
  InstructionWorklist Worklist;
  const bool PreserveForDTrans; // INTEL
  const bool PreserveAddrCompute; // INTEL
  const bool EnableFcmpMinMaxCombine; // INTEL
  const bool EnableUpCasting; // INTEL
  const bool EnableCanonicalizeSwap; // INTEL

public:
  static char ID; // Pass identification, replacement for typeid

#if INTEL_CUSTOMIZATION
  InstructionCombiningPass(bool PreserveForDTrans = false,
                           bool PreserveAddrCompute = false,
                           bool EnableFcmpMinMaxCombine = true,
                           bool EnableUpCasting = true,
                           bool EnableCanonicalizeSwap = true);
  InstructionCombiningPass(bool PreserveForDTrans, bool PreserveAddrCompute,
                           unsigned MaxInterations,
                           bool EnableFcmpMinMaxCombine, bool EnableUpCasting,
                           bool EnableCanonicalizeSwap);
#endif // INTEL_CUSTOMIZATION

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
};

//===----------------------------------------------------------------------===//
//
// InstructionCombining - Combine instructions to form fewer, simple
// instructions. This pass does not modify the CFG, and has a tendency to make
// instructions dead, so a subsequent DCE pass is useful.
//
// This pass combines things like:
//    %Y = add int 1, %X
//    %Z = add int 1, %Y
// into:
//    %Z = add int 2, %X
//
#if INTEL_CUSTOMIZATION
FunctionPass *createInstructionCombiningPass(
    bool PreserveForDTrans = false, bool PreserveAddrCompute = false,
    bool EnableFcmpMinMaxCombine = true, bool EnableUpCasting = true,
    bool EnableCanonicalizeSwap = true);
FunctionPass *createInstructionCombiningPass(bool PreserveForDTrans,
                                             bool PreserveAddrCompute,
                                             unsigned MaxIterations,
                                             bool EnableFcmpMinMaxCombine,
                                             bool EnableUpCasting,
                                             bool EnableCanonicalizeSwap);
#endif // INTEL_CUSTOMIZATION
}

#undef DEBUG_TYPE

#endif
