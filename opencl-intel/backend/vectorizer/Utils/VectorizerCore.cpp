// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "VectorizerCore.h"
#include "ChooseVectorizationDimension.h"
#include "InstCounter.h"
#include "VectorizerCommon.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WeightedInstCount.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <iomanip>
#include <sstream>

#ifndef DEBUG_TYPE
#define DEBUG_TYPE "Vectorizer"
#endif

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;
using CPUDetect = Intel::OpenCL::Utils::CPUDetect;

char intel::VectorizerCore::ID = 0;

extern "C" FunctionPass *createScalarizerPass(const CPUDetect *CpuId,
                                              bool InVPlanPipeline = false);
extern "C" FunctionPass* createPhiCanon();
extern "C" FunctionPass* createPredicator();
extern "C" FunctionPass* createSimplifyGEPPass();
extern "C" FunctionPass *createPacketizerPass(const CPUDetect *, unsigned int);
extern "C" intel::ChooseVectorizationDimension* createChooseVectorizationDimension();
extern "C" Pass *createBuiltinLibInfoPass(ArrayRef<Module *> pRtlModuleList,
                                          std::string type);

extern "C" FunctionPass* createAVX512ResolverPass();
extern "C" FunctionPass *
createX86ResolverPass(Intel::OpenCL::Utils::ECPU cpuArch);
extern "C" FunctionPass* createOCLBuiltinPreVectorizationPass();
extern "C" FunctionPass *createIRPrinterPass(std::string dumpDir, std::string dumpName);

static FunctionPass *createResolverPass(const CPUDetect *CpuId) {
  if (CpuId->HasAVX512Core())
    return createAVX512ResolverPass();
  return createX86ResolverPass(CpuId->GetCPU());
}

static FunctionPass *createScalarizer(const CPUDetect *CpuId) {
  return createScalarizerPass(CpuId);
}

static FunctionPass *createPacketizer(const CPUDetect *CpuId,
                                      unsigned int vectorizationDimension) {
  return createPacketizerPass(CpuId, vectorizationDimension);
}

namespace intel {
/// @returns False if we don't want to vectorize function due to some reasons
/// like usage of channels or infinite loops.
static bool isFunctionVectorizable(Function &F, LoopInfo &LI) {
  auto KMd = KernelMetadataAPI(&F);
  if (KMd.MaxGlobalWorkDim.hasValue() && KMd.MaxGlobalWorkDim.get() == 0)
    return false;

  DPCPPStatistic::ActiveStatsT KernelStats;

  for (auto *L: LI) {
    SmallVector<BasicBlock *, 16> ExitingBlocks;
    L->getExitingBlocks(ExitingBlocks);
    if (ExitingBlocks.empty()) {
      LLVM_DEBUG(dbgs() << "Function contains infinite loops, can not vectorize\n");
      DPCPP_STAT_DEFINE(
          CantVectInfLoops,
          "Unable to vectorizer because infinite loops are present",
          KernelStats);
      CantVectInfLoops++;
      DPCPPStatistic::pushFunctionStats(KernelStats, F, DEBUG_TYPE);

      return false;
    }
  }

  return true;
}

VectorizerCore::VectorizerCore(const OptimizerConfig* pConfig) :
  FunctionPass(ID), m_packetWidth(0), m_isFunctionVectorized(false),
  m_pConfig(pConfig), m_preWeight(0.0f), m_postWeight(0.0f),
  m_vectorizationDim(0), m_canUniteWorkgroups(false)
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

  // Let's do an early exit from Vectorizer in some cases releated to
  // single work-item kernels, infinite loops, channels
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  if (!isFunctionVectorizable(F, LI)) {
    return false;
  }

  V_PRINT(VectorizerCore, "\nEntered VectorizerCore Wrapper!\n");

  auto kmd = KernelMetadataAPI(&F);
  uint32_t forcedVecWidth = 0;
  // If global setting is not set check vec_len_hint attr
  if (TRANSPOSE_SIZE_NOT_SET != m_pConfig->GetTransposeSize())
    forcedVecWidth = (uint32_t)m_pConfig->GetTransposeSize();
  else
    forcedVecWidth = (kmd.hasVecLength()) ? kmd.getVecLength() : 0;

  // No need to vectorize kernel, exiting...
  if (1 == forcedVecWidth)
    return false;

  // Vector width specified by user is not valid for current arch,
  // fall back to autovectorization mode.
  if (Intel::OpenCL::Utils::SUPPORTED !=
      m_pConfig->GetCpuId()->isTransposeSizeSupported(
          (ETransposeSize)forcedVecWidth)) {
    forcedVecWidth = 0;
  }

  V_ASSERT(forcedVecWidth <= MAX_PACKET_WIDTH && "unssupported vector width");

  auto vkimd = KernelInternalMetadataAPI(&F);

  bool KernelHasSubgroups = vkimd.KernelHasSubgroups.get();

  Module *M = F.getParent();

  TargetMachine* targetMachine = m_pConfig->GetTargetMachine();
  TargetLibraryInfoImpl TLII(Triple(M->getTargetTriple()));

  ArrayRef<Module *> RtlModuleList = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                                         .getResult()
                                         .getBuiltinModules();
  std::map<BasicBlock*, int> preVectorizationCosts; // used for statiscal purposes.
  VFISAKind ISA =
      Intel::VectorizerCommon::getCPUIdISA(m_pConfig->GetCpuId());
  // Emulate the entire pass-chain right here //
  //////////////////////////////////////////////
  V_PRINT(VectorizerCore, "\nBefore preparations!\n");
  // Function-wide (preparations)
  {
    legacy::FunctionPassManager fpm1(M);
    // there maybe no TargetMachine, with RenderScript for instance
    if (targetMachine != nullptr)
      fpm1.add(createTargetTransformInfoWrapperPass(
                   targetMachine->getTargetIRAnalysis()));
    fpm1.add(new TargetLibraryInfoWrapperPass(TLII));
    fpm1.add(createBuiltinLibInfoAnalysisLegacyPass(RtlModuleList));
    fpm1.add(createBuiltinLibInfoPass(RtlModuleList, ""));

    // Register lowerswitch
    fpm1.add(createLowerSwitchPass());

    fpm1.add(createSROAPass());
    fpm1.add(createInstructionCombiningPass());
    fpm1.add(createOCLBuiltinPreVectorizationPass());
    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm1.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_scalarizer"));
    fpm1.add(createDeadCodeEliminationPass());

    WeightedInstCountAnalysisLegacy *preCounter = nullptr;
    if (!forcedVecWidth) {
      preCounter = (WeightedInstCountAnalysisLegacy *)
          llvm::createWeightedInstCountAnalysisLegacyPass(ISA, true);
      fpm1.add(preCounter);
    }
    fpm1.add(createScalarizer(m_pConfig->GetCpuId()));

    // Register mergereturn
    FunctionPass *mergeReturn = new UnifyFunctionExitNodesLegacyPass();
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
    fpm1.add(chooser);


    fpm1.doInitialization();
    fpm1.run(F);
    fpm1.doFinalization();

    // Decide on preliminary width.
    // If the kernel is not vectorizable, leave it as 0.
    // Otherwise, look at the configuration. If the configuration says 0,
    // the width is set automatically, otherwise manually.
    if (vecPossiblity->isVectorizable()) {
      m_vectorizationDim = chooser->getVectorizationDim();
      m_canUniteWorkgroups = chooser->getCanUniteWorkgroups();
      if(!forcedVecWidth) {
         V_ASSERT(preCounter && "pre counter should be initialized");
         m_packetWidth = preCounter->getResult().getDesiredVF();
         m_preWeight = preCounter->getResult().getWeight();
         // for statistical purposes, we need to keep the
         // prevectorization costs until after vectorization.
         preCounter->getResult().copyBlockCosts(&preVectorizationCosts);
      } else {
         m_packetWidth = forcedVecWidth;
      }
    } else {
      // Unsafe to vectorize the function should quit now.
      // In case of subroups this has to be a compilation error
      // We have to vectorize otherwise subgroup semantics is broken.
      V_ASSERT(!KernelHasSubgroups && "Subgroup code could not be vectorized!");
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
    // there may not be TTI available, with RenderScript driver, for instance.
    if (targetMachine != nullptr)
      fpm2.add(createTargetTransformInfoWrapperPass(
                   targetMachine->getTargetIRAnalysis()));
    fpm2.add(new TargetLibraryInfoWrapperPass(TLII));
    fpm2.add(createBuiltinLibInfoAnalysisLegacyPass(RtlModuleList));
    BuiltinLibInfo *pBuiltinInfoPass =
        (BuiltinLibInfo *)createBuiltinLibInfoPass(RtlModuleList, "");
    pBuiltinInfoPass->getRuntimeServices()->setPacketizationWidth(m_packetWidth);
    fpm2.add(pBuiltinInfoPass);

    // add WorkItemAnalysis for the predicator.
    fpm2.add(llvm::createWorkItemAnalysisLegacyPass(m_vectorizationDim));

    // Register predicate
    FunctionPass *predicate = createPredicator();
    fpm2.add(predicate);

    // Register mem2reg
    FunctionPass *mem2reg = createPromoteMemoryToRegisterPass();
    fpm2.add(mem2reg);

    // Register DCE
    FunctionPass *dce = createDeadCodeEliminationPass();
    fpm2.add(dce);

    // Add WorkItemAnalysis for SimplifyGEP.
    fpm2.add(llvm::createWorkItemAnalysisLegacyPass(m_vectorizationDim));

    // Register SimplifyGEP
    FunctionPass *simplifyGEP = createSimplifyGEPPass();
    fpm2.add(simplifyGEP);

    // add WorkItemAnalysis for the packetizer.
    fpm2.add(llvm::createWorkItemAnalysisLegacyPass(m_vectorizationDim));

    // Register packetize
    FunctionPass *packetize = createPacketizer(m_pConfig->GetCpuId(), m_vectorizationDim);
    fpm2.add(packetize);

    // Register DCE
    FunctionPass *dce2 = createDeadCodeEliminationPass();
    fpm2.add(dce2);

    // Register GVN to clean up duplicate indices for gather/scatter
    fpm2.add(createGVNPass(/*NoLoads*/ true));

    if (m_pConfig->GetDumpHeuristicIRFlag())
      fpm2.add(createIRPrinterPass(m_pConfig->GetDumpIRDir(), "pre_resolver"));

    //We only need the "post" run if there's doubt about what to do.
    WeightedInstCountAnalysisLegacy *postCounter = nullptr;
    if (!forcedVecWidth) {
      postCounter = (WeightedInstCountAnalysisLegacy *)
          llvm::createWeightedInstCountAnalysisLegacyPass(ISA, false);
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
    fpm2.doFinalization();

    // If we reach the end of the function this means we choose to vectorize the kernel!!
    m_isFunctionVectorized = true;
    if (!forcedVecWidth)  {
      V_ASSERT(postCounter && "uninitialized postCounter");
      // for statistical purposes:: //////////////////////////////////////////////
      postCounter->getResult().countPerBlockHeuristics(&preVectorizationCosts,
                                                       m_packetWidth);
      preVectorizationCosts.clear();
      ////////////////////////////////////////////////////////////////////////////
      m_postWeight = postCounter->getResult().getWeight();
      float Ratio = (float)m_postWeight / m_preWeight;
#ifndef NDEBUG
      int attemptedWidth = m_packetWidth;
#endif
      // TODO: I'm allowing forced vec width to override the subgroup size for now.
      // It's debatable how exactly that should behave.
      if ((Ratio >= InstCountResult::RatioMultiplier * m_packetWidth) &&
          (!KernelHasSubgroups)) {
        m_packetWidth = 1;
        m_isFunctionVectorized = false;
        DPCPPStatistic::ActiveStatsT kernelStats;
        DPCPP_STAT_DEFINE(
            Vectorized_version_discarded,
            "Vectorized version was discarded since the scalar version seems to"
            " be better",
            kernelStats);
        Vectorized_version_discarded++;
        DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
      }
      LLVM_DEBUG(dbgs() << "Function: " << F.getName() << "\n");
      LLVM_DEBUG(dbgs() << "Pre count: " << (long long)m_preWeight << "\n");
      LLVM_DEBUG(dbgs() << "Post count: " << (long long)m_postWeight << "\n");
      std::ostringstream os;
      os << std::setprecision(3) << Ratio;
      LLVM_DEBUG(dbgs() << "Ratio: " << os.str() << "\n");
      LLVM_DEBUG(dbgs() << "Attempted Width: " << attemptedWidth << "\n");
      LLVM_DEBUG(dbgs() << "New Decision: " <<  (m_packetWidth ? m_packetWidth : 1) << "\n");
    }
  }
  return true;
}

} //namespace intel

extern "C"
intel::VectorizerCore *createVectorizerCorePass(const intel::OptimizerConfig* pConfig=nullptr)
{
  return new intel::VectorizerCore(pConfig);
}
