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

File Name:  BufferContainer.h

\*****************************************************************************/
#ifndef __BUFFER_CONTAINER_H__
#define __BUFFER_CONTAINER_H__

#include <vector>
#include <cstddef>              // for std::size_t
#include "IMemoryObject.h"
#include "BufferDesc.h"
#include "ImageDesc.h"
#include "IBufferContainer.h"

namespace Validation
{
    class BufferContainerList;
    class IContainerVisitor;

    class BufferContainer : public IBufferContainer
    {
    public:
        /// ctor
        BufferContainer(){}
        /// dtor. Should be called only by BufferContainerList object
        virtual ~BufferContainer();
        std::size_t GetMemoryObjectCount() const;
        IMemoryObject* GetMemoryObject(std::size_t buffId) const;
        IMemoryObject* CreateBuffer(const BufferDesc& buffDesc);
        IMemoryObject* CreateImage(const ImageDesc& imDesc);

        void Accept( IContainerVisitor& visitor ) const;

    private:
        /// hide copy constructor
        BufferContainer(const BufferContainer& ) : IBufferContainer() {}
        /// hide assignment operator
        void operator =(BufferContainer&){}
        /// typedef for MemoryObjectList
        typedef std::vector<IMemoryObject*> MemoryObjectList;
        /// List of buffers
        MemoryObjectList m_buffs;
        /// declare friend for access private members
        friend class BufferContainerList;
    };
} // End of Validation namespace

#endif // __BUFFERS_CONTEINER_H__

