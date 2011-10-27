/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Buffer.cpp

\*****************************************************************************/
#include "Buffer.h"
#include "IContainerVisitor.h"
#include "llvm/System/DataTypes.h"

namespace Validation
{

    ///////////////////////////////////
    /// Buffer implementation
    ///////////////////////////////////////
    void Buffer::AllocateMemoryForData()
    {
        // Compute size of memory to allocate
        std::size_t buffSize = m_desc.GetBufferSizeInBytes();
        // Allocate memory
        m_data = new uint8_t[buffSize];
    }

    Buffer::Buffer( const BufferDesc& desc ) : m_desc(desc)
    {
        if (!m_desc.IsFloatingPoint() && m_desc.IsNEAT())
        {
            throw Exception::InvalidArgument("NEAT Buffer without floating point couldn't be created.");
        }
        // Allocate memory
        AllocateMemoryForData();
    }

    Buffer::~Buffer()
    {
        if (0 != m_data)
        {
            delete[] m_data;
        }
    }

    void Buffer::Accept( IContainerVisitor& visitor ) const
    {
        visitor.visitBuffer(this);
    }

    BufferDesc GetBufferDescription( const IMemoryObjectDesc* iDesc )
    {
        assert(NULL != dynamic_cast<const BufferDesc *>(iDesc) && "BufferDesc is expected");
        return *static_cast<const BufferDesc *>(iDesc);
    }

} // End of Validation namespace

