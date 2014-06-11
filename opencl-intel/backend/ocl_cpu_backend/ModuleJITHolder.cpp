/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ModuleJITHolder.cpp

\*****************************************************************************/

#include "ModuleJITHolder.h"

#include "MICSerializationService.h"
#include "Serializer.h"

#include "malloc.h"
#include "stddef.h"

#include <cassert>
#include <map>
#include <utility>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class IInputStream;
class IOutputStream;

ModuleJITHolder::ModuleJITHolder():
    m_pJITCode(NULL),
    m_JITCodeSize(0),
    m_alignment(0),
    m_pJITAllocator(NULL)
{}

ModuleJITHolder::~ModuleJITHolder()
{
    if(NULL != m_pJITAllocator && NULL != m_pJITCode)
    {
        m_pJITAllocator->FreeExecutable(m_pJITCode);
    }
}

MemoryHolder &ModuleJITHolder::GetJITMemoryHolder()
{
    return m_memBuffer;
}


void ModuleJITHolder::SetJITCodeSize(int jitSize)
{
    m_JITCodeSize = jitSize;
}

int ModuleJITHolder::GetJITCodeSize() const
{
    return m_JITCodeSize;
}

void ModuleJITHolder::SetJITAlignment(size_t alignment)
{
    m_alignment = alignment;
}

size_t ModuleJITHolder::GetJITAlignment() const
{
    return m_alignment;
}

const void* ModuleJITHolder::GetJITCodeStartPoint() const
{
    return m_pJITCode;
}

void ModuleJITHolder::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    // using unsigned long long int instead of size_t is because that size_t
    // varies in it's size relating to the platform (32/64 bit)
    unsigned long long int tmp = (unsigned long long int)m_memBuffer.getSize();
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);

    tmp = (unsigned long long int)m_alignment;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);

    int mapSize = m_KernelsMap.size();
    Serializer::SerialPrimitive<int>(&mapSize, ost);
    for(std::map<KernelID, KernelInfo>::const_iterator 
        it = m_KernelsMap.begin();
        it != m_KernelsMap.end();
        it++)
    {
        SerializeKernelInfo(it->first, it->second, ost);
    }

    for (unsigned chunkIdx = 0; chunkIdx < m_memBuffer.getNumChunks();
        chunkIdx++)
    {
      char *ptr = m_memBuffer.getChunkPtr(chunkIdx);
      unsigned long long size = m_memBuffer.getChunkSize(chunkIdx);
      for(size_t i = 0; i < size; i++)
      {
          Serializer::SerialPrimitive<char>(&(ptr[i]), ost);
      }
    }
}

void ModuleJITHolder::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    unsigned long long int tmp = 0;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_JITCodeSize = tmp;

    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_alignment = tmp;

    int mapSize = 0;
    Serializer::DeserialPrimitive<int>(&mapSize, ist);

    m_KernelsMap.clear();
    for(int i = 0; i < mapSize; ++i)
    {
        KernelID kernelID;
        KernelInfo kernelInfo;
        DeserializeKernelInfo(kernelID, kernelInfo, ist);
        m_KernelsMap[kernelID] = kernelInfo;
    }

    // Deserialize the JIT code itself
    ICLDevBackendJITAllocator* pAllocator = stats->GetJITAllocator();
    if(NULL == pAllocator) throw Exceptions::SerializationException("Cannot Get JIT Allocator");

    if(NULL != m_pJITCode)
    {
        m_pJITAllocator->FreeExecutable(m_pJITCode); // free by the old allocator
        m_pJITCode = NULL;
    }

    m_pJITAllocator = pAllocator;
    m_pJITCode = (char*)(m_pJITAllocator->AllocateExecutable(sizeof(char) * m_JITCodeSize, m_alignment));
    if(NULL == m_pJITCode) throw Exceptions::SerializationException("JIT Allocator Failed Allocating Memory");

    for(size_t i = 0; i < m_JITCodeSize; i++)
    {
        Serializer::DeserialPrimitive<char>(&(m_pJITCode[i]), ist);
    }
}

void ModuleJITHolder::SerializeKernelInfo(KernelID id, KernelInfo info,
                                          IOutputStream& ost) const
{
    Serializer::SerialPrimitive<unsigned long long int>(&id, ost);

    Serializer::SerialPrimitive<int>(&(info.functionId), ost);

    Serializer::SerialPrimitive<int>(&(info.kernelOffset), ost);

    Serializer::SerialPrimitive<int>(&(info.kernelSize), ost);

    Serializer::SerialString(info.filename, ost);

    int lineNumberTableSize = info.lineNumberTable.size();
    Serializer::SerialPrimitive<int>(&lineNumberTableSize, ost);
    for (int i = 0; i < lineNumberTableSize; i++)
    {
        LineNumberEntry entry = info.lineNumberTable[i];
        Serializer::SerialPrimitive<int>(&(entry.offset), ost);
        Serializer::SerialPrimitive<int>(&(entry.line), ost);
    }

    int inlinedFunctionsSize = info.inlinedFunctions.size();
    Serializer::SerialPrimitive<int>(&inlinedFunctionsSize, ost);
    for (int i = 0; i < inlinedFunctionsSize; i++)
    {
        InlinedFunction inlinedFunc = info.inlinedFunctions[i];
        Serializer::SerialPrimitive<int>(&(inlinedFunc.id), ost);
        Serializer::SerialPrimitive<int>(&(inlinedFunc.parentId), ost);
        Serializer::SerialPrimitive<int>(&(inlinedFunc.from), ost);
        Serializer::SerialPrimitive<unsigned>(&(inlinedFunc.size), ost);
        Serializer::SerialString(inlinedFunc.funcname, ost);
        Serializer::SerialString(inlinedFunc.filename, ost);
    }
}

void ModuleJITHolder::DeserializeKernelInfo(KernelID& id, KernelInfo& info,
                                            IInputStream& ist) const {
    Serializer::DeserialPrimitive<unsigned long long int>(&id, ist);

    Serializer::DeserialPrimitive<int>(&(info.functionId), ist);

    Serializer::DeserialPrimitive<int>(&(info.kernelOffset), ist);

    Serializer::DeserialPrimitive<int>(&(info.kernelSize), ist);

    Serializer::DeserialString(info.filename, ist);

    int lineNumberTableSize;
    Serializer::DeserialPrimitive<int>(&lineNumberTableSize, ist);
    for (int j = 0; j < lineNumberTableSize; j++) {
        LineNumberEntry entry;
        Serializer::DeserialPrimitive<int>(&(entry.offset), ist);
        Serializer::DeserialPrimitive<int>(&(entry.line), ist);
        info.lineNumberTable.push_back(entry);
    }

    int inlinedFunctionsSize;
    Serializer::DeserialPrimitive<int>(&inlinedFunctionsSize, ist);
    for (int j = 0; j < inlinedFunctionsSize; j++) {
        InlinedFunction inlinedFunc;
        Serializer::DeserialPrimitive<int>(&(inlinedFunc.id), ist);
        Serializer::DeserialPrimitive<int>(&(inlinedFunc.parentId), ist);
        Serializer::DeserialPrimitive<int>(&(inlinedFunc.from), ist);
        Serializer::DeserialPrimitive<unsigned>(&(inlinedFunc.size), ist);
        Serializer::DeserialString(inlinedFunc.funcname, ist);
        Serializer::DeserialString(inlinedFunc.filename, ist);
        info.inlinedFunctions.push_back(inlinedFunc);
    }

}

void ModuleJITHolder::RegisterKernel(KernelID kernelId, KernelInfo kernelinfo)
{
    size_t count = m_KernelsMap.count(kernelId);
    if ( 0 != count)
    {
        assert( false && "kernel ID already exists");
        return ;
    }
    m_KernelsMap[kernelId] = kernelinfo;
}

int ModuleJITHolder::GetKernelEntryPoint(KernelID kernelId) const
{
    std::map<KernelID, KernelInfo>::const_iterator it = m_KernelsMap.find(kernelId);
    if ( m_KernelsMap.end() == it )
    {
        assert( false && "Kernel not found");
        return -1;
    }
    return it->second.kernelOffset;
}

int ModuleJITHolder::GetKernelJITSize( KernelID kernelId ) const
{
    std::map<KernelID, KernelInfo>::const_iterator it = m_KernelsMap.find(kernelId);
    if ( m_KernelsMap.end() == it )
    {
        assert( false && "Kernel not found");
        return -1;
    }
    return it->second.kernelSize;
}

const LineNumberTable* ModuleJITHolder::GetKernelLineNumberTable(KernelID kernelId) const {
    std::map<KernelID, KernelInfo>::const_iterator it = m_KernelsMap.find(kernelId);
    if ( m_KernelsMap.end() == it )
    {
        assert( false && "Kernel not found");
        return NULL;
    }
    return &(it->second.lineNumberTable);
}

const char * ModuleJITHolder::GetKernelFilename(KernelID kernelId) const {
    std::map<KernelID, KernelInfo>::const_iterator it = m_KernelsMap.find(kernelId);
    if ( m_KernelsMap.end() == it )
    {
        assert( false && "Kernel not found");
        return NULL;
    }
    return it->second.filename.c_str();
}

const InlinedFunctions* ModuleJITHolder::GetKernelInlinedFunctions(KernelID kernelId) const
{
    std::map<KernelID, KernelInfo>::const_iterator it = m_KernelsMap.find(kernelId);
    if ( m_KernelsMap.end() == it )
    {
        assert( false && "Kernel not found");
        return NULL;
    }
    return &(it->second.inlinedFunctions);
}

int ModuleJITHolder::GetKernelVtuneFunctionId(KernelID kernelId) const {
    std::map<KernelID, KernelInfo>::const_iterator it = m_KernelsMap.find(kernelId);
    if ( m_KernelsMap.end() == it )
    {
        assert( false && "Kernel not found");
        return -1;
    }
    return it->second.functionId;
}

int ModuleJITHolder::GetKernelCount() const
{
    return m_KernelsMap.size();
}

}}} // namespace 
