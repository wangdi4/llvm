// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#pragma once

#include "cl_utils.h"

#include <map>
#include <string>

namespace Intel {
namespace OpenCL {
namespace Utils {
class IAtExitCentralPoint;

/************************************************************************
 * OclDynamicLib is a class that handles load/release and function pointer
 * retrival from the dynmaically loaded libraries
 ************************************************************************/
class OclDynamicLib {
public:
  OclDynamicLib(bool bUnloadOnDestructor = true);
  virtual ~OclDynamicLib();

  // Checks for existance of a file with specified name
  // Input
  //        pLibName    - A pointer to null tirminated string that describes
  //        library file name
  // Returns
  //        true - file exists
  //        false - file doesn't exist
  static bool IsExists(const char *pLibName);

  // Loads a dynamically link library into process address space
  // Input
  //        pLibName    - A pointer to null tirminated string that
  //        describes library file name
  // Returns
  //        0  - if succesully loaded
  //        -1 - if the library is already loaded.
  //        1  - if dlopen fails.
  //             Linux: The reason will be record by m_errInfo.
  //             Windows: If LoadLibraryEx fails on windows, System
  //                      Error Codes will be recorded by m_errInfo.
  //                      Please refer to microsoft documentation.
  int Load(const char *pLibName);

  // Release all allocated resourses and unloads the library
  void Close();

  // Informative functions

  // Returns a number of named functions found in the library
  unsigned int GetNumberOfFunctions() const;

  // Returns a pointer to function name
  // Input
  //        uiFuncId    -    An ordinal number of the function
  // Return
  //        A pointer to valid function name
  //        NULL - if ordinal number is out of bounds
  const char *GetFunctionName(unsigned int uiFuncId) const;

  // Returns a function pointer
  // Input
  //        uiFuncId    -    An ordinal number of the function
  // Return
  //        A pointer to valid function
  //        NULL - if ordinal number is out of bounds
  const void *GetFunctionPtr(unsigned int uiFuncId) const;

  // Returns a function pointer by name
  // Input
  //        szFuncName    -    Function name, null terminated string
  // Return
  //        A pointer to valid function
  //        NULL - if ordinal number is out of bounds
  ptrdiff_t GetFunctionPtrByName(const char *szFuncName) const;

  //
  // Set atexit() notification function
  //
  static void SetGlobalAtExitNotification(IAtExitCentralPoint *fn) {
    m_atexit_fn = fn;
  }

  // Get function pointer from library handle
  static ptrdiff_t GetFuntionPtrByNameFromHandle(void *hLibrary,
                                                 const char *szFuncName);

  const std::string GetError() { return m_vErrInfo; }

protected:
  void *m_hLibrary; // A handle to loaded library

  unsigned int m_uiFuncCount = 0;
  unsigned int *m_pOffsetNames =
      nullptr; // A pointer to offsets of function names
  unsigned int *m_pOffsetFunc = nullptr; // A pointer to offsets of functions

  bool m_bUnloadOnDestructor;

  std::string m_vErrInfo;

  static IAtExitCentralPoint *m_atexit_fn;

  typedef void (*RegisterAtExitNotification_Func)(IAtExitCentralPoint *fn);

#define OclDynamicLib_AT_EXIT_REGISTER_FUNC_NAME                               \
  "RegisterGlobalAtExitNotification"
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
