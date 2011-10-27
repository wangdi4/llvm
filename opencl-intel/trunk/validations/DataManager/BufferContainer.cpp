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

File Name:  BufferContainer.cpp

\*****************************************************************************/
#include "BufferContainer.h"
#include "IContainerVisitor.h"
#include <string>
#include "Buffer.h"
#include "Image.h"

namespace Validation
{
    std::size_t BufferContainer::GetMemoryObjectCount() const
    {
        return m_buffs.size();
    }

    IMemoryObject* BufferContainer::GetMemoryObject( std::size_t buffId ) const
    {
        // assert(buffId >= m_buffs.size());
        return m_buffs[buffId];
    }

    IMemoryObject* BufferContainer::CreateBuffer( const BufferDesc& buffDesc )
    {
        // Create a Buffer and put it into container.
        try
        {
            Buffer * pB = new Buffer(buffDesc);
            m_buffs.push_back(pB);
            return pB;
        }
        catch (std::bad_alloc e)
        {
            std::ostringstream oss;
            oss << buffDesc.GetBufferSizeInBytes();
            throw Exception::InvalidArgument("CreateBuffer was unable to allocate buffer with size: " + oss.str() + " bytes.");
        }
    }

    IMemoryObject* BufferContainer::CreateImage( const ImageDesc& imDesc )
    {
        // Create an Image and put it into container.
        try
        {
            Image *pIm = new Image(imDesc);
            m_buffs.push_back(pIm);
            return pIm;
        }
        catch (std::bad_alloc e)
        {
            std::ostringstream oss;
            oss << imDesc.GetImageSizeInBytes();
            throw Exception::InvalidArgument("CreateImage was unable to allocate image with size: " + oss.str() + " bytes.");
        }
    }

    BufferContainer::~BufferContainer()
    {
        // TODO: Make this method thread-safe
        if(!m_buffs.empty())
        {
            for(MemoryObjectList::iterator it = m_buffs.begin();
                it != m_buffs.end();
                ++it)
            {
                delete *it;
            }
            m_buffs.clear();
        }
    }

    void BufferContainer::Accept( IContainerVisitor& visitor ) const
    {
        visitor.visitBufferContainer(this);
        for(MemoryObjectList::const_iterator it = m_buffs.begin(); it != m_buffs.end(); ++it)
        {
            (*it)->Accept(visitor);
        }
    }

} // End of Validation namespace
