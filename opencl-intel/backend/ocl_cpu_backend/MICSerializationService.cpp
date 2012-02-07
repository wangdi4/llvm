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

File Name:  MICSerializationService.cpp

\*****************************************************************************/

#include "MICSerializationService.h"
#include "MICProgram.h"
#include "TargetDescription.h"
#include "Serializer.h"
#include "MICDeviceBackendFactory.h"
#include <assert.h>

#if defined(__LP64__)
#include <sys/mman.h>
#endif

#define PAGE_SIZE 4096 // move this

namespace Intel { namespace OpenCL { namespace DeviceBackend {

DefaultJITMemoryManager* DefaultJITMemoryManager::s_pInstance = NULL;

void DefaultJITMemoryManager::Init()
{
    assert(!s_pInstance);
    s_pInstance = new DefaultJITMemoryManager();
}

void DefaultJITMemoryManager::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

DefaultJITMemoryManager* DefaultJITMemoryManager::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

void* DefaultJITMemoryManager::AllocateExecutable(size_t size, size_t alignment)
{
    size_t required_size = (size % PAGE_SIZE == 0) ? size : ((size_t)(size/PAGE_SIZE) + 1)*PAGE_SIZE;
    
    size_t aligned_size = 
        required_size +    // required size
        (alignment - 1) +  // for alignment 
        sizeof(void*) +    // for the free ptr
        sizeof(size_t);    // to save the original size (for mprotect)
    void* pMem = malloc(aligned_size);
    if(NULL == pMem) return NULL;
    
    char* pAligned = ((char*)pMem) + aligned_size - required_size;
    pAligned = (char*)(((size_t)pAligned) & ~(alignment - 1));
    ((void**)pAligned)[-1] = pMem;
    void* pSize = (void*)(((char*)pAligned) - sizeof(void*));
    ((size_t*)pSize)[-1] = required_size;
    
#if defined(__LP64__)
    int ret = mprotect( (void*)pAligned, required_size, PROT_READ | PROT_WRITE | PROT_EXEC );
    if (0 != ret)
    {
        free(pMem);
        return NULL;
    }
#else
    assert(false && "Not implemented");
#endif
    
    return pAligned;
}

void DefaultJITMemoryManager::FreeExecutable(void* ptr)
{
    void* pMem  = ((void**)ptr)[-1];
    void* pSize = (void*)(((char*)ptr) - sizeof(void*));
    size_t size = ((size_t*)pSize)[-1];
    
#if defined(__LP64__)
    mprotect( (void*)ptr, size, PROT_READ | PROT_WRITE );
#else
    assert(false && "Not implemented");
#endif

    free(pMem);
}

class CountingOutputStream : public IOutputStream
{
public:
    CountingOutputStream() : m_Counter(0) { };
    
    CountingOutputStream& Write(const char* s, size_t count)
    {
        m_Counter += count;
        return *this;
    }
    
    size_t GetCount()
    {
        return m_Counter;
    }
    
private:
    size_t m_Counter;
};

class OutputBufferStream : public IOutputStream
{
public:
    OutputBufferStream(char* pBuffer, size_t size) 
        : m_pBuffer(pBuffer), m_Size(size), m_Pos(0) { };
        
    OutputBufferStream& Write(const char* s, size_t count)
    {
        if(m_Pos == m_Size) return *this;
        
        for(size_t i = 0; m_Pos < m_Size && i < count; i++, m_Pos++)
        {
            m_pBuffer[m_Pos] = s[i];
        }
        
        return *this;
    }
private:
    char* m_pBuffer;
    size_t m_Size;
    size_t m_Pos;
};

class InputBufferStream : public IInputStream
{
public:
    InputBufferStream(const char* pBuffer, size_t size) 
        : m_pBuffer(pBuffer), m_Size(size), m_Pos(0) { };
        
    InputBufferStream& Read(char* s, size_t count)
    {
        if(m_Pos == m_Size) return *this;
        
        for(size_t i = 0; m_Pos < m_Size && i < count; i++, m_Pos++)
        {
            s[i] = m_pBuffer[m_Pos];
        }
        
        return *this;
    }
    
private:
    const char* m_pBuffer;
    size_t m_Size;
    size_t m_Pos;
};

MICSerializationService::MICSerializationService(const ICLDevBackendOptions* pBackendOptions)
{
    DefaultJITMemoryManager* pJITMemManager = DefaultJITMemoryManager::GetInstance();
    m_pJITAllocator = NULL;

    void* pCallBack = NULL;
    size_t size = 0;
    if(NULL != pBackendOptions && 
       pBackendOptions->GetValue(CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR, &pCallBack, &size))
    {
        m_pJITAllocator = (ICLDevBackendJITAllocator*)pCallBack;
    }
    else
    {
        m_pJITAllocator = pJITMemManager;
    }
    
    m_pBackendFactory = MICDeviceBackendFactory::GetInstance(); 
    assert(m_pBackendFactory && "Backend Factory is null");
}

cl_dev_err_code MICSerializationService::GetSerializationBlobSize(
        cl_serialization_type serializationType,
        const ICLDevBackendProgram_* pProgram, size_t* pSize) const
{
    assert((SERIALIZE_TO_DEVICE == serializationType) && "Serialization type not supported");
    CountingOutputStream cs;

    SerializationStatus stats;
    static_cast<const MICProgram*>(pProgram)->Serialize(cs, &stats);
    *pSize = cs.GetCount();

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICSerializationService::SerializeProgram(
        cl_serialization_type serializationType, 
        const ICLDevBackendProgram_* pProgram, 
        void* pBlob, size_t blobSize) const
{
    assert((SERIALIZE_TO_DEVICE == serializationType) && "Serialization type not supported");
    OutputBufferStream obs((char*)pBlob, blobSize);

    SerializationStatus stats;
    static_cast<const MICProgram*>(pProgram)->Serialize(obs, &stats);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICSerializationService::DeSerializeProgram(
        ICLDevBackendProgram_** ppProgram, 
        const void* pBlob,
        size_t blobSize) const
{
    assert(m_pJITAllocator && "JIT memory Allocator is null");

    try
    {
        SerializationStatus stats;
        stats.SetJITAllocator(m_pJITAllocator);
        stats.SetBackendFactory(m_pBackendFactory);

        InputBufferStream ibs((char*)pBlob, blobSize);
        *ppProgram = stats.GetBackendFactory()->CreateProgram();
        
        static_cast<MICProgram*>(*ppProgram)->Deserialize(ibs, &stats);

        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::SerializationException& )
    {
        return CL_DEV_ERROR_FAIL;
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

cl_dev_err_code MICSerializationService::GetTargetDescriptionBlobSize(
    const TargetDescription* pTargetDescription, 
    size_t* pSize) const
{
    CountingOutputStream cs;

    pTargetDescription->Serialize(cs, NULL);
    *pSize = cs.GetCount();

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICSerializationService::SerializeTargetDescription(
    const TargetDescription* pTargetDescription, 
    void* pBlob, 
    size_t blobSize) const
{
    OutputBufferStream obs((char*)pBlob, blobSize);

    pTargetDescription->Serialize(obs, NULL);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICSerializationService::DeSerializeTargetDescription(
    TargetDescription** pTargetDescription, 
    const void* pBlob,
    size_t blobSize) const
{
    try
    {
        InputBufferStream ibs((const char*)pBlob, blobSize);
        *pTargetDescription = new TargetDescription();
        
        (*pTargetDescription)->Deserialize(ibs, NULL);

        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::SerializationException& )
    {
        return CL_DEV_ERROR_FAIL;
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

void MICSerializationService::Release()
{
}

SerializationStatus::SerializationStatus():
    m_pJITAllocator(NULL),
    m_pBackendFactory(NULL)
    {}

void SerializationStatus::SetPointerMark(const std::string& mark, void* pointer)
{
    size_t count = m_marksMap.count(mark);
    if ( 0 != count)
    {
        assert( false && "Mark already exist on the serialization status");
        return ;
    }

    m_marksMap[mark] = pointer;
}

void* SerializationStatus::GetPointerMark(const std::string& mark)
{
    size_t count = m_marksMap.count(mark);
    if ( 0 == count )
    {
        assert( false && "Mark do not exist on the serialization status");
        return NULL;
    }

    return m_marksMap[mark];
}

void SerializationStatus::SetJITAllocator(ICLDevBackendJITAllocator* pJITAllocator)
{
    m_pJITAllocator = pJITAllocator;
}

ICLDevBackendJITAllocator* SerializationStatus::GetJITAllocator()
{
    return m_pJITAllocator;
}

void SerializationStatus::SetBackendFactory(IAbstractBackendFactory* pBackendFactory)
{
    m_pBackendFactory = pBackendFactory;
}

IAbstractBackendFactory* SerializationStatus::GetBackendFactory()
{
    return m_pBackendFactory;
}

}}} // namespace
