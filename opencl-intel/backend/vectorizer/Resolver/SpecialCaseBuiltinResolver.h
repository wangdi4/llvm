/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __SPECIAL_CASE_BUILTIN_RESOLVER_H__
#define __SPECIAL_CASE_BUILTIN_RESOLVER_H__

#include "llvm/Pass.h"
#include "Logger.h"
#include "llvm/Type.h"
#include "OpenclRuntime.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace llvm;


namespace intel {

  typedef SmallVector<Value *, 16> ArgsVector;

  typedef struct retAttr{
    bool isArrayOfVec;          // true iff return is array of vectors [n x <m x T>]
    bool isVoid;                // true iff return is void
    unsigned nVals;             // number of vals returned [n x <m x T>]=n \ <m x T>=1 \ void = 0
    const ArrayType *arrType;   // if isArrayOfVec the type otherwise NULL
    const Type *elType;         // the basic type returned [n x <m x T>]=<m x T> \ <m x T>=<m x T> \ void=NULL
    unsigned vecWidth;          // if isArrayOfVec [n x <m x T>]=m  otherwise 0(unused)
  } retAttr;

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
  virtual const char *getPassName() const {
    return "Special Case Builtin Resolver";
  }

  virtual bool runOnModule(Module &M);

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
  void obtainRetAttrs(const Type *theType);

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