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

#ifndef __PREVENT_DIV_CRASHES_H__
#define __PREVENT_DIV_CRASHES_H__

#include "BuiltinLibInfo.h"

#include "llvm/Pass.h"
#include "llvm/IR/InstrTypes.h"

#include <vector>

namespace intel {

  using namespace llvm;

  /// @brief  PreventDivCrashes class adds dynamic checks that make sure the divisor in
  ///         div and rem instructions is not 0 and that there is no integer overflow (MIN_INT/-1).
  ///         In case the divisor is 0, PreventDivCrashes or there is integer overflow the pass
  ///         replaces the divisor with 1.
  ///         PreventDivCrashes is intended to prevent crashes during division.
  class PreventDivCrashes : public FunctionPass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    // Constructor
    PreventDivCrashes() : FunctionPass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
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

  /// @brief  OptimizeIDiv class replaces integer division and integer remainder
  ///         with call to svml
  class OptimizeIDiv : public FunctionPass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;

    // Constructor
    OptimizeIDiv() : FunctionPass(ID){}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "OptimizeIDiv";
    }

    /// @brief    LLVM Function pass entry
    /// @param F  Function to transform
    /// @returns  true if changed
    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }
  };
} // namespace intel

#endif // __PREVENT_DIV_CRASHES_H__
