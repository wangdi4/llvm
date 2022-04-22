// Copyright 2021 Intel Corporation.
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

#include "OptimizerLTO.h"
#include "SPIRVToOCL.h"
#include "VecConfig.h"
#include "VectorizerCommon.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#ifndef NDEBUG
#include "llvm/IR/Verifier.h"
#endif // #ifndef NDEBUG
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/InferAddressSpaces.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopDeletion.h"
#include "llvm/Transforms/Scalar/LoopIdiomRecognize.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/Vectorize/IntelMFReplacement.h"

#include "SPIRVLowerConstExpr.h"

// If set, then optimization passes will process functions as if they have the
// optnone attribute.
extern bool DPCPPForceOptnone;

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerLTO::OptimizerLTO(Module &M, SmallVector<Module *, 2> &RtlModuleList,
                           const intel::OptimizerConfig &Config,
                           bool DebugPassManager)
    : Optimizer(M, RtlModuleList, Config), DebugPassManager(DebugPassManager) {}

OptimizerLTO::~OptimizerLTO() {}

void OptimizerLTO::Optimize(raw_ostream &LogStream) {
  TargetMachine *TM = Config.GetTargetMachine();
  assert(TM && "Uninitialized TargetMachine!");

  PipelineTuningOptions PTO;
  PTO.LoopUnrolling = true;
  PTO.LoopInterleaving = false;
  PTO.LoopVectorization = false;
  PTO.SLPVectorization = false;
  PTO.MergeFunctions = false;
  PTO.CallGraphProfile = true;

  Optional<PGOOptions> PGOOpt;
  PassInstrumentationCallbacks PIC;
  PrintPassOptions PrintPassOpts;
  PrintPassOpts.Verbose = false;
  PrintPassOpts.SkipAnalyses = false;
  StandardInstrumentations SI(DebugPassManager, /*VerifyEachPass*/ false,
                              PrintPassOpts);
  SI.registerCallbacks(PIC);
  PassBuilder PB(TM, PTO, PGOOpt, &PIC);

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  // Register the AA manager first so that our version is the one used.
  FAM.registerPass([&] { return PB.buildDefaultAAPipeline(); });

  // Register the target library analysis directly and give it a customized
  // preset TLI.
  Triple TargetTriple(m_M.getTargetTriple());
  TLII.reset(new TargetLibraryInfoImpl(TargetTriple));
  FAM.registerPass([&] { return TargetLibraryAnalysis(*TLII); });

  // Register module analysis with custom parameters.
  MAM.registerPass([&] { return BuiltinLibInfoAnalysis(m_RtlModules); });

  // Register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  registerPipelineStartCallback(PB);
  registerOptimizerEarlyCallback(PB);
  registerVectorizerStartCallback(PB);
  registerOptimizerLastCallback(PB);

  ModulePassManager MPM;

  if (Config.GetDisableOpt())
    MPM = PB.buildO0DefaultPipeline(OptimizationLevel::O0);
  else
    MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O3);

  DPCPPForceOptnone = Config.GetDisableOpt();

  registerLastPasses(MPM);

  MPM.run(m_M, MAM);
}

void OptimizerLTO::registerPipelineStartCallback(PassBuilder &PB) {
  PB.registerPipelineStartEPCallback(
      [this](ModulePassManager &MPM, OptimizationLevel Level) {
        MPM.addPass(DPCPPPreprocessSPIRVFriendlyIRPass());
        MPM.addPass(SPIRVLowerConstExprPass());
        MPM.addPass(SPIRVToOCL20Pass());
        MPM.addPass(NameAnonGlobalPass());

#ifndef NDEBUG
        MPM.addPass(VerifierPass());
#endif // #ifndef NDEBUG

        if (Level != OptimizationLevel::O0 && (m_IsOcl20 || m_IsSPIRV))
          MPM.addPass(createModuleToFunctionPassAdaptor(InferAddressSpacesPass(
              DPCPPKernelCompilationUtils::ADDRESS_SPACE_GENERIC)));

        MPM.addPass(DPCPPEqualizerPass());
        Triple TargetTriple(m_M.getTargetTriple());
        if (TargetTriple.isArch64Bit() && TargetTriple.isOSWindows())
          MPM.addPass(CoerceWin64TypesPass());
        MPM.addPass(DuplicateCalledKernelsPass());
        if (Level != OptimizationLevel::O0)
          MPM.addPass(InternalizeNonKernelFuncPass());
        MPM.addPass(AddFunctionAttrsPass());
        MPM.addPass(LinearIdResolverPass());
        MPM.addPass(createModuleToFunctionPassAdaptor(BuiltinCallToInstPass()));
      });
}

void OptimizerLTO::registerOptimizerEarlyCallback(llvm::PassBuilder &PB) {
  PB.registerOptimizerEarlyEPCallback(
      [&](ModulePassManager &MPM, OptimizationLevel Level) {
        MPM.addPass(DPCPPKernelAnalysisPass());
        if (Level != OptimizationLevel::O0) {
          MPM.addPass(WGLoopBoundariesPass());
          FunctionPassManager FPM;
          FPM.addPass(DCEPass());
          FPM.addPass(SimplifyCFGPass());
          FPM.addPass(ReassociatePass());
          if (m_IsOcl20 || m_IsSPIRV) {
            // Repeat resolution of generic address space pointers after LLVM
            // IR was optimized.
            FPM.addPass(InferAddressSpacesPass(
                DPCPPKernelCompilationUtils::ADDRESS_SPACE_GENERIC));
            // Cleanup after InferAddressSpaces pass.
            FPM.addPass(SimplifyCFGPass());
            FPM.addPass(SROAPass());
            FPM.addPass(EarlyCSEPass());
            FPM.addPass(PromotePass());
            FPM.addPass(InstCombinePass());
            // No need to run function inlining pass here, because if there are
            // still non-inlined functions, then we don't have to inline new
            // ones.
          }
          if (Config.GetTransposeSize() != 1) {
            FPM.addPass(SinCosFoldPass());
            // Replace 'div' and 'rem' instructions with calls to optimized
            // library functions
            FPM.addPass(MathLibraryFunctionsReplacementPass());
            FPM.addPass(UnifyFunctionExitNodesPass());
          }
          MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
          MPM.addPass(DeduceMaxWGDimPass());
        }
        MPM.addPass(InstToFuncCallPass());

        if (Config.GetTransposeSize() == 1)
          return;

        VectorVariant::ISAClass ISA =
            Intel::VectorizerCommon::getCPUIdISA(Config.GetCpuId());
        // Analyze and set VF for kernels.
        MPM.addPass(SetVectorizationFactorPass(ISA));

        // Create and materialize "vector-variants" attribute.
        MPM.addPass(VectorVariantLowering(ISA));
        MPM.addPass(CreateSimdVariantPropagation());
        MPM.addPass(SGSizeCollectorPass(ISA));
        MPM.addPass(SGSizeCollectorIndirectPass(ISA));

        // We won't automatically switch vectorization dimension for SYCL.
        if (!m_IsSYCL)
          MPM.addPass(
              RequireAnalysisPass<VectorizationDimensionAnalysis, Module>());

        MPM.addPass(DPCPPKernelVecClonePass(getVectInfos(), ISA,
                                            !m_IsSYCL && !m_IsOMP));
        MPM.addPass(VectorVariantFillIn());
        MPM.addPass(UpdateCallAttrs());
      });
}

void OptimizerLTO::registerVectorizerStartCallback(PassBuilder &PB) {
  PB.registerVectorizerStartEPCallback(
      [this](FunctionPassManager &FPM, OptimizationLevel Level) {
        if (Level == OptimizationLevel::O0 || Config.GetTransposeSize() == 1)
          return;

        FPM.addPass(PromotePass());
        FPM.addPass(LoopSimplifyPass());
        FPM.addPass(createFunctionToLoopPassAdaptor(
            LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                     /*AllowSpeculation*/ true),
            /*UseMemorySSA=*/true, /*UseBlockFrequencyInfo=*/true));
      });
}

void OptimizerLTO::registerOptimizerLastCallback(PassBuilder &PB) {
  PB.registerOptimizerLastEPCallback([this](ModulePassManager &MPM,
                                            OptimizationLevel Level) {
    // Post-vectorizer cleanup.
    if (Config.GetTransposeSize() != 1) {
      MPM.addPass(DPCPPKernelPostVecPass());
      if (Level != OptimizationLevel::O0) {
        FunctionPassManager FPM;
        FPM.addPass(InstCombinePass());
        FPM.addPass(SimplifyCFGPass());
        FPM.addPass(PromotePass());
        FPM.addPass(ADCEPass());
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
      }
      MPM.addPass(HandleVPlanMask(&getVPlanMaskedFuncs()));
    }

    MPM.addPass(
        ResolveSubGroupWICallPass(m_RtlModules, /*ResolveSGBarrier*/ false));
    if (Level != OptimizationLevel::O0 && Config.GetStreamingAlways())
      MPM.addPass(createModuleToFunctionPassAdaptor(AddNTAttrPass()));
    MPM.addPass(DPCPPKernelWGLoopCreatorPass());

    // Can't run loop unroll between WGLoopCreator and LoopIdiom for scalar
    // workload, which would can from LoopIdiom.
    if (Level != OptimizationLevel::O0 && Config.GetTransposeSize() != 1) {
      LoopUnrollOptions UnrollOpts(Level.getSpeedupLevel());
      UnrollOpts.setPartial(false);
      UnrollOpts.setRuntime(true);
      MPM.addPass(
          createModuleToFunctionPassAdaptor(LoopUnrollPass(UnrollOpts)));
    }

    addBarrierPasses(MPM, Level);

    if (m_UseTLSGlobals)
      MPM.addPass(AddTLSGlobalsPass());
    else
      MPM.addPass(AddImplicitArgsPass());
    MPM.addPass(ResolveWICallPass(Config.GetUniformWGSize(), m_UseTLSGlobals));
    MPM.addPass(LocalBuffersPass(m_UseTLSGlobals));
    MPM.addPass(BuiltinImportPass(m_RtlModules, CPUPrefix));
    if (Level != OptimizationLevel::O0)
      MPM.addPass(GlobalDCEPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(BuiltinCallToInstPass()));
    if (Level != OptimizationLevel::O0) {
      auto InlineParams = getInlineParams();
      InlineParams.DefaultThreshold = 4096;
      MPM.addPass(ModuleInlinerWrapperPass(InlineParams));
      // AddImplicitArgs pass may create dead implicit arguments.
      MPM.addPass(DeadArgumentEliminationPass());
      FunctionPassManager FPM;
      FPM.addPass(SROAPass());
      FPM.addPass(LoopSimplifyPass());
      LoopPassManager LPM;
      LPM.addPass(LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                           /*AllowSpeculation*/ true));
      LPM.addPass(LoopIdiomRecognizePass());
      LPM.addPass(LoopDeletionPass());
      LPM.addPass(LoopStridedCodeMotionPass());
      FPM.addPass(
          createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true,
                                          /*UseBlockFrequencyInfo=*/true));
      FPM.addPass(SimplifyCFGPass());
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    } else {
      MPM.addPass(AlwaysInlinerPass());
    }
    MPM.addPass(PrepareKernelArgsPass(m_UseTLSGlobals));

    if (Level != OptimizationLevel::O0) {
      // These passes come after PrepareKernelArgsLegacyPass to eliminate the
      // redundancy produced by it.
      FunctionPassManager FPM;
      FPM.addPass(SimplifyCFGPass());
      FPM.addPass(SROAPass());
      FPM.addPass(InstCombinePass());
      FPM.addPass(GVNPass());
      FPM.addPass(DSEPass());
      FPM.addPass(ADCEPass());
      FPM.addPass(EarlyCSEPass());
      FPM.addPass(InstCombinePass());
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    }
  });
}

void OptimizerLTO::addBarrierPasses(ModulePassManager &MPM, OptimizationLevel Level) {
  if (Level != OptimizationLevel::O0) {
    // TODO: insert ReplaceScalarWithMask pass here
    // Resolve subgreoup call introduced by ReplaceScalarWithMask pass.
    MPM.addPass(
        ResolveSubGroupWICallPass(m_RtlModules, /*ResolveSGBarrier*/ false));
  }
  MPM.addPass(createModuleToFunctionPassAdaptor(PhiCanonicalization()));
  MPM.addPass(createModuleToFunctionPassAdaptor(RedundantPhiNode()));
  MPM.addPass(GroupBuiltinPass());
  MPM.addPass(BarrierInFunction());
   // Resolve subgroup barriers after subgroup emulation passes
  MPM.addPass(
      ResolveSubGroupWICallPass(m_RtlModules, /*ResolveSGBarrier*/ true));
  MPM.addPass(SplitBBonBarrier());
  if (Level != OptimizationLevel::O0)
    MPM.addPass(ReduceCrossBarrierValuesPass());
  MPM.addPass(KernelBarrier(m_debugType == intel::Native, m_UseTLSGlobals));
}

void OptimizerLTO::registerLastPasses(ModulePassManager &MPM) {
  MPM.addPass(CleanupWrappedKernelPass());
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
