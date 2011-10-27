/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "Scalarize.h"
#include "VectorizerUtils.h"

namespace intel {


void ScalarizeFunction::preScalarizeScanFunctions()
{
  V_PRINT(scalarizer, "\t@@@ Pre-scanning function\n");

  // Iterate over the entire function, looking for function calls
  for (inst_iterator sI = inst_begin(m_currFunc); sI != inst_end(m_currFunc); ++sI)
  {
    // Grab the next instruction (if its a CALL)
    CallInst *CI = dyn_cast<CallInst>(&*sI);
    if (!CI) continue;

    // Generate vector for holding function root values.
    funcRootsVect rootVals;

    // CALL instructions are scanned, to find their inputs and output (which may be
    // converted/truncated/etc.. - so we want to find the *root* value - before it was converted).
    if (scanFunctionCall(CI, rootVals))
    {
      // Copy the new root values to the function calls map
      m_scalarizableRootsMap[CI] = rootVals;
    }
  }
  V_PRINT(scalarizer, "\t@@@ Done Pre-scanning function\n");
}


bool ScalarizeFunction::scanFunctionCall(CallInst *CI, funcRootsVect &rootVals)
{
  // Look for the called function in the built-in functions hash 
  Function *vectorFunc = CI->getCalledFunction();
  std::string vectorFuncName = vectorFunc->getName();
  V_PRINT(scalarizer, "\tRoot function scanning for function: " << vectorFuncName << "\n");
  const RuntimeServices::funcEntry foundFunction =
    m_rtServices->findBuiltinFunction(vectorFuncName);
  unsigned vectorWidth = 0;
  if (foundFunction.first)
  {
    vectorWidth = foundFunction.second;
  }
  if (vectorWidth <= 1 || ! foundFunction.first->isScalarizable)
  {
    V_PRINT(scalarizer, "\t\tFound scalar function, wrapper, or not found in hash\n");
    return false;
  }
  V_ASSERT(vectorWidth > 1 && "Only scalarize vector functions");

  // Vector function was found in hash. Now find the function prototype of the scalar function
  const char *scalarFuncName = foundFunction.first->funcs[0];
  const Function *scalarFunc = m_rtServices->findInRuntimeModule(scalarFuncName);
  if (!scalarFunc)
  {
    V_ASSERT(0 && "Functions hash mismatch with runtime module!");
    // In release mode - fail scalarizing function call "gracefully"
    return false;
  }

  // Define the "desired" type of vectorized function, by expanding
  // the scalar function's argument types to vector width
  const FunctionType *scalarFuncType = scalarFunc->getFunctionType();
  const FunctionType *vectorFuncType = vectorFunc->getFunctionType();
  unsigned numInputParams = scalarFuncType->getNumParams();
  V_ASSERT(!scalarFuncType->getReturnType()->isVoidTy() && "scalar func expected to have ret val");
  bool isVectorFuncReturnsVoid = vectorFuncType->getReturnType()->isVoidTy();

  std::vector<const Type*> desiredArgsTypes;
  const Type *desiredRetValType = VectorType::get(scalarFuncType->getReturnType(), vectorWidth);
  unsigned vectorIndex = isVectorFuncReturnsVoid ? 1 : 0;
  for (unsigned scalarIndex = 0; scalarIndex < numInputParams; ++scalarIndex, ++vectorIndex)
  {
    if (scalarFuncType->getParamType(scalarIndex) == vectorFuncType->getParamType(vectorIndex))
    {
      // Same argument type in both scalar and vector functions - means no scalarization needed
      desiredArgsTypes.push_back(scalarFuncType->getParamType(scalarIndex));
    }
    else
    {
      // Different arg type means value requires scalarization (and maybe a cast)
      desiredArgsTypes.push_back(VectorType::get(
        scalarFuncType->getParamType(scalarIndex), vectorWidth));
    }
  }
  FunctionType *desiredFuncType = FunctionType::get(desiredRetValType, desiredArgsTypes, false);

  // Check if desired function type really fits actual vector function type
  if (desiredFuncType == vectorFuncType)
  {
    // No scanning to do. Just fill the output vector and exit
    rootVals.push_back(CI);
    for (unsigned i = 0; i < numInputParams; ++i)
    {
      rootVals.push_back(CI->getArgOperand(i));
    }
    return true;
  }

  // Getting here, the function doesn't have the desired type.
  // Need to scan the code, and find the "root" values (before they were casted)
  // First, obtain the root Return value
  unsigned callArgumentIndex = 0;
  Value *actualRetValue = CI;
  if (isVectorFuncReturnsVoid)
  {
    // vector func return void, meaning return value is passed as first function argument.
    actualRetValue = CI->getArgOperand(callArgumentIndex);
    ++callArgumentIndex;
  }
  Value *rootRetVal = VectorizerUtils::RootReturnValue(actualRetValue, desiredRetValType, CI);
  // If root return value was not found, give up on the scanning
  if (!rootRetVal) return false;
  // add return-value root to rootVals vector
  rootVals.push_back(rootRetVal); 

  // Second, obtain the input arguments, and fill the rootVals vector
  for (unsigned i = 0; i < numInputParams; ++i)
  {
    Value *rootInputArg =  VectorizerUtils::RootInputArgument(
      CI->getArgOperand(callArgumentIndex), desiredArgsTypes[i], CI);
    // If root value was not found, give up on the scanning
    if (!rootInputArg) return false;
    rootVals.push_back(rootInputArg);
    ++callArgumentIndex;
  }
  return true;
}

} // Namespace


