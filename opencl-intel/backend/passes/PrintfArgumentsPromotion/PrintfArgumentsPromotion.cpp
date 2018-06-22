// INTEL CONFIDENTIAL
//
// Copyright 2015-2018 Intel Corporation.
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

#define DEBUG_TYPE "PrintfArgumentsPromotion"

#include "PrintfArgumentsPromotion.h"
#include "OCLPassSupport.h"

#include <llvm/IR/Attributes.h>
#include <llvm/IR/Module.h>

using namespace llvm;

extern "C" {
  ModulePass *createPrintfArgumentsPromotionPass() {
    return new intel::PrintfArgumentsPromotion();
  }
}

namespace intel {
  namespace {
    char const * printfFuncName = "printf";

    // Only f32 scalars and vectors of i8, i16, f16 and f32 are to be promoted.
    bool isPromotionNeeded(Type * ty) {
      // Scalars are promoted by Clang in accordance with C99 specificaiton.
      // But, following the OpenCL C specification:
      // "6.12.13.3 Differences between OpenCL C and C99 printf
      //   The conversion specifiers f, F, e, E, g, G, a, A convert a float
      //   argument to a double only if the double data type is supported.
      //   Refer to the description of CL_DEVICE_DOUBLE_FP_CONFIG. If the
      //   double data type is not supported, the argument will be a float
      //   instead of a double."
      // Clang doesn't promote floats if double data type is not supported.
      //
      // On the other hand the current implementation of printf reuses
      // printf-like functions, provided by the host compiler run-time.
      // Since these functions have ellipsis as the last argument, the default
      // argument promotion (see C spec 6.5.2.2 Function calls p6, p7) is to be
      // performed on trailing arguments to match the system ABI for variadic
      // functions. Therefore we have to promote floats.
      if(!ty->isVectorTy() && !ty->isFloatTy()) return false;

      Type * scaTy = ty->getScalarType();
      return scaTy->isIntegerTy(8) || scaTy->isIntegerTy(16) ||
             scaTy->isHalfTy() || scaTy->isFloatTy();
    }

    Type * getPromotedTy(Type * srcTy) {
      assert((srcTy->isFloatTy() || srcTy->isVectorTy()) &&
             "scalar float or vector types are to be promoted only");
      Type * scaTy = srcTy->getScalarType();
      Type * scaPromoTy = nullptr;
      LLVMContext & ctx = srcTy->getContext();

      if(scaTy->isIntegerTy(8) || scaTy->isIntegerTy(16))
        scaPromoTy = Type::getInt32Ty(ctx);
      else if(scaTy->isHalfTy() || scaTy->isFloatTy())
        scaPromoTy = Type::getDoubleTy(ctx);
      assert(scaPromoTy && "unsupported source type");

      if (VectorType *srcVecTy = dyn_cast<VectorType>(srcTy))
        return VectorType::get(scaPromoTy, srcVecTy->getNumElements());
      return scaPromoTy;
    }
  }

  // Register pass to for opt
  OCL_INITIALIZE_PASS(PrintfArgumentsPromotion, "printf-args-promotion",
                      "Promote variadic arguments of printf function for CPU BE",
                      false, false)

  char PrintfArgumentsPromotion::ID = 0;

  bool PrintfArgumentsPromotion::runOnModule(Module &M) {
    Function * printfFunc = M.getFunction(printfFuncName);
    // Check if the module has the call to printf function.
    if(!printfFunc) return false;
    assert(printfFunc->isVarArg() && "printf must be variadic");

    bool changed = false;
    for(User * user: printfFunc->users()) {
      CallInst * printfCI = dyn_cast<CallInst>(user);
      if(!printfCI || printfCI->getNumArgOperands() < 2) continue;

      // Set NoBuiltin attribute to avoid replacements by 'puts'/'putc'.
      if (!printfCI->isNoBuiltin())
        printfCI->addAttribute(AttributeList::FunctionIndex,
                               Attribute::NoBuiltin);

      for(Use & use: printfCI->arg_operands()) {
        Value * argVal = use;
        Type * argTy = argVal->getType();
        if(!isPromotionNeeded(argTy)) continue;

        // According to OpenCL 1.2 spec. the vector aruments aren't subjects for the
        // "default arugment promotions". Also it doesn't matter if the resulting integer
        // vector type has sign or doesn't which is allowed the spec., see the following
        // excerpt from it.
        //
        // 6.12.13.2 printf format string
        //   If a vector specifier appears without a length modifier, the behavior is
        //   undefined. The vector data type described by the vector specifier and length
        //   modifier must match the data type of the argument; otherwise the behavior is
        //   undefined.

        // It is safe here to use Cast instructions instead of OpenCL built-ins since
        // the resulting type is wider so no need for truncation or rounding.
        Type * promoTy = getPromotedTy(argTy);
        Value * promoVal =  promoTy->getScalarType()->isIntegerTy() ?
          CastInst::CreateIntegerCast(argVal, promoTy, false, "printf.promoted", printfCI) :
          CastInst::CreateFPCast(argVal, promoTy, "printf.promoted", printfCI);
        // Replace the operand with the promoted value. It should be OK that the values
        // have different types because these are variadic arguments.
        use.getUser()->setOperand(use.getOperandNo(), promoVal);
        changed = true;
      }
    }

    return changed;
  }

} // namespace intel
