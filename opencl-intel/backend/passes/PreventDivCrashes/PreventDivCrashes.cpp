/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "PreventDivCrashes.h"
#include "OCLPassSupport.h"
#include "NameMangleAPI.h"
#include "ParameterType.h"
#include "InitializePasses.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"

extern "C" {
  /// @brief Creates new PreventDivCrashes module pass
  /// @returns new PreventDivCrashes module pass
  void* createPreventDivisionCrashesPass() {
    return new intel::PreventDivCrashes();
  }

  void* createOptimizeIDivPass() {
    return new intel::OptimizeIDiv();
  }
}


namespace intel {

  char PreventDivCrashes::ID = 0;
  char OptimizeIDiv::ID = 0;

/// Register pass to for opt
  OCL_INITIALIZE_PASS(PreventDivCrashes, "prevent-div-crash", "Dynamically handle cases that divisor is zero or there is integer overflow during division", false, false)

  OCL_INITIALIZE_PASS_BEGIN(OptimizeIDiv, "optimize-idiv-irem", "Replace integer divide and integer remainder with call to SVML", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(OptimizeIDiv, "optimize-idiv-irem", "Replace integer divide and integer remainder with call to SVML", false, false)

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

    for (unsigned i=0; i< m_divInstuctions.size(); ++i) {

      BinaryOperator* divInst = m_divInstuctions[i];

      // Extract the context
      LLVMContext& context = divInst->getContext();

      // Extract the divisor and its type
      Value* divisor = divInst->getOperand(DIVISOR_POSITION);
      Type* divisorType = divInst->getType();
      Type* divisorElemType = divisorType;                            // Needed in case operandType is VectorTy
      Type* isIntegerOverflowType = IntegerType::getInt1Ty(context);  // Needed for Integer overflow cases, creates i1 type to represebt a boolean

      // Check if divisor is a vector, and update divisorElemType, isIntegerOverflowType
      if (divisorType->isVectorTy()) {
        VectorType* vecType = static_cast<VectorType *>(divisorType);
        divisorElemType = vecType->getElementType();
        // Create vector type of i1 to represent a vector of booleans
        isIntegerOverflowType = VectorType::get(isIntegerOverflowType, vecType->getNumElements());
      }

      assert(divisorElemType->isIntegerTy() && "Unexpected divisor element type");

      // Create a false value (or false vector) that will correspond to the divisor type
      Value* isIntegerOverflow = ConstantInt::getFalse(isIntegerOverflowType);

      // We need to handle integer overflow only for signed integers
      if ((divInst->getOpcode() == Instruction::SDiv) || (divInst->getOpcode() == Instruction::SRem)) {

        // Get dividend
        Value* dividend = divInst->getOperand(DIVIDEND_POSITION);

        // Create integer -1 and MIN_INT constants (MIN_INT is based on the size of the current int)
        Constant* negOne   = ConstantInt::get(divisorType, -1);                                                              // Creates vector constant if divisorType is vectorType
        Constant* minInt   = ConstantInt::get(divisorType, APInt::getSignedMinValue(divisorType->getScalarSizeInBits()));    // Creates vector constant if divisorType is vectorType

        // %isDivisorNegOne = cmp %divisor, %negOne
        CmpInst* isDivisorNegOne =  CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, divisor, negOne, "isDivisorNegOne", divInst);

        // %isDividendMinInt = cmp %dividend, %minInt
        CmpInst* isDividendMinInt = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, dividend, minInt, "isDividendMinInt", divInst);


        // Update isIntegerOverflow (whihc is currently flase) with the comparison result
        // %isIntegerOverflow   =   and %isDivisorNegOne, %isDividendMinInt
        isIntegerOverflow = BinaryOperator::Create(Instruction::And, isDivisorNegOne, isDividendMinInt, "isIntegerOverflow", divInst);
      }

      // Creare integer 0 and 1 constants
      Constant* zero  = ConstantInt::get(divisorType, 0);    // Creates vector constant if divisorType is vectorType
      Constant* one   = ConstantInt::get(divisorType, 1);    // Creates vector constant if divisorType is vectorType

      // %isDivisorZero = cmp %divisor, %zero
      CmpInst* isDivisorZero = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, divisor, zero, "isDivisorZero", divInst);

      // %isDivisorBad        =   or %isIntegerOverflow, %isDivisorZero
      Value* isDivisorBad = BinaryOperator::Create(Instruction::Or, isIntegerOverflow, isDivisorZero, "isDivisorBad", divInst);

      // %newdivisor = select %isDivisorBad, %one, %divisor
      SelectInst* newDivisor = SelectInst::Create(isDivisorBad, one, divisor, "newiDvisor", divInst);

      // Replace original divisor
      divInst->setOperand(DIVISOR_POSITION, newDivisor);
    }

    return true;
  }

  /// @brief Replace the given instruction with a call to built-in function (if possible)
  /// @returns true if instructin was replaced
  static
  bool ReplaceWithBuiltInCall(BinaryOperator* divInst, const intel::RuntimeServices *rtServices) {

    Type* divisorType = divInst->getType();

    // Performance measurements show that such replacement is good only for vectors
    if (!divisorType->isVectorTy())
      return false;

    Type* divisorElemType = ((VectorType *)divisorType)->getElementType();

    assert(divisorElemType->isIntegerTy() && "Unexpected divisor element type");

    // this optimization exists only for 32 bit integers
    if (((IntegerType*)divisorElemType)->getBitWidth() != 32)
      return false;

    Value* divisor = divInst->getOperand(1);
    // do not optimize constants that may be replaced with shifts in the future
    if (dyn_cast<Constant>(divisor)) {
      Constant *constOp = dyn_cast<Constant>(divisor)->getSplatValue();
      if (constOp) {
        const APInt &constIntVal = cast<ConstantInt>(constOp)->getValue();
        if (constIntVal.isPowerOf2() || (-constIntVal).isPowerOf2())
          return false;
      }
    }
    VectorType* vecType = static_cast<VectorType *>(divisorType);
    reflection::FunctionDescriptor funcDesc;
    funcDesc.width = (reflection::width::V)vecType->getNumElements();
    if ((funcDesc.width != 8) && (funcDesc.width != 16))
      return false;

    reflection::TypePrimitiveEnum primitiveType;
    switch(divInst->getOpcode()) {
      default:
        return false;
      case Instruction::SRem:
        funcDesc.name = "irem";
        primitiveType = reflection::PRIMITIVE_INT;
        break;
      case Instruction::URem:
        funcDesc.name = "urem";
        primitiveType = reflection::PRIMITIVE_UINT;
        break;
      case Instruction::SDiv:
        funcDesc.name = "idiv";
        primitiveType = reflection::PRIMITIVE_INT;
        break;
      case Instruction::UDiv:
        funcDesc.name = "udiv";
        primitiveType = reflection::PRIMITIVE_UINT;
        break;
    }

    reflection::RefParamType scalarTy(new reflection::PrimitiveType(primitiveType));

    reflection::RefParamType vectorTy(new reflection::VectorType(scalarTy, vecType->getNumElements()));
    funcDesc.parameters.push_back(vectorTy);
    funcDesc.parameters.push_back(vectorTy);

    // get name of the built-in function
    std::string funcName = mangle(funcDesc);

    Function* divFuncRT = rtServices->findInRuntimeModule(funcName);
    if (!divFuncRT)
      return false;

    // put function declaration inside current module
    SmallVector<Type *, 2> types;
    types.push_back(divisorType);
    types.push_back(divisorType);
    FunctionType *intr = FunctionType::get(divisorType, types, false);
    llvm::Module* pModule = divInst->getParent()->getParent()->getParent();
    Function* divFuncInModule = cast<Function>(pModule->getOrInsertFunction(funcName.c_str(), intr));

    // Create a call
    SmallVector<Value*, 2> args;
    args.push_back(divInst->getOperand(0));
    args.push_back(divInst->getOperand(1));
    CallInst* newCall = CallInst::Create(divFuncInModule, args, funcName, divInst);
    newCall->setDebugLoc(divInst->getDebugLoc());

    // replace original instruction with call
    divInst->replaceAllUsesWith(newCall);
    divInst->eraseFromParent();
    
    return true;
  }

  bool OptimizeIDiv::runOnFunction(Function &F) {

    std::vector<BinaryOperator*> divInstuctions;
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
        divInstuctions.push_back(inst);

      }
    }

    const RuntimeServices* rtServices =
      getAnalysis<BuiltinLibInfo>().getRuntimeServices();
    bool changed = false;
    for (unsigned i=0; i< divInstuctions.size(); ++i) {

      BinaryOperator* divInst = divInstuctions[i];
      changed |= ReplaceWithBuiltInCall(divInst, rtServices);
    }
    return changed;
  }

} // namespace intel
