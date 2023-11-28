//===- llvm/Transforms/IPO.h - Interprocedural Transformations --*- C++ -*-===//
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
//
// This header file defines prototypes for accessor functions that expose passes
// in the IPO transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_H
#define LLVM_TRANSFORMS_IPO_H

#include "llvm/ADT/SmallVector.h"
#include <functional>
#include <vector>

namespace llvm {

class ModulePass;
class Pass;
class raw_ostream;
class InlineReportBuilder; // INTEL

//===----------------------------------------------------------------------===//
/// createDeadArgEliminationPass - This pass removes arguments from functions
/// which are not used by the body of the function.
///
ModulePass *createDeadArgEliminationPass();

/// DeadArgHacking pass - Same as DAE, but delete arguments of external
/// functions as well.  This is definitely not safe, and should only be used by
/// bugpoint.
ModulePass *createDeadArgHackingPass();

/// DeadArgumentElimination pass for SYCL kernel functions
ModulePass *createDeadArgEliminationSYCLPass();

//===----------------------------------------------------------------------===//
//
/// createLoopExtractorPass - This pass extracts all natural loops from the
/// program into a function if it can.
///
Pass *createLoopExtractorPass();

/// createSingleLoopExtractorPass - This pass extracts one natural loop from the
/// program into a function if it can.  This is used by bugpoint.
///
Pass *createSingleLoopExtractorPass();

//===----------------------------------------------------------------------===//
/// createBarrierNoopPass - This pass is purely a module pass barrier in a pass
/// manager.
ModulePass *createBarrierNoopPass();

/// What to do with the summary when running passes that operate on it.
enum class PassSummaryAction {
  None,   ///< Do nothing.
  Import, ///< Import information from summary.
  Export, ///< Export information to summary.
};
} // End llvm namespace

#endif
