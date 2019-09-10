// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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
#ifdef _WIN32
#include "CPUCompiler.h"
#include "LLDJIT.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class LLDJITBuilder {
public:
  /// @brief Creates an LLDJIT instance, that owns the passed module.
  static llvm::ExecutionEngine *CreateExecutionEngine(llvm::Module *M,
                                                      llvm::TargetMachine *TM);

  /// @brief Makes the module linkable.
  /// This includes adding DllMain() and exporting kernel symbols, etc.
  static void prepareModuleForLLD(llvm::Module *M);

private:
  static void convertToMSVCModule(llvm::Module *M);
  static void exportKernelSymbols(llvm::Module *M);
  static void addDllMainFunction(llvm::Module *M);
  static void adjustFunctionAttributes(llvm::Module *M);
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
#endif
