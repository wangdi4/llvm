/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __DETECT_FUNCTION_PTRCALLS_H__
#define __DETECT_FUNCTION_PTRCALLS_H__

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
//
// DetectFuncPtrCalls analysis pass detects function pointer calls in module
// Example of function pointer call:   %13 = call i32 %12(i8* %10, i32 3)
//            calls %12 pointer
//
//===----------------------------------------------------------------------===//

namespace intel {
  using namespace llvm;
  
  /// detect pointer calls pass
  struct DetectFuncPtrCalls : public ModulePass {
    static char ID;
    /// ctor
    DetectFuncPtrCalls();
    
    /// parse module
    virtual bool runOnModule(Module &M);
    
    /// analysis does not transform input code
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }

    /// print out results
    virtual void print(raw_ostream &O, const Module *M) const;

    /// @brief Is function pointer call detected 
    /// @return true is there are >0 function pointer calls in Module
    bool hasFuncPtrCalls() const{
      return m_DetectedFuncPtrCall;
    }

    /// @brief obtain functions names with function pointer calls detected
    /// @return true is there are >0 function pointer calls in Module
    const std::vector<std::string>& getFunctionPtrCalls() const{
      return m_funcWithFuncPtrCall;
    }

  private:
    /// function handling
    bool DetectInFunction(Function& F);
    /// flag if function pointer call detected in module
    bool m_DetectedFuncPtrCall;
    /// array of function names with function pointer calls detected
    std::vector<std::string> m_funcWithFuncPtrCall;

  }; /// struct DetectFuncPtrCalls

  /// @brief create pass
  Pass *createDetectFuncPtrCalls();
} // namespace intel 
#endif
