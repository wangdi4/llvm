//==---- Compile.h --- OpenCL front-end compiler ------------------*- C++ -*---=
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-----------------------------------------------------------------------===

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