// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "frontend_api.h"
#include "LLVMSPIRVOpts.h"

namespace llvm {
class Module;
}

namespace Intel {
namespace OpenCL {
namespace ClangFE {

class ClangFECompilerMaterializeSPIRVTask {
public:
  ClangFECompilerMaterializeSPIRVTask(
      Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *pProgDesc,
      SPIRV::TranslatorOpts opts)
      : m_pProgDesc(pProgDesc), m_opts(opts) {}

  /// \brief Correct the given module to be processed by the BE.
  ///
  /// More concretely:
  /// - updates LLVM IR for some possible improvements
  bool MaterializeSPIRV(llvm::Module *&pM);

private:
  Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *m_pProgDesc;
  SPIRV::TranslatorOpts m_opts;
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
