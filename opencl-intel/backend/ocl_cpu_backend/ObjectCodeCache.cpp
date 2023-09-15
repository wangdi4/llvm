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

#include "ObjectCodeCache.h"
#include "BitCodeContainer.h"
#include "Compiler.h"
#include "ObjectCodeContainer.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

ObjectCodeCache::ObjectCodeCache(llvm::Module *pModule, const char *pObject,
                                 size_t ObjectSize)
    : m_pObjectBuffer(nullptr), m_isObjectAvailable(false) {

  if (pObject && pModule) {
    llvm::StringRef data = llvm::StringRef((const char *)pObject, ObjectSize);
    m_pObjectBuffer.reset(llvm::MemoryBuffer::getMemBufferCopy(data).release());

    m_isObjectAvailable = true;
  }
}

ObjectCodeCache::~ObjectCodeCache() {}

void ObjectCodeCache::notifyObjectCompiled(const llvm::Module * /*pModule*/,
                                           llvm::MemoryBufferRef pBuffer) {

  assert(!m_isObjectAvailable &&
         "We do not expect a second Module to save its object");

  // A module has been compiled and the resulting object is in a MemoryBuffer

  m_pObjectBuffer.reset(
      llvm::MemoryBuffer::getMemBufferCopy(pBuffer.getBuffer()).release());

  m_isObjectAvailable = true;
}

std::unique_ptr<llvm::MemoryBuffer>
ObjectCodeCache::getObject(const llvm::Module * /*pModule*/) {
  if (m_isObjectAvailable) {
    assert(m_pObjectBuffer.get() && "Mapped Object is null");
    return std::unique_ptr<llvm::MemoryBuffer>(std::move(m_pObjectBuffer));
  }

  return nullptr;
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
