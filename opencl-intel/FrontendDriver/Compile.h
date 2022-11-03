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

namespace Intel {
namespace OpenCL {
namespace Utils {
class BasicCLConfigWrapper;
}
namespace ClangFE {
struct CLANG_DEV_INFO;
}
namespace ClangFE {

class ClangFECompilerCompileTask {
public:
  ClangFECompilerCompileTask(
      Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor *pProgDesc,
      const Intel::OpenCL::ClangFE::CLANG_DEV_INFO &sDeviceInfo,
      const Intel::OpenCL::Utils::BasicCLConfigWrapper &config)
      : m_pProgDesc(pProgDesc), m_sDeviceInfo(sDeviceInfo), m_config(config) {}

  ClangFECompilerCompileTask(const ClangFECompilerCompileTask &) = delete;
  ClangFECompilerCompileTask &
  operator=(ClangFECompilerCompileTask const &) = delete;
  ~ClangFECompilerCompileTask() {}

  int Compile(IOCLFEBinaryResult **pBinaryResult);

private:
  Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor *m_pProgDesc;
  const Intel::OpenCL::ClangFE::CLANG_DEV_INFO &m_sDeviceInfo;
  const Intel::OpenCL::Utils::BasicCLConfigWrapper &m_config;
};

// ClangFECompilerCheckCompileOptions
// Input: szOptions - a string representing the compile options
// Output: szUnrecognizedOptions - a new string containing the unrecognized
// options separated by spaces Returns: 'true' if the compile options are
// legal and 'false' otherwise
bool ClangFECompilerCheckCompileOptions(
    const char *szOptions, char *szUnrecognizedOptions,
    size_t uiUnrecognizedOptionsSize,
    const Intel::OpenCL::Utils::BasicCLConfigWrapper &config);

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
