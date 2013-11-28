/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  builtin_functions.cpp

\*****************************************************************************/


#include "ExecutionContext.h"
#include "cpu_dev_limits.h"

#include <assert.h>
#include <string>
#include <map>
#include <algorithm>


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
extern "C" LLVM_BACKEND_API void mic_dbg_print(const char* fmt, ...)
{
  assert(false && "Need to Implement mic_dbg_print");
}

// set the rounding mode to the given one, and return the previous mode.
extern "C" LLVM_BACKEND_API unsigned int set_rounding_mode(unsigned int mode)
{
    unsigned int old_csr = _mm_getcsr();
    _mm_setcsr((old_csr & ~_MM_ROUND_MASK) | mode);
    return old_csr & _MM_ROUND_MASK;
}

extern "C" LLVM_BACKEND_API unsigned long long mic_get_time_counter()
{
  assert(false && "Need to Implement mic_get_time_counter");
  return 0;
}

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API int opencl_mic_printf(const char* format, char* args, ICLDevBackendDeviceAgentCallback* pCallback, void* pHandle);
extern "C" LLVM_BACKEND_API int opencl_snprintf(char* outstr, size_t size, const char* format, char* args, ICLDevBackendDeviceAgentCallback* pCallback, void* pHandle);

void RegisterMICBIFunctions(std::map<std::string, unsigned long long int>& functionsTable)
{
    functionsTable["dbg_print"] = (unsigned long long int)(intptr_t)mic_dbg_print;
    functionsTable["get_time_counter"] = (unsigned long long int)(intptr_t)mic_get_time_counter;
    functionsTable["opencl_printf"] = (unsigned long long int)(intptr_t)opencl_mic_printf;
    functionsTable["opencl_snprintf"] = (unsigned long long int)(intptr_t)opencl_snprintf;
    functionsTable["set_rounding_mode"] = (unsigned long long int)(intptr_t)set_rounding_mode;
}
