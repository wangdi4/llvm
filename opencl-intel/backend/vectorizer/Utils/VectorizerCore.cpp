/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "Vectorizer"
#include "VectorizerCore.h"
#include "InstCounter.h"
#include "VectorizerCommon.h"
#include "OclTune.h"
#include "ChooseVectorizationDimension.h"

#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <iomanip>
#include <sstream>

using namespace llvm;

char intel::VectorizerCore::ID = 0;

extern "C" FunctionPass* createScalarizerPass(const Intel::CPUId& CpuId);
extern "C" FunctionPass* createPhiCanon();
extern "C" FunctionPass* createPredicator();
extern "C" FunctionPass* createSimplifyGEPPass();
extern "C" FunctionPass* createPacketizerPass(const Intel::CPUId&, unsigned int);
extern "C" intel::ChooseVectorizationDimension* createChooseVectorizationDimension();
extern "C" Pass* createBuiltinLibInfoPass(llvm::SmallVector<llvm::Module*, 2> pRtlModuleList, std::string type);

extern "C" FunctionPass* createAVX512ResolverPass();
extern "C" FunctionPass* createX86ResolverPass(Intel::ECPU cpuArch);
extern "C" FunctionPass* createOCLBuiltinPreVectorizationPass();
extern "C" FunctionPass* createWeightedInstCounter(bool, Intel::CPUId);
extern "C" FunctionPass *createIRPrinterPass(std::string dumpDir, std::string dumpName);


static FunctionPass* createResolverPass(const Intel::CPUId& CpuId) {
  if (CpuId.HasAVX512())
    return createAVX512ResolverPass();
  return createX86ResolverPass(CpuId.GetCPU());
}

static FunctionPass* createScalarizer(const Intel::CPUId& CpuId) {
  return createScalarizerPass(CpuId);
}

static FunctionPass* createPacketizer(const Intel::CPUId& CpuId,
                                      unsigned int vectorizationDimension) {
  return createPacketizerPass(CpuId, vectorizationDimension);
}

namespace intel {

static const bool enableDebugPrints = false;
static raw_ostream &dbgPrint() {
  static raw_null_ostream devNull;
  return enableDebugPrints ? errs() : devNull;
}
VectorizerCore::VectorizerCore(const OptimizerConfig* pConfig) :
FunctionPass(ID),
m_pConfig(pConfig)
{
}

VectorizerCore::~VectorizerCore()
{
}

unsigned VectorizerCore::getPacketWidth()
{
  return m_packetWidth;
}

bool VectorizerCore::isFunctionVectorized() {
  return m_isFunctionVectorized;
}

unsigned int VectorizerCore::getVectorizationDim() {
  return m_vectorizationDim;
}

bool VectorizerCore::getCanUniteWorkgroups() {
  return m_canUniteWorkgroups;
}

void VectorizerCore::setScalarFunc(Function* F) {
  m_scalarFunc = F;
}

bool VectorizerCore::runOnFunction(Function &F) {
  // Before doing anything set default return values, function was not vectorized
  // width of 0.
  m_isFunctionVectorized = false;
  m_packetWidth = 0;

  // Case the config was not set quit gracefully.
  // TODO: add default config or find another solutiuon for config options.
  if (!m_pConfig) {
    return false;
  }


  Module *M = F.getParent();
  V_PRINT(VectorizerCore, "\nEntered VectorizerCore Wrapper!\n");

  bool autoVec =  (m_pConfig->GetTransposeSize() == 0);
  V_ASSERT(m_pConfig->GetTransposeSize() <= MAX_PACKET_WIDTH && "unssupported vector width");

  std::map<BasicBlock*, int> preVectorizationCosts; // used for statiscal purposes.
  // Emulate the entire pass-chain right here //
  //////////////////////////////////////////////
  V_PRINT(VectorizerCore, "\nBefore preparations!\n");
  // Function-wide (preparations)
  {
    legacy::FunctionPassManager fpm1(M);
    fpm1.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));

    // Register lowerswitch
    fpm1.add(createLowerSwitchPass());

    fpm1.add(createSROAPass());
    fpm1.add(createInstructionCombiningPass());
    fpm1.add(createOCLBuiltinPreVectorizationPass());
    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm1.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_scalarizer"));
    fpm1.add(createDeadCodeEliminationPass());

    WeightedInstCounter* preCounter = NULL;
    if (autoVec) {
      preCounter = (WeightedInstCounter*)createWeightedInstCounter(true, m_pConfig->GetCpuId());
      fpm1.add(preCounter);
    }
    fpm1.add(createScalarizer(m_pConfig->GetCpuId()));

    // Register mergereturn
    FunctionPass *mergeReturn = new UnifyFunctionExitNodes();
    fpm1.add(mergeReturn);

    // Register phiCanon
    FunctionPass *phiCanon = createPhiCanon();
    fpm1.add(phiCanon);

    // Simplify loops
    // This must happen after phiCanon since phi canonization can undo
    // loop simplification by breaking dedicated exit nodes.
    fpm1.add(createLoopSimplifyPass());

    fpm1.add(createDeadCodeEliminationPass());
    // Need to check for vectorization possibly AFTER phi canonization.
    // In theory this shouldn't matter, since we should never introduce anything
    // that prohibits vectorization in these three passes.
    // In practice, however, phi canonization already had a bug that introduces
    // irreducible control-flow, so a defensive check appears to be necessary.
    VectorizationPossibilityPass* vecPossiblity = new VectorizationPossibilityPass();
    fpm1.add(vecPossiblity);

    // choose the vectorization dimension (if vectorized). Usually zero,
    // but in some unusual cases we choose differently (to gain performance.)
    ChooseVectorizationDimension* chooser = createChooseVectorizationDimension();
    // Todo: remove next line once the metadata bug is solved and the scalar func
    // will be accessible through through the metadata.
    chooser->setScalarFunc(m_scalarFunc);
    fpm1.add(chooser);


    fpm1.run(F);

    // Decide on preliminary width.
    // If the kernel is not vectorizable, leave it as 0.
    // Otherwise, look at the configuration. If the configuration says 0,
    // the width is set automatically, otherwise manually.
    if (vecPossiblity->isVectorizable()) {
      m_vectorizationDim = chooser->getVectorizationDim();
      m_canUniteWorkgroups = chooser->getCanUniteWorkgroups();
      if(autoVec) {
         V_ASSERT(preCounter && "pre counter should be initialzied");
         m_packetWidth = preCounter->getDesiredWidth();
         m_preWeight = preCounter->getWeight();
         // for statistical purposes, we need to keep the
         // prevectorization costs until after vectorization.
         preCounter->copyBlockCosts(&preVectorizationCosts);
      } else {
         m_packetWidth = m_pConfig->GetTransposeSize();
      }
    } else {
      // Unsafe to vectorize the function should quit now.
      m_isFunctionVectorized = false;
      m_packetWidth = 0;
      return true;
    }

  }

  // Sanity in debug check that packetization with is supported.
  V_ASSERT(m_packetWidth == 4 || m_packetWidth == 8 || m_packetWidth == 16);

  // Function-wide (vectorization)
  V_PRINT(VectorizerCore, "\nBefore vectorization passes!\n");
  {
    legacy::FunctionPassManager fpm2(M);
    BuiltinLibInfo* pBuiltinInfoPass = (BuiltinLibInfo*)
      createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), "");
    pBuiltinInfoPass->getRuntimeServices()->setPacketizationWidth(m_packetWidth);
    fpm2.add(pBuiltinInfoPass);

    // add WIAnalysis for the predicator.
    fpm2.add(new WIAnalysis(m_vectorizationDim));

    // Register predicate
    FunctionPass *predicate = createPredicator();
    fpm2.add(predicate);

    // Register mem2reg
    FunctionPass *mem2reg = createPromoteMemoryToRegisterPass();
    fpm2.add(mem2reg);

    // Register DCE
    FunctionPass *dce = createDeadCodeEliminationPass();
    fpm2.add(dce);

    // Add WIAnalysis for SimplifyGEP.
    fpm2.add(new WIAnalysis(m_vectorizationDim));

    // Register SimplifyGEP
    FunctionPass *simplifyGEP = createSimplifyGEPPass();
    fpm2.add(simplifyGEP);

    // add WIAnalysis for the packetizer.
    fpm2.add(new WIAnalysis(m_vectorizationDim));

    // Register packetize
    FunctionPass *packetize = createPacketizer(m_pConfig->GetCpuId(), m_vectorizationDim);
    fpm2.add(packetize);

    // Register DCE
    FunctionPass *dce2 = createDeadCodeEliminationPass();
    fpm2.add(dce2);

    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm2.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_resolver"));

    //We only need the "post" run if there's doubt about what to do.
    WeightedInstCounter* postCounter = NULL;
    if (autoVec) {
      postCounter = (WeightedInstCounter*) createWeightedInstCounter(false,  m_pConfig->GetCpuId());
      fpm2.add(postCounter);
    }

    //Register resolve
    FunctionPass *resolver = createResolverPass(m_pConfig->GetCpuId());
    fpm2.add(resolver);

    // Final cleaning up
    fpm2.add(createInstructionCombiningPass());

    fpm2.add(createCFGSimplificationPass());
    fpm2.add(createPromoteMemoryToRegisterPass());
    fpm2.add(createAggressiveDCEPass());
    if (m_pConfig->GetDumpHeuristicIRFlag()) {
      fpm2.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "vec_end"));
    }

    fpm2.doInitialization();
    fpm2.run(F);

    // If we reach the end of the function this means we choose to vectorize the kernel!!
    m_isFunctionVectorized = true;
    if (autoVec)  {
      V_ASSERT(postCounter && "uninitialized postCounter");
      // for statistical purposes:: //////////////////////////////////////////////
      postCounter->countPerBlockHeuristics(&preVectorizationCosts, m_packetWidth);
      preVectorizationCosts.clear();
      ////////////////////////////////////////////////////////////////////////////
      m_postWeight = postCounter->getWeight();
      float Ratio = (float)m_postWeight / m_preWeight;
      int attemptedWidth = m_packetWidth;
      if (Ratio >= WeightedInstCounter::RATIO_MULTIPLIER * m_packetWidth) {
        m_packetWidth = 1;
        m_isFunctionVectorized = false;
        Statistic::ActiveStatsT kernelStats;
        OCLSTAT_DEFINE(Vectorized_version_discarded,
            "Vectorized version was discarded since the scalar version seems to"
            " be better",kernelStats);
        Vectorized_version_discarded++;
        intel::Statistic::pushFunctionStats (kernelStats, F, DEBUG_TYPE);
      }
      if (enableDebugPrints) {
        dbgPrint() << "Function: " << F.getName() << "\n";
        dbgPrint() << "Pre count: " << (long long)m_preWeight << "\n";
        dbgPrint() << "Post count: " << (long long)m_postWeight << "\n";
        std::ostringstream os;
        os << std::setprecision(3) << Ratio;
        dbgPrint() << "Ratio: " << os.str() << "\n";
        dbgPrint() << "Attempted Width: " << attemptedWidth << "\n";
        dbgPrint() << "New Decision: " <<  (m_packetWidth ? m_packetWidth : 1) << "\n";
      }
    }
  }
  return true;
}

} //namespace intel

extern "C"
intel::VectorizerCore *createVectorizerCorePass(const intel::OptimizerConfig* pConfig=NULL)
{
  return new intel::VectorizerCore(pConfig);
}
