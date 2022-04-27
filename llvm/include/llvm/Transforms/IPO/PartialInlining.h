//===- PartialInlining.h - Inline parts of functions ------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
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
//
// This pass performs partial inlining, typically by inlining an if statement
// that surrounds the body of the function.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_PARTIALINLINING_H
#define LLVM_TRANSFORMS_IPO_PARTIALINLINING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;

/// Pass to remove unused function declarations.
class PartialInlinerPass : public PassInfoMixin<PartialInlinerPass> {
private:                                               // INTEL
  bool RunLTOPartialInline;                            // INTEL
  bool EnableSpecialCases;                             // INTEL
public:
#if INTEL_CUSTOMIZATION
  PartialInlinerPass(bool RunLTOPartialInline = false,
      bool EnableSpecialCases = false) :
      RunLTOPartialInline(RunLTOPartialInline),
      EnableSpecialCases(EnableSpecialCases) {}
#endif // INTEL_CUSTOMIZATION
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_PARTIALINLINING_H
