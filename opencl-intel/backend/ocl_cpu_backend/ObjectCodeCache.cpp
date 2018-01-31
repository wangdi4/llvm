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
#include "MetadataAPI.h"

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/MemoryBuffer.h"

extern "C" {
  llvm::ModulePass *createStripIntelIPPass();
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

static void StripIntelIP(llvm::Module *M) {
  llvm::legacy::PassManager PM;

  PM.add(createStripIntelIPPass());
#ifndef NDEBUG
  PM.add(llvm::createVerifierPass());
#endif
  PM.run(*M);
}

ObjectCodeCache::ObjectCodeCache(llvm::Module* pModule, const char* pObject, size_t ObjectSize):
  m_CachedModuleBuffer(""),
  m_pObjectBuffer(nullptr),
  m_isObjectAvailable(false) {

  if(pObject && pModule) {
    llvm::raw_string_ostream stream(m_CachedModuleBuffer);
    llvm::WriteBitcodeToFile(pModule, stream);
    stream.flush();

    llvm::StringRef data = llvm::StringRef((const char*)pObject, ObjectSize);
    m_pObjectBuffer.reset(llvm::MemoryBuffer::getMemBufferCopy(data).release());

    m_isObjectAvailable = true;
  }
}

ObjectCodeCache::~ObjectCodeCache() {
}

void ObjectCodeCache::notifyObjectCompiled(const llvm::Module* pModule,
   llvm::MemoryBufferRef pBuffer) {

  assert(!m_isObjectAvailable && "We do not expect a second Module to save its object");

  // A module has been compiled and the resulting object is in a MemoryBuffer
  m_CachedModuleBuffer = std::string();
  llvm::raw_string_ostream stream(m_CachedModuleBuffer);
  // Strip optimized Module of IP-sensitive content before saving.
  // This callback is the safe spot when we know the optimized Module
  // has been JITted and is safe to tamper with before serialization.
  StripIntelIP(const_cast<llvm::Module*>(pModule));
  llvm::WriteBitcodeToFile(pModule, stream);
  stream.flush();

  m_pObjectBuffer.reset(llvm::MemoryBuffer::getMemBufferCopy(pBuffer.getBuffer()).release());

  m_isObjectAvailable = true;
}

std::unique_ptr<llvm::MemoryBuffer> ObjectCodeCache::getObject(const llvm::Module* pModule) {
  if (m_isObjectAvailable) {
    assert(m_pObjectBuffer.get() && "Mapped Object is null");
    return std::unique_ptr<llvm::MemoryBuffer>(std::move(m_pObjectBuffer));
  }

  return nullptr;
}

const std::string& ObjectCodeCache::getCachedModule() {
  return m_CachedModuleBuffer;
}

const llvm::MemoryBuffer* ObjectCodeCache::getCachedObject() {
  return m_pObjectBuffer.get();
}

}}}
