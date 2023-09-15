// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "cl_dev_backend_api.h"
#include "cl_types.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/Module.h"
#include <memory>

// forward decl
namespace llvm {
class MemoryBuffer;
}

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class ProgramContainerMemoryBuffer;

/**
 * Represents the container for LLVM serialized bitcode
 *
 * Also get ownership on LLVM materialized module
 */
class BitCodeContainer : public ICLDevBackendCodeContainer {
public:
  BitCodeContainer(const void *pBinary, size_t uiBinarySize,
                   const char *name = "");
  BitCodeContainer(std::unique_ptr<llvm::MemoryBuffer> memBuffer);
  ~BitCodeContainer();

  BitCodeContainer(const BitCodeContainer &) = delete;
  BitCodeContainer &operator=(const BitCodeContainer &) = delete;

  const void *GetCode() const override;

  size_t GetCodeSize() const override;

  /**
   * Get ownership on passed module
   */
  void SetModule(std::unique_ptr<llvm::Module> M);

  /**
   * Get ownership on passed ThreadSafeModule
   */
  void SetModule(llvm::orc::ThreadSafeModule TSM);

  /**
   * Return the LLVM Module pointer
   */
  llvm::Module *GetModule() const;

  /**
   * Return the LLVM Module with ownership
   */
  std::unique_ptr<llvm::Module> GetModuleOwner();

  /**
   * Returns the serialized bitcode buffer as a plain pointer (convert to LLVM
   * MemoryBuffer)
   */
  llvm::MemoryBuffer *GetMemoryBuffer() const;

  /**
   * Releases the Bit Code Container
   */
  void Release();

private:
  // Module is owned by either m_M or m_TSM
  std::unique_ptr<llvm::Module> m_M;
  llvm::orc::ThreadSafeModule m_TSM;

  std::unique_ptr<llvm::MemoryBuffer> m_pBuffer;
};
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
