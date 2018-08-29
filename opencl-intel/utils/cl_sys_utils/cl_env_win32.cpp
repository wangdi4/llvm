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

#include "cl_env.h"

using namespace Intel::OpenCL::Utils;

#include <windows.h>

cl_err_code Intel::OpenCL::Utils::GetEnvVar(std::string & strVarValue, const std::string strVarName)
{
    // An environment variable has a maximum size limit of 32,767 characters,
    // including the null-terminating character (MSDN).
    DWORD szBufferSize = 32767 * sizeof(CHAR);
    strVarValue.resize(szBufferSize);
    DWORD szWritten =
        GetEnvironmentVariableA(strVarName.c_str(),
                                &strVarValue[0], szBufferSize);
    // szWritten - number of characters stored in the buffer pointed to by the
    // second argument of GetEnvironmentVariableA, not including the terminating
    // null character (MSDN).
    strVarValue.resize(szWritten);
    if (szWritten == 0)
    {
        return CL_ERR_FAILURE;
    }

    return CL_SUCCESS;
}
