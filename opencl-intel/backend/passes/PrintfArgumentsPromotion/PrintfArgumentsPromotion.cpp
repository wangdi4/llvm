// Copyright (c) 2015 Intel Corporation
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

#define DEBUG_TYPE "PrintfArgumentsPromotion"

#include <llvm/IR/Module.h>

#include <OCLPassSupport.h>

#include "PrintfArgumentsPromotion.h"

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
