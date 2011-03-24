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
#ifndef TAL_PLATFORMS
#define TAL_PLATFORMS


/* Set of TAL build configuration 
TAL_DISABLE - end user set to cause all TAL macros to compile out. 
TAL_STATIC vs TAL_WEAK -- Set for build on TAL_WEAK indicates that capture library is seperate and will be loaded at run time. 
TAL_64 vs TAL_32 -- is the platform a 32bit or 64bit
TAL_PLATFORM -- What the target platform is set to one of the TAL_PLATFORM_* macros
TAL_DEBUG -- If this is a debug build of tal. 
TAL_KERNEL -- If this is a kernel build of TAL
TAL_COMPILER -- The compiler used in this build
*/

#define	TAL_PLATFORM_UNKNOWN 0
#define	TAL_PLATFORM_WINDOWS 1
#define	TAL_PLATFORM_LARRYSIM 2
#define	TAL_PLATFORM_LRB 3
#define	TAL_PLATFORM_NIX 4 // freebsd,linux,etc

#undef  TAL_64
#undef  TAL_32

#ifndef TAL_DISABLE
    #define TAL_ENABLE
#endif // !defined(TAL_DISABLE)

#define TAL_COMPILER_MSVC 10
#define TAL_COMPILER_ICC 20
#define TAL_COMPILER_GCC 30

#if defined(__INTEL_COMPILER)
#  define TAL_COMPILER TAL_COMPILER_ICC
#elif defined(__GNUC__)
#  define TAL_COMPILER TAL_COMPILER_GCC
#elif defined(_MSC_VER)
# define TAL_COMPILER  TAL_COMPILER_MSVC
#else
# error Unrecognized compiler
#endif

/* ************************************************************************* **
** ************************************************************************* **
** Platform Auto Detection
** ************************************************************************* **
** ************************************************************************* */
#if !defined(TAL_PLATFORM)
    #if defined( WIN32 ) || defined( _WIN32 )||defined(_MSC_VER)
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
        #ifndef TAL_DOXYGEN
            // windows.h cannot be included in public headers. 
            // as it collides and breaks ddk(umd/kmd) builds on windows.  
			// If I catch you including windows.h in to the tal public headers, I will ruin your day. :)
			//#define VC_EXTRA_LEAN
			//#ifndef _WIN32_WINNT 
			//	#define _WIN32_WINNT 0x0501 
			//#endif // ndef(_WIN32_WINNT)
            //#define WIN32_LEAN_AND_MEAN
            //#include <windows.h>
        #endif // TAL_DOXYGEN

#if defined(_KERNEL)
#define TAL_KERNEL
#endif

#elif defined(__INTEL_COMPILER) // todo bi3151744: improve detection of LRB so that folks using ICC on *Nix don't also get stuck with LRB impl
#define TAL_PLATFORM TAL_PLATFORM_LRB
#define TAL_64

#if defined(DEBUG) || defined(_DEBUG)
#define TAL_DEBUG 1
#endif 

#define TAL_INLINE inline 
#define TAL_CONST const
#define TAL_CALL

#elif defined(__GNUC__)
#define TAL_PLATFORM TAL_PLATFORM_NIX
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
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
#else
#error Unrecognized platform!
//  #else // !WIN32
//      #define TAL_PLATFORM TAL_PLATFORM_UNKNOWN
#endif // !WIN32
#endif // !defined(TAL_PLATFORM)

#if !defined(TAL_PLATFORM)
#error "No TAL platform defined."
#endif // TAL_PLATFORM == TAL_PLATFORM_UNKNOWN

#endif //TAL_PLATFORMS
/* ************************************************************************* **
** ************************************************************************* **
** EOF 
** ************************************************************************* **
** ************************************************************************* */
