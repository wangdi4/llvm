// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __PIPE_SUPPORT_H__
#define __PIPE_SUPPORT_H__

// This pass processes functions with pipe built-in calls to support
// pipe internal caching mechanism. Pipe writes and reads are not immediately
// committed to a pipe, and it should be flushed to clear that cache.
//
// To prevent dead locks it's required to flush all pipes used
// by current function in two cases:
// 1. after every failed read_pipe/write_pipe call (if the call returned -1),
// For example:
//   When 2 kernels (A and B) are communicating over pipes:
//           ..
//   ------  -----> ------     '.' means a cached packet
//   | A  |         | B  |     'x' means a visible packet
//   ------  -----> ------
//           xxxxx
//
//   - Kernel A writes 2 packets, but they are cached, i.e. kernel B
//     doesn't see them yet.
//   - Then Kernel A writes 5 packets and cannot proceed further, because
//     the pipe is full.
//   - According to the algorithm, kernel B should read packets from
//     the 1st pipe, than proceed to the 2nd pipe.
//
//   Since packets in the 1st pipe are cached, kernel B cannot read anything,
//   and kernel A is blocked on the 2nd pipe ('pipe full' condition).
//
//   If non-blocking pipe built-in returned -1, we must flush all pipes,
//   because keeping elements in cache may cause a deadlock.
//
// Blocking pipe BIs calls are resolved here to non-blocking with a spin-loop
// over them to make possible to insert flushes into the spin-loop
// i.e. to flush every time when the blocking call fails and repeat the call.
//
// 2. at exit of every function, that uses pipe built-ins to ensure
// nothing is cached before exit.

#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

namespace intel {

class PipeSupport : public llvm::ModulePass {
public:
  static char ID;
  PipeSupport();

  llvm::StringRef getPassName() const override { return "PipeSupport"; }

  bool runOnModule(llvm::Module &M);

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

} // namespace intel

#endif // __PIPE_SUPPORT_H__
