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

File Name:  OpenCLKernelArgumentsParser.h

\*****************************************************************************/


#ifndef __OPENCL_KERNEL_ARGUMENTS_PARSER__
#define __OPENCL_KERNEL_ARGUMENTS_PARSER__
#include<string>
#include<list>
#include"llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include"IMemoryObjectDesc.h"
#include"IOpenCLKernelArgumentsParser.h"
#include "Exception.h"
#include"TypeDesc.h"

namespace Validation
{
    class OpenCLKernelArgumentsParser : public IOpenCLKernelArgumentsParser
    {
    public:
        /// @brief parse of kernel argument descriptions
        /// @param [IN] programObject LLVM program object
        /// @param [IN] kernelName Name of kernel
        /// @return list of kernel argument descriptions
        std::list<IMemoryObjectDescPtr> KernelArgumentsParser(const std::string& kernelName,const llvm::Module* programObject);
    private:
        /// @brief parse of struct
        /// @param [IN] struct of LLVM program object
        /// @return struct descriptions
        TypeDesc forParserStruct(llvm::StructType *structTy);
    };

} // End of Validation namespace
#endif // __OPENCL_KERNEL_ARGUMENTS_PARSER__