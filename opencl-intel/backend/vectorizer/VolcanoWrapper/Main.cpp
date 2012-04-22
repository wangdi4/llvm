/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

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
#include "RuntimeServices.h"
#include "X86Lower.h"
#include "Packetizer.h"
#include "Resolver.h"
#include "MICResolver.h"
#include "WIAnalysis.h"
#include "VecHeuristics.h"
#include "VecConfig.h"
#include "TargetArch.h"

// Placeholders for debug log files
FILE * prtFile;
FILE * moduleDmp;

using namespace llvm;

namespace llvm {
    extern Pass *createFunctionInliningPass(int Threshold);
}

char intel::Vectorizer::ID = 0;

extern "C" FunctionPass* createScalarizerPass();
extern "C" FunctionPass* createPhiCanon();
extern "C" FunctionPass* createPredicator();
extern "C" FunctionPass* createPacketizerPass(bool);
extern "C" FunctionPass* createMICResolverPass();
extern "C" FunctionPass* createX86ResolverPass();
extern "C" FunctionPass *createOCLBuiltinPreVectorizationPass();
extern "C" Pass *createSpecialCaseBuiltinResolverPass();


static FunctionPass* createResolverPass(const Intel::CPUId& CpuId) {
  if (CpuId.IsMIC()) return createMICResolverPass();
  return createX86ResolverPass();
}


static FunctionPass* createPacketizer(const Intel::CPUId& CpuId) {
  return createPacketizerPass(CpuId.IsMIC());
}


namespace intel {

  class PrintIRPass : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    PrintIRPass() : FunctionPass(ID) {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "PrintIRPass";
    }

    virtual bool runOnFunction(Function &F) {
      F.dump();
      return false;
    }
    llvm::FunctionPass *createPrintIRPass();
};
}
extern "C" {
  FunctionPass* createPrintIRPass() {
    return new intel::PrintIRPass();
  }
}
char intel::PrintIRPass::ID = 0;


namespace intel {


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
        MDString *tag = dyn_cast<MDString>(MDVTH->getOperand(0));
        
        if (tag->getString() == "vec_type_hint") {
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
            // Note the vector is initialized with zeroes. Later if we that the kernel will
            // be packetized this vector will be updated with the packet-width.
            // In the end we will erase all the kernels with 0 width in this vector.
            m_targetFunctionsWidth.push_back(0);
        }

        // Emulate the entire pass-chain right here //
        //////////////////////////////////////////////

        // Load wrappers module from user's root
        ///////

        V_PRINT(wrapper, "\nBefore inlining!\n");

        // Module-wide (inlining)
        {
            PassManager mpm1;

            // Register inliner
            Pass *inlinerPass = createFunctionInliningPass(4096);
            mpm1.add(inlinerPass);

            mpm1.run(M);
        }
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
            FunctionPass *scalarizer = createScalarizerPass();
            fpm1.add(createScalarReplAggregatesPass(1024));
            fpm1.add(createInstructionCombiningPass());
            fpm1.add(createOCLBuiltinPreVectorizationPass());
            fpm1.add(scalarizer);

            // Register mergereturn
            FunctionPass *mergeReturn = new UnifyFunctionExitNodes();
            fpm1.add(mergeReturn);

            // Register phiCanon
            FunctionPass *phiCanon = createPhiCanon();
            fpm1.add(phiCanon);

            // Loop over vectorized kernels and run passes
            for (unsigned i = 0; i < m_numOfKernels; i++)
            {
                Function *funcToProcess = m_targetFunctionsList[i];
                if (funcToProcess) 
                    fpm1.run(*(funcToProcess));
            }
        }
        
        V_PRINT(wrapper, "\nBefore loop simplify!\n");
        // Simplify loops
        {
            PassManager mpm;
            mpm.add(createLoopSimplifyPass());
            mpm.run(M);
        }

        VectorizationHeuristics* vhe = new VectorizationHeuristics(
            m_pConfig->GetCpuId());
        FunctionPassManager fpm(&M);
        fpm.add(createDeadCodeEliminationPass());
        fpm.add(vhe);
        for (unsigned i = 0; i < m_numOfKernels; i++)    
        {
          Function *funcToProcess = m_targetFunctionsList[i];
          if (!funcToProcess) {
            // note that targetFunctionsWidth is initialized with 0
            continue;
          }

          fpm.run(*funcToProcess);

          // When finally choosing the vectorization width, the following rules apply by order:
          // 1. If it is not safe to vectorize (mayv==false), just leave m_targetFunctionsWidth[i] 
          //     with 0 so it will not be vectorize at all.
          // 2. If the configuration specifies a vectorization width, use that width.
          // 3. Otherwise, Use the recommended vectorization width by the Hueristics.
          if (vhe->mayVectorize())
          {
            if( 0 == m_pConfig->GetTransposeSize())
            {
              m_targetFunctionsWidth[i] = vhe->getVectorSize();
            }
            else
            {
              // Disregard Hueristics and use enforced width
              m_targetFunctionsWidth[i] = m_pConfig->GetTransposeSize();
            }
          }

          vhe->reset();
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
            
            // Register resolve
            FunctionPass *resolver = createResolverPass(m_pConfig->GetCpuId());
            fpm2.add(resolver);

            fpm2.add(createInstructionCombiningPass());
            fpm2.add(createCFGSimplificationPass());
            //if (cpuId.HasAVX1()) {
            //  fpm2.add(new intel::X86Lower(X86Lower::AVX));
            //} else if (cpuId.HasSSE41() || cpuId.HasSSE42()) {
            //  fpm2.add(new intel::X86Lower(X86Lower::SSE4));
            //} else if (cpuId.HasSSE2()){
            //  fpm2.add(new intel::X86Lower(X86Lower::SSE2));
            //}
            fpm2.add(createPromoteMemoryToRegisterPass());
            fpm2.add(createAggressiveDCEPass());
            fpm2.add(createInstructionCombiningPass());
            fpm2.add(createDeadCodeEliminationPass());

            RuntimeServices *RTS = RuntimeServices::get();
            // Loop over vectorized kernels and run passes
            for (unsigned i = 0; i < m_numOfKernels; i++)
            {
              Function *funcToProcess = m_targetFunctionsList[i];
              if (m_targetFunctionsWidth[i])
              {
                // Update the RTS with the selected packet size
                RTS->setPacketizationWidth(m_targetFunctionsWidth[i]);
                fpm2.doInitialization();
                fpm2.run(*(funcToProcess));
              }
            }
        }



        // If vectorization was aborted - make sure to erase the cloned kernel
        for (unsigned i = 0; i < m_numOfKernels; i++)
        {
          V_ASSERT(1 != m_targetFunctionsWidth[i] && "No vectorization width should be 0");
          if (m_targetFunctionsList[i] && 0 == m_targetFunctionsWidth[i]) 
          {
            m_targetFunctionsList[i]->eraseFromParent();
            m_targetFunctionsList[i] = NULL;
            m_scalarFuncsList[i] = NULL;
          }

          m_optimizerFunctions->push_back(m_targetFunctionsList[i]);
          m_optimizerWidths->push_back(m_targetFunctionsWidth[i]);
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

