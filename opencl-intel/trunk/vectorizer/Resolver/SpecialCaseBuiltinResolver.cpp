
#include "SpecialCaseBuiltinResolver.h"
#include "llvm/Support/StandardPasses.h"
#include "VectorizerUtils.h"
#include "Mangler.h"

#include <vector>

namespace llvm {
    extern Pass *createFunctionInliningPass(int Threshold);
}


char intel::SpecialCaseBuiltinResolver::ID = 0;
namespace intel {

SpecialCaseBuiltinResolver::SpecialCaseBuiltinResolver():
ModulePass(ID) {
}

SpecialCaseBuiltinResolver::~SpecialCaseBuiltinResolver() {
}

bool SpecialCaseBuiltinResolver::runOnModule(Module &M) {
  m_changedKernels.clear();
  m_curModule = &M;
  m_runtimeServices = (OpenclRuntime *)RuntimeServices::get();
  bool changed = false;
  SmallVector<Function *, 8> fakeFunctions;
  
  for (Module::iterator it = M.begin(), e = M.end(); it!=e; ++it) {
    std::string curFuncName = it->getNameStr();
    if (m_runtimeServices->needSpecialCaseResolving(curFuncName)) {
      fakeFunctions.push_back(it);
      fillWrapper(it , curFuncName);
      changed =true;
    }
  }
  
  if (changed){
    PassManager mpm1;
    // Register inliner
    Pass *inlinerPass = createFunctionInliningPass(4096);
    mpm1.add(inlinerPass);
    mpm1.run(M);

    // removing the inlined \ unused fake functions
    SmallVector<Function *, 8>::iterator it = fakeFunctions.begin();
    SmallVector<Function *, 8>::iterator e  = fakeFunctions.end();
    for (;it != e ; ++it) {
      Function *curFakeFunc = *it;
      if (curFakeFunc->getNumUses() == 0)
        curFakeFunc->eraseFromParent();
      else 
        V_ASSERT(0 && "fake function still has uses after inlining");
    }
  }
  
  // running instcombine on affected kernels
  if (changed) {
    FunctionPassManager fpm(&M);
    fpm.add(createInstructionCombiningPass());
    SmallPtrSet<Function*, 8>::iterator it = m_changedKernels.begin();
    SmallPtrSet<Function*, 8>::iterator e  = m_changedKernels.end();
    for (; it!=e ; ++it)
      fpm.run(**it);
  }
  return changed;
}

void SpecialCaseBuiltinResolver::obtainArguments(Function *F, const FunctionType *resolvedFuncType,
                                               Instruction *loc, ArgsVector &resolvedArgs) {
  Function::arg_iterator argIt = F->arg_begin();
  Function::arg_iterator argE = F->arg_end();
  unsigned resolvedFuncArgInd = 0;
  for ( ;argIt != argE; ++argIt ) {
    const ArrayType *arrType = dyn_cast<ArrayType>(argIt->getType());
    if (arrType) {
      // incase wrapper argument is array of vectors than we extract the vectors 
      // and pass each one separately to the true builtin
      unsigned nElts = arrType->getNumElements();
      for (unsigned i=0; i<nElts; ++i) {
        ExtractValueInst *EVI = ExtractValueInst::Create(argIt, i, "extract_param", loc);
        const Type *desiredType = resolvedFuncType->getParamType(resolvedFuncArgInd);
        Value *curArg = VectorizerUtils::getCastedArgIfNeeded(EVI, desiredType, loc); 
        resolvedArgs.push_back(curArg);
        resolvedFuncArgInd++;
      }
    } else {
      const Type *desiredType = resolvedFuncType->getParamType(resolvedFuncArgInd);
      Value *curArg = VectorizerUtils::getCastedArgIfNeeded(argIt, desiredType, loc);
      resolvedArgs.push_back(curArg);
      resolvedFuncArgInd++;
    }
  }
}

void SpecialCaseBuiltinResolver::addRetPtrToArgsVec(ArgsVector &resolvedArgs, Instruction *loc) {
  for (unsigned i=0; i<m_wrraperRetAttr.nVals; ++i) {
    AllocaInst *AI = new AllocaInst(m_wrraperRetAttr.elType, 0, "retPtr", loc);
    resolvedArgs.push_back(AI);
  }    
}

Value *SpecialCaseBuiltinResolver::obtainReturnValueArgsPtr(ArgsVector &resolvedArgs, Instruction *loc) {
  Value *retVal;
  unsigned retPointerInd = resolvedArgs.size() - m_wrraperRetAttr.nVals;
  if (m_wrraperRetAttr.isArrayOfVec) {
    // assembled to the output into array of vectors and set retVal
    retVal = UndefValue::get(m_wrraperRetAttr.arrType);
    for (unsigned i=0; i<m_wrraperRetAttr.nVals; ++i) {
      LoadInst *LI = new LoadInst(resolvedArgs[retPointerInd + i] , "load_ret" , loc );  
      retVal = InsertValueInst::Create(retVal , LI, i, "ret_agg" , loc);
    }
  } else  {
    // just load the only pointer for output and set retVal
    retVal =  new LoadInst(resolvedArgs[retPointerInd] , "load_ret" , loc );
  }
  return retVal;
}


Value *SpecialCaseBuiltinResolver::obtainReturnValueGatheredVector(CallInst *CI, Instruction *loc) {
  //some sanity checks
  V_ASSERT(CI->getType()->isVectorTy() && "expected vector return from builtin");
  V_ASSERT(m_wrraperRetAttr.isArrayOfVec && "expected array of vectors return by wrraper");
  
  Value *retVal = UndefValue::get(m_wrraperRetAttr.arrType);
  Value *undefVect = UndefValue::get(CI->getType());
  unsigned ind=0; // element to obtain from shuffle
  std::vector<Constant *> maskVec;
  for (unsigned i=0; i<m_wrraperRetAttr.nVals; ++i) {
    maskVec.clear();
    for(unsigned j=0; j<m_wrraperRetAttr.vecWidth; ++j)
      maskVec.push_back(ConstantInt::get(IntegerType::get(CI->getContext(), 32), ind++));
    Constant *mask = ConstantVector::get(maskVec);
    ShuffleVectorInst *SVI = new ShuffleVectorInst(CI, undefVect, mask, "vector.ret.breakdown", loc);
    retVal = InsertValueInst::Create(retVal , SVI, i, "ret_agg" , loc);  
  }
  return retVal;
}



void SpecialCaseBuiltinResolver::fillWrapper(Function *F, std::string& funcName) {
  std::string resolvedName = Mangler::demangle_fake_builtin(funcName);
  // not need to implement wrapper with no uses
  if (F->getNumUses() == 0) return;
  
  BasicBlock *entry = BasicBlock::Create(F->getContext(), "entry" , F);
  // first check if function already found in module
  // this is important incase function contains opaque pointers 
  Function *resolvedFunc = m_curModule->getFunction(resolvedName);
  if (!resolvedFunc)  {
    Function *LibFunc = m_runtimeServices->findInRuntimeModule(resolvedName);
    Constant *resolvedFunctionConst = F->getParent()->getOrInsertFunction(
        LibFunc->getName(), LibFunc->getFunctionType(), LibFunc->getAttributes());
    resolvedFunc = dyn_cast<Function>(resolvedFunctionConst);
  }
  const FunctionType *resolvedFuncType = resolvedFunc->getFunctionType();

  // creating ret in the just to be used as insrtion point for argumnet casting
  const Type *wrapperRetType = F->getReturnType();
  ReturnInst *fakeRet = ReturnInst::Create(F->getContext(), UndefValue::get(wrapperRetType), entry);
  
  // obtain input arguments into the builtin actual type
  ArgsVector resolvedArgs;
  obtainArguments(F, resolvedFuncType, fakeRet, resolvedArgs);

  // obtain return value attributes into m_wrraperRetAttr
  obtainRetAttrs(wrapperRetType);
  const Type *resolvedFuncRetType = resolvedFunc->getReturnType();
  bool retByPtr = resolvedFuncRetType->isVoidTy() && !m_wrraperRetAttr.isVoid;
  bool retByGatheredVec = resolvedFuncRetType->isVectorTy() && m_wrraperRetAttr.isArrayOfVec; 
  
  // if resolved buitin return with pointer allocate pointer and add them as arguments
  if (retByPtr)
    addRetPtrToArgsVec(resolvedArgs, entry->getFirstNonPHI());
  
  // sanity check that all argument have the correct type 
  V_ASSERT(resolvedFuncType->getNumParams() == resolvedArgs.size() &&
      "mismatch with between fake function and true function parameters");
  for (unsigned i=0; i<resolvedArgs.size(); ++i)
    V_ASSERT(resolvedArgs[i]->getType() == resolvedFuncType->getParamType(i) &&
        "resolved argument type mismatch");
  
  // creating call to resolved function
  CallInst *newCall = CallInst::Create(resolvedFunc, resolvedArgs.begin(), resolvedArgs.end(), "" , fakeRet);

  // obtaining return value
  Value *retVal = NULL; // this capture case of void return 
  if (retByPtr) // incase return is by pointer load from them
    retVal = obtainReturnValueArgsPtr(resolvedArgs, fakeRet);
  else if (retByGatheredVec) // incase return is by big gatherd break it to the actual transposed values
    retVal = obtainReturnValueGatheredVector(newCall, fakeRet);
  else if (!m_wrraperRetAttr.isVoid) // getting here we just need to cast the resolved builtin return
    retVal = VectorizerUtils::getCastedRetIfNeeded(newCall, wrapperRetType);

  // no need for fake ret any more 
  fakeRet->eraseFromParent();

  //creating return
  ReturnInst::Create(F->getContext(), retVal, entry);

  // update chenged kernels
  for (Function::use_iterator it = F->use_begin(), e = F->use_end(); it!=e; ++it) {
    CallInst *CI = dyn_cast<CallInst> (*it);
    V_ASSERT(CI && "unexpected use - expect only call uses of wrappers");
    if (!CI) return;
    m_changedKernels.insert(CI->getParent()->getParent());
  }
}

void SpecialCaseBuiltinResolver::obtainRetAttrs(const Type *theType) {
  // first check if void return
  if (theType->isVoidTy()) {
    m_wrraperRetAttr.isVoid = true;
    m_wrraperRetAttr.nVals = 0;
    m_wrraperRetAttr.isArrayOfVec = true;
    return;
  }
  m_wrraperRetAttr.isVoid = false;
  
  // check if multiple return (array of vectors)
  const ArrayType *arrType = dyn_cast<ArrayType>(theType);
  const VectorType *vecType =  arrType ? dyn_cast<VectorType>(arrType->getElementType()) : NULL;
  if (vecType) {
    m_wrraperRetAttr.isArrayOfVec = true;
    m_wrraperRetAttr.arrType = arrType;
    m_wrraperRetAttr.nVals = arrType->getNumElements();
    m_wrraperRetAttr.vecWidth = vecType->getNumElements();
    m_wrraperRetAttr.elType = vecType;
  } else {
    m_wrraperRetAttr.isArrayOfVec = false;
    m_wrraperRetAttr.nVals = 1;
    m_wrraperRetAttr.elType = theType;
  }
}



}// namespace

extern "C"
Pass *createSpecialCaseBuiltinResolverPass() { 
  return new intel::SpecialCaseBuiltinResolver();
}

static RegisterPass<intel::SpecialCaseBuiltinResolver>
CLBltnResolve("CLBltnResolve", "resolve ocl special case builtins");