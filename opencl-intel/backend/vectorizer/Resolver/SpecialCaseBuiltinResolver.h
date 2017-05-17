/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __SPECIAL_CASE_BUILTIN_RESOLVER_H__
#define __SPECIAL_CASE_BUILTIN_RESOLVER_H__

#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "Logger.h"
#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LegacyPassManager.h" 
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;


namespace intel {

  typedef SmallVector<Value *, 16> ArgsVector;

  struct retAttr{
    bool isArrayOfVec;          // true iff return is array of vectors [n x <m x T>]
    bool isVoid;                // true iff return is void
    unsigned nVals;             // number of vals returned [n x <m x T>]=n \ <m x T>=1 \ void = 0
    ArrayType *arrType;   // if isArrayOfVec the type otherwise NULL
    Type *elType;         // the basic type returned [n x <m x T>]=<m x T> \ <m x T>=<m x T> \ void=NULL
    unsigned vecWidth;          // if isArrayOfVec [n x <m x T>]=m  otherwise 0(unused)
  };

/// @brief
/// This class implements reolver for built-ins that were replaced with fake function
/// for the purpose of vectorization. it works by implementing the fake function
/// declaration as wrappers that call the true built-in and then inline them.
/// This Pass assumes runtime services of type OpenclRuntime
class SpecialCaseBuiltinResolver : public ModulePass {

public:

  static char ID;
  /// @brief C'tor
  SpecialCaseBuiltinResolver();
  /// @brief D'tor
  ~SpecialCaseBuiltinResolver();
  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "Special Case Builtin Resolver";
  }

  virtual bool runOnModule(Module &M);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<BuiltinLibInfo>();
  }

private:

  ///@brief main function. Takes the fake function declaraion and implements it as
  /// as a wrapper that calls the true corresponding built-in.
  ///@param F - fake function to be implemented as wrapper
  ///@param funcName - name of builtin to resolve
  void fillWrapper(Function *F, std::string& funcName);

  ///@brief obtain resolved built-in arguments (not return pointers) by spreading array of vectors
  /// and casting if needed
  ///@param F - wrapper function
  ///@param resolvedFuncType - resolved builtin function type
  ///@param loc - location to insert casting \ extract value instructions
  ///@param resolvedArgs - argumetns vector for resolved built-in
  void obtainArguments(Function *F, const FunctionType *resolvedFuncType,
                     Instruction *loc, ArgsVector &resolvedArgs);

  ///@brief utility function to get relevant attributes of wrraper return type
  ///@param theType - type to inspect
  void obtainRetAttrs(Type *theType);

  ///@brief allocates pointers to hold return valus and add them to args vector
  ///@param resolvedArgs - argumetns vector for resolved built-in
  ///@param loc - location to insert alloca's
  void addRetPtrToArgsVec(ArgsVector &resolvedArgs, Instruction *loc);

  ///@brief load values from pointers argumets that hold return value
  ///@param resolvedArgs - argumetns vector for resolved built-in
  ///@param loc - location to insert loads (and insertvalue incase of array of vectors return)
  ///@return wrapper return value
  Value *obtainReturnValueArgsPtr(ArgsVector &resolvedArgs, Instruction *loc);

  ///@brief obtain array of vector return from gathered vector
  ///@param CI - resolved built-in call (the big vector)
  ///@param loc - location to insert shuffles and insertvalue
  ///@return wrapper return value
  Value *obtainReturnValueGatheredVector(CallInst *CI, Instruction *loc);

  ///@brief - appleOCLRuntime interface for getting builtins attributes.
  const OpenclRuntime *m_runtimeServices;

  ///@brief holds kernels that call the fake builtin implemented
  SmallPtrSet<Function*, 8> m_changedKernels;

  ///@brief current module to be transformed
  Module *m_curModule;

  ///@brief return attributes of the wrapper
  retAttr m_wrraperRetAttr;

};// SpecialCaseBuiltinResolver

} // namespace intel


#endif //__SPECIAL_CASE_BUILTIN_RESOLVER_H__
