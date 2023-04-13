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
#ifndef TAL_PLATFORMS
#define TAL_PLATFORMS

/* Set of TAL build configuration
TAL_DISABLE - end user set to cause all TAL macros to compile out.
TAL_STATIC vs TAL_WEAK -- Set for build on TAL_WEAK indicates that capture
library is seperate and will be loaded at run time. TAL_64 vs TAL_32 -- is the
platform a 32bit or 64bit TAL_PLATFORM -- What the target platform is set to one
of the TAL_PLATFORM_* macros TAL_DEBUG -- If this is a debug build of tal.
TAL_KERNEL -- If this is a kernel build of TAL
TAL_COMPILER -- The compiler used in this build
 * TAL_FEATURE_* -- Enable flags for various tal features may be defined in
project
 *      or some are automatically enabled for platforms/OSs at the bottom of
this file
 *      -- TAL_FEATURE_LRB -- Enable lrb features Thread affinity, ring buffer
transports, etc
 *      -- TAL_FEATURE_STATIC_PUSH_FUNCS -- enables linked verison of the static
TAL pushers
*/

// This option uses pragma mesages
// #define TAL_PRINT_CONFIG

#define TAL_PLATFORM_WINDOWS 1
#define TAL_PLATFORM_LARRYSIM 2
#define TAL_PLATFORM_NIX 4 // freebsd,linux,etc

#undef TAL_64
#undef TAL_32

#ifndef TAL_DISABLE
#define TAL_ENABLE
#endif // !defined(TAL_DISABLE)

#define TAL_COMPILER_MSVC 10
#define TAL_COMPILER_ICC 20
#define TAL_COMPILER_GCC 30

#if defined(__INTEL_COMPILER)
#define TAL_COMPILER TAL_COMPILER_ICC
#elif defined(__GNUC__)
#define TAL_COMPILER TAL_COMPILER_GCC
#elif defined(_MSC_VER)
#define TAL_COMPILER TAL_COMPILER_MSVC
#else
#error Unrecognized compiler
#endif

/* ************************************************************************* **
** ************************************************************************* **
** Platform Auto Detection
** ************************************************************************* **
** ************************************************************************* */
#if !defined(TAL_PLATFORM)
#if defined(WIN32) || defined(_WIN32) || defined(_MSC_VER)
#define TAL_PLATFORM TAL_PLATFORM_WINDOWS
#ifdef _M_X64
#define TAL_64
#else // !WIN64
#define TAL_32
#endif // !WIN64

#if defined(DEBUG) || defined(_DEBUG)
#define TAL_DEBUG 1
#endif

#ifdef __cplusplus
#define TAL_INLINE inline
#define TAL_CONST const
#else
#define TAL_INLINE __inline
#define TAL_CONST
#endif
#define TAL_CALL __cdecl
#define TAL_THREAD_LOCAL __declspec(thread)
#ifndef TAL_DOXYGEN
// windows.h cannot be included in public headers.
// as it collides and breaks ddk(umd/kmd) builds on windows.
// If I catch you including windows.h in to the tal public headers, I will ruin
// your day. :)
// #define VC_EXTRA_LEAN
// #ifndef _WIN32_WINNT
//  #define _WIN32_WINNT 0x0501
// #endif // ndef(_WIN32_WINNT)
// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>
#endif // TAL_DOXYGEN

#if defined(_KERNEL)
#define TAL_KERNEL
#endif

// FIXME better detection
#define TAL_FEATURE_LARRABEE

#elif defined(                                                                 \
    __INTEL_COMPILER) // todo bi3151744: improve detection of LRB so that folks
                      // using ICC on *Nix don't also get stuck with LRB impl
#define TAL_PLATFORM TAL_PLATFORM_NIX
#define TAL_64

#if defined(DEBUG) || defined(_DEBUG)
#define TAL_DEBUG 1
#endif

#if defined(_KERNEL) || defined(LRB_KMD)
#define TAL_KERNEL
#endif

#define TAL_INLINE inline
#define TAL_CONST const
#define TAL_CALL
#define TAL_THREAD_LOCAL __declspec(thread)

// FIXME better detection
#define TAL_FEATURE_LARRABEE
#elif defined(__GNUC__)
#define TAL_PLATFORM TAL_PLATFORM_NIX
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||           \
    defined(__x86_64)
#define TAL_64
#else
#define TAL_32
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define TAL_DEBUG 1
#endif

#if defined(_KERNEL) || defined(LRB_KMD)
#define TAL_KERNEL
#endif

#define TAL_INLINE static inline
#define TAL_CONST const
#define TAL_CALL
#define TAL_THREAD_LOCAL __thread

#define _FARQ
#define _SIZT size_t
#define _PDFT ptrdiff_t

#define _THROWS(x)
#define _TRY_BEGIN try {
#define _CATCH(x)                                                              \
  }                                                                            \
  catch (x) {
#define _CATCH_ALL                                                             \
  }                                                                            \
  catch (...) {
#define _CATCH_END }
#define _RAISE(x) throw x
#define _RERAISE throw
#define _THROW0() throw()
#define _THROW1(x) throw(...)
#define _THROW(x, y) throw x(y)
#define _THROW_NCEE(x, y) _THROW(x, y)

#else
#error Unable to auto detect TAL platform!
#endif // Tal Platform Detection
#endif // !defined(TAL_PLATFORM)

#if !defined(TAL_PLATFORM)
#error "No TAL platform defined."
#endif // TAL_PLATFORM == TAL_PLATFORM_UNKNOWN

// Enabled TAL features that are default on different OSs(Platforms)
#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#ifndef TAL_ENABLE_LRB
#define TAL_ENABLE_LRB
#endif // TAL_ENABLE_LRB
#elif TAL_PLATFORM == TAL_PLATFORM_NIX

#endif // TAL_PLATFORM

#ifdef TAL_PRINT_CONFIG

#if defined(TAL_DISABLE)
#pragma message("\tTAL_DISABLE")
#elif defined(TAL_ENABLE)
#pragma message("\tTAL_ENABLE")
#else //
#pragma message(                                                               \
    "\tTAL is neither enabled or disabled (TAL_DISABLE | TAL_DISABLE)")
#endif //

#if TAL_PLATFORM == TAL_PLATFORM_UNKNOWN
#pragma message("\tTAL_PLATFORM_UNKNOWN")
#elif TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#pragma message("\tTAL_PLATFORM_WINDOWS")
#elif TAL_PLATFORM == TAL_PLATFORM_LARRYSIM
#pragma message("\tTAL_PLATFORM_LARRYSIM")
#elif TAL_PLATFORM == TAL_PLATFORM_NIX // freebsd,linux,etc
#pragma message("\tTAL_PLATFORM_NIX")
#else // TAL_PLATFORM
#pragma message("\tTAL_PLATFORM UNDEFINED")
#endif // TAL_PLATFORM

#if defined(TAL_64)
#pragma message("\tTAL_64")
#elif defined(TAL_32)
#pragma message("\tTAL_32")
#else //
#pragma message("TAL bitness not defined (TAL_64|TAL_32)")
#endif //

#if TAL_COMPILER == TAL_COMPILER_MSVC
#pragma message("\tTAL_COMPILER_MSVC")
#elif TAL_COMPILER == TAL_COMPILER_ICC
#pragma message("\tTAL_COMPILER_ICC")
#elif TAL_COMPILER == TAL_COMPILER_GCC
#pragma message("\tTAL_COMPILER_GCC")
#else //
#pragma message("\tTAL_COMPILER Undefinded.")
#endif // TAL_PLATFORM

#if defined(TAL_KERNEL)
#pragma message("\tTAL_KERNEL.")
#endif // defined(TAL_KERNEL)

#if defined(TAL_DEBUG)
#pragma message("\tTAL_DEBUG.")
#endif // defined(TAL_DEBUG)

// FIXME Add enables

#endif // TAL_PRINT_CONFIG

#define TAL_ON_LRB                                                             \
  (defined(TAL_FEATURE_LARRABEE) && (TAL_PLATFORM == TAL_PLATFORM_NIX))
#define TAL_SUPPORT_LRB                                                        \
  (defined(TAL_FEATURE_LARRABEE) && (TAL_PLATFORM == TAL_PLATFORM_WINDOWS))

#endif // TAL_PLATFORMS
/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
