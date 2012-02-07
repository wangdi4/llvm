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

File Name:  PreventDivisionCrashes.h

\*****************************************************************************/

#ifndef __PREVENT_DEVISION_CRASHES_H__
#define __PREVENT_DEVISION_CRASHES_H__

#include "llvm/Pass.h"
#include "llvm/InstrTypes.h"

#include <set>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  using namespace llvm;

  /// @brief  PreventDivisionCrashes class adds dynamic checks that make sure the divisor in
  ///         div and rem instructions is not 0 and that there is no integer overflow (MIN_INT/-1). 
  ///         In case the divisor is 0, PreventDivisionCrashes or there is integer overflow the pass
  ///         replaces the divisor with 1.
  ///         PreventDivisionCrashes is intendent to prevent crashes during division.
  /// @Author Marina Yatsina
  class PreventDivisionCrashes : public FunctionPass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;
    
    // Constructor
    PreventDivisionCrashes() : FunctionPass(ID) {}

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

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __PREVENT_DEVISION_CRASHES_H__
