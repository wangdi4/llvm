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

#ifndef OMPTARGET_VERIFY_DLL_SIGNATURE
// We don't know if digital signing will follow after build, so we cannot
// use WinVerifyTrust blindly.
#define OMPTARGET_VERIFY_DLL_SIGNATURE 0
#endif

#if OMPTARGET_VERIFY_DLL_SIGNATURE
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#pragma comment(lib, "Wintrust.lib")
#endif // OMPTARGET_VERIFY_DLL_SIGNATURE

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

#if OMPTARGET_VERIFY_DLL_SIGNATURE
/// Safe LoadLibrary wrapper adopted from oneDAAL and from MS document.
/// FileName is supposed to be a full path.
static HMODULE WINAPI loadLibraryWVT(const wchar_t *FileName) {
  // Initialize the WINTRUST_FILE_INFO structure.

  WINTRUST_FILE_INFO FileData;
  memset(&FileData, 0, sizeof(FileData));
  FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
  FileData.pcwszFilePath = FileName;
  FileData.hFile = NULL;
  FileData.pgKnownSubject = NULL;

  GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
  WINTRUST_DATA WinTrustData;

  // Initialize the WinVerifyTrust input data structure.

  // Default all fields to 0.
  memset(&WinTrustData, 0, sizeof(WinTrustData));

  WinTrustData.cbStruct = sizeof(WinTrustData);

  // Use default code signing EKU.
  WinTrustData.pPolicyCallbackData = NULL;

  // No data to pass to SIP.
  WinTrustData.pSIPClientData = NULL;

  // Disable WVT UI.
  WinTrustData.dwUIChoice = WTD_UI_NONE;

  // No revocation checking.
  WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;

  // Verify an embedded signature on a file.
  WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

  // Verify action.
  WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;

  // Verification sets this value.
  WinTrustData.hWVTStateData = NULL;

  // Not used.
  WinTrustData.pwszURLReference = NULL;

  // This is not applicable if there is no UI because it changes
  // the UI to accommodate running applications instead of
  // installing applications.
  WinTrustData.dwUIContext = 0;

  // Set pFile.
  WinTrustData.pFile = &FileData;

  DWORD LastError;
  LONG WVTResult = WinVerifyTrust(NULL, &WVTPolicyGUID, &WinTrustData);

  switch (WVTResult) {
  case TRUST_E_NOSIGNATURE:
    LastError = GetLastError();
    if (TRUST_E_NOSIGNATURE == LastError ||
        TRUST_E_SUBJECT_FORM_UNKNOWN == LastError ||
        TRUST_E_PROVIDER_UNKNOWN == LastError) {
      DP("Error: %ls is not signed.\n", FileName);
    } else {
      DP("Error: Unknown error occurred when verifying the signature of %ls.\n",
         FileName);
    }
    return NULL;
  case TRUST_E_EXPLICIT_DISTRUST:
    DP("Error: The signature/publisher of %ls is disallowed.\n", FileName);
    return NULL;
  case ERROR_SUCCESS:
    break;
  case TRUST_E_SUBJECT_NOT_TRUSTED:
    DP("Error: The signature of %ls in not trusted.\n", FileName);
    return NULL;
  case CRYPT_E_SECURITY_SETTINGS:
    DP("Error: The subject hash or publisher of %ls was not explicitly trusted "
       "and user trust was not allowed (CRYPT_E_SECURITY_SETTINGS).\n",
       FileName);
    return NULL;
  default:
    DP("Error: Unknown error (%" PRId64 ") occurred when verifying the "
       "signature of %ls.\n", WVTResult, FileName);
    return NULL;
  }

  // Release hWVTStateData
  WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
  WVTResult = WinVerifyTrust(NULL, &WVTPolicyGUID, &WinTrustData);
  if (WVTResult != ERROR_SUCCESS) {
    DP("Error: Failed to release verification result.\n");
    return NULL;
  }

  HMODULE Ret = LoadLibraryW(FileName);
  if (Ret) {
    DP("Successfully verified and loaded %ls\n", FileName);
  }

  return Ret;
}
#endif // OMPTARGET_VERIFY_DLL_SIGNATURE

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

#if OMPTARGET_VERIFY_DLL_SIGNATURE
  Ret = loadLibraryWVT(FullPath.c_str());
#else // OMPTARGET_VERIFY_DLL_SIGNATURE
  Ret = LoadLibraryW(FullPath.c_str());
#ifdef OMPTARGET_DEBUG
  if (!static_cast<void*>(Ret)) {
    DP("Call to LoadLibray() was unsuccessful with code 0x%lx\n",
       GetLastError());
  }
#endif // OMPTARGET_DEBUG
#endif // OMPTARGET_VERIFY_DLL_SIGNATURE

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
