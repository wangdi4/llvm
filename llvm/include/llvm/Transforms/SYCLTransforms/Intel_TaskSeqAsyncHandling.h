// ===--- TaskSeqAsyncHandling.h -------------------------------- C++ -*--=== //
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_TASK_SEQ_ASYNC_HANDLING_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_TASK_SEQ_ASYNC_HANDLING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// This pass handles all async APIs called by task_sequence class.
///
/// 1) It finds all declarations of __async, __get, __create_task_sequence,
///    and __release_task_sequence from frontend, and fill their
///    function bodies -- preparing args and calling backend built-ins;
/// 2) It creates function invokes for the called async functions.
class TaskSeqAsyncHandling : public PassInfoMixin<TaskSeqAsyncHandling> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_TASK_SEQ_ASYNC_HANDLING_H
