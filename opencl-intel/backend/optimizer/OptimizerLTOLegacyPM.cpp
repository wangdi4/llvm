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

#include "OptimizerLTOLegacyPM.h"
#include "LLVMSPIRVLib.h"
#include "VecConfig.h"
#include "VectorizerCommon.h"
#include "llvm/Analysis/LoopPass.h"
#ifndef NDEBUG
#include "llvm/IR/Verifier.h"
#endif // #ifndef NDEBUG
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Vectorize.h"

// If set, then optimization passes will process functions as if they have the
// optnone attribute.
extern bool DPCPPForceOptnone;

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
  PMBuilder.PrepareForThinLTO = false;
  PMBuilder.PrepareForLTO = false;
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
    auto Params =
        getInlineParams(PMBuilder.OptLevel, PMBuilder.SizeLevel,
                        PMBuilder.PrepareForThinLTO, PMBuilder.PrepareForLTO);
    Params.DefaultThreshold = 4096;
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
  MaterializerMPM.add(createNameAnonGlobalPass());
#ifndef NDEBUG
  MaterializerMPM.add(llvm::createVerifierPass());
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
  if (PMBuilder.OptLevel > 0 && (m_IsOcl20 || m_IsSPIRV))
    FPM.add(createInferAddressSpacesPass(
        DPCPPKernelCompilationUtils::ADDRESS_SPACE_GENERIC));

  auto EP = (PMBuilder.OptLevel == 0)
                ? PassManagerBuilder::EP_EnabledOnOptLevel0
                : PassManagerBuilder::EP_ModuleOptimizerEarly;
  PMBuilder.addExtension(
      EP, [this](const PassManagerBuilder &PMB, legacy::PassManagerBase &MPM) {
        MPM.add(createParseAnnotateAttributesPass());
        MPM.add(createBuiltinLibInfoAnalysisLegacyPass(m_RtlModules));
        MPM.add(createDPCPPEqualizerLegacyPass());
        Triple TargetTriple(m_M.getTargetTriple());
        if (TargetTriple.isArch64Bit() && TargetTriple.isOSWindows())
          MPM.add(createCoerceWin64TypesLegacyPass());
        MPM.add(createDuplicateCalledKernelsLegacyPass());
        if (PMB.OptLevel > 0)
          MPM.add(createInternalizeNonKernelFuncLegacyPass());
        MPM.add(createAddFunctionAttrsLegacyPass());
        MPM.add(createLinearIdResolverPass());
        MPM.add(createBuiltinCallToInstLegacyPass());
      });
}

void OptimizerLTOLegacyPM::registerVectorizerStartCallback(
    PassManagerBuilder &PMBuilder) {
  PMBuilder.addExtension(
      PassManagerBuilder::EP_VectorizerStart,
      [this](const PassManagerBuilder &, legacy::PassManagerBase &MPM) {
        MPM.add(createReassociatePass());

        if (m_IsOcl20 || m_IsSPIRV) {
          // Repeat resolution of generic address space pointers after LLVM
          // IR was optimized.
          MPM.add(createInferAddressSpacesPass(
              DPCPPKernelCompilationUtils::ADDRESS_SPACE_GENERIC));
          // Cleanup after InferAddressSpaces pPass.
          MPM.add(createCFGSimplificationPass());
          MPM.add(createSROAPass());
          MPM.add(createEarlyCSEPass());
          MPM.add(createPromoteMemoryToRegisterPass());
          MPM.add(createInstructionCombiningPass());
          // No need to run function inlining pass here, because if there are
          // still non-inlined functions, then we don't have to inline new ones.
        }

        VectorVariant::ISAClass ISA =
            Intel::VectorizerCommon::getCPUIdISA(Config.GetCpuId());

        MPM.add(createDPCPPKernelAnalysisLegacyPass());
        MPM.add(createWGLoopBoundariesLegacyPass());
        MPM.add(createDeadCodeEliminationPass());
        MPM.add(createCFGSimplificationPass());
        MPM.add(createDeduceMaxWGDimLegacyPass());
        MPM.add(createInstToFuncCallLegacyPass(ISA));
        if (Config.GetTransposeSize() == 1)
          return;

        MPM.add(createSinCosFoldLegacyPass());
        // Replace 'div' and 'rem' instructions with calls to optimized library
        // functions
        MPM.add(createMathLibraryFunctionsReplacementPass());

        // Analyze and set VF for kernels.
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
  if (OptLevel > 0) {
    if (Config.GetTransposeSize() != 1) {
      MPM.add(createDPCPPKernelPostVecPass());
      MPM.add(createVPODirectiveCleanupPass());
      MPM.add(createHandleVPlanMaskLegacyPass(&getVPlanMaskedFuncs()));
    }
    MPM.add(createInstructionCombiningPass());
    MPM.add(createCFGSimplificationPass());
    MPM.add(createPromoteMemoryToRegisterPass());
    MPM.add(createAggressiveDCEPass());
  }
  MPM.add(createResolveSubGroupWICallLegacyPass(m_RtlModules,
                                                /*ResolveSGBarrier*/ false));
  if (OptLevel > 0 && Config.GetStreamingAlways())
    MPM.add(createAddNTAttrLegacyPass());
  MPM.add(createDPCPPKernelWGLoopCreatorLegacyPass());

  // Can't run loop unroll between WGLoopCreator and LoopIdiom for scalar
  // workload, which can benefit from LoopIdiom.
  if (OptLevel > 0 && Config.GetTransposeSize() != 1)
    MPM.add(createLoopUnrollPass(OptLevel, false, false, 24, 0, 0, 1));

  // Resolve __intel_indirect_call for scalar kernels.
  MPM.add(createIndirectCallLoweringLegacyPass());

  addBarrierPasses(OptLevel, MPM);

  // The following three passes (AddImplicitArgs/AddImplicitArgs,
  // ResolveWICall, LocalBuffers) must run before BuiltinImportLegacyPass.
  if (m_UseTLSGlobals)
    MPM.add(createAddTLSGlobalsLegacyPass());
  else
    MPM.add(createAddImplicitArgsLegacyPass());
  MPM.add(createResolveWICallLegacyPass(Config.GetUniformWGSize(),
                                        m_UseTLSGlobals));
  MPM.add(createLocalBuffersLegacyPass(m_UseTLSGlobals));
  MPM.add(createBuiltinImportLegacyPass(m_RtlModules, CPUPrefix));
  if (OptLevel > 0)
    MPM.add(createGlobalDCEPass());
  MPM.add(createBuiltinCallToInstLegacyPass());
  if (OptLevel > 0) {
    MPM.add(createFunctionInliningPass(4096));
    // AddImplicitArgs pass may create dead implicit arguments.
    MPM.add(createDeadArgEliminationPass());
    MPM.add(createSROAPass());
    MPM.add(createLoopSimplifyPass());
    MPM.add(createLICMPass());
    MPM.add(createLoopIdiomPass());
    MPM.add(createLoopDeletionPass());
    MPM.add(createLoopStridedCodeMotionLegacyPass());
    MPM.add(createCFGSimplificationPass());
  } else {
    MPM.add(createAlwaysInlinerLegacyPass());
  }
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
  if (OptLevel > 0) {
    // TODO: insert ReplaceScalarWithMask pass here
    // Resolve subgreoup call introduced by ReplaceScalarWithMask pass.
    MPM.add(createResolveSubGroupWICallLegacyPass(m_RtlModules,
                                                  /*ResolveSGBarrier*/ false));
  }
  MPM.add(createPhiCanonicalizationLegacyPass());
  MPM.add(createRedundantPhiNodeLegacyPass());
  MPM.add(createGroupBuiltinLegacyPass());
  MPM.add(createBarrierInFunctionLegacyPass());

  // Resolve subgroup barriers after subgroup emulation passes
  MPM.add(createResolveSubGroupWICallLegacyPass(m_RtlModules,
                                                /*ResolveSGBarrier*/ true));
  MPM.add(createSplitBBonBarrierLegacyPass());
  if (OptLevel > 0)
    MPM.add(createReduceCrossBarrierValuesLegacyPass());
  MPM.add(createKernelBarrierLegacyPass(m_debugType == intel::Native,
                                        m_UseTLSGlobals));
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
