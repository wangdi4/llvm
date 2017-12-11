// Copyright (c) 2017-2018 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

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
