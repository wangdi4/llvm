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

#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerLTOLegacyPM::OptimizerLTOLegacyPM(
    Module *M, llvm::SmallVector<llvm::Module *, 2> &RtlModuleList,
    const intel::OptimizerConfig *Config)
    : Optimizer(M, RtlModuleList, Config), FPM(M) {
  CreatePasses();
}

OptimizerLTOLegacyPM::~OptimizerLTOLegacyPM() {}

/// Ported from clang EmitAssemblyHelper::CreatePasses.
void OptimizerLTOLegacyPM::CreatePasses() {
  TargetMachine *TM = Config->GetTargetMachine();
  assert(TM && "Uninitialized TargetMachine!");

  Triple TargetTriple(m_M->getTargetTriple());
  TLII.reset(new TargetLibraryInfoImpl(TargetTriple));
  PassManagerBuilder PMBuilder;

  PMBuilder.OptLevel = Config->GetDisableOpt() ? 0 : 3;
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

  // At O0 and O1 we only run the always inliner which is more efficient. At
  // higher optimization levels we run the normal inliner.
  if (PMBuilder.OptLevel <= 1) {
    PMBuilder.Inliner =
        createAlwaysInlinerLegacyPass(/*InsertLifetimeIntrinsics*/ false);
  } else {
    // We do not want to inline hot callsites for SamplePGO module-summary build
    // because profile annotation will happen again in ThinLTO backend, and we
    // want the IR of the hot path to match the profile.
    PMBuilder.Inliner = createFunctionInliningPass(
        PMBuilder.OptLevel, PMBuilder.SizeLevel, PMBuilder.PrepareForThinLTO,
        PMBuilder.PrepareForLTO);
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
  FPM.add(createInferAddressSpacesPass());

  auto EP = (PMBuilder.OptLevel == 0)
                ? PassManagerBuilder::EP_EnabledOnOptLevel0
                : PassManagerBuilder::EP_ModuleOptimizerEarly;
  PMBuilder.addExtension(
      EP, [&](const PassManagerBuilder &PMB, legacy::PassManagerBase &MPM) {
        MPM.add(createParseAnnotateAttributesPass());
        MPM.add(createDPCPPEqualizerLegacyPass());
        Triple TargetTriple(m_M->getTargetTriple());
        if (TargetTriple.isArch64Bit() && TargetTriple.isOSWindows())
          MPM.add(createCoerceWin64TypesLegacyPass());
        MPM.add(createDuplicateCalledKernelsLegacyPass());
        if (PMB.OptLevel > 0)
          MPM.add(createInternalizeNonKernelFuncLegacyPass());
        MPM.add(createAddFunctionAttrsLegacyPass());
        MPM.add(createLinearIdResolverPass());
        MPM.add(createBuiltinCallToInstLegacyPass());
        MPM.add(createDPCPPKernelAnalysisLegacyPass());
      });
}

void OptimizerLTOLegacyPM::registerVectorizerStartCallback(
    PassManagerBuilder &PMBuilder) {
  PMBuilder.addExtension(
      PassManagerBuilder::EP_VectorizerStart,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &MPM) {
        if (Config->GetTransposeSize() == 1)
          return;
        VectorVariant::ISAClass ISA =
            Intel::VectorizerCommon::getCPUIdISA(Config->GetCpuId());
        // Analyze and set VF for kernels.
        MPM.add(createSetVectorizationFactorLegacyPass(ISA));

        // Create and materialize "vector-variants" attribute.
        MPM.add(createVectorVariantLoweringLegacyPass(ISA));
        MPM.add(createCreateSimdVariantPropagationLegacyPass());
        MPM.add(createSGSizeCollectorLegacyPass(ISA));
        MPM.add(createSGSizeCollectorIndirectLegacyPass(ISA));

        MPM.add(createDPCPPKernelVecClonePass(getVectInfos(), ISA,
                                              !m_IsSYCL && !m_IsOMP));
        MPM.add(createVectorVariantFillInLegacyPass());
        MPM.add(createUpdateCallAttrsLegacyPass());
      });
}

void OptimizerLTOLegacyPM::registerOptimizerLastCallback(
    PassManagerBuilder &PMBuilder) {
  PMBuilder.addExtension(
      PassManagerBuilder::EP_OptimizerLast,
      [&](const PassManagerBuilder &, legacy::PassManagerBase &MPM) {
        if (Config->GetTransposeSize() != 1) {
          MPM.add(createDPCPPKernelPostVecPass());
          MPM.add(createVPODirectiveCleanupPass());
          MPM.add(createHandleVPlanMaskLegacyPass(&getVPlanMaskedFuncs()));
        }
        MPM.add(createInstructionCombiningPass());
        MPM.add(createCFGSimplificationPass());
        MPM.add(createPromoteMemoryToRegisterPass());
        MPM.add(createAggressiveDCEPass());
        MPM.add(createResolveSubGroupWICallLegacyPass(
            m_RtlModules, /*ResolveSGBarrier*/ false));
        MPM.add(createDPCPPKernelWGLoopCreatorLegacyPass());
        // Resolve __intel_indirect_call for scalar kernels.
        MPM.add(createIndirectCallLoweringLegacyPass());
        // Barrier passes begin.
        // TODO: insert ReplaceScalarWithMask pass here

        // Resolve subgreoup call introduced by ReplaceScalarWithMask pass.
        MPM.add(createResolveSubGroupWICallLegacyPass(
            m_RtlModules, /*ResolveSGBarrier*/ false));
        MPM.add(createPhiCanonicalizationLegacyPass());
        MPM.add(createRedundantPhiNodeLegacyPass());
        MPM.add(createGroupBuiltinLegacyPass(m_RtlModules));
        MPM.add(createBarrierInFunctionLegacyPass());

        // Resolve subgroup barriers after subgroup emulation passes
        MPM.add(createResolveSubGroupWICallLegacyPass(
            m_RtlModules, /*ResolveSGBarrier*/ true));
        MPM.add(createSplitBBonBarrierLegacyPass());
        MPM.add(createKernelBarrierLegacyPass(m_debugType == intel::Native,
                                              /*UseTLSGlobals*/ false));
        // Barrier passes end.

        MPM.add(createLICMPass());
        MPM.add(createLoopIdiomPass());
        MPM.add(createCFGSimplificationPass());
        MPM.add(createAddImplicitArgsLegacyPass());
        MPM.add(createResolveWICallLegacyPass(false, false));
        MPM.add(createLocalBuffersLegacyPass(/*UseTLSGlobals*/false));
        MPM.add(createBuiltinImportLegacyPass(m_RtlModules, CPUPrefix));
        MPM.add(createBuiltinCallToInstLegacyPass());
        // AddImplicitArgs pass may create dead implicit arguments.
        MPM.add(createDeadArgEliminationPass());
        MPM.add(createPrepareKernelArgsLegacyPass(false));
      });
}

void OptimizerLTOLegacyPM::registerLastPasses(
    llvm::PassManagerBuilder &PMBuilder) {
  if (PMBuilder.OptLevel == 0) {
    // In O0 pipeline, there is no EP_OptimizerLast extension point, so we add
    // following passes to the end of pipeline.
    MPM.add(createResolveSubGroupWICallLegacyPass(m_RtlModules,
                                                  /*ResolveSGBarrier*/ false));
    MPM.add(createDPCPPKernelWGLoopCreatorLegacyPass());
    MPM.add(createIndirectCallLoweringLegacyPass());
    // Barrier passes begin.
    MPM.add(createPhiCanonicalizationLegacyPass());
    MPM.add(createRedundantPhiNodeLegacyPass());
    MPM.add(createGroupBuiltinLegacyPass(m_RtlModules));
    MPM.add(createBarrierInFunctionLegacyPass());
    // Resolve subgroup barriers after subgroup emulation passes
    MPM.add(createResolveSubGroupWICallLegacyPass(m_RtlModules,
                                                  /*ResolveSGBarrier*/ true));
    MPM.add(createSplitBBonBarrierLegacyPass());
    MPM.add(createKernelBarrierLegacyPass(m_debugType == intel::Native,
                                          /*UseTLSGlobals*/ false));
    // Barrier passes end.
    MPM.add(createAddImplicitArgsLegacyPass());
    MPM.add(createResolveWICallLegacyPass(false, false));
    MPM.add(createLocalBuffersLegacyPass(/*UseTLSGlobals*/false));
    MPM.add(createBuiltinImportLegacyPass(m_RtlModules, CPUPrefix));
    MPM.add(createBuiltinCallToInstLegacyPass());
    MPM.add(createPrepareKernelArgsLegacyPass(false));
  }

  MPM.add(createCleanupWrappedKernelLegacyPass());
}

void OptimizerLTOLegacyPM::Optimize(llvm::raw_ostream &LogStream) {
  MaterializerMPM.run(*m_M);

  FPM.doInitialization();
  for (Function &F : *m_M) {
    if (!F.isDeclaration())
      FPM.run(F);
  }
  FPM.doFinalization();

  MPM.run(*m_M);
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
