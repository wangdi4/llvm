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

#ifndef __CHANNEL_PIPE_TRANSFORMATION_H__
#define __CHANNEL_PIPE_TRANSFORMATION_H__

#include <llvm/IR/Module.h>

namespace intel {

class ChannelPipeTransformation : public llvm::ModulePass {
public:
  static char ID;
  ChannelPipeTransformation();

  virtual llvm::StringRef getPassName() const {
    return "ChannelPipeTransformation";
  }

  bool runOnModule(llvm::Module &M);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

} // namespace intel

#endif // __CHANNEL_PIPE_TRANSFORMATION_H__
