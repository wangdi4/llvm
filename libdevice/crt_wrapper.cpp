//==------ crt_wrapper.cpp - wrappers for libc internal functions ----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "wrapper.h"

#if defined(__SPIR__) || defined(__NVPTX__)
#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp declare target
#endif  // OMP_LIBDEVICE
#endif  // INTEL_COLLAB

DEVICE_EXTERN_C_INLINE
void *memcpy(void *dest, const void *src, size_t n) {
  return __devicelib_memcpy(dest, src, n);
}

DEVICE_EXTERN_C_INLINE
void *memset(void *dest, int c, size_t n) {
  return __devicelib_memset(dest, c, n);
}

DEVICE_EXTERN_C_INLINE
int memcmp(const void *s1, const void *s2, size_t n) {
  return __devicelib_memcmp(s1, s2, n);
}

#if defined(_WIN32)
// Truncates a wide (16 or 32 bit) string (wstr) into an ASCII string (str).
// Any non-ASCII characters are replaced by question mark '?'.
static void __truncate_wchar_char_str(const wchar_t *wstr, char *str,
                                      size_t str_size) {
  str_size -= 1; // reserve for NULL terminator
  while (str_size > 0 && *wstr != L'\0') {
    wchar_t w = *wstr++;
    *str++ = (w > 0 && w < 127) ? (char)w : '?';
    str_size--;
  }
  *str = '\0';
}

DEVICE_EXTERN_C
void _wassert(const wchar_t *wexpr, const wchar_t *wfile, unsigned line) {
  // Paths and expressions that are longer than 256 characters are going to be
  // truncated.
  char file[256];
  __truncate_wchar_char_str(wfile, file, sizeof(file));
  char expr[256];
  __truncate_wchar_char_str(wexpr, expr, sizeof(expr));

  __devicelib_assert_fail(
      expr, file, line, /*func=*/nullptr, __spirv_GlobalInvocationId_x(),
      __spirv_GlobalInvocationId_y(), __spirv_GlobalInvocationId_z(),
      __spirv_LocalInvocationId_x(), __spirv_LocalInvocationId_y(),
      __spirv_LocalInvocationId_z());
}
#else
DEVICE_EXTERN_C
void __assert_fail(const char *expr, const char *file, unsigned int line,
                   const char *func) {
  __devicelib_assert_fail(
      expr, file, line, func, __spirv_GlobalInvocationId_x(),
      __spirv_GlobalInvocationId_y(), __spirv_GlobalInvocationId_z(),
      __spirv_LocalInvocationId_x(), __spirv_LocalInvocationId_y(),
      __spirv_LocalInvocationId_z());
}
#endif

#if INTEL_COLLAB
#if OMP_LIBDEVICE
extern "C" {
  typedef void * omp_allocator_handle_t;
  void *__kmpc_alloc(int, size_t, omp_allocator_handle_t);
  void __kmpc_free(int, void *, omp_allocator_handle_t);
}
void operator delete(void *ptr) { __kmpc_free(0, ptr, nullptr); }
void operator delete[](void *ptr) { __kmpc_free(0, ptr, nullptr); }
void *operator new(size_t size) { return __kmpc_alloc(0, size, nullptr); }
void *operator new[](size_t size) { return __kmpc_alloc(0, size, nullptr); }
#pragma omp end declare target
#endif  // OMP_LIBDEVICE
#endif  // INTEL_COLLAB
#endif // __SPIR__ || __NVPTX__
