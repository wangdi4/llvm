//===- InstrumentationBindings.h - instrumentation bindings -----*- C++ -*-===//
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
// This file defines C bindings for the Transforms/Instrumentation component.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BINDINGS_GO_LLVM_INSTRUMENTATIONBINDINGS_H
#define LLVM_BINDINGS_GO_LLVM_INSTRUMENTATIONBINDINGS_H

#include "llvm-c/Core.h"

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: These bindings shouldn't be Go-specific and should eventually move to
// a (somewhat) less stable collection of C APIs for use in creating bindings of
// LLVM in other languages.

#if INTEL_CUSTOMIZATION
void LLVMAddThreadSanitizerPass(LLVMPassManagerRef PM);
void LLVMAddMemorySanitizerLegacyPassPass(LLVMPassManagerRef PM);
#endif // INTEL_CUSTOMIZATION
void LLVMAddDataFlowSanitizerPass(LLVMPassManagerRef PM, int ABIListFilesNum,
                                  const char **ABIListFiles);

#ifdef __cplusplus
}
#endif

#endif
