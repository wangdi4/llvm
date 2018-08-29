// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "ShiftZeroUpperBits.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"

extern "C" {
  /// @brief Creates new ShiftZeroUpperBits module pass
  /// @returns new ShiftZeroUpperBits module pass
  void* createShiftZeroUpperBitsPass() {
    return new intel::ShiftZeroUpperBits();
  }
}


namespace intel{

  char ShiftZeroUpperBits::ID = 0;

  OCL_INITIALIZE_PASS(ShiftZeroUpperBits, "shift-ignore-upper-bits", "Dynamically handle vector shifts (shl, ashr) makes sure that shift value is modulo number of bits in the element", false, false)

  static const unsigned int SHIFT_BY_POSITION = 1;

  bool ShiftZeroUpperBits::runOnFunction(Function &F) {
    m_vecShift.clear();

    findmVecShitInstructions(F);
    return handleVecShift();

  }

  void ShiftZeroUpperBits::findmVecShitInstructions(Function &F) {

    // Go over all instructiuons in the function
    // Add vector shl and ashr instruction to m_vecShift
    for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {

      // Div and rem instructions are of type BinaryOperator
      BinaryOperator* inst = dyn_cast<BinaryOperator>(&*i);
      if (!inst) continue;

      Instruction::BinaryOps opcode = inst->getOpcode();

      // Check if opcode is shl, ashr or lshr and that the instruction works on vectors
      if(((opcode == Instruction::Shl) || (opcode == Instruction::AShr) || (opcode == Instruction::LShr))
          &&  inst->getType()->isVectorTy()) {

        m_vecShift.push_back(inst);

      }
    }
  }

  bool ShiftZeroUpperBits::handleVecShift() {

    // Check if there are possible vector shift instrcutions
    if (m_vecShift.empty()) return false;

    // Go over all possible vector shift instructions, extract the shift value,
    // add an "and" instruction that will zero the upper bits of each element in
    // the shift value based on the vector element size.

    // Example:
    // %shr = ashr <2 x i32> %0, %1
    // Transfers to:
    // %2 = and <2 x i32> %1, <i32 31, i32 31>
    // %shr = ashr <2 x i32> %0, %2

    for(unsigned i=0; i<m_vecShift.size(); ++i) {

      BinaryOperator* shiftInst = m_vecShift[i];

      // Extract the shift value, its type
      Value* shiftBy = shiftInst->getOperand(SHIFT_BY_POSITION);
      Type* shiftType = shiftInst->getType();

      // Extract the maximum number of bits in the shift value vector element
      // For vectors it gets the size of the element
      unsigned int elemSizeInBits = shiftType->getScalarSizeInBits();
      unsigned int maxElemShiftBits = elemSizeInBits - 1;

       // Creates vector constant because shiftType is vectorType
      Constant* zeroUpperBitsVector  = ConstantInt::get(shiftType, maxElemShiftBits);
      // Create and instruction with the constant value that will zero the upper bits of each element in the shift value
      // %newShiftBy = and %shiftBy, %zeroUpperBitsVector
      BinaryOperator* newShiftBy = BinaryOperator::Create(Instruction::And, shiftBy, zeroUpperBitsVector, "", shiftInst);

      // Replace original shift value
      shiftInst->setOperand(SHIFT_BY_POSITION, newShiftBy);

    }

    return true;
  }

} // namespace intel
