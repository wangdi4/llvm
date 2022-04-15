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
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 *  Run llvm legacy pass manager on the given module.
 */
class OptimizerLTOLegacyPM : public Optimizer {
public:
  OptimizerLTOLegacyPM(llvm::Module &M,
                       llvm::SmallVector<llvm::Module *, 2> &RtlModules,
                       const intel::OptimizerConfig &Config);

  ~OptimizerLTOLegacyPM();

  /// Run pass manager on a module.
  void Optimize(llvm::raw_ostream &LogStream) override;

private:
  /// Add passes to pass managers.
  void CreatePasses();

  /// Register a callback to the start of the pipeline.
  void registerPipelineStartCallback(llvm::PassManagerBuilder &PMBuilder);

  /// Register a callback to before the vectorizer.
  void registerVectorizerStartCallback(llvm::PassManagerBuilder &PMBuilder);

  /// Register a callback to the very end of the function optimization pipeline.
  void registerOptimizerLastCallback(llvm::PassManagerBuilder &PMBuilder);

  /// Register passes that run at the end of pipeline.
  void registerLastPasses(llvm::PassManagerBuilder &PMBuilder);

  /// Method shared by registerOptimizerLastCallback and registerLastPasses.
  void addLastPassesImpl(unsigned OptLevel, llvm::legacy::PassManagerBase &MPM);

  /// Add barrier related passes to pass manager.
  void addBarrierPasses(unsigned OptLevel, legacy::PassManagerBase &MPM);

  std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII;

  //  CodeGenOptions m_CodeGenOpts;
  llvm::legacy::FunctionPassManager FPM;
  llvm::legacy::PassManager MPM;
  llvm::legacy::PassManager MaterializerMPM;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
