// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
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
#include "debuggingservicetype.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

#include <vector>

enum class DebugLogging { None, Normal, Quiet, Verbose };

namespace intel {
class OptimizerConfig;
}

namespace llvm {
class Pass;
class Module;
class Function;
class ModulePass;
class LLVMContext;
} // namespace llvm

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class Optimizer {
public:
  Optimizer(llvm::Module &M, llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
            const intel::OptimizerConfig &Config);

  virtual ~Optimizer() {}

  enum InvalidFunctionType {
    RECURSION,
    RECURSION_WITH_BARRIER,
    FPGA_PIPE_DYNAMIC_ACCESS,
    VECTOR_VARIANT_FAILURE
  };

  enum InvalidGVType { FPGA_DEPTH_IS_IGNORED };

  virtual void Optimize(llvm::raw_ostream &LogStream) = 0;

  /// @brief recursion was detected after standard LLVM optimizations
  /// @return for SYCL returns true if the recursive function also calls
  /// barrier; for OpenCL returns true if any recursive function is present.
  bool hasUnsupportedRecursion();

  /// @brief checks if some pipes access were not resolved statically
  bool hasFpgaPipeDynamicAccess() const;

  /// @brief checks if there are any issues with vector-varian attributes.
  bool hasVectorVariantFailure() const;

  /// @brief checks if there are some channels whose depths are differs from
  /// real depth on FPGA hardware due to channel depth mode, so we should emit
  /// diagnostic message
  bool hasFPGAChannelsWithDepthIgnored() const;

  /// @brief obtain functions names wich are not valid for OpenCL
  /// @param Ty is a type of invalid function
  /// @return std::vector with function names
  std::vector<std::string> GetInvalidFunctions(InvalidFunctionType Ty) const;

  /// @brief obtain global variable names wich are not valid due to some
  /// limitations
  /// @param Ty is a type of global variables to search
  /// @return std::vector with global variable names
  std::vector<std::string> GetInvalidGlobals(InvalidGVType Ty) const;

  static llvm::ArrayRef<llvm::VectItem> getVectInfos();

  static const StringSet<> &getVPlanMaskedFuncs();

protected:
  /// Register OCLDiagnosticHandler callback to LLVMContext.
  void setDiagnosticHandler(llvm::raw_ostream &LogStream);

  llvm::Module &m_M;
  /// Builtin rtl modules (not owned by this class).
  llvm::SmallVector<llvm::Module *, 2> m_RtlModules;

  llvm::VFISAKind ISA;

  const intel::OptimizerConfig &Config;

  StringRef CPUPrefix;

  /// True if OpenCL version is greater or equal to 2.0.
  bool m_IsOcl20;

  /// True if source is spirv.
  bool m_IsSPIRV;

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

  intel::DebuggingServiceType m_debugType;
  bool m_UseTLSGlobals;

  bool UnrollLoops;
};

/**
 *  Responsible for running the IR=>IR optimization passes on given program
    using legacy OCL pass manager.
 */
class OptimizerOCLLegacy : public Optimizer {
public:
  OptimizerOCLLegacy(llvm::Module &pModule,
                     llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
                     const intel::OptimizerConfig &pConfig);

  void Optimize(llvm::raw_ostream &LogStream) override;

  /// @brief register OpenCL passes to LLVM PassRegistry
  static void initializePasses();

private:
  // hold the collection of passes
  llvm::legacy::PassManager m_PM;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
