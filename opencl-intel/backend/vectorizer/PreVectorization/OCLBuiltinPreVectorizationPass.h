/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __OCL_BUILTIN_PRE_VECTORIZATION_PASS_H__
#define __OCL_BUILTIN_PRE_VECTORIZATION_PASS_H__

#include "OpenclRuntime.h"
#include "Logger.h"

#include "llvm/Pass.h"
#include "llvm/Type.h"
#include "llvm/PassManager.h"

using namespace llvm;


namespace intel {

///@brief 
/// This pass takes an openCL scalar kernel and prepare it for vectorization
/// it replaces scalar function calls with fake scalar function that are more 
/// suitable for vectorization and root it's arguments and return value
/// This Pass assumes runtime services of type OpenclRuntime
class OCLBuiltinPreVectorizationPass : public FunctionPass {

public:

  static char ID;
  /// @brief C'tor
  OCLBuiltinPreVectorizationPass();
  
  /// @brief D'tor
  ~OCLBuiltinPreVectorizationPass();
  
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "OCL Builtin Pre Vectorization Pass";
  }
  
  virtual bool runOnFunction(Function &M);

#if LLVM_VERSION >= 3425
  /// @brief LLVM interface.
  /// @param AU - usage of analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<TargetLibraryInfo>();
  };
#endif

private:

  /// @brief handle cases of scalar select by either maing sure mask works with MSB
  ///  or by replacing the function with select instruction
  /// @param CI - scalar select call instruction tio handle
  /// @param funcName - name of scalar select
  void handleScalarSelect(CallInst *CI, std::string &funcName);

  /// @brief handle cases of write image by extracting the coord elements
  /// @param CI - scalar write image call instruction tio handle
  /// @param funcName - name of builtin
  void handleWriteImage(CallInst *CI, std::string &funcName);


  void handleInlineDot(CallInst *CI, unsigned opWidth);

  void handleReturnByPtrBuiltin(CallInst *CI, const std::string &funcName);

  /// @brief utility function that inserts declaration to into m_curModule
  /// @param name - name of function to insert
  /// @param fType - FunctionType of function to insert
  /// @param attrs - attributeList of function to insert
  /// @return inserted functuion declaration in curModule
  Function *getOrInsertDeclarationToModule(std::string &name, const FunctionType *fType,
                                           const AttrListPtr &attrs);

  /// @brief utility function that inserts a fake declaration to into m_curModule
  /// @param name - name of function to insert
  /// @return inserted functuion declaration in curModule
  Function *getOrInsertFakeDeclarationToModule(const std::string &name);

  /// @brief main fucntion implements regular function replacement f
  ///  queries the runtimeServices for the fakeFunction and roots all input arguments
  ///   according to fake function type. roots the return value and replace the function
  /// @param CI - call instruction to replace with fake function
  /// @param funcName - name of builtin
  void replaceCallWithFakeFunction(CallInst *CI, std::string &funcName);

  ///@brief - appleOCLRuntime interface for getting builtins attributes.
  const OpenclRuntime *m_runtimeServices;

  ///@brief  holds the module of the processed function
  Module *m_curModule;

  ///@brief hold the instruction marked for removal
  std::vector<Instruction *> m_removedInsts;
  


};// OCLBuiltinPreVectorizationPass

} // namespace intel


#endif //__OCL_BUILTIN_PRE_VECTORIZATION_PASS_H__
