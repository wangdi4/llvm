// Copyright (c) 2016 Intel Corporation
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

#ifndef __PRINTF_ARGUMENTS_PROMOTION_PASS__
#define __PRINTF_ARGUMENTS_PROMOTION_PASS__

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>

namespace intel {
  // Promote variadic argument for printf function according to C99 spec.
  //
  // SPIR 1.2/2.0/V generators do not promote variadic arguments to printf
  // function which is OK by the spec. Although there is a subtle case with
  // scalar float to double promotion. C99 spec. demands it to be promoted
  // while OpenCL C 1.2/2.0 do not mention it in spite of that some devices
  // may have doubles unsupported.
  // Also, CClang does the promotion of the vector types:
  // charN/ucharN   -> intN/uintN
  // shortN/ushortN -> intN/uintN
  // floatN         -> doubleN
  // and CPU BE implementation of the printf function expects the arguments
  // to be promoted. So, this pass does the promotion of the variadic
  // arguments to overcome these discrepancies/uncertanties.
  class PrintfArgumentsPromotion : public llvm::ModulePass {
  public:
    static char ID;

    PrintfArgumentsPromotion() : llvm::ModulePass(ID)
    {}

    bool runOnModule(llvm::Module &) final;

    virtual llvm::StringRef getPassName () const final {
      return "intel::PrintfArgumentsPromotion";
    }
  };

} // namespace intel
#endif
