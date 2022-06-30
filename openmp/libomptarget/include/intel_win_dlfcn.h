//===-------- intel_win_dlfcn.h - Dynamic linking API stub for Windows ----===//
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
/// Implementation of LIBDL API for Windows.
///
///===---------------------------------------------------------------------===//

#ifndef DLFCN_H
#define DLFCN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>

#define WIN32
#define WIN32_LEAN_AND_MEAN
#pragma warning( push )
#pragma warning( disable: 271 310 )
#include <windows.h>
#pragma warning( pop )

#define RTLD_NOW 0

static std::string LastErrorMessage;

static wchar_t * utf8_to_wchar(const char *in) {
  // Account for multi-byte chars (up to 4 bytes per char for UTF8)
  size_t str_len = strnlen_s(in, MAX_PATH*4 + 1);

  // Worst case, every char in the multi-byte coded string can turn into a
  // single wchar_t
  wchar_t * new_str = (wchar_t *) malloc((str_len + 1) * sizeof(wchar_t));
#ifdef OMPTARGET_DEBUG
  if (!new_str)
    DP("utf8_to_wchar(): malloc failed\n");
#endif // OMPTARGET_DEBUG

  if (new_str) {
    int new_str_len = MultiByteToWideChar(CP_UTF8, 0, in, str_len, new_str,
        str_len + 1);
    new_str[new_str_len] = 0;
  }

  return new_str;
}

#ifdef __cplusplus
extern "C" {
#endif

void *dlopen(const char *File, int Mode) {
  // By default, provide minimum safety as follows.
  // -- Resolve absolute path
  // -- Exclude users' CWD when loading dll.
  wchar_t CurrModulePath[MAX_PATH];
  HMODULE CurrModule = NULL;
  if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCWSTR) &LastErrorMessage, &CurrModule)) {
    DP("Cannot find omptarget module\n");
    return NULL;
  }
  if (!GetModuleFileNameW(CurrModule, CurrModulePath, sizeof(CurrModulePath))) {
    DP("Cannot resolve absolute path of omptarget module\n");
    return NULL;
  }
  wchar_t *FilePath = utf8_to_wchar(File);
  std::wstring FullPath(CurrModulePath);
  size_t Loc = FullPath.find_last_of('\\');
  FullPath.replace(Loc + 1, std::wstring::npos, FilePath);
  free(FilePath);

  // Exclude users' current directory from the search path
  if (!SetDllDirectoryA("")) {
    DP("Error: Cannot exclude current directory from serch path.\n");
    return NULL;
  }

  // Do not display message box with error if the call below fails to load the
  // dynamic library.
  int ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
                               SEM_NOOPENFILEERRORBOX);

  HMODULE Ret = NULL;

  Ret = LoadLibraryW(FullPath.c_str());
#ifdef OMPTARGET_DEBUG
  if (!static_cast<void*>(Ret)) {
    DP("Call to LoadLibray() was unsuccessful with code 0x%lx\n",
       GetLastError());
  }
#endif // OMPTARGET_DEBUG

  // Restore error mode
  SetErrorMode(ErrorMode);

  // Restore current directory from the search path
  SetDllDirectory(NULL);

  return static_cast<void*>(Ret);
}

int dlclose(void *handle) {
  return FreeLibrary(static_cast<HMODULE>(handle));
}

void *dlsym(void *handle, const char *name) {
  void *addr = reinterpret_cast<void *>(
      GetProcAddress(static_cast<HMODULE>(handle), name));
  return addr;
}

// Covert GetLastError() to string.
char *dlerror(void) {
  LastErrorMessage.clear();
  DWORD error = GetLastError();
  if (error) {
    LPVOID lpMsgBuf;
    DWORD bufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPTSTR) &lpMsgBuf, 0, NULL);
    if (bufLen) {
      LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
      LastErrorMessage.assign(lpMsgStr, lpMsgStr+bufLen);
      LocalFree(lpMsgBuf);
    }
  }

  return const_cast<char *>(LastErrorMessage.c_str());
}

#ifdef __cplusplus
}
#endif

#endif // DLFCN_H
