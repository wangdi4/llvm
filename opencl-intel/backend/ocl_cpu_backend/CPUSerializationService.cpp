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

#include "CPUSerializationService.h"
#include "CPUProgram.h"
#include "Serializer.h"
#include "CPUDeviceBackendFactory.h"
#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CountingOutputStream : public IOutputStream
{
public:
    CountingOutputStream() : m_Counter(0) { }
    
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
        : m_pBuffer(pBuffer), m_Size(size), m_Pos(0) { }
        
    OutputBufferStream& Write(const char* s, size_t count)
    {
        if(m_Pos == m_Size) return *this;
        
        size_t copySize = std::min(count, m_Size - m_Pos);
        std::copy(s, s + copySize, m_pBuffer + m_Pos);
        m_Pos += copySize;
        
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
        
        size_t copySize = std::min(count, m_Size - m_Pos);
        std::copy(m_pBuffer + m_Pos, m_pBuffer + m_Pos + copySize, s);
        m_Pos += copySize;
        
        return *this;
    }
    
private:
    const char* m_pBuffer;
    size_t m_Size;
    size_t m_Pos;
};

CPUSerializationService::CPUSerializationService(const ICLDevBackendOptions* pBackendOptions)
{ 
    m_pBackendFactory = CPUDeviceBackendFactory::GetInstance(); 
    assert(m_pBackendFactory && "Backend Factory is null");
}

cl_dev_err_code CPUSerializationService::GetSerializationBlobSize(
        cl_serialization_type serializationType,
        const ICLDevBackendProgram_* pProgram, size_t* pSize) const
{
    assert((SERIALIZE_PERSISTENT_IMAGE == serializationType) && "Serialization type not supported");
    CountingOutputStream cs;

    SerializationStatus stats;
    stats.SerializeVersion(cs);
    static_cast<const CPUProgram*>(pProgram)->Serialize(cs, &stats);
    *pSize = cs.GetCount();

    return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUSerializationService::SerializeProgram(
        cl_serialization_type serializationType, 
        const ICLDevBackendProgram_* pProgram, 
        void* pBlob, size_t blobSize) const
{
    assert((SERIALIZE_PERSISTENT_IMAGE == serializationType) && "Serialization type not supported");
    OutputBufferStream obs((char*)pBlob, blobSize);

    SerializationStatus stats;
    stats.SerializeVersion(obs);
    static_cast<const CPUProgram*>(pProgram)->Serialize(obs, &stats);

    return CL_DEV_SUCCESS;
}

void CPUSerializationService::ReleaseProgram(ICLDevBackendProgram_* pProgram) const
{
    delete pProgram;
}

cl_dev_err_code CPUSerializationService::ReloadProgram(
        cl_serialization_type serializationType, 
        ICLDevBackendProgram_* pProgram, 
        const void* pBlob, size_t blobSize) const
{
    try
    {
        SerializationStatus stats;
        stats.SetBackendFactory(m_pBackendFactory);

        InputBufferStream ibs((char*)pBlob, blobSize);
        stats.DeserialVersion(ibs);
        
        static_cast<CPUProgram*>(pProgram)->Deserialize(ibs, &stats);

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


cl_dev_err_code CPUSerializationService::DeSerializeProgram(
        cl_serialization_type serializationType, 
        ICLDevBackendProgram_** ppProgram, 
        const void* pBlob, size_t blobSize) const
{

    try
    {
        SerializationStatus stats;
        stats.SetBackendFactory(m_pBackendFactory);

        std::auto_ptr<ICLDevBackendProgram_> tmpProgram(stats.GetBackendFactory()->CreateProgram());
        cl_dev_err_code err = ReloadProgram(serializationType, *ppProgram, pBlob, blobSize);
        if(CL_DEV_SUCCESS == err) *ppProgram = tmpProgram.release();
        return err;
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

void CPUSerializationService::Release()
{
}

}}} // namespace
