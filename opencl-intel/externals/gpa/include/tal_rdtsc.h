/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2010).  All rights reserved.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS LICENSED
** ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
** INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY UPDATES,
** ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
** MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
** OTHER WARRANTY.  Intel disclaims all liability, including liability for
** infringement of any proprietary rights, relating to use of the code. No license,
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
#elif TAL_PLATFORM == TAL_PLATFORM_LRB
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
#elif TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#ifdef TAL_64
    unsigned __int64 __rdtsc(void);
    #pragma intrinsic(__rdtsc)
    #define _TAL_RDTSC __rdtsc
#elif defined(TAL_32)
    #define _TAL_RDTSC_STACK(ts)                    \
    __asm rdtsc \
    __asm mov DWORD PTR [ts], eax \
    __asm mov DWORD PTR [ts+4], edx

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
        __asm__ __volatile__("rdtsc" : "=A" (result));
#else // TAL_64
        do {                                                        \
            unsigned int __a,__d;                                             \
            __asm volatile("rdtsc" : "=a" (__a), "=d" (__d));                   \
            (result) = ((unsigned long)__a) | (((unsigned long)__d)<<32);        \
        } while(0);
#endif
        return result;
    }
#define _TAL_RDTSC TAL_Rdtsc32
#else
#error ERROR: Unsupported platform
#endif 

TAL_INLINE TAL_UINT64 TAL_GetCurrentTime_Internal(void)
{
    return _TAL_RDTSC();
}

#ifdef __cplusplus
} // extern "C"
#endif
#endif // ndef TALX_NO_DOX
#endif /* TAL_RDTSC_H */

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
