/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ObjectCodeCache.h

\*****************************************************************************/
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

  ObjectCodeCache( const ObjectCodeCache& rhs );
  ObjectCodeCache& operator=( const ObjectCodeCache& rhs );

public:
  ObjectCodeCache(): 
    m_CachedModuleBuffer("")
    { }

  ObjectCodeCache(llvm::Module* pModule, const char* pObject, size_t ObjectSize);

  virtual ~ObjectCodeCache();

  /// notifyObjectCompiled - will be called once the codegen genrates an object
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
