//==-- RemoveDeviceLibAssertFallback.cpp - Remove devicelib assert def --==//
//
// Copyright (C) 2023 Intel Corporation
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
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/RemoveDeviceLibAssertFallback.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-remove-devicelib-assert-fallback"

PreservedAnalyses
RemoveDeviceLibAssertFallbackPass::run(Module &M, ModuleAnalysisManager &AM) {
  bool Changed = false;
  auto *FuncFail = M.getFunction("__devicelib_assert_fail");
  auto *FuncRead = M.getFunction("__devicelib_assert_read");

  if (FuncFail && !FuncFail->isDeclaration()) {
    // deleteBody will change the function from definition to declaration.
    // It will use the function body from cpu backend builtin.
    // The name is changed to __devicelib_assert_fail_opencl to avoid
    // multiple definition on acc.
    FuncFail->deleteBody();
    FuncFail->setName("__devicelib_assert_fail_opencl");
    Changed = true;
  }

  if (FuncRead && !FuncRead->isDeclaration()) {
    // deleteBody will change the function from definition to declaration.
    // It will use the function body from cpu backend builtin.
    // The name is changed to __devicelib_assert_read__opencl to avoid
    // multiple definition on acc.
    FuncRead->deleteBody();
    FuncRead->setName("__devicelib_assert_read_opencl");
    Changed = true;
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
