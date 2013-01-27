/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "OCLBuiltinPreVectorizationPass.h"
#include "VectorizerUtils.h"
#include "Mangler.h"
#include "OCLPassSupport.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"

namespace intel{

char OCLBuiltinPreVectorizationPass::ID = 0;

OCL_INITIALIZE_PASS(OCLBuiltinPreVectorizationPass, "CLBltnPreVec", "prepare ocl builtin for vectoriation", false, false)

OCLBuiltinPreVectorizationPass::OCLBuiltinPreVectorizationPass():
FunctionPass(ID) {
}

OCLBuiltinPreVectorizationPass::~OCLBuiltinPreVectorizationPass() {
}

bool OCLBuiltinPreVectorizationPass::runOnFunction(Function& F) {
  V_PRINT(prevectorization, "\n\nin pre vectorization, input is:\n----------------------------------------\n" << F);
  m_removedInsts.clear();
  bool changed = false;
  m_curModule = F.getParent();
  m_runtimeServices = (OpenclRuntime *)RuntimeServices::get();
  for ( inst_iterator ii = inst_begin(&F), ie = inst_end(&F); ii != ie; ++ii ) {
    if (CallInst *CI = dyn_cast<CallInst>(&*ii)) {
      std::string funcName = CI->getCalledFunction()->getName().str();
      if (unsigned opWidth = m_runtimeServices->isInlineDot(funcName)) {
        handleInlineDot(CI, opWidth);
        changed = true;
      } else if (m_runtimeServices->isReturnByPtrBuiltin(funcName)) {
        handleReturnByPtrBuiltin(CI, funcName);
        changed = true;
      } else if (m_runtimeServices->isWriteImage(funcName)) {
        handleWriteImage(CI, funcName);
        changed = true;
      } else if (m_runtimeServices->isScalarSelect(funcName)) {
        handleScalarSelect(CI, funcName);
        changed = true;
      } else if (m_runtimeServices->needPreVectorizationFakeFunction(funcName)) {
        replaceCallWithFakeFunction(CI, funcName);
        changed = true;
      }
    }
  }


  // incase we we replaced some function than we erase these instruction - since DCE fails some times
  // and run DCE for removing Instruction that became dead by rooting
  if (changed) {
    std::vector<Instruction *>::iterator it = m_removedInsts.begin();
    std::vector<Instruction *>::iterator e = m_removedInsts.end();
    for (; it!=e; ++it) {
      Instruction *curInst = *it;
      curInst->replaceAllUsesWith(UndefValue::get(curInst->getType()));
      curInst->eraseFromParent();
    }
  }

  V_PRINT(prevectorization, "\n\nafter pre vectorization\n----------------------------------------\n" << F);
  return changed;
}

void OCLBuiltinPreVectorizationPass::handleWriteImage(CallInst *CI, std::string &funcName) {
  // write image is special case since we need to break the input coordinates
  // since they are not passed
  Function *fakeFunc = getOrInsertFakeDeclarationToModule(funcName);
  if (!fakeFunc) return;

  const FunctionType *fakeFuncType = fakeFunc->getFunctionType();

  // some sanity checks
  V_ASSERT (fakeFuncType->getNumParams() == 4 && "unexpected num params in fake write image");
  V_ASSERT (CI->getCalledFunction()->getFunctionType()->getNumParams() == 3 &&
      "unexpected num params in orig write image");
  V_ASSERT(CI->getType()->isVoidTy() && "write image should return void");

  SmallVector<Value *,4> fakeFuncArgs(4);
  // putting image pointer
  Value *origImg = CI->getArgOperand(0);
  Type *targetImgType = fakeFuncType->getParamType(0);
  fakeFuncArgs[0] = VectorizerUtils::RootInputArgument(origImg, targetImgType, CI);

  // handling image coords fake function contains x,y coords separate since they are not
  // packetized but transfered as multi-scalar value (assuming x consequtive y uniform)
  Type *i32Ty = Type::getInt32Ty(CI->getContext());
  Type *coordType = VectorType::get(i32Ty, 2);
  Constant *constZero = ConstantInt::get(i32Ty, 0);
  Constant *constOne = ConstantInt::get(i32Ty, 1);
  Value *origCoords = CI->getArgOperand(1);
  Value *rootCoords = VectorizerUtils::RootInputArgument(origCoords, coordType, CI);
  V_ASSERT(rootCoords && "failed rooting colors");
  if (!rootCoords) return;
  fakeFuncArgs[1] = ExtractElementInst::Create(rootCoords, constZero, "extract.x", CI);
  fakeFuncArgs[2] = ExtractElementInst::Create(rootCoords, constOne, "extract.y", CI);

  // handling colors just root to 4xfloat
  Type *colorType = VectorType::get(Type::getFloatTy(CI->getContext()), 4);
  Value *origColors = CI->getArgOperand(2);
  Value *rootColors = VectorizerUtils::RootInputArgument(origColors, colorType, CI);
  V_ASSERT(rootColors && "failed rooting colors");
  if (!rootColors) return;
  fakeFuncArgs[3] = rootColors;

  // Create call to fake read sampler
  CallInst::Create(fakeFunc, ArrayRef<Value*>(fakeFuncArgs), "", CI);

  m_removedInsts.push_back(CI);
}

// select builtins use integer mask as bool for scalar built-in (any bit set means true)
// but for vector select built-in mask is according to MSB
// so in case the scalar kernel containing scalar select built-in we need to make sure
// the mask has the MSB set, so we replace Zext with Sext.
// if we can not be sure than we replace the scalar select builtin fake builtin
// so it won't be vectorized but duplicated and then resolved
void OCLBuiltinPreVectorizationPass::handleScalarSelect(CallInst *CI, std::string &funcName) {
  Value *inputMaskVal = CI->getArgOperand(2);

  // Check if the root of the mask is a sext or zext instruction for i1 origin value
  Instruction * maskInst = dyn_cast<Instruction>(inputMaskVal);
  if (!maskInst ||
    (!isa<ZExtInst>(maskInst) && !isa<SExtInst>(maskInst)) ||
    maskInst->getOperand(0)->getType() != Type::getInt1Ty(CI->getContext())) {
    V_ASSERT(CI->getType() == CI->getArgOperand(0)->getType() &&
             CI->getType() == CI->getArgOperand(1)->getType() &&
		"select func return value type is not the same as xy operands types");

    std::string fakeFuncName = Mangler::getFakeBuiltinName(funcName);
	Function *origFunc = CI->getCalledFunction();
    Constant * funcConst = m_curModule->getOrInsertFunction(fakeFuncName,
	  origFunc->getFunctionType(), origFunc->getAttributes());
    V_ASSERT(funcConst && "failed generating function in current module");
    Function *fakeFunc = dyn_cast<Function>(funcConst);
    V_ASSERT(fakeFunc && "Function type mismatch, caused a constant expression cast!");
  	CI->setCalledFunction(fakeFunc);
    return;
  }
  if (isa<ZExtInst>(maskInst)) {
    // mask is Zext than we replace it with Sext
    Value * signExtend = new SExtInst(maskInst->getOperand(0), maskInst->getType(), "sign.extend", CI);
    CI->setArgOperand(2, signExtend);
  }
}


void OCLBuiltinPreVectorizationPass::replaceCallWithFakeFunction(CallInst *CI, std::string &funcName) {
  // Find (or create) declaration for newly called function
  Function *fakeFunc = getOrInsertFakeDeclarationToModule(funcName);
  if (!fakeFunc) return;

  V_PRINT(prevectorization, "\nreplacing:    " << *CI << "\nwith:   " << *fakeFunc << "\n");
  const FunctionType *fakeFuncType = fakeFunc->getFunctionType();
  unsigned fakeNumArgs = fakeFuncType->getNumParams();
  unsigned origNumArgs = CI->getNumArgOperands();
  V_ASSERT(fakeNumArgs == origNumArgs && "fake function have different number of arguments");
  if (fakeNumArgs != origNumArgs) return;

  std::vector<Value *> fakeFuncArgs(fakeNumArgs , 0);
  for (unsigned i=0; i<fakeNumArgs; ++i) {
    Value *origArg = CI->getArgOperand(i);
    Type *rootType =  fakeFuncType->getParamType(i);
    V_PRINT(prevectorization, "going to root:  " << *origArg << " into " << *rootType << "\n");
    fakeFuncArgs[i] = VectorizerUtils::RootInputArgument(origArg, rootType, CI);
    V_ASSERT(fakeFuncArgs[i] && "failed rooting argument in pre vectorization");
    if (!fakeFuncArgs[i]) return;
  }

  // currently not supporting void return - void return should be handled separtely
  if (CI->getType()->isVoidTy()) return;
  Value *rootRet = VectorizerUtils::RootReturnValue(CI, fakeFuncType->getReturnType(), CI);
  V_ASSERT(rootRet && "failed rooting return in pre vectorization");
  if (!rootRet) return;

  // Create call to fake read sampler
  CallInst *fakeCall = CallInst::Create(fakeFunc, ArrayRef<Value*>(fakeFuncArgs), "fake.func", CI);

  rootRet->replaceAllUsesWith(fakeCall);

  m_removedInsts.push_back(CI);
}

Function *OCLBuiltinPreVectorizationPass::getOrInsertFakeDeclarationToModule(const std::string &funcName) {
  std::string MangledFuncName = Mangler::getFakeBuiltinName(funcName);
  Function *FuncRT = m_runtimeServices->findInRuntimeModule(MangledFuncName);
  V_ASSERT(FuncRT && "function was not found in RT module!!!");
  if (!FuncRT) return NULL;

  Constant * funcConst = m_curModule->getOrInsertFunction(MangledFuncName,
	  FuncRT->getFunctionType(), FuncRT->getAttributes());
  V_ASSERT(funcConst && "failed generating function in current module");
  Function *func = dyn_cast<Function>(funcConst);
  V_ASSERT(func && "Function type mismatch, caused a constant expression cast!");
  return func;
}

void OCLBuiltinPreVectorizationPass::handleReturnByPtrBuiltin(CallInst* CI, const std::string &funcName) {
  //This optimization does not work yet with LLVM IR generated by Apple's Clang!
  //TODO: remove this #ifndef after fixing the optimization.
#ifndef __APPLE__
  //%sinval = <4 x float> sincos(<4 x float> %arg, <4 x float>* %cos)
  // -->
  //%sincos = [2 x <4 x float>] sincos_retbyarray(<4 x float> %arg)
  //%sinval = <4 x float> extractvalue %sincos, 0
  //%extractval1 = <4 x float> extractvalue %sincos, 1
  //store <4 x float>* %cos, %extractval1
  V_ASSERT(CI->getNumArgOperands() == 2 && "Function is expected to have exactly two arguments");
  Value* Op0 = CI->getArgOperand(0);
  Value* Op1 = CI->getArgOperand(1);
  const Function* originalFunc = CI->getCalledFunction();
  V_ASSERT(cast<PointerType>(Op1->getType())->getElementType() == Op0->getType() &&
      originalFunc->getReturnType() == Op0->getType() &&
      "Function must be of form: gentype foo(gentype, gentype*)");
  bool isOriginalFuncRetVector = originalFunc->getReturnType()->isVectorTy();
  //This function has no implementation it's later replaced by the Packetizer
  //by a call to a 'real' function.
  std::string newFuncName = Mangler::getRetByArrayBuiltinName(funcName);
  SmallVector<Value *, 1> args(1, Op0);
  Type* retType = isOriginalFuncRetVector ?
    static_cast<Type*>(ArrayType::get(originalFunc->getReturnType(), 2)) :
    static_cast<Type*>(VectorType::get(originalFunc->getReturnType(), 2));
  SmallVector<Attributes, 4> attrs;
  attrs.push_back(Attribute::ReadNone);
  attrs.push_back(Attribute::NoUnwind);
  CallInst *newCall = VectorizerUtils::createFunctionCall(m_curModule, newFuncName, retType, args, attrs, CI);    
  V_ASSERT(newCall && "adding function failed");
  SmallVector<Instruction*, 2> extractVals;
  for (unsigned i=0; i < 2; ++i) {
    if (isOriginalFuncRetVector) {
      SmallVector<unsigned, 1> Idx(1, i);
      extractVals.push_back(ExtractValueInst::Create(newCall, Idx, newFuncName + "_extract.", CI));
    } else {
      Value *constIndex = ConstantInt::get(Type::getInt32Ty(CI->getContext()), i);
      extractVals.push_back(ExtractElementInst::Create(newCall, constIndex, newFuncName + "_extract.", CI));
    }
  }
  new StoreInst(extractVals[1], Op1, CI);
  CI->replaceAllUsesWith(extractVals[0]);
  VectorizerUtils::SetDebugLocBy(extractVals[0], CI);
  m_removedInsts.push_back(CI);
#endif
}


void OCLBuiltinPreVectorizationPass::handleInlineDot(CallInst* CI, unsigned opWidth) {
  V_PRINT("scalarizer", "Before: "<<*CI->getParent()<<"\n\n");
  V_ASSERT(CI->getNumArgOperands() == 2 && "expect two operands");
  // TODO : why it fails build?
  //V_ASSERT(CI->getType()->isFloatingPointTy() && "expect float\double return");
  if (!CI->getType()->isFloatingPointTy()) return;

  Type *opDesiredType = CI->getType();
  if(opWidth > 1) opDesiredType = VectorType::get(CI->getType(), opWidth);
  Value *A =  VectorizerUtils::RootInputArgument(CI->getArgOperand(0), opDesiredType, CI);
  Value *B =  VectorizerUtils::RootInputArgument(CI->getArgOperand(1), opDesiredType, CI);
  V_ASSERT(A && B && "failed rooting");
  if (!A || !B) return;

  SmallVector<Value *, 4> scalarsA;
  SmallVector<Value *, 4> scalarsB;
  if (opWidth == 1) {
    scalarsA.push_back(A);
    scalarsB.push_back(B);
  } else {
    for (unsigned i=0; i<opWidth; ++i) {
      Constant *constInd = ConstantInt::get( Type::getInt32Ty(CI->getContext()), i);
      ExtractElementInst *extractA =
        ExtractElementInst::Create(A, constInd, "extract.dot", CI);
      VectorizerUtils::SetDebugLocBy(extractA, CI);
      scalarsA.push_back(extractA);
      ExtractElementInst *extractB =
        ExtractElementInst::Create(B, constInd, "extract.dot", CI);
      VectorizerUtils::SetDebugLocBy(extractB, CI);
      scalarsB.push_back(extractB);
    }
  }

  // dot opaeration is on floating point type
  Instruction::BinaryOps mulKind = Instruction::FMul;
  Instruction::BinaryOps addKind = Instruction::FAdd;
  V_PRINT("scalarizer", "Starting: \n");
  Instruction* sum = NULL;
  for (unsigned i=0; i<opWidth; i++) {
    V_PRINT("scalarizer", "Iteration #"<<i<<"\n");
    Instruction* mul =  BinaryOperator::Create(mulKind, scalarsA[i], scalarsB[i], "mul_dot", CI);
    VectorizerUtils::SetDebugLocBy(mul, CI);
    V_PRINT("scalarizer", "Creating sum #"<<i<<"\n");
    if (sum) {
      sum = BinaryOperator::Create(addKind, sum, mul, "sum_dot", CI);
      VectorizerUtils::SetDebugLocBy(sum, CI);
    } else {
      sum = mul;
    }
  }
  V_ASSERT(sum && "unexpected error sum is NULL");
  CI->replaceAllUsesWith(sum);
  m_removedInsts.push_back(CI);
  V_PRINT("scalarizer", "After: "<<*CI->getParent()<<"\n\n");
  return;
}



} // namespace intel

extern "C"
FunctionPass *createOCLBuiltinPreVectorizationPass() {
  return new intel::OCLBuiltinPreVectorizationPass();
}
