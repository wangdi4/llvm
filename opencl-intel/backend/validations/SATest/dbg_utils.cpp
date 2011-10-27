/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  mem_utils.cpp

\*****************************************************************************/

// Include the platform-specific parts of this class.
#ifdef _MSC_VER
    #include <tchar.h>
    #include <Windows.h>
    #include <intrin.h>
#endif

namespace Validation 
{ 

#if defined(i386) || defined(__i386__) || defined(__x86__) || defined(_M_IX86)\
 || defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64)

/// GenINT3 - generate interrupt 3 (debug break) 
void GenINT3() {
#if defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64)
  #if defined(__GNUC__)
    // gcc doesn't know cpuid would clobber ebx/rbx. Preseve it manually.
    asm ("int $0x03");
  #elif defined(_MSC_VER)
    __debugbreak();
  #endif
#elif defined(i386) || defined(__i386__) || defined(__x86__) || defined(_M_IX86)
  #if defined(__GNUC__)
    asm ("int 3");
  #elif defined(_MSC_VER)
    __asm {
      int 3
    }
  #endif
#endif
}
#endif
} // namespace