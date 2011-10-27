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

File Name:  OCLUpdatePass.h

\*****************************************************************************/

#ifndef  LLI_OCLUPDATE_PASS_H
#define LLI_OCLUPDATE_PASS_H
#pragma warning (disable : 4985 ) /* disable ceil warnings */ 

#include <list>
#include <map>
#include <vector>
#include <string>
#include "cl_types.h"
#include "llvm/Pass.h"
#include "llvm/LLVMContext.h"

namespace Validation{

    llvm::ModulePass *createOCLReferenceKernelUpdatePass(llvm::Pass *, llvm::SmallVectorImpl<llvm::Function*> &vectFunctions,
        llvm::LLVMContext *context, std::vector<std::string> &UndefinedExternalFunctions);
    
    /// structure used for passing work item data to kernel
    /// used in OCLReferenceKernelUpdatePass and OpenCLReferenceRunner
    /// it is clone of Intel::OpenCL::DeviceBackend::sWorkInfo structure
    /// from llvm_binary.h
    /// Clone is made for OCL Reference to be independent from OCL back-end
    struct sWorkInfoVal
    {
        unsigned int    uiWorkDim;
        size_t          GlobalOffset[MAX_WORK_DIM];
        size_t          GlobalSize[MAX_WORK_DIM];
        size_t          LocalSize[MAX_WORK_DIM];
        size_t          WGNumber[MAX_WORK_DIM];
    };

}

#endif // LLI_OCLUPDATE_PASS_H