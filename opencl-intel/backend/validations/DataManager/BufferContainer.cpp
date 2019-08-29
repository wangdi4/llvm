// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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
        catch (std::bad_alloc& e)
        {
            std::ostringstream oss;
            oss << buffDesc.GetSizeInBytes();
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
        catch (std::bad_alloc& e)
        {
            std::ostringstream oss;
            oss << imDesc.GetSizeInBytes();
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
