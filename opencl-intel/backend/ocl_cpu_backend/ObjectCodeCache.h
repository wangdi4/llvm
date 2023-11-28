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

#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 *  Manages calls for saving and loading the Objects for saving compilation time
 *  Implements llvm::ObjectCache interface.
 */
class ObjectCodeCache : public llvm::ObjectCache {
private:
  // Pointers on the represented program
  std::unique_ptr<llvm::MemoryBuffer> m_pObjectBuffer;
  bool m_isObjectAvailable;

public:
  ObjectCodeCache() : m_isObjectAvailable(false) {}

  ObjectCodeCache(llvm::Module *pModule, const char *pObject,
                  size_t ObjectSize);

  virtual ~ObjectCodeCache();

  ObjectCodeCache(const ObjectCodeCache &) = delete;
  ObjectCodeCache &operator=(const ObjectCodeCache &) = delete;

  /// notifyObjectCompiled - will be called once the codegen generates an object
  virtual void notifyObjectCompiled(const llvm::Module *,
                                    llvm::MemoryBufferRef) override;

  /// getObject - Returns a pointer to a pre-compiled object buffer previously
  /// added to the cache or 0 if the object not found
  virtual std::unique_ptr<llvm::MemoryBuffer>
  getObject(const llvm::Module *M) override;

  virtual bool isObjectAvailable() const { return m_isObjectAvailable; }
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
