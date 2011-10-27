#include "OCLBuiltinPreVectorizationPass.h"
#include "llvm/Support/InstIterator.h"
#include "VectorizerUtils.h"
#include "llvm/Support/StandardPasses.h"
#include "Mangler.h"

char intel::OCLBuiltinPreVectorizationPass::ID = 0;
namespace intel{

OCLBuiltinPreVectorizationPass::OCLBuiltinPreVectorizationPass():
FunctionPass(ID) {
  m_dce = createDeadCodeEliminationPass();
}

OCLBuiltinPreVectorizationPass::~OCLBuiltinPreVectorizationPass() {
  delete m_dce;
}

bool OCLBuiltinPreVectorizationPass::runOnFunction(Function& F) {
  V_PRINT(prevectorization, "\n\nin pre vectorization, input is:\n----------------------------------------\n" << F);
  m_removedInsts.clear();
  bool changed = false;
  m_curModule = F.getParent();
  m_runtimeServices = (OpenclRuntime *)RuntimeServices::get();
  for ( inst_iterator ii = inst_begin(&F), ie = inst_end(&F); ii != ie; ++ii ) {
    if (CallInst *CI = dyn_cast<CallInst>(&*ii)) {
      std::string funcName = CI->getCalledFunction()->getNameStr();
      if (unsigned opWidth = m_runtimeServices->isInlineDot(funcName)) {
        handleInlineDot(CI, opWidth);
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
    m_dce->runOnFunction(F);
  }

  V_PRINT(prevectorization, "\n\nafter pre vectorization\n----------------------------------------\n" << F);
  return changed;
}

void OCLBuiltinPreVectorizationPass::handleWriteImage(CallInst *CI, std::string &funcName) {
  // write image is special case since we need to break the input coordinates 
  // since they are not passed
  Function *fakeFunc = getOrInsertFakeDeclarationModule(funcName);
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
  const Type *targetImgType = fakeFuncType->getParamType(0);
  fakeFuncArgs[0] = VectorizerUtils::RootInputArgument(origImg, targetImgType, CI);
 
  // handling image coords fake function contains x,y coords separate since they are not 
  // packetized but transfered as multi-scalar value (assuming x consequtive y uniform)
  const Type *i32Ty = Type::getInt32Ty(CI->getContext());
  const Type *coordType = VectorType::get(i32Ty, 2);
  Constant *constZero = ConstantInt::get(i32Ty, 0);
  Constant *constOne = ConstantInt::get(i32Ty, 1);
  Value *origCoords = CI->getArgOperand(1);
  Value *rootCoords = VectorizerUtils::RootInputArgument(origCoords, coordType, CI);
  V_ASSERT(rootCoords && "failed rooting colors");
  if (!rootCoords) return;
  fakeFuncArgs[1] = ExtractElementInst::Create(rootCoords, constZero, "extract.x", CI);
  fakeFuncArgs[2] = ExtractElementInst::Create(rootCoords, constOne, "extract.y", CI);

  // handling colors just root to 4xfloat 
  const Type *colorType = VectorType::get(Type::getFloatTy(CI->getContext()), 4);
  Value *origColors = CI->getArgOperand(2);
  Value *rootColors = VectorizerUtils::RootInputArgument(origColors, colorType, CI);
  V_ASSERT(rootColors && "failed rooting colors");
  if (!rootColors) return;
  fakeFuncArgs[3] = rootColors;

  // Create call to fake read sampler
  CallInst::Create(fakeFunc, fakeFuncArgs.begin(), fakeFuncArgs.end(), "", CI);

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
  Function *fakeFunc = getOrInsertFakeDeclarationModule(funcName);
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
    const Type *rootType =  fakeFuncType->getParamType(i);
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
  CallInst *fakeCall = CallInst::Create(fakeFunc, fakeFuncArgs.begin(), fakeFuncArgs.end(), "fake.func", CI);

  rootRet->replaceAllUsesWith(fakeCall);
 
  m_removedInsts.push_back(CI);
}

Function *OCLBuiltinPreVectorizationPass::getOrInsertFakeDeclarationModule(std::string &funcName) {
  std::string fakeFuncName = Mangler::getFakeBuiltinName(funcName);
  Function *fakeFuncRT = m_runtimeServices->findInRuntimeModule(fakeFuncName);
  V_ASSERT(fakeFuncRT && "fake function was not found in RT module!!!");
  if (!fakeFuncRT) return NULL;

  Constant * funcConst = m_curModule->getOrInsertFunction(fakeFuncName,
	  fakeFuncRT->getFunctionType(), fakeFuncRT->getAttributes());
  V_ASSERT(funcConst && "failed generating function in current module");
  Function *func = dyn_cast<Function>(funcConst);
  V_ASSERT(func && "Function type mismatch, caused a constant expression cast!");
  return func;
}


void OCLBuiltinPreVectorizationPass::handleInlineDot(CallInst* CI, unsigned opWidth) {
  V_PRINT("scalarizer", "Before: "<<*CI->getParent()<<"\n\n");
  V_ASSERT(CI->getNumArgOperands() == 2 && "expect two operands");
  // TODO : why it fails build?
  //V_ASSERT(CI->getType()->isFloatingPointTy() && "expect float\double return");
  if (!CI->getType()->isFloatingPointTy()) return;
  
  const Type *opDesiredType = CI->getType();
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
      scalarsA.push_back(ExtractElementInst::Create(A, constInd, "extract.dot", CI));
      scalarsB.push_back(ExtractElementInst::Create(B, constInd, "extract.dot", CI));
    }
  }

  // dot opaeration is on floating point type
  Instruction::BinaryOps mulKind = Instruction::FMul;
  Instruction::BinaryOps addKind = Instruction::FAdd;
  V_PRINT("scalarizer", "Starting: \n");
  Value* sum = NULL;
  for (unsigned i=0; i<opWidth; i++) {
    V_PRINT("scalarizer", "Iteration #"<<i<<"\n");
    Value* mul =  BinaryOperator::Create(mulKind, scalarsA[i], scalarsB[i], "mul_dot", CI);
    V_PRINT("scalarizer", "Creating sum #"<<i<<"\n");
    if (sum) {
      sum = BinaryOperator::Create(addKind, sum, mul, "sum_dot", CI);
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

static RegisterPass<intel::OCLBuiltinPreVectorizationPass>
CLBltnPreVec("CLBltnPreVec", "prepare ocl builtin for vectoriation");