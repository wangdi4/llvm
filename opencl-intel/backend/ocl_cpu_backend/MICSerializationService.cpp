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

DefaultJITMemoryManager* DefaultJITMemoryManager::s_pInstance = nullptr;

void DefaultJITMemoryManager::Init()
{
    assert(!s_pInstance);
    s_pInstance = new DefaultJITMemoryManager();
}

void DefaultJITMemoryManager::Terminate()
{
    if( nullptr != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
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
    if(nullptr == pMem) return nullptr;
    
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
        return nullptr;
    }
#else
    assert(false && "Not implemented");
#endif
    
    return pAligned;
}

void DefaultJITMemoryManager::FreeExecutable(void* ptr)
{
    void* pMem  = ((void**)ptr)[-1];

#if defined(__LP64__)
    void* pSize = (void*)(((char*)ptr) - sizeof(void*));
    size_t size = ((size_t*)pSize)[-1];
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
    m_pJITAllocator = nullptr;

    void* pCallBack = nullptr;
    size_t size = 0;
    if(nullptr != pBackendOptions && 
       pBackendOptions->GetValue(CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR, &pCallBack, &size))
    {
        if(nullptr == pCallBack)
        {
            throw Exceptions::DeviceBackendExceptionBase("JIT Allocator pointer in the options is NULL", CL_DEV_INVALID_VALUE);
        }
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
    assert((SERIALIZE_OFFLOAD_IMAGE == serializationType) && "Serialization type not supported");
    CountingOutputStream cs;

    SerializationStatus stats;
    stats.SerializeVersion(cs);
    static_cast<const MICProgram*>(pProgram)->Serialize(cs, &stats);
    *pSize = cs.GetCount();

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICSerializationService::SerializeProgram(
        cl_serialization_type serializationType, 
        const ICLDevBackendProgram_* pProgram, 
        void* pBlob, size_t blobSize) const
{
    assert((SERIALIZE_OFFLOAD_IMAGE == serializationType) && "Serialization type not supported");
    OutputBufferStream obs((char*)pBlob, blobSize);

    SerializationStatus stats;
    stats.SerializeVersion(obs);
    static_cast<const MICProgram*>(pProgram)->Serialize(obs, &stats);

    return CL_DEV_SUCCESS;
}

void MICSerializationService::ReleaseProgram(ICLDevBackendProgram_* pProgram) const
{
    delete pProgram;
}

cl_dev_err_code MICSerializationService::GetTargetDescriptionBlobSize(
    const TargetDescription* pTargetDescription, 
    size_t* pSize) const
{
    CountingOutputStream cs;

    pTargetDescription->Serialize(cs, nullptr);
    *pSize = cs.GetCount();

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICSerializationService::SerializeTargetDescription(
    const TargetDescription* pTargetDescription, 
    void* pBlob, 
    size_t blobSize) const
{
    OutputBufferStream obs((char*)pBlob, blobSize);

    pTargetDescription->Serialize(obs, nullptr);

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
        
        (*pTargetDescription)->Deserialize(ibs, nullptr);

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

cl_dev_err_code MICSerializationService::ReloadProgram(
        cl_serialization_type serializationType, 
        ICLDevBackendProgram_* pProgram, 
        const void* pBlob, size_t blobSize) const
{
    assert(m_pJITAllocator && "JIT memory Allocator is null");

    try
    {
        SerializationStatus stats;
        stats.SetJITAllocator(m_pJITAllocator);
        stats.SetBackendFactory(m_pBackendFactory);

        InputBufferStream ibs((char*)pBlob, blobSize);
        stats.DeserialVersion(ibs);
        
        static_cast<MICProgram*>(pProgram)->Deserialize(ibs, &stats);

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


cl_dev_err_code MICSerializationService::DeSerializeProgram(
        cl_serialization_type serializationType, 
        ICLDevBackendProgram_** ppProgram, 
        const void* pBlob, size_t blobSize) const
{

    try
    {
        SerializationStatus stats;
        stats.SetBackendFactory(m_pBackendFactory);

        std::auto_ptr<ICLDevBackendProgram_> tmpProgram(stats.GetBackendFactory()->CreateProgram());
        cl_dev_err_code err = ReloadProgram(serializationType, tmpProgram.get(), pBlob, blobSize);
        if(CL_DEV_SUCCESS == err) *ppProgram = tmpProgram.release();
        return err;
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

void MICSerializationService::Release()
{
}

}}} // namespace
