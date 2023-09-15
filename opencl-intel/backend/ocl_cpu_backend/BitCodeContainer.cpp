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

#include "BitCodeContainer.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
BitCodeContainer::BitCodeContainer(const void *pBinary, size_t uiBinarySize,
                                   const char *name) {
  assert(pBinary && "Code container pointer must be valid");
  m_pBuffer = llvm::MemoryBuffer::getMemBufferCopy(
      llvm::StringRef((const char *)pBinary, uiBinarySize), name);
}

BitCodeContainer::BitCodeContainer(
    std::unique_ptr<llvm::MemoryBuffer> memBuffer)
    : m_pBuffer(std::move(memBuffer)) {}

BitCodeContainer::~BitCodeContainer() {
  llvm::Module *pModule = GetModule();
  if (pModule) {
    // [LLVM 3.6 UPGRADE] FIXME: see FIXME below on why it is commented out.
    // llvm::LLVMContext& context = pModule->getContext();

    // Unused metadata nodes are left alive during deletion of Module
    // Module owns Functions which are often used in metadata
    // during function destruction MDNodes referring to the function are
    // marked as non-unique and are placed to Nonuniqued nodes container in
    // LLVMContext LLVMContext will delete Nonuniqued only during its own
    // deletion at clReleaseContext As a result if we have multiple calls to
    // clBuildProgram on the same context e.g. in loop then number of unused
    // MDNodes grows and we have memory leak reported cleanup() was added to
    // LLVMContext and is called to find and free memory by unused Metadata
    // nodes

    // [LLVM 3.6 UPGRADE] FIXME: The patch that provides cleanup() functionality
    // was refined during upgrade and wasn't applied, in order to compile the
    // file the cleanup() call is commented out. context.cleanup();
  }
  // TODO: Check if the memory referred by m_pBuffer is to be revoked manually
  // here
}

const void *BitCodeContainer::GetCode() const {
  return m_pBuffer->getBufferStart();
}

size_t BitCodeContainer::GetCodeSize() const {
  return m_pBuffer->getBufferSize();
}

void BitCodeContainer::SetModule(std::unique_ptr<llvm::Module> M) {
  m_M = std::move(M);
}

void BitCodeContainer::SetModule(llvm::orc::ThreadSafeModule TSM) {
  // At this point, module ownership is already transferred to TSM
  m_M.reset(nullptr);

  m_TSM = std::move(TSM);
}

llvm::Module *BitCodeContainer::GetModule() const {
  return m_TSM ? const_cast<llvm::Module *>(m_TSM.getModuleUnlocked())
               : m_M.get();
}

std::unique_ptr<llvm::Module> BitCodeContainer::GetModuleOwner() {
  return std::move(m_M);
}

llvm::MemoryBuffer *BitCodeContainer::GetMemoryBuffer() const {
  return m_pBuffer.get();
}

void BitCodeContainer::Release() { delete this; }
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
