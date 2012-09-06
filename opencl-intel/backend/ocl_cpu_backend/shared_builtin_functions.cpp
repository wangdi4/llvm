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

File Name:  shared_builtin_functions.cpp

\*****************************************************************************/

#include "ExecutionContext.h"
#include "ICLDevBackendServiceFactory.h"
#include "cpu_dev_limits.h"

#include <algorithm>
#include <assert.h>

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

typedef size_t event_t;
extern "C" LLVM_BACKEND_API void shared_lwait_group_events(int num_events, event_t *event_list, CallbackContext* pContext)
{
  assert(pContext && "Invalid context pointer");

  bool bBarrier = false;
  for (int i=0; i<num_events; ++i)
  {
    bBarrier |= pContext->ResetAsyncCopy(event_list[i]);
  }

  if ( bBarrier )
  {
    assert( false && "lwait_group_events needs to call a barrier!" );
    //lbarrier(0, pContext);
  }
}

// New functions for 1.1
// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t shared_lasync_wg_copy_strided_l2g(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
                                size_t elemSize, CallbackContext* pContext) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t shared_lasync_wg_copy_strided_l2g(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
                                size_t elemSize, CallbackContext* pContext)
{
  assert(pContext && "Invalid context pointer");

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
  if ( !pContext->SetAndCheckAsyncCopy(int_event) )
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
      std::copy(pSrc, pSrc + elemSize, pDst);
      //memcpy(pDst, pSrc, elemSize);
      pSrc += elemSize;
      pDst += stride*elemSize;
    }
  }

  return event;
}
// usage of the function forward declaration prior to the function definition is because "__noinline__" attribute cannot appear with definition 
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t shared_lasync_wg_copy_strided_g2l(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
                              size_t elemSize, CallbackContext* pContext) LLVM_BACKEND_NOINLINE_POST;
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE event_t shared_lasync_wg_copy_strided_g2l(char* pDst, char* pSrc, size_t numElem, size_t stride, event_t event,
                              size_t elemSize, CallbackContext* pContext)
{
  assert(pContext && "Invalid context pointer");

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
  if ( !pContext->SetAndCheckAsyncCopy(int_event) )
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
      std::copy(pSrc, pSrc + elemSize, pDst);
      //memcpy(pDst, pSrc, elemSize);
      pSrc += stride*elemSize;
      pDst += elemSize;
    }
  }

  return event;
}

