// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
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

#include "PreventDivCrashes.h"
#include "InitializePasses.h"
#include "NameMangleAPI.h"
#include "OCLPassSupport.h"
#include "ParameterType.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::NameMangleAPI;

extern "C" {
  /// @brief Creates new PreventDivCrashes module pass
  /// @returns new PreventDivCrashes module pass
  void* createPreventDivisionCrashesPass() {
    return new intel::PreventDivCrashes();
  }
}

// Add command line to specify EyeQ device behavior
static llvm::cl::opt<bool> EyeQDivCrashBehavior(
    "eyeq-div-crash-behavior", llvm::cl::init(false),
    llvm::cl::desc("The flag indicates that bad integer divisions (e.g. 1/0) should behave"
                   "As they would in an EyeQ device"));

namespace intel {

char PreventDivCrashes::ID = 0;

/// Register pass to for opt
  OCL_INITIALIZE_PASS(PreventDivCrashes, "prevent-div-crash", "Dynamically handle cases that divisor is zero or there is integer overflow during division", false, false)

  static const unsigned int DIVIDEND_POSITION = 0;
  static const unsigned int DIVISOR_POSITION = 1;

  bool PreventDivCrashes::runOnFunction(Function &F) {
    m_divInstuctions.clear();
    findDivInstructions(F);
    return  handleDiv();
  }

  void PreventDivCrashes::findDivInstructions(Function &F) {

    // Go over all instructiuons in the function
    // Add div and rem instruction to m_divInstuctions
    for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {

      // Div and rem instructions are of type BinaryOperator
      BinaryOperator* inst = dyn_cast<BinaryOperator>(&*i);
      if (!inst) continue;

      Instruction::BinaryOps opcode = inst->getOpcode();

      // Check if opcode is div or rem
      if((opcode == Instruction::UDiv)
            || (opcode == Instruction::SDiv)
            || (opcode == Instruction::URem)
            || (opcode == Instruction::SRem)) {

        m_divInstuctions.push_back(inst);

      }
    }
  }

  bool PreventDivCrashes::handleDiv() {

    // Check if there are division instrcutions
    if (m_divInstuctions.empty()) return false;

    // Go over all possible division instructions, extract the divisor and dividend and:
    // 1. Compares divisor to -1 and dividend to MIN_INT.
    //        This prevents integer overflow: Because of 2's-complement representation
    //        the only problematic case is INT_MIN / -1, which equals INT_MAX + 1.
    // 2. Compares divisor to 0
    //        This prevent division by zero.
    // If one of the above comparisons is true the function replace the divisor with 1, so that we won't crash.

    // For EyeQ devices (for EyeQ bit exactness) return 0 for division by zero
    // (Replace dividend by 0 in addition to replacing divisor by 1).


    // Example:

    // %div = sdiv i32 %conv2, %conv

    // Transfers to:

    // %isDivisorNegOne     =   icmp eq i32 %1, -1
    // %isDividendMinInt    =   icmp eq i32 %0, -2147483648
    // %isIntegerOverflow   =   and i1 %isDivisorNegOne, %isDividendMinInt
    // %isDivisorZero       =   icmp eq i32 %1, 0
    // %isDivisorBad        =   or i1 %isIntegerOverflow, %isDivisorZero
    // %newiDvisor          =   select i1 %isDivisorBad, i32 1, i32 %1
    // %div                 =   sdiv i32 %0, %newiDvisor

    // For EyeQ devices will transfer to:
    // %isDivisorNegOne     =   icmp eq i32 %1, -1
    // %isDividendMinInt    =   icmp eq i32 %0, -2147483648
    // %isIntegerOverflow   =   and i1 %isDivisorNegOne, %isDividendMinInt
    // %isDivisorZero       =   icmp eq i32 %1, 0
    // %isDivisorBad        =   or i1 %isIntegerOverflow, %isDivisorZero
    // %newDivisor          =   select i1 %isDivisorBad, i32 1, i32 %1
    // %newDividend         =   select i1 %isDivisorZero, i32 0, i32 %0
    // %div                 =   sdiv i32 %newDividend, %newiDvisor

    IRBuilder<> Builder(m_divInstuctions[0]);
    for (unsigned i=0; i< m_divInstuctions.size(); ++i) {

      BinaryOperator* divInst = m_divInstuctions[i];
      Builder.SetInsertPoint(divInst);

      // Extract the context
      LLVMContext& context = divInst->getContext();

      // Extract the divisor and its type
      Value* divisor = divInst->getOperand(DIVISOR_POSITION);
      Type* divisorType = divInst->getType();
      Type* divisorElemType = divisorType;                            // Needed in case operandType is VectorTy
      Type* isIntegerOverflowType = IntegerType::getInt1Ty(context);  // Needed for Integer overflow cases, creates i1 type to represebt a boolean

      // Check if divisor is a vector, and update divisorElemType, isIntegerOverflowType
      if (divisorType->isVectorTy()) {
        FixedVectorType* vecType = static_cast<FixedVectorType *>(divisorType);
        divisorElemType = vecType->getElementType();
        // Create vector type of i1 to represent a vector of booleans
        isIntegerOverflowType = FixedVectorType::get(isIntegerOverflowType, vecType->getNumElements());
      }

      (void)divisorElemType;
      assert(divisorElemType->isIntegerTy() && "Unexpected divisor element type");

      // Get dividend
      Value* dividend = divInst->getOperand(DIVIDEND_POSITION);

      // Create a false value (or false vector) that will correspond to the divisor type
      Value* isIntegerOverflow = ConstantInt::getFalse(isIntegerOverflowType);

      // We need to handle integer overflow only for signed integers
      if ((divInst->getOpcode() == Instruction::SDiv) || (divInst->getOpcode() == Instruction::SRem)) {

        // Create integer -1 and MIN_INT constants (MIN_INT is based on the size of the current int)
        Constant* negOne   = ConstantInt::get(divisorType, -1);                                                              // Creates vector constant if divisorType is vectorType
        Constant* minInt   = ConstantInt::get(divisorType, APInt::getSignedMinValue(divisorType->getScalarSizeInBits()));    // Creates vector constant if divisorType is vectorType

        // %isDivisorNegOne = cmp %divisor, %negOne
        Value *isDivisorNegOne =
            Builder.CreateICmpEQ(divisor, negOne, "isDivisorNegOne");

        // %isDividendMinInt = cmp %dividend, %minInt
        Value *isDividendMinInt =
            Builder.CreateICmpEQ(dividend, minInt, "isDividendMinInt");

        // Update isIntegerOverflow (whihc is currently flase) with the
        // comparison result %isIntegerOverflow   =   and %isDivisorNegOne,
        // %isDividendMinInt
        isIntegerOverflow = Builder.CreateAnd(isDivisorNegOne, isDividendMinInt,
                                              "isIntegerOverflow");
      }

      // Creare integer 0 and 1 constants
      Constant *zero = ConstantInt::get(
          divisorType,
          0); // Creates vector constant if divisorType is vectorType
      Constant *one = ConstantInt::get(
          divisorType,
          1); // Creates vector constant if divisorType is vectorType

      // %isDivisorZero = cmp %divisor, %zero
      Value *isDivisorZero =
          Builder.CreateICmpEQ(divisor, zero, "isDivisorZero");

      // %isDivisorBad        =   or %isIntegerOverflow, %isDivisorZero
      Value *isDivisorBad =
          Builder.CreateOr(isIntegerOverflow, isDivisorZero, "isDivisorBad");

      // %newdivisor = select %isDivisorBad, %one, %divisor
      Value *newDivisor =
          Builder.CreateSelect(isDivisorBad, one, divisor, "newDivisor");

      // Replace original divisor
      divInst->setOperand(DIVISOR_POSITION, newDivisor);

      if (EyeQDivCrashBehavior) {
        // For EyeQ devices: %newDividend = select i1 %isDivisorZero, %zero,
        // %dividend
        Value *newDividend =
            Builder.CreateSelect(isDivisorZero, zero, dividend, "NewDividend");
        // Replace original dividend
        divInst->setOperand(DIVIDEND_POSITION, newDividend);
      }
    }

    return true;
  }
} // namespace intel
