#include "debug_server.h"
#include "cl_dev_backend_api.h"
#include "llvm/System/DataTypes.h"
#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Metadata.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


template <class T>
static inline T* objptr_from_addr(uint64_t addr)
{
    void* ptr = reinterpret_cast<void*>(addr);
    return static_cast<T*>(ptr);
}


// Is the global ID computed matching the one being debugged?
//
static inline bool global_id_match(uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    return DebugServer::GetInstance().DebuggedGlobalIdMatch(gid0, gid1, gid2);
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_declare_local(
    void* addr, uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    if (global_id_match(gid0, gid1, gid2)) {
	    MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
	    DebugServer::GetInstance().DeclareLocal(addr, metadata_ptr);
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_declare_global(
    void* addr, uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    if (global_id_match(gid0, gid1, gid2)) {
        MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
        DebugServer::GetInstance().DeclareGlobal(addr, metadata_ptr);
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_enter_function(
    uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    if (global_id_match(gid0, gid1, gid2)) {
        MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
        DebugServer::GetInstance().EnterFunction(metadata_ptr);
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_exit_function(
    uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    if (global_id_match(gid0, gid1, gid2)) {
        MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
        DebugServer::GetInstance().ExitFunction(metadata_ptr);
    }
}


extern "C" LLVM_BACKEND_API void __opencl_dbg_stoppoint(
    uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2)
{
    if (global_id_match(gid0, gid1, gid2)) {
        MDNode* metadata_ptr = objptr_from_addr<MDNode>(metadata_addr);
        DebugServer::GetInstance().Stoppoint(metadata_ptr);
    }
}
