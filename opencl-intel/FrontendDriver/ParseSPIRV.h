//==---- ParseSPIRV.h --- OpenCL front-end compiler ---------------*- C++ -*---=
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

#include <cstdint> // std::uint32_t
#include <string>

namespace Intel {
namespace OpenCL {
namespace Utils {
class BasicCLConfigWrapper;
}
namespace ClangFE {
struct CLANG_DEV_INFO;
}
namespace ClangFE {

// SPIR-V -> llvm::Module converter wrapper.
class ClangFECompilerParseSPIRVTask {
public:
  ClangFECompilerParseSPIRVTask(
      Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *pProgDesc,
      Intel::OpenCL::ClangFE::CLANG_DEV_INFO const &sDeviceInfo);
  /// \brief Translate SPIR-V into LLVM-IR
  /// \return error code of clCompileProgram API
  int ParseSPIRV(IOCLFEBinaryResult **pBinaryResult);

private:
  /// \brief Read 32bit integer value and convert it to little-endian if
  /// necessary
  std::uint32_t getSPIRVWord(std::uint32_t const *wordPtr) const;
  /// \brief Check a SPIR-V module's version, capabilities, and memory model are
  /// supported
  /// \param error - contains explanation why the module is not supported.
  bool isSPIRVSupported() const;

  Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *m_pProgDesc;
  const Intel::OpenCL::ClangFE::CLANG_DEV_INFO& m_sDeviceInfo;
  bool m_littleEndian; ///< True if SPIR-V module byte order is little-endian
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel