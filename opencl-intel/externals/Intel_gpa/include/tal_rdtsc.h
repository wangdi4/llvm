/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) 2010, Intel Corporation. All rights reserved.
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
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef TAL_RDTSC_H
#define TAL_RDTSC_H
#ifndef TAL_DOXYGEN
#ifdef __cplusplus
extern "C" {
#endif

#if defined(LARRYSIM)
// LarrySim impl doesn't use this inline
#elif TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#ifdef TAL_64
unsigned __int64 __rdtsc(void);
#pragma intrinsic(__rdtsc)
#define _TAL_RDTSC __rdtsc
#elif defined(TAL_32)
#define _TAL_RDTSC_STACK(ts)                                                   \
  __asm rdtsc __asm mov DWORD PTR[ts], eax __asm mov DWORD PTR[ts + 4], edx

__inline unsigned __int64 TAL_Rdtsc32() {
  unsigned __int64 t;
  _TAL_RDTSC_STACK(t);
  return t;
}
#define _TAL_RDTSC TAL_Rdtsc32
#endif
#elif TAL_PLATFORM == TAL_PLATFORM_NIX
TAL_INLINE TAL_UINT64 TAL_Rdtsc32(void) {
  TAL_UINT64 result;
#ifdef TAL_32
  __asm__ __volatile__("rdtsc" : "=A"(result));
#else // TAL_64
  do {
    unsigned int __a, __d;
    __asm volatile("rdtsc" : "=a"(__a), "=d"(__d));
    (result) = ((unsigned long)__a) | (((unsigned long)__d) << 32);
  } while (0);

  /* FIXME is this code identical or do we need a lrb case?
      #define _TAL_RDTSC_STACK(ts) \
      __asm rdtsc \
      __asm mov DWORD PTR [ts], eax \
      __asm mov DWORD PTR [ts+4], edx

      __inline unsigned __int64 TAL_Rdtsc32() {
              unsigned __int64 t;
              _TAL_RDTSC_STACK(t);
              return t;
      }
      #define _TAL_RDTSC TAL_Rdtsc32
*/
#endif
  return result;
}
#define _TAL_RDTSC TAL_Rdtsc32
#else
#error ERROR: Unsupported platform
#endif

TAL_INLINE TAL_UINT64 TAL_GetCurrentTime_Internal(void) { return _TAL_RDTSC(); }

#ifdef __cplusplus
} // extern "C"
#endif
#endif // ndef TALX_NO_DOX

#ifdef TAL_DISABLE

#define TAL_GetCurrentTime() 0

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

#endif // TAL_DISABLE

#endif /* TAL_RDTSC_H */

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
