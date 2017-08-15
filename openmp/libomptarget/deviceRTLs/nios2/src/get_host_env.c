//======== deviceRTLs/nios2/main.c - Target RTLs implementation -*- C -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// getenv-like routine for retrieving host environment variables on Nios II.
///
//===----------------------------------------------------------------------===//

// Null ternminated array of host environment variables. Initial value is
// patched by plugin before offloading target image to device.
const char **__nios2_environ = (const char**) 0xffffffff;

// Returns host environment variable value for the given name.
const char* __nios2_get_host_env(const char *Name) {
  if (__nios2_environ != (const char**) 0xffffffff) {
    for (const char **Env = __nios2_environ; *Env != 0; ++Env) {
      const char *N = Name;
      const char *V = *Env;
      while (*N == *V) {
        ++N;
        ++V;
      }
      if (*N == '\0' && *V == '=') {
        return V + 1;
      }
    }
  }
  return 0;
}

