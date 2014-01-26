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

File Name:  IOpenCLKernelArgumentsParser.h

\*****************************************************************************/


#ifndef __IOPENCL_KERNEL_ARGUMENTS_PARSER__
#define __IOPENCL_KERNEL_ARGUMENTS_PARSER__
#include<string>
#include<vector>
#include"llvm/IR/Module.h"
#include"IMemoryObjectDesc.h"

namespace Validation
{
    typedef std::vector<IMemoryObjectDescPtr> OCLKernelArgumentsList;
    /// @brief Interface to a Kernel Arguments Parser.
    class IOpenCLKernelArgumentsParser
    {
    public:
        /// @brief parse of kernel argument descriptions 
        /// @param [IN] programObject LLVM program object
        /// @param [IN] kernelName Name of kernel
        /// @return list of kernel argument descriptions
        virtual OCLKernelArgumentsList KernelArgumentsParser(const std::string& kernelName,const llvm::Module* programObject) = 0;
    };


} // End of Validation namespace
#endif // __OPENCL_KERNEL_ARGUMENTS_PARSER__