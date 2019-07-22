//===-------- dlfcn_stub.h - Dynamic linking API stub for Windows ---------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// Declarations of LIBDL APIs for Windows.
///
/// FIXME: this is a temporary solution until we implement dlopen() and dlsym()
///        or use some external implementation
///        (e.g. https://github.com/dlfcn-win32/dlfcn-win32).
///
///===---------------------------------------------------------------------===//

#ifndef DLFCN_H
#define DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_NOW 0

__declspec(dllexport) void *dlopen(const char *file, int mode) {
  return (void *)0;
}

__declspec(dllexport) void *dlsym(void *handle, const char *name) {
  return (void *)0;
}

#ifdef __cplusplus
}
#endif

#endif // DLFCN_H
