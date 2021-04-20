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
#include "VecConfig.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddImplicitArgs.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierInFunctionPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/CleanupWrappedKernel.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPEqualizer.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelWGLoopCreator.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PhiCanonicalization.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PrepareKernelArgs.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/RedundantPhiNodePass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveWICall.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SplitBBonBarrierPass.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerLTO::OptimizerLTO(Module *M, const intel::OptimizerConfig *Config)
    : Optimizer(M), Config(Config), DebugPassManager(false) {}

OptimizerLTO::~OptimizerLTO() {}

void OptimizerLTO::Optimize() {
  TargetMachine *TM = Config->GetTargetMachine();
  assert(TM && "Uninitialized TargetMachine!");

  PipelineTuningOptions PTO;
  PTO.LoopUnrolling = false;
  PTO.LoopInterleaving = false;
  PTO.LoopVectorization = false;
  PTO.SLPVectorization = false;
  PTO.MergeFunctions = false;
  PTO.CallGraphProfile = true;
  PTO.Coroutines = false;

  Optional<PGOOptions> PGOOpt;
  PassInstrumentationCallbacks PIC;
  StandardInstrumentations SI(DebugPassManager);
  SI.registerCallbacks(PIC);
  PassBuilder PB(DebugPassManager, TM, PTO, PGOOpt, &PIC);

  LoopAnalysisManager LAM(DebugPassManager);
  FunctionAnalysisManager FAM(DebugPassManager);
  CGSCCAnalysisManager CGAM(DebugPassManager);
  ModuleAnalysisManager MAM(DebugPassManager);

  // Register the AA manager first so that our version is the one used.
  FAM.registerPass([&] { return PB.buildDefaultAAPipeline(); });

  // Register the target library analysis directly and give it a customized
  // preset TLI.
  Triple TargetTriple(m_M->getTargetTriple());
  TLII.reset(new TargetLibraryInfoImpl(TargetTriple));
  FAM.registerPass([&] { return TargetLibraryAnalysis(*TLII); });

  // Register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  registerPipelineStartCallback(PB);
  registerVectorizerStartCallback(PB);
  registerOptimizerLastCallback(PB);

  ModulePassManager MPM(DebugPassManager);

  if (Config->GetDisableOpt())
    MPM = PB.buildO0DefaultPipeline(PassBuilder::OptimizationLevel::O0);
  else
    MPM = PB.buildPerModuleDefaultPipeline(PassBuilder::OptimizationLevel::O3);

  MPM.run(*m_M, MAM);
}

void OptimizerLTO::registerPipelineStartCallback(PassBuilder &PB) {
  PB.registerPipelineStartEPCallback(
      [](ModulePassManager &MPM, PassBuilder::OptimizationLevel Level) {
        MPM.addPass(DPCPPEqualizerPass());
        MPM.addPass(DPCPPKernelAnalysisPass());
      });
}

void OptimizerLTO::registerVectorizerStartCallback(PassBuilder &PB) {
  PB.registerVectorizerStartEPCallback(
      [](FunctionPassManager &FPM, PassBuilder::OptimizationLevel Level) {
        FPM.addPass(UnifyFunctionExitNodesPass());
      });
}

void OptimizerLTO::registerOptimizerLastCallback(PassBuilder &PB) {
  PB.registerOptimizerLastEPCallback([](ModulePassManager &MPM,
                                        PassBuilder::OptimizationLevel Level) {
    MPM.addPass(DPCPPKernelWGLoopCreatorPass());
    // Barrier passes begin.
    MPM.addPass(createModuleToFunctionPassAdaptor(PhiCanonicalization()));
    MPM.addPass(createModuleToFunctionPassAdaptor(RedundantPhiNode()));
    MPM.addPass(BarrierInFunction());
    MPM.addPass(SplitBBonBarrier());
    MPM.addPass(KernelBarrier(/*NativeDebug*/ false, /*UseTLSGlobals*/ false));
    // Barrier passes end.
    MPM.addPass(AddImplicitArgsPass());
    MPM.addPass(ResolveWICallPass());
    MPM.addPass(PrepareKernelArgsPass());
  });
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
