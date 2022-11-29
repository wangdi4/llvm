//==------ OptimizeIDivAndIRem.cpp - OptimizeIDivAndIRem pass - C++ -*------==//
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/OptimizeIDivAndIRem.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"

using namespace llvm;

// TODO: move this functionality to InstToFuncCall pass

#define DEBUG_TYPE "dpcpp-kernel-optimize-idiv-and-irem"

PreservedAnalyses OptimizeIDivAndIRemPass::run(Function &F,
                                               FunctionAnalysisManager &FAM) {

  RuntimeService &RTS =
      FAM.getResult<ModuleAnalysisManagerFunctionProxy>(F)
          .getCachedResult<BuiltinLibInfoAnalysis>(*F.getParent())
          ->getRuntimeService();
  if (!runImpl(F, RTS))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

/// @brief Replace the given instruction with a call to built-in function (if
/// possible)
/// @returns true if instructin was replaced
static bool replaceWithBuiltInCall(BinaryOperator *DivInst,
                                   const RuntimeService &RTS) {
  Type *DivisorType = DivInst->getType();

  // Performance measurements show that such replacement is good only for
  // vectors
  if (!DivisorType->isVectorTy())
    return false;

  Type *DivisorElemType =
      (cast<FixedVectorType>(DivisorType))->getElementType();
  assert(DivisorElemType->isIntegerTy() && "Unexpected divisor element type");

  // this optimization exists only for 32 bit integers
  if ((cast<IntegerType>(DivisorElemType))->getBitWidth() != 32)
    return false;

  Value *Divisor = DivInst->getOperand(1);
  // do not optimize constants that may be replaced with shifts in the future
  if (Constant *C = dyn_cast<Constant>(Divisor)) {
    Constant *ConstOp = C->getSplatValue();
    if (ConstOp) {
      const APInt &ConstIntVal = cast<ConstantInt>(ConstOp)->getValue();
      if (ConstIntVal.isPowerOf2() || (-ConstIntVal).isPowerOf2())
        return false;
    }
  }
  FixedVectorType *VecType = cast<FixedVectorType>(DivisorType);
  reflection::FunctionDescriptor FuncDesc;
  FuncDesc.Width = (reflection::width::V)VecType->getNumElements();
  if ((FuncDesc.Width != 8) && (FuncDesc.Width != 16))
    return false;

  reflection::TypePrimitiveEnum PrimitiveType;
  switch (DivInst->getOpcode()) {
  default:
    return false;
  case Instruction::SRem:
    FuncDesc.Name = "irem";
    PrimitiveType = reflection::PRIMITIVE_INT;
    break;
  case Instruction::URem:
    FuncDesc.Name = "urem";
    PrimitiveType = reflection::PRIMITIVE_UINT;
    break;
  case Instruction::SDiv:
    FuncDesc.Name = "idiv";
    PrimitiveType = reflection::PRIMITIVE_INT;
    break;
  case Instruction::UDiv:
    FuncDesc.Name = "udiv";
    PrimitiveType = reflection::PRIMITIVE_UINT;
    break;
  }

  reflection::RefParamType ScalarTy(
      new reflection::PrimitiveType(PrimitiveType));

  reflection::RefParamType VectorTy(
      new reflection::VectorType(ScalarTy, VecType->getNumElements()));
  FuncDesc.Parameters.push_back(VectorTy);
  FuncDesc.Parameters.push_back(VectorTy);

  // get name of the built-in function
  std::string FuncName = NameMangleAPI::mangle(FuncDesc);

  Function *DivFuncRT = RTS.findFunctionInBuiltinModules(FuncName);
  if (!DivFuncRT)
    return false;

  // put function declaration inside current module
  SmallVector<Type *, 2> Types;
  Types.push_back(DivisorType);
  Types.push_back(DivisorType);
  FunctionType *FuncTy = FunctionType::get(DivisorType, Types, false);
  llvm::Module *M = DivInst->getModule();
  Function *DivFuncInModule = cast<Function>(
      M->getOrInsertFunction(FuncName.c_str(), FuncTy).getCallee());

  // Create a call
  SmallVector<Value *, 2> Args;
  Args.push_back(DivInst->getOperand(0));
  Args.push_back(DivInst->getOperand(1));
  CallInst *NewCall =
      CallInst::Create(DivFuncInModule, Args, FuncName, DivInst);
  NewCall->setDebugLoc(DivInst->getDebugLoc());

  LLVM_DEBUG(dbgs() << "Div instruction: " << *DivInst << "  is replaced with "
                    << *NewCall << "\n");

  // replace original instruction with call
  DivInst->replaceAllUsesWith(NewCall);
  DivInst->eraseFromParent();

  return true;
}

bool OptimizeIDivAndIRemPass::runImpl(Function &F, RuntimeService &RTS) {
  std::vector<BinaryOperator *> DivInstructions;
  for (auto &I : instructions(F)) {
    // Div and rem instructions are of type BinaryOperator
    BinaryOperator *Inst = dyn_cast<BinaryOperator>(&I);
    if (!Inst)
      continue;

    Instruction::BinaryOps opcode = Inst->getOpcode();

    // Check if opcode is div or rem
    if ((opcode == Instruction::UDiv) || (opcode == Instruction::SDiv) ||
        (opcode == Instruction::URem) || (opcode == Instruction::SRem)) {
      DivInstructions.push_back(Inst);
    }
  }

  bool Changed = false;
  for (unsigned i = 0; i < DivInstructions.size(); ++i) {
    BinaryOperator *DivInst = DivInstructions[i];
    Changed |= replaceWithBuiltInCall(DivInst, RTS);
  }
  return Changed;
}
