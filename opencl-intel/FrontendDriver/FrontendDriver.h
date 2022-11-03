// INTEL CONFIDENTIAL
//
// Copyright 2009-2018 Intel Corporation.
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

#include "cl_config.h"
#include "clang_device_info.h"
#include <frontend_api.h>

namespace Intel {
namespace OpenCL {
namespace ClangFE {

class ClangFECompiler : public Intel::OpenCL::FECompilerAPI::IOCLFECompiler {
public:
  ClangFECompiler(const void *pszDeviceInfo);

  // IOCLFECompiler
  int CompileProgram(
      Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor *pProgDesc,
      IOCLFEBinaryResult **pBinaryResult) override;

  int LinkPrograms(
      Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc,
      IOCLFEBinaryResult **pBinaryResult) override;

  int ParseSPIRV(
      Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *pProgDesc,
      IOCLFEBinaryResult **pBinaryResult) override;

  int MaterializeSPIR(
      Intel::OpenCL::FECompilerAPI::FESPIRProgramDescriptor *pProgDesc,
      IOCLFEBinaryResult **pBinaryResult) override;

  int GetKernelArgInfo(
      const void *pBin, size_t uiBinarySize, const char *szKernelName,
      Intel::OpenCL::ClangFE::IOCLFEKernelArgInfo **pArgInfo) override;

  bool CheckCompileOptions(const char *szOptions, char *szUnrecognizedOptions,
                           size_t uiUnrecognizedOptionsSize) override;

  bool CheckLinkOptions(const char *szOptions, char *szUnrecognizedOptions,
                        size_t uiUnrecognizedOptionsSize) override;

  void GetSpecConstInfo(
      Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *pProgDesc,
      IOCLFESpecConstInfo **pSpecConstInfo) override;

  void Release() override { delete this; }

  static void ShutDown();

protected:
  virtual ~ClangFECompiler();
  CLANG_DEV_INFO m_sDeviceInfo;
  Intel::OpenCL::Utils::BasicCLConfigWrapper m_config;
};
} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
