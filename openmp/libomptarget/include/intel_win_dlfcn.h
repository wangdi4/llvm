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

static wchar_t *utf8ToWchar(const char *In) {
  // Account for multi-byte chars (up to 4 bytes per char for UTF8)
  size_t StrLen = strnlen_s(In, MAX_PATH*4 + 1);

  // Worst case, every char in the multi-byte coded string can turn into a
  // single wchar_t
  wchar_t *NewStr = (wchar_t *)malloc((StrLen + 1) * sizeof(wchar_t));
#ifdef OMPTARGET_DEBUG
  if (!NewStr)
    DP("utf8ToWchar(): malloc failed\n");
#endif // OMPTARGET_DEBUG

  if (NewStr) {
    int NewStrLen = MultiByteToWideChar(CP_UTF8, 0, In, StrLen, NewStr,
        StrLen + 1);
    NewStr[NewStrLen] = 0;
  }

  return NewStr;
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
  wchar_t *FilePath = utf8ToWchar(File);
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

int dlclose(void *Handle) {
  return FreeLibrary(static_cast<HMODULE>(Handle));
}

void *dlsym(void *Handle, const char *Name) {
  void *Addr = reinterpret_cast<void *>(
      GetProcAddress(static_cast<HMODULE>(Handle), Name));
  return Addr;
}

// Covert GetLastError() to string.
char *dlerror(void) {
  LastErrorMessage.clear();
  DWORD Error = GetLastError();
  if (Error) {
    LPVOID LPMsgBuf;
    DWORD BufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS, NULL, Error,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPTSTR) &LPMsgBuf, 0, NULL);
    if (BufLen) {
      LPCSTR LPMsgStr = (LPCSTR)LPMsgBuf;
      LastErrorMessage.assign(LPMsgStr, LPMsgStr + BufLen);
      LocalFree(LPMsgBuf);
    }
  }

  return const_cast<char *>(LastErrorMessage.c_str());
}

#ifdef __cplusplus
}
#endif

#endif // DLFCN_H
