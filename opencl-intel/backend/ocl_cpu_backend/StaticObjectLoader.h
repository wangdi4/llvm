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
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 *  Image Callback Library statically compiled object loader. Implements
 * llvm::ObjectCache interface.
 */
class StaticObjectLoader : public llvm::ObjectCache {

  typedef llvm::DenseMap<const llvm::Module *,
                         std::unique_ptr<llvm::MemoryBuffer>>
      ModuleMemBuffers;
  typedef llvm::DenseMap<const llvm::Module *, std::string> ModuleStringMap;

  // Map of modules to buffers containing object files which have been been
  // read from disk.
  ModuleMemBuffers StaticObjects;

  // Map of modules to filesystem paths where pre-compiled object files can be
  // loaded from, or written to on compilation.
  ModuleStringMap Paths;

  bool exists(llvm::StringRef Location) {
    return llvm::sys::fs::exists(Location);
  }

  llvm::MemoryBuffer *readObject(llvm::StringRef Location) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> memBufOrErr =
        llvm::MemoryBuffer::getFile(Location);
    return memBufOrErr.get().release();
  }

public:
  StaticObjectLoader() {}

  virtual ~StaticObjectLoader() {}

  /// addLocation - Adds a mapping between a module and a path on the filesystem
  /// from where the object file for the given module should be loaded,
  /// The contents of the file at ObjectFilePath can be retrieved by calling
  /// getObject() with the same Module pointer as used to call this function.
  virtual void addLocation(const llvm::Module *M,
                           const std::string &ObjectFilePath) {
    Paths[M] = ObjectFilePath;
    if (exists(ObjectFilePath)) {
      // A file exists at ObjectFilePath, so read it now and save it for later
      // retrieval via getObject
      std::unique_ptr<llvm::MemoryBuffer> StaticObject(
          readObject(ObjectFilePath));
      StaticObjects.insert(std::make_pair(M, std::move(StaticObject)));
    }
  }

  /// addPreCompiled - Adds a mapping between a module and a preloaded object
  /// file.
  virtual void addPreCompiled(const llvm::Module *M,
                              const llvm::MemoryBuffer *MemBuff) {
    StaticObjects.insert(
        std::make_pair(M, (std::unique_ptr<llvm::MemoryBuffer>(
                              const_cast<llvm::MemoryBuffer *>(MemBuff)))));
  }

  virtual void notifyObjectCompiled(const llvm::Module *,
                                    llvm::MemoryBufferRef) override {
    // A module has been compiled and the resulting object is in a MemoryBuffer
    assert(0 && "TODO: implement module compiled notification handler");
  }

  /// getObject - Returns a pointer to a pre-compiled object buffer previously
  /// added to the cache (with addLocation or notifyCompiledObject) or 0 if the
  /// Module pointer M is not associated with a statically compiled object.
  virtual std::unique_ptr<llvm::MemoryBuffer>
  getObject(const llvm::Module *M) override {
    ModuleMemBuffers::iterator i = StaticObjects.find(M);
    if (i == StaticObjects.end()) {
      return 0;
    } else {
      return std::move(i->second);
    }
  }
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
