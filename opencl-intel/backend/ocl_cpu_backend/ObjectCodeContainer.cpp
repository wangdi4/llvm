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

File Name:  ObjectCodeContainer.cpp

\*****************************************************************************/

#include "ObjectCodeContainer.h"
#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ObjectCodeContainer::ObjectCodeContainer(const cl_prog_container_header* pContainer)
{
    assert(pContainer && "Code container pointer must be valid");
    size_t blobSize = pContainer->container_size;
    size_t totalSize = blobSize + sizeof(cl_prog_container_header);
    m_pObjectCodeContainer = (cl_prog_container_header*)new char[totalSize];
    std::copy((const char*)pContainer, (const char*)pContainer+totalSize, (char*)m_pObjectCodeContainer);
}

ObjectCodeContainer::~ObjectCodeContainer()
{
    delete m_pObjectCodeContainer;
}

const void* ObjectCodeContainer::GetCode() const
{
    return (const char*)m_pObjectCodeContainer + sizeof(cl_prog_container_header);
}

size_t ObjectCodeContainer::GetCodeSize() const
{
    return (static_cast<const cl_prog_container_header*>(m_pObjectCodeContainer))->container_size;
}

const void* ObjectCodeContainer::GetObject() const
{
    size_t bitCodeSize = (static_cast<const cl_object_container_header*>(GetCode()))->section_size[IR_SECTION_INDEX];
    size_t serializationSize = (static_cast<const cl_object_container_header*>(GetCode()))->section_size[OFFLOAD_SECTION_INDEX];
    size_t rawModuleSize = (static_cast<const cl_object_container_header*>(GetCode()))->section_size[OPT_IR_SECTION_INDEX];
    return ((const char*)GetCode())+sizeof(const cl_prog_container_header)+bitCodeSize+serializationSize+rawModuleSize;
}

size_t ObjectCodeContainer::GetObjectSize() const
{
    return (static_cast<const cl_object_container_header*>(GetCode()))->section_size[OBJECT_SECTION_INDEX];
}


}}} // namespace

