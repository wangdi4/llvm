/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Main.h"
#include "Mangler.h"
#include "VectorizerCore.h"
#include "MetaDataApi.h"
#include "OclTune.h"
#include "OCLPassSupport.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"

#include <sstream>

using namespace std;

// Placeholders for debug log files
FILE * prtFile;
FILE * moduleDmp;

char intel::Vectorizer::ID = 0;

extern "C" FunctionPass* createVectorizerCorePass(const intel::OptimizerConfig*);
extern "C" Pass* createBuiltinLibInfoPass(llvm::Module* pRTModule, std::string type);

using namespace intel;
OCL_INITIALIZE_PASS_BEGIN(Vectorizer, "vpo-vectorize", "vpo vectorizer", false, false)
OCL_INITIALIZE_PASS_END(Vectorizer, "vpo-vectorize", "vpo vectorizer", false, false)
//static RegisterPass<Vectorizer> clerk("vpo-vectorize", "vpo vectorizer", false, false);

namespace intel {

using namespace llvm;

static Intel::ECPU CPU = Intel::CPU_HASWELL;
static unsigned int cpuFeatures =
  Intel::CFS_SSE2 |
  Intel::CFS_SSE3 |
  Intel::CFS_SSSE3 |
  Intel::CFS_SSE41 |
  Intel::CFS_SSE42 |
  Intel::CFS_AVX1 |
  Intel::CFS_AVX2;
static bool is64BitOS = true;
static Intel::CPUId cpuId(CPU, cpuFeatures, is64BitOS);

static int transposeSize = 0; // auto

static std::vector<int> IRDumpAfter;
static std::vector<int> IRDumpBefore;
static std::string IRDumpDir;
static bool debugInfo = false; // -g
static bool profiling = false; // -profiling
static bool disableOpt = false; // -cl-opt-disable
static bool relaxedMath = false; // -cl-fast-relaxed-math
static bool libraryModule = false; // -create-library
static bool dumpHeuristicIR = false;
static int APFLevel = 0; // -auto-prefetch-level

static OptimizerConfig defaultOptimizerConfig(cpuId, transposeSize,
					      IRDumpAfter, IRDumpBefore,
					      IRDumpDir, debugInfo,
					      profiling, disableOpt,
					      relaxedMath, libraryModule,
					      dumpHeuristicIR, APFLevel);

Vectorizer::Vectorizer(const Module * rt, const OptimizerConfig* pConfig) :
  ModulePass(ID),
/* xmain */
#if 0
  m_runtimeModule(rt),
#endif
  m_numOfKernels(0),
  m_isModuleVectorized(false),
  m_pConfig(pConfig)
{
  if (m_pConfig == NULL)
    m_pConfig = &defaultOptimizerConfig;
  // init debug prints
  initializeLoopInfoPass(*PassRegistry::getPassRegistry());
  //  createBuiltinLibInfoPass(new Module("empty", *new LLVMContext()), "");
//  initializeVectorizerPass(*PassRegistry::getPassRegistry());
  V_INIT_PRINT;
}

Vectorizer::~Vectorizer()
{
  // Close the debug log elegantly
  V_DESTROY_PRINT;
}

void Vectorizer::createVectorizationStubs(Module& M) {
  IntegerType* i1Type = Type::getInt1Ty(M.getContext());
  IntegerType* i32Type = Type::getInt32Ty(M.getContext());
  // Declare all-{zero,one}
  for (int i = 1; i <= 16; i *= 2) {
    stringstream version;
    Type* argType = i1Type;
    if (i > 1) {
      version << "_v" << i;
      argType = VectorType::get(i1Type, i);
    }
    std::vector<Type*> parameterTypes;
    parameterTypes.push_back(argType);
    FunctionType* funcType = FunctionType::get(i1Type, parameterTypes, false);
    Function* allOneFunc =
      dyn_cast<Function>(M.getOrInsertFunction(Mangler::name_allOne + version.str(),
					       funcType));
    V_ASSERT(allOneFunc && "Function type is incorrect, so dyn_cast failed");
    allOneFunc->addFnAttr(Attribute::NoUnwind);
    allOneFunc->addFnAttr(Attribute::ReadNone);
    m_vectorizationStubs.push_back(allOneFunc);
    Function* allZeroFunc =
      dyn_cast<Function>(M.getOrInsertFunction(Mangler::name_allZero + version.str(),
					       funcType));
    V_ASSERT(allZeroFunc && "Function type is incorrect, so dyn_cast failed");
    allZeroFunc->addFnAttr(Attribute::NoUnwind);
    allZeroFunc->addFnAttr(Attribute::ReadNone);
    m_vectorizationStubs.push_back(allZeroFunc);
  }

  // Declare masked load/store
  vector<Type*> maskedTypes;
  maskedTypes.push_back(Type::getInt32Ty(M.getContext()));
  maskedTypes.push_back(Type::getInt64Ty(M.getContext()));
  maskedTypes.push_back(Type::getFloatTy(M.getContext()));
  maskedTypes.push_back(Type::getDoubleTy(M.getContext()));
  vector<Type*>::iterator it, end;
  for (it = maskedTypes.begin(), end = maskedTypes.end(); it != end; it++) {
    for (int i = 2; i <= 16; i *= 2) {
      // Create the masked load function
      VectorType* valueType = VectorType::get(*it, i);
      std::vector<Type*> loadParameterTypes;
      loadParameterTypes.push_back(VectorType::get(*it, i)->getPointerTo());
      loadParameterTypes.push_back(VectorType::get(i32Type, i));
      FunctionType* loadFuncType = FunctionType::get(valueType,
						     loadParameterTypes,
						     false);
      string loadFuncName = Mangler::getMaskedLoadStoreBuiltinName(true, valueType);
      Function* loadFunc =
	dyn_cast<Function>(M.getOrInsertFunction(loadFuncName, loadFuncType));
      V_ASSERT(loadFunc && "Function type is incorrect, so dyn_cast failed");
      loadFunc->addFnAttr(Attribute::NoUnwind);
      m_vectorizationStubs.push_back(loadFunc);

      // Create the masked store function
      std::vector<Type*> storeParameterTypes;
      storeParameterTypes.push_back(VectorType::get(*it, i)->getPointerTo());
      storeParameterTypes.push_back(valueType);
      storeParameterTypes.push_back(VectorType::get(i32Type, i));
      FunctionType* storeFuncType = FunctionType::get(Type::getVoidTy(M.getContext()),
						      storeParameterTypes,
						      false);
      string storeFuncName = Mangler::getMaskedLoadStoreBuiltinName(false, valueType);
      Function* storeFunc =
	dyn_cast<Function>(M.getOrInsertFunction(storeFuncName, storeFuncType));
      V_ASSERT(storeFunc && "Function type is incorrect, so dyn_cast failed");
      storeFunc->addFnAttr(Attribute::NoUnwind);
      m_vectorizationStubs.push_back(storeFunc);
    }
  }
}

void Vectorizer::deleteVectorizationStubs() {
  VectorizationStubsVector::iterator it = m_vectorizationStubs.begin();
  VectorizationStubsVector::iterator end = m_vectorizationStubs.end();
  for (; it != end; it++)
    (*it)->eraseFromParent();
}

string guessVectorVariant(Function& F) {
  stringstream sst;
  sst << "_ZGV";
  sst << 'Y'; // AVX2
  sst << 'N'; // No mask
  sst << '8'; // vlen
  const Function::ArgumentListType& arguments = F.getArgumentList();
  Function::ArgumentListType::const_iterator argIt = arguments.begin();
  Function::ArgumentListType::const_iterator argEnd = arguments.end();
  for (; argIt != argEnd; argIt++) {
    const Value& argument = *argIt;
    StringRef argName = argument.getName();
    if (argName[0] == 'u')
      sst << 'u';
    else if (argName[0] == 'l')
      sst << 'l';
    else
      sst << 'v';
  }
  sst << "_";
  return sst.str();
}

Function* Vectorizer::createVectorVersion(Function& vectorizedFunction,
					  VectorVariant& vectorVariant,
					  std::string scalarFuncName) {
  // Create a new function type with vector types for the RANDOM parameters
  FunctionType* originalFunctionType = vectorizedFunction.getFunctionType();
  Type* originalReturnType = originalFunctionType->getReturnType();
  Type* vectorReturnType;
  if (originalReturnType->isVoidTy())
    vectorReturnType = originalReturnType;
  else
    vectorReturnType = VectorType::get(originalFunctionType->getReturnType(),
				       vectorVariant.getVlen());
  std::vector<VectorKind> parameterKinds = vectorVariant.getParameters();
  vector<Type*> parameterTypes;
  assert(originalFunctionType->getNumParams() == parameterKinds.size());
  FunctionType::param_iterator it = originalFunctionType->param_begin();
  FunctionType::param_iterator end = originalFunctionType->param_end();
  std::vector<VectorKind>::iterator vkIt = parameterKinds.begin();
  for (; it != end; it++, vkIt++) {
    if (vkIt->isVector())
      parameterTypes.push_back(VectorType::get(*it, vectorVariant.getVlen()));
    else
      parameterTypes.push_back(*it);
  }
  FunctionType* vectorFunctionType = FunctionType::get(vectorReturnType,
						       parameterTypes,
						       false);
  std::string name = vectorVariant.encode() + scalarFuncName;
  Function* wrapperFunc = Function::Create(vectorFunctionType,
					   vectorizedFunction.getLinkage(),
					   name,
					   vectorizedFunction.getParent());
  wrapperFunc->copyAttributesFrom(&vectorizedFunction);
  BasicBlock* entryBB = BasicBlock::Create(wrapperFunc->getContext(),
					   "wrapper.entry",
					   wrapperFunc);

  std::vector<ExtractElementInst*> extractedValues;
  std::vector<Instruction*> removedInstructions;

  // Generate a call to the vectorized function and return its value
  Value* zero = ConstantInt::get(Type::getInt32Ty(wrapperFunc->getContext()), 0);
  Function::arg_iterator argIt = wrapperFunc->arg_begin();
  Function::arg_iterator argEnd = wrapperFunc->arg_end();
  Function::arg_iterator origArgIt = vectorizedFunction.arg_begin();
  std::vector<Value*> callArguments;
  for (vkIt = parameterKinds.begin(); argIt != argEnd; argIt++, origArgIt++, vkIt++) {
    argIt->setName(origArgIt->getName());
    if (vkIt->isVector()) {
      // Create extract element 0 as the fake scalar argument
      ExtractElementInst* fakeScalarArg =
	ExtractElementInst::Create(&*argIt, zero, "fake_arg", entryBB);
      extractedValues.push_back(fakeScalarArg);
      callArguments.push_back(fakeScalarArg);
    }
    else
      callArguments.push_back(&*argIt);
  }
  CallInst* call = CallInst::Create(&vectorizedFunction,
				    callArguments,
				    (vectorReturnType->isVoidTy() ? "" : "scalar_ret_val"),
				    entryBB);

  if (vectorReturnType->isVoidTy())
    ReturnInst::Create(wrapperFunc->getContext(), entryBB);
  else
  {
    // Return the function's (scalar) return value as a vector
    Value* undefs = UndefValue::get(vectorReturnType);
    Instruction* fakeRetVal =
      InsertElementInst::Create(undefs, call, zero, "fake_ret_val", entryBB);
    ReturnInst::Create(wrapperFunc->getContext(), fakeRetVal, entryBB);
  }

  // Inline the wrapper call
  InlineFunctionInfo ifi;
  bool inlined = llvm::InlineFunction(call, ifi, false);
  assert(inlined && "expected inline to succeed");

  // Fix the vector arguments to be used correctly instead of the fake
  // extract/insert element pair
  std::vector<ExtractElementInst*>::iterator extractedIt = extractedValues.begin();
  std::vector<ExtractElementInst*>::iterator extractedEnd = extractedValues.end();
  for (; extractedIt != extractedEnd; extractedIt++) {
    ExtractElementInst* eei = *extractedIt;
    assert(eei->getNumUses() == 1 && "expected exactly 1 use for extract-element");
    User* eeiUser = eei->user_back();
    InsertElementInst* iei = dyn_cast<InsertElementInst>(eeiUser);
    assert(iei && "expected user to be an insert-element");
    assert(iei->getNumUses() == 1 && "expected exactly 1 use for insert-element");
    User* ieiUser = iei->user_back();
    ShuffleVectorInst* svi = dyn_cast<ShuffleVectorInst>(ieiUser);
    assert(svi && "expected user to be a shuffle-vector");
    svi->replaceAllUsesWith(eei->getVectorOperand());
    removedInstructions.push_back(eei);
    removedInstructions.push_back(iei);
    removedInstructions.push_back(svi);
  }

  if (!vectorReturnType->isVoidTy()) {
    // Fix the return value to use the actual vector result instead of the fake
    // extract/insert element pair
    Function::iterator bbIt = wrapperFunc->begin(), bbEnd = wrapperFunc->end();
    for (; bbIt != bbEnd; bbIt++) {
      TerminatorInst* terminator = bbIt->getTerminator();
      ReturnInst* returnInst = dyn_cast<ReturnInst>(terminator);
      if (!returnInst)
	continue;
      InsertElementInst* iei = dyn_cast<InsertElementInst>(returnInst->getReturnValue());
      assert(iei && "expected return value to be an insert-element");
      assert(iei->getNumUses() == 1 && "expected exactly 1 use for insert-element");
      ExtractElementInst* eei = dyn_cast<ExtractElementInst>(iei->getOperand(1));
      assert(eei && "expected fake scalar return value to be extract-element");
      Value* actualVectorRetVal = eei->getVectorOperand();
      eei->replaceAllUsesWith(actualVectorRetVal);
      removedInstructions.push_back(iei);
      removedInstructions.push_back(eei);
      break; // there can be only one
    }
  }

  // Erase all fake values
  std::vector<Instruction*>::iterator removedIt = removedInstructions.begin();
  std::vector<Instruction*>::iterator removedEnd = removedInstructions.end();
  for (; removedIt != removedEnd; removedIt++) {
    (*removedIt)->replaceAllUsesWith(UndefValue::get((*removedIt)->getType()));
    (*removedIt)->eraseFromParent();
  }

  return wrapperFunc;
}

bool Vectorizer::runOnModule(Module &M)
{
  V_PRINT(wrapper, "\nEntered Vectorizer Wrapper!\n");
  // set isVectorized and proper number of kernels to zero, in case vectorization fails
  m_numOfKernels = 0;
  m_isModuleVectorized = true;

  Intel::MetaDataUtils mdUtils(&M);

  // check for some common module errors, before actually diving in
  if (mdUtils.empty_Kernels())
  {
    V_PRINT(wrapper, "Failed to find annotation. Aborting!\n");
    return false;
  }
  m_numOfKernels = mdUtils.size_Kernels();
  if (m_numOfKernels == 0)
  {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return false;
  }
/* xmain */
  createVectorizationStubs(M);
#if 0
  if (!m_runtimeModule)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }
#endif


  for (Intel::MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels(), e = mdUtils.end_Kernels(); i != e; ++i) {
    Intel::KernelMetaDataHandle kmd = (*i);
    Function *F = kmd->getFunction();
    bool disableVect = false;

    //look for vector type hint metadata
    if (kmd->isVecTypeHintHasValue()) {
      Type* VTHTy = kmd->getVecTypeHint()->getType();
      if (!VTHTy->isFloatTy()     &&
          !VTHTy->isDoubleTy()    &&
          !VTHTy->isIntegerTy(8)  &&
          !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) &&
          !VTHTy->isIntegerTy(64)) {
            disableVect = true;
      }
    }

    // Only add kernels to list, if they have scalar vec-type hint (or none)
    if (!disableVect)
      m_scalarFuncsList.push_back(F);
    else
      m_scalarFuncsList.push_back(NULL);
  }


  // Create the vectorizer core pass that will do the vectotrization work.
  VectorizerCore *vectCore = (VectorizerCore *)createVectorizerCorePass(m_pConfig);
  FunctionPassManager vectPM(&M);
  Module* builtinModule = &M;
//  Module* builtinModule = getAnalysis<BuiltinLibInfo>().getBuiltinModule();
  vectPM.add(createBuiltinLibInfoPass(builtinModule, ""));
  vectPM.add(vectCore);


  funcsVector::iterator fi = m_scalarFuncsList.begin();
  funcsVector::iterator fe = m_scalarFuncsList.end();
  for (; fi != fe; ++fi)
  {
    // default values for non vectorized kernels.
    Function *vectFunc = 0;
    int vectFuncWidth = 1;
    unsigned int vectDim = 0;
    bool canUniteWorkgroups = false;

    if (*fi) {
      // Clone the kernel
      ValueToValueMapTy vmap;
      Function *clone = CloneFunction(*fi, vmap, true, NULL);
      VectorVariant vectorVariant(guessVectorVariant(**fi));
      clone->setName(vectorVariant.encode() + "_Vectorized_." + (*fi)->getName());
      M.getFunctionList().push_back(clone);

      // Todo: due to a bug in the metadata we can't save changes more than once
      // (even if we reinstantiate the metadata object after saving).
      // Until this is fixed, we send the scalar function directly to the vectorizer core.
      //Intel::KernelInfoMetaDataHandle vkimd = mdUtils.getOrInsertKernelsInfoItem(clone);
      //vkimd->setVectorizedKernel(NULL);
      //vkimd->setScalarizedKernel(*fi);
      //Save Metadata to the module
      //mdUtils.save(M.getContext());

      vectCore->setScalarFunc(*fi);

      vectPM.run(*clone);
      if (vectCore->isFunctionVectorized()) {
        // if the function is successfully vectorized update vectFunc and width.
        vectFuncWidth = vectCore->getPacketWidth();
        vectDim = vectCore->getVectorizationDim();
        canUniteWorkgroups = vectCore->getCanUniteWorkgroups();
	// Create the final version version with the corrent signature
        vectFunc = createVectorVersion(*clone, vectorVariant, (*fi)->getName().str());
        // copy stats from the original function to the new one
        intel::Statistic::copyFunctionStats(**fi, *vectFunc);
      } else {
        // We can't or choose not to vectorize the kernel, erase the clone from the module.
        // but first copy the vectorizer stats back to the original function
        intel::Statistic::copyFunctionStats(*clone, **fi);
        intel::Statistic::removeFunctionStats(*clone);
        clone->eraseFromParent();
      }
      V_ASSERT(vectFuncWidth > 0 && "vect width for non vectoized kernels should be 1");
      //Initialize scalar kernel information, which contains:
      // * pointer to vectorized kernel
      // * vectorized width of 1 (as it is the scalar version)
      // * NULL as pointer to scalar version (as there is no scalar version for scalar kernel)
      Intel::KernelInfoMetaDataHandle skimd = mdUtils.getOrInsertKernelsInfoItem(*fi);
      skimd->setVectorizedKernel(vectFunc);
      skimd->setVectorizedWidth(1);
      skimd->setScalarizedKernel(NULL);
      if (vectFunc) {
        //Initialize vector kernel information
        // * NULL pointer to vectorized kernel (as there is no vectorized version for vectroized kernel)
        // * vectorized width
        // * pointer to scalar version
        Intel::KernelInfoMetaDataHandle vkimd = mdUtils.getOrInsertKernelsInfoItem(vectFunc);
        vkimd->setVectorizedKernel(NULL);
        vkimd->setVectorizedWidth(vectFuncWidth);
        vkimd->setScalarizedKernel(*fi);
        vkimd->setVectorizationDimension(vectDim);
        vkimd->setCanUniteWorkgroups(canUniteWorkgroups);
      }
    }
  }

  //Save Metadata to the module
  mdUtils.save(M.getContext());

  V_DUMP_MODULE((&M));
  //////////////////////////////////////////////
  //////////////////////////////////////////////
  V_PRINT(wrapper, "\nCompleted Vectorizer Wrapper!\n");

  return m_isModuleVectorized;
}

} // Namespace intel

namespace llvm {
void initializeVPOVectorizer(PassRegistry &Registry) {
  initializeVectorizerPass(Registry);
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Interface functions for vectorizer
///////////////////////////////////////////////////////////////////////////////////////////////////
extern "C"
  Pass *createVectorizerPass(const Module *runtimeModule, const intel::OptimizerConfig* pConfig)
{
  return new intel::Vectorizer(runtimeModule, pConfig);
}

