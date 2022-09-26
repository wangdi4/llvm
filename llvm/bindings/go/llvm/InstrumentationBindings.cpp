//===- InstrumentationBindings.cpp - instrumentation bindings -------------===//
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
// This file defines C bindings for the instrumentation component.
//
//===----------------------------------------------------------------------===//

#include "InstrumentationBindings.h"
#include "llvm-c/Core.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/MemorySanitizer.h" // INTEL
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h" // INTEL

using namespace llvm;

#if INTEL_CUSTOMIZATION
void LLVMAddThreadSanitizerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createThreadSanitizerLegacyPassPass());
}

void LLVMAddMemorySanitizerLegacyPassPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createMemorySanitizerLegacyPassPass());
}
#endif // INTEL_CUSTOMIZATION

void LLVMAddDataFlowSanitizerPass(LLVMPassManagerRef PM,
                                  int ABIListFilesNum,
                                  const char **ABIListFiles) {
  std::vector<std::string> ABIListFilesVec;
  for (int i = 0; i != ABIListFilesNum; ++i) {
    ABIListFilesVec.push_back(ABIListFiles[i]);
  }
  unwrap(PM)->add(createDataFlowSanitizerLegacyPassPass(ABIListFilesVec));
}
