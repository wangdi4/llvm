// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// Include the platform-specific parts of this class.
#ifdef _MSC_VER
#include <Windows.h>
#include <intrin.h>
#include <tchar.h>
#endif

namespace Validation {

#if defined(i386) || defined(__i386__) || defined(__x86__) ||                  \
    defined(_M_IX86) || defined(__x86_64__) || defined(_M_AMD64) ||            \
    defined(_M_X64)

/// GenINT3 - generate interrupt 3 (debug break)
void GenINT3() {
#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
#if defined(__GNUC__)
  // GCC doesn't know cpuid would clobber ebx/rbx. Preserve it manually.
  asm("int $0x03");
#elif defined(_MSC_VER)
  __debugbreak();
#endif
#elif defined(i386) || defined(__i386__) || defined(__x86__) || defined(_M_IX86)
#if defined(__GNUC__)
  asm("int 3");
#elif defined(_MSC_VER)
  __asm {
      int 3
  }
#endif
#endif
}
#endif
} // namespace Validation
