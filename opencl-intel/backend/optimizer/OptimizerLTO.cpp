// Copyright 2021-2022 Intel Corporation.
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
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"
#include "llvm/Transforms/Intel_OpenCLTransforms/FMASplitter.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/InferAddressSpaces.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/Intel_RemoveRegionDirectives.h"
#include "llvm/Transforms/Scalar/JumpThreading.h"
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
#include "llvm/Transforms/Vectorize/VectorCombine.h"

#include "SPIRVLowerConstExpr.h"

cl::opt<DebugLogging> DebugPM(
    "debug-pass-manager", cl::Hidden, cl::ValueOptional,
    cl::desc("Print pass management debugging information"),
    cl::init(DebugLogging::None),
    cl::values(
        clEnumValN(DebugLogging::Normal, "", ""),
        clEnumValN(DebugLogging::Quiet, "quiet",
                   "Skip printing info about analyses"),
        clEnumValN(
            DebugLogging::Verbose, "verbose",
            "Print extra information about adaptors and pass managers")));

cl::opt<bool> VerifyEachPass("verify-each-pass", cl::Hidden,
                             cl::desc("Verify IR after each pass"),
                             cl::init(false));

// If set, then optimization passes will process functions as if they have the
// optnone attribute.
extern bool DPCPPForceOptnone;

extern cl::opt<bool> DisableVPlanCM;
extern cl::opt<bool> EmitKernelVectorizerSignOn;
extern cl::opt<bool> EnableO0Vectorization;

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerLTO::OptimizerLTO(Module &M, SmallVectorImpl<Module *> &RtlModuleList,
                           const intel::OptimizerConfig &Config)
    : Optimizer(M, RtlModuleList, Config) {}

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
  bool DebugPassManager = DebugPM != DebugLogging::None;
  PrintPassOptions PrintPassOpts;
  PrintPassOpts.Verbose = DebugPM == DebugLogging::Verbose;
  PrintPassOpts.SkipAnalyses = DebugPM == DebugLogging::Quiet;
  StandardInstrumentations SI(DebugPassManager, VerifyEachPass, PrintPassOpts);
  SI.registerCallbacks(PIC);
  PassBuilder PB(TM, PTO, PGOOpt, &PIC);

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  // Register the AA manager first so that our version is the one used.
  FAM.registerPass([&] {
    AAManager AAM = PB.buildDefaultAAPipeline();
    if (Config.EnableOCLAA())
      AAM.registerFunctionAnalysis<DPCPPAliasAnalysis>();
    return AAM;
  });

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

  // Set custom DiagnosticHandler callback.
  setDiagnosticHandler(LogStream);

  MPM.run(m_M, MAM);
}

void OptimizerLTO::registerPipelineStartCallback(PassBuilder &PB) {
  PB.registerPipelineStartEPCallback(
      [this](ModulePassManager &MPM, OptimizationLevel Level) {
        MPM.addPass(DPCPPPreprocessSPIRVFriendlyIRPass());
        MPM.addPass(SPIRVLowerConstExprPass());
        MPM.addPass(SPIRVToOCL20Pass());
        MPM.addPass(NameAnonGlobalPass());
        MPM.addPass(SpecializeConstantPass());

#ifndef NDEBUG
        MPM.addPass(VerifierPass());
#endif // #ifndef NDEBUG

        MPM.addPass(DPCPPEqualizerPass());

        Triple TargetTriple(m_M.getTargetTriple());
        if (TargetTriple.isArch64Bit() &&
            TargetTriple.isOSWindows())
          MPM.addPass(CoerceWin64TypesPass());

        if (m_IsFpgaEmulator)
          MPM.addPass(RemoveAtExitPass());

        MPM.addPass(SetPreferVectorWidthPass(
            VectorizerCommon::getCPUIdISA(Config.GetCpuId())));

        if (m_IsSPIRV && Config.GetRelaxedMath())
          MPM.addPass(createModuleToFunctionPassAdaptor(AddFastMathPass()));

        MPM.addPass(DuplicateCalledKernelsPass());

        if (Level != OptimizationLevel::O0)
          MPM.addPass(InternalizeNonKernelFuncPass());

        MPM.addPass(createModuleToFunctionPassAdaptor(FMASplitterPass()));

        MPM.addPass(AddFunctionAttrsPass());

        if (Level != OptimizationLevel::O0) {
          FunctionPassManager FPM;
          FPM.addPass(SimplifyCFGPass());
          if (Level == OptimizationLevel::O1)
            FPM.addPass(PromotePass());
          else
            FPM.addPass(SROAPass());
          FPM.addPass(InstCombinePass());
          FPM.addPass(InstSimplifyPass());
          MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
        }

        // Flatten get_{local, global}_linear_id()
        if (m_IsOcl20)
          MPM.addPass(LinearIdResolverPass());

        if (m_IsFpgaEmulator) {
          MPM.addPass(DPCPPRewritePipesPass());
          MPM.addPass(ChannelPipeTransformationPass());
          MPM.addPass(PipeIOTransformationPass());
          MPM.addPass(PipeOrderingPass());
          MPM.addPass(AutorunReplicatorPass());
        }

        FunctionPassManager FPM;
        // Static resolution of generic address space pointers
        if (Level != OptimizationLevel::O0 && (m_IsOcl20 || m_IsSPIRV)) {
          FPM.addPass(PromotePass());
          FPM.addPass(
              InferAddressSpacesPass(CompilationUtils::ADDRESS_SPACE_GENERIC));
          MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
        }

        FPM.addPass(BuiltinCallToInstPass());
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

        // Adding IPSCCPPass to the beginning to propogate constant. Otherwise
        // the opportunity may disappear when EarlyCSE pass is run first.
        if (Level != OptimizationLevel::O0)
          MPM.addPass(IPSCCPPass());
      });
}

void OptimizerLTO::registerOptimizerEarlyCallback(PassBuilder &PB) {
  PB.registerOptimizerEarlyEPCallback(
      [&](ModulePassManager &MPM, OptimizationLevel Level) {
        MPM.addPass(DetectRecursionPass());

        // PipeSupport can fail if dynamic pipe access is discovered after LLVM
        // optimizations.
        if (m_IsFpgaEmulator)
          MPM.addPass(PipeSupportPass());

        if (Level != OptimizationLevel::O0) {
          FunctionPassManager FPM;
          FPM.addPass(ReassociatePass());
          if (m_IsOcl20 || m_IsSPIRV) {
            // Repeat resolution of generic address space pointers after LLVM
            // IR was optimized.
            FPM.addPass(InferAddressSpacesPass(
                CompilationUtils::ADDRESS_SPACE_GENERIC));
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
          if (Config.GetTransposeSize() != 1 &&
              (Level != OptimizationLevel::O0 || EnableO0Vectorization)) {
            FPM.addPass(SinCosFoldPass());
            // Replace 'div' and 'rem' instructions with calls to optimized
            // library functions
            FPM.addPass(MathLibraryFunctionsReplacementPass());
            FPM.addPass(UnifyFunctionExitNodesPass());
            FPM.addPass(SimplifyCFGPass(SimplifyCFGOptions()
                                            .bonusInstThreshold(1)
                                            .forwardSwitchCondToPhi(false)
                                            .convertSwitchToLookupTable(false)
                                            .needCanonicalLoops(true)
                                            .sinkCommonInsts(true)));
            FPM.addPass(InstCombinePass());
            FPM.addPass(GVNHoistPass());
            FPM.addPass(DCEPass());
          }
          MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
        }

        MPM.addPass(ResolveVarTIDCallPass());

        if (m_IsSYCL) {
          MPM.addPass(TaskSeqAsyncHandling());

          // Support matrix fill and slice.
          MPM.addPass(ResolveMatrixFillPass());
          MPM.addPass(ResolveMatrixLayoutPass());
          MPM.addPass(ResolveMatrixWISlicePass());
        }

        if (Level != OptimizationLevel::O0)
          MPM.addPass(InferArgumentAliasPass());

        MPM.addPass(DPCPPKernelAnalysisPass());

        if (Level != OptimizationLevel::O0) {
          MPM.addPass(createModuleToFunctionPassAdaptor(SimplifyCFGPass()));
          MPM.addPass(WGLoopBoundariesPass());
          FunctionPassManager FPM;
          FPM.addPass(DCEPass());
          FPM.addPass(SimplifyCFGPass());
          MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
          MPM.addPass(DeduceMaxWGDimPass());
        }

        // Should be called before vectorizer!
        MPM.addPass(InstToFuncCallPass(ISA));

        if (Config.GetTransposeSize() == 1 ||
            (Level == OptimizationLevel::O0 && !EnableO0Vectorization))
          return;

        // In profiling mode remove llvm.dbg.value calls before vectorizer.
        if (Config.GetProfilingFlag())
          MPM.addPass(ProfilingInfoPass());

        MPM.addPass(ReqdSubGroupSizePass());

        // Analyze and set VF for kernels. This pass may throw
        // VFAnalysisDiagInfo error if VF checking fails.
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
        if ((Level == OptimizationLevel::O0 && !EnableO0Vectorization) ||
            Config.GetTransposeSize() == 1)
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
    if (Config.GetTransposeSize() != 1 &&
        (Level != OptimizationLevel::O0 || EnableO0Vectorization)) {
      // Post-vectorizer cleanup.
      MPM.addPass(DPCPPKernelPostVecPass());
      if (Level != OptimizationLevel::O0) {
        FunctionPassManager FPM;
        FPM.addPass(InstCombinePass());
        FPM.addPass(SimplifyCFGPass());
        FPM.addPass(PromotePass());
        FPM.addPass(ADCEPass());
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
      }
      // Add cost model to discard vectorized kernels if they have higher
      // cost. This is done only for native OpenCL program. In SYCL, unless
      // programmer explicitly asks not to vectorize (SG size of 1, OCL env
      // to disable vectorization, etc), compiler shall vectorize along the
      // fastest moving dimension (that maps to get_global_id(0) for LLVM IR
      // in our implementation). The vec/no-vec decision belongs to the
      // programmer.
      if (!m_IsSYCL && !DisableVPlanCM &&
          Config.GetTransposeSize() == TRANSPOSE_SIZE_NOT_SET)
        MPM.addPass(VectorKernelEliminationPass());

      MPM.addPass(HandleVPlanMask(&getVPlanMaskedFuncs()));
    } else {
      // When forced VF equals 1 or in O0 case, check subgroup semantics AND
      // prepare subgroup_emu_size for sub-group emulation.
      MPM.addPass(ReqdSubGroupSizePass());
      MPM.addPass(SetVectorizationFactorPass(ISA));
    }

    MPM.addPass(ResolveSubGroupWICallPass(/*ResolveSGBarrier*/ false));

    FunctionPassManager FPM;
    if (Level != OptimizationLevel::O0)
      FPM.addPass(OptimizeIDivAndIRemPass());
    FPM.addPass(PreventDivCrashesPass());
    if (Level != OptimizationLevel::O0) {
      // We need InstructionCombining and GVN passes after PreventDivCrashes
      // passes to optimize redundancy introduced by those passes
      FPM.addPass(InstCombinePass());
      FPM.addPass(GVNPass());
      FPM.addPass(VectorCombinePass());
      // In specACCEL/124, InstCombine may generate a cross-barrier bool value
      // used as a condition of a 'br' instruction, which leads to performance
      // degradation. JumpThreading eliminates the cross-barrier value.
      FPM.addPass(JumpThreadingPass());
    }
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

    // The m_debugType enum and profiling flag are mutually exclusive, with
    // precedence given to m_debugType.
    if (Config.GetProfilingFlag())
      MPM.addPass(ProfilingInfoPass());

    if (Level != OptimizationLevel::O0 && Config.GetStreamingAlways())
      MPM.addPass(createModuleToFunctionPassAdaptor(AddNTAttrPass()));
    if (m_debugType == intel::Native)
      MPM.addPass(ImplicitGIDPass(/*HandleBarrier*/ false));
    MPM.addPass(DPCPPKernelWGLoopCreatorPass(m_UseTLSGlobals));

    // Can't run loop unroll between WGLoopCreator and LoopIdiom for scalar
    // workload, which can benefit from LoopIdiom.
    // TODO wen can consider move this unroll into ScalarOptimizerLate callback.
    if (UnrollLoops && Level != OptimizationLevel::O0 &&
        Config.GetTransposeSize() != 1) {
      // unroll loops with non-constant trip count
      const int thresholdBase = 16;
      int RTLoopUnrollFactor = Config.GetRTLoopUnrollFactor();
      if (RTLoopUnrollFactor > 1) {
        LoopUnrollOptions UnrollOpts(Level.getSpeedupLevel());
        const unsigned threshold = thresholdBase * RTLoopUnrollFactor;
        // RTLoopUnrollFactor is to customize Count. However, LoopUnrollOptions
        // doesn't allow the customization.
        UnrollOpts.setPartial(false).setRuntime(true).setThreshold(threshold);
        MPM.addPass(
            createModuleToFunctionPassAdaptor(LoopUnrollPass(UnrollOpts)));
      }
    }

    // Resolve __intel_indirect_call for scalar kernels.
    MPM.addPass(IndirectCallLowering());

    FunctionPassManager FPM2;
    // Clean up scalar kernel after WGLoop for native subgroups.
    if (Level != OptimizationLevel::O0) {
      FPM2.addPass(DCEPass());
      FPM2.addPass(SimplifyCFGPass());
    }
    // Barrier pass can't work with a token type, so here we remove region
    // directives
    FPM2.addPass(RemoveRegionDirectivesPass());
    FPM2.addPass(UnifyFunctionExitNodesPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));

    if (m_IsFpgaEmulator)
      MPM.addPass(InfiniteLoopCreatorPass());

    addBarrierPasses(MPM, Level);

    // After adding loops run loop optimizations.
    if (Level != OptimizationLevel::O0) {
      // Add LoopSimplify pass before CLBuiltinLICM pass as CLBuiltinLICM pass
      // requires loops in Simplified Form.
      FunctionPassManager FPM;
      FPM.addPass(PromotePass());
      FPM.addPass(LoopSimplifyPass());
      LoopPassManager LPM;
      LPM.addPass(LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                           /*AllowSpeculation*/ true));
      LPM.addPass(BuiltinLICMPass());
      LPM.addPass(LoopStridedCodeMotionPass());
      FPM.addPass(
          createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true,
                                          /*UseBlockFrequencyInfo=*/true));
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    }

    if (Config.GetRelaxedMath())
      MPM.addPass(RelaxedMathPass());

    // The following three passes (AddTLSGlobals/AddImplicitArgs,
    // ResolveWICall, LocalBuffer) must run before BuiltinImport pass!
    if (m_UseTLSGlobals)
      MPM.addPass(AddTLSGlobalsPass());
    else
      MPM.addPass(AddImplicitArgsPass());
    MPM.addPass(ResolveWICallPass(Config.GetUniformWGSize(), m_UseTLSGlobals));
    MPM.addPass(LocalBuffersPass(m_UseTLSGlobals));

    // clang converts OCL's local to global.
    // LocalBuffersPass changes the local allocation from global to a
    // kernel argument.
    // The next pass GlobalOptPass cleans the unused global
    // allocation in order to make sure we will not allocate redundant space on
    // the jit
    if (Level != OptimizationLevel::O0 && m_debugType != intel::Native)
      MPM.addPass(GlobalOptPass());

#ifdef _DEBUG
    MPM.addPass(VerifierPass());
#endif

    // Externalize globals if IR is generated from OpenMP offloading. Now we
    // cannot get address of globals with internal/private linkage from LLJIT
    // (by design), but it's necessary by OpenMP to pass address of declare
    // target variables to the underlying OpenCL Runtime via
    // clSetKernelExecInfo. So we have to externalize globals for IR generated
    // from OpenMP.
    if (m_IsOMP)
      MPM.addPass(ExternalizeGlobalVariablesPass());

    MPM.addPass(BuiltinImportPass(CPUPrefix));
    if (Level != OptimizationLevel::O0) {
      // After the globals used in built-ins are imported - we can internalize
      // them with further wiping them out with GlobalDCE pass
      MPM.addPass(InternalizeGlobalVariablesPass());
    }
    // Need to convert shuffle calls to shuffle IR before running inline pass
    // on built-ins
    MPM.addPass(createModuleToFunctionPassAdaptor(BuiltinCallToInstPass()));

#ifdef _DEBUG
    MPM.addPass(VerifierPass());
#endif

    if (Level != OptimizationLevel::O0) {
      auto InlineParams = getInlineParams();
      InlineParams.DefaultThreshold = 4096;
      MPM.addPass(ModuleInlinerWrapperPass(InlineParams));
    } else if (m_IsOcl20) {
      // Ensure that the built-in functions to be processed by
      // PatchCallbackArgsPass are inlined.
      MPM.addPass(AlwaysInlinerPass());
    }
    // Some built-in functions contain calls to external functions which take
    // arguments that are retrieved from the function's implicit arguments.
    // Currently only applies to OpenCL 2.x
    if (m_IsOcl20)
      MPM.addPass(PatchCallbackArgsPass(m_UseTLSGlobals));

    if (Level != OptimizationLevel::O0) {
      // Clean up internal globals. ModuleInlinerWrapperPass doesn't discard
      // internal lib function, e.g. udiv, which is inlined and now unused.
      MPM.addPass(GlobalDCEPass());
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
    }

    MPM.addPass(PrepareKernelArgsPass(m_UseTLSGlobals));

    if (Level != OptimizationLevel::O0) {
      // These passes come after PrepareKernelArgs pass to eliminate the
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
  if (Level != OptimizationLevel::O0 || EnableO0Vectorization) {
    MPM.addPass(ReplaceScalarWithMaskPass());

    // Resolve subgreoup call introduced by ReplaceScalarWithMask pass.
    MPM.addPass(ResolveSubGroupWICallPass(/*ResolveSGBarrier*/ false));

    if (Level != OptimizationLevel::O0) {
      FunctionPassManager FPM;
      FPM.addPass(DCEPass());
      FPM.addPass(SimplifyCFGPass());
      FPM.addPass(PromotePass());
      FPM.addPass(PhiCanonicalization());
      FPM.addPass(RedundantPhiNode());
      MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    }
  }

  MPM.addPass(GroupBuiltinPass());
  MPM.addPass(BarrierInFunction());
  // Only run this when not debugging or when not in native (gdb) debugging
  if (m_debugType != intel::Native) {
    // This optimization removes debug information from extraneous barrier
    // calls by deleting them.
    MPM.addPass(RemoveDuplicatedBarrierPass(m_debugType == intel::Native));
  }

  // Begin sub-group emulation
  MPM.addPass(SGBuiltinPass(getVectInfos()));
  MPM.addPass(SGBarrierPropagatePass());
  MPM.addPass(SGBarrierSimplifyPass());
  // Insert ImplicitGIDPass in the middle of subgroup emulation to track GIDs in
  // emulation loops
  // TODO: Refine and re-enable ImplicitGID pass
  // if (m_debugType == intel::Native)
  //   MPM.addPass(ImplicitGIDPass(/*HandleBarrier*/ true));

  // Resume sub-group emulation
  MPM.addPass(SGValueWidenPass());
  MPM.addPass(SGLoopConstructPass());
#ifdef _DEBUG
  MPM.addPass(VerifierPass());
#endif
  // End sub-group emulation

  // Resolve subgroup barriers after subgroup emulation passes
  MPM.addPass(ResolveSubGroupWICallPass(/*ResolveSGBarrier*/ true));
  MPM.addPass(SplitBBonBarrier());
  if (Level != OptimizationLevel::O0)
    MPM.addPass(ReduceCrossBarrierValuesPass());
  MPM.addPass(KernelBarrier(m_debugType == intel::Native, m_UseTLSGlobals));
#ifdef _DEBUG
  MPM.addPass(VerifierPass());
#endif
}

void OptimizerLTO::registerLastPasses(ModulePassManager &MPM) {
  MPM.addPass(CleanupWrappedKernelPass());
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
