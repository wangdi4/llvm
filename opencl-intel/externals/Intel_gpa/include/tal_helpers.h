/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2010).  All rights reserved.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
*LICENSED
** ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
** INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY
*UPDATES,
** ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
** MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
** OTHER WARRANTY.  Intel disclaims all liability, including liability for
** infringement of any proprietary rights, relating to use of the code. No
*license,
** express or implied, by estoppel or otherwise, to any intellectual property
** rights is granted herein.
**
** Purpose:   This file contains helper functions that are used to implement
**            inline functions within the TaskAnalyzer APIs.
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef TAL_HELPERS_H
#define TAL_HELPERS_H

#include "tal_rdtsc.h"
#include "tal_types.h"

#ifdef TAL_DISABLE

#define TAL_GetCurrentTime() 0
#define TAL_AtomicAdd32(addr, amt)
#define TAL_ReadHardwareCounter32(ctr) 0xFFFFFFFF

#define TAL_AtomicExchange(x, y) 0
#define TAL_AtomicIncrement(x) 0
#define TAL_AtomicDecrement(x) 0
#define TAL_AtomicCompareAndSwap(x, y, z) 0

#else // !TAL_DISABLE

/** \ingroup Basics
 ** Gets the current timestamp as seen by TAL.
 ** The TAL_GetCurrentTime() function is used by all TAL_ trace functions to
 *obtain
 ** the current processor time. Typically, this is a wrapper around the rdtsc
 *intrinsic, although
 ** this is completely up to the TAL implementation to decide.
 ** It is critical that this function be used anytime you hand-timestamp an
 *event with the intent
 ** of tracing it later using an Ex variant of the TAL tracing function. TAL
 *ensures that
 ** the time TAL receives is in the clock domain expected by TAL.
 **/
TAL_INLINE TAL_UINT64 TAL_GetCurrentTime() {
#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS && defined(USE_QPC_ON_WIN32)
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return t.QuadPart;
#else
  return TAL_GetCurrentTime_Internal();
#endif // TAL_PLATFORM
}

#ifndef TAL_DOXYGEN
#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#pragma warning(                                                               \
    disable : 4995) // 'XXX' was name was marked as #pragma deprecated
#include <intrin.h>
#include <math.h> // Magical fix for x64 debug compile issue -- warning C4985: 'ceil': attributes not present on previous declaration.
#endif            // TAL_PLATFORM
#endif            // ndef TAL_DOXYGEN

#ifndef TAL_DOXYGEN
TAL_INLINE void TAL_AtomicAdd32(volatile TAL_UINT32 *addr, int amt) {
#if TAL_PLATFORM == TAL_PLATFORM_LARRYSIM
#warning not supported
#elif TAL_PLATFORM == TAL_PLATFORM_WINDOWS
  _InterlockedExchangeAdd((long *)addr, amt);
#elif TAL_PLATFORM == TAL_PLATFORM_LRB // icc/msvc syntax
  int32_t result;
  __asm {
      lea         rcx, addr
      mov         rcx, [rcx]
      mov         eax, amt
      lock add        dword ptr [rcx], eax
  }
#elif TAL_PLATFORM == TAL_PLATFORM_NIX // gcc syntax
  __sync_add_and_fetch(addr, amt);
#endif                                 // TAL_PLATFORM
}
#endif                                 // ndef TAL_DOXYGEN

// inline TAL_UINT32 TAL_AtomicExchange(volatile TAL_UINT32* addr, TAL_UINT32
// val) {   return TAL_AtomicExchange((volatile TAL_UINT32*)addr, val);
// }

// inline TAL_UINT32 TAL_AtomicIncrement(volatile TAL_UINT32* addr) {
//  return TAL_AtomicIncrement((volatile TAL_*)addr);
// }

// inline TAL_UINT32 TAL_AtomicDecrement(volatile TAL_UINT32* addr) {
//  return TAL_AtomicDecrement((volatile TAL_INT32*)addr);
// }

// inline TAL_UINT32 TAL_AtomicCompareAndSwap(volatile TAL_UINT32* addr,
// TAL_UINT32 newval, TAL_UINT32 refvalue) {   return
// TAL_AtomicCompareAndSwap((volatile TAL_INT32*)addr, newval, refvalue);
// }

#ifndef TAL_DOXYGEN
#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#if defined(TAL_32) &&                                                         \
    !defined(_MANAGED) /* The asm really pisses off the clr compiler */
// (Vista32)
TAL_INLINE TAL_INT32 TAL_AtomicExchange(TAL_UINT32 volatile *in_pDestination,
                                        TAL_UINT32 in_exchange) {
  TAL_UINT32 result;
  __asm {
        mov         ecx, dword ptr [in_pDestination] 
        mov         edx, dword ptr [in_exchange] 
        mov         eax, dword ptr [ecx] 
    vbxcvb:
        lock cmpxchg     dword ptr [ecx], edx 
        jne         vbxcvb
        mov         result, eax
  }
  return result;
}

// (Vista32)
TAL_INLINE TAL_UINT32 TAL_AtomicIncrement(TAL_UINT32 volatile *in_pAddend) {
  TAL_UINT32 result;
  __asm {
        mov         ecx, dword ptr [in_pAddend]
        mov         eax, 1
        lock xadd        dword ptr [ecx], eax 
        inc         eax
        mov         result, eax
  }
  return result;
}

// (Vista32)
TAL_INLINE TAL_UINT32 TAL_AtomicDecrement(TAL_UINT32 volatile *in_pAddend) {
  TAL_UINT32 result;
  __asm {
        mov         ecx, dword ptr [in_pAddend]
        mov         eax, -1
        lock xadd        [ecx], eax
        dec         eax
        mov         result, eax
  }
  return result;
}

// (Vista32)
TAL_INLINE TAL_UINT32
TAL_AtomicCompareAndSwap(TAL_UINT32 volatile *in_pDestination,
                         TAL_UINT32 in_exchange, TAL_UINT32 in_comperand) {
  TAL_UINT32 result;
  __asm {
        mov         ecx,dword ptr [in_pDestination]
        mov         edx, in_exchange
        mov         eax, in_comperand
       lock cmpxchg     dword ptr [ecx], edx
        mov         result, eax
  }
  return result;
}

// (Vista32)
TAL_INLINE TAL_UINT32 TAL_AtomicExchangeAdd(
    TAL_UINT32 volatile *in_pDestination, TAL_UINT32 in_value) {
  TAL_UINT32 result;
  __asm {
        mov         ecx, dword ptr [in_pDestination]
        mov         edx, in_value
        mov         eax, dword ptr [ecx]
       lock xadd        dword ptr [ecx], edx
        mov         result, eax
  }
  return result;
}
#else                  // defined(TAL_64) || defined(_MANAGED)
                       // (Vista64)
TAL_INLINE TAL_UINT32 TAL_AtomicExchange(TAL_UINT32 volatile *in_pDestination,
                                         TAL_UINT32 in_exchange) {
  return _InterlockedExchange((long *)in_pDestination, in_exchange);
}

// (Vista64)
TAL_INLINE TAL_UINT32 TAL_AtomicIncrement(TAL_UINT32 volatile *in_pAddend) {
  return _InterlockedIncrement((long *)in_pAddend);
}

// (Vista64)
TAL_INLINE TAL_UINT32 TAL_AtomicDecrement(TAL_UINT32 volatile *in_pAddend) {
  return _InterlockedDecrement((long *)in_pAddend);
}

TAL_INLINE TAL_UINT32 TAL_AtomicExchangeAdd(
    TAL_UINT32 volatile *in_pDestination, TAL_UINT32 in_value) {
  return _InterlockedExchangeAdd((long *)in_pDestination, in_value);
}

// (Vista64)
TAL_INLINE TAL_UINT32
TAL_AtomicCompareAndSwap(TAL_UINT32 volatile *in_pDestination,
                         TAL_UINT32 in_exchange, TAL_UINT32 in_comperand) {
  return _InterlockedCompareExchange((long *)in_pDestination, in_exchange,
                                     in_comperand);
}
#endif                 // TAL_64

#elif TAL_PLATFORM == TAL_PLATFORM_LRB

// (BSD64)
TAL_INLINE TAL_UINT32 TAL_AtomicIncrement(TAL_UINT32 volatile *in_pAddend) {
  int32_t result;
  __asm {
      lea         rcx, in_pAddend
      mov         rcx, [rcx]
      mov         eax, 1
      lock xadd        dword ptr [rcx], eax
      inc         eax
      mov         result, eax
  }
  return result;
}

// (BSD64)
TAL_INLINE TAL_UINT32 TAL_AtomicDecrement(TAL_UINT32 volatile *in_pAddend) {
  int32_t result;
  __asm {
      lea         rcx, in_pAddend
      mov         rcx, [rcx]
      mov         eax, -1
     lock xadd        dword ptr [rcx], eax
      dec         eax
      mov         result, eax
  }
  return result;
}

// (BSD64)
TAL_INLINE TAL_UINT32 TAL_AtomicExchange(TAL_UINT32 volatile *in_pDestination,
                                         TAL_UINT32 in_exchange) {
  int32_t result;
  __asm {
      lea         rcx, in_pDestination
      mov         rcx, [rcx]
      lea         rdx, in_exchange
      mov         edx, dword ptr [rdx]
      mov         eax, dword ptr [rcx]
  sdfsd:
     lock cmpxchg     dword ptr [rcx], edx
      jne         sdfsd
      mov         result, eax
  }
  return result;
}

// (BSD64)
TAL_INLINE TAL_UINT32
TAL_AtomicCompareAndSwap(TAL_UINT32 volatile *in_pDestination,
                         TAL_UINT32 in_exchange, TAL_UINT32 in_comperand) {
  int32_t result;
  __asm {
      lea         rcx, in_pDestination
      mov         rcx, [rcx]
      mov         edx, in_exchange
      mov         eax, in_comperand
     lock cmpxchg     dword ptr [rcx], edx
      mov         result, eax
  }
  return result;
}

#else
#error "Not defined for current platform"
#endif // TAL_PLATFORM
#endif // ndef TAL_DOXYGEN

#ifndef TAL_DOXYGEN
TAL_INLINE TAL_UINT32 TAL_ReadHardwareCounter32(int counter) {
#if TAL_PLATFORM == TAL_PLATFORM_LARRYSIM
#warning not supported
  counter;
  return 0xFFFFFFFF;
#elif TAL_PLATFORM == TAL_PLATFORM_WINDOWS
  counter;
  return 0xFFFFFFFF;
#elif TAL_PLATFORM == TAL_PLATFORM_LRB
  TAL_UINT32 result;
  __asm {
    mov ecx, counter;
    rdpmc;
    mov result, eax;
  }
  return result;
#elif TAL_PLATFORM == TAL_PLATFORM_NIX
  (void)counter;
  return 0xFFFFFFFF;
#endif
}
#endif

#endif //! TAL_DISABLE

#ifndef TAL_DOXYGEN
TAL_INLINE TAL_UINT32 TAL_Pad32(TAL_UINT32 val, TAL_UINT32 pad) {
  return ((val + pad - 1) / pad) * pad;
}
#endif

#ifdef __cplusplus
#ifndef TAL_DOXYGEN
template <class T> TAL_INLINE T TAL_Pad(T val, T pad) {
  return ((val + pad - 1) / pad) * pad;
}

template <class T, class U> TAL_INLINE T TAL_PadPtr(T *ptr, U pad) {
#ifdef TAL_64
  TAL_UINT64 val = reinterpret_cast<TAL_UINT64>(ptr);
  TAL_UINT64 newval = TAL_Pad(val, pad);
  return reinterpret_cast<T *>(newval);
#else  // TAL_32
  TAL_UINT32 val = reinterpret_cast<TAL_UINT32>(ptr);
  TAL_UINT32 newval = TAL_Pad(val, pad);
  return reinterpret_cast<T *>(newval);
#endif // TAL_32
}
#endif // ndef TAL_DOXYGEN

#endif // __cplusplus

#endif // TAL_HELPERS_H

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
