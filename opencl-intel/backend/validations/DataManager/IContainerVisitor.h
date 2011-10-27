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

File Name:  IContainerVisitor.h

\*****************************************************************************/
#ifndef __ICONTAINERVISITOR_H__
#define __ICONTAINERVISITOR_H__

#include "IMemoryObject.h"
#include "IBufferContainer.h"
#include "IBufferContainerList.h"

namespace Validation
{
    /// @brief IContainerVisitor interface
    /// Base interface for all the container visitors
    class IContainerVisitor
    {
    public:
        virtual ~IContainerVisitor(){}

        virtual void visitImage( const IMemoryObject* pImage ) = 0;
        virtual void visitBuffer( const IMemoryObject* pBuffer ) = 0;
        virtual void visitBufferContainer( const IBufferContainer* pBufferContainer) = 0;
        virtual void visitBufferContainerList( const IBufferContainerList* pBufferContainerList ) = 0;
    };
}

#endif // __ICONTAINERVISITOR_H__
