/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ocl_debug_builtins.cpp

\*****************************************************************************/

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
        return NULL;
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
