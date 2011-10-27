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

File Name:  IBufferContainer.h

\*****************************************************************************/
#ifndef __IBUFFER_CONTAINER_H__
#define __IBUFFER_CONTAINER_H__

#include <cstddef>              // for std::size_t
#include "IContainer.h"
#include "IMemoryObject.h"
#include "BufferDesc.h"
#include "ImageDesc.h"

namespace Validation
{
    /// @brief Interface to a container of buffers
    class IBufferContainer : public IContainer
    {
    public:
        /// @brief Factory method, which creates buffer and puts it into buffer container.
        /// @param [IN] buffDesc Description of buffer to create.
        /// @return Pointer to created buffer.
        virtual IMemoryObject* CreateBuffer(const BufferDesc& buffDesc) = 0;

        /// @brief Factory method, which creates buffer and puts it into buffer container.
        /// @param [IN] buffDesc Description of buffer to create.
        /// @return Pointer to created buffer.
        virtual IMemoryObject* CreateImage(const ImageDesc& imDesc) = 0;

        // Methods to iterate over buffers
        // TODO: implement C++-style interator for BufferContainer

        /// @brief Method to get the number of buffers in buffer container.
        /// @return number of buffers
        virtual std::size_t GetMemoryObjectCount() const = 0;
        /// @brief Method to get particular buffer by index.
        /// @param [IN] id Buffer index. Id have to lay inside range [0, GetMemoryObjectCount() - 1]
        /// @return Pointer to the buffer's interface.
        virtual IMemoryObject* GetMemoryObject(std::size_t id) const = 0;
    };

} // End of Validation namespace

#endif // __IBUFFERS_CONTAINER_H__

