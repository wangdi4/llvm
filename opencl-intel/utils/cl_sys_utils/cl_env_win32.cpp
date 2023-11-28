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

#include "cl_env.h"
#include <windows.h>

using namespace Intel::OpenCL::Utils;

bool Intel::OpenCL::Utils::getEnvVar(std::string &EnvVal,
                                     const std::string &EnvName) {
  // An environment variable has a maximum size limit of 32,767 characters,
  // including the null-terminating character (MSDN).
  DWORD BufferSize = 32767 * sizeof(CHAR);
  EnvVal.resize(BufferSize);
  DWORD SizeWritten =
      GetEnvironmentVariableA(EnvName.c_str(), &EnvVal[0], BufferSize);
  // SizeWritten - number of characters stored in the buffer pointed to by the
  // second argument of GetEnvironmentVariableA, not including the terminating
  // null character (MSDN).
  EnvVal.resize(SizeWritten);
  // If GetEnvironmentVariableA fails, the return value is zero. If the
  // specified environment variable was not found in the environment block,
  // GetLastError returns ERROR_ENVVAR_NOT_FOUND (MSDN).
  if (SizeWritten == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
    return false;

  return true;
}
