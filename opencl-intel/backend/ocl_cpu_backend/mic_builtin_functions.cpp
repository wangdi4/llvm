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


#include "Executable.h"
#include "cpu_dev_limits.h"

#include <cassert>
#include <string>
#include <string.h>
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

// macro will return the address of instruction which will be 
// run after return from current function
// reason for using macro instead of inline function is
// that inlined function returns address from caller function of inlined function
// not the caller function of function which invoked inlined function
// using macro makes sure correct _ReturnAddress of caller will be obtained
#if defined(_WIN32)
#define GET_RET_ADDR() ((size_t)_ReturnAddress())
#else
#define GET_RET_ADDR() ((size_t) __builtin_return_address(0))
#endif


/*****************************************************************************************************************************
*    Synchronization functions (Section 6.11.9)
*****************************************************************************************************************************/
extern "C" LLVM_BACKEND_API void mic_dbg_print(const char* fmt, ...)
{
  assert(false && "Need to Implement mic_dbg_print");
}

typedef size_t event_t;
extern "C" LLVM_BACKEND_API void shared_lwait_group_events(int num_events, event_t *event_list, Executable* pExec);

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t mic_lasync_wg_copy_l2g(char* pDst, char* pSrc, size_t numElem, event_t event,
                                size_t elemSize, Executable* pExec) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t mic_lasync_wg_copy_l2g(char* pDst, char* pSrc, size_t numElem, event_t event,
                                size_t elemSize, Executable* pExec)
{
  assert(pExec && "Invalid context pointer");
  
  // make event ID as instruction address in caller function that
  // will be executed after built-in returns
  
  // purpose of int_event is to handle situations when input event is not zero
  // this means that some previous async_copy call created event
  // we need to make sure that current async_copying was done and
  // not only async_copying that created event
  event_t int_event = GET_RET_ADDR();
  
  // if input event is zero create event from internal event
  if ( 0 == event )
  {
    event = int_event;
  }

  // Check if copy is required for this invokation of BI
  if ( !pExec->SetAndCheckAsyncCopy(int_event) )
    return event;

  size_t  uiBytesToCopy = numElem*elemSize;

  // use memcpy
  std::copy(pSrc, pSrc + uiBytesToCopy, pDst);
  //memcpy(pDst, pSrc, uiBytesToCopy);

  return event;
}

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t mic_lasync_wg_copy_g2l(char* pDst, char* pSrc, size_t numElem, event_t event,
                              size_t elemSize, Executable* pExec) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t mic_lasync_wg_copy_g2l(char* pDst, char* pSrc, size_t numElem, event_t event,
                              size_t elemSize, Executable* pExec)
{
  assert(pExec && "Invalid context pointer");

  // make event ID as instruction address in caller function that
  // will be executed after built-in returns
  
  // purpose of int_event is to handle situations when input event is not zero
  // this means that some previous async_copy call created event
  // we need to make sure that current async_copying was done and
  // not only async_copying that created event
  event_t int_event = GET_RET_ADDR();
  
  // if input event is zero create event from internal event
  if ( 0 == event )
  {
    event = int_event;
  }

  // Check if copy is required for this invokation of BI
  if ( !pExec->SetAndCheckAsyncCopy(int_event) )
    return event;

  size_t  uiBytesToCopy = numElem*elemSize;

  // use memcpy
  std::copy(pSrc, pSrc + uiBytesToCopy, pDst);
  //memcpy(pDst, pSrc, uiBytesToCopy);
  return event;
}

extern "C" LLVM_BACKEND_API unsigned long long mic_get_time_counter()
{
  assert(false && "Need to Implement mic_get_time_counter");
  return 0;
}

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API int opencl_mic_printf(const char* format, char* args, Executable* pExec);
extern "C" LLVM_BACKEND_API int opencl_snprintf(char* outstr, size_t size, const char* format, char* args, Executable* pExec);

// New functions for 1.1
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t shared_lasync_wg_copy_strided_l2g(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event, size_t elemSize, Executable* pExec) ;

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t shared_lasync_wg_copy_strided_g2l(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,size_t elemSize, Executable* pExec);

void RegisterMICBIFunctions(std::map<std::string, unsigned long long int>& functionsTable)
{
    functionsTable["dbg_print"] = (unsigned long long int)(intptr_t)mic_dbg_print;
    functionsTable["lwait_group_events"] = (unsigned long long int)(intptr_t)shared_lwait_group_events;
    functionsTable["lasync_wg_copy_l2g"] = (unsigned long long int)(intptr_t)mic_lasync_wg_copy_l2g;
    functionsTable["lasync_wg_copy_g2l"] = (unsigned long long int)(intptr_t)mic_lasync_wg_copy_g2l;
    functionsTable["get_time_counter"] = (unsigned long long int)(intptr_t)mic_get_time_counter;
    functionsTable["lasync_wg_copy_strided_l2g"] = (unsigned long long int)(intptr_t)shared_lasync_wg_copy_strided_l2g;
    functionsTable["lasync_wg_copy_strided_g2l"] = (unsigned long long int)(intptr_t)shared_lasync_wg_copy_strided_g2l;
    functionsTable["opencl_printf"] = (unsigned long long int)(intptr_t)opencl_mic_printf;
    functionsTable["opencl_snprintf"] = (unsigned long long int)(intptr_t)opencl_snprintf;
}
