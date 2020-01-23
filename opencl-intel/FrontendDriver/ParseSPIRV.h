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

  /// \brief Check that stream of bytes given in \a pBinary begins with SPIR-V
  /// magic number, i.e 0x07230203
  static bool isSPIRV(const void* pBinary, const size_t BinarySize);

private:
  /// \brief Read 32bit integer value and convert it to little-endian if
  /// necessary
  std::uint32_t getSPIRVWord(std::uint32_t const *wordPtr) const;
  /// \brief Check a SPIR-V module's version, capabilities, and memory model are
  /// supported
  /// \param error - contains explanation why the module is not supported.
  bool isSPIRVSupported(std::string &error) const;

  Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor *m_pProgDesc;
  const Intel::OpenCL::ClangFE::CLANG_DEV_INFO& m_sDeviceInfo;
  bool m_littleEndian; ///< True if SPIR-V module byte order is little-endian
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
