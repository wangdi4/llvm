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

#include "Binary.h"
#include "Executable.h"
#include "cpu_dev_limits.h"
#include "SystemInfo.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/System/DataTypes.h"

#include <stdio.h>
#include <setjmp.h>

#ifdef _WIN32
  #include <smmintrin.h>
#else
  #ifdef __SSE4_1__
    #include <smmintrin.h>
  #endif
#endif

#include <cassert>
#include <stdarg.h>
#include <string.h>

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
extern "C" LLVM_BACKEND_API void dbg_print(const char* fmt, ...)
{
  va_list va;
  va_start(va, fmt);

  vprintf(fmt, va);
  va_end( va );
}

typedef size_t event_t;
extern "C" LLVM_BACKEND_API void lwait_group_events(int num_events, event_t *event_list, Executable* pExec)
{
  assert(pExec && "Invalid context pointer");

  bool bBarrier = false;
  for (int i=0; i<num_events; ++i)
  {
    bBarrier |= pExec->ResetAsyncCopy(event_list[i]);
  }

  if ( bBarrier )
  {
    assert( false && "lwait_group_events needs to call a barrier!" );
    //lbarrier(0, pExec);
  }
}
// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_l2g(char* pDst, char* pSrc, size_t numElem, event_t event,
                                size_t elemSize, Executable* pExec) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_l2g(char* pDst, char* pSrc, size_t numElem, event_t event,
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
  bool bUseSSE = (!(((size_t)pDst) & 0xF)) && (!((uiBytesToCopy) & 0xF));
  if ( bUseSSE )
  {
    for (unsigned int i=0; i<uiBytesToCopy; i+=16)
    {
#ifdef __SSE4_1__
      __m128i  xmmTmp = _mm_lddqu_si128((__m128i*)(pSrc+i));
      _mm_stream_si128((__m128i*)(pDst+i), xmmTmp);      // TODO: check performance implication of streaming instruction
#else
      __m128i  xmmTmp = _mm_load_si128((__m128i*)(pSrc+i));
      _mm_store_si128((__m128i*)(pDst+i), xmmTmp);      // TODO: check performance implication of streaming instruction
#endif
      
    }
    return event;
  }

  // else use memcpy
  memcpy(pDst, pSrc, uiBytesToCopy);

  return event;
}

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_g2l(char* pDst, char* pSrc, size_t numElem, event_t event,
                              size_t elemSize, Executable* pExec) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_g2l(char* pDst, char* pSrc, size_t numElem, event_t event,
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
  bool bUseSSE = (!(((size_t)pDst) & 0xF)) && (!(((size_t)pSrc) & 0xF)) && (!((uiBytesToCopy) & 0xF));
  if ( bUseSSE )
  {
    for (unsigned int i=0; i<uiBytesToCopy; i+=16)
    {
#ifdef __SSE4_1__
      __m128i  xmmTmp = _mm_stream_load_si128((__m128i*)(pSrc+i)); // TODO: check performance implication of streaming instruction
#else
      __m128i  xmmTmp = _mm_load_si128((__m128i*)(pSrc+i));
#endif
      _mm_store_si128((__m128i*)(pDst+i), xmmTmp);
    }
    return event;
  }

  // else use memcpy
  memcpy(pDst, pSrc, uiBytesToCopy);
  return event;
}

extern "C" LLVM_BACKEND_API void lprefetch(const char* ptr, size_t numElements, size_t elmSize)
{
  size_t totalLines = ((numElements * elmSize) + CPU_DEV_DCU_LINE_SIZE - 1) / CPU_DEV_DCU_LINE_SIZE;

  for (size_t i=0; i<totalLines; ++i)
  {
    _mm_prefetch(ptr, _MM_HINT_T0);
    ptr += CPU_DEV_DCU_LINE_SIZE;
  }
}

extern "C" LLVM_BACKEND_API unsigned long long get_time_counter()
{
  return Intel::OpenCL::DeviceBackend::Utils::SystemInfo::HostTime();
}

// New functions for 1.1
// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_strided_l2g(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
                                size_t elemSize, Executable* pExec) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_strided_l2g(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
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

  switch (elemSize)
  {
  case 1:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_char*)pDst)[i*stride] = ((cl_char*)pSrc)[i];
    }
    break;
  case 2:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_short*)pDst)[i*stride] = ((cl_short*)pSrc)[i];
    }
    break;
  case 3:
  case 4:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_int*)pDst)[i*stride] = ((cl_int*)pSrc)[i];
    }
    break;
  case 8:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_long*)pDst)[i*stride] = ((cl_long*)pSrc)[i];
    }
    break;
  default:
    for(unsigned int i=0; i<numElem; ++i)
    {
      memcpy(pDst, pSrc, elemSize);
      pSrc += elemSize;
      pDst += stride*elemSize;
    }
  }

  return event;
}
// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_strided_g2l(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
                              size_t elemSize, Executable* pExec) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t lasync_wg_copy_strided_g2l(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
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

  switch (elemSize)
  {
  case 1:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_char*)pDst)[i] = ((cl_char*)pSrc)[i*stride];
    }
    break;
  case 2:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_short*)pDst)[i] = ((cl_short*)pSrc)[i*stride];
    }
    break;
  case 3:
  case 4:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_int*)pDst)[i] = ((cl_int*)pSrc)[i*stride];
    }
    break;
  case 8:
    for(unsigned int i=0; i<numElem; ++i)
    {
      ((cl_long*)pDst)[i] = ((cl_long*)pSrc)[i*stride];
    }
    break;
  default:
    for(unsigned int i=0; i<numElem; ++i)
    {
      memcpy(pDst, pSrc, elemSize);
      pSrc += stride*elemSize;
      pDst += elemSize;
    }
  }

  return event;
}

// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API int opencl_printf(const char* format, char* args, Executable* pExec);
extern "C" LLVM_BACKEND_API int opencl_snprintf(char* outstr, size_t size, const char* format, char* args, Executable* pExec);
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_declare_local(void* addr, uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_declare_global(void* addr, uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_stoppoint(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_enter_function(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void __opencl_dbg_exit_function(uint64_t metadata_addr, uint64_t gid0, uint64_t gid1, uint64_t gid2) LLVM_BACKEND_NOINLINE_POST;

//Register BI functions defined above
#define REGISTER_BI_FUNCTION(name,ptr) \
    llvm::sys::DynamicLibrary::AddSymbol(name, (void*)(intptr_t)ptr);
void RegisterBIFunctions(void)
{
    REGISTER_BI_FUNCTION("dbg_print",dbg_print)
    REGISTER_BI_FUNCTION("lwait_group_events",lwait_group_events)
    REGISTER_BI_FUNCTION("lasync_wg_copy_l2g",lasync_wg_copy_l2g)
    REGISTER_BI_FUNCTION("lasync_wg_copy_g2l",lasync_wg_copy_g2l)
    REGISTER_BI_FUNCTION("lprefetch",lprefetch)
    REGISTER_BI_FUNCTION("get_time_counter",get_time_counter)
    REGISTER_BI_FUNCTION("lasync_wg_copy_strided_l2g",lasync_wg_copy_strided_l2g)
    REGISTER_BI_FUNCTION("lasync_wg_copy_strided_g2l",lasync_wg_copy_strided_g2l)
    REGISTER_BI_FUNCTION("opencl_printf",opencl_printf)
    REGISTER_BI_FUNCTION("opencl_snprintf",opencl_snprintf)
    REGISTER_BI_FUNCTION("__opencl_dbg_declare_local",__opencl_dbg_declare_local)
    REGISTER_BI_FUNCTION("__opencl_dbg_enter_function",__opencl_dbg_enter_function)
    REGISTER_BI_FUNCTION("__opencl_dbg_exit_function",__opencl_dbg_exit_function)
    REGISTER_BI_FUNCTION("__opencl_dbg_declare_global",__opencl_dbg_declare_global)
    REGISTER_BI_FUNCTION("__opencl_dbg_stoppoint",__opencl_dbg_stoppoint)
}
