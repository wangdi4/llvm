//===- PrintPasses.cpp ----------------------------------------------------===//
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

#include "llvm/IR/PrintPasses.h"
#include "llvm/Support/CommandLine.h"
#include <unordered_set>

using namespace llvm;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
// Print IR out before/after specified passes.
static cl::list<std::string>
    PrintBefore("print-before",
                llvm::cl::desc("Print IR before specified passes"),
                cl::CommaSeparated, cl::Hidden);

static cl::list<std::string>
    PrintAfter("print-after", llvm::cl::desc("Print IR after specified passes"),
               cl::CommaSeparated, cl::Hidden);

static cl::opt<bool> PrintBeforeAll("print-before-all",
                                    llvm::cl::desc("Print IR before each pass"),
                                    cl::init(false), cl::Hidden);
static cl::opt<bool> PrintAfterAll("print-after-all",
                                   llvm::cl::desc("Print IR after each pass"),
                                   cl::init(false), cl::Hidden);

#if INTEL_CUSTOMIZATION
static cl::opt<bool> HIRPrintBeforeAll("hir-print-before-all",
            llvm::cl::desc("Prints IR before each pass starting with 'hir'"),
            cl::init(false), cl::Hidden);

static cl::opt<bool> HIRPrintAfterAll("hir-print-after-all",
            llvm::cl::desc("Prints IR after each pass starting with 'hir'"),
            cl::init(false), cl::Hidden);

static cl::opt<bool> PrintModuleBeforeLoopopt(
    "print-module-before-loopopt", cl::init(false), cl::Hidden,
    cl::desc("Prints LLVM module to dbgs() before first HIR transform(HIR SSA "
             "deconstruction)"));
#endif //INTEL_CUSTOMIZATION

static cl::opt<bool>
    PrintModuleScope("print-module-scope",
                     cl::desc("When printing IR for print-[before|after]{-all} "
                              "always print a module IR"),
                     cl::init(false), cl::Hidden);

static cl::list<std::string>
    PrintFuncsList("filter-print-funcs", cl::value_desc("function names"),
                   cl::desc("Only print IR for functions whose name "
                            "match this for all print-[before|after][-all] "
                            "options"),
                   cl::CommaSeparated, cl::Hidden);

/// This is a helper to determine whether to print IR before or
/// after a pass.

bool llvm::shouldPrintBeforeSomePass() {
  return PrintBeforeAll || HIRPrintBeforeAll || !PrintBefore.empty(); // INTEL
}

bool llvm::shouldPrintAfterSomePass() {
  return PrintAfterAll || HIRPrintAfterAll || !PrintAfter.empty(); // INTEL
}

static bool shouldPrintBeforeOrAfterPass(StringRef PassID,
                                         ArrayRef<std::string> PassesToPrint) {
  return llvm::is_contained(PassesToPrint, PassID);
}

bool llvm::shouldPrintBeforeAll() { return PrintBeforeAll; }

bool llvm::shouldPrintAfterAll() { return PrintAfterAll; }

#if INTEL_CUSTOMIZATION
bool llvm::shouldHIRPrintBeforeAll() { return HIRPrintBeforeAll; }

bool llvm::shouldHIRPrintAfterAll() { return HIRPrintAfterAll; }

bool llvm::shouldPrintModuleBeforeLoopopt() { return PrintModuleBeforeLoopopt; }
#endif //INTEL_CUSTOMIZATION

bool llvm::shouldPrintBeforePass(StringRef PassID) {
#if INTEL_CUSTOMIZATION
  return PrintBeforeAll || shouldPrintBeforeOrAfterPass(PassID, PrintBefore)
           || (HIRPrintBeforeAll && PassID.startswith("hir"));
#endif //INTEL_CUSTOMIZATION
}

bool llvm::shouldPrintAfterPass(StringRef PassID) {
#if INTEL_CUSTOMIZATION
  return PrintAfterAll || shouldPrintBeforeOrAfterPass(PassID, PrintAfter)
           || (HIRPrintAfterAll && PassID.startswith("hir"));
#endif //INTEL_CUSTOMIZATION
}

std::vector<std::string> llvm::printBeforePasses() {
  return std::vector<std::string>(PrintBefore);
}

std::vector<std::string> llvm::printAfterPasses() {
  return std::vector<std::string>(PrintAfter);
}

bool llvm::forcePrintModuleIR() { return PrintModuleScope; }

#else // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
#if INTEL_CUSTOMIZATION
std::vector<std::string> llvm::printBeforePasses() {
  return std::vector<std::string>();
}

std::vector<std::string> llvm::printAfterPasses() {
  return std::vector<std::string>();
}
#endif // INTEL_CUSTOMIZATION
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL

bool llvm::isFunctionInPrintList(StringRef FunctionName) {
#if defined(NDEBUG) && !defined(LLVM_ENABLE_DUMP) // INTEL
  return false;
#else // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
  static std::unordered_set<std::string> PrintFuncNames(PrintFuncsList.begin(),
                                                        PrintFuncsList.end());
  return PrintFuncNames.empty() ||
         PrintFuncNames.count(std::string(FunctionName));
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP) // INTEL
}
