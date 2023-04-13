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

#ifndef LLVM_OPENCL_INTEL_BACKEND_OPTIMIZER_OPIMIZEROCL_H
#define LLVM_OPENCL_INTEL_BACKEND_OPTIMIZER_OPIMIZEROCL_H

#include "Optimizer.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// Optimize LLVM IR using OCL new pass manager pipeline.
class OptimizerOCL : public Optimizer {
public:
  OptimizerOCL(llvm::Module &M,
               llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
               const intel::OptimizerConfig &Config);

  /// Run pass manager on a module.
  void Optimize(llvm::raw_ostream &LogStream) override;

private:
  void materializerPM(llvm::ModulePassManager &MPM) const;

  void createStandardLLVMPasses(llvm::ModulePassManager &MPM) const;

  void populatePassesPreFailCheck(llvm::ModulePassManager &MPM) const;

  void populatePassesPostFailCheck(llvm::ModulePassManager &MPM) const;

  /// Add barrier related passes to pass manager.
  void addBarrierPasses(llvm::ModulePassManager &MPM) const;

  llvm::OptimizationLevel Level;

  std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif
