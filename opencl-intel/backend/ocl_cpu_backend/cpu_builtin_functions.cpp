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


#include "ExecutionContext.h"
#include "ICLDevBackendServiceFactory.h"
#include "cpu_dev_limits.h"
#include "SystemInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/DataTypes.h"

#include <stdio.h>

#ifdef _WIN32
  #include <smmintrin.h>
#else
  #ifdef __SSE4_1__
    #include <smmintrin.h>
  #endif
#endif

#include <assert.h>
#include <stdarg.h>
#include <string>

#ifndef LLVM_BACKEND_NOINLINE_PRE
   #if defined(_WIN32)
      #define LLVM_BACKEND_NOINLINE_PRE __declspec(noinline)
   #else
      #define LLVM_BACKEND_NOINLINE_PRE
   #endif
#endif
#ifndef LLVM_BACKEND_NOINLINE_POST
   #if defined(_WIN32)
      #define LLVM_BACKEND_NOINLINE_POST
   #else
      #define LLVM_BACKEND_NOINLINE_POST __attribute__ ((__noinline__))
   #endif
#endif

using namespace Intel::OpenCL::DeviceBackend;

#ifndef OclCpuBackEnd_EXPORTS
#define OclCpuBackEnd_EXPORTS
#endif


/*****************************************************************************************************************************
*    Synchronization functions (Section 6.11.9)
*****************************************************************************************************************************/
extern "C" LLVM_BACKEND_API void cpu_dbg_print(const char* fmt, ...)
{
  va_list va;
  va_start(va, fmt);

  vprintf(fmt, va);
  va_end( va );
}

extern "C" LLVM_BACKEND_API void cpu_lprefetch(const char* ptr, size_t numElements, size_t elmSize)
{
  size_t totalLines = ((numElements * elmSize) + CPU_DEV_DCU_LINE_SIZE - 1) / CPU_DEV_DCU_LINE_SIZE;

  for (size_t i=0; i<totalLines; ++i)
  {
    _mm_prefetch(ptr, _MM_HINT_T0);
    ptr += CPU_DEV_DCU_LINE_SIZE;
  }
}

extern "C" LLVM_BACKEND_API unsigned long long cpu_get_time_counter()
{
  return Intel::OpenCL::DeviceBackend::Utils::SystemInfo::HostTime();
}

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API int opencl_printf(const char* format, char* args, ICLDevBackendDeviceAgentCallback* pCallback, void* pHandle);
extern "C" LLVM_BACKEND_API int opencl_snprintf(char* outstr, size_t size, const char* format, char* args, ICLDevBackendDeviceAgentCallback* pCallback, void* pHandle);
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_declare_local(void* addr, uint64_t var_metadata_addr, uint64_t expr_metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_declare_global(void* addr, uint64_t var_metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_stoppoint(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_enter_function(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_exit_function(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;

// Thread-local storage emulation built-in
struct __emutls_control;
extern "C" LLVM_BACKEND_API void *__opencl_emutls_get_address(__emutls_control *control);

// OpenCL20. Extended execution
class IDeviceCommandManager;
class IBlockToKernelMapper;
#include "opencl20_ext_execution.h"

// Register BI functions defined above
#define REGISTER_BI_FUNCTION(name,ptr) \
  llvm::sys::DynamicLibrary::AddSymbol(llvm::StringRef(name), (void*)(intptr_t)ptr);
void RegisterCPUBIFunctions(void)
{

    REGISTER_BI_FUNCTION("dbg_print",cpu_dbg_print)
    REGISTER_BI_FUNCTION("lprefetch",cpu_lprefetch)
    REGISTER_BI_FUNCTION("get_time_counter",cpu_get_time_counter)
    REGISTER_BI_FUNCTION("opencl_printf",opencl_printf)
    REGISTER_BI_FUNCTION("opencl_snprintf",opencl_snprintf)
    REGISTER_BI_FUNCTION("__opencl_dbg_declare_local",__opencl_dbg_declare_local)
    REGISTER_BI_FUNCTION("__opencl_dbg_enter_function",__opencl_dbg_enter_function)
    REGISTER_BI_FUNCTION("__opencl_dbg_exit_function",__opencl_dbg_exit_function)
    REGISTER_BI_FUNCTION("__opencl_dbg_declare_global",__opencl_dbg_declare_global)
    REGISTER_BI_FUNCTION("__opencl_dbg_stoppoint",__opencl_dbg_stoppoint)
    REGISTER_BI_FUNCTION("ocl20_get_default_queue",ocl20_get_default_queue)
    REGISTER_BI_FUNCTION("ocl20_enqueue_kernel_basic",ocl20_enqueue_kernel_basic)
    REGISTER_BI_FUNCTION("ocl20_enqueue_kernel_localmem",ocl20_enqueue_kernel_localmem)
    REGISTER_BI_FUNCTION("ocl20_enqueue_kernel_events",ocl20_enqueue_kernel_events)
    REGISTER_BI_FUNCTION("ocl20_enqueue_kernel_events_localmem",ocl20_enqueue_kernel_events_localmem)
    REGISTER_BI_FUNCTION("ocl20_enqueue_marker",ocl20_enqueue_marker)
    REGISTER_BI_FUNCTION("ocl20_retain_event",ocl20_retain_event)
    REGISTER_BI_FUNCTION("ocl20_release_event",ocl20_release_event)
    REGISTER_BI_FUNCTION("ocl20_create_user_event",ocl20_create_user_event)
    REGISTER_BI_FUNCTION("ocl20_set_user_event_status",ocl20_set_user_event_status)
    REGISTER_BI_FUNCTION("ocl20_capture_event_profiling_info",ocl20_capture_event_profiling_info)
    REGISTER_BI_FUNCTION("ocl20_get_kernel_wg_size",ocl20_get_kernel_wg_size)
    REGISTER_BI_FUNCTION("ocl20_get_kernel_preferred_wg_size_multiple",ocl20_get_kernel_preferred_wg_size_multiple)
    REGISTER_BI_FUNCTION("ocl20_is_valid_event",ocl20_is_valid_event)
    REGISTER_BI_FUNCTION("__emutls_get_address",__opencl_emutls_get_address)
}
