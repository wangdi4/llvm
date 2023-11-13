// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "Compiler.h"
#include "OptimizerConfig.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

enum class DebugLogging { None, Normal, Quiet, Verbose };

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class Optimizer {
public:
  Optimizer(llvm::Module &M, llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
            const intel::OptimizerConfig &Config);

  virtual ~Optimizer() {}

  virtual void Optimize(llvm::raw_ostream &LogStream) = 0;

  static llvm::ArrayRef<llvm::VectItem> getVectInfos();

  /// Initialize cl::opt options in Optimizer.
  static void initOptimizerOptions();
  static DebugLogging getDebugPM();
  static bool getDisableVPlanCM(); // INTEL
  static bool getVerifyEachPass();

  static const llvm::StringSet<> &getVPlanMaskedFuncs(); // INTEL

  void setExceptionMsg(std::string &Msg) { ExceptionMsg = Msg; }
  const std::string &getExceptionMsg() const { return ExceptionMsg; }

protected:
  /// Register OCLDiagnosticHandler callback to LLVMContext.
  void setDiagnosticHandler(llvm::raw_ostream &LogStream);

  llvm::Module &m_M;
  /// Builtin rtl modules (not owned by this class).
  llvm::SmallVector<llvm::Module *, 2> m_RtlModules;

  llvm::VFISAKind ISA;

  const intel::OptimizerConfig &Config;

  llvm::StringRef CPUPrefix;

  /// True if OpenCL version is greater or equal to 2.0.
  bool m_HasOcl20;

  // Indicates whether the module comes from SYCL.
  // The only noticeable difference between SYCL flow and OpenCL flow is the
  // spirv.Source metadata: in SYCL the value for spirv.Source is OpenCL C++
  // (because SYCL does not have a dedicated enum value yet), while in OpenCL
  // spirv.Source is OpenCL C.
  //
  // spirv.Source is an *optional* metadata and can be omitted (optimized)
  // during SPIR-V translation. It also is not emitted if we do not use SPIR-V
  // as an intermediate. These two cases are not supported now.
  bool m_IsSYCL;

  /// Indicates whether the module comes from OpenMP.
  bool m_IsOMP;

  bool m_IsFpgaEmulator;

  bool m_UseTLSGlobals;

  /// Exception message in case of error diagnose emitted from a pass.
  std::string ExceptionMsg;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
