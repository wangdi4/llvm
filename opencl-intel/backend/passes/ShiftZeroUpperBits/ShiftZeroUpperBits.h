/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __SHIFT_ZERO_UPPPER_BITS_H__
#define __SHIFT_ZERO_UPPPER_BITS_H__

#include "llvm/Pass.h"
#include "llvm/IR/InstrTypes.h"

#include <set>

namespace intel{

  using namespace llvm;

  /// @brief  ShiftZeroUpperBits class dynamically adds before shl, arsh of vector types an "and" instruction
  ///         that will zero the upper bits of each element in the shift value,
  ///         based on the size of the vector element.
  ///         This is done in order to be compatiblke with OpenCL spec:
  ///         OpenCL defines behavior for oversized shift values. Shift operations that shift greater
  ///         than or equal to the number of bits in the first operand reduce the shift value modulo the
  ///         number of bits in the element. For example, if we shift an int4 left by 33 bits,
  ///         OpenCL treats this as shift left by 33%32 = 1 bit.
  ///         ShiftZeroUpperBits is intendent to prevent crashes in case the dividor is 0.
  class ShiftZeroUpperBits : public FunctionPass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    // Constructor
    ShiftZeroUpperBits() : FunctionPass(ID) {}

    /// @brief Provides name of pass
    virtual StringRef getPassName() const {
      return "ShiftZeroUpperBits";
    }

    /// @brief    LLVM Function pass entry
    /// @param F  Function to transform
    /// @returns  true if changed
    virtual bool runOnFunction(Function &F);

  private:
    /// @brief    Finds all possible vector shifts instructions (vector shl and ashr) in F
    /// @param F  Function in which to find possible vector shifts
    void findmVecShitInstructions(Function &F);

    /// @brief    Adds dynamic "and" that will zero the upper bits of each element
    ///           in the second operator, based on the size of the vector element.
    /// @returns  true if changed
    bool handleVecShift();

  private:
    /// The possible vector shift instructions (vector shl, ashr) in the function
    std::vector<BinaryOperator*> m_vecShift;

  };

} // namespace intel

#endif // __SHIFT_ZERO_UPPPER_BITS_H__
