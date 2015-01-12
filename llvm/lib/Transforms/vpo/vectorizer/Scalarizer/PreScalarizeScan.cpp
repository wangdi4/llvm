/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
  llvm::SmallVectorImpl<Value*> &rets = getReturns(rootVals);
  llvm::SmallVectorImpl<Value*> &args = getArgs(rootVals);
  // Look for the called function in the built-in functions hash
  Function *vectorFunc = CI->getCalledFunction();
  std::string vectorFuncName = vectorFunc->getName().str();
  V_PRINT(scalarizer, "\tRoot function scanning for function: " << vectorFuncName << "\n");
  const std::unique_ptr<VectorizerFunction> foundFunction =
    m_rtServices->findBuiltinFunction(vectorFuncName);
  unsigned vectorWidth = 0;
  if (!foundFunction->isNull())
  {
    vectorWidth = foundFunction->getWidth();
  }
  if (vectorWidth <= 1 || !foundFunction->isScalarizable())
  {
    V_PRINT(scalarizer, "\t\tFound scalar function, wrapper, or not found in hash\n");
    return false;
  }
  V_ASSERT(vectorWidth > 1 && "Only scalarize vector functions");

  // Vector function was found in hash. Now find the function prototype of the scalar function
  std::string strScalarFuncName = foundFunction->getVersion(0);
  FunctionType * scalarFuncType;
  AttributeSet funcAttrDummy;
  if(!getScalarizedFunctionType(strScalarFuncName, scalarFuncType, funcAttrDummy)) {
    V_ASSERT(false && "Functions hash mismatch with runtime module!");
    // In release mode - fail scalarizing function call "gracefully"
    return false;
  }

  // Define the "desired" type of vectorized function, by expanding
  // the scalar function's argument types to vector width
  FunctionType *vectorFuncType = vectorFunc->getFunctionType();
  Type *SFRT = scalarFuncType->getReturnType();
  unsigned numInputParams = scalarFuncType->getNumParams();
  V_ASSERT(!SFRT->isVoidTy() && "scalar func expected to have ret val");
  bool isVectorFuncReturnsVoid = vectorFuncType->getReturnType()->isVoidTy();

  std::vector<Type*> desiredArgsTypes;
  Type *desiredRetValType;

  if (SFRT->isVectorTy()) {
    // Handle case: [2 x <4 x float>] sincos_retbyarray(<4 x float> %arg) --> <2 x float> prevec_sincos(float %arg)
    VectorType *SFRTAsVec = cast<VectorType>(SFRT);
    assert(cast<ArrayType>(vectorFuncType->getReturnType())->getNumElements() == SFRTAsVec->getNumElements());
    desiredRetValType = ArrayType::get(VectorType::get(SFRTAsVec->getElementType(), vectorWidth) ,2);
  } else {
    desiredRetValType = VectorType::get(SFRT, vectorWidth);
  }
  unsigned vectorIndex = isVectorFuncReturnsVoid ? 1 : 0;
  for (unsigned scalarIndex = 0; scalarIndex < numInputParams; ++scalarIndex, ++vectorIndex) {
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
    if (SFRT->isVectorTy()) {
      //Sort usages (which are assumed to be all extractvalue's) by index

      rets.resize(cast<VectorType>(SFRT)->getNumElements());
      for (Value::user_iterator ui = CI->user_begin(), ue = CI->user_end(); ui!=ue;
          ++ui) {
        unsigned idx = cast<ExtractValueInst>(*ui)->getIndices()[0];
        V_ASSERT(0 == rets[idx]);
        rets[idx] = *ui;
      }
      V_ASSERT(rets.size() == 2 || rets.size() == 1);
    } else {
      rets.push_back(CI);
    }
    for (unsigned i = 0; i < numInputParams; ++i)
    {
      args.push_back(CI->getArgOperand(i));
    }
    return true;
  }
  V_ASSERT(!SFRT->isVectorTy() &&
          "Should have already handled all cases of scalar functions which return a vector");

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
  rets.push_back(rootRetVal);

  // Second, obtain the input arguments, and fill the rootVals vector
  for (unsigned i = 0; i < numInputParams; ++i)
  {
    Value *rootInputArg = VectorizerUtils::RootInputArgument(
      CI->getArgOperand(callArgumentIndex), desiredArgsTypes[i], CI);
    // If root value was not found, give up on the scanning
    if (!rootInputArg) return false;
    args.push_back(rootInputArg);
    ++callArgumentIndex;
  }
  return true;
}

} // Namespace


