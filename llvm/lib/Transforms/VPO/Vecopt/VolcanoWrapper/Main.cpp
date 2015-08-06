/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Main.h"
#include "VecConfig.h"
#include "Mangler.h"
#include "WIAnalysis.h"
#include "InstCounter.h"
#ifdef USE_METADATA_API
#include "MetaDataApi.h"
#endif
#include "OclTune.h"
#include "OCLPassSupport.h"
#include "VectorizerUtils.h"
#include "LoopUtils.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Verifier.h"

#include <sstream>

using namespace std;

// Placeholders for debug log files
FILE * prtFile;
FILE * moduleDmp;

char intel::Vectorizer::ID = 0;

extern "C" FunctionPass* createVectorizerCorePass(const intel::OptimizerConfig*);
extern "C" FunctionPass* createPhiCanon();
extern "C" FunctionPass* createPredicator();
extern "C" FunctionPass* createSimplifyGEPPass();
extern "C" FunctionPass* createPacketizerPass(bool, unsigned int);
extern "C" Pass* createBuiltinLibInfoPass(llvm::Module* pRTModule, std::string type);

extern "C" FunctionPass* createGatherScatterResolverPass();
extern "C" FunctionPass* createX86ResolverPass();
extern "C" FunctionPass *createIRPrinterPass(std::string dumpDir, std::string dumpName);


static FunctionPass* createResolverPass(const Intel::CPUId& CpuId) {
  if (CpuId.HasGatherScatter())
    return createGatherScatterResolverPass();
  return createX86ResolverPass();
}

static FunctionPass* createPacketizer(const Intel::CPUId& CpuId,
                                      unsigned int vectorizationDimension) {
  return createPacketizerPass(CpuId.HasGatherScatter(), vectorizationDimension);
}

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

static unsigned int vectorizationDim = 0;

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
  initializeLoopInfoWrapperPassPass(*PassRegistry::getPassRegistry());
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
  m_functionsToRetain.clear();
  for (auto it = M.begin(), end = M.end(); it != end; it++)
    m_functionsToRetain.insert(&*it);

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
    allOneFunc->addFnAttr(Mangler::vectorizer_builtin_attr);
    allOneFunc->addFnAttr(Attribute::NoUnwind);
    allOneFunc->addFnAttr(Attribute::ReadNone);
    Function* allZeroFunc =
      dyn_cast<Function>(M.getOrInsertFunction(Mangler::name_allZero + version.str(),
					       funcType));
    V_ASSERT(allZeroFunc && "Function type is incorrect, so dyn_cast failed");
    allZeroFunc->addFnAttr(Mangler::vectorizer_builtin_attr);
    allZeroFunc->addFnAttr(Attribute::NoUnwind);
    allZeroFunc->addFnAttr(Attribute::ReadNone);
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
      loadFunc->addFnAttr(Mangler::vectorizer_builtin_attr);
      loadFunc->addFnAttr(Attribute::NoUnwind);

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
      storeFunc->addFnAttr(Mangler::vectorizer_builtin_attr);
      storeFunc->addFnAttr(Attribute::NoUnwind);
    }
  }
}

void Vectorizer::deleteVectorizationStubs(Module& M) {
  std::vector<Function*> stubs;

  // Collect all functions not marked to be retained and not used.
  auto do_not_retain = m_functionsToRetain.end();
  for (auto it = M.begin(), end = M.end(); it != end; it++)
    if (m_functionsToRetain.find(&*it) == do_not_retain &&
	it->use_empty())
      stubs.push_back(&*it);

  // Delete the collected functions
  for (auto it = stubs.begin(), end = stubs.end(); it != end; it++)
    (*it)->eraseFromParent();

  m_functionsToRetain.clear();
}

Function* Vectorizer::createFunctionToVectorize(Function& originalFunction,
						VectorVariant& vectorVariant,
						Type* characteristicDataType) {
  Module* M = originalFunction.getParent();
  std::string functionName =
    vectorVariant.encode() + "_Vectorized_." + originalFunction.getName().str();

  if (!vectorVariant.isMasked()) {
    // Just clone the function
    ValueToValueMapTy vmap;
    Function* clone = CloneFunction(&originalFunction, vmap, true, NULL);
    clone->setName(functionName);
    M->getFunctionList().push_back(clone);
    return clone;
  }

  // Create a new function with the same signature and an additional
  // mask parameter and clone the original function into it.

  FunctionType* originalFunctionType = originalFunction.getFunctionType();
  Type* returnType = originalFunctionType->getReturnType();
  vector<Type*> parameterTypes;
  FunctionType::param_iterator it = originalFunctionType->param_begin();
  FunctionType::param_iterator end = originalFunctionType->param_end();
  for (; it != end; it++) {
    Type* vectorABIType = vectorVariant.promoteToSupportedType(*it);
    parameterTypes.push_back(vectorABIType);
  }
  unsigned int maskSize = characteristicDataType->getPrimitiveSizeInBits();
  Type* maskType = Type::getIntNTy(originalFunction.getContext(), maskSize);
  parameterTypes.push_back(maskType);
  FunctionType* maskedFunctionType = FunctionType::get(returnType,
						       parameterTypes,
						       false);
  Function* functionToVectorize = Function::Create(maskedFunctionType,
						   originalFunction.getLinkage(),
						   functionName,
						   originalFunction.getParent());
  LLVMContext& context = functionToVectorize->getContext();
  functionToVectorize->copyAttributesFrom(&originalFunction);
  ValueToValueMapTy vmap;
  Function::arg_iterator argIt = originalFunction.arg_begin();
  Function::arg_iterator argEnd = originalFunction.arg_end();
  Function::arg_iterator newArgIt = functionToVectorize->arg_begin();
  for (; argIt != argEnd; argIt++, newArgIt++) {
    newArgIt->setName(argIt->getName());
    vmap[argIt] = newArgIt;
  }
  Argument& maskArgument = *newArgIt;
  maskArgument.setName("mask");
  SmallVector<ReturnInst*, 8> returns;
  bool moduleLevelChanges = true;
  CloneFunctionInto(functionToVectorize,
		    &originalFunction,
		    vmap,
		    moduleLevelChanges,
		    returns);

  // Condition the entire body of the function with the mask
  BasicBlock& entryBlock = functionToVectorize->getEntryBlock();
  BasicBlock* earlyExitBB = BasicBlock::Create(context,
					       "earlyExit",
					       functionToVectorize,
					       &entryBlock);
  if (returnType->isVoidTy())
    ReturnInst::Create(context, earlyExitBB);
  else
    ReturnInst::Create(context,
		       Constant::getNullValue(returnType),
		       earlyExitBB);
  BasicBlock* newEntryBlock = BasicBlock::Create(context,
						 "testMask",
						 functionToVectorize,
						 earlyExitBB);
  Value* resetMaskValue = ConstantInt::get(maskArgument.getType(), 0);
  ICmpInst* testMask = new ICmpInst(*newEntryBlock,
				    CmpInst::ICMP_EQ,
				    &maskArgument,
				    resetMaskValue,
				    "maskTest");
  BranchInst::Create(earlyExitBB, &entryBlock, testMask, newEntryBlock);

  // Move any allocas from the previous entry block to the new on, as
  // long as they do not use values from that BB (otherwise we need to
  // move those as well and then need to make sure they have no side
  // effects).
  vector<Instruction*> toBeMoved;
  SmallPtrSet<Instruction*, 20> doNotMove;
  BasicBlock::iterator bbIt = entryBlock.begin();
  BasicBlock::iterator bbEnd = entryBlock.end();
  for (; bbIt != bbEnd; bbIt++) {
    Instruction* inst = bbIt;
    if (!isa<AllocaInst>(inst)) {
      // Do not move non-alloca instructions
      doNotMove.insert(inst);
      continue;
    }
    // Check if this alloca uses any immovable instructions
    bool usingImmovable = false;
    User::op_iterator OI = inst->op_begin(), OE = inst->op_end();
    for (; OI != OE; ++OI) {
      Instruction* usedInstruction = dyn_cast<Instruction>(OI->get());
      if (!usedInstruction)
	continue;
      if (doNotMove.count(usedInstruction) > 0) {
	usingImmovable = true;
	break;
      }
    }
    if (usingImmovable)
      doNotMove.insert(inst);
    else
      toBeMoved.push_back(inst);
  }
  vector<Instruction*>::iterator tbmIt = toBeMoved.begin();
  vector<Instruction*>::iterator tbmEnd = toBeMoved.end();
  for (; tbmIt != tbmEnd; tbmIt++) {
    (*tbmIt)->moveBefore(testMask);
  }

  return functionToVectorize;
}

bool Vectorizer::runOnModule(Module &M)
{
  V_PRINT(wrapper, "\nEntered Vectorizer Wrapper!\n");

  // set isVectorized and proper number of kernels to zero, 
  // in case vectorization fails

  m_numOfKernels = 0;
  m_isModuleVectorized = true;

  VectorizerUtils::FunctionVariants functionsToVectorize;
  VectorizerUtils::getFunctionsToVectorize(M, functionsToVectorize);
  if (functionsToVectorize.empty()) {
    // No functions to vectorize
    return false;
  }

#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(&M);
#endif

/* xmain */
  createVectorizationStubs(M);
#if 0
  if (!m_runtimeModule)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }
#endif


  for (auto& pair : functionsToVectorize)
  {
    Function& F = *pair.first;
    VectorizerUtils::DeclaredVariants& declaredVariants = pair.second;
    for (auto& declaredVariant : declaredVariants)
    {
      VectorVariant vectorVariant(declaredVariant);
      assert(F.arg_size() ==
	     (vectorVariant.getParameters().size() -
	      (vectorVariant.isMasked() ? 1 : 0)) &&
	     "function and vector variant differ in number of parameters");

      Type* characteristicDataType =
          VectorizerUtils::calcCharacteristicType(F, vectorVariant);

      // Get a working copy of the function to operate on
      Function *clone = createFunctionToVectorize(F,
						  vectorVariant,
						  characteristicDataType);

      // Prepare the (clone) scalar function for vectorization
      bool canVectorize = preVectorizeFunction(*clone);
      if (!canVectorize) {
        // We can't or choose not to vectorize the function.
        // Erase the clone from the module, but first copy the vectorizer
        // stats back to the original function
        intel::Statistic::copyFunctionStats(*clone, F);
        intel::Statistic::removeFunctionStats(*clone);
        clone->eraseFromParent();
        continue;
      }

      // Generate the vector variant of the scalar function. This function
      // has the correct signature for this variant but computes the requested
      // N instances of the scalar function by calling it in a 0..N loop (call
      // is inlined, so the cloned scalar function is no longer needed). 
      Function* vectFunc = createVectorLoopFunction(*clone,
                                                    vectorVariant,
                                                    F.getName());
      // copy stats from the original function to the new one
      intel::Statistic::copyFunctionStats(*clone, *vectFunc);
      intel::Statistic::removeFunctionStats(*clone);
      // Delete the scalar pre-vectorized clone
      clone->eraseFromParent();

      // Do actual vectorization work on the vector variant
      vectorizeFunction(*vectFunc, vectorVariant);

      m_functionsToRetain.insert(vectFunc);
    }
  }

  deleteVectorizationStubs(M);

#ifdef USE_METADATA_API
  //Save Metadata to the module
  mdUtils.save(M.getContext());
#endif

  V_DUMP_MODULE((&M));
  //////////////////////////////////////////////
  //////////////////////////////////////////////
  V_PRINT(wrapper, "\nCompleted Vectorizer Wrapper!\n");

  return m_isModuleVectorized;
}

bool Vectorizer::preVectorizeFunction(Function& F) {
  // Case the config was not set quit gracefully.
  // TODO: add default config or find another solutiuon for config options.
  if (!m_pConfig) {
    return false;
  }

  Module *M = F.getParent();

  legacy::FunctionPassManager fpm(M);
  fpm.add(createBuiltinLibInfoPass(M, ""));

  // Register lowerswitch
  fpm.add(createLowerSwitchPass());

  // A workaround to fix regression in sgemm on CPU and not causing new regression on Machine with Gather Scatter
  int sroaArrSize = -1;
  if (! m_pConfig->GetCpuId().HasGatherScatter())
    sroaArrSize = 16;

  fpm.add(createScalarReplAggregatesPass(1024, true, -1, sroaArrSize, 64));
  fpm.add(createInstructionCombiningPass());
  if (m_pConfig->GetDumpHeuristicIRFlag())
    fpm.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_scalarizer"));
  fpm.add(createDeadCodeEliminationPass());

  // Register mergereturn
  FunctionPass *mergeReturn = new UnifyFunctionExitNodes();
  fpm.add(mergeReturn);

  // Register phiCanon
  FunctionPass *phiCanon = createPhiCanon();
  fpm.add(phiCanon);

  // Simplify loops
  // This must happen after phiCanon since phi canonization can undo
  // loop simplification by breaking dedicated exit nodes.
  fpm.add(createLoopSimplifyPass());

  fpm.add(createDeadCodeEliminationPass());
  // Need to check for vectorization possibly AFTER phi canonization.
  // In theory this shouldn't matter, since we should never introduce anything
  // that prohibits vectorization in these three passes.
  // In practice, however, phi canonization already had a bug that introduces
  // irreducible control-flow, so a defensive check appears to be necessary.
  VectorizationPossibilityPass* vecPossiblity = new VectorizationPossibilityPass();
  fpm.add(vecPossiblity);

  fpm.run(F);

  V_ASSERT(!verifyFunction(F) && "pre-vectorized function failed to verify");

  return vecPossiblity->isVectorizable();
}

/// @brief Utility pass to collapse the 0..VL outer loop into a single iteration
class CollapseOuterLoop : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  CollapseOuterLoop() : FunctionPass(ID) {}
  ~CollapseOuterLoop() {}
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "PacketizeFunction";
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
  }
  virtual bool runOnFunction(Function &F) {
    LoopInfo* LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    assert(LI && "Unable to get LoopInfo pass");
    LoopInfo::iterator it = LI->begin();
    LoopInfo::iterator end = LI->end();
    assert(it != end && "Expected at least one top-level loops");
    llvm::Loop* outerLoop = *it;
    it++;
    assert(it == end && "Expected at most one top-level loops");
    PHINode* indVar = outerLoop->getCanonicalInductionVariable();
    assert(indVar && "Outer loop has no canonical induction variable");
    assert(indVar->hasOneUse() && "Expected single use for induction variable");
    BasicBlock* latch = outerLoop->getLoopLatch();
    assert(latch && "Outer loop does not have a single latch");
    Value* valueFromLatch = indVar->getIncomingValueForBlock(latch);
    auto userIt = valueFromLatch->user_begin();
    auto userEnd = valueFromLatch->user_end();
    for (; userIt != userEnd; ++userIt) {
      Value* user = *userIt;
      ICmpInst* limit = dyn_cast<ICmpInst>(user);
      if (!limit)
        continue;
      IntegerType* int32Type = Type::getInt32Ty(F.getContext());
      ICmpInst* collapsedLimit = new ICmpInst(limit,
                                              limit->getPredicate(),
                                              limit->getOperand(0),
                                              ConstantInt::get(int32Type, 1),
                                              "collapsed.ind.var");
      limit->replaceAllUsesWith(collapsedLimit);
      limit->eraseFromParent();
      assert(!verifyFunction(F) && "I broke this module");
      break; // there should be only one
    }
    return true;
  }
};
char CollapseOuterLoop::ID = 0;

void Vectorizer::vectorizeFunction(Function& F, VectorVariant& vectorVariant) {
  // Function-wide (vectorization)
  V_PRINT(VectorizerCore, "\nBefore vectorization passes!\n");

  Module *M = F.getParent();
  legacy::FunctionPassManager fpm(M);

  BuiltinLibInfo* pBuiltinInfoPass =
    (BuiltinLibInfo*)createBuiltinLibInfoPass(M, "");
  pBuiltinInfoPass->getRuntimeServices()->setPacketizationWidth(vectorVariant.getVlen());
  fpm.add(pBuiltinInfoPass);

  // add WIAnalysis for the predicator.
  fpm.add(new WIAnalysis(vectorizationDim));

  // Register predicate
  FunctionPass *predicate = createPredicator();
  fpm.add(predicate);

  // Register mem2reg
  FunctionPass *mem2reg = createPromoteMemoryToRegisterPass();
  fpm.add(mem2reg);

  // Register DCE
  fpm.add(createDeadCodeEliminationPass());

  // Add WIAnalysis for SimplifyGEP.
  fpm.add(new WIAnalysis(vectorizationDim));

  // Register SimplifyGEP
  FunctionPass *simplifyGEP = createSimplifyGEPPass();
  fpm.add(simplifyGEP);

  // add WIAnalysis for the packetizer.
  fpm.add(new WIAnalysis(vectorizationDim));

  // Register packetize
  FunctionPass *packetize = createPacketizer(m_pConfig->GetCpuId(), vectorizationDim);
  fpm.add(packetize);

  // Register DCE
  fpm.add(createDeadCodeEliminationPass());

  if (m_pConfig->GetDumpHeuristicIRFlag())
    fpm.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_resolver"));

  // Register resolve
  FunctionPass *resolver = createResolverPass(m_pConfig->GetCpuId());
  fpm.add(resolver);
  fpm.add(new CollapseOuterLoop());
  fpm.add(createLoopUnrollPass());

  // Final cleaning up
  // TODO:: support patterns generated by instcombine in LoopWIAnalysis so
  // it will be able to identify strided values which are important in apple
  // for stream samplers handling.
  fpm.add(createInstructionCombiningPass());
  fpm.add(createCFGSimplificationPass());
  fpm.add(createPromoteMemoryToRegisterPass());
  fpm.add(createAggressiveDCEPass());
  if (m_pConfig->GetDumpHeuristicIRFlag()) {
    fpm.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "vec_end"));
  }

  fpm.doInitialization();
  fpm.run(F);
  V_ASSERT(!verifyFunction(F) && "vectorized function failed to verify");
}

// Create a function implementing a given vector variant of a scalar function.
// The resulting function will have the signature induced by the vector variant
// but will still use a scalar loop to compute the N instances. The loop,
// however, will be tailored for the vectorizer (i.e. strip-mined and
// interchanged). For example, for a scalar function
//   Tr f(uniform T1 p1, consecutive T2 p2, random T3 p3);
// annotated with some vectorlength=VL, the function will have the following
// schematic structure (where AVL is the actual vector length chosen by the
// vectorizer):
//   <VL x Tr> fv(T1 p1, T2 p2, <VL x T1> p3) {
//     T3 s3[VL];
//     Tr sr[VL];
//     <VL x Tr> rv;
//     s3 <- p3;
//     unsigned int AVL = VL; // always, for now
//     for (i = 0; i < AVL; ++i)      // 'for (k = 0; k < VL; ++k)' strip-mined
//       for (j = 0; j < VL; j+=AVL)  // by factor AVL and interchanged
//         sr[j+i] = f(p1, p2+j+i, s3[j+i]); // inlined
//     rv <- sr;
//     return rv;
//   }
// The outer loop is the loop to be vectorized (i.e. runs AVL iterations at once),
// while the inner loop is the region to be packetized (i.e. executes AVL times
// in parallel), with 'i' predefined as CONSECUTIVE.
// This allows us to treat the inner loop and its body uniformly and transform
// the inner loop into a vector loop of the form:
//   for (j=<0,...,0>; j < <VL,...,VL>; j+= <AVL,...,AVL>)
// Once the inner loop is packetized the outer loop is removed. The result is
// as if a single 'for (k = 0; k < VL; ++k)' loop was vectorized.
// Note that:
// - We assume VL % AVL == 0 to keep the inner loop UNIFORM. The more general
// condition 'i + j < VL' would also support remainder iterations (last
// iteration partly masked) but we need WIAnalysis to support piecewise-uniform
// values to avoid masking when VL % AVL == 0 does hold.
// - The inner loop can be removed after vectorization if VL == AVL.
// The temporary arrays used for scalarizing the accesses to vector parameters
// and return value are removed after vectorization.
Function* Vectorizer::createVectorLoopFunction(Function& scalarFunction,
                                                 VectorVariant& vectorVariant,
                                                 StringRef scalarFuncName) {
  // Create a new function type with vector types for the RANDOM parameters
  FunctionType* originalFunctionType = scalarFunction.getFunctionType();
  Type* originalReturnType = originalFunctionType->getReturnType();
  Type* vectorReturnType;
  if (originalReturnType->isVoidTy())
    vectorReturnType = originalReturnType;
  else
    vectorReturnType = VectorType::get(originalReturnType,
				       vectorVariant.getVlen());
  std::vector<VectorKind> parameterKinds = vectorVariant.getParameters();
  vector<Type*> parameterTypes;
  FunctionType::param_iterator it = originalFunctionType->param_begin();
  FunctionType::param_iterator end = originalFunctionType->param_end();
  std::vector<VectorKind>::iterator vkIt = parameterKinds.begin();
  for (; it != end; it++, ++vkIt) {
    if (vkIt->isVector())
      parameterTypes.push_back(VectorType::get(*it, vectorVariant.getVlen()));
    else
      parameterTypes.push_back(*it);
  }
  FunctionType* vectorFunctionType = FunctionType::get(vectorReturnType,
						       parameterTypes,
						       false);
  std::string name = vectorVariant.generateFunctionName(scalarFuncName);
  Function* wrapperFunc = Function::Create(vectorFunctionType,
					   scalarFunction.getLinkage(),
					   name,
					   scalarFunction.getParent());
  // Copy all the attributes from the scalar function to its vector version
  // except for the vector variant attributes.
  wrapperFunc->copyAttributesFrom(&scalarFunction);
  AttrBuilder attrBuilder;
  for (auto attribute : VectorizerUtils::getVectorVariantAttributes(*wrapperFunc)) {
    attrBuilder.addAttribute(attribute);
  }
  AttributeSet attrsToRemove = AttributeSet::get(wrapperFunc->getContext(),
                                                 AttributeSet::FunctionIndex,
                                                 attrBuilder);
  wrapperFunc->removeAttributes(AttributeSet::FunctionIndex, attrsToRemove);
      
  wrapperFunc->setCallingConv(CallingConv::Intel_regcall);
  BasicBlock* entryBB = BasicBlock::Create(wrapperFunc->getContext(),
					   "wrapper.entry",
					   wrapperFunc);

  // Create the loops.
  IntegerType* int32Type = Type::getInt32Ty(wrapperFunc->getContext());
  Value* zero = ConstantInt::get(int32Type, 0);
  Value* one = ConstantInt::get(int32Type, 1);
  unsigned vlen = vectorVariant.getVlen();
  unsigned avlen = vectorVariant.getVlen();
  Value* vlenVal = ConstantInt::get(int32Type, vlen);
  Value* avlenVal = ConstantInt::get(int32Type, avlen);
  string vecLoopName = "vec_loop";
  loopRegion vecLoop = LoopUtils::createLoop(entryBB, entryBB, zero, avlenVal,
                                             vlenVal, vecLoopName,
                                             wrapperFunc->getContext());
  string outerLoopName = "outer_loop";
  loopRegion outerLoop = LoopUtils::createLoop(vecLoop.m_preHeader,
                                               vecLoop.m_exit, zero,
                                               one, avlenVal, outerLoopName,
                                               wrapperFunc->getContext());

  // Store vector arguments into arrays so loop is fully scalar, i.e. contains
  // '%s = %v[%i]' access patterns instead of '%s = extractelement %v, %i'.
  Instruction* insAtPH = outerLoop.m_preHeader->getTerminator();
  BasicBlock::iterator insAtLB = entryBB->getFirstInsertionPt();
  Value* adjustedIndex =
    BinaryOperator::CreateAdd(BinaryOperator::CreateMul(vecLoop.m_indVar,
                                                        avlenVal,
                                                        "offset",
                                                        insAtLB),
                              outerLoop.m_indVar,
                              "index",
                              insAtLB);
  FunctionType::param_iterator wft_it = vectorFunctionType->param_begin();
  FunctionType::param_iterator wft_end = vectorFunctionType->param_end();
  Function::arg_iterator wfa_it = wrapperFunc->arg_begin();
  Function::arg_iterator ofa_it = scalarFunction.arg_begin();
  vkIt = parameterKinds.begin();
  std::vector<Value*> callArguments;
  ArrayRef<Value*> loopIndex(adjustedIndex);
  map<int, Value*> ithStrideValues;
  for (; wft_it != wft_end; ++wft_it, ++wfa_it, ++ofa_it, ++vkIt) {
    Argument& arg = *wfa_it;
    Type* argType = *wft_it;
    arg.setName(ofa_it->getName());
    if (!argType->isVectorTy()) {
      Value* callArg = NULL;
      if (vkIt->isLinear()) {
        // Linear parameters translate to (arg + i * stride)
        stringstream stride_name;
        int stride = vkIt->getStride();
        Value* ithStride;
        auto existingValue = ithStrideValues.find(stride);
        if (existingValue == ithStrideValues.end()) {
          Value* strideValue;
          if (stride != 1) {
            strideValue = ConstantInt::get(int32Type, stride);
            stride_name << "i_x_" << vkIt->getStride();
            ithStride = BinaryOperator::CreateMul(adjustedIndex,
                                                  strideValue,
                                                  stride_name.str(),
                                                  insAtLB);
          }
          else
            ithStride = adjustedIndex;
          ithStrideValues[stride] = ithStride;
        }
        else
          ithStride = existingValue->second;
        string ithElemName = arg.getName().str() + "_i";
        if (argType->isPointerTy()) {
          // Express the linear stride using GEP
          callArg = GetElementPtrInst::Create(nullptr,
                                              &arg,
                                              loopIndex,
                                              ithElemName,
                                              insAtLB);
        }
        else {
          // Express the linear stride using addition
          callArg = BinaryOperator::CreateAdd(&arg,
                                               ithStride,
                                               ithElemName,
                                               insAtLB);
        }
      }
      else {
        // Just pass the argument as is
        callArg = &arg;
      }
      callArguments.push_back(callArg);
      continue;
    }
    // Argument passed as vector. Store and access as an array of scalars.
    VectorType* vectorType = dyn_cast<VectorType>(*wft_it);
    AllocaInst* vecStorage = new AllocaInst(vectorType, "tmp_array", insAtPH);
    new StoreInst(&arg, vecStorage, insAtPH);
    string elemName = arg.getName().str() + "_i";
    string elemAddrName = elemName + "_addr";
    Type* scalarPointerType = vectorType->getElementType()->getPointerTo();
    BitCastInst* toScalarPointer = new BitCastInst(vecStorage,
                                                   scalarPointerType,
                                                   "scalar_" + elemAddrName,
                                                   insAtPH);
    GetElementPtrInst* GEP = GetElementPtrInst::CreateInBounds(toScalarPointer,
                                                               loopIndex,
                                                               elemAddrName,
                                                               insAtLB);
    callArguments.push_back(new LoadInst(GEP, elemName, insAtLB));
  }

  CallInst* call = CallInst::Create(&scalarFunction, callArguments,
				                    "", insAtLB);

  // Create a return value if needed and temporary storage for the scalar
  // return values.
  if (vectorReturnType->isVoidTy())
    ReturnInst::Create(wrapperFunc->getContext(), outerLoop.m_exit);
  else
  {
    // Return as vector a temporary array to which we'll assign the return
    // values of the scalar call to.
    Value* tmpRetVal = new AllocaInst(vectorReturnType, "tmpRetVal", insAtPH);
    ReturnInst::Create(wrapperFunc->getContext(),
                       new LoadInst(tmpRetVal, "retVal", outerLoop.m_exit),
                       outerLoop.m_exit);
    // Store the scalar call's return value into the temporary array returned
    // as vector.
    Type* scalarPointerType = originalReturnType->getPointerTo();
    BitCastInst* toScalarPointer = new BitCastInst(tmpRetVal,
                                                   scalarPointerType,
                                                   "scalar_ret_val_addr",
                                                   insAtPH);
    GetElementPtrInst* scalarRetStorage =
      GetElementPtrInst::CreateInBounds(toScalarPointer,
                                        loopIndex,
                                        "scalar_ret_addr",
                                        insAtLB);
    new StoreInst(call, scalarRetStorage, insAtLB);
  }

  // Inline the wrapper call
  InlineFunctionInfo ifi;
  bool inlined = llvm::InlineFunction(call, ifi, false);
  assert(inlined && "expected inline to succeed");

  return wrapperFunc;
}

} // Namespace intel

namespace llvm {
void initializeVPOVectorizer(PassRegistry &Registry) {
  initializeVectorizerPass(Registry);
}

ModulePass *createVPOVectorizerPass() {
  return new intel::Vectorizer();
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

