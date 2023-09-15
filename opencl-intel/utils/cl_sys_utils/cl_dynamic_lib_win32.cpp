// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#include "cl_dynamic_lib.h"
#include "cl_sys_info.h"

#include <windows.h>

using namespace Intel::OpenCL::Utils;

IAtExitCentralPoint *OclDynamicLib::m_atexit_fn = nullptr;

// Get function pointer from library handle
ptrdiff_t OclDynamicLib::GetFuntionPtrByNameFromHandle(void *hLibrary,
                                                       const char *szFuncName) {
  return (ptrdiff_t)GetProcAddress((HMODULE)hLibrary, szFuncName);
}

OclDynamicLib::OclDynamicLib(bool bUnloadOnDestructor)
    : m_hLibrary(nullptr), m_bUnloadOnDestructor(bUnloadOnDestructor),
      m_vErrInfo("") {}

OclDynamicLib::~OclDynamicLib() {
  if (m_bUnloadOnDestructor) {
    Close();
  }
}

// ------------------------------------------------------------------------------
// Checks for existance of a file with specified name
bool OclDynamicLib::IsExists(const char *pLibName) {
  DWORD rc;

  rc = GetFileAttributesA(pLibName);

  return (INVALID_FILE_ATTRIBUTES != rc);
}

// ------------------------------------------------------------------------------
// Loads a dynamically link library into process address space
int OclDynamicLib::Load(const char *pLibName) {
  if (nullptr != m_hLibrary) {
    return -1;
  }

  // Load library
  m_hLibrary = LoadLibraryEx(pLibName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

  if (nullptr == m_hLibrary) {
    m_vErrInfo =
        std::string("Windows error code: " + std::to_string(GetLastError()));
    return 1;
  }

  return 0;
}

// Loads a dynamically link library into process address space
void OclDynamicLib::Close() {
  if (nullptr == m_hLibrary) {
    return;
  }

  FreeLibrary((HMODULE)m_hLibrary);
  m_hLibrary = nullptr;
}

// Returns a function pointer
ptrdiff_t OclDynamicLib::GetFunctionPtrByName(const char *szFuncName) const {
  if (nullptr == m_hLibrary) {
    return NULL;
  }

  return (ptrdiff_t)GetProcAddress((HMODULE)m_hLibrary, szFuncName);
}
