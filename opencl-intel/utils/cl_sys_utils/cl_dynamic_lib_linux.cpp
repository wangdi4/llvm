// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation. All rights reserved.
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

#include <assert.h>
#include <dlfcn.h>
#include <sys/stat.h>

using namespace Intel::OpenCL::Utils;

IAtExitCentralPoint *OclDynamicLib::m_atexit_fn = nullptr;

// Get function pointer from library handle
ptrdiff_t OclDynamicLib::GetFuntionPtrByNameFromHandle(void *hLibrary,
                                                       const char *szFuncName) {
  // clear errors
  dlerror();
  void *func = dlsym(hLibrary, szFuncName);
  const char *error;
  if ((error = dlerror()) != nullptr) {
    return 0;
  }

  return (ptrdiff_t)func;
}

OclDynamicLib::OclDynamicLib(bool bUnloadOnDestructor)
    : m_hLibrary(nullptr), m_bUnloadOnDestructor(bUnloadOnDestructor),
      m_vErrInfo("") {}

OclDynamicLib::~OclDynamicLib() {
  if (m_bUnloadOnDestructor) {
    Close();
  }
}

// -----------------------------------------------------------------------------
// Checks for existance of a file with specified name
bool OclDynamicLib::IsExists(const char *pLibName) {
  struct stat stFileInfo;
  int rc;

  rc = stat(pLibName, &stFileInfo);

  return (0 == rc);
}

// -----------------------------------------------------------------------------
// Loads a dynamically link library into process address space
int OclDynamicLib::Load(const char *pLibName) {
  if (nullptr != m_hLibrary) {
    return -1;
  }

  // Load library
  std::string strLibName(MAX_PATH, '\0');
  // Get a full path of a library from where the function was called. We
  // expect, that a callee library is having the same path as the caller.
  // Add this path to string to pass in dlopen function.
  Intel::OpenCL::Utils::GetModuleDirectory(&strLibName[0], MAX_PATH);
  strLibName.resize(strLibName.find_first_of('\0'));
  // To make library name canonical add a version string at its ending.
  strLibName +=
      std::string(pLibName) + std::string(".") + std::string(VERSIONSTRING);
  m_hLibrary = dlopen(strLibName.c_str(), RTLD_LAZY);

  if (nullptr == m_hLibrary) {
    // We didn't find the called library by a full path. Step back and try
    // to find it disregarding the calculated path.
    strLibName =
        std::string(pLibName) + std::string(".") + std::string(VERSIONSTRING);
    m_hLibrary = dlopen(strLibName.c_str(), RTLD_LAZY);
    if (nullptr == m_hLibrary) {
      const char *strDllError = dlerror();
      m_vErrInfo =
          std::string(nullptr == strDllError ? "Unknown reason." : strDllError);
      return 1;
    }
  }

  RegisterAtExitNotification_Func AtExitFunc =
      (RegisterAtExitNotification_Func)GetFunctionPtrByName(
          OclDynamicLib_AT_EXIT_REGISTER_FUNC_NAME);

  if (nullptr != AtExitFunc) {
    AtExitFunc(m_atexit_fn);
  }

  return 0;
}

// Loads a dynamically link library into process address space
void OclDynamicLib::Close() {
  if (nullptr == m_hLibrary) {
    return;
  }

  dlclose(m_hLibrary);
  m_hLibrary = nullptr;
}

// Returns a function pointer
ptrdiff_t OclDynamicLib::GetFunctionPtrByName(const char *szFuncName) const {
  if (nullptr == m_hLibrary) {
    return 0;
  }

  void *func;
  const char *error;

  // clear errors
  dlerror();
  func = dlsym(m_hLibrary, szFuncName);
  if ((error = dlerror()) != nullptr) {
    return 0;
  }

  return (ptrdiff_t)func;
}
