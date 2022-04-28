//===- InstrumentationBindings.cpp - instrumentation bindings -------------===//
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
#include "llvm/Transforms/Instrumentation/AddressSanitizer.h" // INTEL
#include "llvm/Transforms/Instrumentation.h"

using namespace llvm;

<<<<<<< HEAD
void LLVMAddThreadSanitizerPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createThreadSanitizerLegacyPassPass());
}

#if INTEL_CUSTOMIZATION
void LLVMAddAddressSanitizerFunctionPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createAddressSanitizerFunctionPass());
}

void LLVMAddAddressSanitizerModulePass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createModuleAddressSanitizerLegacyPassPass());
}

void LLVMAddMemorySanitizerLegacyPassPass(LLVMPassManagerRef PM) {
  unwrap(PM)->add(createMemorySanitizerLegacyPassPass());
}
#endif // INTEL_CUSTOMIZATION

=======
>>>>>>> c74a706893f0667d6aae2d7704d21af97c92dc07
void LLVMAddDataFlowSanitizerPass(LLVMPassManagerRef PM,
                                  int ABIListFilesNum,
                                  const char **ABIListFiles) {
  std::vector<std::string> ABIListFilesVec;
  for (int i = 0; i != ABIListFilesNum; ++i) {
    ABIListFilesVec.push_back(ABIListFiles[i]);
  }
  unwrap(PM)->add(createDataFlowSanitizerLegacyPassPass(ABIListFilesVec));
}
