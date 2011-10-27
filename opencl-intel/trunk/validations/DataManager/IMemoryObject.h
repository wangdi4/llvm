/*****************************************************************************\

Copyright (c) Intel Corporation (2010, 2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  IMemoryObject.h

\*****************************************************************************/
#ifndef __I_MEMORY_OBJECT_H__
#define __I_MEMORY_OBJECT_H__

#include "BufferDesc.h"                  // Data Manager library data types
#include "IContainer.h"
#include <string>

namespace Validation
{
    ///////////////////////////////////////////////////
    /// Interface to data container objects.
    class IMemoryObject : public IContainer
    {
    public:
        /// @brief Provide access to buffer's data.
        /// @return Pointer to memory region with buffer's data.
        virtual void* GetDataPtr() const = 0;
        /// @brief Provide access to buffer's description.
        /// @return buffer's description.
        virtual const IMemoryObjectDesc* GetMemoryObjectDesc() const = 0;
        /// @brief Name of object class.
        /// @return String with the name of the class.
        virtual std::string GetName() const = 0;
    };
} // End of Validation namespace

#endif // __I_MEMORY_OBJECT_H__

