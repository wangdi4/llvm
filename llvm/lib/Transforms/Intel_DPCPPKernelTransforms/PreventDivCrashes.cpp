//==------ PreventDivCrashes.cpp - PreventDivCrashes pass - C++ -*------==//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PreventDivCrashes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-prevent-div-crashes"

// Add command line to specify EyeQ device behavior
llvm::cl::opt<bool> OptEyeQDivCrashBehavior(
    "dpcpp-eyeq-div-crash-behavior", llvm::cl::init(false),
    llvm::cl::desc(
        "The flag indicates that bad integer divisions (e.g. 1/0) should behave"
        "As they would in an EyeQ device"));

namespace llvm {
/// PreventDivCrashesLegacy pass for legacy pass manager.
class PreventDivCrashesLegacy : public FunctionPass {

public:
  PreventDivCrashesLegacy() : FunctionPass(ID) {
    initializePreventDivCrashesLegacyPass(*PassRegistry::getPassRegistry());
  }

  static char ID;

  StringRef getPassName() const override {
    return "Intel DPCPP Kernel PreventDivCrashes Pass";
  }

  bool runOnFunction(Function &F) override { return Impl.runImpl(F); }

private:
  PreventDivCrashesPass Impl;
};
} // namespace llvm

INITIALIZE_PASS(PreventDivCrashesLegacy, DEBUG_TYPE,
                "Dynamically handle cases that divisor is ConstantZero or "
                "there is integer overflow during division",
                false, false)

char PreventDivCrashesLegacy::ID = 0;

PreservedAnalyses PreventDivCrashesPass::run(Function &F,
                                             FunctionAnalysisManager &FAM) {

  if (!runImpl(F))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool PreventDivCrashesPass::runImpl(Function &F) {
  DivInstructions.clear();
  findDivInstructions(F);

  // Check if there are division instructions
  if (DivInstructions.empty())
    return false;

  return handleDiv();
}

void PreventDivCrashesPass::findDivInstructions(Function &F) {
  // Go over all instructiuons in the function
  // Add div and rem instruction to DivInstructions
  for (auto &I : instructions(F)) {
    // Div and rem instructions are of type BinaryOperator
    BinaryOperator *Inst = dyn_cast<BinaryOperator>(&I);
    if (!Inst)
      continue;

    Instruction::BinaryOps Opcode = Inst->getOpcode();
    // Check if opcode is div or rem
    if ((Opcode == Instruction::UDiv) || (Opcode == Instruction::SDiv) ||
        (Opcode == Instruction::URem) || (Opcode == Instruction::SRem)) {
      DivInstructions.push_back(Inst);
      LLVM_DEBUG(dbgs() << "Add div instructions to be handled: " << *Inst
                        << "\n");
    }
  }
}

static const unsigned int DIVIDEND_POSITION = 0;
static const unsigned int DIVISOR_POSITION = 1;

bool PreventDivCrashesPass::handleDiv() {
  // Go over all possible division instructions, extract the divisor and
  // dividend and:
  // 1. Compares divisor to -1 and dividend to MIN_INT.
  //        This prevents integer overflow: Because of 2's-complement
  //        representation the only problematic case is INT_MIN / -1, which
  //        equals INT_MAX + 1.
  // 2. Compares divisor to 0
  //        This prevent division by ConstantZero.
  // If one of the above comparisons is true the function replace the divisor
  // with 1, so that we won't crash.

  // For EyeQ devices (for EyeQ bit exactness) return 0 for division by
  // ConstantZero (Replace dividend by 0 in addition to replacing divisor by 1).

  // Example:
  // %div = sdiv i32 %conv2, %conv

  // Transfers to:
  // %IsDivisorNegOne     =   icmp eq i32 %1, -1
  // %IsDividendMinInt    =   icmp eq i32 %0, -2147483648
  // %IsIntegerOverflow   =   and i1 %IsDivisorNegOne, %IsDividendMinInt
  // %IsDivisorZero       =   icmp eq i32 %1, 0
  // %IsDivisorBad        =   or i1 %IsIntegerOverflow, %IsDivisorZero
  // %NewiDvisor          =   select i1 %IsDivisorBad, i32 1, i32 %1
  // %div                 =   sdiv i32 %0, %NewiDvisor

  // For EyeQ devices, it will transfer to:
  // %IsDivisorNegOne     =   icmp eq i32 %1, -1
  // %IsDividendMinInt    =   icmp eq i32 %0, -2147483648
  // %IsIntegerOverflow   =   and i1 %IsDivisorNegOne, %IsDividendMinInt
  // %IsDivisorZero       =   icmp eq i32 %1, 0
  // %IsDivisorBad        =   or i1 %IsIntegerOverflow, %IsDivisorZero
  // %NewDivisor          =   select i1 %IsDivisorBad, i32 1, i32 %1
  // %NewDividend         =   select i1 %IsDivisorZero, i32 0, i32 %0
  // %div                 =   sdiv i32 %NewDividend, %NewiDvisor

  for (auto *DivInst : DivInstructions) {
    LLVM_DEBUG(dbgs() << "Handle div instruction: " << *DivInst << "\n");
    IRBuilder<> Builder(DivInst);

    // Extract the divisor and its type
    Value *Divisor = DivInst->getOperand(DIVISOR_POSITION);
    Type *DivisorType = DivInst->getType();
    // Needed in case operandType is VectorTy
    Type *DivisorElemType = DivisorType;
    // Needed for Integer overflow cases, create i1 type to represent a boolean
    Type *IsIntegerOverflowType = Builder.getInt1Ty();

    // Check if divisor is a vector, and update divisorElemType,
    // IsIntegerOverflowType
    if (DivisorType->isVectorTy()) {
      FixedVectorType *VecType = cast<FixedVectorType>(DivisorType);
      DivisorElemType = VecType->getElementType();
      // Create vector type of i1 to represent a vector of booleans
      IsIntegerOverflowType = FixedVectorType::get(IsIntegerOverflowType,
                                                   VecType->getNumElements());
    }
    (void)DivisorElemType;
    assert(DivisorElemType->isIntegerTy() && "Unexpected divisor element type");

    // Get dividend
    Value *Dividend = DivInst->getOperand(DIVIDEND_POSITION);

    // Create a false value (or false vector) that will correspond to the
    // divisor type
    Value *IsIntegerOverflow = ConstantInt::getFalse(IsIntegerOverflowType);

    // We need to handle integer overflow only for signed integers
    if ((DivInst->getOpcode() == Instruction::SDiv) ||
        (DivInst->getOpcode() == Instruction::SRem)) {
      // Create integer -1 and MIN_INT constants (MIN_INT is based on the size
      // of the current int)
      // Creates vector constant if DivisorType is vectorType
      Constant *NegOne = ConstantInt::get(DivisorType, -1);
      Constant *MinInt = ConstantInt::get(
          DivisorType,
          APInt::getSignedMinValue(DivisorType->getScalarSizeInBits()));

      // %IsDivisorNegOne = cmp %divisor, %NegOne
      Value *IsDivisorNegOne =
          Builder.CreateICmpEQ(Divisor, NegOne, "IsDivisorNegOne");

      // %IsDividendMinInt = cmp %Dividend, %MinInt
      Value *IsDividendMinInt =
          Builder.CreateICmpEQ(Dividend, MinInt, "IsDividendMinInt");

      // Update IsIntegerOverflow (whihc is currently flase) with the
      // comparison result %IsIntegerOverflow   =   and %IsDivisorNegOne,
      // %IsDividendMinInt
      IsIntegerOverflow = Builder.CreateAnd(IsDivisorNegOne, IsDividendMinInt,
                                            "IsIntegerOverflow");
    }

    // Create integer 0 and 1 constants
    // Creates vector constant if divisorType is vectorType
    Constant *ConstantZero = ConstantInt::get(DivisorType, 0);
    Constant *ConstantOne = ConstantInt::get(DivisorType, 1);

    // %IsDivisorZero = cmp %divisor, %ConstantZero
    Value *IsDivisorZero =
        Builder.CreateICmpEQ(Divisor, ConstantZero, "IsDivisorZero");

    // %IsDivisorBad = or %IsIntegerOverflow, %IsDivisorZero
    Value *IsDivisorBad =
        Builder.CreateOr(IsIntegerOverflow, IsDivisorZero, "IsDivisorBad");

    // %newdivisor = select %IsDivisorBad, %ConstantOne, %divisor
    Value *NewDivisor =
        Builder.CreateSelect(IsDivisorBad, ConstantOne, Divisor, "NewDivisor");

    // Replace original divisor
    DivInst->setOperand(DIVISOR_POSITION, NewDivisor);

    if (OptEyeQDivCrashBehavior) {
      // For EyeQ devices:
      //   %NewDividend = select i1 %IsDivisorZero, %ConstantZero, %Dividend
      Value *NewDividend = Builder.CreateSelect(IsDivisorZero, ConstantZero,
                                                Dividend, "NewDividend");
      // Replace original Dividend
      DivInst->setOperand(DIVIDEND_POSITION, NewDividend);
    }
  }

  return true;
}

FunctionPass *llvm::createPreventDivCrashesLegacyPass() {
  return new PreventDivCrashesLegacy();
}
