#if INTEL_CUSTOMIZATION
//===-- Coroutines.h - Coroutine Transformations ----------------*- C++ -*-===//
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
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declare accessor functions for coroutine lowering passes.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_COROUTINES_H
#define LLVM_TRANSFORMS_COROUTINES_H

namespace llvm {

class Pass;
class PassManagerBuilder;

/// Add all coroutine passes to appropriate extension points.
void addCoroutinePassesToExtensionPoints(PassManagerBuilder &Builder);

/// Lower coroutine intrinsics that are not needed by later passes.
Pass *createCoroEarlyLegacyPass();

/// Split up coroutines into multiple functions driving their state machines.
Pass *createCoroSplitLegacyPass(bool IsOptimizing = false);

/// Analyze coroutines use sites, devirtualize resume/destroy calls and elide
/// heap allocation for coroutine frame where possible.
Pass *createCoroElideLegacyPass();

/// Lower all remaining coroutine intrinsics.
Pass *createCoroCleanupLegacyPass();

}

#endif
#endif // INTEL_CUSTOMIZATION
