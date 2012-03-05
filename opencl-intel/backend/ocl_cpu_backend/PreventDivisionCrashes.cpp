/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  PreventDivisionCrashes.cpp

\*****************************************************************************/

#include "PreventDivisionCrashes.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/InstIterator.h"

extern "C" {
  /// @brief Creates new PreventDivisionCrashes module pass
  /// @returns new PreventDivisionCrashes module pass  
  void* createPreventDivisionCrashesPass() {
    return new Intel::OpenCL::DeviceBackend::PreventDivisionCrashes();
  }
}

/// Register pass to for opt
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::PreventDivisionCrashes> PreventDivisionCrashesPass("prevent-div-crash", "Dynamically handle cases that divisor is zero or there is integer overflow during division");


namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char PreventDivisionCrashes::ID = 0;
  
  static const unsigned int DIVIDEND_POSITION = 0;
  static const unsigned int DIVISOR_POSITION = 1;

  bool PreventDivisionCrashes::runOnFunction(Function &F) {
    m_divInstuctions.clear();

    findDivInstructions(F);
    return  handleDiv();
  }
  
  void PreventDivisionCrashes::findDivInstructions(Function &F) {
    
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
  
  bool PreventDivisionCrashes::handleDiv() {
  
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

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {