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
#include "Serializer.h"
#include "MICSerializationService.h"
#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ModuleJITHolder::ModuleJITHolder():
    m_pJITBuffer(NULL),
    m_pJITCode(NULL),
    m_JITCodeSize(0),
    m_alignment(0),
    m_pJITAllocator(NULL)
{}

ModuleJITHolder::~ModuleJITHolder()
{
    if(NULL != m_pJITBuffer)
    {
        // in case m_pJITAllocator is NULL this means that PCG generated the JIT
        if(NULL == m_pJITAllocator) free(m_pJITBuffer);
        else m_pJITAllocator->FreeExecutable(m_pJITBuffer);
    }
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

void ModuleJITHolder::SetJITCodeStartPoint(const void* pJITCodeStartpoint)
{
    m_pJITCode = (const char*)pJITCodeStartpoint;
}

const void* ModuleJITHolder::GetJITCodeStartPoint() const
{
    return m_pJITCode;
}

void ModuleJITHolder::SetJITBufferPointer(void* pJITBuffer)
{
    m_pJITBuffer = (char*)pJITBuffer;
}

const void* ModuleJITHolder::GetJITBufferPointer() const
{
    return m_pJITBuffer;
}

void ModuleJITHolder::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    // using unsigned long long int instead of size_t is because that size_t
    // varies in it's size relating to the platform (32/64 bit)
    unsigned long long int tmp = (unsigned long long int)m_JITCodeSize;
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
        KernelID kernelID = it->first;
        Serializer::SerialPrimitive<unsigned long long int>(&kernelID, ost);
        
        KernelInfo kernelInfo = it->second;
        Serializer::SerialPrimitive<int>(&(kernelInfo.kernelOffset), ost);
        Serializer::SerialPrimitive<int>(&(kernelInfo.kernelSize), ost);
    }
    
    for(size_t i = 0; i < m_JITCodeSize; i++)
    {
        Serializer::SerialPrimitive<char>(&(m_pJITCode[i]), ost);
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
        Serializer::DeserialPrimitive<unsigned long long int>(&kernelID, ist);
        
        KernelInfo kernelInfo;
        Serializer::DeserialPrimitive<int>(&(kernelInfo.kernelOffset), ist);
        Serializer::DeserialPrimitive<int>(&(kernelInfo.kernelSize), ist);
        m_KernelsMap[kernelID] = kernelInfo;
    }
    
    // Deserailize the JIT code itself
    ICLDevBackendJITAllocator* pAllocator = stats->GetJITAllocator();
    if(NULL == pAllocator) throw Exceptions::SerializationException("Cannot Get JIT Allocator");
    
    if(NULL != m_pJITBuffer)
    {
        m_pJITAllocator->FreeExecutable(m_pJITBuffer); // free by the old allocator
        m_pJITBuffer = NULL;
    }
    
    m_pJITAllocator = pAllocator;
    m_pJITBuffer = (char*)(m_pJITAllocator->AllocateExecutable(sizeof(char) * m_JITCodeSize, m_alignment));
    if(NULL == m_pJITBuffer) throw Exceptions::SerializationException("JIT Allocator Failed Allocating Memory");
    
    for(size_t i = 0; i < m_JITCodeSize; i++)
    {
        Serializer::DeserialPrimitive<char>(&(m_pJITBuffer[i]), ist);
    }
	// we get the buffer already aligned so the pJITCode == pJITBuffer
    m_pJITCode = m_pJITBuffer;
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

int ModuleJITHolder::GetKernelCount() const
{
    return m_KernelsMap.size();
}

}}} // namespace 
