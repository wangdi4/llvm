//===- PrintPasses.h - Determining whether/when to print IR ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_PRINTPASSES_H
#define LLVM_IR_PRINTPASSES_H

#include "llvm/ADT/StringRef.h"
#include <vector>

namespace llvm {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

// Returns true if printing before/after some pass is enabled, whether all
// passes or a specific pass.
bool shouldPrintBeforeSomePass();
bool shouldPrintAfterSomePass();

// Returns true if we should print before/after a specific pass. The argument
// should be the pass ID, e.g. "instcombine".
bool shouldPrintBeforePass(StringRef PassID);
bool shouldPrintAfterPass(StringRef PassID);

// Returns true if we should print before/after all passes.
bool shouldPrintBeforeAll();
bool shouldPrintAfterAll();

#if INTEL_CUSTOMIZATION
// Returns true if we should print before/after all HIR passes.
bool shouldHIRPrintBeforeAll();
bool shouldHIRPrintAfterAll();
#endif //INTEL_CUSTOMIZATION
// Returns true if we should always print the entire module.
bool forcePrintModuleIR();

#endif //!defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

// The list of passes to print before/after, if we only want to print
// before/after specific passes.
std::vector<std::string> printBeforePasses();
std::vector<std::string> printAfterPasses();

// Returns true if we should print the function.
bool isFunctionInPrintList(StringRef FunctionName);

} // namespace llvm

#endif // LLVM_IR_PRINTPASSES_H
