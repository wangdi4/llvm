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

#ifndef __OCL_BUILTIN_PRE_VECTORIZATION_PASS_H__
#define __OCL_BUILTIN_PRE_VECTORIZATION_PASS_H__

#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "Logger.h"

#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/PassManager.h"

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
  virtual llvm::StringRef getPassName() const {
    return "OCL Builtin Pre Vectorization Pass";
  }

  virtual bool runOnFunction(Function &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<BuiltinLibInfo>();
  }

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
                                           const AttributeSet &attrs);

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
