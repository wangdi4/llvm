// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "OptimizerOCL.h"
#include "VecConfig.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#ifndef NDEBUG
#include "llvm/IR/Verifier.h"
#endif // #ifndef NDEBUG
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/ArgumentPromotion.h"
#include "llvm/Transforms/IPO/ConstantMerge.h"
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/IPO/StripDeadPrototypes.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"
#include "llvm/Transforms/Intel_OpenCLTransforms/FMASplitter.h"
#include "llvm/Transforms/Intel_VPO/VPODirectiveCleanup.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/IndVarSimplify.h"
#include "llvm/Transforms/Scalar/InferAddressSpaces.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/Intel_RemoveRegionDirectives.h"
#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopDeletion.h"
#include "llvm/Transforms/Scalar/LoopIdiomRecognize.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/MemCpyOptimizer.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"
#include "llvm/Transforms/Utils/LowerSwitch.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/VPO/Utils/CFGRestructuring.h"
#include "llvm/Transforms/Vectorize/IntelMFReplacement.h"
#include "llvm/Transforms/Vectorize/IntelVPlanDriver.h"
#include "llvm/Transforms/Vectorize/VectorCombine.h"

#include "SPIRVLowerConstExpr.h"
#include "SPIRVToOCL.h"

using namespace llvm;

extern cl::opt<DebugLogging> DebugPM;
extern cl::opt<bool> VerifyEachPass;
extern bool DPCPPForceOptnone;
extern cl::opt<bool> DisableVPlanCM;
extern cl::opt<bool> EnableO0Vectorization;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerOCL::OptimizerOCL(Module &M, SmallVectorImpl<Module *> &RtlModuleList,
                           const intel::OptimizerConfig &Config)
    : Optimizer(M, RtlModuleList, Config) {
  Level =
      Config.GetDisableOpt() ? OptimizationLevel::O0 : OptimizationLevel::O3;
}

void OptimizerOCL::Optimize(raw_ostream &LogStream) {
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

  // Several PMs are populated in the Optimizer flow:
  // Materializer, PreFail and PostFail PMs.
  // In order to dump IR for debug purposes one can schedule PrintModulePass
  // to a Pass Manager of choice.
  ModulePassManager MPM;
  materializerPM(MPM);
  populatePassesPreFailCheck(MPM);
  populatePassesPostFailCheck(MPM);

  // Set custom DiagnosticHandler callback.
  setDiagnosticHandler(LogStream);

  MPM.run(m_M, MAM);
}

void OptimizerOCL::materializerPM(ModulePassManager &MPM) const {
  if (m_IsSYCL)
    MPM.addPass(SPIRVToOCL20Pass());

  MPM.addPass(NameAnonGlobalPass());
  MPM.addPass(DPCPPEqualizerPass());
  Triple TargetTriple(m_M.getTargetTriple());
  if (TargetTriple.isArch64Bit()) {
    if (TargetTriple.isOSLinux())
      MPM.addPass(CoerceTypesPass());
    else if (TargetTriple.isOSWindows())
      MPM.addPass(CoerceWin64TypesPass());
  }
  if (m_IsFpgaEmulator)
    MPM.addPass(RemoveAtExitPass());
}

void OptimizerOCL::createStandardLLVMPasses(ModulePassManager &MPM) const {
  if (Level == OptimizationLevel::O0)
    return;

  // When running the standard optimization passes, do not change the
  // loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for
  // barriers.
  bool UnitAtATime = true;

  if (UnitAtATime) {
    // If a function has internal linkage type this pass can eliminate one or
    // even more arguments in a function call. Due to VPO passes are split
    // in the optimizer that will lead to a mismatch between number of
    // parameters in the function callee and its vectorized form. Therefore,
    // this pass should be launched before VPO.
    MPM.addPass(DeadArgumentEliminationPass());
  }

  if (UnitAtATime) {
    MPM.addPass(GlobalOptPass()); // Optimize out global vars
    MPM.addPass(IPSCCPPass());    // IP SCCP
  }

  FunctionPassManager FPM1;
  FPM1.addPass(InstSimplifyPass());
  FPM1.addPass(InstCombinePass()); // Clean up after IPCP & DAE
  FPM1.addPass(SimplifyCFGPass()); // Clean up after IPCP & DAE
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM1)));

  // Set readonly/readnone attrs
  if (UnitAtATime)
    MPM.addPass(InferFunctionAttrsPass());
  if (Level.getSpeedupLevel() > 2)
    MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(
        ArgumentPromotionPass())); // Scalarize uninlined fn args

  FunctionPassManager FPM2;
  // Break up aggregate allocas
  FPM2.addPass(SROAPass());
  FPM2.addPass(EarlyCSEPass()); // Catch trivial redundancies
  FPM2.addPass(InstSimplifyPass());
  FPM2.addPass(InstCombinePass());   // Cleanup for scalarrepl.
  FPM2.addPass(JumpThreadingPass()); // Thread jumps.
  // Propagate conditionals
  FPM2.addPass(CorrelatedValuePropagationPass());
  FPM2.addPass(SimplifyCFGPass()); // Merge & remove BBs
  FPM2.addPass(InstCombinePass()); // Combine silly seq's

  FPM2.addPass(TailCallElimPass()); // Eliminate tail calls
  FPM2.addPass(SimplifyCFGPass());  // Merge & remove BBs
  FPM2.addPass(ReassociatePass());  // Reassociate expressions

  LoopPassManager LPM1;
  LPM1.addPass(LoopRotatePass()); // Rotate Loop
  LPM1.addPass(LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                        /*AllowSpeculation*/ true)); // Hoist loop invariants
  FPM2.addPass(createFunctionToLoopPassAdaptor(std::move(LPM1),
                                               /*UseMemorySSA=*/true,
                                               /*UseBlockFrequencyInfo=*/true));
  FPM2.addPass(InstCombinePass());
  FPM2.addPass(InstSimplifyPass());
  LoopPassManager LPM2;
  LPM2.addPass(IndVarSimplifyPass()); // Canonicalize indvars
  LPM2.addPass(LoopDeletionPass());   // Delete dead loops
  FPM2.addPass(createFunctionToLoopPassAdaptor(std::move(LPM2),
                                               /*UseMemorySSA=*/true,
                                               /*UseBlockFrequencyInfo=*/true));
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));

  // If a function appeared in a loop is a candidate to be inlined,
  // LoopUnroll pass refuses to unroll the loop, so we should inline the
  // function first to help unroller to decide if it's worthy to unroll the
  // loop.
  // Inline (not only small) functions.
  auto InlineParams = getInlineParams();
  InlineParams.DefaultThreshold = 16384;
  MPM.addPass(ModuleInlinerWrapperPass(InlineParams));

  FunctionPassManager FPM3;
  if (UnrollLoops) {
    // Unroll small loops
    // Parameters for unrolling are as follows:
    // Optimization level, OnlyWhenForced (If false, use cost model to
    // determine loop unrolling profitability. If true, only loops that
    // explicitly request unrolling via metadata are considered),
    // ForgetAllSCEV (If false, when SCEV is invalidated, only forget
    // everything in the top-most loop), cost threshold, explicit unroll
    // count, allow partial unrolling, allow runtime unrolling.
    LoopUnrollOptions UnrollOpts(Level.getSpeedupLevel());
    UnrollOpts.setPartial(false).setRuntime(false).setThreshold(512);
    FPM3.addPass(LoopUnrollPass(UnrollOpts));

    // unroll loops with non-constant trip count
    const int thresholdBase = 16;
    int RTLoopUnrollFactor = Config.GetRTLoopUnrollFactor();
    if (RTLoopUnrollFactor > 1) {
      LoopUnrollOptions UnrollOpts(Level.getSpeedupLevel());
      const unsigned threshold = thresholdBase * RTLoopUnrollFactor;
      // RTLoopUnrollFactor is to customize Count. However, LoopUnrollOptions
      // doesn't allow the customization.
      UnrollOpts.setPartial(false).setRuntime(true).setThreshold(threshold);
      FPM3.addPass(LoopUnrollPass(UnrollOpts));
    }
  }
  // Break up aggregate allocas
  FPM3.addPass(SROAPass());
  // Clean up after the unroller
  FPM3.addPass(InstCombinePass());
  FPM3.addPass(InstSimplifyPass());
  bool AllowAllocaModificationOpt =
      !Config.GetCpuId()->HasGatherScatterPrefetch();
  if (AllowAllocaModificationOpt) {
    if (Level.getSpeedupLevel() > 1)
      FPM3.addPass(GVNPass());     // Remove redundancies
    FPM3.addPass(MemCpyOptPass()); // Remove memcpy / form memset
  }
  FPM3.addPass(SCCPPass()); // Constant prop with SCCP

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  FPM3.addPass(InstCombinePass());
  FPM3.addPass(JumpThreadingPass()); // Thread jumps
  FPM3.addPass(CorrelatedValuePropagationPass());
  // We pass "false" to DSE here (and in all other instances) to disable
  // the MemorySSA algorithm, to improve compile speed. We can re-enable it
  // later if the community improves it.
  FPM3.addPass(DSEPass());         // Delete dead stores
  FPM3.addPass(ADCEPass());        // Delete dead instructions
  FPM3.addPass(SimplifyCFGPass()); // Merge & remove BBs
  FPM3.addPass(InstCombinePass()); // Clean up after everything.
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM3)));

  if (UnitAtATime) {
    // Get rid of dead prototypes
    MPM.addPass(StripDeadPrototypesPass());

    // GlobalOpt already deletes dead functions and globals, at -O3 try a
    // late pass of GlobalDCE.  It is capable of deleting dead cycles.
    if (Level.getSpeedupLevel() > 2)
      MPM.addPass(GlobalDCEPass()); // Remove dead fns and globals.

    if (Level.getSpeedupLevel() > 1)
      MPM.addPass(ConstantMergePass()); // Merge dup global constants
  }
  MPM.addPass(createModuleToFunctionPassAdaptor(UnifyFunctionExitNodesPass()));
}

void OptimizerOCL::populatePassesPreFailCheck(ModulePassManager &MPM) const {
  MPM.addPass(SetPreferVectorWidthPass(ISA));
  if (m_IsSPIRV && Config.GetRelaxedMath())
    MPM.addPass(createModuleToFunctionPassAdaptor(AddFastMathPass()));

  // Here we are internalizing non-kernal functions to allow inliner to remove
  // functions' bodies without call sites
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

  // OCL2.0 add Generic Address Resolution
  // LLVM IR converted from any version of SPIRV may have Generic
  // adress space pointers.
  if ((m_IsOcl20 || m_IsSPIRV) && Level != OptimizationLevel::O0) {
    FunctionPassManager FPM;
    // Static resolution of generic address space pointers
    FPM.addPass(PromotePass());
    FPM.addPass(
        InferAddressSpacesPass(CompilationUtils::ADDRESS_SPACE_GENERIC));
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  MPM.addPass(createModuleToFunctionPassAdaptor(BuiltinCallToInstPass()));

  createStandardLLVMPasses(MPM);

  // check there is no recursion, if there is fail compilation
  MPM.addPass(DetectRecursionPass());

  // PipeSupport can fail if dynamic pipe access is discovered after LLVM
  // optimizations
  if (m_IsFpgaEmulator)
    MPM.addPass(PipeSupportPass());
}

void OptimizerOCL::populatePassesPostFailCheck(ModulePassManager &MPM) const {
  bool isProfiling = Config.GetProfilingFlag();

  // Tune the maximum size of the basic block for memory dependency analysis
  // utilized by GVN.

  MPM.addPass(RequireAnalysisPass<ImplicitArgsAnalysis, Module>());

  if ((m_IsOcl20 || m_IsSPIRV) && Level != OptimizationLevel::O0) {
    FunctionPassManager FPM;
    // Repeat resolution of generic address space pointers after LLVM
    // IR was optimized
    FPM.addPass(
        InferAddressSpacesPass(CompilationUtils::ADDRESS_SPACE_GENERIC));
    // Cleanup after InferAddressSpacesPass
    FPM.addPass(SimplifyCFGPass());
    FPM.addPass(SROAPass());
    FPM.addPass(EarlyCSEPass());
    FPM.addPass(PromotePass());
    FPM.addPass(InstCombinePass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  MPM.addPass(ResolveVarTIDCallPass());

  if (m_IsSYCL)
    MPM.addPass(TaskSeqAsyncHandling());

  // Support matrix fill and slice.
  if (m_IsSYCL) {
    MPM.addPass(ResolveMatrixFillPass());
    MPM.addPass(ResolveMatrixLayoutPass());
    MPM.addPass(ResolveMatrixWISlicePass());
  }

  // Run few more passes after GenericAddressStaticResolution
  if (Level != OptimizationLevel::O0)
    MPM.addPass(InferArgumentAliasPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(UnifyFunctionExitNodesPass()));

  // Should be called before vectorizer!
  MPM.addPass(InstToFuncCallPass(ISA));

  MPM.addPass(DuplicateCalledKernelsPass());

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

  if (Config.GetTransposeSize() != 1 &&
      (Level != OptimizationLevel::O0 || EnableO0Vectorization)) {
    // In profiling mode remove llvm.dbg.value calls before vectorizer.
    if (isProfiling)
      MPM.addPass(ProfilingInfoPass());

    FunctionPassManager FPM1;
    FPM1.addPass(SinCosFoldPass());

    // Replace 'div' and 'rem' instructions with calls to optimized library
    // functions
    FPM1.addPass(MathLibraryFunctionsReplacementPass());

    // Merge returns : this pass ensures that the function has at most one
    // return instruction.
    FPM1.addPass(UnifyFunctionExitNodesPass());
    FPM1.addPass(SimplifyCFGPass(SimplifyCFGOptions()
                                     .bonusInstThreshold(1)
                                     .forwardSwitchCondToPhi(false)
                                     .convertSwitchToLookupTable(false)
                                     .needCanonicalLoops(true)
                                     .sinkCommonInsts(true)));
    FPM1.addPass(InstCombinePass());
    FPM1.addPass(GVNHoistPass());
    FPM1.addPass(DCEPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM1)));

    MPM.addPass(ReqdSubGroupSizePass());
    // This pass may throw VFAnalysisDiagInfo error if VF checking fails.
    MPM.addPass(SetVectorizationFactorPass(ISA));
    MPM.addPass(VectorVariantLowering(ISA));
    MPM.addPass(CreateSimdVariantPropagation());
    MPM.addPass(SGSizeCollectorPass(ISA));
    MPM.addPass(SGSizeCollectorIndirectPass(ISA));

    // Prepare Function for VecClone and call VecClone
    // We won't automatically switch vectorization dimension for SYCL.
    if (!m_IsSYCL)
      MPM.addPass(
          RequireAnalysisPass<VectorizationDimensionAnalysis, Module>());
    MPM.addPass(DPCPPKernelVecClonePass(Optimizer::getVectInfos(), ISA,
                                        !m_IsSYCL && !m_IsOMP));

    MPM.addPass(VectorVariantFillIn());
    MPM.addPass(UpdateCallAttrs());

    // Call VPlan
    FunctionPassManager FPM2;
    FPM2.addPass(PromotePass());
    FPM2.addPass(LowerSwitchPass());
    FPM2.addPass(LoopSimplifyPass());
    FPM2.addPass(LCSSAPass());
    FPM2.addPass(createFunctionToLoopPassAdaptor(
        LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                 /*AllowSpeculation*/ true),
        /*UseMemorySSA=*/true, /*UseBlockFrequencyInfo=*/true));
    FPM2.addPass(VPOCFGRestructuringPass());
    // TODO support FatalErrorHandler
    // [](Function *F) {
    //      F->addFnAttr(llvm::KernelAttribute::VectorVariantFailure,
    //                   "failed to vectorize");
    // }
    FPM2.addPass(vpo::VPlanDriverPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));
    MPM.addPass(DPCPPKernelPostVecPass());

    // Final cleaning up
    FunctionPassManager FPM3;
    FPM3.addPass(VPODirectiveCleanupPass());
    FPM3.addPass(InstCombinePass());
    FPM3.addPass(SimplifyCFGPass());
    FPM3.addPass(PromotePass());
    FPM3.addPass(ADCEPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM3)));

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

    MPM.addPass(HandleVPlanMask(&Optimizer::getVPlanMaskedFuncs()));
  } else {
    // When forced VF equals 1 or in O0 case, check subgroup semantics AND
    // prepare subgroup_emu_size for sub-group emulation.
    MPM.addPass(ReqdSubGroupSizePass());
    MPM.addPass(SetVectorizationFactorPass(ISA));
  }

#ifdef _DEBUG
  MPM.addPass(VerifierPass());
#endif

  MPM.addPass(ResolveSubGroupWICallPass(
      /*ResolveSGBarrier*/ false));

  // Unroll small loops with unknown trip count.
  FunctionPassManager FPM1;
  if (Level != OptimizationLevel::O0) {
    if (UnrollLoops) {
      LoopUnrollOptions UnrollOpts(Level.getSpeedupLevel());
      UnrollOpts.setPartial(false).setRuntime(false).setThreshold(16);
      FPM1.addPass(LoopUnrollPass(UnrollOpts));
    }

    FPM1.addPass(OptimizeIDivAndIRemPass());
  }
  FPM1.addPass(PreventDivCrashesPass());
  // We need InstructionCombining and GVN passes after PreventDivCrashes
  // passes to optimize redundancy introduced by those passes
  if (Level != OptimizationLevel::O0) {
    FPM1.addPass(InstCombinePass());
    FPM1.addPass(GVNPass());
    FPM1.addPass(VectorCombinePass());
    // In specACCEL/124, InstCombine may generate a cross-barrier bool value
    // used as a condition of a 'br' instruction, which leads to performance
    // degradation. JumpThreading eliminates the cross-barrier value.
    FPM1.addPass(JumpThreadingPass());
  }
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM1)));

  // The m_debugType enum and isProfiling flag are mutually exclusive, with
  // precedence given to m_debugType.
  if (isProfiling)
    MPM.addPass(ProfilingInfoPass());

  // Adding WG loops
  if (Level != OptimizationLevel::O0 && Config.GetStreamingAlways())
    MPM.addPass(createModuleToFunctionPassAdaptor(AddNTAttrPass()));

  if (m_debugType == intel::Native)
    MPM.addPass(ImplicitGIDPass(/*HandleBarrier*/ false));
  MPM.addPass(DPCPPKernelWGLoopCreatorPass(m_UseTLSGlobals));

  MPM.addPass(IndirectCallLowering());

  // Clean up scalar kernel after WGLoop for native subgroups.
  if (Level != OptimizationLevel::O0) {
    FunctionPassManager FPM;
    FPM.addPass(DCEPass());         // Delete dead instructions
    FPM.addPass(SimplifyCFGPass()); // Simplify CFG
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  if (m_IsFpgaEmulator)
    MPM.addPass(InfiniteLoopCreatorPass());

  // Barrier pass can't work with a token type, so here we remove region
  // directives
  FunctionPassManager FPM2;
  FPM2.addPass(RemoveRegionDirectivesPass());
  FPM2.addPass(UnifyFunctionExitNodesPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));

  addBarrierPasses(MPM);

  // After adding loops run loop optimizations.
  if (Level != OptimizationLevel::O0) {
    FunctionPassManager FPM2;
    // Add LoopSimplify pass before BuiltinLICM pass as BuiltinLICM pass
    // requires loops in Simplified Form.
    FPM2.addPass(LoopSimplifyPass());
    LoopPassManager LPM;
    LPM.addPass(LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                         /*AllowSpeculation*/ true));
    LPM.addPass(BuiltinLICMPass());
    LPM.addPass(LoopStridedCodeMotionPass());
    FPM2.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true,
                                        /*UseBlockFrequencyInfo=*/true));
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));
  }

  if (Config.GetRelaxedMath())
    MPM.addPass(RelaxedMathPass());

  // The following three passes (AddImplicitArgs/AddTLSGlobals,
  // ResolveWICall, LocalBuffer) must run before BuiltinImportPass!
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

  if (!m_RtlModules.empty()) {
    // Inline BI function
    MPM.addPass(BuiltinImportPass(CPUPrefix));
    if (Level != OptimizationLevel::O0) {
      // After the globals used in built-ins are imported - we can internalize
      // them with further wiping them out with GlobalDCE pass
      MPM.addPass(InternalizeGlobalVariablesPass());
    }
    // Need to convert shuffle calls to shuffle IR before running inline pass
    // on built-ins
    MPM.addPass(createModuleToFunctionPassAdaptor(BuiltinCallToInstPass()));
  }

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
    // Cleaning up internal globals
    MPM.addPass(GlobalDCEPass());
    // AddImplicitArgs pass may create dead implicit arguments.
    MPM.addPass(DeadArgumentEliminationPass());
    MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(
        ArgumentPromotionPass())); // Scalarize uninlined fn args
    FunctionPassManager FPM;
    FPM.addPass(InstCombinePass()); // Cleanup for scalarrepl.
    FPM.addPass(DSEPass());         // Delete dead stores
    FPM.addPass(ADCEPass());        // Delete dead instructions
    FPM.addPass(SimplifyCFGPass()); // Merge & remove BBs
    FPM.addPass(InstCombinePass()); // Cleanup for scalarrepl.
    FPM.addPass(PromotePass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }
  // Only support CPU Device
  if (Level != OptimizationLevel::O0 && !m_IsFpgaEmulator) {
    LoopPassManager LPM;
    LPM.addPass(LICMPass(SetLicmMssaOptCap, SetLicmMssaNoAccForPromotionCap,
                         /*AllowSpeculation*/ true));
    LPM.addPass(LoopIdiomRecognizePass()); // Transform simple loops to non-loop
                                           // form, e.g. memcpy
    LPM.addPass(LoopDeletionPass());       // Delete dead loops
    FunctionPassManager FPM;
    FPM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true,
                                        /*UseBlockFrequencyInfo=*/true));
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  MPM.addPass(PrepareKernelArgsPass(m_UseTLSGlobals));

  if (Level != OptimizationLevel::O0) {
    // These passes come after PrepareKernelArgsPass to eliminate the
    // redundancy reducced by it
    FunctionPassManager FPM;
    FPM.addPass(DCEPass());         // Delete dead instructions
    FPM.addPass(SimplifyCFGPass()); // Simplify CFG
    FPM.addPass(InstCombinePass()); // Instruction combining
    FPM.addPass(DSEPass());         // Eliminated dead stores
    FPM.addPass(EarlyCSEPass());
    // In legacy pipeline, SmartGVNPass(true) is used here.
    FPM.addPass(GVNPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
#ifdef _DEBUG
    MPM.addPass(VerifierPass());
#endif
  }

  // After kernels are inlined into their wrappers we can cleanup the bodies
  MPM.addPass(CleanupWrappedKernelPass());

  // Unroll small loops
  if (UnrollLoops && Level != OptimizationLevel::O0) {
    LoopUnrollOptions UnrollOpts(Level.getSpeedupLevel());
    UnrollOpts.setPartial(false).setRuntime(false).setThreshold(4);
    MPM.addPass(createModuleToFunctionPassAdaptor(LoopUnrollPass(UnrollOpts)));
  }
}

void OptimizerOCL::addBarrierPasses(ModulePassManager &MPM) const {
  if (Level != OptimizationLevel::O0) {
    // Currently, vectorizer is enabled only when Level > O0.
    MPM.addPass(ReplaceScalarWithMaskPass());
    // Reslove sub_group call introduced by ReplaceScalarWithMask pass.
    MPM.addPass(ResolveSubGroupWICallPass(/*ResolveSGBarrier*/ false));
  }

  if (Level != OptimizationLevel::O0) {
    FunctionPassManager FPM;
    FPM.addPass(DCEPass());
    FPM.addPass(SimplifyCFGPass());
    FPM.addPass(PromotePass());
    FPM.addPass(PhiCanonicalization());
    FPM.addPass(RedundantPhiNode());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
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
  // Insert ImplicitGIDPass in the middle of subgroup emulation
  // to track GIDs in emulation loops
  if (m_debugType == intel::Native)
    MPM.addPass(ImplicitGIDPass(/*HandleBarrier*/ true));

  // Resume sub-group emulation
  MPM.addPass(SGValueWidenPass());
  MPM.addPass(SGLoopConstructPass());
#ifdef _DEBUG
  MPM.addPass(VerifierPass());
#endif
  // End sub-group emulation

  // Since previous passes didn't resolve sub-group barriers, we need to
  // resolve them here.
  MPM.addPass(ResolveSubGroupWICallPass(/*ResolveSGBarrier*/ true));

  MPM.addPass(SplitBBonBarrier());
  if (Level != OptimizationLevel::O0) {
    MPM.addPass(ReduceCrossBarrierValuesPass());
#ifdef _DEBUG
    MPM.addPass(VerifierPass());
#endif
  }
  MPM.addPass(KernelBarrier(m_debugType == intel::Native, m_UseTLSGlobals));
#ifdef _DEBUG
  MPM.addPass(VerifierPass());
#endif

  if (Level != OptimizationLevel::O0)
    MPM.addPass(createModuleToFunctionPassAdaptor(PromotePass()));
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
