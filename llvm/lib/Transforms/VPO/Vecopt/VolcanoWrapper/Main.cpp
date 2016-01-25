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
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <sstream>

using namespace std;
using namespace vpo;

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

extern "C" FunctionPass* createX86ResolverPass();
extern "C" FunctionPass* createZMMResolverPass();
extern "C" FunctionPass *createIRPrinterPass(std::string dumpDir, std::string dumpName);

static FunctionPass* createResolverPass(ISAClass isaClass) {
  if (isaClass == ISAClass::ZMM)
    return createZMMResolverPass();
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

// Forward declaration.
static void insertBeginRegion(Module& M, Function *Clone,
                                VectorVariant &V, BasicBlock *EntryBlock);

#ifndef LOOP_VECTORIZATION_HINTS_IS_VISIBLE
// --------------------------------------------------------------------------
// The following code is LLVM's interface between the frontend and the loop
// vectorizer. Unfortunately, it resides in a source file, not a header file.
// To avoid a massive change from community source the code is copied here
// until this interface is (hopefully) refactored to a header in LLVM.
// --------------------------------------------------------------------------
/// Utility class for getting and setting loop vectorizer hints in the form
/// of loop metadata.
/// This class keeps a number of loop annotations locally (as member variables)
/// and can, upon request, write them back as metadata on the loop. It will
/// initially scan the loop for existing metadata, and will update the local
/// values based on information in the loop.
/// We cannot write all values to metadata, as the mere presence of some info,
/// for example 'force', means a decision has been made. So, we need to be
/// careful NOT to add them if the user hasn't specifically asked so.
class LoopVectorizeHints {
  enum HintKind {
    HK_WIDTH,
    HK_UNROLL,
    HK_FORCE
  };

  /// Hint - associates name and validation with the hint value.
  struct Hint {
    const char * Name;
    unsigned Value; // This may have to change for non-numeric values.
    HintKind Kind;

    Hint(const char * Name, unsigned Value, HintKind Kind)
      : Name(Name), Value(Value), Kind(Kind) { }

    bool validate(unsigned Val) {
      switch (Kind) {
      case HK_WIDTH:
        return isPowerOf2_32(Val);
      case HK_UNROLL:
        return isPowerOf2_32(Val);
      case HK_FORCE:
        return (Val <= 1);
      }
      return false;
    }
  };

  /// Vectorization width.
  Hint Width;
  /// Vectorization interleave factor.
  Hint Interleave;
  /// Vectorization forced
  Hint Force;

  /// Return the loop metadata prefix.
  static StringRef Prefix() { return "llvm.loop."; }

public:
  enum ForceKind {
    FK_Undefined = -1, ///< Not selected.
    FK_Disabled = 0,   ///< Forcing disabled.
    FK_Enabled = 1,    ///< Forcing enabled.
  };

  LoopVectorizeHints(const Loop *L, bool DisableInterleaving)
      : Width("vectorize.width", 8,
              HK_WIDTH),
        Interleave("interleave.count", DisableInterleaving, HK_UNROLL),
        Force("vectorize.enable", FK_Undefined, HK_FORCE),
        TheLoop(L) {
    // Populate values with existing loop metadata.
    getHintsFromMetadata();
  }

  /// Mark the loop L as already vectorized by setting the width to 1.
  void setAlreadyVectorized() {
    Width.Value = Interleave.Value = 1;
    Hint Hints[] = {Width, Interleave};
    writeHintsToMetadata(Hints);
  }

  unsigned getWidth() const { return Width.Value; }
  unsigned getInterleave() const { return Interleave.Value; }
  enum ForceKind getForce() const { return (ForceKind)Force.Value; }

private:
  /// Find hints specified in the loop metadata and update local values.
  void getHintsFromMetadata() {
    MDNode *LoopID = TheLoop->getLoopID();
    if (!LoopID)
      return;

    // First operand should refer to the loop id itself.
    assert(LoopID->getNumOperands() > 0 && "requires at least one operand");
    assert(LoopID->getOperand(0) == LoopID && "invalid loop id");

    for (unsigned i = 1, ie = LoopID->getNumOperands(); i < ie; ++i) {
      const MDString *S = nullptr;
      SmallVector<Metadata *, 4> Args;

      // The expected hint is either a MDString or a MDNode with the first
      // operand a MDString.
      if (const MDNode *MD = dyn_cast<MDNode>(LoopID->getOperand(i))) {
        if (!MD || MD->getNumOperands() == 0)
          continue;
        S = dyn_cast<MDString>(MD->getOperand(0));
        for (unsigned i = 1, ie = MD->getNumOperands(); i < ie; ++i)
          Args.push_back(MD->getOperand(i));
      } else {
        S = dyn_cast<MDString>(LoopID->getOperand(i));
        assert(Args.size() == 0 && "too many arguments for MDString");
      }

      if (!S)
        continue;

      // Check if the hint starts with the loop metadata prefix.
      StringRef Name = S->getString();
      if (Args.size() == 1)
        setHint(Name, Args[0]);
    }
  }

  /// Checks string hint with one operand and set value if valid.
  void setHint(StringRef Name, Metadata *Arg) {
    if (!Name.startswith(Prefix()))
      return;
    Name = Name.substr(Prefix().size(), StringRef::npos);

    const ConstantInt *C = mdconst::dyn_extract<ConstantInt>(Arg);
    if (!C) return;
    unsigned Val = C->getZExtValue();

    Hint *Hints[] = {&Width, &Interleave, &Force};
    for (auto H : Hints) {
      if (Name == H->Name) {
        if (H->validate(Val))
          H->Value = Val;
        else
          dbgs() << "LV: ignoring invalid hint '" << Name << "'\n";
        break;
      }
    }
  }

  /// Create a new hint from name / value pair.
  MDNode *createHintMetadata(StringRef Name, unsigned V) const {
    LLVMContext &Context = TheLoop->getHeader()->getContext();
    Metadata *MDs[] = {MDString::get(Context, Name),
                       ConstantAsMetadata::get(
                           ConstantInt::get(Type::getInt32Ty(Context), V))};
    return MDNode::get(Context, MDs);
  }

  /// Matches metadata with hint name.
  bool matchesHintMetadataName(MDNode *Node, ArrayRef<Hint> HintTypes) {
    MDString* Name = dyn_cast<MDString>(Node->getOperand(0));
    if (!Name)
      return false;

    for (auto H : HintTypes)
      if (Name->getString().endswith(H.Name))
        return true;
    return false;
  }

  /// Sets current hints into loop metadata, keeping other values intact.
  void writeHintsToMetadata(ArrayRef<Hint> HintTypes) {
    if (HintTypes.size() == 0)
      return;

    // Reserve the first element to LoopID (see below).
    SmallVector<Metadata *, 4> MDs(1);
    // If the loop already has metadata, then ignore the existing operands.
    MDNode *LoopID = TheLoop->getLoopID();
    if (LoopID) {
      for (unsigned i = 1, ie = LoopID->getNumOperands(); i < ie; ++i) {
        MDNode *Node = cast<MDNode>(LoopID->getOperand(i));
        // If node in update list, ignore old value.
        if (!matchesHintMetadataName(Node, HintTypes))
          MDs.push_back(Node);
      }
    }

    // Now, add the missing hints.
    for (auto H : HintTypes)
      MDs.push_back(createHintMetadata(Twine(Prefix(), H.Name).str(), H.Value));

    // Replace current metadata node with new one.
    LLVMContext &Context = TheLoop->getHeader()->getContext();
    MDNode *NewLoopID = MDNode::get(Context, MDs);
    // Set operand 0 to refer to the loop id itself.
    NewLoopID->replaceOperandWith(0, NewLoopID);

    TheLoop->setLoopID(NewLoopID);
  }

  /// The loop these hints belong to.
  const Loop *TheLoop;
};
#endif // LOOP_VECTORIZATION_HINTS_IS_VISIBLE

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

  set<Loop*> vectorizableLoops;
  set<Function*> functionsWithVectorizableLoops;

/* xmain */
#if 0
  if (!m_runtimeModule)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }
#endif

  // Collect all loops marked vectorizable and annotate them to-be-vectorized
  // by VPO notation.
  for (auto& F : M) {
    if (F.isDeclaration())
      continue;
    DominatorTree DT;
    DT.recalculate(F);
    LoopInfo LI;
    LI.analyze(DT);
    bool functionHasAtLeastOneVectorizableLoop = false;
    for (Loop* TopLevelLoop : LI) {
      for (Loop* L : depth_first(TopLevelLoop)) {
        LoopVectorizeHints hints(L, true);
        if (hints.getForce() == LoopVectorizeHints::FK_Enabled) {
          vectorizableLoops.insert(L);
          if (!functionHasAtLeastOneVectorizableLoop) {
            functionHasAtLeastOneVectorizableLoop = true;
            functionsWithVectorizableLoops.insert(L->getHeader()->getParent());
          }
          // Annotate the loop as to-be-vectorized (we only need 'begin region')
          assert(L->getLoopPreheader() &&
                 "Expected parallel loop to have a preheader");
          std::vector<VectorKind> parameters;
          VectorVariant vectorVariant(XMM, false, hints.getWidth(), parameters);
          insertBeginRegion(*F.getParent(),
                            &F,
                            vectorVariant,
                            L->getLoopPreheader());
        }
      }
    }
  }

  // TODO: The vectorizer assumes certain properties to hold on the loops
  // being vectorized (enforced by preVectorizeFunction). Ideally, we would
  // apply these prerequisite transformations on the loops, not the containing
  // functions. Until we narrow it down we pre-vectorize those functions here.
  for (auto F : functionsWithVectorizableLoops)
    preVectorizeFunction(*F);

  VectorizerUtils::FunctionVariants functionsToVectorize;
  VectorizerUtils::getFunctionsToVectorize(M, functionsToVectorize);

#ifdef USE_METADATA_API
  Intel::MetaDataUtils mdUtils(&M);
#endif

  // Function vectorization support: generate the required vector variant
  // functions ready for loop-vectorization: each function will execute a
  // 0..VL-1 loop calling the scalar function (with scalar transformations
  // required by the vectorizer).
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

      // Mark the vector variant function to protect it from the vectorizer's
      // cleanup which removes any stub it may have created.
      m_functionsToRetain.insert(vectFunc);

      // Add the vector variant function to the list of functions with
      // vectorizable loops.
      functionsWithVectorizableLoops.insert(vectFunc);
    }
  }

  // Now, for each function we identified with vectorizable loops do actual
  // vectorization work on the (loops in the) function.
  if (!functionsWithVectorizableLoops.empty()) {
    createVectorizationStubs(M);
    for (auto F : functionsWithVectorizableLoops)
      vectorizeFunction(*F);
    deleteVectorizationStubs(M);
  }

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

  // TODO: we actually only need the vectorized loop rotated.
  fpm.add(llvm::createLoopRotatePass());

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

/// @brief Utility pass to remove VPO's vectorization annotation from the code.
class RemoveVectorizationHints : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  RemoveVectorizationHints() : FunctionPass(ID) {}
  ~RemoveVectorizationHints() {}
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "RemoveVectorizationHints";
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
  }
  virtual bool runOnFunction(Function &F) {
    vector<Instruction*> hints;
    for (auto& BB : F)
      for (auto& I : BB) {
        IntrinsicInst* Inst = dyn_cast<IntrinsicInst>(&I);
        if (Inst) {
          Intrinsic::ID intrinsicID = Inst->getIntrinsicID();
          if (intrinsicID == Intrinsic::intel_directive ||
              intrinsicID == Intrinsic::intel_directive_qual ||
              intrinsicID == Intrinsic::intel_directive_qual_opnd ||
              intrinsicID == Intrinsic::intel_directive_qual_opndlist)
            hints.push_back(Inst);
        }
      }
    for (auto hint : hints)
      hint->eraseFromParent();
    assert(!verifyFunction(F) && "I broke this module");
    return !hints.empty();
  }
};
char RemoveVectorizationHints::ID = 0;

/// @brief Utility pass to bring loops marked for vectorization into the
/// form expected by the vectorizer:
///
/// Given a canonical loop. e.g.
///   for (i = 0; i < N; ++i)
///     a[i] = b[i] + c[i];
///
/// stripmine the loop by factor of 'length' and interchange the loops to get
///   for (j = 0; j < VL; ++j)
///     for (i = 0; (i*VL+j) < N; ++i)
///       a[i+j*VL] = b[i*VL+j] + c[i+j*VL];
///
/// The outer loop is the loop to be vectorized (i.e. runs VL iterations at
/// once), while the inner loop is the region to be packetized (i.e. executes
/// VL times in parallel), with 'j' predefined as CONSECUTIVE.
/// This allows us to treat the inner loop and its body uniformly and transform
/// the inner loop into a vector loop of the form:
///   for (i=<0,...,0>; (i*VL+j) < <N,...,N>; i+= <VL,...,VL>)
/// Once the inner loop is packetized the outer loop is removed. The result is
/// as if a single 'for (k = 0; k < VL; ++k)' loop was vectorized.
/// Note that:
/// - We assume VL % AVL == 0 to keep the inner loop UNIFORM. The more general
/// condition 'i + j < VL' would also support remainder iterations (last
/// iteration partly masked) but we need WIAnalysis to support piecewise-uniform
/// values to avoid masking when N % VL == 0 does hold.
/// - The inner loop can be removed after vectorization if VL == N.
class PrepareLoopForVectorization : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  PrepareLoopForVectorization() : FunctionPass(ID) {}
  ~PrepareLoopForVectorization() {}
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "PrepareLoopForVectorization";
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    //    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<VectorizableLoopsAnalysis>();
  }
  virtual bool runOnFunction(Function &F) {
    assert(!verifyFunction(F) && "I broke this module");
    VectorizableLoopsAnalysis* VLA = &getAnalysis<VectorizableLoopsAnalysis>();
    assert(VLA && "Unable to get vectorizable-loops analysis");
    bool anyChangeMade = false;
    for (auto& pair : VLA->getLoops()) {
      stripmineAndInterchangeLoop(*pair.first, pair.second.Vlen);
      anyChangeMade = true;
      assert(!verifyFunction(F) && "I broke this module");
    }
    return anyChangeMade;
  }

private:

  // Note: we are not updating LoopInfo, so calling this function invalidates it.
  void stripmineAndInterchangeLoop(llvm::Loop& loop, unsigned length) {
    BasicBlock* preheader = loop.getLoopPreheader();
    assert(preheader && "Expected loop to have a preheader");
    PHINode* indVar = loop.getCanonicalInductionVariable();
    assert(indVar && "loop has no canonical induction variable");
    BasicBlock* latch = loop.getLoopLatch();
    assert(latch && "Expected loop to have a (single) latch");
    Value* valueFromLatch = indVar->getIncomingValueForBlock(latch);
    assert(isa<BinaryOperator>(valueFromLatch) &&
           "Induction variable is not canonical");
    BinaryOperator* autoIncrement = cast<BinaryOperator>(valueFromLatch);
    assert(autoIncrement->getOpcode() == Instruction::Add &&
           "Induction variable is not canonical");
    unsigned phiOpIndex = 0;
    unsigned oneOpIndex = 1;
    if (!isa<ConstantInt>(autoIncrement->getOperand(oneOpIndex))) {
      phiOpIndex = 1;
      oneOpIndex = 0;
      assert(isa<ConstantInt>(autoIncrement->getOperand(oneOpIndex)) &&
             "Induction variable is not canonical");
    }
    assert(autoIncrement->getOperand(phiOpIndex) == indVar &&
           "Expected to find induction variable at auto-increment");
    assert(isa<ConstantInt>(autoIncrement->getOperand(oneOpIndex)) &&
           cast<ConstantInt>(autoIncrement->getOperand(oneOpIndex))->isOne() &&
           "Induction variable is not canonical");

    // Wrap the loop with a new [0..length-1] loop, replacing:
    // PH -> H1 -> ... -> L1 -> EX
    //        ^------------+-----^
    // with:
    // PH -> H2 -> H1 -> ... -> L1 -> EX -> E2
    //             ^------------+-----^
    //       ^------------------------+----^
    // We'll split the loop's (assumed) single exit block EX into EX -> E2 and
    // use EX and E2 for the new loop as the latch and exit block, respectively.

    BasicBlock* newLoopLatch = loop.getExitBlock();
    assert(newLoopLatch && "Expected loop to have a single exit block");
    BasicBlock* newLoopExit = llvm::SplitBlock(newLoopLatch,
                                               newLoopLatch->begin());
    newLoopLatch->getTerminator()->eraseFromParent();
    BasicBlock* newLoopHeader = llvm::SplitBlock(preheader,
                                                 preheader->getTerminator());
    string outerLoopName = "outer_loop";
    IntegerType* intType = dyn_cast<IntegerType>(indVar->getType());
    assert(intType && "Expected induction variable to be of integer type");
    Value* zero = ConstantInt::get(intType, 0);
    Value* one = ConstantInt::get(intType, 1);
    Value* lengthVal = ConstantInt::get(intType, length);
    loopRegion outerLoop = LoopUtils::createLoop(preheader, newLoopHeader,
                                                 newLoopLatch, newLoopExit,
                                                 zero, one, lengthVal,
                                                 outerLoopName,
                                                 preheader->getContext());

    // Replace all uses of the loops induction variable 'i' with the expression
    // 'i * length + j'.
    Instruction* insAtLB = indVar->getParent()->getFirstInsertionPt();
    Instruction* chunkOffset =
      BinaryOperator::CreateMul(indVar, lengthVal, "offset", insAtLB);
    Value* stripminedIndex =
      BinaryOperator::CreateAdd(chunkOffset,
                                outerLoop.m_indVar,
                                "stripminedIndex",
                                insAtLB);
    indVar->replaceAllUsesWith(stripminedIndex);

    // The former step also replaced two uses of 'i' that need to linger on:
    // 1) in the chunk offset value 'offset = i * length'.
    // 2) the auto-increment of the induction variable 'i = i + 1'
    // Restore these specific uses here.
    chunkOffset->setOperand(0, indVar);
    autoIncrement->setOperand(phiOpIndex, indVar);

    BranchInst* latchBranch = dyn_cast<BranchInst>(latch->getTerminator());
    assert(latchBranch && latchBranch->isConditional() &&
           "Expected latch terminator to be a conditional branch");
    ICmpInst* loopCond = dyn_cast<ICmpInst>(latchBranch->getCondition());
    assert(loopCond && "Expected loop condition to be icmp");
    Instruction* nextChunkOffset = BinaryOperator::CreateMul(autoIncrement,
                                                             lengthVal,
                                                             "offset.next",
                                                             loopCond);

    // Complete the stripmining by replacing the loop's limit-reached check
    // from '(i + 1) == X' to '(i + 1) * length + j >= X'. In order to take
    // advantage of current support for piecewise-uniform classification we'll
    // actually use 'j >= X - (i + 1) * length' for the latter.
    Value* currentLimit = loopCond->getOperand(1);
    if (currentLimit == autoIncrement)
      currentLimit = loopCond->getOperand(0);
    Value* stripminedLimit = BinaryOperator::CreateSub(currentLimit,
                                                       nextChunkOffset,
                                                       "stripminedLimit",
                                                       loopCond);
    loopCond->replaceUsesOfWith(autoIncrement, outerLoop.m_indVar);
    loopCond->replaceUsesOfWith(currentLimit, stripminedLimit);
    
    switch (loopCond->getPredicate()) {
      case CmpInst::Predicate::ICMP_EQ:
        // Should-exit test, fix it to work for vectors.
        loopCond->setPredicate(CmpInst::Predicate::ICMP_UGE);
        break;
      case CmpInst::Predicate::ICMP_SLT:
      case CmpInst::Predicate::ICMP_ULT:
        // Should-continue tests, will work as-is for vectors.
        break;
      default:
        assert(false && "Unexpected loop latch predicate");
        break;
    }
  }
};
char PrepareLoopForVectorization::ID = 0;

/// @brief Utility pass to collapse the 0..VL outer loop into a single iteration
class CollapseOuterLoop : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  CollapseOuterLoop() : FunctionPass(ID) {}
  ~CollapseOuterLoop() {}
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "CollapseOuterLoop";
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<VectorizableLoopsAnalysis>();
  }
  virtual bool runOnFunction(Function &F) {
    VectorizableLoopsAnalysis* VLA = &getAnalysis<VectorizableLoopsAnalysis>();
    assert(VLA && "Unable to get vectorizable-loops analysis");
    bool anyChangeMade = false;
    for (auto& pair : VLA->getLoops()) {
      llvm::Loop* outerLoop = pair.first;
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
        IntegerType* intType =
          dyn_cast<IntegerType>(limit->getOperand(0)->getType());
        assert(intType && "Expected limit to be of integer type");
        ICmpInst* collapsedLimit = new ICmpInst(limit,
                                                limit->getPredicate(),
                                                limit->getOperand(0),
                                                ConstantInt::get(intType, 1),
                                                "collapsed.ind.var");
        limit->replaceAllUsesWith(collapsedLimit);
        limit->eraseFromParent();
        assert(!verifyFunction(F) && "I broke this module");
        break; // there should be only one
      }
      anyChangeMade = true;
    }
    return anyChangeMade;
  }
};
char CollapseOuterLoop::ID = 0;

/// @brief Utility pass to remove the temporary alloca instructions used for
/// representing vector arguments and return value inside the loop being
/// vectorized as scalar, induction variable based memory accesses.
class RemoveTempScalarizingAllocas : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  RemoveTempScalarizingAllocas() : FunctionPass(ID) {}
  ~RemoveTempScalarizingAllocas() {}
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "RemoveTempScalarizingAllocas";
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
  }
  virtual bool runOnFunction(Function &F) {
    bool Modified = false;
    // For each argument stored to an alloca, replace all uses of that alloca
    // with the argument. We'll let standard cleanup passes remove the now
    // redundant alloca code.
    Function::arg_iterator ArgIt = F.arg_begin();
    Function::arg_iterator ArgEnd = F.arg_end();
    for (; ArgIt != ArgEnd; ArgIt++) {
      Argument& Arg = *ArgIt;
      if (!Arg.getType()->isVectorTy())
        continue; // skip non-vector arguments
      auto UserIt = Arg.user_begin();
      auto UserEnd = Arg.user_end();
      for (; UserIt != UserEnd; ++UserIt) {
        Value* User = *UserIt;
        if (StoreInst* Store = dyn_cast<StoreInst>(User)) {
          Value* Address = Store->getPointerOperand();
          if (isa<AllocaInst>(Address) && Store->getValueOperand() == &Arg)
            Modified = replaceLoadsWithArgument(Address, Arg) || Modified;
        }
      }
    }
    // If the return value is loaded from an alloca, find the value being
    // stored and return it instead.
    for (auto BBI = F.begin(), BBE = F.end(); BBI != BBE; ++BBI) {
      ReturnInst* RetInst = dyn_cast<ReturnInst>(BBI->getTerminator());
      if (!RetInst)
        continue; // BB doesn't terminate with a 'return'
      Value* RetVal = RetInst->getReturnValue();
      if (!RetVal)
        break; // Function returns void
      if (LoadInst* Load = dyn_cast<LoadInst>(RetVal)) {
        Value* Address = Load->getPointerOperand();
        if (isa<AllocaInst>(Address))
          Modified = replaceValueWithStoredValue(Address, RetVal) || Modified;
      }
      break; // We expect a single return instruction
    }
    assert(!verifyFunction(F) && "I broke this module");
    return Modified;
  }
private:
  /// @brief Recursively go through a value and its users and replace any Load
  /// instruction among them with an argument.
  /// @param Val value to replace with Arg if it is a Load
  /// @param Arg value to substitute Val with
  /// @return whether we modified the code
  bool replaceLoadsWithArgument(Value* Val, Argument& Arg) {
    LoadInst* Load = dyn_cast<LoadInst>(Val);
    if (Load) {
      // Make sure we're loading the same type (just in case
      // the load was scalarized for some reason).
      if (Load->getType() == Arg.getType()) {
        // This is what we came for: after this, the argument is used
        // directly by the former user of this load.
        Load->replaceAllUsesWith(&Arg);
        return true;
      }
      return false; // Either way, we're done
    }
    // Val isn't a load, but its users might be (we follow only certain types of
    /// users to limit the recursion while still supporting common patterns for
    /// reaching the load).
    bool Modified = false;
    auto UserIt = Val->user_begin();
    auto UserEnd = Val->user_end();
    for (; UserIt != UserEnd; ++UserIt)
      if (isa<LoadInst>(*UserIt) ||
          isa<GetElementPtrInst>(*UserIt) ||
          isa<ExtractElementInst>(*UserIt) ||
#ifndef VECTOR_GEP_TAKES_SCALARS
          isa<InsertElementInst>(*UserIt) || // uniform address is broadcasted
          isa<ShuffleVectorInst>(*UserIt) || // uniform address is broadcasted
#endif
          isa<BitCastInst>(*UserIt))
        Modified = replaceLoadsWithArgument(*UserIt, Arg) || Modified;
    return Modified;
  }
  /// @brief Recursively go through a value to find a StoreInst and replace
  /// a given value with the value it stores (assume there is only one).
  /// @param Stored value to explore looking for a Store
  /// @param Loaded value to replace with the stored value
  bool replaceValueWithStoredValue(Value* Stored, Value* Loaded) {
    StoreInst* Store = dyn_cast<StoreInst>(Stored);
    if (Store) {
      Value* StoredValue = Store->getValueOperand();
      // This is what we came for: after this, the stored value is used
      // directly by the user of the loaded value.
      assert(StoredValue->getType() == Loaded->getType() &&
             "stored value not of the same type as loaded value");
      Loaded->replaceAllUsesWith(StoredValue);
      Store->eraseFromParent();
      return true;
    }
    // This isn't a store, but one of its users may be (we follow only certain
    // types of users to limit the recursion while still supporting common
    // patterns for reaching the store).
    auto UserIt = Stored->user_begin();
    auto UserEnd = Stored->user_end();
    for (; UserIt != UserEnd; ++UserIt)
      if (isa<StoreInst>(*UserIt) ||
          isa<GetElementPtrInst>(*UserIt) ||
          isa<ExtractElementInst>(*UserIt) ||
#ifndef VECTOR_GEP_TAKES_SCALARS
          isa<InsertElementInst>(*UserIt) || // uniform address is broadcasted
          isa<ShuffleVectorInst>(*UserIt) || // uniform address is broadcasted
#endif
          isa<BitCastInst>(*UserIt))
        if (replaceValueWithStoredValue(*UserIt, Loaded))
          return true;
    return false;
  }
};
char RemoveTempScalarizingAllocas::ID = 0;

/// @brief Temporary utility pass to scalarize gathers and scatters.
///
/// Until CG handles IR gathers/scatters correctly on all platforms
/// we use this pass to scalarize such calls at IR level.
struct ScatterGatherScalarizer : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  ScatterGatherScalarizer() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    std::set<Instruction *> gathers, scatters;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction *currInst = &*I;
      if (isGather(currInst)) {
        gathers.insert(&*I);
      } else if (isScatter(currInst)) {
        scatters.insert(&*I);
      }
    }
    if (gathers.empty() && scatters.empty())
      return false;
    for(auto gather : gathers)
      scalarizeGather(cast<CallInst>(gather)); 
    for(auto scatter : scatters)
      scalarizeScatter(cast<CallInst>(scatter));
    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
  }

private:

  bool isGather(Instruction *insn) {
    return isIntrinsic(insn, Intrinsic::masked_gather);
  }

  bool isScatter(Instruction *insn) {
    return isIntrinsic(insn, Intrinsic::masked_scatter);
  }

  bool isIntrinsic(Instruction *insn, Intrinsic::ID ID) {
    if (insn->getOpcode() != Instruction::Call)
      return false;
    CallInst *CI = cast<CallInst>(insn);
    //StringRef FnName = CI->getCalledFunction()->getName();
    Function *F = CI->getCalledFunction();
    Intrinsic::ID IntrinsicID = F->getIntrinsicID();
    return (IntrinsicID == ID);
  }

  void scalarizeScatter(CallInst *ScatterInsn) {
    LLVMContext& context = ScatterInsn->getContext();
    IRBuilder<> Builder(ScatterInsn);
    /* process the arguments of the scatter call */
    unsigned numArguments = ScatterInsn->getNumArgOperands();
    assert(numArguments == 4 && "argument error");
    /* Values argument */
    Value *Values = ScatterInsn->getArgOperand(0); 
	Type *VectorTy = Values->getType();
    assert(isa<VectorType>(VectorTy));
	unsigned NumElements = VectorTy->getVectorNumElements();
    /* Ptrs argument */
    Value *Ptrs = ScatterInsn->getArgOperand(1); 
	/* Alignment argument */
	Value *Align = ScatterInsn->getArgOperand(2);
    ConstantInt *Alignment = dyn_cast<ConstantInt>(Align);
	assert(Alignment && "expected alignment to be a constant");
	/* Mask argument */
	Value *Mask = ScatterInsn->getArgOperand(3);
    Type *VTy = Mask->getType();
    assert(isa<VectorType>(VTy));
	assert(VTy->getVectorNumElements() == NumElements && "mask type doesn't match");

	//assert(VTy is same type as VectorTy) // todo

	/* Create the following sequence for each element:
            v = extractelement(value, index)
            if (mask[index] == true)
               store(v, ptrs[index])
    */
    Instruction *insertPoint = ScatterInsn;
    Builder.SetInsertPoint(insertPoint); // checkme: needed?
	for (unsigned index = 0; index < (unsigned)NumElements; ++index) {
      // m = extractelement (Mask, index)
      Value *constIndex = ConstantInt::get(Type::getInt32Ty(context), index);

      Builder.SetInsertPoint(insertPoint); // checkme: needed?
      Value *newEE = Builder.CreateExtractElement(Mask, constIndex, "extractMask"); 
      //ExtractElementInst::Create(Mask, constIndex, "extractMask"); 
      assert(newEE && "extract creation failure"); //checkme: needed?

      // if (m)
      Value *Cmp = Builder.CreateICmp(ICmpInst::ICMP_EQ, newEE, ConstantInt::get(newEE->getType(), 1)); //boolean compare?
      TerminatorInst *ThenTerm = SplitBlockAndInsertIfThen(Cmp,
                                                           insertPoint,
                                                           false);
      IRBuilder<> ThenBuilder(ThenTerm);
      Builder.SetInsertPoint(insertPoint); // checkme

      // then: store (extractelement(values,index),
      //              extractelement(ptrs,index))
      Value* ScalarValue = ThenBuilder.CreateExtractElement(Values,
                                                            constIndex,
                                                            "scalarizedValue");    
      Value* ScalarPtr = ThenBuilder.CreateExtractElement(Ptrs,
                                                          constIndex,
                                                          "scalarizedPtr");    
	  StoreInst *newSI = ThenBuilder.CreateStore(ScalarValue, ScalarPtr);
      newSI->setAlignment((Alignment->getValue()).getZExtValue()); 
    }

    ScatterInsn->eraseFromParent(); //checkme: Remove original instruction?
  }

  /// @brief Scalarize a gather intrinsic call
  ///
  /// %res = call <4 x double> @llvm.masked.gather.v4f64(<4 x double*> %ptrs, 
  ///                                                    i32 8,
  ///                                                    <4 x i1>%mask, 
  ///                                                    <4 x double> <true, true, true, true>)
  ///
  ///		for i=0 to 8
  /// 			pi = extract pointer from gep vector
  ///			elemi = load (pi)
  ///			res_vec = insert_element (res_vec, elemi, i)
  ///	res_vec should replace original result vector
  /// @param GatherInsn Gather to scalarize
  void scalarizeGather(CallInst *GatherInsn) {
    LLVMContext& context = GatherInsn->getContext();
    IRBuilder<> Builder(GatherInsn);
	Type *VectorTy = GatherInsn->getType();
    assert(isa<VectorType>(VectorTy));
    Type *ElemTy = (cast<VectorType>(VectorTy))->getElementType();
	unsigned NumElements = VectorTy->getVectorNumElements(); 
    /* process the arguments of the gather call */
    unsigned numArguments = GatherInsn->getNumArgOperands();
    assert(numArguments == 4 && "argument error");
    /* Ptrs argument */
    Value *Ptrs = GatherInsn->getArgOperand(0); 
	/* Alignment argument */
	Value *Align = GatherInsn->getArgOperand(1);
    ConstantInt *Alignment = dyn_cast<ConstantInt>(Align);
	assert(Alignment && "expected alignment to be a constant");
	/* Mask argument */
	Value *Mask = GatherInsn->getArgOperand(2);
    Type *VTy = Mask->getType();
    assert(isa<VectorType>(VTy));
	assert(VTy->getVectorNumElements() == NumElements && "mask type doesn't match");
	/* Passthru argument */
	Value *Passthru = GatherInsn->getArgOperand(3);
    VTy = Passthru->getType();
    assert(isa<VectorType>(VTy));
	assert(VTy->getVectorNumElements() == NumElements && "mask type doesn't match");
	//assert(VTy is same type as VectorTy) // todo

	/* Create the following sequence for each element:
            if (mask[index] == true)
               tmp = load(ptrs[index])
            else
               tmp = passthru[index]
            v=insertelement(v,tmp,indx)   
    */
    UndefValue *undefVect = UndefValue::get(VectorTy);
    Value *prevResult = undefVect;
    Instruction *insertPoint = GatherInsn;
    Builder.SetInsertPoint(insertPoint); // checkme: needed?
	for (unsigned index = 0; index < (unsigned)NumElements; ++index) {
      // m = extractelement (Mask, index)
      Value *constIndex = ConstantInt::get(Type::getInt32Ty(context), index);

      Builder.SetInsertPoint(insertPoint); // checkme: needed?
      Value *newEE = Builder.CreateExtractElement(Mask, constIndex, "extractMask"); 
      //ExtractElementInst::Create(Mask, constIndex, "extractMask"); 
      assert(newEE && "extract creation failure"); //checkme: needed?

      // if (m)
      Value *Cmp = Builder.CreateICmp(ICmpInst::ICMP_EQ, newEE, ConstantInt::get(newEE->getType(), 1)); //boolean compare?
      TerminatorInst *ThenTerm, *ElseTerm;
      SplitBlockAndInsertIfThenElse(Cmp, insertPoint, &ThenTerm, &ElseTerm);
      IRBuilder<> ThenBuilder(ThenTerm);
      IRBuilder<> ElseBuilder(ElseTerm);
      Builder.SetInsertPoint(insertPoint); // checkme

      // then: tmp = load ( extractelement(ptrs,index) ) 
      newEE = ThenBuilder.CreateExtractElement(Ptrs, constIndex, "extractPtr");    
      assert(newEE && "extract creation failure"); //checkme: needed?
	  LoadInst *newLI = ThenBuilder.CreateLoad(newEE, "scalarizedGather");
      newLI->setAlignment((Alignment->getValue()).getZExtValue()); 
      assert(newLI && "load creation failure"); //checkme: needed?

      // else: tmp = extractelement(passthru,index) 
      newEE = ElseBuilder.CreateExtractElement(Passthru, constIndex, "extractPassthru");    
      assert(newEE && "extract creation failure"); //checkme: needed?
          
      // tail: t = phi(then_tmp,else_tmp)
      //       v = inserelement(v,t,index) 
      PHINode *elmntOrPassthruVal =
        Builder.CreatePHI(ElemTy, 2, "elmntOrPassthruVal");
      elmntOrPassthruVal->addIncoming(newLI, ThenTerm->getParent());
      elmntOrPassthruVal->addIncoming(newEE, ElseTerm->getParent());
      Value *insertVal = elmntOrPassthruVal; //checkme
      Instruction *newIE = InsertElementInst::Create(prevResult,
                                                     insertVal,
                                                     constIndex,
                                                     "temp.vect"); //fixme!! use builder
      assert(newIE && "insert creation failure"); // checkme: needed?
      //VectorizerUtils::SetDebugLocBy(newIE, GatherInst); //checkme
      newIE->insertBefore(insertPoint);

      prevResult = newIE;
    }
        
    GatherInsn->replaceAllUsesWith(prevResult);
    GatherInsn->eraseFromParent(); //checkme: Remove original instruction?
  }
};
char ScatterGatherScalarizer::ID = 0;

void Vectorizer::vectorizeFunction(Function& F) {
  // Function-wide (vectorization)
  V_PRINT(VectorizerCore, "\nBefore vectorization passes!\n");

  Module *M = F.getParent();
  legacy::FunctionPassManager fpm(M);

  BuiltinLibInfo* pBuiltinInfoPass =
    (BuiltinLibInfo*)createBuiltinLibInfoPass(M, "");
  fpm.add(pBuiltinInfoPass);

  // Register PrepareLoopForVectorization
  fpm.add(new PrepareLoopForVectorization());

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
#if 0
  FunctionPass *simplifyGEP = createSimplifyGEPPass();
  fpm.add(simplifyGEP);
#endif

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
//  fpm.add(createZMMResolverPass()); // TODO
  fpm.add(createX86ResolverPass());

  fpm.add(new CollapseOuterLoop());

  // At this point we no longer the vectorization hints.
  fpm.add(new RemoveVectorizationHints);

//  fpm.add(createLoopUnrollPass()); // TODO: cost model still crashes
  fpm.add(new RemoveTempScalarizingAllocas());
  fpm.add(new ScatterGatherScalarizer()); // TODO: remove when CG can scalarize

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

// This function is the IR vectorizer's version of VPO's insertBeginRegion()
// function for marking the begining of a vectorized region.
static void insertBeginRegion(Module& M, Function *Clone,
                              VectorVariant &V, BasicBlock *EntryBlock)
{
  // Insert directive indicating the beginning of a SIMD loop.
  CallInst *SIMDBeginCall = VPOUtils::createDirectiveCall(M, "dir.simd");
  SIMDBeginCall->insertBefore(EntryBlock->getTerminator());

  // Now, split into its own basic block and insert the remaining intrinsics
  // after this one.
  BasicBlock *BeginRegionBlock =
      EntryBlock->splitBasicBlock(SIMDBeginCall, "simd.begin.region");

  // Insert vectorlength directive
  Constant *VL = ConstantInt::get(Type::getInt32Ty(Clone->getContext()),
                                  V.getVlen());

  Constant* ISAArg =
    ConstantDataArray::getString(Clone->getContext(),
                                 VectorVariant::ISAClassToString(V.getISA()),
                                 false);
  CallInst *ISACall =
      VPOUtils::createDirectiveQualOpndCall(M, "dir.qual.simd.isa", ISAArg);
  ISACall->insertAfter(SIMDBeginCall);

  CallInst *VlenCall =
      VPOUtils::createDirectiveQualOpndCall(M, "dir.qual.simd.vlen", VL);
  VlenCall->insertAfter(ISACall);

  CallInst *QualEndCall = VPOUtils::createDirectiveQualCall(M, "dir.qual.end");
  QualEndCall->insertBefore(BeginRegionBlock->getTerminator());
}

// Create a function implementing a given vector variant of a scalar function.
// The resulting function will have the signature induced by the vector variant
// but will still use a scalar loop to compute the N instances.
// For example, for a scalar function
//   Tr f(uniform T1 p1, consecutive T2 p2, random T3 p3);
// annotated with some vectorlength=VL, the function will have the following
// schematic structure:
//   <VL x Tr> fv(T1 p1, T2 p2, <VL x T1> p3) {
//     T3 s3[VL];
//     Tr sr[VL];
//     <VL x Tr> rv;
//     s3 <- p3;
//     intel.directive("dir.simd");
//     intel.directive("dir.simd.qual.opnd", VL);
//     intel.directive("dir.qual.end");
//     for (i = 0; i < VL; ++i)
//       sr[i] = f(p1, p2+i, s3[i]); // inlined
//     rv <- sr;
//     return rv;
//   }
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
      
  wrapperFunc->setCallingConv(CallingConv::X86_RegCall);
  BasicBlock* entryBB = BasicBlock::Create(wrapperFunc->getContext(),
					   "wrapper.entry",
					   wrapperFunc);

  // Create the loop.
  IntegerType* int32Type = Type::getInt32Ty(wrapperFunc->getContext());
  Value* zero = ConstantInt::get(int32Type, 0);
  Value* one = ConstantInt::get(int32Type, 1);
  unsigned vlen = vectorVariant.getVlen();
  Value* vlenVal = ConstantInt::get(int32Type, vlen);
  string SIMDLoopName = "simd_loop";
  loopRegion SIMDLoop = LoopUtils::createLoop(entryBB, entryBB, zero, one,
                                              vlenVal, SIMDLoopName,
                                              wrapperFunc->getContext());

  // Store vector arguments into arrays so loop is fully scalar, i.e. contains
  // '%s = %v[%i]' access patterns instead of '%s = extractelement %v, %i'.
  Instruction* insAtPH = SIMDLoop.m_preHeader->getTerminator();
  BasicBlock::iterator insAtLB = entryBB->getFirstInsertionPt();
  FunctionType::param_iterator wft_it = vectorFunctionType->param_begin();
  FunctionType::param_iterator wft_end = vectorFunctionType->param_end();
  Function::arg_iterator wfa_it = wrapperFunc->arg_begin();
  Function::arg_iterator ofa_it = scalarFunction.arg_begin();
  vkIt = parameterKinds.begin();
  std::vector<Value*> callArguments;
  ArrayRef<Value*> loopIndex(SIMDLoop.m_indVar);
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
            ithStride = BinaryOperator::CreateMul(SIMDLoop.m_indVar,
                                                  strideValue,
                                                  stride_name.str(),
                                                  insAtLB);
          }
          else
            ithStride = SIMDLoop.m_indVar;
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
    AllocaInst* vecStorage = new AllocaInst(vectorType, "__wrapper__.arg", insAtPH);
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
    ReturnInst::Create(wrapperFunc->getContext(), SIMDLoop.m_exit);
  else
  {
    // Return as vector a temporary array to which we'll assign the return
    // values of the scalar call to.
    Value* tmpRetVal = new AllocaInst(vectorReturnType, "__wrapper__.ret", insAtPH);
    ReturnInst::Create(wrapperFunc->getContext(),
                       new LoadInst(tmpRetVal, "retVal", SIMDLoop.m_exit),
                       SIMDLoop.m_exit);
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

  // Annotate the loop as to-be-vectorized (we only need 'begin region')
  insertBeginRegion(*wrapperFunc->getParent(),
                    wrapperFunc,
                    vectorVariant,
                    SIMDLoop.m_preHeader);

  // Inline the wrapper call
  InlineFunctionInfo ifi;
  bool inlined = llvm::InlineFunction(call, ifi, nullptr, false);
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

