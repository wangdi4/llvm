// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include "ServiceFactory.h"
#include "icldebuggingservice.h"
#include "cl_dev_backend_api.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

template <class T>
static inline T* objptr_from_addr(uint64_t addr)
{
    void* ptr = reinterpret_cast<void*>(addr);
    return static_cast<T*>(ptr);
}


// Obtain a pointer to the debugging service from the global service factory.
// Return NULL if no valid debugging service could be found.
//
static inline ICLDebuggingService* TheDebuggingService()
{
    ICLDevBackendServiceFactoryInternal* serviceFactory =
        ServiceFactory::GetInstanceInternal();
    ICLDebuggingService* debuggingService;
    if (serviceFactory->GetDebuggingService(&debuggingService) == CL_DEV_SUCCESS)
        return debuggingService;
    else
        return nullptr;
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_declare_local(
    void* addr, uint64_t var_metadata_addr, uint64_t expr_metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    ICLDebuggingService* debuggingService = TheDebuggingService();
    if (debuggingService) {
        if (debuggingService->DebuggedGlobalIdMatch(gid0, gid1, gid2)) {
          MDNode* var_metadata_ptr = objptr_from_addr<MDNode>(var_metadata_addr);
          MDNode* expr_metadata_ptr = objptr_from_addr<MDNode>(expr_metadata_addr);
          debuggingService->DeclareLocal(addr, var_metadata_ptr, expr_metadata_ptr);
        }
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_declare_global(
    void* addr, uint64_t var_metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    ICLDebuggingService* debuggingService = TheDebuggingService(); 
    if (debuggingService) {
        if (debuggingService->DebuggedGlobalIdMatch(gid0, gid1, gid2)) {
            MDNode* metadata_ptr = objptr_from_addr<MDNode>(var_metadata_addr);
            debuggingService->DeclareGlobal(addr, metadata_ptr);
        }
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_enter_function(
    uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    ICLDebuggingService* debuggingService = TheDebuggingService(); 
    if (debuggingService) {
        if (debuggingService->DebuggedGlobalIdMatch(gid0, gid1, gid2)) {
            MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
            debuggingService->EnterFunction(metadata_ptr);
        }
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_exit_function(
    uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    ICLDebuggingService* debuggingService = TheDebuggingService(); 
    if (debuggingService) {
        if (debuggingService->DebuggedGlobalIdMatch(gid0, gid1, gid2)) {
            MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
            debuggingService->ExitFunction(metadata_ptr);
        }
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_stoppoint(
    uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    ICLDebuggingService* debuggingService = TheDebuggingService(); 
    if (debuggingService) {
        if (debuggingService->DebuggedGlobalIdMatch(gid0, gid1, gid2)) {
            MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
            debuggingService->Stoppoint(metadata_ptr);
        }
    }
}
