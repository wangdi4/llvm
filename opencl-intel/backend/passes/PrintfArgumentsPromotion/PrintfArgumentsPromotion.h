// INTEL CONFIDENTIAL
//
// Copyright 2016-2018 Intel Corporation.
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
