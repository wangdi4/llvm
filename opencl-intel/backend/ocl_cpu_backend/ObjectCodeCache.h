// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include <string>
#include <memory>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 *  Manages calls for saving and loading the Objects for saving compilation time
 *  Implements llvm::ObjectCache interface.
 */
class ObjectCodeCache : public llvm::ObjectCache {
private:
  // Pointers on the represented program
  std::string m_CachedModuleBuffer;
  std::unique_ptr<llvm::MemoryBuffer> m_pObjectBuffer;
  bool m_isObjectAvailable;

  ObjectCodeCache( const ObjectCodeCache& rhs );
  ObjectCodeCache& operator=( const ObjectCodeCache& rhs );

public:
  ObjectCodeCache(): 
    m_CachedModuleBuffer(""),
    m_isObjectAvailable(false)
    { }

  ObjectCodeCache(llvm::Module* pModule, const char* pObject, size_t ObjectSize);

  virtual ~ObjectCodeCache();

  /// notifyObjectCompiled - will be called once the codegen generates an object
  virtual void notifyObjectCompiled(const llvm::Module*, llvm::MemoryBufferRef);

  /// getObject - Returns a pointer to a pre-compiled object buffer previously
  /// added to the cache or 0 if the object not found
  virtual std::unique_ptr<llvm::MemoryBuffer> getObject(const llvm::Module* M);

  /// return pointer to the cached module raw data
  virtual const std::string& getCachedModule();

  /// return pointer to the object buffer
  virtual const llvm::MemoryBuffer* getCachedObject();
};

}}}
