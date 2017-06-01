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

File Name:  ObjectCodeCache.cpp

\*****************************************************************************/

#include "ObjectCodeCache.h"
#include "BitCodeContainer.h"
#include "ObjectCodeContainer.h"
#include "Compiler.h"

#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"

#include "llvm/Bitcode/BitcodeWriter.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ObjectCodeCache::ObjectCodeCache(llvm::Module* pModule, const char* pObject, size_t ObjectSize):
  m_CachedModuleBuffer(""),
  m_pObjectBuffer(nullptr) {

  if(pObject && pModule) {
    llvm::raw_string_ostream stream(m_CachedModuleBuffer);
    llvm::WriteBitcodeToFile(pModule, stream);
    stream.flush();
    
    llvm::StringRef data = llvm::StringRef((const char*)pObject, ObjectSize);
    m_pObjectBuffer.reset(llvm::MemoryBuffer::getMemBufferCopy(data).release());
  }
}

ObjectCodeCache::~ObjectCodeCache() {
}

void ObjectCodeCache::notifyObjectCompiled(const llvm::Module* pModule,
   llvm::MemoryBufferRef pBuffer) {

  // A module has been compiled and the resulting object is in a MemoryBuffer
  m_CachedModuleBuffer = std::string();
  llvm::raw_string_ostream stream(m_CachedModuleBuffer);
  llvm::WriteBitcodeToFile(pModule, stream);
  stream.flush();
  
  m_pObjectBuffer.reset(llvm::MemoryBuffer::getMemBufferCopy(pBuffer.getBuffer()).release());
}

std::unique_ptr<llvm::MemoryBuffer> ObjectCodeCache::getObject(const llvm::Module* pModule) {
  std::string moduleBuffer = "";
  llvm::raw_string_ostream stream(moduleBuffer);
  llvm::WriteBitcodeToFile(pModule, stream);
  stream.flush();

  if(moduleBuffer == m_CachedModuleBuffer) {
    assert(m_pObjectBuffer.get() && "Mapped Object is null");
    return std::unique_ptr<llvm::MemoryBuffer>(std::move(m_pObjectBuffer));
  }

  return NULL;
}

const std::string& ObjectCodeCache::getCachedModule() {
  return m_CachedModuleBuffer;
}

const llvm::MemoryBuffer* ObjectCodeCache::getCachedObject() {
  return m_pObjectBuffer.get();
}

}}}
