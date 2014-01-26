/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __PREVENT_DIV_CRASHES_H__
#define __PREVENT_DIV_CRASHES_H__

#include "llvm/Pass.h"
#include "llvm/IR/InstrTypes.h"

#include <set>

namespace intel {

  using namespace llvm;

  /// @brief  PreventDivCrashes class adds dynamic checks that make sure the divisor in
  ///         div and rem instructions is not 0 and that there is no integer overflow (MIN_INT/-1).
  ///         In case the divisor is 0, PreventDivCrashes or there is integer overflow the pass
  ///         replaces the divisor with 1.
  ///         PreventDivCrashes is intendent to prevent crashes during division.
  class PreventDivCrashes : public FunctionPass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    // Constructor
    PreventDivCrashes() : FunctionPass(ID) {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "PreventDivCrashes";
    }

    /// @brief    LLVM Function pass entry
    /// @param F  Function to transform
    /// @returns  true if changed
    virtual bool runOnFunction(Function &F);

  private:
    /// @brief    Finds all division instructions (div and rem) in F
    /// @param F  Function in which to find division instructions
    void findDivInstructions(Function &F);

    /// @brief    Adds dynamic checks that make sure the divisor in div and rem
    ///           instructions is not 0 and that there is no integer overflow (MIN_INT/-1).
    ///           In case the divisor is 0, PreventDivisionCrashes or there is integer overflow the pass
    ///           replaces the divisor with 1.
    /// @returns  true if changed
    bool handleDiv();

  private:
    /// The division instructions (div, rem) in the function
    std::vector<BinaryOperator*> m_divInstuctions;

  };

} // namespace intel

#endif // __PREVENT_DIV_CRASHES_H__
