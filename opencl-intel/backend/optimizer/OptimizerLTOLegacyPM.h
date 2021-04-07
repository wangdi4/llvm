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

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 *  Run llvm legacy pass manager on the given module.
 */
class OptimizerLTOLegacyPM : public Optimizer {
public:
  OptimizerLTOLegacyPM(llvm::Module *M, const intel::OptimizerConfig *Config);

  ~OptimizerLTOLegacyPM();

  /// Run pass manager on a module.
  void Optimize() override;

private:
  /// Add passes to pass managers.
  void CreatePasses();

  const intel::OptimizerConfig *Config;

  std::unique_ptr<llvm::TargetLibraryInfoImpl> TLII;

  //  CodeGenOptions m_CodeGenOpts;
  llvm::legacy::FunctionPassManager FPM;
  llvm::legacy::PassManager MPM;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
