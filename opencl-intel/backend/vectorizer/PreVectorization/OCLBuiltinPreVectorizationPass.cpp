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
#include "InitializePasses.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Version.h"

namespace intel{

char OCLBuiltinPreVectorizationPass::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(OCLBuiltinPreVectorizationPass, "CLBltnPreVec", "prepare ocl builtin for vectoriation", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(OCLBuiltinPreVectorizationPass, "CLBltnPreVec", "prepare ocl builtin for vectoriation", false, false)

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
  m_runtimeServices = static_cast<OpenclRuntime *>(getAnalysis<BuiltinLibInfo>().getRuntimeServices());
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
// if we can not be sure then we replace the scalar select builtin fake builtin
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
  //%sinval = <4 x float> sincos(<4 x float> %arg, <4 x float>* %cos)
  // -->
  //%sincos = [2 x <4 x float>] sincos_retbyarray(<4 x float> %arg)
  //%sinval = <4 x float> extractvalue %sincos, 0
  //%extractval1 = <4 x float> extractvalue %sincos, 1
  //store <4 x float>* %cos, %extractval1
  const unsigned int argStart = (CI->getType()->isVoidTy()) ? 1 : 0;
  V_ASSERT(CI->getNumArgOperands() == (argStart + 2) && "Function is expected to have exactly two arguments");
  Value* Op0 = VectorizerUtils::RootInputArgumentBySignature(CI->getArgOperand(argStart + 0), 0, CI);
  Value* Op1 = CI->getArgOperand(argStart + 1);
  PointerType* Op1Type = dyn_cast<PointerType>(Op1->getType());
  if (!Op0 || !Op1Type) {
    //Removed the assertion: this could happen with double3 input value. In
    //  this case the built-in accepts the value as byvalue pointer parameter.
    //  However it will first convert the double3* to double4* before store
    //  the input value (that will be converted itself to double4).
    //TODO: When this case being fixed, enable this assertion.
    //V_ASSERT(false && "Failed to root built-in inputs");
    return;
  }
  V_ASSERT(Op1Type->getElementType() == Op0->getType() &&
    "Function must be of form: gentype foo(gentype, gentype*) or void foo(gentype*, gentype, gentype*)");
  // Assuming built-ins of these types: gentype foo(gentype, gentype*) or void foo(gentype*, gentype, gentype*)
  // Thus, we can count on type of first operand as the return type.
  Type *retValType = Op0->getType();
  bool isOriginalFuncRetVector = retValType->isVectorTy();
  //This function has no implementation it's later replaced by the Packetizer
  //by a call to a 'real' function.
  std::string newFuncName = Mangler::getRetByArrayBuiltinName(funcName);
  SmallVector<Value *, 1> args(1, Op0);
  Type* retType = isOriginalFuncRetVector ?
    static_cast<Type*>(ArrayType::get(retValType, 2)) :
    static_cast<Type*>(VectorType::get(retValType, 2));
#if LLVM_VERSION == 3200
  SmallVector<Attributes, 4> attrs;
  attrs.push_back(Attributes::get(CI->getContext(), Attributes::ReadNone));
  attrs.push_back(Attributes::get(CI->getContext(), Attributes::NoUnwind));
#elif LLVM_VERSION == 3425
  SmallVector<Attributes, 4> attrs;
  attrs.push_back(Attribute::ReadNone);
  attrs.push_back(Attribute::NoUnwind);
#else
  SmallVector<Attribute::AttrKind, 4> attrs;
  attrs.push_back(Attribute::ReadNone);
  attrs.push_back(Attribute::NoUnwind);
#endif
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
  Type *retOrigType = (CI->getType()->isVoidTy()) ? 
    (cast<PointerType>(CI->getOperand(0)->getType()))->getElementType() : CI->getType();
  if (extractVals[0]->getType() != retOrigType) {
    //Assuming that original return value is with same size or larger than actual return value
    extractVals[0] = VectorizerUtils::ExtendValToType(extractVals[0], retOrigType, CI);
  }
  //Update first return value usages
  if (CI->getType()->isVoidTy()) {
    new StoreInst(extractVals[0], CI->getOperand(0), CI);
  } else {
    CI->replaceAllUsesWith(extractVals[0]);
    VectorizerUtils::SetDebugLocBy(extractVals[0], CI);
  }
  //Update second return value usages
  new StoreInst(extractVals[1], Op1, CI);
  m_removedInsts.push_back(CI);
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
