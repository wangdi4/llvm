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
    llvm::WriteBitcodeToFile(*pModule, stream);
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
  llvm::WriteBitcodeToFile(*pModule, stream);
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
