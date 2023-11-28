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

namespace llvm {
class Module;
}

namespace Intel {
namespace OpenCL {
namespace FECompilerAPI {
struct FESPIRProgramDescriptor;
}
namespace ClangFE {

struct IOCLFEBinaryResult;

// Updates SPIR 1.2 to current LLVM IR version.
class ClangFECompilerMaterializeSPIRTask {
public:
  ClangFECompilerMaterializeSPIRTask(
      Intel::OpenCL::FECompilerAPI::FESPIRProgramDescriptor *pProgDesc)
      : m_pProgDesc(pProgDesc) {}

  /// \brief Updates SPIR 1.2 to consumable by the compiler back-end LLVM IR.
  /// \return error code of clCompileProgram API
  int MaterializeSPIR(IOCLFEBinaryResult **pBinaryResult);

  /// \brief Adjusts the given module to be processed by the BE.
  ///
  /// More concretely:
  /// - replaces SPIR artifacts with Intel-implementation specific stuff.
  /// - updates LLVM IR to version supported by back-end compiler
  static int MaterializeSPIR(llvm::Module &M, bool isSpir12 = false);

private:
  Intel::OpenCL::FECompilerAPI::FESPIRProgramDescriptor *m_pProgDesc;
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
