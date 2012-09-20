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

File Name:  StaticObjectLoader.h

\*****************************************************************************/
#pragma once


#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 *  Image Callback Library statically compiled object loader. Implements llvm::ObjectCache interface.
 */
class StaticObjectLoader : public llvm::ObjectCache {

  typedef llvm::DenseMap<const llvm::Module*, const llvm::MemoryBuffer* > ModuleMemBuffers;
  typedef llvm::DenseMap<const llvm::Module*, std::string> ModuleStringMap;

  // Map of modules to buffers containing object files which have been been
  // read from disk.
  ModuleMemBuffers StaticObjects;

  // Map of modules to filesystem paths where pre-compiled object files can be
  // loaded from, or written to on compilation.
  ModuleStringMap Paths;

  bool Exists(llvm::StringRef Location) {
    return llvm::sys::fs::exists(Location);
  }

  llvm::MemoryBuffer* ReadObject(llvm::StringRef Location) {
    llvm::OwningPtr<llvm::MemoryBuffer> MemBuf;
    llvm::MemoryBuffer::getFile(Location, MemBuf);
    return MemBuf.take();
  }

public:

  StaticObjectLoader()  { }

  virtual ~StaticObjectLoader() {
    // Delete all the in-memory static object copies owned by this class
    ModuleMemBuffers::iterator i = StaticObjects.begin();
    ModuleMemBuffers::iterator e = StaticObjects.end();
    for (; i != e; ++i) {
      delete i->second;
    }
  }

  /// AddLocation - Adds a mapping between a module and a path on the filesystem
  /// from where the object file for the given module should be loaded,
  /// The contents of the file at ObjectFilePath can be retrieved by calling
  /// GetObject() with the same Module pointer as used to call this function.
  virtual void AddLocation(const llvm::Module* M,
                           const std::string& ObjectFilePath) {
    Paths[M] = ObjectFilePath;
    if (Exists(ObjectFilePath)) {
      // A file exists at ObjectFilePath, so read it now and save it for later retrieval via GetObject
      llvm::OwningPtr<const llvm::MemoryBuffer> StaticObject(
        ReadObject(ObjectFilePath));
      StaticObjects.insert(std::make_pair(M, StaticObject.take()));
    }
  }

  /// AddPreCompiled - Adds a mapping between a module and a preloaded object
  /// file.
  virtual void AddPreCompiled(const llvm::Module* M,
                              const llvm::MemoryBuffer* MemBuff) {
    StaticObjects.insert(std::make_pair(M, MemBuff));
  }

  virtual void NotifyObjectCompiled(const llvm::Module*, const llvm::MemoryBuffer*) {
    // A module has been compiled and the resulting object is in a MemoryBuffer
    assert(0 && "TODO: implement module compiled notification handler");
  }

  /// GetObject - Returns a pointer to a pre-compiled object buffer previously
  /// added to the cache (with AddLocation or NotifyCompiledObject) or 0 if the
  /// Module pointer M is not associated with a statically compiled object.
  virtual const llvm::MemoryBuffer* GetObject(const llvm::Module* M) {
    ModuleMemBuffers::iterator i = StaticObjects.find(M);
    if (i == StaticObjects.end()) {
      return 0;
    } else {
      return i->second;
    }
  }
};

}}}
