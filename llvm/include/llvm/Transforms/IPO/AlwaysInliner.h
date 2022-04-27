//===-- AlwaysInliner.h - Pass to inline "always_inline" functions --------===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Provides passes to inlining "always_inline" functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_ALWAYSINLINER_H
#define LLVM_TRANSFORMS_IPO_ALWAYSINLINER_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h" // INTEL
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h" // INTEL

namespace llvm {

class Module;
class Pass;

/// Inlines functions marked as "always_inline".
///
/// Note that this does not inline call sites marked as always_inline and does
/// not delete the functions even when all users are inlined. The normal
/// inliner should be used to handle call site inlining, this pass's goal is to
/// be the simplest possible pass to remove always_inline function definitions'
/// uses by inlining them. The \c GlobalDCE pass can be used to remove these
/// functions once all users are gone.
class AlwaysInlinerPass : public PassInfoMixin<AlwaysInlinerPass> {
  bool InsertLifetime;

  // INTEL The inline report
  InlineReport *Report; // INTEL
  InlineReportBuilder *MDReport; // INTEL

public:
  AlwaysInlinerPass(bool InsertLifetime = true);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

  InlineReport* getReport() { return Report; } // INTEL
  InlineReportBuilder* getMDReport() { return MDReport; } // INTEL
  static bool isRequired() { return true; }
};

/// Create a legacy pass manager instance of a pass to inline and remove
/// functions marked as "always_inline".
Pass *createAlwaysInlinerLegacyPass(bool InsertLifetime = true);

#if INTEL_CUSTOMIZATION
Pass *createUnskippableAlwaysInlinerLegacyPass(bool InsertLifetime = true);
#endif // INTEL_CUSTOMIZATION
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_ALWAYSINLINER_H
