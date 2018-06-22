// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE 
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS  
#ifndef __MODULE_JIT_HOLDER
#define __MODULE_JIT_HOLDER

#include "stddef.h"

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <malloc.h>
#include <assert.h>
#include <stdint.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendJITAllocator;
class IInputStream;
class IOutputStream;
class SerializationStatus;
class IDynamicFunctionsResolver;

typedef unsigned long long int KernelID;

struct LineNumberEntry {
    int offset;
    int line;
};

struct InlinedFunction {
    int id;
    int parentId;
    int from;
    unsigned size;
    std::string funcname;
    std::string filename;
};

struct RelocationInfo {
    std::string symName;
    unsigned int offset;
};

struct DynRelocationInfo {
  unsigned int offset;
  uint64_t addend;
};

typedef std::vector<LineNumberEntry> LineNumberTable;
typedef std::vector<InlinedFunction> InlinedFunctions;

struct KernelInfo {
    int functionId;
    int kernelOffset;
    int kernelSize;
    std::string filename;
    LineNumberTable lineNumberTable;
    InlinedFunctions inlinedFunctions;
};

/**
 * Container for chunks of allocated memory
 */
class MemoryHolder {
public:

  MemoryHolder() : m_size(0) {}

  ~MemoryHolder() {
    for (unsigned i = 0; i < m_chunks.size(); i++)
      m_chunks[i].release();
    m_chunks.clear();
  }

  void addChunk(char *ptr, uint64_t size) {
    m_chunks.push_back(ChunkHolder(ptr, size));
    m_size += size;
  }

  // return total JIT size held by this MemoryHolder
  uint64_t getSize() const {
    return m_size;
  }

  // the following 3 methods are used for serialization
  unsigned getNumChunks() const {
    return m_chunks.size();
  }

  char *getChunkPtr(unsigned i) const {
    assert (i < m_chunks.size() && "trying to access non-existing memory chunk");
    if (i >= m_chunks.size()) return NULL;
    return m_chunks[i].getPtr();
  }

  uint64_t getChunkSize(unsigned i) const {
    assert (i < m_chunks.size() && "trying to access non-existing memory chunk");
    if (i >= m_chunks.size()) return 0;
    return m_chunks[i].getSize();
  }

private:

  class ChunkHolder {
  public:
    ChunkHolder(char *ptr, uint64_t size) :
      m_allocPtr(ptr), m_size(size) {}

    // free memory owned by this chunk
    void release() {
      if (m_allocPtr != NULL) {
        free(m_allocPtr);
        m_allocPtr = NULL;
      }
    }

    char *getPtr() const {
      return m_allocPtr;
    }
    // get net size of JIT held by this chunk
    uint64_t getSize() const {
      return m_size;
    }

  private:

    ChunkHolder();

    char *m_allocPtr;               // pointer to allocated memory
    uint64_t m_size;      // size of use memory in this chunk
  };

private:
  std::vector<ChunkHolder> m_chunks;
  // total size used for JIT in all chunks
  uint64_t m_size;
};

/**
 * Represent JIT Code Holder for Module, which contains the main properties about
 * the jitted code, used in case the JIT is for the whole module at once
 */
class ModuleJITHolder
{
public:
    ModuleJITHolder();
    virtual ~ModuleJITHolder();

    /**
     * @return the whole JIT Buffer (Code\Data) size
     */
    virtual MemoryHolder &GetJITMemoryHolder();

    /**
     * @effects sets the JIT code buffer size
     */
    virtual void SetJITCodeSize(int jitSize);

    /**
     * @return the whole JIT Buffer (Code\Data)
     */
    virtual int GetJITCodeSize() const;

    /**
     * @effects sets the required alignment for the executable code
     */
    virtual void SetJITAlignment(size_t alignment);

    /**
     * @return the required alignment for the executable code
     */
    virtual size_t GetJITAlignment() const;

    /**
     * @return the JIT Buffer (Code/Data) start point for execution
     */
    virtual const void* GetJITCodeStartPoint() const;

    /**
     * @effects registers a new kernel, for each kernel need to specify
     *      it's id and info, the info should be related to the given JIT
     */
    virtual void RegisterKernel(KernelID kernelId, KernelInfo kernelinfo);

    /**
     * @effects registers a new symbol usage, for each usage need to specify
     *      it's offset and info, the info should be related to the given JIT
     */
    virtual void RegisterRelocation(const RelocationInfo& info);

    /**
     * @effects registers a relocation that has to be executed in the JIT
     *    buffer
     */
    virtual void RegisterDynRelocation(const DynRelocationInfo& info);

    /**
     * @param kernel identifier
     * @return the entry point of the specified function (relative to the start
     *    point of the JIT buffer); Exception will be raised if errors occurs
     */
    virtual int GetKernelEntryPoint(KernelID kernelId) const;

    /**
     * @param kernel identifier
     * @returns the size (in bytes) of the given kernel JIT code
     */
    virtual int GetKernelJITSize( KernelID kernelId ) const;

    /**
     * @param kernel identifier
     * @returns a table mapping code offset from kernel start to line number
     */
    virtual const LineNumberTable* GetKernelLineNumberTable(KernelID kernelId) const;

    /**
     * @param kernel identifier
     * @returns the name of the file in which the kernel was defined
     */
    virtual const char* GetKernelFilename(KernelID kernelId) const;

    /**
     * @param kernel identifier
     * @returns a collection with data for all the functions inlined in the kernel
     */
    virtual const InlinedFunctions* GetKernelInlinedFunctions(KernelID kernelId) const;

    /**
     * @param kernel identifier
     * @returns the function id used for vtune
     */
    virtual int GetKernelVtuneFunctionId(KernelID kernelID) const;

    /**
     * @returns the count of kernels in the JIT code
     */
    virtual int GetKernelCount() const;

    /**
     * @effects modify the jit to call the new external function addresses
     */
    virtual void RelocateSymbolAddresses(IDynamicFunctionsResolver* resolver);

    /**
     * @effects modify the jit according to the specified dynamic relocations
     */
    virtual void RelocateJITCode();

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats);
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);
private:
    // container for JIT in chunks as created by the compiler
    MemoryHolder m_memBuffer;
    // pointer to JIT on the device
    char*  m_pJITCode;

    size_t m_JITCodeSize;
    size_t m_alignment;
    std::map<KernelID, KernelInfo> m_KernelsMap;
    std::vector<RelocationInfo> m_RelocationTable;
    std::vector<DynRelocationInfo> m_DynRelocationTable;

    ICLDevBackendJITAllocator* m_pJITAllocator;

    // Klockwork Issue
    ModuleJITHolder ( const ModuleJITHolder& x );

    // Klockwork Issue
    ModuleJITHolder& operator= ( const ModuleJITHolder& x );

    void SerializeKernelInfo(KernelID id, KernelInfo info, IOutputStream& ost) const;
    void DeserializeKernelInfo(KernelID& id, KernelInfo& info, IInputStream& ist) const;

    void SerializeRelocationInfo(RelocationInfo info,
                             IOutputStream& ost) const;
    void DeserializeRelocationInfo(RelocationInfo& info,
                               IInputStream& ist) const;

    void SerializeDynRelocationInfo(DynRelocationInfo info,
                                 IOutputStream& ost) const;
    void DeserializeDynRelocationInfo(DynRelocationInfo& info,
                                     IInputStream& ist) const;

    void EncodeSymbolAddress(unsigned int offset, unsigned long long int address);
};

}}} // namespace

#endif // __MODULE_JIT_HOLDER
