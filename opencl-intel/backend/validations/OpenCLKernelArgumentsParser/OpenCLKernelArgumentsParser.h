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
#include <string>
#include <list>
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "IMemoryObjectDesc.h"
#include "IOpenCLKernelArgumentsParser.h"
#include "Exception.h"
#include "TypeDesc.h"
#include <stack>

namespace Validation
{
    class OpenCLKernelArgumentsParser : public IOpenCLKernelArgumentsParser
    {
    public:
        /// @brief parse kernel argument descriptions
        /// @param [IN] programObject LLVM program object
        /// @param [IN] kernelName Name of kernel
        /// @return list of kernel argument descriptions
        OCLKernelArgumentsList KernelArgumentsParser(const std::string& kernelName,const llvm::Module* programObject);
        /// @brief static function that tries to guess 
        /// number of pointed elements by pointer
        /// @param [IN] Args - reference to output of arg parser
        /// @param [IN] globalworksize is the pointer to array that 
        /// contains number of work items for dimention i
        /// @param [IN] dim the number of dimentions
        /// @return new list of arguments
        static OCLKernelArgumentsList KernelArgHeuristics(const OCLKernelArgumentsList &Args,
                                                            const size_t* globalworksize, const uint64_t dim);
    private:
        /// @brief parse of struct
        /// @param [IN] struct of LLVM program object
        /// @return struct descriptions
        TypeDesc forParserStruct(llvm::StructType *structTy);
    };

    /// @brief recurively iterates elements of the TypeDesc tree
    /// @param [in] head is a refence to head of current sub-tree
    /// @param [in] def_size is the number of elemnts in pointer
    /// @return new sub-tree
    static TypeDesc RecursiveDFS(const TypeDesc& head, const uint64_t def_size){
        uint64_t i;
        TypeDesc node;
        TypeDesc tmp;

        node = head;
        if(head.GetType()==TPOINTER){
            node.SetNumberOfElements(def_size);
        }
        for(i=0;i<head.GetNumOfSubTypes();++i){
            tmp = head.GetSubTypeDesc(i);
            tmp = RecursiveDFS(tmp, def_size);
            node.SetSubTypeDesc(i, tmp);
        }
        return node;
    }

} // End of Validation namespace
#endif // __OPENCL_KERNEL_ARGUMENTS_PARSER__