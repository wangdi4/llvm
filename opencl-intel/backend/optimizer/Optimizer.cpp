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

#include "Optimizer.h"
#include "BarrierMain.h"
#include "InitializeOCLPasses.hpp"
#include "PrintIRPass.h"
#include "VecConfig.h"
#include "VectorizerCommon.h"
#include "cl_config.h"
#include "cl_cpu_detect.h"
#include "exceptions.h"
#include "mic_dev_limits.h"

#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticHandler.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VFAnalysis.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/Intel_OpenCLTransforms/Passes.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/Vectorize/VectorCombine.h"

#include "LLVMSPIRVLib.h"

cl::opt<bool>
    DisableVPlanCM("disable-ocl-vplan-cost-model", cl::init(false), cl::Hidden,
                   cl::desc("Disable cost model for VPlan vectorizer"));

// This flag enables VPlan for OpenCL.
cl::opt<bool> EnableVPlan("enable-vplan-kernel-vectorizer", cl::init(true),
                          cl::Hidden,
                          cl::desc("Enable VPlan Kernel Vectorizer"));

// Enables kernel vectorizer identification message.
cl::opt<bool>
    EmitKernelVectorizerSignOn("emit-kernel-vectorizer-sign-on",
                               cl::init(false), cl::Hidden,
                               cl::desc("Emit which vectorizer is used "
                                        "(Volcano or Vplan)"));

// Enable vectorization at O0 optimization level.
cl::opt<bool> EnableO0Vectorization(
    "enable-o0-vectorization", cl::init(false), cl::Hidden,
    cl::desc("Enable vectorization at O0 optimization level"));

// If set, then optimization passes will process functions as if they have the
// optnone attribute.
extern bool DPCPPForceOptnone;

using CPUDetect = Intel::OpenCL::Utils::CPUDetect;

extern "C"{

llvm::Pass *createVectorizerPass(SmallVectorImpl<Module *> &builtinModules,
                                 const intel::OptimizerConfig *pConfig);
llvm::Pass *createCLStreamSamplerPass();
llvm::Pass *createBuiltinLibInfoPass(ArrayRef<Module *> pRtlModuleList,
                                     std::string type);
llvm::ModulePass *createRemovePrefetchPass();
llvm::ModulePass *createDebugInfoPass();
llvm::Pass *createSmartGVNPass(bool);

}

using namespace intel;
namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// Load Table-Gen'erated VectInfo.gen
static constexpr llvm::VectItem Vect[] = {
  #include "VectInfo.gen"
};
static constexpr llvm::ArrayRef<llvm::VectItem> VectInfos(Vect);

llvm::ArrayRef<llvm::VectItem>
Optimizer::getVectInfos() {
  return VectInfos;
}

const StringSet<> &Optimizer::getVPlanMaskedFuncs() {
  static const StringSet<> VPlanMaskedFuncs =
#define IMPORT_VPLAN_MASKED_VARIANTS
#include "VectInfo.gen"
#undef IMPORT_VPLAN_MASKED_VARIANTS
      ;
  return VPlanMaskedFuncs;
}

// Several PMs are populated in the Optimizer flow:
// Materializer, PreFail and PostFail PMs.
// In order to dump IR for debug purposes one can schedule PrintModulePass
// to a Pass Manager of choice.

/// createStandardModulePasses - Add the standard module passes.  This is
/// expected to be run after the standard function passes.
static inline void createStandardLLVMPasses(llvm::legacy::PassManagerBase *PM,
                                            unsigned OptLevel, bool UnitAtATime,
                                            bool UnrollLoops,
                                            int rtLoopUnrollFactor,
                                            bool allowAllocaModificationOpt,
                                            bool /*UseVplan*/) {
  if (OptLevel == 0) {
    return;
  }

  if (UnitAtATime) {
    // If a function has internal linkage type this pass can eliminate one or
    // even more arguments in a function call. Due to VPO passes are split
    // in the optimizer that will lead to a mismatch between number of parameters in
    // the function callee and its vectorized form. Therefore, this pass should
    // be launched before VPO.
    PM->add(llvm::createDeadArgEliminationPass());
  }

  if (UnitAtATime) {
    PM->add(llvm::createGlobalOptimizerPass());    // Optimize out global vars
    PM->add(llvm::createIPSCCPPass());             // IP SCCP
  }

  PM->add(llvm::createInstSimplifyLegacyPass());
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after IPCP & DAE
  PM->add(llvm::createCFGSimplificationPass());    // Clean up after IPCP & DAE

  // Set readonly/readnone attrs
  if (UnitAtATime)
    PM->add(llvm::createInferFunctionAttrsLegacyPass());

  // Break up aggregate allocas
  PM->add(llvm::createSROAPass());
  PM->add(llvm::createEarlyCSEPass());           // Catch trivial redundancies
  PM->add(llvm::createInstSimplifyLegacyPass());
  PM->add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
  PM->add(llvm::createJumpThreadingPass());        // Thread jumps.
  // Propagate conditionals
  PM->add(llvm::createCorrelatedValuePropagationPass());
  PM->add(llvm::createCFGSimplificationPass());      // Merge & remove BBs
  PM->add(llvm::createInstructionCombiningPass());   // Combine silly seq's

  PM->add(llvm::createTailCallEliminationPass()); // Eliminate tail calls
  PM->add(llvm::createCFGSimplificationPass());   // Merge & remove BBs
  PM->add(llvm::createReassociatePass());         // Reassociate expressions
  PM->add(llvm::createLoopRotatePass());          // Rotate Loop
  PM->add(llvm::createLICMPass());                // Hoist loop invariants
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstSimplifyLegacyPass());
  PM->add(llvm::createIndVarSimplifyPass()); // Canonicalize indvars
  PM->add(llvm::createLoopDeletionPass());   // Delete dead loops

  // If a function appeared in a loop is a candidate to be inlined,
  // LoopUnroll pass refuses to unroll the loop, so we should inline the function
  // first to help unroller to decide if it's worthy to unroll the loop.
  PM->add(llvm::createFunctionInliningPass(16384)); // Inline (not only small)
                                                    // functions
  if (UnrollLoops) {
    // Unroll small loops
    // Parameters for unrolling are as follows:
    // Optimization level, OnlyWhenForced (If false, use cost model to
    // determine loop unrolling profitability. If true, only loops that
    // explicitly request unrolling via metadata are considered),
    // ForgetAllSCEV (If false, when SCEV is invalidated, only forget
    // everything in the top-most loop), cost threshold, explicit unroll
    // count, allow partial unrolling, allow runtime unrolling.
    PM->add(llvm::createLoopUnrollPass(OptLevel, false, false, 512, 0, 0, 0));
    // unroll loops with non-constant trip count
    const int thresholdBase = 16;
    if (rtLoopUnrollFactor > 1) {
      const int threshold = thresholdBase * rtLoopUnrollFactor;
      PM->add(llvm::createLoopUnrollPass(OptLevel, false, false,
                                         threshold, rtLoopUnrollFactor, 0, 1));
    }
  }
  // Break up aggregate allocas
  PM->add(llvm::createSROAPass());
  // Clean up after the unroller
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstSimplifyLegacyPass());
  if (allowAllocaModificationOpt) {
    if (OptLevel > 1)
      PM->add(llvm::createGVNPass());     // Remove redundancies
    PM->add(llvm::createMemCpyOptPass()); // Remove memcpy / form memset
  }
  PM->add(llvm::createSCCPPass()); // Constant prop with SCCP

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createJumpThreadingPass()); // Thread jumps
  PM->add(llvm::createCorrelatedValuePropagationPass());
  // We pass "false" to DSE here (and in all other instances) to disable
  // the MemorySSA algorithm, to improve compile speed. We can re-enable it
  // later if the community improves it.
  PM->add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
  PM->add(llvm::createAggressiveDCEPass());        // Delete dead instructions
  PM->add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after everything.

  if (UnitAtATime) {
    // Get rid of dead prototypes
    PM->add(llvm::createStripDeadPrototypesPass());

    // GlobalOpt already deletes dead functions and globals, at -O3 try a
    // late pass of GlobalDCE.  It is capable of deleting dead cycles.
    if (OptLevel > 2)
      PM->add(llvm::createGlobalDCEPass()); // Remove dead fns and globals.

    if (OptLevel > 1)
      PM->add(llvm::createConstantMergePass()); // Merge dup global constants
  }
  PM->add(llvm::createUnifyFunctionExitNodesPass());
}

static void populatePassesPreFailCheck(llvm::legacy::PassManagerBase &PM,
                                       llvm::Module & /*M*/, unsigned OptLevel,
                                       const intel::OptimizerConfig &pConfig,
                                       bool isOcl20, bool isFpgaEmulator,
                                       bool UnrollLoops, bool isSPIRV,
                                       bool UseVplan) {
  bool HasGatherScatterPrefetch =
      pConfig.GetCpuId()->HasGatherScatterPrefetch();

  PM.add(llvm::createSetPreferVectorWidthLegacyPass(
      VectorizerCommon::getCPUIdISA(pConfig.GetCpuId())));
  if (isSPIRV && pConfig.GetRelaxedMath()) {
    PM.add(llvm::createAddFastMathLegacyPass());
  }

  // Here we are internalizing non-kernal functions to allow inliner to remove
  // functions' bodies without call sites
  if (OptLevel > 0)
    PM.add(llvm::createInternalizeNonKernelFuncLegacyPass());

  PM.add(llvm::createFMASplitterPass());
  PM.add(llvm::createAddFunctionAttrsLegacyPass());

  if (OptLevel > 0) {
    PM.add(llvm::createCFGSimplificationPass());
    if (OptLevel == 1)
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    else {
      PM.add(llvm::createSROAPass());
    }
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(llvm::createInstSimplifyLegacyPass());
  }

  if (isOcl20) {
    // Flatten get_{local, global}_linear_id()
    PM.add(llvm::createLinearIdResolverPass());
  }

  if (isFpgaEmulator) {
      PM.add(createDPCPPRewritePipesLegacyPass());
      PM.add(createChannelPipeTransformationLegacyPass());
      PM.add(createPipeIOTransformationLegacyPass());
      PM.add(createPipeOrderingLegacyPass());
      PM.add(createAutorunReplicatorLegacyPass());
  }

  // Adding module passes.

  // OCL2.0 add Generic Address Resolution
  // LLVM IR converted from any version of SPIRV may have Generic
  // adress space pointers.
  if ((isOcl20 || isSPIRV) && OptLevel > 0) {
    // Static resolution of generic address space pointers
    PM.add(llvm::createPromoteMemoryToRegisterPass());
    PM.add(llvm::createInferAddressSpacesPass(
        llvm::CompilationUtils::ADDRESS_SPACE_GENERIC));
  }

  PM.add(llvm::createBasicAAWrapperPass());
  if (pConfig.EnableOCLAA()) {
    PM.add(createDPCPPAliasAnalysisLegacyPass());
    PM.add(createDPCPPExternalAliasAnalysisLegacyPass());
  }

  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "DISMPF") ||
      DPCPPStatistic::isEnabled())
    PM.add(createRemovePrefetchPass());

  PM.add(llvm::createBuiltinCallToInstLegacyPass());

  // When running the standard optimization passes, do not change the
  // loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for
  // barriers.
  bool UnitAtATime = true;

  int rtLoopUnrollFactor = pConfig.GetRTLoopUnrollFactor();

  bool allowAllocaModificationOpt = !HasGatherScatterPrefetch;

  createStandardLLVMPasses(&PM, OptLevel, UnitAtATime, UnrollLoops,
                           rtLoopUnrollFactor, allowAllocaModificationOpt,
                           UseVplan);

  // check there is no recursion, if there is fail compilation
  PM.add(llvm::createDetectRecursionLegacyPass());

  // PipeSupport can fail if dynamic pipe access is discovered after LLVM
  // optimizations
  if (isFpgaEmulator) {
    PM.add(llvm::createPipeSupportLegacyPass());
  }
}

static void populatePassesPostFailCheck(
    llvm::legacy::PassManagerBase &PM, llvm::Module &M,
    SmallVectorImpl<Module *> &pRtlModuleList, unsigned OptLevel,
    const intel::OptimizerConfig &pConfig, bool isOcl20, bool isFpgaEmulator,
    bool UnrollLoops, bool UseVplan, bool IsSPIRV,
    bool IsSYCL, bool IsOMP, DebuggingServiceType debugType,
    bool UseTLSGlobals) {
  bool isProfiling = pConfig.GetProfilingFlag();
  bool HasGatherScatter = pConfig.GetCpuId()->HasGatherScatter();
  // Tune the maximum size of the basic block for memory dependency analysis
  // utilized by GVN.
  VectorVariant::ISAClass ISA =
      VectorizerCommon::getCPUIdISA(pConfig.GetCpuId());

  if (pConfig.EnableOCLAA()) {
    PM.add(createDPCPPAliasAnalysisLegacyPass());
    PM.add(createDPCPPExternalAliasAnalysisLegacyPass());
  }

  PM.add(createImplicitArgsAnalysisLegacyPass());

  if ((isOcl20 || IsSPIRV) && OptLevel > 0) {
    // Repeat resolution of generic address space pointers after LLVM
    // IR was optimized
    PM.add(llvm::createInferAddressSpacesPass(
        llvm::CompilationUtils::ADDRESS_SPACE_GENERIC));
    // Cleanup after InferAddressSpacesPass
    PM.add(llvm::createCFGSimplificationPass());
    PM.add(llvm::createSROAPass());
    PM.add(llvm::createEarlyCSEPass());
    PM.add(llvm::createPromoteMemoryToRegisterPass());
    PM.add(llvm::createInstructionCombiningPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  PM.add(llvm::createResolveVarTIDCallLegacyPass());

  if (IsSYCL)
    PM.add(createTaskSeqAsyncHandlingLegacyPass());

  // Support matrix fill and slice.
  if (IsSYCL) {
    PM.add(createResolveMatrixFillLegacyPass());
    PM.add(createResolveMatrixLayoutLegacyPass());
    PM.add(createResolveMatrixWISliceLegacyPass());
  }

  // Run few more passes after GenericAddressStaticResolution
  if (OptLevel > 0)
    PM.add(llvm::createInferArgumentAliasLegacyPass());
  PM.add(llvm::createUnifyFunctionExitNodesPass());


  PM.add(llvm::createBasicAAWrapperPass());

  // Should be called before vectorizer!
  PM.add(llvm::createInstToFuncCallLegacyPass(ISA));

  PM.add(createDuplicateCalledKernelsLegacyPass());

  // Dirty hack:
  // On Windows, Intel(R) Advisor doesn't work with LLDJIT (based on MCJIT), so
  // native debugger cannot be used. However, in simulator mode, we insert a
  // lot debug callbacks, which cause significant performance degradation. So I
  // add an environment veriables to disable DebugInfo pass.  When using
  // Advisor, CL_CONFIG_CPU_NO_DBG_CBK should be explicitly set to non-false
  // value and CL_CONFIG_USE_NATIVE_DEBUGGER should be explicitly set to false,
  // so that no redundant call to ocl_dbg_* will be inserted and LLJIT engine
  // will be used.
  // On Linux, there's no such issue so 'NoDbgCbk' is always false.
  // This part of ugly code should be definitely removed if we switch the JIT
  // engine of LLDJIT from MCJIT to LLJIT, or when Advisor can happily work
  // with MCJIT.
#ifdef _WIN32
  std::string NoDbgCbkEnv;
  Intel::OpenCL::Utils::getEnvVar(NoDbgCbkEnv, "CL_CONFIG_CPU_NO_DBG_CBK");
  bool NoDbgCbk =
      Intel::OpenCL::Utils::ConfigFile::ConvertStringToType<bool>(NoDbgCbkEnv);
#else
  constexpr bool NoDbgCbk = false;
#endif

  if (debugType == Simulator && !NoDbgCbk) {
    // DebugInfo pass must run before KernelAnalysis and Barrier pass when
    // debugging with simulator. DebugInfo inserts get_global_id call to
    // un-inlined callee, and KernelAnalysis will set no_barrier_path to false
    // for caller kernel. The kernel will be handled in Barrier instead of
    // LoopCreator pass.
    PM.add(createDebugInfoPass());
  }

  PM.add(llvm::createDPCPPKernelAnalysisLegacyPass());
  if (OptLevel > 0) {
    PM.add(llvm::createCFGSimplificationPass());
    PM.add(llvm::createWGLoopBoundariesLegacyPass());
    PM.add(llvm::createDeadCodeEliminationPass());
    PM.add(llvm::createCFGSimplificationPass());
    PM.add(llvm::createDeduceMaxWGDimLegacyPass());
  }

  // In Apple build TRANSPOSE_SIZE_1 is not declared
  if (pConfig.GetTransposeSize() != 1 /*TRANSPOSE_SIZE_1*/
      && (OptLevel != 0 || EnableO0Vectorization)) {

    // In profiling mode remove llvm.dbg.value calls before vectorizer.
    if (isProfiling) {
      PM.add(createProfilingInfoLegacyPass());
    }

    PM.add(createSinCosFoldLegacyPass());

    if (!pRtlModuleList.empty()) {
      if (UseVplan) {
        if (EmitKernelVectorizerSignOn)
          dbgs() << "Kernel Vectorizer\n";

        // Replace 'div' and 'rem' instructions with calls to optimized library
        // functions
        PM.add(createMathLibraryFunctionsReplacementPass());

        // Merge returns : this pass ensures that the function has at most one
        // return instruction.
        PM.add(createUnifyFunctionExitNodesPass());
        PM.add(createCFGSimplificationPass(
            SimplifyCFGOptions()
            .bonusInstThreshold(1)
            .forwardSwitchCondToPhi(false)
            .convertSwitchToLookupTable(false)
            .needCanonicalLoops(true)
            .sinkCommonInsts(true)));
        PM.add(createInstructionCombiningPass());
        PM.add(createGVNHoistPass());
        PM.add(createDeadCodeEliminationPass());
        PM.add(createReqdSubGroupSizeLegacyPass());

        // This pass may throw VFAnalysisDiagInfo error if VF checking fails.
        PM.add(llvm::createSetVectorizationFactorLegacyPass(ISA));
        PM.add(createVectorVariantLoweringLegacyPass(ISA));
        PM.add(createCreateSimdVariantPropagationLegacyPass());
        PM.add(createSGSizeCollectorLegacyPass(ISA));
        PM.add(createSGSizeCollectorIndirectLegacyPass(ISA));

        // Prepare Function for VecClone and call VecClone
        // We won't automatically switch vectorization dimension for SYCL.
        if (!IsSYCL)
          PM.add(llvm::createVectorizationDimensionAnalysisLegacyPass());
        PM.add(llvm::createDPCPPKernelVecClonePass(Optimizer::getVectInfos(),
                                                   ISA, !IsSYCL && !IsOMP));

        PM.add(llvm::createVectorVariantFillInLegacyPass());
        PM.add(llvm::createUpdateCallAttrsLegacyPass());

        // Call VPlan
        PM.add(llvm::createPromoteMemoryToRegisterPass());
        PM.add(createLowerSwitchPass());
        PM.add(createLoopSimplifyPass());
        PM.add(createLCSSAPass());
        PM.add(createLICMPass());
        PM.add(createVPOCFGRestructuringPass());
        PM.add(createVPlanDriverPass([](Function *F) {
          F->addFnAttr(llvm::KernelAttribute::VectorVariantFailure,
                       "failed to vectorize");
        }));
        PM.add(llvm::createDPCPPKernelPostVecPass());

        // Final cleaning up
        PM.add(createVPODirectiveCleanupPass());
        PM.add(createInstructionCombiningPass());
        PM.add(createCFGSimplificationPass());
        PM.add(createPromoteMemoryToRegisterPass());
        PM.add(createAggressiveDCEPass());

        // Add cost model to discard vectorized kernels if they have higher
        // cost. This is done only for native OpenCL program. In SYCL, unless
        // programmer explicitly asks not to vectorize (SG size of 1, OCL env
        // to disable vectorization, etc), compiler shall vectorize along the
        // fastest moving dimension (that maps to get_global_id(0) for LLVM IR
        // in our implementation). The vec/no-vec decision belongs to the
        // programmer.
        if (!IsSYCL && !DisableVPlanCM &&
            pConfig.GetTransposeSize() == TRANSPOSE_SIZE_NOT_SET)
          PM.add(llvm::createVectorKernelEliminationLegacyPass());
      } else {
        if (EmitKernelVectorizerSignOn)
          dbgs() << "OpenCL Kernel Vectorizer\n";
        PM.add(createVectorizerPass(pRtlModuleList, &pConfig));
      }
    }

    if (UseVplan)
      PM.add(
          createHandleVPlanMaskLegacyPass(&Optimizer::getVPlanMaskedFuncs()));
  } else {
    // When forced VF equals 1 or in O0 case, check subgroup semantics AND
    // prepare subgroup_emu_size for sub-group emulation.
    if (UseVplan) {
      PM.add(createReqdSubGroupSizeLegacyPass());
      PM.add(llvm::createSetVectorizationFactorLegacyPass(ISA));
    }
  }
#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  PM.add(createResolveSubGroupWICallLegacyPass(
      /*ResolveSGBarrier*/ false));

  // Unroll small loops with unknown trip count.
  if (OptLevel > 0) {
    PM.add(llvm::createLoopUnrollPass(OptLevel, false, false, 16, 0, 0, 1));
    PM.add(createOptimizeIDivAndIRemLegacyPass());
  }
  PM.add(createPreventDivCrashesLegacyPass());
  // We need InstructionCombining and GVN passes after PreventDivCrashes
  // passes to optimize redundancy introduced by those passes
  if (OptLevel > 0) {
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(createSmartGVNPass(false));
    PM.add(createVectorCombinePass());
    // In specACCEL/124, InstCombine may generate a cross-barrier bool value
    // used as a condition of a 'br' instruction, which leads to performance
    // degradation. JumpThreading eliminates the cross-barrier value.
    PM.add(llvm::createJumpThreadingPass());
  }

  // The debugType enum and isProfiling flag are mutually exclusive, with
  // precedence given to debugType.
  if (isProfiling) {
    PM.add(createProfilingInfoLegacyPass());
  }

  // Adding WG loops
  if (OptLevel > 0 && pConfig.GetStreamingAlways())
    PM.add(createAddNTAttrLegacyPass());

  if (debugType == Native)
    PM.add(createImplicitGIDLegacyPass(/*HandleBarrier*/ false));
  PM.add(llvm::createDPCPPKernelWGLoopCreatorLegacyPass(UseTLSGlobals));

  PM.add(createIndirectCallLoweringLegacyPass());

  // Clean up scalar kernel after WGLoop for native subgroups.
  if (OptLevel > 0) {
    PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());   // Simplify CFG
  }

  if (isFpgaEmulator) {
    PM.add(llvm::createInfiniteLoopCreatorLegacyPass());
  }

  // Barrier pass can't work with a token type, so here we remove region
  // directives
  PM.add(llvm::createRemoveRegionDirectivesLegacyPass());

  PM.add(createUnifyFunctionExitNodesPass());
  addBarrierMainPasses(PM, OptLevel, debugType, UseTLSGlobals,
                       Optimizer::getVectInfos());

  // After adding loops run loop optimizations.
  if (OptLevel > 0) {
    // Add LoopSimplify pass before CLBuiltinLICM pass as CLBuiltinLICM pass
    // requires loops in Simplified Form.
    PM.add(createLoopSimplifyPass());
    PM.add(llvm::createLICMPass());
    PM.add(llvm::createBuiltinLICMLegacyPass());
    PM.add(llvm::createLoopStridedCodeMotionLegacyPass());
    PM.add(createCLStreamSamplerPass());
  }

  if (pConfig.GetRelaxedMath()) {
    PM.add(llvm::createRelaxedMathLegacyPass());
  }

  // The following three passes (AddImplicitArgsLegacy/AddTLSGlobals,
  // ResolveWICall, LocalBuffer) must run before createBuiltinImportLegacyPass!
  if (UseTLSGlobals)
    PM.add(llvm::createAddTLSGlobalsLegacyPass());
  else
    PM.add(llvm::createAddImplicitArgsLegacyPass());

  PM.add(llvm::createResolveWICallLegacyPass(pConfig.GetUniformWGSize(), UseTLSGlobals));
  PM.add(llvm::createLocalBuffersLegacyPass(UseTLSGlobals));
  // clang converts OCL's local to global.
  // createLocalBuffersLegacyPass changes the local allocation from global to a
  // kernel argument.
  // The next pass createGlobalOptimizerPass cleans the unused global
  // allocation in order to make sure we will not allocate redundant space on
  // the jit
  if (OptLevel > 0 && debugType != Native)
    PM.add(llvm::createGlobalOptimizerPass());

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // Externalize globals if IR is generated from OpenMP offloading. Now we
  // cannot get address of globals with internal/private linkage from LLJIT
  // (by design), but it's necessary by OpenMP to pass address of declare
  // target variables to the underlying OpenCL Runtime via
  // clSetKernelExecInfo. So we have to externalize globals for IR generated
  // from OpenMP.
  if (IsOMP)
    PM.add(llvm::createExternalizeGlobalVariablesLegacyPass());

  if (!pRtlModuleList.empty()) {
    // Inline BI function
    const char *CPUPrefix = pConfig.GetCpuId()->GetCPUPrefix();
    assert(CPUPrefix && "CPU Prefix should not be null");
    PM.add(llvm::createBuiltinImportLegacyPass(CPUPrefix));
    if (OptLevel > 0) {
      // After the globals used in built-ins are imported - we can internalize
      // them with further wiping them out with GlobalDCE pass
      PM.add(llvm::createInternalizeGlobalVariablesLegacyPass());
    }
    // Need to convert shuffle calls to shuffle IR before running inline pass
    // on built-ins
    PM.add(llvm::createBuiltinCallToInstLegacyPass());
  }

// funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  if (OptLevel > 0) {
    if (HasGatherScatter)
      // Original motivation for this customization was a limitation of the
      // vectorizer to vectorize only kernels without function calls.
      // Missing vectorization caused huge performance loss on KNC,
      // so inlining was done more agressively.
      // This was transferred to AVX-512 SKX/CNL through HasGatherScatter flag.
      // So far there's no data that would suggest dropping this customization.
      PM.add(llvm::createFunctionInliningPass(4096)); // Inline (not only small) functions.
    else
      PM.add(llvm::createFunctionInliningPass());     // Inline small functions
  } else if (isOcl20) {
    // Ensure that the built-in functions to be processed by PatchCallbackArgsPass
    // are inlined.
    PM.add(llvm::createAlwaysInlinerLegacyPass());
  }
  // Some built-in functions contain calls to external functions which take
  // arguments that are retrieved from the function's implicit arguments.
  // Currently only applies to OpenCL 2.x
  if (isOcl20)
    PM.add(llvm::createPatchCallbackArgsLegacyPass(UseTLSGlobals));

  if (OptLevel > 0) {
    // Cleaning up internal globals
    PM.add(llvm::createGlobalDCEPass());
    // AddImplicitArgs pass may create dead implicit arguments.
    PM.add(llvm::createDeadArgEliminationPass());
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createPromoteMemoryToRegisterPass());
  }
  // Only support CPU Device
  if (OptLevel > 0 && !isFpgaEmulator) {
    PM.add(llvm::createLICMPass());      // Hoist loop invariants
    PM.add(llvm::createLoopIdiomPass()); // Transform simple loops to non-loop form, e.g. memcpy
    PM.add(createLoopDeletionPass()); // Delete dead loops
  }

  // PrepareKernelArgsLegacyPass must run in debugging mode as well
  PM.add(llvm::createPrepareKernelArgsLegacyPass(UseTLSGlobals));

  if ( OptLevel > 0 ) {
    // These passes come after PrepareKernelArgsLegacyPass to eliminate the
    // redundancy reducced by it
    PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());   // Simplify CFG
    PM.add(llvm::createInstructionCombiningPass()); // Instruction combining
    PM.add(llvm::createDeadStoreEliminationPass()); // Eliminated dead stores
    PM.add(llvm::createEarlyCSEPass());
    PM.add(createSmartGVNPass(true)); // GVN with "no load" heuristic
#ifdef _DEBUG
    PM.add(llvm::createVerifierPass());
#endif
  } else {
    // Functions with the alwaysinline attribute need to be inlined for
    // functional purposes
    PM.add(llvm::createAlwaysInlinerLegacyPass());
  }

  // After kernels are inlined into their wrappers we can cleanup the bodies
  PM.add(llvm::createCleanupWrappedKernelLegacyPass());

  if (UnrollLoops && OptLevel > 0) {
    // Unroll small loops
    PM.add(llvm::createLoopUnrollPass(OptLevel, false, false, 4, 0, 0));
  }
}

OptimizerOCLLegacy::OptimizerOCLLegacy(
    llvm::Module &pModule, llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
    const intel::OptimizerConfig &pConfig)
    : Optimizer(pModule, RtlModules, pConfig) {
  TargetMachine* targetMachine = pConfig.GetTargetMachine();
  assert(targetMachine && "Uninitialized TargetMachine!");

  unsigned int OptLevel = 3;
  if (pConfig.GetDisableOpt())
    OptLevel = 0;
  DPCPPForceOptnone = OptLevel == 0;

  bool UnrollLoops = true;

  // Initialize TTI
  m_PM.add(createTargetTransformInfoWrapperPass(
      targetMachine->getTargetIRAnalysis()));

  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfoImpl TLII(Triple(pModule.getTargetTriple()));
  m_PM.add(new TargetLibraryInfoWrapperPass(TLII));

  auto materializerPM = [this]() {
    if (m_IsSYCL)
      m_PM.add(createSPIRVToOCL20Legacy());
    m_PM.add(llvm::createBuiltinLibInfoAnalysisLegacyPass(m_RtlModules));
    m_PM.add(createBuiltinLibInfoPass(m_RtlModules, ""));
    m_PM.add(createDPCPPEqualizerLegacyPass());
    Triple TargetTriple(m_M.getTargetTriple());
    if (TargetTriple.isArch64Bit()) {
      if (TargetTriple.isOSLinux())
        m_PM.add(createCoerceTypesLegacyPass());
      else if (TargetTriple.isOSWindows())
        m_PM.add(createCoerceWin64TypesLegacyPass());
    }
    if (m_IsFpgaEmulator)
      m_PM.add(llvm::createRemoveAtExitLegacyPass());
  };
  materializerPM();

  populatePassesPreFailCheck(m_PM, pModule, OptLevel, pConfig, m_IsOcl20,
                             m_IsFpgaEmulator, UnrollLoops, m_IsSPIRV,
                             EnableVPlan);

  populatePassesPostFailCheck(m_PM, pModule, m_RtlModules, OptLevel, pConfig,
                              m_IsOcl20, m_IsFpgaEmulator,
                              UnrollLoops, EnableVPlan, m_IsSPIRV, m_IsSYCL,
                              m_IsOMP, m_debugType, m_UseTLSGlobals);
}

/// Customized diagnostic handler to be registered to LLVMContext before running
/// passes. Prints error messages and throw exception if received an error
/// diagnostic.
/// - Handles VFAnalysisDiagInfo emitted by VFAnalysis.
class OCLDiagnosticHandler : public llvm::DiagnosticHandler {
public:
  OCLDiagnosticHandler(llvm::raw_ostream &OS) : OS(OS) {}
  bool handleDiagnostics(const llvm::DiagnosticInfo &DI) override {
    // Handle VFAnalysisDiagInfo emitted by VFAnalysis.
    if (auto *VFADI = dyn_cast<llvm::VFAnalysisDiagInfo>(&DI)) {
      OS << llvm::LLVMContext::getDiagnosticMessagePrefix(VFADI->getSeverity())
         << ": ";
      VFADI->print(OS);
      OS << ".\n";
      if (VFADI->getSeverity() == DS_Error)
        throw Exceptions::CompilerException(
            "Checking vectorization factor failed", CL_DEV_INVALID_BINARY);
      return true;
    }
    return false;
  }

private:
  llvm::DiagnosticPrinterRawOStream OS;
};

void OptimizerOCLLegacy::Optimize(llvm::raw_ostream &LogStream) {
  // Set custom DiagnosticHandler callback.
  setDiagnosticHandler(LogStream);

  m_PM.run(m_M);

  // if there are still unsupported recursive calls after standard LLVM
  // optimizations applied, compilation will report failure.
  if (hasUnsupportedRecursion()) {
    return;
  }

  // if not all pipe access were resolved statically.
  if (hasFpgaPipeDynamicAccess()) {
    return;
  }

  // if not all must vec functions have been vectorized.
  // Serves as a safe guard to not execute the code below that
  // might be added in the future.
  if (hasVectorVariantFailure()) {
    return;
  }
}

Optimizer::Optimizer(llvm::Module &M,
                     llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
                     const intel::OptimizerConfig &OptConfig)
    : m_M(M), m_RtlModules(RtlModules.begin(), RtlModules.end()),
      Config(OptConfig),
      m_IsSPIRV(llvm::CompilationUtils::generatedFromSPIRV(M)),
      m_IsSYCL(llvm::CompilationUtils::isGeneratedFromOCLCPP(M)),
      m_IsOMP(llvm::CompilationUtils::isGeneratedFromOMP(M)),
      m_IsFpgaEmulator(Config.isFpgaEmulator()) {
  assert(Config.GetCpuId() && "Invalid optimizer config");
  ISA = VectorizerCommon::getCPUIdISA(Config.GetCpuId());
  CPUPrefix = Config.GetCpuId()->GetCPUPrefix();
  DPCPPForceOptnone = Config.GetDisableOpt();
  m_IsOcl20 = llvm::CompilationUtils::fetchCLVersionFromMetadata(M) >=
              llvm::CompilationUtils::OclVersion::CL_VER_2_0;
  m_debugType = getDebuggingServiceType(Config.GetDebugInfoFlag(), &M,
                                        Config.GetUseNativeDebuggerFlag());
  m_UseTLSGlobals = (m_debugType == intel::Native);
}

void Optimizer::setDiagnosticHandler(llvm::raw_ostream &LogStream) {
  m_M.getContext().setDiagnosticHandler(
      std::make_unique<OCLDiagnosticHandler>(LogStream));
}

bool Optimizer::hasUnsupportedRecursion() {
  return m_IsSYCL
             ? !GetInvalidFunctions(InvalidFunctionType::RECURSION_WITH_BARRIER)
                    .empty()
             : !GetInvalidFunctions(InvalidFunctionType::RECURSION).empty();
}

bool Optimizer::hasFpgaPipeDynamicAccess() const {
  return !GetInvalidFunctions(
      InvalidFunctionType::FPGA_PIPE_DYNAMIC_ACCESS).empty();
}

bool Optimizer::hasVectorVariantFailure() const {
  return !GetInvalidFunctions(InvalidFunctionType::VECTOR_VARIANT_FAILURE)
              .empty();
}

bool Optimizer::hasFPGAChannelsWithDepthIgnored() const {
  return !GetInvalidGlobals(InvalidGVType::FPGA_DEPTH_IS_IGNORED).empty();
}

std::vector<std::string> Optimizer::GetInvalidGlobals(InvalidGVType Ty) const {
  std::vector<std::string> Res;

  for (auto &GV : m_M.globals()) {
    auto GVM = DPCPPKernelMetadataAPI::GlobalVariableMetadataAPI(&GV);

    switch (Ty) {
      case FPGA_DEPTH_IS_IGNORED:
        if (GVM.DepthIsIgnored.hasValue() && GVM.DepthIsIgnored.get()) {
          assert(GV.getName().endswith(".pipe") &&
              "Only global pipes are expected");
          Res.push_back(std::string(GV.getName().drop_back(5)));
        }
    }
  }

  return Res;
}

std::vector<std::string>
Optimizer::GetInvalidFunctions(InvalidFunctionType Ty) const {
  std::vector<std::string> Res;

  for (auto &F : m_M) {
    auto KMD = DPCPPKernelMetadataAPI::FunctionMetadataAPI(&F);

    bool Invalid = false;

    switch (Ty) {
    case RECURSION:
      Invalid = KMD.RecursiveCall.hasValue() && KMD.RecursiveCall.get();
      break;
    case RECURSION_WITH_BARRIER:
      Invalid = F.hasFnAttribute(llvm::KernelAttribute::RecursionWithBarrier);
      break;
    case FPGA_PIPE_DYNAMIC_ACCESS:
      Invalid = KMD.FpgaPipeDynamicAccess.hasValue() &&
        KMD.FpgaPipeDynamicAccess.get();
      break;
    case VECTOR_VARIANT_FAILURE:
      Invalid = F.hasFnAttribute(llvm::KernelAttribute::VectorVariantFailure);
      break;
    }

    if (Invalid) {
      std::string Message;
      llvm::raw_string_ostream MStr(Message);
      MStr << std::string(F.getName());
      if (auto SP = F.getSubprogram()) {
        MStr << " at ";
        MStr << "file: " << SP->getFilename () << ", line:" << SP->getLine();
      }
      MStr.str();
      Res.push_back(std::move(Message));
    }
  }

  return Res;
}

void OptimizerOCLLegacy::initializePasses() {
  // Initialize passes so that -print-after/-print-before work.
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeCore(Registry);
  initializeScalarOpts(Registry);
  initializeObjCARCOpts(Registry);
  initializeVectorization(Registry);
  initializeIPO(Registry);
  initializeAnalysis(Registry);
  initializeTransformUtils(Registry);
  initializeInstCombine(Registry);
  initializeInstrumentation(Registry);
  initializeTarget(Registry);
  // For codegen passes, only passes that do IR to IR transformation are
  // supported.
  initializeExpandMemCmpPassPass(Registry);
  initializeScalarizeMaskedMemIntrinLegacyPassPass(Registry);
  initializeCodeGenPreparePass(Registry);
  initializeAtomicExpandPass(Registry);
  initializeRewriteSymbolsLegacyPassPass(Registry);
  initializeWinEHPreparePass(Registry);
  initializeDwarfEHPrepareLegacyPassPass(Registry);
  initializeSafeStackLegacyPassPass(Registry);
  initializeSjLjEHPreparePass(Registry);
  initializePreISelIntrinsicLoweringLegacyPassPass(Registry);
  initializeGlobalMergePass(Registry);
  initializeInterleavedAccessPass(Registry);
  initializeUnreachableBlockElimLegacyPassPass(Registry);
  initializeExpandReductionsPass(Registry);
  initializeWriteBitcodePassPass(Registry);

  initializeIntel_LoopAnalysis(Registry);
  initializeIntel_LoopTransforms(Registry);
  initializeVecClonePass(Registry);
  initializeMapIntrinToImlPass(Registry);
  initializeIntel_OpenCLTransforms(Registry);
  initializeVPOAnalysis(Registry);
  initializeVPOTransforms(Registry);
  initializeOptimizeDynamicCastsWrapperPass(Registry);

  initializeOCLPasses(Registry);
}
}}}
