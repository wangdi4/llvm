/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __KERNEL_ANALYSIS_H__
#define __KERNEL_ANALYSIS_H__

#include <set>
#include "llvm/Pass.h"
#include "llvm/Module.h"
using namespace llvm;


namespace intel {
// class KernelAnalysis
//-----------------------------------------------------------------------------
// This class implements a pass that recieves opencl module. and writes to the 
// metadata which kernel will take the early exit - CLWGLoopCreator path, and
// which will take the barrier pass. 
// kernels can take early exit - CLWGLoopCreator if the following conditions 
// are met:
// 1. does not call barrier.
// 2. does not contain variable get***id call (although can be handled)
// 3. does not contain call to kernel or called by another kernel.
// 4. does not contain call to function that use get***id calls. 
// both 3,4 should be eliminated by the inliner.

class KernelAnalysis : public ModulePass {

private:
  
  typedef SmallVector<Function *, 8> FVec;
  typedef std::set<Function *> FSet;

public:
  ///@brief pass identifier.
  static char ID;

  /// @brief C'tor
  KernelAnalysis();
  
  /// @brief D'tor
  ~KernelAnalysis();
  
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "KernelAnalysis";
  }

  ///@brief LLVM interface.
  ///@param M - module to process.
  ///@returns true if the module changed
  virtual bool runOnModule(Module &M);

  ///@brief LLVM interface.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {};

private:

  ///@brief returns true iff the Value is constant int < 3
  ///@param v - value to check.
  ///@returns as above.
  bool isUnsupportedDim(Value *v);

  ///@brief fills the unsupported set with function that call (also indirectly)
  //        barrier.
  void fillBarrierUsersFuncs();

  ///@brief fills the unsupported set with function that have non constant
  ///       dimension get***id calls, or indirect calls to get***id.
  void fillUnsupportedTIDFuncs();

  ///@brief fills the unsupported set with function that have non constant
  ///       dimension get***id calls, or indirect calls to get***id.
  void fillKernelCallers();

  ///@brief fills the users function through call instructions of roots
  ///       (also indirect users) into userFuncs.
  ///@param roots - function to obtain their user functions.
  ///@param userFuncs - set to fill with users of roots
  void fillFuncUsersSet(FSet &roots, FSet &userFuncs);

  ///@brief fills direct users through call of funcs into userFuncs. If a 
  ///       function is introduced into userFuncs for the first time it will
  ///       be inserted into newUsers also.
  ///@param funcs - function to obtain direct users.
  ///@param userFuncs - set of users functions to fills.
  ///@param newUsers - set of newly found users.
  void fillDirectUsers(FSet *funcs, FSet *userFuncs, FSet *newUsers);
  
  ///@brief fills the unsupported set with function that have non constant
  ///       dimension get***id calls, and gets the direct users of the call
  ///       into TIDUsers.
  ///@param name - name of get***id.
  ///@param directTIDUsers - set of direct get***id users.
  void fillUnsupportedTIDFuncs(const char *name, FSet &directTIDUsers);

  ///@brief current module.
  Module *m_M;

  ///@brief opencl kernels.
  FVec m_kernels;

  ///@brief set of unsupported funcs.
  FSet m_unsupportedFunc;

  /// @brief print data collected by the pass on the given module
  /// @param OS stream to print the info regarding the module into
  /// @param M pointer to the Module
  void print(raw_ostream &OS, const Module *M = 0) const;

};// KernelAnalysis

} // namespace intel


#endif //__KERNEL_ANALYSIS_H__
