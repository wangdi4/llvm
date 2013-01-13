/*********************************************************************************************
* Copyright © 2010, Intel Corporation
* Subject to the terms and conditions of the Master Development License
* Agreement between Intel and Apple dated August 26, 2005; under the Intel
* CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
********************************************************************************************/

#include "VectorizerCore.h"
#include "InstCounter.h"
#include "VectorizerCommon.h"

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <iomanip>
#include <sstream>

using namespace llvm;

char intel::VectorizerCore::ID = 0;

extern "C" FunctionPass* createScalarizerPass(bool);
extern "C" FunctionPass* createPhiCanon();
extern "C" FunctionPass* createPredicator();
extern "C" FunctionPass* createSimplifyGEPPass();
extern "C" FunctionPass* createPacketizerPass(bool);


extern "C" FunctionPass* createAppleWIDepPrePacketizationPass();
#ifndef __APPLE__
extern "C" FunctionPass* createGatherScatterResolverPass();
#endif
extern "C" FunctionPass* createX86ResolverPass();
extern "C" FunctionPass* createOCLBuiltinPreVectorizationPass();
extern "C" FunctionPass* createWeightedInstCounter(bool, Intel::CPUId);
extern "C" FunctionPass *createIRPrinterPass(std::string dumpDir, std::string dumpName);


static FunctionPass* createResolverPass(const Intel::CPUId& CpuId) {
#ifndef __APPLE__
  if (CpuId.HasGatherScatter()) return createGatherScatterResolverPass();
#endif
  return createX86ResolverPass();
}

static FunctionPass* createScalarizer(const Intel::CPUId& CpuId) {
  return createScalarizerPass(CpuId.HasGatherScatter());
}

static FunctionPass* createPacketizer(const Intel::CPUId& CpuId) {
  return createPacketizerPass(CpuId.HasGatherScatter());
}

namespace intel {

static const bool enableDebugPrints = false;
static raw_ostream &dbgPrint() {
  static raw_null_ostream devNull;
  return enableDebugPrints ? errs() : devNull;
}
VectorizerCore::VectorizerCore(const OptimizerConfig* pConfig) :
FunctionPass(ID),
m_runtimeServices(RuntimeServices::get()),
m_pConfig(pConfig)
{
  assert(m_runtimeServices && "unintialized runtime services");
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

  // No runtime services so we can not vectorizer.
  if (!m_runtimeServices)   {
    V_PRINT(VectorizerCore, "Failed to find runtime module. Aborting!\n");
    return false;
  }


  bool autoVec =  (m_pConfig->GetTransposeSize() == 0);
  V_ASSERT(m_pConfig->GetTransposeSize() <= MAX_PACKET_WIDTH && "unssupported vector width");

  // Emulate the entire pass-chain right here //
  //////////////////////////////////////////////
  V_PRINT(VectorizerCore, "\nBefore preparations!\n");
  // Function-wide (preparations)
  {
    FunctionPassManager fpm1(M);
    TargetData *TD = new TargetData(M);
    fpm1.add(TD);

    // Register lowerswitch
    fpm1.add(createLowerSwitchPass());

    // Register Scalarizer
    fpm1.add(createScalarReplAggregatesPass(1024));
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
    // irreducible control-flow, so a defensive check appears to be neccesary.
    VectorizationPossibilityPass* vecPossiblity = new VectorizationPossibilityPass();
    fpm1.add(vecPossiblity);

    fpm1.run(F);

    // Decide on preliminary width.
    // If the kernel is not vectorizaeble, leave it as 0.
    // Otherwise, look at the congiruation. If the configuration says 0,
    // the width is set automatically, otherwise manually.
    if (vecPossiblity->isVectorizable()) {
      if(autoVec) {
         V_ASSERT(preCounter && "pre counter should be initialzied");
         m_packetWidth = preCounter->getDesiredWidth();
         m_preWeight = preCounter->getWeight();
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
  V_ASSERT(m_packetWidth ==4 || m_packetWidth == 8 || m_packetWidth ==16);
  m_runtimeServices->setPacketizationWidth(m_packetWidth);

  // Function-wide (vectorization)
  V_PRINT(VectorizerCore, "\nBefore vectorization passes!\n");
  {
    FunctionPassManager fpm2(M);

    // Register predicate
    FunctionPass *predicate = createPredicator();
    fpm2.add(predicate);

    // Register mem2reg
    FunctionPass *mem2reg = createPromoteMemoryToRegisterPass();
    fpm2.add(mem2reg);

    // Register DCE
    FunctionPass *dce = createDeadCodeEliminationPass();
    fpm2.add(dce);

    // WI-dep pass
#ifdef __APPLE__
    FunctionPass *widDepPass = createAppleWIDepPrePacketizationPass();
    fpm2.add(widDepPass);
#endif

    if (m_pConfig->GetCpuId().HasGatherScatter()) {
      // Register simplifyGEP only is GatherScatter is supported
      FunctionPass *simplifyGEP = createSimplifyGEPPass();
      fpm2.add(simplifyGEP);
    }

    // Register packetize
    FunctionPass *packetize = createPacketizer(m_pConfig->GetCpuId());
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
#ifndef __APPLE__
      // TODO:: support patterns generated by instcombine in LoopWIAnalysis so
      // it will be able to identify strided values which are important in apple
      // for stream samplers handling.
      fpm2.add(createInstructionCombiningPass());
#endif

    fpm2.add(createCFGSimplificationPass());
    fpm2.add(createPromoteMemoryToRegisterPass());
    fpm2.add(createAggressiveDCEPass());
    if (m_pConfig->GetDumpHeuristicIRFlag()) {
      fpm2.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "vec_end"));
    }

    fpm2.doInitialization();
    fpm2.run(F);

    if (autoVec)  {
      V_ASSERT(postCounter && "uninitialized postCounter");
      m_postWeight = postCounter->getWeight();
      float Ratio = (float)m_postWeight / m_preWeight;
      int attemptedWidth = m_packetWidth;
      if (Ratio >= WeightedInstCounter::RATIO_MULTIPLIER * m_packetWidth) {
        m_packetWidth = 1;
        m_isFunctionVectorized = false;
        return true;
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
  // If we reach the end of the function this means we choose to vectorize the kernel!!
  m_isFunctionVectorized = true;
  return true;
}

} //namespace intel

extern "C"
intel::VectorizerCore *createVectorizerCorePass(const intel::OptimizerConfig* pConfig=NULL)
{
  return new intel::VectorizerCore(pConfig);
}
