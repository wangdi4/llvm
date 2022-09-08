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

#include "OptimizerLTOLegacyPM.h"
#include "LLVMSPIRVLib.h"
#include "VecConfig.h"
#include "VectorizerCommon.h"
#include "llvm/Analysis/LoopPass.h"
#ifndef NDEBUG
#include "llvm/IR/Verifier.h"
#endif // #ifndef NDEBUG
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_OpenCLTransforms/Passes.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Vectorize.h"

// If set, then optimization passes will process functions as if they have the
// optnone attribute.
extern bool DPCPPForceOptnone;

extern cl::opt<bool> DisableVPlanCM;
extern cl::opt<bool> EnableO0Vectorization;

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerLTOLegacyPM::OptimizerLTOLegacyPM(
    Module &M, SmallVector<Module *, 2> &RtlModuleList,
    const intel::OptimizerConfig &Config)
    : Optimizer(M, RtlModuleList, Config), FPM(&M) {
  CreatePasses();
}

OptimizerLTOLegacyPM::~OptimizerLTOLegacyPM() {}

/// Ported from clang EmitAssemblyHelper::CreatePasses.
void OptimizerLTOLegacyPM::CreatePasses() {
  TargetMachine *TM = Config.GetTargetMachine();
  assert(TM && "Uninitialized TargetMachine!");

  Triple TargetTriple(m_M.getTargetTriple());
  TLII.reset(new TargetLibraryInfoImpl(TargetTriple));
  PassManagerBuilder PMBuilder;

  PMBuilder.OptLevel = Config.GetDisableOpt() ? 0 : 3;
  PMBuilder.SizeLevel = 0;
  PMBuilder.SLPVectorize = false;
  PMBuilder.LoopVectorize = false;
  PMBuilder.CallGraphProfile = true;
  PMBuilder.DisableUnrollLoops = false;
  PMBuilder.LoopsInterleaved = false;
  PMBuilder.MergeFunctions = false;
  PMBuilder.RerollLoops = false;

  DPCPPForceOptnone = PMBuilder.OptLevel == 0;

  // At O0 and O1 we only run the always inliner which is more efficient. At
  // higher optimization levels we run the normal inliner.
  if (PMBuilder.OptLevel <= 1) {
    PMBuilder.Inliner =
        createAlwaysInlinerLegacyPass(/*InsertLifetimeIntrinsics*/ false);
  } else {
    // We do not want to inline hot callsites for SamplePGO module-summary build
    // because profile annotation will happen again in ThinLTO backend, and we
    // want the IR of the hot path to match the profile.
    auto Params = getInlineParams(PMBuilder.OptLevel, PMBuilder.SizeLevel,
                                  false, false, /*SYCLOptimizationMode=*/false);
    Params.DefaultThreshold = 16384;
    PMBuilder.Inliner = createFunctionInliningPass(Params);
  }

  // Initialize TTI
  MPM.add(createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis()));
  FPM.add(createTargetTransformInfoWrapperPass(TM->getTargetIRAnalysis()));

  MPM.add(new TargetLibraryInfoWrapperPass(*TLII));
  FPM.add(new TargetLibraryInfoWrapperPass(*TLII));

  // Translate SPV-IR to OCL20-IR first.
  MaterializerMPM.add(createDPCPPPreprocessSPIRVFriendlyIRLegacyPass());
  MaterializerMPM.add(createSPIRVLowerConstExprLegacy());
  MaterializerMPM.add(createSPIRVToOCL20Legacy());
#ifndef NDEBUG
  MaterializerMPM.add(createVerifierPass());
#endif // #ifndef NDEBUG

  registerPipelineStartCallback(PMBuilder);
  registerVectorizerStartCallback(PMBuilder);
  registerOptimizerLastCallback(PMBuilder);

  PMBuilder.populateFunctionPassManager(FPM);
  PMBuilder.populateModulePassManager(MPM);

  registerLastPasses(PMBuilder);
}

void OptimizerLTOLegacyPM::registerPipelineStartCallback(
    PassManagerBuilder &PMBuilder) {
  FPM.add(createUnifyFunctionExitNodesPass());
  auto EP = (PMBuilder.OptLevel == 0)
                ? PassManagerBuilder::EP_EnabledOnOptLevel0
                : PassManagerBuilder::EP_ModuleOptimizerEarly;
  PMBuilder.addExtension(
      EP, [this](const PassManagerBuilder &PMB, legacy::PassManagerBase &MPM) {
        MPM.add(createBuiltinLibInfoAnalysisLegacyPass(m_RtlModules));
        MPM.add(createDPCPPEqualizerLegacyPass());
        Triple TargetTriple(m_M.getTargetTriple());
        if (TargetTriple.isArch64Bit() &&
            TargetTriple.isOSWindows())
          MPM.add(createCoerceWin64TypesLegacyPass());

        if (m_IsFpgaEmulator)
          MPM.add(createRemoveAtExitLegacyPass());

        MPM.add(createSetPreferVectorWidthLegacyPass(
            VectorizerCommon::getCPUIdISA(Config.GetCpuId())));
        if (m_IsSPIRV && Config.GetRelaxedMath())
          MPM.add(createAddFastMathLegacyPass());

        MPM.add(createDuplicateCalledKernelsLegacyPass());
        if (PMB.OptLevel > 0)
          MPM.add(createInternalizeNonKernelFuncLegacyPass());
        MPM.add(createFMASplitterPass());
        MPM.add(createAddFunctionAttrsLegacyPass());

        if (PMB.OptLevel > 0) {
          MPM.add(createCFGSimplificationPass());
          if (PMB.OptLevel == 1)
            MPM.add(createPromoteMemoryToRegisterPass());
          else
            MPM.add(createSROAPass());
          MPM.add(createInstructionCombiningPass());
          MPM.add(createInstSimplifyLegacyPass());
        }

        // Flatten get_{local, global}_linear_id()
        if (m_IsOcl20)
          MPM.add(createLinearIdResolverPass());

        if (m_IsFpgaEmulator) {
          MPM.add(createDPCPPRewritePipesLegacyPass());
          MPM.add(createChannelPipeTransformationLegacyPass());
          MPM.add(createPipeIOTransformationLegacyPass());
          MPM.add(createPipeOrderingLegacyPass());
          MPM.add(createAutorunReplicatorLegacyPass());
        }

        // OCL2.0 add Generic Address Resolution
        // LLVM IR converted from any version of SPIRV may have Generic
        // adress space pointers.
        if (PMB.OptLevel > 0 && (m_IsOcl20 || m_IsSPIRV)) {
          // Static resolution of generic address space pointers
          MPM.add(createPromoteMemoryToRegisterPass());
          MPM.add(createInferAddressSpacesPass(
              CompilationUtils::ADDRESS_SPACE_GENERIC));
        }

        MPM.add(createBasicAAWrapperPass());
        if (Config.EnableOCLAA()) {
          MPM.add(createDPCPPAliasAnalysisLegacyPass());
          MPM.add(createDPCPPExternalAliasAnalysisLegacyPass());
        }

        MPM.add(createBuiltinCallToInstLegacyPass());
      });
}

void OptimizerLTOLegacyPM::registerVectorizerStartCallback(
    PassManagerBuilder &PMBuilder) {
  PMBuilder.addExtension(
      PassManagerBuilder::EP_VectorizerStart,
      [this](const PassManagerBuilder &, legacy::PassManagerBase &MPM) {
        MPM.add(createDetectRecursionLegacyPass());

        if (m_IsFpgaEmulator)
          MPM.add(createPipeSupportLegacyPass());

        if (Config.EnableOCLAA()) {
          MPM.add(createDPCPPAliasAnalysisLegacyPass());
          MPM.add(createDPCPPExternalAliasAnalysisLegacyPass());
        }

        MPM.add(createReassociatePass());

        if (m_IsOcl20 || m_IsSPIRV) {
          // Repeat resolution of generic address space pointers after LLVM
          // IR was optimized.
          MPM.add(createInferAddressSpacesPass(
              CompilationUtils::ADDRESS_SPACE_GENERIC));
          // Cleanup after InferAddressSpaces pPass.
          MPM.add(createCFGSimplificationPass());
          MPM.add(createSROAPass());
          MPM.add(createEarlyCSEPass());
          MPM.add(createPromoteMemoryToRegisterPass());
          MPM.add(createInstructionCombiningPass());
          // No need to run function inlining pass here, because if there are
          // still non-inlined functions, then we don't have to inline new ones.
        }

        MPM.add(createResolveVarTIDCallLegacyPass());

        if (m_IsSYCL) {
          MPM.add(createTaskSeqAsyncHandlingLegacyPass());

          // Support matrix fill and slice.
          MPM.add(createResolveMatrixFillLegacyPass());
          MPM.add(createResolveMatrixLayoutLegacyPass());
          MPM.add(createResolveMatrixWISliceLegacyPass());
        }

        MPM.add(createInferArgumentAliasLegacyPass());
        MPM.add(createUnifyFunctionExitNodesPass());

        MPM.add(createBasicAAWrapperPass());

        // Should be called before vectorizer!
        MPM.add(createInstToFuncCallLegacyPass(ISA));

        MPM.add(createDPCPPKernelAnalysisLegacyPass());
        MPM.add(createCFGSimplificationPass());
        MPM.add(createWGLoopBoundariesLegacyPass());
        MPM.add(createDeadCodeEliminationPass());
        MPM.add(createCFGSimplificationPass());
        MPM.add(createDeduceMaxWGDimLegacyPass());

        if (Config.GetTransposeSize() == 1)
          return;

        // In profiling mode remove llvm.dbg.value calls before vectorizer.
        if (Config.GetProfilingFlag())
          MPM.add(createProfilingInfoLegacyPass());

        MPM.add(createSinCosFoldLegacyPass());

        // Replace 'div' and 'rem' instructions with calls to optimized library
        // functions
        MPM.add(createMathLibraryFunctionsReplacementPass());

        // Merge returns : this pass ensures that the function has at most one
        // return instruction.
        MPM.add(createUnifyFunctionExitNodesPass());
        MPM.add(
            createCFGSimplificationPass(SimplifyCFGOptions()
                                            .bonusInstThreshold(1)
                                            .forwardSwitchCondToPhi(false)
                                            .convertSwitchToLookupTable(false)
                                            .needCanonicalLoops(true)
                                            .sinkCommonInsts(true)));
        MPM.add(createInstructionCombiningPass());
        MPM.add(createGVNHoistPass());
        MPM.add(createDeadCodeEliminationPass());
        MPM.add(createReqdSubGroupSizeLegacyPass());

        // Analyze and set VF for kernels. This pass may throw
        // VFAnalysisDiagInfo error if VF checking fails.
        MPM.add(createSetVectorizationFactorLegacyPass(ISA));

        // Create and materialize "vector-variants" attribute.
        MPM.add(createVectorVariantLoweringLegacyPass(ISA));
        MPM.add(createCreateSimdVariantPropagationLegacyPass());
        MPM.add(createSGSizeCollectorLegacyPass(ISA));
        MPM.add(createSGSizeCollectorIndirectLegacyPass(ISA));

        // We won't automatically switch vectorization dimension for SYCL.
        if (!m_IsSYCL)
          MPM.add(createVectorizationDimensionAnalysisLegacyPass());

        MPM.add(createDPCPPKernelVecClonePass(getVectInfos(), ISA,
                                              !m_IsSYCL && !m_IsOMP));
        MPM.add(createVectorVariantFillInLegacyPass());
        MPM.add(createUpdateCallAttrsLegacyPass());

        MPM.add(createPromoteMemoryToRegisterPass());
        MPM.add(createLoopSimplifyPass());
        MPM.add(createLICMPass());
      });
}

void OptimizerLTOLegacyPM::registerOptimizerLastCallback(
    PassManagerBuilder &PMBuilder) {
  PMBuilder.addExtension(
      PassManagerBuilder::EP_OptimizerLast,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &MPM) {
        addLastPassesImpl(PMBuilder.OptLevel, MPM);
      });
}

void OptimizerLTOLegacyPM::addLastPassesImpl(unsigned OptLevel,
                                             legacy::PassManagerBase &MPM) {
  if (Config.GetTransposeSize() != 1 &&
      (OptLevel != 0 || EnableO0Vectorization)) {
    // Post-vectorizer cleanup.
    MPM.add(createDPCPPKernelPostVecPass());
    if (OptLevel != 0) {
      MPM.add(createInstructionCombiningPass());
      MPM.add(createCFGSimplificationPass());
      MPM.add(createPromoteMemoryToRegisterPass());
      MPM.add(createAggressiveDCEPass());
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
      MPM.add(createVectorKernelEliminationLegacyPass());

    MPM.add(createHandleVPlanMaskLegacyPass(&getVPlanMaskedFuncs()));
  } else {
    // When forced VF equals 1 or in O0 case, check subgroup semantics AND
    // prepare subgroup_emu_size for sub-group emulation.
    MPM.add(createReqdSubGroupSizeLegacyPass());
    MPM.add(createSetVectorizationFactorLegacyPass(ISA));
  }

#ifdef _DEBUG
  MPM.add(createVerifierPass());
#endif

  MPM.add(createResolveSubGroupWICallLegacyPass(
      /*ResolveSGBarrier*/ false));
  if (OptLevel > 0)
    MPM.add(createOptimizeIDivAndIRemLegacyPass());

  MPM.add(createPreventDivCrashesLegacyPass());
  // We need InstructionCombining and GVN passes after PreventDivCrashes
  // passes to optimize redundancy introduced by those passes
  if (OptLevel > 0) {
    MPM.add(createInstructionCombiningPass());
    MPM.add(createGVNPass());
    MPM.add(createVectorCombinePass());
    // In specACCEL/124, InstCombine may generate a cross-barrier bool value
    // used as a condition of a 'br' instruction, which leads to performance
    // degradation. JumpThreading eliminates the cross-barrier value.
    MPM.add(createJumpThreadingPass());
  }

  // The m_debugType enum and profiling flag are mutually exclusive, with
  // precedence given to m_debugType.
  if (Config.GetProfilingFlag())
    MPM.add(createProfilingInfoLegacyPass());

  if (OptLevel > 0 && Config.GetStreamingAlways())
    MPM.add(createAddNTAttrLegacyPass());
  if (m_debugType == intel::Native)
    MPM.add(createImplicitGIDLegacyPass(/*HandleBarrier*/ false));
  MPM.add(createDPCPPKernelWGLoopCreatorLegacyPass(m_UseTLSGlobals));

  // Can't run loop unroll between WGLoopCreator and LoopIdiom for scalar
  // workload, which can benefit from LoopIdiom.
  if (OptLevel > 0 && Config.GetTransposeSize() != 1)
    MPM.add(createLoopUnrollPass(OptLevel, false, false, 24, 0, 0, 1));

  // Resolve __intel_indirect_call for scalar kernels.
  MPM.add(createIndirectCallLoweringLegacyPass());

  if (OptLevel > 0) {
    MPM.add(createDeadCodeEliminationPass()); // Delete dead instructions
    MPM.add(createCFGSimplificationPass());   // Simplify CFG
  }

  if (m_IsFpgaEmulator)
    MPM.add(createInfiniteLoopCreatorLegacyPass());

  // Barrier pass can't work with a token type, so here we remove region
  // directives
  MPM.add(createRemoveRegionDirectivesLegacyPass());

  MPM.add(createUnifyFunctionExitNodesPass());

  addBarrierPasses(OptLevel, MPM);

  // After adding loops run loop optimizations.
  if (OptLevel > 0) {
    // Add LoopSimplify pass before CLBuiltinLICM pass as CLBuiltinLICM pass
    // requires loops in Simplified Form.
    MPM.add(createLoopSimplifyPass());
    MPM.add(createLICMPass());
    MPM.add(createBuiltinLICMLegacyPass());
    MPM.add(createLoopStridedCodeMotionLegacyPass());
  }

  if (Config.GetRelaxedMath())
    MPM.add(createRelaxedMathLegacyPass());

  // The following three passes (AddImplicitArgs/AddImplicitArgs,
  // ResolveWICall, LocalBuffers) must run before BuiltinImportLegacyPass.
  if (m_UseTLSGlobals)
    MPM.add(createAddTLSGlobalsLegacyPass());
  else
    MPM.add(createAddImplicitArgsLegacyPass());
  MPM.add(createResolveWICallLegacyPass(Config.GetUniformWGSize(),
                                        m_UseTLSGlobals));
  MPM.add(createLocalBuffersLegacyPass(m_UseTLSGlobals));

  // clang converts OCL's local to global.
  // createLocalBuffersLegacyPass changes the local allocation from global to a
  // kernel argument.
  // The next pass createGlobalOptimizerPass cleans the unused global
  // allocation in order to make sure we will not allocate redundant space on
  // the jit
  if (OptLevel > 0 && m_debugType != intel::Native)
    MPM.add(createGlobalOptimizerPass());

#ifdef _DEBUG
  MPM.add(createVerifierPass());
#endif

  // Externalize globals if IR is generated from OpenMP offloading. Now we
  // cannot get address of globals with internal/private linkage from LLJIT
  // (by design), but it's necessary by OpenMP to pass address of declare
  // target variables to the underlying OpenCL Runtime via
  // clSetKernelExecInfo. So we have to externalize globals for IR generated
  // from OpenMP.
  if (m_IsOMP)
    MPM.add(createExternalizeGlobalVariablesLegacyPass());

  MPM.add(createBuiltinImportLegacyPass(CPUPrefix));
  if (OptLevel > 0) {
    // After the globals used in built-ins are imported - we can internalize
    // them with further wiping them out with GlobalDCE pass
    MPM.add(createInternalizeGlobalVariablesLegacyPass());
  }
  MPM.add(createBuiltinCallToInstLegacyPass());

#ifdef _DEBUG
  MPM.add(createVerifierPass());
#endif

  if (OptLevel > 0) {
    MPM.add(createFunctionInliningPass(4096));
    // Cleaning up internal globals
    MPM.add(createGlobalDCEPass());
    // AddImplicitArgs pass may create dead implicit arguments.
    MPM.add(createDeadArgEliminationPass());
    MPM.add(createSROAPass());
    MPM.add(createLoopSimplifyPass());
    MPM.add(createLICMPass());
    MPM.add(createLoopIdiomPass());
    MPM.add(createLoopDeletionPass());
    MPM.add(createLoopStridedCodeMotionLegacyPass());
    MPM.add(createCFGSimplificationPass());
  } else if (m_IsOcl20) {
    // Ensure that the built-in functions to be processed by
    // PatchCallbackArgsPass are inlined.
    MPM.add(createAlwaysInlinerLegacyPass());
  }

  // Some built-in functions contain calls to external functions which take
  // arguments that are retrieved from the function's implicit arguments.
  // Currently only applies to OpenCL 2.x
  if (m_IsOcl20)
    MPM.add(createPatchCallbackArgsLegacyPass(m_UseTLSGlobals));

  // PrepareKernelArgs pass creates a wrapper function and inlines the
  // original kernel to the wrapper. The kernel usually contains several
  // "noalias" pointer args, such as '%pSpecialBuf', '%pWorkDim', '%pWGId'
  // and '%pLocalMemBase', and inliner (which is explicitly invoked in
  // PrepareKernelArgs pass) would try to add new alias scopes for each
  // noalias argument.
  // See llvm/lib/Transforms/Utils/InlineFunction.cpp::AddAliasScopeMetadata()
  // However, the added alias scopes are of relatively coarse granularity:
  // e.g. A load from '%pSpecialBuf' with limited offset might be assumed
  // to potentially access arbitrary memory range based on '%pSpecialBuf'
  // after inlining.
  // This makes AliasAnalysis-related transforms hard to optimize after
  // PrepareKernelArgs pass.
  // Also, PrepareKernelArgs only creates a thin wrapper for the kernel --
  // this pass itself won't generate much suboptimal codes.
  // Therefore, we basically should run the most optimization passes
  // (especially AliasAnalysis-related) before the PrepareKernelArgs pass.
  MPM.add(createPrepareKernelArgsLegacyPass(m_UseTLSGlobals));

  if (OptLevel > 0) {
    // These passes come after PrepareKernelArgsLegacyPass to eliminate the
    // redundancy produced by it.
    MPM.add(createCFGSimplificationPass());
    MPM.add(createSROAPass());
    MPM.add(createInstructionCombiningPass());
    MPM.add(createGVNPass());
    MPM.add(createDeadStoreEliminationPass());
    MPM.add(createAggressiveDCEPass());
    MPM.add(createEarlyCSEPass());
    MPM.add(createInstructionCombiningPass());
  }
}

void OptimizerLTOLegacyPM::addBarrierPasses(unsigned OptLevel, legacy::PassManagerBase &MPM) {
  if (OptLevel > 0 || EnableO0Vectorization) {
    MPM.add(createReplaceScalarWithMaskLegacyPass());

    // Resolve subgreoup call introduced by ReplaceScalarWithMask pass.
    MPM.add(createResolveSubGroupWICallLegacyPass(
        /*ResolveSGBarrier*/ false));
    if (OptLevel > 0) {
      MPM.add(createDeadCodeEliminationPass());
      MPM.add(createCFGSimplificationPass());
      MPM.add(createPromoteMemoryToRegisterPass());
      MPM.add(createPhiCanonicalizationLegacyPass());
      MPM.add(createRedundantPhiNodeLegacyPass());
    }
  }

  MPM.add(createGroupBuiltinLegacyPass());
  MPM.add(createBarrierInFunctionLegacyPass());

  // Only run this when not debugging or when not in native (gdb) debugging
  if (m_debugType != intel::Native) {
    // This optimization removes debug information from extraneous barrier
    // calls by deleting them.
    MPM.add(
        createRemoveDuplicatedBarrierLegacyPass(m_debugType == intel::Native));
  }

  // Begin sub-group emulation
  MPM.add(createSGBuiltinLegacyPass(getVectInfos()));
  MPM.add(createSGBarrierPropagateLegacyPass());
  MPM.add(createSGBarrierSimplifyLegacyPass());
  // Insert ImplicitGIDPass in the middle of subgroup emulation to track GIDs in
  // emulation loops
  if (m_debugType == intel::Native)
    MPM.add(createImplicitGIDLegacyPass(/*HandleBarrier*/ true));

  // Resume sub-group emulation
  MPM.add(createSGValueWidenLegacyPass());
  MPM.add(createSGLoopConstructLegacyPass());
#ifdef _DEBUG
  MPM.add(createVerifierPass());
#endif
  // End sub-group emulation

  // Resolve subgroup barriers after subgroup emulation passes
  MPM.add(createResolveSubGroupWICallLegacyPass(
      /*ResolveSGBarrier*/ true));
  MPM.add(createSplitBBonBarrierLegacyPass());
  if (OptLevel > 0)
    MPM.add(createReduceCrossBarrierValuesLegacyPass());
  MPM.add(createKernelBarrierLegacyPass(m_debugType == intel::Native,
                                        m_UseTLSGlobals));
#ifdef _DEBUG
  MPM.add(createVerifierPass());
#endif

  if (OptLevel > 0)
    MPM.add(createPromoteMemoryToRegisterPass());
}

void OptimizerLTOLegacyPM::registerLastPasses(PassManagerBuilder &PMBuilder) {
  if (PMBuilder.OptLevel == 0) {
    // In O0 pipeline, there is no EP_OptimizerLast extension point, so we add
    // passes to the end of pipeline.
    MPM.add(createDPCPPKernelAnalysisLegacyPass());
    addLastPassesImpl(PMBuilder.OptLevel, MPM);
  }

  MPM.add(createCleanupWrappedKernelLegacyPass());
}

void OptimizerLTOLegacyPM::Optimize(raw_ostream &LogStream) {
  // Set custom DiagnosticHandler callback.
  setDiagnosticHandler(LogStream);

  MaterializerMPM.run(m_M);

  FPM.doInitialization();
  for (Function &F : m_M) {
    if (!F.isDeclaration())
      FPM.run(F);
  }
  FPM.doFinalization();

  MPM.run(m_M);
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
