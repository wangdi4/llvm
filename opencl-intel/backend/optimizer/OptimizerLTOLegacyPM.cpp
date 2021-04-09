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
#include "VecConfig.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

OptimizerLTOLegacyPM::OptimizerLTOLegacyPM(Module *M,
                                           const intel::OptimizerConfig *Config)
    : Optimizer(M), Config(Config), FPM(M) {
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
  PMBuilder.DisableUnrollLoops = true;
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

  // Set up the per-function pass manager.
  FPM.add(new TargetLibraryInfoWrapperPass(*TLII));

  PMBuilder.populateFunctionPassManager(FPM);
  PMBuilder.populateModulePassManager(MPM);
}

void OptimizerLTOLegacyPM::Optimize() {
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
