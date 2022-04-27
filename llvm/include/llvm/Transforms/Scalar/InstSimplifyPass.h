//===- InstSimplifyPass.h ---------------------------------------*- C++ -*-===//
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
/// \file
///
/// Defines passes for running instruction simplification across chunks of IR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INSTSIMPLIFYPASS_H
#define LLVM_TRANSFORMS_SCALAR_INSTSIMPLIFYPASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Run instruction simplification across each instruction in the function.
///
/// Instruction simplification has useful constraints in some contexts:
/// - It will never introduce *new* instructions.
/// - There is no need to iterate to a fixed point.
///
/// Many passes use instruction simplification as a library facility, but it may
/// also be useful (in tests and other contexts) to have access to this very
/// restricted transform at a pass granularity. However, for a much more
/// powerful and comprehensive peephole optimization engine, see the
/// `instcombine` pass instead.
class InstSimplifyPass : public PassInfoMixin<InstSimplifyPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

FunctionPass *createUnskippableInstSimplifyLegacyPass(); // INTEL
} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INSTSIMPLIFYPASS_H
