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

File Name:  Serializer.cpp

\*****************************************************************************/

#include "Serializer.h"
#include <map>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

SerializationStatus::SerializationStatus():
    m_pJITAllocator(nullptr),
    m_pBackendFactory(nullptr),
    m_RuntimeVersion(1),
    m_LLVMVersion(32)
    {}

void SerializationStatus::SetPointerMark(const std::string& mark, void* pointer)
{
    std::map<std::string, void*>::iterator it = m_marksMap.find(mark);
    if ( m_marksMap.end() != it )
    {
        assert( false && "Mark already exist on the serialization status");
        return ;
    }

    m_marksMap[mark] = pointer;
}

void* SerializationStatus::GetPointerMark(const std::string& mark)
{
    std::map<std::string, void*>::iterator it = m_marksMap.find(mark);
    if ( m_marksMap.end() == it )
    {
        assert( false && "Mark do not exist on the serialization status");
        return nullptr;
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

void SerializationStatus::SerializeVersion(IOutputStream& stream)
{
    Serializer::SerialPrimitive<int>(&m_RuntimeVersion, stream);
    Serializer::SerialPrimitive<int>(&m_LLVMVersion, stream);
}

void SerializationStatus::DeserialVersion(IInputStream& stream)
{
    Serializer::DeserialPrimitive<int>(&m_RuntimeVersion, stream);
    Serializer::DeserialPrimitive<int>(&m_LLVMVersion, stream);
}

int SerializationStatus::GetRuntimeVersion() const
{
    return m_RuntimeVersion;
}

int SerializationStatus::GetLLVMVersion() const
{
    return m_LLVMVersion;
}

} } } // namespace

