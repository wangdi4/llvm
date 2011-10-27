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

File Name:  IBufferContainerList.h

\*****************************************************************************/
#ifndef __IBUFFER_CONTAINER_LIST_H__
#define __IBUFFER_CONTAINER_LIST_H__

#include <vector>
#include <cstddef>              // for std::size_t

#include "IContainer.h"
#include "IBufferContainer.h"

namespace Validation
{

    /// @brief Interface to a BufferContainerList object
    /// this object stores collection of BufferContainer objects
    class IBufferContainerList : public IContainer
    {
    public:
        /// @brief Factory method, which creates empty BufferContainer objectand puts it into BufferContainerList container.
        /// @return Pointer to created BufferContainer object interface.
        virtual IBufferContainer* CreateBufferContainer() = 0;
        // Methods to iterate over buffers
        // TODO: implement C++-style interator for BufferContainer
        /// @brief Method to get the number of BufferContainers
        /// @return number of BufferContainer objects
        virtual std::size_t GetBufferContainerCount() const = 0;
        /// @brief Method to get particular buffer by index.
        /// @param [IN] id Buffer index. Id have to lay inside range [0, GetMemoryObjectCount() - 1]
        /// @return Pointer to the buffer's interface.
        virtual IBufferContainer* GetBufferContainer(std::size_t id) const = 0;
    };

}
#endif // __IBUFFER_CONTAINER_LIST_H__

