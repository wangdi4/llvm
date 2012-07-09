/*********************************************************************************************
* Copyright © 2010, Intel Corporation
* Subject to the terms and conditions of the Master Development License
* Agreement between Intel and Apple dated August 26, 2005; under the Intel
* CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
*********************************************************************************************/

#include <iomanip>

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/IPO/InlinerPass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Pass.h"
#include "llvm/Linker.h"
#include "llvm/PassManager.h"
#include "Main.h"
#include "InstCounter.h"
#include "RuntimeServices.h"
#include "X86Lower.h"
#include "Packetizer.h"
#include "Resolver.h"
#include "MICResolver.h"
#include "WIAnalysis.h"
#include "VecConfig.h"
#include "IRPrinter.h"
#include "TargetArch.h"

// Placeholders for debug log files
FILE * prtFile;
FILE * moduleDmp;

using namespace llvm;

char intel::Vectorizer::ID = 0;

extern "C" FunctionPass* createScalarizerPass();
extern "C" FunctionPass* createPhiCanon();
extern "C" FunctionPass* createPredicator();
extern "C" FunctionPass* createPacketizerPass(bool);
extern "C" FunctionPass* createMICResolverPass();
extern "C" FunctionPass* createX86ResolverPass();
extern "C" FunctionPass* createOCLBuiltinPreVectorizationPass();
extern "C" Pass *createSpecialCaseBuiltinResolverPass();



static FunctionPass* createResolverPass(const Intel::CPUId& CpuId) {
  if (CpuId.IsMIC()) return createMICResolverPass();
  return createX86ResolverPass();
}

static FunctionPass* createPacketizer(const Intel::CPUId& CpuId) {
  return createPacketizerPass(CpuId.IsMIC());
}

namespace intel {

static const bool enableDebugPrints = false;
static raw_ostream &dbgPrint() {
  static raw_null_ostream devNull;
  return enableDebugPrints ? errs() : devNull;
}

Vectorizer::Vectorizer(const Module * rt, const OptimizerConfig* pConfig,
  SmallVectorImpl<Function*> &optimizerFunctions,
  SmallVectorImpl<int> &optimizerWidths) : 
  ModulePass(ID),
  m_runtimeModule(rt),
  m_numOfKernels(0),
  m_isModuleVectorized(false),
  m_pConfig(pConfig),
  m_optimizerFunctions(&optimizerFunctions),
  m_optimizerWidths(&optimizerWidths)
{
  // init debug prints
  initializeLoopInfoPass(*PassRegistry::getPassRegistry());
  V_INIT_PRINT;
}

Vectorizer::~Vectorizer()
{
  // Close the debug log elegantly
  V_DESTROY_PRINT;
}


bool Vectorizer::runOnModule(Module &M)
{
  V_PRINT(wrapper, "\nEntered Vectorizer Wrapper!\n");
  // set isVectorized and proper number of kernels to zero, in case vectorization fails
  m_numOfKernels = 0;
  m_isModuleVectorized = true;

  bool autoVec =  (m_pConfig->GetTransposeSize() == 0);

  // check for some common module errors, before actually diving in
  NamedMDNode *KernelsMD = M.getNamedMetadata("opencl.kernels");
  if (!KernelsMD)
  {
    V_PRINT(wrapper, "Failed to find annotation. Aborting!\n");
    return false;
  }
  m_numOfKernels = KernelsMD->getNumOperands();
  if (m_numOfKernels == 0)
  {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return false;
  }
  if (!m_runtimeModule)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }


  for (int i = 0, e = KernelsMD->getNumOperands(); i < e; i++) {
    MDNode *FuncInfo = KernelsMD->getOperand(i);
    Value *field0 = FuncInfo->getOperand(0)->stripPointerCasts();
    Function *F = dyn_cast<Function>(field0);
    bool disableVect = false;

    //look for vector type hint metadata
    for (int i = 1, e = FuncInfo->getNumOperands(); i < e; i++) {
      MDNode *MDVTH = dyn_cast<MDNode>(FuncInfo->getOperand(i));
      assert(MDVTH && "Malformed metadata!");

      MDString *tag = dyn_cast<MDString>(MDVTH->getOperand(0));
      assert(tag && "Malformed metadata!");

      if (tag && tag->getString() == "vec_type_hint") {
        // extract type
        Type *VTHTy = MDVTH->getOperand(1)->getType();

        if (!VTHTy->isFloatTy()     &&
          !VTHTy->isDoubleTy()    &&
          !VTHTy->isIntegerTy(8)  &&
          !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) &&
          !VTHTy->isIntegerTy(64)) {
            disableVect = true;
        }
        break;
      }
    }

    // Only add kernels to list, if they have scalar vec-type hint (or none)
    if (!disableVect)
      m_scalarFuncsList.push_back(F);
    else
      m_scalarFuncsList.push_back(NULL);
  }


  // Clone all kernels
  funcsVector::iterator fi = m_scalarFuncsList.begin();
  funcsVector::iterator fe = m_scalarFuncsList.end();
  for (; fi != fe; ++fi)
  {
    if (*fi)
    {
      Function *clone = CloneFunction(*fi);
      clone->setName("__Vectorized_." + (*fi)->getName());
      M.getFunctionList().push_back(clone);
      m_targetFunctionsList.push_back(clone);
    }
    else
    {
      // Keep empty entry for non-vectorized functions, to align with kernels list
      m_targetFunctionsList.push_back(NULL);
    }
    // Initialize vector with zeroes. 
    // The "pre" pass will put preferred values.
    m_targetFunctionsWidth.push_back(0);
      
    //This is extremely ugly, but I don't want to change the way things work, so..
    m_PreWeight.push_back(1);
    m_PostWeight.push_back(1);
  }

  // Emulate the entire pass-chain right here //
  //////////////////////////////////////////////

  // Load wrappers module from user's root
  ///////

  V_PRINT(wrapper, "\nBefore preparations!\n");
  // Function-wide (preparations)
  {
    FunctionPassManager fpm1(&M);

    TargetData *TD = new TargetData(&M);
    fpm1.add(TD);
    // Some basic optimizations
    fpm1.add(createPromoteMemoryToRegisterPass());
    fpm1.add(createInstructionCombiningPass());
    // Register lowerswitch
    fpm1.add(createLowerSwitchPass());

    // Register Scalarizer
    fpm1.add(createScalarReplAggregatesPass(1024));
    fpm1.add(createInstructionCombiningPass());
    fpm1.add(createOCLBuiltinPreVectorizationPass());            

    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm1.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_scalarizer"));

    fpm1.add(createDeadCodeEliminationPass());
    WeightedInstCounter* preCounter = new WeightedInstCounter(true, !autoVec, m_pConfig->GetCpuId());
    fpm1.add(preCounter);
    fpm1.add(createScalarizerPass());

    // Register mergereturn
    FunctionPass *mergeReturn = new UnifyFunctionExitNodes();
    fpm1.add(mergeReturn);

    // Register phiCanon
    FunctionPass *phiCanon = createPhiCanon();
    fpm1.add(phiCanon);
    fpm1.add(createDeadCodeEliminationPass());
    // Need to check for vectorization possibly AFTER phi canonization.
    // In theory this shouldn't matter, since we should never introduce anything
    // that prohibits vectorization in these three passes.
    // In practice, however, phi canonization already had a bug that introduces
    // irreducible control-flow, so a defensive check appears to be neccesary.
    VectorizationPossibilityPass* vecPossiblity = new VectorizationPossibilityPass();
    fpm1.add(vecPossiblity);

    // Loop over vectorized kernels and run passes
    for (unsigned i = 0; i < m_numOfKernels; i++)
    {
      Function *funcToProcess = m_targetFunctionsList[i];
      if (funcToProcess) 
      {
        fpm1.run(*(funcToProcess));

        // Decide on preliminary width.
        // If the kernel is not vectorizaeble, leave it as 0.
        // Otherwise, look at the congiruation. If the configuration says 0,
        // the width is set automatically, otherwise manually.
        if (vecPossiblity->isVectorizable())
        {
          if(autoVec)
          {
            m_targetFunctionsWidth[i] = preCounter->getDesiredWidth();
          }
          else
          {
            m_targetFunctionsWidth[i] = m_pConfig->GetTransposeSize();
          }
          m_PreWeight[i] = preCounter->getWeight();
        }
      }
    }
  }

  V_PRINT(wrapper, "\nBefore loop simplify!\n");
  // Simplify loops
  {
    PassManager mpm;
    mpm.add(createLoopSimplifyPass());
    mpm.run(M);
  }

  V_PRINT(wrapper, "\nBefore vectorization passes!\n");
  // Function-wide (vectorization)
  {
    Intel::CPUId cpuId = m_pConfig->GetCpuId();
    FunctionPassManager fpm2(&M);

    // Register predicate
    FunctionPass *predicate = createPredicator();
    fpm2.add(predicate);

    // Register mem2reg
    FunctionPass *mem2reg = createPromoteMemoryToRegisterPass();
    fpm2.add(mem2reg);

    // Register DCE
    FunctionPass *dce = createDeadCodeEliminationPass();
    fpm2.add(dce);

    // Register packetize
    FunctionPass *packetize = createPacketizer(m_pConfig->GetCpuId());
    fpm2.add(packetize);

    // Register DCE
    FunctionPass *dce2 = createDeadCodeEliminationPass();
    fpm2.add(dce2);

    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm2.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_resolver"));
      
    //We only need the "post" run if there's doubt about what to do.
    WeightedInstCounter* postCounter = new WeightedInstCounter(false, !autoVec, m_pConfig->GetCpuId());
    if (autoVec)
      fpm2.add(postCounter);

    //Register resolve
    FunctionPass *resolver = createResolverPass(m_pConfig->GetCpuId());
    fpm2.add(resolver);


    fpm2.add(createInstructionCombiningPass());
    fpm2.add(createCFGSimplificationPass());
    fpm2.add(createPromoteMemoryToRegisterPass());
    fpm2.add(createAggressiveDCEPass());
    fpm2.add(createInstructionCombiningPass());
    fpm2.add(createDeadCodeEliminationPass());

    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm2.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "vec_end"));

    RuntimeServices *RTS = RuntimeServices::get();
    // Loop over vectorized kernels and run passes
    for (unsigned i = 0; i < m_numOfKernels; i++)
    {
      Function *funcToProcess = m_targetFunctionsList[i];
      if (!funcToProcess)
        continue;

      V_STAT(
        V_PRINT(vectorizer_stat, "\n\n\n=========== Analyzing function: "<<funcToProcess->getName()<<" ==========\n");
      V_PRINT(vectorizer_stat_excel, "\n\n\n=========== Analyzing function: "<<funcToProcess->getName()<<" ==========\n");
      )
        if (m_targetFunctionsWidth[i])
        {
          // Update the RTS with the selected packet size
          RTS->setPacketizationWidth(m_targetFunctionsWidth[i]);
          fpm2.doInitialization();
          fpm2.run(*(funcToProcess));
          if (autoVec)
          {
            m_PostWeight[i] = postCounter->getWeight();               
            float Ratio = (float)m_PostWeight[i] / m_PreWeight[i];            

            int attemptedWidth = m_targetFunctionsWidth[i];
            if (Ratio >= WeightedInstCounter::RATIO_MULTIPLIER * m_targetFunctionsWidth[i])
                m_targetFunctionsWidth[i] = 0;

            if (enableDebugPrints) {
              dbgPrint() << "Function: " << m_scalarFuncsList[i]->getName() << "\n";
              dbgPrint() << "Pre count: " << (long long)m_PreWeight[i] << "\n";
              dbgPrint() << "Post count: " << (long long)m_PostWeight[i] << "\n";
              std::ostringstream os; 
              os << std::setprecision(3) << Ratio;
              dbgPrint() << "Ratio: " << os.str() << "\n";
              dbgPrint() << "Attempted Width: " << attemptedWidth << "\n";
              dbgPrint() << "New Decision: " <<  (m_targetFunctionsWidth[i] ? m_targetFunctionsWidth[i] : 1) << "\n";
            }
          }
        }
    }
  }

  // If vectorization was aborted - make sure to erase the cloned kernel
  for (unsigned i = 0; i < m_numOfKernels; i++)
  {
    V_ASSERT(1 != m_targetFunctionsWidth[i] && "No vectorization width should be 0, not 1");
    if (m_targetFunctionsList[i] && 0 == m_targetFunctionsWidth[i]) 
    {
      m_targetFunctionsList[i]->eraseFromParent();
      m_targetFunctionsList[i] = NULL;
      m_scalarFuncsList[i] = NULL;
    }

    m_optimizerFunctions->push_back(m_targetFunctionsList[i]);
    m_optimizerWidths->push_back(m_targetFunctionsWidth[i] ? m_targetFunctionsWidth[i] : 1);
  }

  {
    PassManager mpm;
    mpm.add(createSpecialCaseBuiltinResolverPass());
    mpm.run(M);
  }

  V_DUMP_MODULE((&M));
  //////////////////////////////////////////////
  //////////////////////////////////////////////
  V_PRINT(wrapper, "\nCompleted Vectorizer Wrapper!\n");

  return m_isModuleVectorized;
}

} // Namespace intel


///////////////////////////////////////////////////////////////////////////////////////////////////
// Interface functions for vectorizer
///////////////////////////////////////////////////////////////////////////////////////////////////
extern "C"
  Pass *createVectorizerPass(const Module *runtimeModule, const intel::OptimizerConfig* pConfig, 
  SmallVectorImpl<Function*> &optimizerFunctions,
  SmallVectorImpl<int> &optimizerWidths)
{    
  return new intel::Vectorizer(runtimeModule, pConfig,
    optimizerFunctions, optimizerWidths);
}

