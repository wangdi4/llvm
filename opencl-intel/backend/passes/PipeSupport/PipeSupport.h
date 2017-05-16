// Copyright (c) 2017 Intel Corporation
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

// This pass inserts __flush_read/write_pipe calls at the end of every
// functions, that use pipe built-in functions.
//
// This is important to support pipe BIs internal caching mechanism,
// because writes and reads are not immediately committed to the pipe,
// and we must flush them at exist to clear that cache.
//
// Currently it only supports pipes for FPGA emulation, so it works only
// with the Single Call Site restriction applied to the user program:
//   - all pipes used by the function must used only once
//   - pipe call may be in a loop, but this loop cannot be unrolled

#include <llvm/IR/Module.h>

namespace intel {

class PipeSupport : public llvm::ModulePass {
public:
  static char ID;
  PipeSupport();

  virtual StringRef getPassName() const {
    return "PipeSupport";
  }

  bool runOnModule(llvm::Module &M);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

} // namespace intel

#endif // __PIPE_SUPPORT_H__
