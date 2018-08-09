// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
        size_t          LocalSize[WG_SIZE_NUM][MAX_WORK_DIM];
        size_t          WGNumber[MAX_WORK_DIM];
    };

}

#endif // LLI_OCLUPDATE_PASS_H
