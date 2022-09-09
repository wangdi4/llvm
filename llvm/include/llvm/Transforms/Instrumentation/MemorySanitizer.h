//===- Transforms/Instrumentation/MemorySanitizer.h - MSan Pass -----------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
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
// This file defines the memoy sanitizer pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_MEMORYSANITIZER_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_MEMORYSANITIZER_H

#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class Function;
class Module;
class StringRef;
class raw_ostream;

struct MemorySanitizerOptions {
  MemorySanitizerOptions() : MemorySanitizerOptions(0, false, false, false){};
  MemorySanitizerOptions(int TrackOrigins, bool Recover, bool Kernel)
      : MemorySanitizerOptions(TrackOrigins, Recover, Kernel, false) {}
  MemorySanitizerOptions(int TrackOrigins, bool Recover, bool Kernel,
                         bool EagerChecks);
  bool Kernel;
  int TrackOrigins;
  bool Recover;
  bool EagerChecks;
};

#if INTEL_CUSTOMIZATION
// Insert MemorySanitizer instrumentation (detection of uninitialized reads)
FunctionPass *
createMemorySanitizerLegacyPassPass(MemorySanitizerOptions Options = {});
#endif // INTEL_CUSTOMIZATION

/// A module pass for msan instrumentation.
///
/// Instruments functions to detect unitialized reads. This function pass
/// inserts calls to runtime library functions. If the functions aren't declared
/// yet, the pass inserts the declarations. Otherwise the existing globals are
/// used.
struct ModuleMemorySanitizerPass : public PassInfoMixin<ModuleMemorySanitizerPass> {
  ModuleMemorySanitizerPass(MemorySanitizerOptions Options) : Options(Options) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  void printPipeline(raw_ostream &OS,
                     function_ref<StringRef(StringRef)> MapClassName2PassName);
  static bool isRequired() { return true; }

private:
  MemorySanitizerOptions Options;
};
}

#endif /* LLVM_TRANSFORMS_INSTRUMENTATION_MEMORYSANITIZER_H */
