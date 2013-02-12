/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OCLKernelBufferContainerListAllocator.h

\*****************************************************************************/
#ifndef _OCL_KERNEL_BUFFER_CONTAINER_LIST_ALLOCATOR_
#define _OCL_KERNEL_BUFFER_CONTAINER_LIST_ALLOCATOR_

#include"IDataReader.h"
#include"OpenCLKernelArgumentsParser.h"
#include"IContainer.h"

namespace Validation
{
    ///class - memory allocator
    class OCLKernelBufferContainerListAllocator: public IDataReader
    {
    public:
        ///@brief ctor
        ///@param [in] list is a const reference to list of kernel arguments(OCLKernelArgumentsList)
        OCLKernelBufferContainerListAllocator(const OCLKernelArgumentsList& list)
            : m_list(list)
        {}
        ///@brief allocate memory for arguments
        ///@param [in out] p is a pointer to instance of BufferContainerList
        ///creates buffer in BufferContainerList for each argument from OCLKernelArgumentsList
        virtual void Read(IContainer *p);
    private:
        OCLKernelArgumentsList m_list;
    };


}
#endif
