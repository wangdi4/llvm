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

#pragma once

#include "Optimizer.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 *  Run llvm pass manager on the given module.
 */
class OptimizerLTO : public Optimizer {
public:
  OptimizerLTO(llvm::Module &M,
               llvm::SmallVector<llvm::Module *, 2> &RtlModules,
               const intel::OptimizerConfig &Config, bool DebugPassManager);

  ~OptimizerLTO();

  /// Run pass manager on a module.
  void Optimize(llvm::raw_ostream &LogStream) override;

private:
  /// Register a callback to the start of the pipeline.
  void registerPipelineStartCallback(llvm::PassBuilder &PB);

  /// Register a callback before the function optimization pipeline.
  void registerOptimizerEarlyCallback(llvm::PassBuilder &PB);

  /// Register a callback to before the vectorizer.
  void registerVectorizerStartCallback(llvm::PassBuilder &PB);

  /// Register a callback to the very end of the function optimization pipeline.
  void registerOptimizerLastCallback(llvm::PassBuilder &PB);

  /// Add barrier related passes to pass manager.
  void addBarrierPasses(ModulePassManager &MPM, OptimizationLevel Level);

  /// Register passes that run at the end of pipeline.
  void registerLastPasses(llvm::ModulePassManager &MPM);

  bool DebugPassManager;

  std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
