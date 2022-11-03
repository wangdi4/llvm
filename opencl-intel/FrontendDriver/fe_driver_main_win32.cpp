// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "Logger.h"
#include "cl_disable_sys_dialog.h"
#include "cl_dynamic_lib.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "fe_driver_main.h"
#include <Windows.h>
#include <string>

#pragma comment(lib, "cl_sys_utils.lib")

using namespace Intel::OpenCL::Utils;
using Intel::OpenCL::Utils::g_pUserLogger;

namespace {
OclDynamicLib *m_dlClangLib;
LoggerClient *m_pLoggerClient;
} // namespace

std::string GetDriverStorePath() {
  char dllPath[MAX_PATH];
#ifdef _M_X64
  const char *crt_name = "IntelOpenCL64.dll";
#else  //(_M_X64)
  const char *crt_name = "IntelOpenCL32.dll";
#endif //(_M_X64)
  DWORD Length =
      GetModuleFileNameA(GetModuleHandleA(crt_name), dllPath, MAX_PATH);

  for (DWORD l = Length; l > 0; l--) {
    if (dllPath[l - 1] == '\\') {
      dllPath[l] = '\0';
      break;
    }
  }

  return std::string(dllPath);
}

bool LoadCommonClang() {
  // CCLANG_LIB_NAME is defined in CMakeLists.txt
  std::string clangPath = GetDriverStorePath() + CCLANG_LIB_NAME;
  if (m_dlClangLib->Load(clangPath.c_str()) != 0) {
    // Failed to load library from Driver Store
    // Try to load from the location of current library
    clangPath = std::string(MAX_PATH, '\0');

    Intel::OpenCL::Utils::GetModuleDirectory(&clangPath[0], MAX_PATH);
    clangPath.resize(clangPath.find_first_of('\0'));
    clangPath += CCLANG_LIB_NAME;

    if (m_dlClangLib->Load(clangPath.c_str()) != 0) {
      LogErrorA("Failed to load common clang from %s", clangPath.c_str());
      if (g_pUserLogger && g_pUserLogger->IsErrorLoggingEnabled())
        g_pUserLogger->PrintError(
            "Failed to load " + std::string(CCLANG_LIB_NAME) +
            " with error info: " + m_dlClangLib->GetError());

      return false;
    }
  }
  return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    INIT_LOGGER_CLIENT("FrontendDriver", LL_DEBUG);
#if !defined(INTEL_PRODUCT_RELEASE) && !defined(_DEBUG)
    Intel::OpenCL::Utils::DisableSystemDialogsOnCrash();
#endif
    m_dlClangLib = new OclDynamicLib;
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    delete m_dlClangLib;
    break;
  }
  return TRUE;
}
